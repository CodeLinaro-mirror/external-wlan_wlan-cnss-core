// SPDX-License-Identifier: GPL-2.0-only
/**
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/devcoredump.h>
#include <linux/dma-direction.h>
#ifdef CONFIG_CNSS_OUT_OF_TREE
#include "linux_inc/linux/mhi.h"
#else
#include <linux/mhi.h>
#endif
#include "pci.h"
#include "debug.h"
#include "coredump.h"

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

int cnss_coredump_remote_dump(struct cnss_plat_data *plat_priv)
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
	cnss_pr_err("[FOR PARSING VmCore] remote_dump mem: 0x%llx, size: %u\n",
		    crash_data->remote_buf, offset);
	return 0;
}

static size_t cnss_get_qdss_buf_len(struct cnss_fw_mem *qdss_mem)
{
	unsigned int i;
	size_t len = 0;

	for (i = 0; i < QMI_WLFW_MAX_NUM_MEM_SEG_V01; i++) {
		if (qdss_mem[i].va && qdss_mem[i].size)
			len += qdss_mem[i].size;
	}

	return len;
}

int cnss_coredump_qdss_dump(struct cnss_pci_data *pci_priv)
{
	struct cnss_fw_mem *qdss_mem = pci_priv->plat_priv->qdss_mem;
	struct mhi_fw_crash_data *crash_data = &pci_priv->plat_priv->fw_crash_data;
	u32 offset = 0;
	u8 i;

	crash_data->qdss_dump_buf_len = cnss_get_qdss_buf_len(qdss_mem);

	crash_data->qdss_dump_buf = vzalloc(crash_data->qdss_dump_buf_len);

	if (!crash_data->qdss_dump_buf)
		return -ENOMEM;

	for (i = 0; i < QMI_WLFW_MAX_NUM_MEM_SEG_V01; i++) {
		if (qdss_mem[i].va && qdss_mem[i].size) {
			cnss_pr_err("qdss mem: 0x%p, size: 0x%lx\n",
					qdss_mem[i].va,
					qdss_mem[i].size);
			memcpy(crash_data->qdss_dump_buf + offset,
					qdss_mem[i].va, qdss_mem[i].size);
			offset += qdss_mem[i].size;
		}
	}
	cnss_pr_err("[FOR PARSING VmCore] qdss_dump mem: 0x%llx, size: %u\n",
			crash_data->qdss_dump_buf, offset);

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
	int entries = 0;

	if (!img) {
		cnss_pr_err("rddm img null, skip rddm ramdump\n");
		return 0;
	}
	entries = img->entries;

	crash_data->ramdump_buf_len = (entries - 1) * mhi_cntrl->seg_len +
		(entries - 1) * sizeof(struct mhi_vec_entry);

	cnss_pr_err("entries=%d, ramdump_buf_len:%d\n", entries, crash_data->ramdump_buf_len);

	crash_data->ramdump_buf = vzalloc(crash_data->ramdump_buf_len);
	if (!crash_data->ramdump_buf)
		return -ENOMEM;

	for (seg = 0; seg < entries; seg++) {
		buf = img->mhi_buf[seg].buf;
		size = img->mhi_buf[seg].len;
		cnss_pr_err("write rddm memory: mem: 0x%p, size: 0x%x\n",
			    buf, size);
		memcpy(crash_data->ramdump_buf + seg * size, buf, size);
	}
	cnss_pr_err("[FOR PARSING VmCore] fw_rddm_dump mem: 0x%llx, size: %d\n",
		    crash_data->ramdump_buf, crash_data->ramdump_buf_len);

	return 0;
}

int cnss_coredump_fw_paging_dump(struct cnss_pci_data *pci_priv)
{
	struct mhi_controller *mhi_cntrl = pci_priv->mhi_ctrl;
	struct image_info *img = mhi_cntrl->fbc_image;
	struct mhi_fw_crash_data *crash_data = &pci_priv->plat_priv->fw_crash_data;
	char *buf = NULL;
	unsigned int size = 0;
	int seg = 0;

	if (!img) {
		cnss_pr_err("fbc image null, skip paging dump\n");
		return 0;
	}

	crash_data->paging_dump_buf_len = (img->entries - 1) * mhi_cntrl->seg_len +
					(img->entries - 1) * sizeof(struct mhi_vec_entry);

	cnss_pr_err("entries=%d, fwdump_buf_len=%d\n",
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
	cnss_pr_err("last block: mem: 0x%p, size: 0x%x\n", buf, size);
	cnss_pr_err("[FOR PARSING VmCore] fw_paging_dump mem: 0x%llx, size: %d\n",
		    crash_data->paging_dump_buf, crash_data->paging_dump_buf_len);

	return 0;
}

static struct cnss_dump_file_data * cnss_coredump_build(struct cnss_plat_data *plat_priv)

{
	struct mhi_fw_crash_data *crash_data = &(plat_priv->fw_crash_data);
	struct fw_remote_crash_data *remote_crash_data = &(plat_priv->remote_crash_data);

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
	len += sizeof(*dump_tlv) + crash_data->sram_dump_buf_len;
	len += sizeof(*dump_tlv) + crash_data->qdss_dump_buf_len;

	sofar += hdr_len;

	/* This is going to get big when we start dumping FW RAM and such,
	 * so go ahead and use vmalloc.
	 */
	buf = vzalloc(len);
	if (!buf)
		return NULL;

	dump_data = (struct cnss_dump_file_data *)(buf);
	strscpy(dump_data->df_magic, "CNSS_FW_DUMP",
		sizeof(dump_data->df_magic));
	dump_data->len = cpu_to_le32(len);
	dump_data->version = cpu_to_le32(CNSS_FW_CRASH_DUMP_VERSION);
	guid_gen(&dump_data->guid);
	ktime_get_real_ts64(&timestamp);
	dump_data->tv_sec = cpu_to_le64(timestamp.tv_sec);
	dump_data->tv_nsec = cpu_to_le64(timestamp.tv_nsec);
	dump_data->crash_reason = crash_data->reason;

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

	/* gather sram memory */
	dump_tlv = (struct cnss_tlv_dump_data *)(buf + sofar);
	dump_tlv->type = cpu_to_le32(CNSS_FW_CRASH_SRAM_DATA);
	dump_tlv->tlv_len = cpu_to_le32(crash_data->sram_dump_buf_len);
	memcpy(dump_tlv->tlv_data, crash_data->sram_dump_buf,
	       crash_data->sram_dump_buf_len);
	sofar += sizeof(*dump_tlv) + crash_data->sram_dump_buf_len;

	/* gather qdss memory */
	dump_tlv = (struct cnss_tlv_dump_data *)(buf + sofar);
	dump_tlv->type = cpu_to_le32(CNSS_FW_QDSS_MEM_DATA);
	dump_tlv->tlv_len = cpu_to_le32(crash_data->qdss_dump_buf_len);
	memcpy(dump_tlv->tlv_data, crash_data->qdss_dump_buf,
	       crash_data->qdss_dump_buf_len);
	sofar += sizeof(*dump_tlv) + crash_data->qdss_dump_buf_len;

	return dump_data;
}

int cnss_qcom_devcd_dump(struct device *dev, void *data, size_t datalen,
				gfp_t gfp);

int cnss_coredump_submit(struct cnss_pci_data *pci_priv)
{
	struct cnss_dump_file_data *dump;

	dump = cnss_coredump_build(pci_priv->plat_priv);

	if (!dump)
		return -ENODATA;

	cnss_save_buf_to_file((char *)dump, dump->len, "/var/crash/Trieste%s.bin");
	cnss_qcom_devcd_dump(pci_priv->mhi_ctrl->cntrl_dev, dump, le32_to_cpu(dump->len), GFP_KERNEL);
	cnss_invoke_qca_dump_app(FW_RDDM_DUMP);
	cnss_pr_info("fw core devcoredump\n");

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
	cnss_coredump_qdss_dump(pci_priv);
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
