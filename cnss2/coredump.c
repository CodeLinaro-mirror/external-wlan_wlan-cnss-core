// SPDX-License-Identifier: GPL-2.0-only
/**
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifdef CONFIG_DUMP_FW_TO_FILE
#include <linux/export.h>
#include <linux/rtc.h>
#include <linux/fs.h>
#include <linux/version.h>
#endif

#include <linux/devcoredump.h>
#include <linux/dma-direction.h>
#include <linux/mhi.h>
#include "coredump.h"
#include "pci.h"
#include "debug.h"

static size_t cnss_get_remote_buf_len(struct fw_remote_mem *fw_mem)
{
	unsigned int i;
	size_t len = 0;

	for (i = 0; i < BHI_WLFW_MAX_NUM_MEM_SEG_V01; i++) {
		if (fw_mem[i].vaddr && fw_mem[i].size)
			len += fw_mem[i].size;
	}

	return len;
}

static int cnss_coredump_remote_dump(struct cnss_plat_data *plat_priv)
{
	struct fw_remote_crash_data *crash_data = &plat_priv->remote_crash_data;
	struct fw_remote_mem *fw_mem = plat_priv->remote_mem;
	u32 offset = 0;
	u8 i;

	crash_data->remote_buf_len = cnss_get_remote_buf_len(fw_mem);
	cnss_pr_err("%s remote buffer len=%lu\n", __func__,
		    crash_data->remote_buf_len);

	crash_data->remote_buf = vzalloc(crash_data->remote_buf_len);
	if (!crash_data->remote_buf)
		return -ENOMEM;

	for (i = 0; i < BHI_WLFW_MAX_NUM_MEM_SEG_V01; i++) {
		if (fw_mem[i].vaddr && fw_mem[i].size) {
			cnss_pr_err("remote mem: 0x%p, size: 0x%lx\n",
				    fw_mem[i].vaddr,
				    fw_mem[i].size);
			memcpy(crash_data->remote_buf + offset,
			       fw_mem[i].vaddr, fw_mem[i].size);
			offset += fw_mem[i].size;
		}
	}
	return 0;
}

static int cnss_coredump_fw_rddm_dump(struct cnss_pci_data *pci_priv)
{
	struct mhi_controller *mhi_cntrl = pci_priv->mhi_ctrl;
	struct mhi_fw_crash_data *crash_data = &pci_priv->plat_priv->fw_crash_data;
	struct image_info *img = mhi_cntrl->rddm_image;
	char *buf = NULL;
	unsigned int size = 0;
	int seg = 0;
	int entries = img->entries;

	crash_data->ramdump_buf_len = (entries - 1) * mhi_cntrl->seg_len +
		(entries - 1) * sizeof(struct mhi_vec_entry);

	cnss_pr_err("entries=%d, ramdump_buf_len:%d\n", entries, crash_data->ramdump_buf_len);

	crash_data->ramdump_buf = vzalloc(crash_data->ramdump_buf_len);
	if (!crash_data->ramdump_buf)
		return -ENOMEM;

	for (seg = 0; seg < entries; seg++) {
		buf = img->mhi_buf[seg].buf;
		size = img->mhi_buf[seg].len;
		cnss_pr_err(
			    "write rddm memory: mem: 0x%p, size: 0x%x\n",
			    buf, size);
		memcpy(crash_data->ramdump_buf + seg * size, buf, size);
	}

	return 0;
}

static int cnss_coredump_fw_paging_dump(struct cnss_pci_data *pci_priv)
{
	struct mhi_controller *mhi_cntrl = pci_priv->mhi_ctrl;
	struct image_info *img = mhi_cntrl->fbc_image;
	struct mhi_fw_crash_data *crash_data = &pci_priv->plat_priv->fw_crash_data;
	char *buf = NULL;
	unsigned int size = 0;
	int seg = 0;

	crash_data->paging_dump_buf_len = (img->entries - 1) * mhi_cntrl->seg_len +
					(img->entries - 1) * sizeof(struct mhi_vec_entry);

	cnss_pr_dbg("entries=%d, fwdump_buf_len=%d\n",
		     img->entries, crash_data->paging_dump_buf_len);

	crash_data->paging_dump_buf = vzalloc(crash_data->paging_dump_buf_len);
	if (!crash_data->paging_dump_buf)
		return -ENOMEM;

	for (seg = 0; seg < img->entries; seg++) {
		buf = img->mhi_buf[seg].buf;
		size = img->mhi_buf[seg].len;
		memcpy(crash_data->paging_dump_buf + seg * size, buf, size);
	}

	buf = crash_data->paging_dump_buf + seg * size;
	size = img->mhi_buf[img->entries - 1].len;
	cnss_pr_info("to write last block: mem: 0x%p, size: 0x%x\n",
		    buf, size);

	return 0;
}

static struct cnss_dump_file_data *
cnss_coredump_build(struct mhi_fw_crash_data *crash_data,
		      struct fw_remote_crash_data *remote_crash_data)
{
	struct cnss_dump_file_data *dump_data;
	struct cnss_tlv_dump_data *dump_tlv;
	size_t hdr_len = sizeof(*dump_data);
	size_t len, sofar = 0;
	unsigned char *buf;
	struct timespec64 timestamp;

	len = hdr_len;

	len += sizeof(*dump_tlv) + crash_data->paging_dump_buf_len;
	len += sizeof(*dump_tlv) + crash_data->ramdump_buf_len;
	len += sizeof(*dump_tlv) + remote_crash_data->remote_buf_len;

	sofar += hdr_len;

	/* This is going to get big when we start dumping FW RAM and such,
	 * so go ahead and use vmalloc.
	 */
	buf = vzalloc(len);
	if (!buf)
		return NULL;

	dump_data = (struct cnss_dump_file_data *)(buf);
	strscpy(dump_data->df_magic, "ATH11K-FW-DUMP",
		sizeof(dump_data->df_magic));
	dump_data->len = cpu_to_le32(len);
	dump_data->version = cpu_to_le32(CNSS_FW_CRASH_DUMP_VERSION);
	guid_gen(&dump_data->guid);
	ktime_get_real_ts64(&timestamp);
	dump_data->tv_sec = cpu_to_le64(timestamp.tv_sec);
	dump_data->tv_nsec = cpu_to_le64(timestamp.tv_nsec);

	/* Gather FW paging dump */
	dump_tlv = (struct cnss_tlv_dump_data *)(buf + sofar);
	dump_tlv->type = cpu_to_le32(CNSS_FW_CRASH_PAGING_DATA);
	dump_tlv->tlv_len = cpu_to_le32(crash_data->paging_dump_buf_len);
	memcpy(dump_tlv->tlv_data, crash_data->paging_dump_buf,
	       crash_data->paging_dump_buf_len);
	sofar += sizeof(*dump_tlv) + crash_data->paging_dump_buf_len;

	/* Gather RDDM dump */
	dump_tlv = (struct cnss_tlv_dump_data *)(buf + sofar);
	dump_tlv->type = cpu_to_le32(CNSS_FW_CRASH_RDDM_DATA);
	dump_tlv->tlv_len = cpu_to_le32(crash_data->ramdump_buf_len);
	memcpy(dump_tlv->tlv_data, crash_data->ramdump_buf,
	       crash_data->ramdump_buf_len);
	sofar += sizeof(*dump_tlv) + crash_data->ramdump_buf_len;

	/* gather remote memory */
	dump_tlv = (struct cnss_tlv_dump_data *)(buf + sofar);
	dump_tlv->type = cpu_to_le32(CNSS_FW_REMOTE_MEM_DATA);
	dump_tlv->tlv_len = cpu_to_le32(remote_crash_data->remote_buf_len);
	memcpy(dump_tlv->tlv_data, remote_crash_data->remote_buf,
	       remote_crash_data->remote_buf_len);
	sofar += sizeof(*dump_tlv) + remote_crash_data->remote_buf_len;

	return dump_data;
}

int cnss_qcom_devcd_dump(struct device *dev, void *data, size_t datalen,
				gfp_t gfp);

#ifdef CONFIG_DUMP_FW_TO_FILE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#define vfs_write kernel_write
#endif

static int get_time_of_the_day_in_hr_min_sec(char *tbuf, int len)
{
	struct timespec64 tv;
	struct rtc_time tm;
	int time_len = 0;

	ktime_get_real_ts64(&tv);
	/* Convert rtc to local time */
	tv.tv_sec -= sys_tz.tz_minuteswest * 60;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	rtc_time64_to_tm(tv.tv_sec, &tm);
#else
	rtc_time_to_tm(tv.tv_sec, &tm);
#endif
	time_len = scnprintf(tbuf, len,
		"%04d-%02d-%02d-%02d-%02d-%02d-",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
	return time_len;
}

#define BUF_SIZE 64
static int dump_fw_to_file(struct cnss_dump_file_data *dump)
{
	char file_full_path[BUF_SIZE];
	char time_buf[24];
	int len = 0;
	struct file *fp;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	mm_segment_t fs;
#endif
	loff_t pos;
	int status = 0;

	memset(file_full_path, 0, sizeof(file_full_path));
	len = get_time_of_the_day_in_hr_min_sec(time_buf, sizeof(time_buf));
	len = scnprintf(file_full_path,
			sizeof(file_full_path),
			"/var/crash/Trieste%s.bin",
			time_buf);
	cnss_pr_err("enter\n");
	fp = filp_open(file_full_path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		cnss_pr_err("create file:%s error\n",
			file_full_path);
		return -EIO;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	fs = get_fs();
	set_fs(KERNEL_DS);
#endif
	pos = 0;
	cnss_pr_err("to write file:%s, mem: 0x%p, size: 0x%x\n",
		file_full_path,
		dump,
		(unsigned int)(dump->len));
	status = vfs_write(fp,
			   (const char __user *)(dump),
			   dump->len,
			   &pos);
	if (status < 0) {
		cnss_pr_err("write file:%s error\n",
			file_full_path);
		return status;
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		cnss_pr_err("close file: %s, error\n",
			file_full_path);
		return status;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	set_fs(fs);
#endif
	cnss_pr_err("exit\n");
	return status;
}
#else
static int dump_fw_to_file(struct cnss_dump_file_data *dump)
{
	return 0;
}
#endif

static int cnss_coredump_submit(struct cnss_pci_data *pci_priv)
{
	struct cnss_dump_file_data *dump;

	dump = cnss_coredump_build(&pci_priv->plat_priv->fw_crash_data,
				     &pci_priv->plat_priv->remote_crash_data);
	if (!dump)
		return -ENODATA;

	dump_fw_to_file(dump);
	cnss_qcom_devcd_dump(pci_priv->mhi_ctrl->cntrl_dev, dump, le32_to_cpu(dump->len), GFP_KERNEL);

	return 0;
}

static void cnss_coredump_buf_release(struct cnss_pci_data *pci_priv)
{
	struct fw_remote_crash_data *remote = &pci_priv->plat_priv->remote_crash_data;
	struct mhi_fw_crash_data *mhi = &pci_priv->plat_priv->fw_crash_data;

	if (remote->remote_buf) {
		vfree(remote->remote_buf);
		remote->remote_buf = NULL;
	}

	if (mhi->ramdump_buf) {
		vfree(mhi->ramdump_buf);
		mhi->ramdump_buf = NULL;
	}

	if (mhi->paging_dump_buf) {
		vfree(mhi->paging_dump_buf);
		mhi->paging_dump_buf = NULL;
	}
}

void cnss_rddm_submit(void *bus_priv)
{
	struct cnss_pci_data *pci_priv = (struct cnss_pci_data *)bus_priv;

	cnss_coredump_submit(pci_priv);
}

void cnss_rddm_collect(void *bus_priv)
{
	struct cnss_pci_data *pci_priv = (struct cnss_pci_data *)bus_priv;

	//mhi_download_rddm_image(pci_priv->mhi_ctrl, false);

	cnss_coredump_fw_rddm_dump(pci_priv);
	cnss_coredump_fw_paging_dump(pci_priv);
	cnss_coredump_remote_dump(pci_priv->plat_priv);
}

void cnss_mhi_pm_rddm_worker(struct work_struct *work)
{
	struct cnss_pci_data *pci_priv = container_of(work,
						 struct cnss_pci_data,
						 rddm_worker);

	mhi_download_rddm_image(pci_priv->mhi_ctrl, false);

	cnss_coredump_fw_rddm_dump(pci_priv);
	cnss_coredump_fw_paging_dump(pci_priv);
	cnss_coredump_remote_dump(pci_priv->plat_priv);

	cnss_coredump_submit(pci_priv);
	cnss_coredump_buf_release(pci_priv);
}
EXPORT_SYMBOL(cnss_mhi_pm_rddm_worker);
