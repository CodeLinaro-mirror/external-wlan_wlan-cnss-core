/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/export.h>
#include <linux/rtc.h>

#include "mhi.h"
#include "mhi_bhi.h"
#include "mhi_sys.h"

#include <linux/version.h>
#include "cnss_module.h"

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
	rtc_time_to_tm(tv.tv_sec, &tm);
	time_len = scnprintf(tbuf, len,
		"%04d-%02d-%02d-%02d-%02d-%02d-",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
	return time_len;
}

#define BUF_SIZE 64
static int firmware_dump(struct mhi_device_ctxt *mhi_dev_ctxt,
			 struct bhie_vec_table *rddm_table,
			 char *file_full_path)
{
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	int seg = 0;
	int status = 0;
	char *buf = NULL;
	unsigned int size = 0;

	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "enter\n");
	fs = get_fs();
	set_fs(KERNEL_DS);

	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
		"to create file:%s\n", file_full_path);

	fp = filp_open(file_full_path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR, "create file:%s error\n",
			file_full_path);
		return -EIO;
	}
	pos = 0;
	for (seg = 0; seg < rddm_table->segment_count; seg++) {
		buf = rddm_table->bhie_mem_info[seg].aligned;
		size = rddm_table->bhie_mem_info[seg].size;
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"to write file:%s, mem: 0x%p, size: 0x%x\n",
			file_full_path,
			buf,
			size);
		status = vfs_write(fp, buf, size, &pos);
		if (status < 0) {
			mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
				"write file:%s error\n", file_full_path);
			return status;
		}
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"close file: %s, error\n", file_full_path);
		return status;
	}
	set_fs(fs);
	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "exit\n");
	return status;
}

static int extract_fw_mem_dump(struct mhi_device_ctxt *mhi_dev_ctxt,
			 struct bhie_vec_table *rddm_table,
			 char *file_full_path,
			 unsigned int target_offset,
			 unsigned int target_size)
{
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	int seg = 0;
	int status = 0;
	char *buf = NULL;
	unsigned int size = 0;
	unsigned int handle_size = 0;
	unsigned int gap = 0;
	bool found = false;
	bool complete = false;

	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "enter\n");
	fs = get_fs();
	set_fs(KERNEL_DS);

	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
		"to create file:%s\n", file_full_path);
	fp = filp_open(file_full_path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR, "create file:%s error\n",
			file_full_path);
		return -EIO;
	}
	pos = 0;
	for (seg = 0; seg < rddm_table->segment_count; seg++) {
		buf = rddm_table->bhie_mem_info[seg].aligned;
		size = rddm_table->bhie_mem_info[seg].size;
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"to write file:%s, mem: 0x%p, size: 0x%x\n",
			file_full_path,
			buf,
			size);
		handle_size += size;
		if (found == false) {
			if (handle_size < target_offset) {
				found = false;
				gap = target_offset - handle_size;
				continue;
			} else if (handle_size >= target_offset
			&& handle_size < target_offset + target_size) {
				found = true;
				buf += (size - (handle_size - target_offset));
				size = handle_size - target_offset;
			} else if (handle_size >= target_offset + target_size) {
				buf += gap;
				size = target_size;
				complete = true;
			}
		} else {
			if (handle_size > target_offset + target_size) {
				size -= ( handle_size - (target_offset + target_size));
				complete = true;
			}
		}
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			" mem: 0x%p, size: 0x%x\n",
			buf,
			size);
		status = vfs_write(fp, buf, size, &pos);
		if (status < 0) {
			mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
				"write file:%s error\n", file_full_path);
			return status;
		}
		if (complete == true)
			break;
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"close file: %s, error\n", file_full_path);
		return status;
	}
	set_fs(fs);
	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "exit\n");
	return status;
}

struct paging_header_t {
	u64 version;   /* dump version */
	u64 seg_num;   /* paging seg num */
};
static struct paging_header_t paging_header;

/*paging dump 1 seg for header, save version, seg_num, each seg address, size*/
static char paging_dump_header[512];

static int fw_paging_dump(struct mhi_device_ctxt *mhi_dev_ctxt,
			 struct bhie_vec_table *fw_table,
			 char *file_full_path)
{
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	int seg = 0;
	int status = 0;
	char *buf = NULL;
	unsigned int size = 0;

	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "enter\n");
	fs = get_fs();
	set_fs(KERNEL_DS);

	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
		"to create file:%s\n", file_full_path);

	fp = filp_open(file_full_path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR, "create file:%s error\n",
			file_full_path);
		return -EIO;
	}

	pos = 0;
	paging_header.version = 0;
	paging_header.seg_num = fw_table->segment_count-1;
	memcpy(paging_dump_header, &paging_header, sizeof(paging_header));
	buf = fw_table->bhie_mem_info[paging_header.seg_num].aligned;
	size = fw_table->bhie_mem_info[paging_header.seg_num].size;
	memcpy(paging_dump_header+sizeof(paging_header), buf, size);
	status = vfs_write(fp,
			paging_dump_header,
			sizeof(paging_dump_header),
			&pos);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"write file:%s error\n", file_full_path);
		return status;
	}
	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"to write file:%s, mem: 0x%p, size: 0x%x\n",
			file_full_path,
			paging_dump_header,
			(unsigned int)sizeof(paging_dump_header));
	for (seg = 0; seg < fw_table->segment_count-1; seg++) {
		buf = fw_table->bhie_mem_info[seg].aligned;
		size = fw_table->bhie_mem_info[seg].size;
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"to write file:%s, mem: 0x%p, size: 0x%x\n",
			file_full_path,
			buf,
			size);
		status = vfs_write(fp, buf, size, &pos);
		if (status < 0) {
			mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
				"write file:%s error\n", file_full_path);
			return status;
		}
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"close file: %s, error\n", file_full_path);
		return status;
	}
	set_fs(fs);
	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "exit\n");
	return status;
}


int fw_remote_mem_dump(struct mhi_device_ctxt *mhi_dev_ctxt,
		       struct fw_remote_mem *fw_mem,
		       char *file_full_path)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
	int status = 0;

	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "enter\n");
	fp = filp_open(file_full_path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"create file:%s error\n",
			file_full_path);
		return -EIO;
	}
	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
		"to write file:%s, mem: 0x%p, size: 0x%x\n",
		file_full_path,
		fw_mem->vaddr,
		(unsigned int)(fw_mem->size));
	status = vfs_write(fp,
			   (const char __user *)(fw_mem->vaddr),
			   fw_mem->size,
			   &pos);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"write file:%s error\n",
			file_full_path);
		return status;
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
			"close file: %s, error\n",
			file_full_path);
		return status;
	}
	set_fs(fs);
	mhi_log(mhi_dev_ctxt, MHI_MSG_INFO, "exit\n");
	return status;

}

#define MAX_RAMDUMP_TABLE_SIZE  6

typedef struct
{
	uint64_t base_address;
	uint64_t actual_phys_address;
	uint64_t size;
	char description[20];
	char file_name[20];
}ramdump_entry;

typedef struct
{
	uint32_t version;
	uint32_t header_size;
	ramdump_entry ramdump_table[MAX_RAMDUMP_TABLE_SIZE];
}ramdump_header_t;

void dump_fw_to_file(struct mhi_device_ctxt *mhi_dev_ctxt)
{
	int ret = 0;
	struct bhi_ctxt_t *bhi_ctxt = &mhi_dev_ctxt->bhi_ctxt;
	struct bhie_vec_table *rddm_table = &bhi_ctxt->rddm_table;
	struct bhie_vec_table *fw_table =   &bhi_ctxt->fw_table;
	struct fw_remote_mem *fw_mem = &bhi_ctxt->fw_mem;
	char file_full_path[BUF_SIZE];
	char time_buf[24];
	int len, i = 0;
	int len_left = 0;
	char *p = file_full_path;
	ramdump_header_t *head;
	ramdump_entry *entry;
	unsigned int offset = 0;

	len = get_time_of_the_day_in_hr_min_sec(time_buf, sizeof(time_buf));
	len = scnprintf(file_full_path,
			sizeof(file_full_path),
			"/var/crash/%s",
			time_buf);
	p += len;
	len_left =  sizeof(file_full_path)-len;

	len = scnprintf(p, len_left, "paging.bin");
	ret = fw_paging_dump(mhi_dev_ctxt, fw_table, file_full_path);

	len = scnprintf(p, len_left, "remote.bin");
	ret = fw_remote_mem_dump(mhi_dev_ctxt, fw_mem, file_full_path);

	len = scnprintf(p, len_left, "fwsram.bin");
	ret = firmware_dump(mhi_dev_ctxt, rddm_table, file_full_path);

	head = (ramdump_header_t *)(rddm_table->bhie_mem_info[0].aligned);
	offset = sizeof(ramdump_header_t);
	mhi_log(mhi_dev_ctxt, MHI_MSG_ERROR,
		"version:0x%x, size: 0x%x\n", head->version, head->header_size);
	MHI_ASSERT(head->header_size <= rddm_table->bhie_mem_info[0].size,
		"Too small to contain ramdump header\n");
	for (i = 0; i < MAX_RAMDUMP_TABLE_SIZE; i++) {
		entry = &head->ramdump_table[i];
		if (entry->size == 0)
			continue;
		scnprintf(p, len_left, entry->file_name);
		extract_fw_mem_dump(mhi_dev_ctxt,
		 rddm_table, file_full_path, offset, entry->size);
		offset += entry->size;
	}
}

bool is_ramdump_all_zero(struct mhi_device_ctxt *mhi_dev_ctxt)
{
	struct bhi_ctxt_t *bhi_ctxt = &mhi_dev_ctxt->bhi_ctxt;
	struct bhie_vec_table *rddm_table = &bhi_ctxt->rddm_table;
	int seg = 0;
	char *buf = NULL;
	unsigned head_len;

	head_len = sizeof(ramdump_header_t);

	for (seg = 0; seg < rddm_table->segment_count; seg++) {
		buf = rddm_table->bhie_mem_info[seg].aligned;

		if (buf[head_len+1]+buf[head_len+3]+buf[head_len+5]
			+buf[head_len+7]+buf[head_len+9] != 0) {
			return false;
		}
	}
	return true;
}

#define FW_DUMP_INFO_FORMAT_STR \
	"[%s] to write file:none, mem: 0x%p, size: 0x%x\n"
void dump_fw_info_to_kmsg(struct mhi_device_ctxt *mhi_dev_ctxt)
{
	struct bhi_ctxt_t *bhi_ctxt = &mhi_dev_ctxt->bhi_ctxt;
	struct bhie_vec_table *rddm_table = &bhi_ctxt->rddm_table;
	struct bhie_vec_table *fw_table =   &bhi_ctxt->fw_table;
	struct fw_remote_mem *fw_mem = &bhi_ctxt->fw_mem;
	int seg = 0;
	char *buf = NULL;
	unsigned int size = 0;

	/* firmware_dump */
	for (seg = 0; seg < rddm_table->segment_count; seg++) {
		buf = rddm_table->bhie_mem_info[seg].aligned;
		size = rddm_table->bhie_mem_info[seg].size;
		pr_alert(FW_DUMP_INFO_FORMAT_STR, "firmware_dump", buf, size);
	}

	/* fw_paging_dump */
	paging_header.version = 0;
	paging_header.seg_num = fw_table->segment_count-1;
	memcpy(paging_dump_header, &paging_header, sizeof(paging_header));
	buf = fw_table->bhie_mem_info[paging_header.seg_num].aligned;
	size = fw_table->bhie_mem_info[paging_header.seg_num].size;
	memcpy(paging_dump_header+sizeof(paging_header), buf, size);

	pr_alert(FW_DUMP_INFO_FORMAT_STR, "fw_paging_dump", paging_dump_header,
			(unsigned int)sizeof(paging_dump_header));
	for (seg = 0; seg < fw_table->segment_count-1; seg++) {
		buf = fw_table->bhie_mem_info[seg].aligned;
		size = fw_table->bhie_mem_info[seg].size;
		pr_alert(FW_DUMP_INFO_FORMAT_STR, "fw_paging_dump", buf, size);
	}

	/* fw_remote_mem_dump */
	pr_alert(FW_DUMP_INFO_FORMAT_STR, "fw_remote_mem_dump",
		fw_mem->vaddr, (unsigned int)(fw_mem->size));
}

void mhi_set_fw_remote_mem(struct mhi_device *mhi_device,
			   void *vaddr,
			   size_t size)
{
	struct mhi_device_ctxt *mhi_dev_ctxt = mhi_device->mhi_dev_ctxt;
	struct bhi_ctxt_t *bhi_ctxt = &mhi_dev_ctxt->bhi_ctxt;

	bhi_ctxt->fw_mem.vaddr = vaddr;
	bhi_ctxt->fw_mem.size = size;
}
cnss_export_symbol(mhi_set_fw_remote_mem);


