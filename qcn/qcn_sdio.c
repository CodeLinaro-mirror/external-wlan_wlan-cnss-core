/* Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/pm_runtime.h>
#include "qcn_sdio.h"
#ifdef CONFIG_WLAN_CNSS_CORE
#include "unified_wlan_cnsscore.h"
#endif

#include "cnss2/main.h"
#include "cnss2/debug.h"
#include "oob_wake.h"

static bool tx_dump;
module_param(tx_dump, bool, S_IRUGO | S_IWUSR | S_IWGRP);

static bool rx_dump;
module_param(rx_dump, bool, S_IRUGO | S_IWUSR | S_IWGRP);

static int dump_len = 32;
module_param(dump_len, int, S_IRUGO | S_IWUSR | S_IWGRP);

static bool retune;
module_param(retune, bool, S_IRUGO | S_IWUSR | S_IWGRP);

/**
 * driver_state :
 *	QCN_SDIO_SW_RESET = 0,
 *	QCN_SDIO_SW_PBL,
 *	QCN_SDIO_SW_SBL,
 *	QCN_SDIO_SW_RDDM,
 *	QCN_SDIO_SW_MROM,
*/
static int driver_state;
module_param(driver_state, int, S_IRUGO | S_IRUSR | S_IRGRP);

/*
 * ssr_enable :
 *	Gate for qcn_daemon.sh's automatic SSR recovery (rmmod/fw-dl/insmod).
 *	Off by default; set via insmod ssr_enable=1 or
 *	/sys/module/wlan_cnss_core_sdio/parameters/ssr_enable at runtime.
 */
static bool ssr_enable;
module_param(ssr_enable, bool, S_IRUGO | S_IWUSR | S_IWGRP);

static bool FW_RDDM = false;
/* Maximum duration the system can remain in RDDM state */
#define QCN_SDIO_SW_RDDM_TIMEOUT_MS (5000)
static DECLARE_COMPLETION(rddm_completion);

static atomic_t rpm_in_own_transition = ATOMIC_INIT(0);
static atomic_t rpm_suspended = ATOMIC_INIT(0);

int qcn_sw_mode_change(enum qcn_sdio_sw_mode mode);
int qcn_channel_change(enum qcn_sdio_sw_mode mode);
int reset_thread(void *data);
int switch_to_rddm_thread(void *data);
static void qcn_set_host_clock(unsigned int hz);
void qcn_set_rpm_suspended(int val);
int qcn_get_rpm_suspended(void);

static struct mmc_host *current_host;

#define IS_TX_INVALID(cid, dir)	\
	(cid != QCN_SDIO_CH_0 && FW_RDDM && dir == SDIO_AL_TX)

#define HEX_DUMP(mode, buf, len)				\
	print_hex_dump(KERN_ERR, mode, 2, 32, 4, buf,		\
			dump_len > len ? len : dump_len, 0)

#define IS_WLAN_CH(cid)	\
	(cid == QCN_SDIO_CH_1 || cid == QCN_SDIO_CH_2)

#define HTC_FRAME_HDR 8
#define MAX_HTC_HDR_RECORD 10
static u64 HTC_HDR_HISTORY[MAX_HTC_HDR_RECORD] = {0};
static u8 htc_hdr_index = 0;

#ifdef CONFIG_QCN_SDIO_OOB_IRQ
/**
 * enum qcn_sdio_irq_mode - current owner of the SDIO status path
 * @QCN_SDIO_IRQ_NONE: unowned, before probe and after teardown.
 * @QCN_SDIO_IRQ_POLLING: register polling from the poll work.
 * @QCN_SDIO_IRQ_OOB: GPIO out-of-band interrupt.
 */
enum qcn_sdio_irq_mode {
	QCN_SDIO_IRQ_NONE = 0,
	QCN_SDIO_IRQ_POLLING,
	QCN_SDIO_IRQ_OOB,
};

/* Gap the status-path poll work sleeps between register reads, so the resulting
 * period is this plus the time one poll iteration takes. usleep_range() keeps it
 * at sub-jiffy resolution. 0 polls back-to-back; values below ~10us are
 * dominated by the wakeup overhead itself.
 */
static uint oob_poll_interval_us = 500;
module_param(oob_poll_interval_us, uint, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(oob_poll_interval_us,
		 "Gap between status-path polls in us (default 500, 0 = back-to-back)");

#ifndef QCN_SDIO_OOB_GPIO_NUM_DEFAULT
#define QCN_SDIO_OOB_GPIO_NUM_DEFAULT 43
#endif

static uint oob_irq_gpio = QCN_SDIO_OOB_GPIO_NUM_DEFAULT;
module_param(oob_irq_gpio, uint, S_IRUGO);
#endif

struct qcn_sdio {
	enum qcn_sdio_sw_mode curr_sw_mode;
	struct sdio_func *func;
	const struct sdio_device_id *id;
	struct qcn_sdio_ch_info *ch[QCN_SDIO_CH_MAX];
	atomic_t ch_status[QCN_SDIO_CH_MAX];
	spinlock_t lock_free_q;
	spinlock_t lock_wait_q;
	struct mutex lock_cmpl_q;
	u32 rx_addr_base;
	u32 tx_addr_base;
	u8 rx_cnum_base;
	u8 tx_cnum_base;
	struct qcn_sdio_rw_info rw_req_info[QCN_SDIO_RW_REQ_MAX];
	struct list_head rw_free_q;
	struct list_head rw_wait_q;
	struct list_head rw_cmpl_q;
	atomic_t free_list_count;
	atomic_t wait_list_count;
	struct workqueue_struct *qcn_sdio_wq;
	struct workqueue_struct *qcn_sdio_cmpl_wq;
	struct work_struct sdio_rw_w;
	struct work_struct sdio_cmpl_w;
	struct dentry *dbg_dentry;
#ifdef CONFIG_LPM
	bool low_power_disabled;
	atomic_t suspended;
#endif
	int tx_bundle_buf_size;
#ifdef CONFIG_QCN_SDIO_OOB_IRQ
		struct workqueue_struct *oob_poll_wq;
		struct work_struct oob_poll_work;
		enum qcn_sdio_irq_mode irq_mode;
		struct mutex oob_lock;
		bool oob_irq_enabled;
		int oob_irq;
		bool oob_ready_seen;
		bool poll_stopping;
#endif
};

static struct qcn_sdio *sdio_ctxt;
struct completion client_probe_complete;
static bool client_probe_complete_flag;
static struct mutex lock;
static struct list_head cinfo_head;
static atomic_t status;
static atomic_t xport_status;
static spinlock_t async_lock;
static struct task_struct *reset_task;
static struct task_struct *switch_rddm_task;

#ifndef CONFIG_NAPIER_X86
static int qcn_create_sysfs(struct device *dev);
#endif

char *envp[QCN_SDIO_SW_MAX] = {
	[QCN_SDIO_SW_RESET] = "WLAN_MODE=QCN_SDIO_SW_RESET",
	[QCN_SDIO_SW_PBL] = "WLAN_MODE=QCN_SDIO_SW_PBL",
	[QCN_SDIO_SW_SBL] = "WLAN_MODE=QCN_SDIO_SW_SBL",
	[QCN_SDIO_SW_RDDM] = "WLAN_MODE=QCN_SDIO_SW_RDDM",
	[QCN_SDIO_SW_MROM] = "WLAN_MODE=QCN_SDIO_SW_MROM",
};

#if (QCN_SDIO_META_VER_0)
#define	META_INFO(event, data)						  \
	((u32)((u32)data << QCN_SDIO_HMETA_DATA_SHFT) |			  \
	(u32)(((u32)event << QCN_SDIO_HMETA_EVENT_SHFT) &		  \
	QCN_SDIO_HMETA_EVENT_BMSK) | (u32)(((u32)(sdio_ctxt->curr_sw_mode)\
	<< QCN_SDIO_HMETA_SW_SHFT) & QCN_SDIO_HMETA_SW_BMSK) |		  \
	(u32)(QCN_SDIO_HMETA_FMT_VER & QCN_SDIO_HMETA_VER_BMSK))
#elif (QCN_SDIO_META_VER_1)
#define	META_INFO(event, data)						  \
	((u32)(((u32)event << QCN_SDIO_HMETA_EVENT_SHFT) &		  \
	QCN_SDIO_HMETA_EVENT_BMSK) | (u32)(((u32)data <<		  \
	QCN_SDIO_HMETA_DATA_SHFT) & QCN_SDIO_HMETA_DATA_BMSK))
#endif

#define	SDIO_RW_OFFSET		31
#define	SDIO_RW_MASK		1
#define	SDIO_FUNCTION_OFFSET	28
#define	SDIO_FUNCTION_MASK	7
#define	SDIO_MODE_OFFSET	27
#define	SDIO_MODE_MASK		1
#define	SDIO_OPCODE_OFFSET	26
#define	SDIO_OPCODE_MASK	1
#define	SDIO_ADDRESS_OFFSET	9
#define	SDIO_ADDRESS_MASK	0x1FFFF
#define	SDIO_RAW_OFFSET		27
#define	SDIO_RAW_MASK		1
#define	SDIO_STUFF_OFFSET1	26
#define	SDIO_STUFF_OFFSET2	8
#define	SDIO_STUFF_MASK		1
#define	SDIO_BLOCKSZ_MASK	0x1FF
#define	SDIO_DATA_MASK		0xFF


void qcn_set_rpm_suspended(int val)
{
	atomic_set(&rpm_suspended, val);
}

int qcn_get_rpm_suspended(void)
{
	return atomic_read(&rpm_suspended);
}

static inline
void qcn_sdio_set_cmd53_arg(u32 *arg, u8 rw, u8 func, u8 mode, u8 opcode,
							u32 addr, u16 blksz)
{
	*arg = (((rw & SDIO_RW_MASK) << SDIO_RW_OFFSET) |
		((func & SDIO_FUNCTION_MASK) << SDIO_FUNCTION_OFFSET) |
		((mode & SDIO_MODE_MASK) << SDIO_MODE_OFFSET) |
		((opcode & SDIO_OPCODE_MASK) << SDIO_OPCODE_OFFSET) |
		((addr & SDIO_ADDRESS_MASK) << SDIO_ADDRESS_OFFSET) |
		(blksz & SDIO_BLOCKSZ_MASK));
}

static inline
void qcn_sdio_set_cmd52_arg(u32 *arg, u8 rw, u8 func, u8 raw, u32 addr, u8 val)
{
	*arg = ((rw & SDIO_RW_MASK) << SDIO_RW_OFFSET) |
		((func & SDIO_FUNCTION_MASK) << SDIO_FUNCTION_OFFSET) |
		((raw & SDIO_RAW_MASK) << SDIO_RAW_OFFSET) |
		(SDIO_STUFF_MASK << SDIO_STUFF_OFFSET1) |
		((addr & SDIO_ADDRESS_MASK) << SDIO_ADDRESS_OFFSET) |
		(SDIO_STUFF_MASK << SDIO_STUFF_OFFSET2) |
		(val & SDIO_DATA_MASK);
}

static void qcn_sdio_free_rw_req(struct qcn_sdio_rw_info *rw_req)
{
	spin_lock_bh(&sdio_ctxt->lock_free_q);
	list_add_tail(&rw_req->list, &sdio_ctxt->rw_free_q);
	atomic_inc(&sdio_ctxt->free_list_count);
	atomic_dec(&sdio_ctxt->wait_list_count);
	spin_unlock_bh(&sdio_ctxt->lock_free_q);
}

static void qcn_sdio_purge_rw_buff(void)
{
	struct qcn_sdio_rw_info *rw_req = NULL;
	struct sdio_al_xfer_result result ;
	struct sdio_al_channel_handle *ch_handle = NULL;

	spin_lock_bh(&sdio_ctxt->lock_wait_q);
	while (!list_empty(&sdio_ctxt->rw_wait_q)) {
		rw_req = list_first_entry(&sdio_ctxt->rw_wait_q,
						struct qcn_sdio_rw_info, list);
		list_del(&rw_req->list);
		spin_unlock_bh(&sdio_ctxt->lock_wait_q);

		ch_handle = &sdio_ctxt->ch[rw_req->cid]->ch_handle;
		result.xfer_status = -EINVAL;
		result.buf_addr = rw_req->buf;
		result.xfer_len = rw_req->len;
		if (rw_req->dir)
			sdio_ctxt->ch[rw_req->cid]->ch_data.dl_xfer_cb(
					ch_handle, &result, rw_req->ctxt);
		else
			sdio_ctxt->ch[rw_req->cid]->ch_data.ul_xfer_cb(
					ch_handle, &result, rw_req->ctxt);

		qcn_sdio_free_rw_req(rw_req);
		spin_lock_bh(&sdio_ctxt->lock_wait_q);
	}
	spin_unlock_bh(&sdio_ctxt->lock_wait_q);
	/* wait for sdio complete work to finish */
	flush_work(&sdio_ctxt->sdio_cmpl_w);
}

static void dump_htc_hdr_history(void)
{
	int i;

	for (i = htc_hdr_index; i < MAX_HTC_HDR_RECORD; i++)
		HEX_DUMP("HTC_HDR history: ", &HTC_HDR_HISTORY[i], HTC_FRAME_HDR);
	for (i = 0; i < htc_hdr_index; i++)
		HEX_DUMP("HTC_HDR history: ", &HTC_HDR_HISTORY[i], HTC_FRAME_HDR);
}

void qcn_sdio_client_probe_complete(int id)
{
	complete(&client_probe_complete);
}
EXPORT_SYMBOL(qcn_sdio_client_probe_complete);

static struct qcn_sdio_rw_info *qcn_sdio_alloc_rw_req(void)
{
	struct qcn_sdio_rw_info *rw_req = NULL;

	spin_lock_bh(&sdio_ctxt->lock_free_q);
	if (list_empty(&sdio_ctxt->rw_free_q)) {
		spin_unlock_bh(&sdio_ctxt->lock_free_q);
		return rw_req;
	}

	rw_req = list_first_entry(&sdio_ctxt->rw_free_q,
						struct qcn_sdio_rw_info, list);
	list_del(&rw_req->list);
	atomic_dec(&sdio_ctxt->free_list_count);
	spin_unlock_bh(&sdio_ctxt->lock_free_q);

	return rw_req;
}

static void qcn_sdio_add_cmpl_req(struct qcn_sdio_rw_info *rw_req)
{
	mutex_lock(&sdio_ctxt->lock_cmpl_q);
	list_add_tail(&rw_req->list, &sdio_ctxt->rw_cmpl_q);
	mutex_unlock(&sdio_ctxt->lock_cmpl_q);
}

static void qcn_sdio_add_rw_req(struct qcn_sdio_rw_info *rw_req)
{
	spin_lock_bh(&sdio_ctxt->lock_wait_q);
	list_add_tail(&rw_req->list, &sdio_ctxt->rw_wait_q);
	atomic_inc(&sdio_ctxt->wait_list_count);
	spin_unlock_bh(&sdio_ctxt->lock_wait_q);
}

/*
 * qcn_enable_async_irq() is unused only when both CONFIG_QCN_SDIO_OOB_IRQ
 * and CONFIG_NAPIER_X86 are defined (both call sites below are then compiled out).
 */
#if !defined(CONFIG_QCN_SDIO_OOB_IRQ) || !defined(CONFIG_NAPIER_X86)
static int qcn_enable_async_irq(bool enable)
{
	unsigned int num = 0;
	int ret = 0;
	u32 data = 0;

	num = sdio_ctxt->func->num;
	sdio_claim_host(sdio_ctxt->func);
	sdio_ctxt->func->num = 0;
	data = sdio_readb(sdio_ctxt->func, SDIO_CCCR_INTERRUPT_EXTENSION, NULL);
	if (enable)
		data |= SDIO_ENABLE_ASYNC_INTR;
	else
		data &= ~SDIO_ENABLE_ASYNC_INTR;
	sdio_writeb(sdio_ctxt->func, data, SDIO_CCCR_INTERRUPT_EXTENSION, &ret);
	sdio_ctxt->func->num = num;
	sdio_release_host(sdio_ctxt->func);

	return ret;
}
#endif

static int qcn_send_io_abort(void)
{
	unsigned int num = 0;
	int ret = 0;

	num = sdio_ctxt->func->num;
	sdio_claim_host(sdio_ctxt->func);
	sdio_ctxt->func->num = 0;
	sdio_writeb(sdio_ctxt->func, 0x1, SDIO_CCCR_ABORT, &ret);
	sdio_ctxt->func->num = num;
	sdio_release_host(sdio_ctxt->func);

	return ret;
}

/**
 * qcn_sdio_rpm_get() - Resume the SDIO func device before accessing it
 *
 * Wraps pm_runtime_get_sync() so every SDIO access site shares the same
 * error-logging behavior. The usage count is incremented even on failure,
 * so callers must always pair this with qcn_sdio_rpm_put().
 *
 * Runtime PM for this device is enabled by HIF (hif_rtpm_init(), via
 * hif_rtpm_start()), not by this driver, and that only happens once WLAN
 * bring-up reaches that point (and may never happen at all, e.g. ini
 * "gRuntimePM"=0 or single-MSI mode). Until/unless it does,
 * pm_runtime_get_sync() returns -EACCES here, which is expected and not
 * logged; any other negative return is unexpected and logged.
 */
static void qcn_sdio_rpm_get(void)
{
	int ret = 0;

	if (atomic_read(&rpm_in_own_transition)) {
		pm_runtime_get_noresume(&sdio_ctxt->func->dev);
	} else {
		ret = pm_runtime_get_sync(&sdio_ctxt->func->dev);
	}

	if (ret < 0 && ret != -EACCES)
		pr_warn("%s: pm_runtime_get_sync failed, ret=%d\n",
			__func__, ret);
}

/**
 * qcn_sdio_rpm_put() - Allow the SDIO func device to autosuspend
 */
static void qcn_sdio_rpm_put(void)
{
	int ret;
	if (atomic_read(&rpm_in_own_transition)) {
		pm_runtime_put_noidle(&sdio_ctxt->func->dev);
	} else {
		pm_runtime_mark_last_busy(&sdio_ctxt->func->dev);
		ret = pm_runtime_put_autosuspend(&sdio_ctxt->func->dev);
	}
}


static int qcn_send_meta_info(u8 event, u32 data)
{
	int ret = 0;
	u32 i = 0;
	u32 value = 0;
	u8 temp = 0;

	value =	META_INFO(event, data);

	qcn_sdio_rpm_get();
	sdio_claim_host(sdio_ctxt->func);
	if (sdio_ctxt->curr_sw_mode < QCN_SDIO_SW_SBL) {
		for (i = 0; i < 4; i++) {
			temp = (u8)((value >> (i * 8)) & 0x000000FF);
			sdio_writeb(sdio_ctxt->func, temp,
						(SDIO_QCN_HRQ_PUSH + i), &ret);
		}
	} else {
		sdio_writel(sdio_ctxt->func, value, SDIO_QCN_HRQ_PUSH, &ret);
	}

	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();

	return ret;
}

static int qcn_read_crq_info(const u8 *crq)
{
	int ret = 0;
	u32 temp = 0;
	u32 data = 0;
	u32 len = 0;
	u8 cid = 0;

	struct sdio_al_channel_handle *ch_handle = NULL;

	data = get_unaligned_le32(crq);

	if (data & SDIO_QCN_CRQ_PULL_TRANS_MASK) {
		cid = (u8)(data & SDIO_QCN_CRQ_PULL_CH_NUM_MASK);
		cid -= sdio_ctxt->rx_cnum_base;
		len = (data & SDIO_QCN_CRQ_PULL_BLK_CNT_MASK) >>
			SDIO_QCN_CRQ_PULL_BLK_CNT_SHIFT;

		if (data & SDIO_QCN_CRQ_PULL_BLK_MASK)
			len *= sdio_ctxt->func->cur_blksize;
		temp = (data & SDIO_QCN_CRQ_PULL_UD_MASK) >>
						SDIO_QCN_CRQ_PULL_UD_SHIFT;

		if (!sdio_ctxt->ch[cid]) {
			pr_err("Client Id:%d not initialized\n",cid);
			return -EINVAL;
		}
		switch (temp) {
		case QCN_SDIO_CRQ_START:
			sdio_ctxt->ch[cid]->crq_len = len;
			return ret;
		case QCN_SDIO_CRQ_END:
			sdio_ctxt->ch[cid]->crq_len += len;
			break;
		default:
			sdio_ctxt->ch[cid]->crq_len = len;
		}

		ch_handle = &(sdio_ctxt->ch[cid]->ch_handle);
		if (sdio_ctxt->ch[cid]->ch_data.dl_data_avail_cb)
			sdio_ctxt->ch[cid]->ch_data.dl_data_avail_cb(ch_handle,
					sdio_ctxt->ch[cid]->crq_len);
	}

	return ret;
}

static int qcn_sdio_config(struct qcn_sdio_client_info *cinfo)
{
	int ret = 0;
	u32 data = 0;

	qcn_sdio_rpm_get();

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_set_block_size(sdio_ctxt->func,
				  cinfo->cli_handle.block_size);

	if (ret) {
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}

	data = sdio_readb(sdio_ctxt->func, SDIO_QCN_CONFIG, &ret);
	if (ret) {
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}
	data |= SDIO_QCN_CONFIG_QE_MASK;
	sdio_writeb(sdio_ctxt->func, (u8)data, SDIO_QCN_CONFIG, &ret);
	if (ret) {
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}

	data = (SDIO_QCN_IRQ_EN_LOCAL_MASK |
			SDIO_QCN_IRQ_EN_SYS_ERR_MASK |
			SDIO_QCN_IRQ_UNDERFLOW_MASK |
			SDIO_QCN_IRQ_OVERFLOW_MASK |
			SDIO_QCN_IRQ_CH_MISMATCH_MASK |
			SDIO_QCN_IRQ_CRQ_READY_MASK);

	sdio_writeb(sdio_ctxt->func, (u8)data, SDIO_QCN_IRQ_EN, &ret);
	sdio_release_host(sdio_ctxt->func);
	if (ret) {
		pr_err("%s: failed write config\n", __func__);
		goto err;
	}

	sdio_ctxt->rx_addr_base = SDIO_QCN_MC_DMA0_RX_CH0;
	sdio_ctxt->rx_cnum_base	= QCN_SDIO_DMA0_RX_CNUM;
	sdio_ctxt->tx_addr_base = SDIO_QCN_MC_DMA1_TX_CH0;
	sdio_ctxt->tx_cnum_base = QCN_SDIO_DMA1_TX_CNUM;

#if (QCN_SDIO_META_VER_0)
	data = ((cinfo->cli_handle.block_size / 8) - 1);
#elif (QCN_SDIO_META_VER_1)
	data = cinfo->cli_handle.block_size;
#endif
	ret = qcn_send_meta_info(QCN_SDIO_BLK_SZ_HEVENT, data);
err:
	qcn_sdio_rpm_put();
	return ret;
}

#define BUF_SIZE 64
static int save_fw_mem_to_file(void *buff, char *file_name, u32 total_size)
{
	char file_full_path[BUF_SIZE];
	struct file *fp;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	mm_segment_t fs;
#endif
	loff_t pos;
	int status = 0;

	scnprintf(file_full_path,
			sizeof(file_full_path),
			"/var/crash/%s",
			file_name);
	fp = filp_open(file_full_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (IS_ERR(fp)) {
		pr_err("create file:%s error\n",file_full_path);
		return -EIO;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	fs = get_fs();
	set_fs(KERNEL_DS);
#endif
	pos = 0;
	status = kernel_write(fp, buff, total_size, &pos);
	if (status < 0) {
		pr_err("write file:%s error\n", file_full_path);
		return status;
	}

	/* flush write to file */
	vfs_fsync(fp, 0);

	status = filp_close(fp, NULL);
	if (status < 0) {
		pr_err("close file: %s, error\n", file_full_path);
		return status;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(CONFIG_SET_FS))
	set_fs(fs);
#endif
	return status;
}

static int qcn_save_fw_memory_dump(void)
{
	struct cnss_plat_data *plat_priv = cnss_get_plat_priv(NULL);
	char file_name[] = "remote.bin";
	int ret = 0;

	if (plat_priv->fw_mem[FW_MEM_SEG_INDEX_0].va) {
		/* save the fw memory to file system */
		ret = save_fw_mem_to_file(plat_priv->fw_mem[FW_MEM_SEG_INDEX_0].va,
				file_name, plat_priv->fw_mem[FW_MEM_SEG_INDEX_0].size);
		if (ret < 0) {
			pr_err("Fail to save fw mem data: %d\n", ret);
		}
	}

	return ret;
}

int qcn_channel_change(enum qcn_sdio_sw_mode mode)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	struct qcn_sdio_ch_info *chinfo = NULL;

	mutex_lock(&lock);
	list_for_each_entry(cinfo, &cinfo_head, cli_list) {
		while (!list_empty(&cinfo->ch_head)) {
			chinfo = list_first_entry(&cinfo->ch_head,
				      struct qcn_sdio_ch_info, ch_list);
			sdio_al_deregister_channel(&chinfo->ch_handle);
		}
		cinfo->cli_handle.func = NULL;


		if (cinfo->is_probed) {
			cinfo->cli_data.remove(&cinfo->cli_handle);
			cinfo->is_probed = 0;
		}

		if (mode > QCN_SDIO_SW_MROM)
			continue;

		if (cinfo->cli_handle.id != QCN_SDIO_CLI_ID_TTY &&
		    mode != QCN_SDIO_SW_MROM) {
			/*
			 * At PBL the new sdio_func is available after card reset.
			 * Update func for WLAN so pld_sdio_remove receives the
			 * correct new dev when rmmod is triggered at PBL.
			 * Other clients (non-TTY, non-WLAN) skip as before.
			 */
			if (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_WLAN)
				cinfo->cli_handle.func = sdio_ctxt->func;
			else
				continue;
		}

		qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT,
				(u32)(mode | QCN_SDIO_MAJOR_VER
				| QCN_SDIO_MINOR_VER));
		if (mode == QCN_SDIO_SW_MROM)
			cinfo->cli_handle.block_size = QCN_SDIO_MROM_BLK_SZ;
		else
			cinfo->cli_handle.block_size = QCN_SDIO_TTY_BLK_SZ;
		cinfo->cli_handle.func = sdio_ctxt->func;
		qcn_sdio_config(cinfo);
		cinfo->is_probed = !cinfo->cli_data.probe(
					&cinfo->cli_handle);
		qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);
	}
	mutex_unlock(&lock);

	driver_state = mode;
	sdio_ctxt->curr_sw_mode = mode;
	return 0;
}

#ifdef CONFIG_QCN_SDIO_OOB_IRQ
static int qcn_sdio_read_oob_ready(bool *ready)
{
	int ret;
	u32 value;

	qcn_sdio_rpm_get();
	sdio_claim_host(sdio_ctxt->func);
	value = sdio_readl(sdio_ctxt->func, SDIO_QCN_CLIENT_TRANS_REG0, &ret);
	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();

	if (ret)
		return ret;

	*ready = value & BIT(0);

	return 0;
}

static int qcn_sdio_write_oob_en(bool enable)
{
	int ret;
	u8 config;

	qcn_sdio_rpm_get();
	sdio_claim_host(sdio_ctxt->func);
	config = sdio_readb(sdio_ctxt->func, SDIO_QCN_CONFIG, &ret);
	if (ret) {
		sdio_release_host(sdio_ctxt->func);
		qcn_sdio_rpm_put();
		return ret;
	}

	if (enable)
		config |= SDIO_QCN_CONFIG_OOB_MASK;
	else
		config &= ~SDIO_QCN_CONFIG_OOB_MASK;

	sdio_writeb(sdio_ctxt->func, config, SDIO_QCN_CONFIG, &ret);
	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();

	return ret;
}

static const char *qcn_sdio_irq_mode_str(enum qcn_sdio_irq_mode mode)
{
	switch (mode) {
	case QCN_SDIO_IRQ_POLLING:
		return "polling";
	case QCN_SDIO_IRQ_OOB:
		return "oob_irq";
	case QCN_SDIO_IRQ_NONE:
	default:
		return "none";
	}
}

/**
 * qcn_sdio_enter_polling_locked() - hand the status path to the poll worker
 *
 * Undoes the OOB setup if it was active, then arms the poll work. Also serves
 * the initial NONE -> POLLING arm from qcn_sdio_setup_irq_path(), where there
 * is nothing to undo. Caller holds oob_lock.
 *
 * Return: 0 always.
 */
static int qcn_sdio_enter_polling_locked(void)
{
	int ret;

	if (sdio_ctxt->irq_mode == QCN_SDIO_IRQ_OOB) {
		ret = qcn_sdio_write_oob_en(false);
		if (ret)
			pr_err("%s: failed to clear OOB_EN, ret=%d\n",
			       __func__, ret);
		if (sdio_ctxt->oob_irq_enabled) {
			disable_irq(sdio_ctxt->oob_irq);
			sdio_ctxt->oob_irq_enabled = false;
		}
		sdio_ctxt->irq_mode = QCN_SDIO_IRQ_POLLING;
	}

	sdio_ctxt->oob_ready_seen = false;

	if (!sdio_ctxt->poll_stopping)
		queue_work(sdio_ctxt->oob_poll_wq, &sdio_ctxt->oob_poll_work);

	return 0;
}

/**
 * qcn_sdio_enter_oob_locked() - hand the status path to the GPIO OOB IRQ
 *
 * Two-stage FW handshake: the device advertises readiness, then the host sets
 * OOB_EN. Stage 1 is latched in oob_ready_seen so a retry only redoes the stage
 * that failed. Caller holds oob_lock.
 *
 * Only ever reached from the poll work, which stops re-arming itself once
 * irq_mode is OOB, so there is no pending poll to cancel here.
 *
 * Return: 0 on success, -EAGAIN while the device is not ready yet, -ENODEV if
 * no OOB IRQ was requested, or a negative errno from the register access.
 */
static int qcn_sdio_enter_oob_locked(void)
{
	bool ready;
	int ret;

	if (sdio_ctxt->irq_mode == QCN_SDIO_IRQ_OOB)
		return 0;

	if (sdio_ctxt->oob_irq <= 0)
		return -ENODEV;

	if (!sdio_ctxt->oob_ready_seen) {
		ret = qcn_sdio_read_oob_ready(&ready);
		if (ret)
			return ret;
		if (!ready)
			return -EAGAIN;

		sdio_ctxt->oob_ready_seen = true;
		pr_info("SDIO_QCN_CLIENT_TRANS_REG0(BIT0)=0x1, target OOB ready\n");
	}

	ret = qcn_sdio_write_oob_en(true);
	if (ret) {
		pr_err_ratelimited("Failed to set OOB_EN, ret=%d\n", ret);
		return ret;
	}

	enable_irq(sdio_ctxt->oob_irq);
	sdio_ctxt->oob_irq_enabled = true;
	sdio_ctxt->irq_mode = QCN_SDIO_IRQ_OOB;

	return 0;
}

static int qcn_sdio_set_irq_mode(enum qcn_sdio_irq_mode mode)
{
	enum qcn_sdio_irq_mode prev;
	int ret = 0;

	mutex_lock(&sdio_ctxt->oob_lock);
	prev = sdio_ctxt->irq_mode;

	switch (mode) {
	case QCN_SDIO_IRQ_POLLING:
		ret = qcn_sdio_enter_polling_locked();
		break;
	case QCN_SDIO_IRQ_OOB:
		ret = qcn_sdio_enter_oob_locked();
		break;
	default:
		mutex_unlock(&sdio_ctxt->oob_lock);
		return -EINVAL;
	}

	if (sdio_ctxt->irq_mode != prev)
		pr_info("OOB irq mode: %s -> %s\n", qcn_sdio_irq_mode_str(prev),
			qcn_sdio_irq_mode_str(sdio_ctxt->irq_mode));

	mutex_unlock(&sdio_ctxt->oob_lock);

	return ret;
}
#endif

int switch_to_rddm_thread(void *data)
{
	enum qcn_sdio_sw_mode mode = QCN_SDIO_SW_RDDM;
	char *uevent[2];

	/* wait for sdio rw work to finish */
	flush_work(&sdio_ctxt->sdio_rw_w);
	/* wait for sdio complete work to finish */
	flush_work(&sdio_ctxt->sdio_cmpl_w);
	qcn_save_fw_memory_dump();
	dump_htc_hdr_history();
	qcn_channel_change(mode);
	reinit_completion(&rddm_completion);
	uevent[0] = envp[QCN_SDIO_SW_RDDM];
	uevent[1] = NULL;
	kobject_uevent_env(&sdio_ctxt->func->dev.kobj, KOBJ_CHANGE, uevent);
	//wait rddm exit
	if (!wait_for_completion_timeout(&rddm_completion,
					 msecs_to_jiffies(QCN_SDIO_SW_RDDM_TIMEOUT_MS)))
		pr_err("[%s:%d] RDDM completion timeout!\n",
		       __func__, __LINE__);
	// if rddm mode not change, the driver should reset sdio
	if(sdio_ctxt->curr_sw_mode == QCN_SDIO_SW_RDDM) {
		if(!qcn_sdio_card_state(false))
			qcn_sdio_card_state(true);
	}
	return 0;
}

bool qcn_rddm_is_processing(void)
{
	return FW_RDDM;
}
EXPORT_SYMBOL(qcn_rddm_is_processing);

static int qcn_sdio_rddm_handler(void)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	int ret = -1;

	list_for_each_entry(cinfo, &cinfo_head, cli_list) {
		if (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_WLAN &&
		    cinfo->is_probed) {
			cnss_sdio_notify_fw_down(&cinfo->cli_handle);
			break;
		}
	}

	switch_rddm_task = kthread_run(switch_to_rddm_thread, NULL,
				       "qcn_sdio_rddm_handler");
	if (IS_ERR(switch_rddm_task)) {
		pr_err("Failed to run switch_to_rddm_thread thread\n");
		return ret;
	}

	return 0;
}

int qcn_sw_mode_change(enum qcn_sdio_sw_mode mode)
{
	int ret = 0;

	if (!(mode) && !(mode < QCN_SDIO_SW_MAX))
		return -EINVAL;

	pr_info("%s: curr_sw_mode %d new mode %d\n",
		__func__, sdio_ctxt->curr_sw_mode, mode);
	if (sdio_ctxt->curr_sw_mode == mode)
		return 0;

	if (mode == QCN_SDIO_SW_RDDM) {
		FW_RDDM = true;
		if (current_host && current_host->ios.clock &&
		    current_host->ios.clock > 100000000) {
			pr_info("Try to reduce the frequency\n");
			qcn_set_host_clock(50000000);
		}
	} else {
		FW_RDDM = false;
	}

	if ((sdio_ctxt->curr_sw_mode == QCN_SDIO_SW_PBL) &&
						(mode == QCN_SDIO_SW_SBL)) {
		sdio_ctxt->curr_sw_mode = QCN_SDIO_SW_SBL;
		qcn_send_meta_info(QCN_SDIO_BLK_SZ_HEVENT,
						sdio_ctxt->func->cur_blksize);
		qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);
		return 0;
	}

	switch (sdio_ctxt->curr_sw_mode) {
	case QCN_SDIO_SW_PBL:
	case QCN_SDIO_SW_SBL:
	case QCN_SDIO_SW_RDDM:
		qcn_channel_change(mode);
		break;
	case QCN_SDIO_SW_RESET:
		/* Reached after RDDM */
		complete(&rddm_completion);
		if (!client_probe_complete_flag) {
			client_probe_complete_flag = true;
			ret = wait_for_completion_timeout(&client_probe_complete,
								msecs_to_jiffies(3000));
			if (!ret)
				pr_err("Timeout waiting for clients\n");
		}
		qcn_channel_change(mode);
		break;
	case QCN_SDIO_SW_MROM:
		qcn_sdio_rddm_handler();
		break;
	default:
		pr_err("Invalid mode\n");
	}

	if (mode != QCN_SDIO_SW_RDDM) {
		driver_state = mode;
		sdio_ctxt->curr_sw_mode = mode;
	}

	return 0;
}

static int qcn_read_meta_info(void)
{
	int ret = 0;
	u32 i = 0;
	u32 data = 0;
	u32 temp = 0;

	qcn_sdio_rpm_get();

	sdio_claim_host(sdio_ctxt->func);

	if (sdio_ctxt->curr_sw_mode < QCN_SDIO_SW_SBL) {
		for (i = 0; i < 4; i++) {
			temp = sdio_readb(sdio_ctxt->func,
					(SDIO_QCN_LOCAL_INFO + i), &ret);
			temp = temp << (i * 8);
			data |= temp;
		}
	} else {
		data = sdio_readl(sdio_ctxt->func, SDIO_QCN_LOCAL_INFO,	&ret);
	}

	sdio_writeb(sdio_ctxt->func, (u8)SDIO_QCN_IRQ_CLR_LOCAL_MASK,
		    SDIO_QCN_IRQ_CLR, NULL);

	sdio_release_host(sdio_ctxt->func);

	qcn_sdio_rpm_put();

	if (ret)
		return ret;

	temp = (data & QCN_SDIO_LMETA_EVENT_BMSK) >> QCN_SDIO_LMETA_EVENT_SHFT;
	switch (temp) {
	case QCN_SDIO_SW_MODE_LEVENT:
		temp = (data & QCN_SDIO_LMETA_SW_BMSK) >>
						QCN_SDIO_LMETA_SW_SHFT;
		qcn_sw_mode_change((enum qcn_sdio_sw_mode)temp);
		break;
	default:
		if ((temp >= QCN_SDIO_META_START_CH0) &&
				(temp < QCN_SDIO_META_START_CH1)) {
			if (sdio_ctxt->ch[0] &&
				sdio_ctxt->ch[0]->ch_data.dl_meta_data_cb)
				sdio_ctxt->ch[0]->ch_data.dl_meta_data_cb(
					&(sdio_ctxt->ch[0]->ch_handle), data);
		} else if ((temp >= QCN_SDIO_META_START_CH1) &&
			(temp < QCN_SDIO_META_START_CH2)) {
			if (sdio_ctxt->ch[1] &&
				sdio_ctxt->ch[1]->ch_data.dl_meta_data_cb)
				sdio_ctxt->ch[1]->ch_data.dl_meta_data_cb(
					&(sdio_ctxt->ch[1]->ch_handle), data);
		} else if ((temp >= QCN_SDIO_META_START_CH2) &&
				(temp < QCN_SDIO_META_START_CH3)) {
			if (sdio_ctxt->ch[2] &&
				sdio_ctxt->ch[2]->ch_data.dl_meta_data_cb)
				sdio_ctxt->ch[2]->ch_data.dl_meta_data_cb(
					&(sdio_ctxt->ch[2]->ch_handle), data);
		} else if ((temp >= QCN_SDIO_META_START_CH3) &&
					(temp < QCN_SDIO_META_END)) {
			if (sdio_ctxt->ch[3] &&
				sdio_ctxt->ch[3]->ch_data.dl_meta_data_cb)
				sdio_ctxt->ch[3]->ch_data.dl_meta_data_cb(
					&(sdio_ctxt->ch[3]->ch_handle), data);
		} else {
			ret = -EINVAL;
		}
	}

	return ret;
}

int reset_thread(void *data)
{
	qcn_sdio_purge_rw_buff();
	if(!qcn_sdio_card_state(false))
		qcn_sdio_card_state(true);
	/* Safe to drop reference here; kthread_stop(reset_task) not needed */
	reset_task = NULL;

	return 0;
}

static int qcn_sdio_reset(void)
{
	int ret = -1;

	reset_task = kthread_run(reset_thread, NULL, "qcn_sdio_reset");
	if (IS_ERR(reset_task)) {
		pr_err("Failed to run qcn_sdio_reset thread\n");
		return ret;
	}

	return 0;
}

static void qcn_set_host_clock(unsigned int hz)
{
	if (current_host->ios.clock <= hz)
		return;

	pr_info("%s: %u hz", __func__, hz);
	qcn_sdio_rpm_get();
	sdio_claim_host(sdio_ctxt->func);
	current_host->ios.clock = hz;
	if (current_host->ops->set_ios)
		current_host->ops->set_ios(current_host, &current_host->ios);
	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();
}

static int irq_max_loops = 1;
module_param(irq_max_loops, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(irq_max_loops, "Max IRQ handler loop count (1-16, default MAX_SDIO_IRQ_LOOPS)");

#define IRQ_STATUS_LEN	(SDIO_QCN_CRQ_PULL - SDIO_QCN_IRQ_STATUS + 4)
#define CRQ_OFFSET	(SDIO_QCN_CRQ_PULL - SDIO_QCN_IRQ_STATUS)

static void qcn_sdio_irq_service(void)
{
	u8 buf[IRQ_STATUS_LEN];
	int ret = 0;
	int max_loops = clamp(irq_max_loops, 1, 16);

	qcn_sdio_rpm_get();

	do {
		sdio_claim_host(sdio_ctxt->func);
		ret = sdio_memcpy_fromio(sdio_ctxt->func, buf,
					 SDIO_QCN_IRQ_STATUS, sizeof(buf));
		if (ret) {
			sdio_release_host(sdio_ctxt->func);

			pr_err_ratelimited("%s: IRQ status read error ret = %d\n",
					   __func__, ret);

#ifdef CONFIG_QCN_SDIO_OOB_IRQ
			/* The device drops off the bus by itself at the end of
			 * RDDM, and the OOB line keeps firing because it is
			 * independent of the bus state. Forcing a host reset
			 * here would contend for the host claim with the mmc
			 * detect/remove path and delay re-enumeration past
			 * QCN_SDIO_SW_RDDM_TIMEOUT_MS, so let the normal
			 * remove/rescan flow handle it.
			 */
			if (FW_RDDM)
				return;
#endif

			if (current_host && current_host->ios.clock &&
			    current_host->ios.clock > 100000000) {
				pr_info("Try to reduce the frequency\n");
				qcn_set_host_clock(50000000);
				qcn_sdio_rpm_put();
				return;
			}

			ret = qcn_sdio_reset();
			if (ret)
				pr_err("Failed to run qcn_sdio_reset thread\n");

			qcn_sdio_rpm_put();
			return;
		}
		sdio_release_host(sdio_ctxt->func);

		if (!buf[0])
			break;

		if (buf[0] & SDIO_QCN_IRQ_CRQ_READY_MASK) {
			qcn_read_crq_info(&buf[CRQ_OFFSET]);
		} else if (buf[0] & SDIO_QCN_IRQ_LOCAL_MASK) {
			qcn_read_meta_info();
		} else if (buf[0] & SDIO_QCN_IRQ_EN_SYS_ERR_MASK) {
			sdio_claim_host(sdio_ctxt->func);
			sdio_writeb(sdio_ctxt->func, (u8)SDIO_QCN_IRQ_CLR_SYS_ERR_MASK,
					SDIO_QCN_IRQ_CLR, NULL);
			sdio_release_host(sdio_ctxt->func);
			pr_err("%s: sys_err interrupt triggered\n", __func__);
		} else if (buf[0] & SDIO_QCN_IRQ_EN_UNDERFLOW_MASK) {
			sdio_claim_host(sdio_ctxt->func);
			sdio_writeb(sdio_ctxt->func,
						(u8)SDIO_QCN_IRQ_CLR_UNDERFLOW_MASK,
						SDIO_QCN_IRQ_CLR, NULL);
			sdio_release_host(sdio_ctxt->func);
			pr_err("%s: underflow interrupt triggered\n", __func__);
		} else if (buf[0] & SDIO_QCN_IRQ_EN_OVERFLOW_MASK) {
			sdio_claim_host(sdio_ctxt->func);
			sdio_writeb(sdio_ctxt->func, (u8)SDIO_QCN_IRQ_CLR_OVERFLOW_MASK,
					SDIO_QCN_IRQ_CLR, NULL);
			sdio_release_host(sdio_ctxt->func);
			pr_err("%s: overflow interrupt triggered\n", __func__);
		} else if (buf[0] & SDIO_QCN_IRQ_EN_CH_MISMATCH_MASK) {
			sdio_claim_host(sdio_ctxt->func);
			sdio_writeb(sdio_ctxt->func,
						(u8)SDIO_QCN_IRQ_CLR_CH_MISMATCH_MASK,
						SDIO_QCN_IRQ_CLR, NULL);
			sdio_release_host(sdio_ctxt->func);
			pr_err("%s: channel mismatch interrupt triggered\n", __func__);
		} else {
			pr_err("%s: Unknown interrupt: 0x%02x\n", __func__, buf[0]);
			sdio_claim_host(sdio_ctxt->func);
			sdio_writeb(sdio_ctxt->func, buf[0], SDIO_QCN_IRQ_CLR, NULL);
			sdio_release_host(sdio_ctxt->func);
		}
	} while (--max_loops > 0);

	qcn_sdio_rpm_put();

}

#ifdef CONFIG_QCN_SDIO_OOB_IRQ
/**
 * qcn_sdio_poll_work() - poll the status path and retry the OOB handshake
 * @work: the oob_poll_work work item.
 *
 * Drives the status path before OOB is up, and retries the OOB handshake once
 * the device reaches Mission ROM. Sleeps for one poll gap and re-arms itself
 * until it hands over to OOB mode or teardown sets poll_stopping.
 */
static void qcn_sdio_poll_work(struct work_struct *work)
{
	struct qcn_sdio *ctx = container_of(work, struct qcn_sdio,
					    oob_poll_work);
	uint interval_us = oob_poll_interval_us;

	if (ctx->poll_stopping)
		return;

	qcn_sdio_irq_service();

	if (ctx->curr_sw_mode == QCN_SDIO_SW_MROM)
		qcn_sdio_set_irq_mode(QCN_SDIO_IRQ_OOB);

	if (interval_us)
		usleep_range(interval_us, interval_us + interval_us / 4);

	if (!ctx->poll_stopping && ctx->irq_mode != QCN_SDIO_IRQ_OOB)
		queue_work(ctx->oob_poll_wq, &ctx->oob_poll_work);
}

static irqreturn_t qcn_sdio_oob_irq_thread(int irq, void *data)
{
	struct qcn_sdio *ctx = data;

	if (ctx->irq_mode != QCN_SDIO_IRQ_OOB)
		return IRQ_HANDLED;

	qcn_sdio_irq_service();

	return IRQ_HANDLED;
}
#else
static void qcn_sdio_irq_handler(struct sdio_func *func)
{
	qcn_sdio_irq_service();
}
#endif

static int qcn_sdio_send_buff(u32 cid, void *buff, size_t len)
{
	int ret = 0;
	int i = 0;

	if (cid != QCN_SDIO_CH_0 && FW_RDDM)
		return -EINVAL;

	qcn_sdio_rpm_get();

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_writesb(sdio_ctxt->func,
			(sdio_ctxt->tx_addr_base + (cid * (u32)4)), buff, len);

	if (ret)
		qcn_send_io_abort();

	if (IS_WLAN_CH(cid) && !ret) {
		if (len < sdio_ctxt->tx_bundle_buf_size) {
			memcpy(&HTC_HDR_HISTORY[htc_hdr_index], buff,
			       HTC_FRAME_HDR);
			htc_hdr_index =
				(htc_hdr_index + 1) % MAX_HTC_HDR_RECORD;
		}

		for (i = 0; i < (len / sdio_ctxt->tx_bundle_buf_size); i++) {
			memcpy(&HTC_HDR_HISTORY[htc_hdr_index],
			       buff + (i * sdio_ctxt->tx_bundle_buf_size), HTC_FRAME_HDR);
			htc_hdr_index =
				(htc_hdr_index + 1) % MAX_HTC_HDR_RECORD;
		}
	}

	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();

	return ret;
}

static int qcn_sdio_recv_buff(u32 cid, void *buff, size_t len)
{
	int ret = 0;

	qcn_sdio_rpm_get();

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_readsb(sdio_ctxt->func, buff,
			(sdio_ctxt->rx_addr_base + (cid * (u32)4)), len);

	if (ret)
		qcn_send_io_abort();

	sdio_release_host(sdio_ctxt->func);
	qcn_sdio_rpm_put();

	return ret;
}

static void qcn_sdio_cmpl_work(struct work_struct *work)
{
	struct qcn_sdio_rw_info *rw_req = NULL;
	struct sdio_al_channel_handle *ch_handle = NULL;

	while (1) {
		mutex_lock(&sdio_ctxt->lock_cmpl_q);
		if (list_empty(&sdio_ctxt->rw_cmpl_q)) {
			mutex_unlock(&sdio_ctxt->lock_cmpl_q);
			break;
		}
		rw_req = list_first_entry(&sdio_ctxt->rw_cmpl_q,
						struct qcn_sdio_rw_info, list);
		list_del(&rw_req->list);
		mutex_unlock(&sdio_ctxt->lock_cmpl_q);

		ch_handle = &sdio_ctxt->ch[rw_req->cid]->ch_handle;
		rw_req->result.buf_addr = rw_req->buf;
		rw_req->result.xfer_len = rw_req->len;
		if (rw_req->dir)
			sdio_ctxt->ch[rw_req->cid]->ch_data.dl_xfer_cb(
					ch_handle, &rw_req->result, rw_req->ctxt);
		else
			sdio_ctxt->ch[rw_req->cid]->ch_data.ul_xfer_cb(
					ch_handle, &rw_req->result, rw_req->ctxt);
		qcn_sdio_free_rw_req(rw_req);
	}
}

static void qcn_sdio_rw_work(struct work_struct *work)
{
	int ret = 0;
	struct qcn_sdio_rw_info *rw_req = NULL;

	while (1) {
		spin_lock_bh(&sdio_ctxt->lock_wait_q);
		if (list_empty(&sdio_ctxt->rw_wait_q)) {
			spin_unlock_bh(&sdio_ctxt->lock_wait_q);
			break;
		}
		rw_req = list_first_entry(&sdio_ctxt->rw_wait_q,
						struct qcn_sdio_rw_info, list);
		list_del(&rw_req->list);
		spin_unlock_bh(&sdio_ctxt->lock_wait_q);

		if (rw_req->dir) {
			ret = qcn_sdio_recv_buff(rw_req->cid, rw_req->buf,
								rw_req->len);
			if (rx_dump)
				HEX_DUMP("ASYNC_RECV: ", rw_req->buf,
								rw_req->len);
		} else {
			ret = qcn_sdio_send_buff(rw_req->cid, rw_req->buf,
								rw_req->len);
			if (tx_dump)
				HEX_DUMP("ASYNC_SEND: ", rw_req->buf,
								rw_req->len);
		}

		rw_req->result.xfer_status = ret;
		qcn_sdio_add_cmpl_req(rw_req);
		queue_work(sdio_ctxt->qcn_sdio_cmpl_wq, &sdio_ctxt->sdio_cmpl_w);
	}
}

#ifdef CONFIG_LPM
static int qcn_sdio_lpm_notify_client(enum sdio_al_lpm_event event)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	int ret = 0;

	list_for_each_entry(cinfo, &cinfo_head, cli_list) {
		if (cinfo->is_probed && cinfo->cli_data.lpm_notify_cb)
			ret = cinfo->cli_data.lpm_notify_cb(&cinfo->cli_handle,
							    event);
	}

	return ret;
}

static int qcn_sdio_suspend_bus(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	u8 value = 0;
	int ret = 0;

	sdio_claim_host(func);
	value = sdio_readb(func, SDIO_QCN_LOW_PWR, &ret);
	if (ret) {
		pr_err("low power status read error: %d\n", ret);
		goto out;
	}

	value = value | SDIO_QCN_LOW_PWR_GO_MASK;
	sdio_writeb(func, value, SDIO_QCN_LOW_PWR, &ret);
	if (ret) {
		pr_err("low power status write error: %d\n", ret);
		goto out;
	}

	/* cologne-434: do not poll for SDIO_QCN_LOW_PWR as cologne will
	 * not respond to sdio commands on receiving SDIO_QCN_LOW_PWR_GO
	 */
	msleep(5);

	sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);

out:
	sdio_release_host(func);
	return ret;
}

static int qcn_sdio_suspend(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	int ret = 0, suspended;

	if (sdio_ctxt->low_power_disabled) {
		pr_err("Low power has been disabled\n");
		return 0;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 0, 1);
	if (suspended) {
		pr_info("Already suspended\n");
		return 0;
	}

	dev_info(dev, "Notify client to suspend");
	ret = qcn_sdio_lpm_notify_client(LPM_ENTER);
	if (ret) {
		dev_err(dev, "Client failed to suspend: %d", ret);
		atomic_set(&sdio_ctxt->suspended, 0);
		return ret;
	}

	pr_info("%s: func %d curr_sw_mode=%d\n", __func__,
		func->num, sdio_ctxt->curr_sw_mode);

	ret = qcn_sdio_suspend_bus(dev);

	dev_info(dev, "suspend exit with ret %d", ret);
	return ret;
}

static int qcn_sdio_resume(struct device *dev)
{
	int ret = 0, suspended;

	if (sdio_ctxt->low_power_disabled) {
		pr_err("Low power has been disabled\n");
		return 0;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 1, 0);
	if (!suspended) {
		pr_info("Already resumed\n");
		return 0;
	}

	qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);

	cnss_pr_dbg("Notify client to resume");
	if ((ret = qcn_sdio_lpm_notify_client(LPM_EXIT)) != 0)
		dev_err(dev, "Client failed to resume: %d", ret);

	cnss_pr_dbg("resume exit\n");
	return ret;
}

int cnss_auto_suspend(struct device *dev)
{
	int ret = 0;
	
	ret = qcn_sdio_suspend_bus(dev);
	qcn_set_rpm_suspended(1);

	return ret;
}
EXPORT_SYMBOL(cnss_auto_suspend);

static int qcn_sdio_runtime_suspend(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	int ret = 0, suspended;

	if (sdio_ctxt->low_power_disabled) {
		pr_err("Low power has been disabled\n");
		return 0;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 0, 1);
	if (suspended) {
		pr_info("Already suspended\n");
		return 0;
	}

	cnss_pr_dbg("Notify client to runtime suspend");
	atomic_set(&rpm_in_own_transition, 1);
	ret = qcn_sdio_lpm_notify_client(LPM_RUNTIME_SUSPEND);
	if (ret) {
		dev_err(dev, "Client failed to runtime suspend: %d", ret);
		ret = -EBUSY;
		goto fail;
	}
	atomic_set(&rpm_in_own_transition, 0);

	cnss_pr_dbg("%s: func %d curr_sw_mode=%d\n", __func__,
		func->num, sdio_ctxt->curr_sw_mode);

	qcn_enable_gpio_wakeup_irq();
	cnss_pr_dbg("runtime suspend done with ret %d", ret);
	return ret;
fail:
	atomic_set(&sdio_ctxt->suspended, 0);
	atomic_set(&rpm_in_own_transition, 0);
	return ret;
}

int cnss_auto_resume(struct device *dev)
{
	qcn_set_rpm_suspended(0);
	return 0;
}
EXPORT_SYMBOL(cnss_auto_resume);

/* extern void cnss2_gpio_rtpm_enable_wakeup(void); */
static int qcn_sdio_runtime_resume(struct device *dev)
{
	int ret = 0, suspended;

	if (sdio_ctxt->low_power_disabled) {
		pr_err("Low power has been disabled\n");
		return 0;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 1, 0);
	if (!suspended) {
		pr_info("Already resumed\n");
		return 0;
	}

	atomic_set(&rpm_in_own_transition, 1);
	ret = qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
	}

	if ((ret = qcn_sdio_lpm_notify_client(LPM_RUNTIME_RESUME)) != 0) {
		dev_err(dev, "Client failed to resume: %d", ret);
		ret = -EBUSY;
		goto fail;
	}
	atomic_set(&rpm_in_own_transition, 0);

	cnss_auto_resume(dev);
	cnss_pr_dbg("runtime resume done with ret = %d\n", ret);
	return ret;
fail:
	atomic_set(&sdio_ctxt->suspended, 1);
	atomic_set(&rpm_in_own_transition, 0);
	return ret;
}

static int qcn_sdio_runtime_idle(struct device *dev)
{
	dev_dbg(dev, "Runtime idle\n");

	pm_request_autosuspend(dev);

	return -EBUSY;
}

static int qcn_sdio_lpm_set(struct qcn_sdio *sdio_ctxt, bool enable)
{
	pr_info("%s: %s\n", __func__, enable ? "enable" : "disable");
	sdio_ctxt->low_power_disabled = !enable;
	return 0;
}
#else
static inline int qcn_sdio_suspend(struct device *dev)
{
	return -ENOTSUPP;
}

static inline int qcn_sdio_resume(struct device *dev)
{
	return -ENOTSUPP;
}

static inline int qcn_sdio_runtime_suspend(struct device *dev)
{
	return -ENOTSUPP;
}

static inline int qcn_sdio_runtime_resume(struct device *dev)
{
	return -ENOTSUPP;
}

static inline int qcn_sdio_runtime_idle(struct device *dev)
{
	return -ENOTSUPP;
}

static inline int qcn_sdio_lpm_set(struct qcn_sdio *sdio_ctxt, bool enable)
{
	return -ENOTSUPP;
}
#endif

static int qcn_sdio_inject_sys_err(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	int ret = 0;
	u32 value = 0;

	pr_info("%s: func %d curr_sw_mode=%d\n", __func__,
		func->num, sdio_ctxt->curr_sw_mode);

	value = META_INFO(QCN_SDIO_SYS_ERR_HEVENT, (u32)0);

	qcn_sdio_rpm_get();

	sdio_claim_host(func);
	sdio_writel(func, value, SDIO_QCN_HRQ_PUSH, &ret);
	sdio_release_host(func);

	qcn_sdio_rpm_put();

	pr_info("%s: exit with ret %d\n", __func__, ret);
	return ret;
}

static int qcn_sdio_action_show(struct seq_file *s, void *data)
{
	seq_puts(s, "\nUsage: echo <action> > <debugfs_path>/qcn_sdio/action\n");
	seq_puts(s, "<action> can be one of below:\n");

#ifdef CONFIG_LPM
	seq_puts(s, "lpm_enable: Enable Low Power Feature\n");
	seq_puts(s, "lpm_disable: Disable Low Power Feature\n");
	seq_puts(s, "suspend: Trigger suspend\n");
	seq_puts(s, "resume: Trigger resume\n");
#endif

	seq_puts(s, "inject_sys_err: Inject sys err to trigger SSR\n");
	seq_puts(s, "reset: Reset sdio\n");
	return 0;
}

static int qcn_sdio_action_open(struct inode *inode, struct file *file)
{
	return single_open(file, qcn_sdio_action_show, inode->i_private);
}

static ssize_t qcn_sdio_action_write(struct file *fp,
					 const char __user *user_buf,
					 size_t count, loff_t *off)
{
	struct qcn_sdio *sdio_ctxt =
		((struct seq_file *)fp->private_data)->private;
	char buf[64];
	char *cmd;
	unsigned int len = 0;
	int ret = 0;
	struct device *dev;

	if (!sdio_ctxt || !sdio_ctxt->func) {
		pr_err("Invalid sdio context");
		return -ENODEV;
	}

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	cmd = buf;

	dev = &sdio_ctxt->func->dev;

	if (sysfs_streq(cmd, "suspend")) {
		ret = qcn_sdio_suspend(dev);
	} else if (sysfs_streq(cmd, "resume")) {
		ret = qcn_sdio_resume(dev);
	} else if (sysfs_streq(cmd, "lpm_enable")) {
		ret = qcn_sdio_lpm_set(sdio_ctxt, true);
	} else if (sysfs_streq(cmd, "lpm_disable")) {
		ret = qcn_sdio_lpm_set(sdio_ctxt, false);
	} else if (sysfs_streq(cmd, "inject_sys_err")) {
		ret = qcn_sdio_inject_sys_err(dev);
	} else if (sysfs_streq(cmd, "reset")) {
		ret = qcn_sdio_reset();
	} else {
		pr_err("Invalid command %s\n", cmd);
		ret = -EINVAL;
	}

	if (ret) {
		pr_err("%s: failed with ret %d\n", __func__, ret);
		return ret;
	}

	return count;
}
int qcn_sdio_inject_sys_err_handle(struct device *dev)
{
	return qcn_sdio_inject_sys_err(dev);
}

static const struct file_operations qcn_sdio_action_fops = {
	.read		= seq_read,
	.write		= qcn_sdio_action_write,
	.release	= single_release,
	.open		= qcn_sdio_action_open,
	.owner		= THIS_MODULE,
	.llseek		= seq_lseek,
};


static int qcn_sdio_debugfs_create(struct qcn_sdio *sdio_ctxt)
{
	int ret = 0;
	struct dentry *root_dentry;

	root_dentry = debugfs_create_dir("qcn_sdio", 0);
	if (IS_ERR(root_dentry)) {
		ret = PTR_ERR(root_dentry);
		pr_err("Unable to create debugfs %d\n", ret);
		return -EINVAL;
	}

	sdio_ctxt->dbg_dentry = root_dentry;
	debugfs_create_file("action", 0644, root_dentry, sdio_ctxt,
			    &qcn_sdio_action_fops);
	pr_debug("qcn_sdio debugfs created\n");
	return 0;
}

static void qcn_sdio_debugfs_destroy(struct qcn_sdio *sdio_ctxt)
{
	if (sdio_ctxt->dbg_dentry) {
		debugfs_remove_recursive(sdio_ctxt->dbg_dentry);
		sdio_ctxt->dbg_dentry = NULL;
		pr_debug("qcn_sdio debugfs destroyed\n");
	}
}

#ifdef CONFIG_QCN_SDIO_OOB_IRQ
/**
 * qcn_sdio_setup_oob_irq() - request the OOB IRQ and leave it disabled
 * @func: the SDIO function whose of_node carries the "oob-irq" interrupt.
 *
 * Done up front in probe so the later polling -> OOB switch only has to
 * enable_irq() an already-requested line and cannot lose the first event to a
 * request/enable race.
 *
 * Return: 0 on success, -ENODEV if the DT node or interrupt is missing, or a
 * negative errno from request_threaded_irq().
 */
static int qcn_sdio_setup_oob_irq(struct sdio_func *func)
{
	struct device_node *node = func->dev.of_node;
	int irq, ret;

	if (!node)
		return -ENODEV;

	irq = of_irq_get_byname(node, "oob-irq");
	if (irq <= 0)
		return irq ? irq : -ENODEV;

	ret = request_threaded_irq(irq, NULL,
				   qcn_sdio_oob_irq_thread,
				   IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				   "oob_irq", sdio_ctxt);
	if (ret)
		return ret;

	sdio_ctxt->oob_irq = irq;
	disable_irq(irq);

	return 0;
}

/**
 * qcn_sdio_setup_irq_path() - arm the OOB-capable interrupt mechanism
 * @func: the SDIO function being probed.
 *
 * Called from probe with the host claimed, and releases it. Programs the OOB
 * GPIO number while the device is still in PBL so SBL can mux the pin, requests
 * the OOB IRQ disabled, and starts in polling mode. An OOB setup failure is not
 * fatal: the driver stays in polling mode. Losing the poll workqueue is, as
 * there would be no status path left at all.
 *
 * Return: 0 on success, -ENOMEM if the poll workqueue cannot be created.
 */
static int qcn_sdio_setup_irq_path(struct sdio_func *func)
{
	int ret;

	sdio_ctxt->oob_poll_wq =
		alloc_ordered_workqueue("qcn_sdio_poll",
					WQ_HIGHPRI | WQ_MEM_RECLAIM);
	if (!sdio_ctxt->oob_poll_wq) {
		pr_err("%s: Error: SDIO create poll wq\n", __func__);
		sdio_release_host(sdio_ctxt->func);
		return -ENOMEM;
	}

	mutex_init(&sdio_ctxt->oob_lock);
	INIT_WORK(&sdio_ctxt->oob_poll_work, qcn_sdio_poll_work);

	sdio_writel(sdio_ctxt->func, oob_irq_gpio,
		    SDIO_QCN_HOST_TRANS_REG0, &ret);
	if (ret)
		pr_err("%s: failed to program OOB GPIO number, ret=%d\n",
		       __func__, ret);
	pr_info("Set OOB interrupt GPIO num: %d\n", oob_irq_gpio);

	sdio_release_host(sdio_ctxt->func);

	ret = qcn_sdio_setup_oob_irq(func);
	if (ret)
		pr_err("%s: OOB IRQ setup failed, ret=%d; staying in polling mode\n",
		       __func__, ret);

	if (qcn_read_meta_info()) {
		pr_err("%s: Error: SDIO Config\n", __func__);
		qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT, (u32)0);
	}

	qcn_sdio_set_irq_mode(QCN_SDIO_IRQ_POLLING);

	current_host = func->card->host;
	return 0;
}

static void qcn_sdio_remove_irq(void)
{
	sdio_ctxt->poll_stopping = true;
	cancel_work_sync(&sdio_ctxt->oob_poll_work);
	destroy_workqueue(sdio_ctxt->oob_poll_wq);
	sdio_ctxt->oob_poll_wq = NULL;

	qcn_sdio_write_oob_en(false);

	if (sdio_ctxt->oob_irq > 0) {
		if (sdio_ctxt->oob_irq_enabled) {
			disable_irq(sdio_ctxt->oob_irq);
			sdio_ctxt->oob_irq_enabled = false;
		}
		free_irq(sdio_ctxt->oob_irq, sdio_ctxt);
		sdio_ctxt->oob_irq = 0;
	}
	sdio_ctxt->irq_mode = QCN_SDIO_IRQ_NONE;
}

static void qcn_sdio_ctxt_free(void)
{
	mutex_destroy(&sdio_ctxt->oob_lock);
	kfree(sdio_ctxt);
	sdio_ctxt = NULL;
}
#else
/**
 * qcn_sdio_setup_irq_path() - arm the legacy in-band interrupt
 * @func: the SDIO function being probed.
 *
 * Called from probe with the host claimed, and releases it. Claims the SDIO
 * DAT1 in-band interrupt. Unlike the OOB variant, a failure here is fatal as
 * there is no fallback path.
 *
 * Return: 0 on success or a negative errno from sdio_claim_irq().
 */
static int qcn_sdio_setup_irq_path(struct sdio_func *func)
{
	int ret;

	ret = sdio_claim_irq(sdio_ctxt->func, qcn_sdio_irq_handler);
	if (ret) {
		pr_err("%s: Error:%d SDIO claim irq\n", __func__, ret);
		sdio_release_host(sdio_ctxt->func);
		return ret;
	}

	qcn_enable_async_irq(true);
	sdio_release_host(sdio_ctxt->func);

	if (qcn_read_meta_info()) {
		pr_err("%s: Error: SDIO Config\n", __func__);
		qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT, (u32)0);
	}

	current_host = func->card->host;
	return 0;
}

static void qcn_sdio_remove_irq(void)
{
	sdio_claim_host(sdio_ctxt->func);
	sdio_release_irq(sdio_ctxt->func);
	sdio_release_host(sdio_ctxt->func);
}

static void qcn_sdio_ctxt_free(void)
{
	kfree(sdio_ctxt);
	sdio_ctxt = NULL;
}
#endif

static
int qcn_sdio_probe(struct sdio_func *func, const struct sdio_device_id *id)
{
	int ret = 0;

	sdio_ctxt = kzalloc(sizeof(struct qcn_sdio), GFP_KERNEL);
	if (!sdio_ctxt)
		return -ENOMEM;

	sdio_ctxt->func = func;
	sdio_ctxt->id = id;
	sdio_set_drvdata(func, sdio_ctxt);

	sdio_ctxt->qcn_sdio_wq = create_singlethread_workqueue("qcn_sdio");
	if (!sdio_ctxt->qcn_sdio_wq) {
		pr_err("%s: Error: SDIO create wq\n", __func__);
		goto err;
	}

	sdio_ctxt->qcn_sdio_cmpl_wq = create_singlethread_workqueue("qcn_sdio_complete");
	if (!sdio_ctxt->qcn_sdio_cmpl_wq) {
		pr_err("%s: Error: SDIO create complete wq\n", __func__);
		goto err;
	}

	for (ret = 0; ret < QCN_SDIO_CH_MAX; ret++) {
		sdio_ctxt->ch[ret] = NULL;
		atomic_set(&sdio_ctxt->ch_status[ret], -1);
	}

	spin_lock_init(&sdio_ctxt->lock_free_q);
	spin_lock_init(&sdio_ctxt->lock_wait_q);
	spin_lock_init(&async_lock);
	mutex_init(&sdio_ctxt->lock_cmpl_q);
	INIT_WORK(&sdio_ctxt->sdio_rw_w, qcn_sdio_rw_work);
	INIT_WORK(&sdio_ctxt->sdio_cmpl_w, qcn_sdio_cmpl_work);
	INIT_LIST_HEAD(&sdio_ctxt->rw_free_q);
	INIT_LIST_HEAD(&sdio_ctxt->rw_wait_q);
	INIT_LIST_HEAD(&sdio_ctxt->rw_cmpl_q);

	atomic_set(&sdio_ctxt->wait_list_count, QCN_SDIO_RW_REQ_MAX);
	for (ret = 0; ret < QCN_SDIO_RW_REQ_MAX; ret++)
		qcn_sdio_free_rw_req(&sdio_ctxt->rw_req_info[ret]);

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_enable_func(sdio_ctxt->func);
	if (ret) {
		pr_err("%s: Error:%d SDIO enable func\n", __func__, ret);
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}

	ret = qcn_sdio_setup_irq_path(func);
	if (ret)
		goto err;

	if (!retune) {
		pr_info("%s Probing driver with retune disabled\n", __func__);
		mmc_retune_disable(current_host);
	}

#ifdef BLOCK_SIZE_FIX
	/* After setting this flag, the current block size will be the maximum
	 * size in byte mode transmission.
	 */
	func->card->quirks |= MMC_QUIRK_BLKSZ_FOR_BYTE_MODE;
#endif
	qcn_sdio_debugfs_create(sdio_ctxt);
	atomic_set(&xport_status, 1);
	return 0;
err:
	kfree(sdio_ctxt);
	sdio_ctxt = NULL;
	return ret;
}

static void qcn_sdio_remove(struct sdio_func *func)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	struct qcn_sdio_ch_info *ch_info = NULL;

	pm_runtime_get_sync(&func->dev);

	qcn_sdio_debugfs_destroy(sdio_ctxt);
	atomic_set(&xport_status, 0);

#ifndef CONFIG_NAPIER_X86
	sdio_claim_host(sdio_ctxt->func);
	qcn_enable_async_irq(false);
	sdio_release_host(sdio_ctxt->func);
#endif
	qcn_sdio_remove_irq();

	qcn_sdio_purge_rw_buff();

	destroy_workqueue(sdio_ctxt->qcn_sdio_wq);
	destroy_workqueue(sdio_ctxt->qcn_sdio_cmpl_wq);
	mutex_destroy(&sdio_ctxt->lock_cmpl_q);

	mutex_lock(&lock);
	list_for_each_entry(cinfo, &cinfo_head, cli_list) {
		while (!list_empty(&cinfo->ch_head)) {
			ch_info = list_first_entry(&cinfo->ch_head,
					struct qcn_sdio_ch_info, ch_list);
			sdio_al_deregister_channel(&ch_info->ch_handle);
		}
		mutex_unlock(&lock);
		if (cinfo->is_probed) {
			cinfo->cli_data.remove(&cinfo->cli_handle);
			cinfo->is_probed = 0;
		}
		mutex_lock(&lock);
	}
	mutex_unlock(&lock);

	qcn_sdio_ctxt_free();
	pm_runtime_put(&func->dev);
	mmc_retune_enable(current_host);
}

static const struct sdio_device_id qcn_sdio_devices[] = {
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCN_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCN_V2))},
	{},
};

MODULE_DEVICE_TABLE(sdio, qcn_sdio_devices);

#ifdef CONFIG_LPM
static const struct dev_pm_ops qcn_sdio_pm_ops = {
    .suspend = qcn_sdio_suspend,
    .resume = qcn_sdio_resume,
    SET_RUNTIME_PM_OPS(qcn_sdio_runtime_suspend, qcn_sdio_runtime_resume,
			   qcn_sdio_runtime_idle)
};
#endif

static struct sdio_driver qcn_sdio_driver = {
	.name = "qcn_sdio",
	.id_table = qcn_sdio_devices,
	.probe = qcn_sdio_probe,
	.remove = qcn_sdio_remove,
#ifdef CONFIG_LPM
	.drv = {
		.pm = &qcn_sdio_pm_ops,
	},
#endif
};

static int __qcn_sdio_register_driver(void *data)
{
#ifndef CONFIG_NAPIER_X86
	struct platform_device *pdev = data;
#endif
	int ret = 0;

	ret = sdio_register_driver(&qcn_sdio_driver);
	if (ret) {
		pr_err("SDIO driver registration failed: %d\n", ret);
		mutex_destroy(&lock);
		atomic_set(&status, 0);
		return ret;
	}

	pr_info("sdio_register_driver done\n");

#ifndef CONFIG_NAPIER_X86
	qcn_create_sysfs(&pdev->dev);
#endif
	return 0;
}

#ifdef CONFIG_WLAN_CNSS_CORE
int qcn_sdio_remove_all_clients(void)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	struct qcn_sdio_ch_info *ch_info = NULL;

	mutex_lock(&lock);
	list_for_each_entry(cinfo, &cinfo_head, cli_list) {
		while (!list_empty(&cinfo->ch_head)) {
			ch_info = list_first_entry(&cinfo->ch_head,
					struct qcn_sdio_ch_info, ch_list);
			sdio_al_deregister_channel(&ch_info->ch_handle);
		}
		mutex_unlock(&lock);
		if (cinfo->is_probed) {
			cinfo->cli_data.remove(&cinfo->cli_handle);
			cinfo->is_probed = 0;
		}
		mutex_lock(&lock);
	}
	mutex_unlock(&lock);

	return 0;
}
#endif

static int qcn_sdio_register_driver_async(struct platform_device *pdev)
{
	struct task_struct *thread;

	thread = kthread_run(__qcn_sdio_register_driver, pdev, "qcn_reg_sdio");
	if (IS_ERR(thread)) {
		pr_err("Failed to run qcn_reg_sdio thread\n");
		return -EIO;
	}

	return 0;
}

static int qcn_sdio_plat_probe(struct platform_device *pdev)
{
	mutex_init(&lock);
	INIT_LIST_HEAD(&cinfo_head);
	atomic_set(&status, 1);
	init_completion(&client_probe_complete);

	return qcn_sdio_register_driver_async(pdev);
}

static int qcn_sdio_plat_remove(struct platform_device *pdev)
{
	struct qcn_sdio_client_info *cinfo = NULL;

	mutex_lock(&lock);
	while (!list_empty(&cinfo_head)) {
		cinfo = list_first_entry(&cinfo_head, struct
						qcn_sdio_client_info, cli_list);
		mutex_unlock(&lock);
		sdio_al_deregister_client(&cinfo->cli_handle);
		mutex_lock(&lock);
	}

	mutex_unlock(&lock);
	sdio_unregister_driver(&qcn_sdio_driver);

	mutex_destroy(&lock);
	if (sdio_ctxt) {
		qcn_sdio_remove_irq();
		destroy_workqueue(sdio_ctxt->qcn_sdio_wq);
		destroy_workqueue(sdio_ctxt->qcn_sdio_cmpl_wq);
		mutex_destroy(&sdio_ctxt->lock_cmpl_q);
		qcn_sdio_ctxt_free();
	}

	atomic_set(&status, 0);
	return 0;
}

static const struct of_device_id qcn_sdio_dt_match[] = {
	{.compatible = "qcom,qcn-sdio"},
	{}
};
MODULE_DEVICE_TABLE(of, qcn_sdio_dt_match);

#ifndef CONFIG_NAPIER_X86
static struct platform_driver qcn_sdio_plat_driver = {
	.probe  = qcn_sdio_plat_probe,
	.remove = qcn_sdio_plat_remove,
	.driver = {
		.name = "qcn-sdio",
		.owner = THIS_MODULE,
		.of_match_table = qcn_sdio_dt_match,
	},
};
#endif

#ifdef CONFIG_WLAN_CNSS_CORE
int qcn_sdio_init(void)
#else
static int __init qcn_sdio_init(void)
#endif
{
#ifdef CONFIG_NAPIER_X86
	return qcn_sdio_plat_probe(NULL);
#else	
	return platform_driver_register(&qcn_sdio_plat_driver);
#endif
}

#ifdef CONFIG_WLAN_CNSS_CORE
void qcn_sdio_exit(void)
#else
static void __exit qcn_sdio_exit(void)
#endif
{
#ifdef CONFIG_NAPIER_X86
	qcn_sdio_plat_remove(NULL);
#else
	platform_driver_unregister(&qcn_sdio_plat_driver);
#endif
}

#ifndef CONFIG_WLAN_CNSS_CORE
module_init(qcn_sdio_init);
module_exit(qcn_sdio_exit);
#endif

int sdio_al_is_ready(void)
{
	if (atomic_read(&status))
		return 0;
	else
		return -EBUSY;
}
EXPORT_SYMBOL(sdio_al_is_ready);

struct sdio_al_client_handle *sdio_al_register_client(
					struct sdio_al_client_data *client_data)
{
	struct qcn_sdio_client_info *client_info = NULL;

	if (!((client_data) && (client_data->name) &&
			(client_data->probe) && (client_data->remove))) {
		pr_err("%s: SDIO: Invalid param\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	client_info = (struct qcn_sdio_client_info *)
		kzalloc(sizeof(struct qcn_sdio_client_info), GFP_KERNEL);
	if (!client_info)
		return ERR_PTR(-ENOMEM);

	memcpy(&client_info->cli_data, client_data,
					sizeof(struct sdio_al_client_data));

	if (!strcmp(client_data->name, "SDIO_AL_CLIENT_TTY")) {
		client_info->cli_handle.id = QCN_SDIO_CLI_ID_TTY;
		client_info->cli_handle.block_size = QCN_SDIO_TTY_BLK_SZ;
	} else if (!strcmp(client_data->name, "SDIO_AL_CLIENT_WLAN")) {
		client_info->cli_handle.id = QCN_SDIO_CLI_ID_WLAN;
		client_info->cli_handle.block_size = QCN_SDIO_MROM_BLK_SZ;
	} else if (!strcmp(client_data->name, "SDIO_AL_CLIENT_QMI")) {
		client_info->cli_handle.id = QCN_SDIO_CLI_ID_QMI;
		client_info->cli_handle.block_size = QCN_SDIO_MROM_BLK_SZ;
	} else if (!strcmp(client_data->name, "SDIO_AL_CLIENT_DIAG")) {
		client_info->cli_handle.id = QCN_SDIO_CLI_ID_DIAG;
		client_info->cli_handle.block_size = QCN_SDIO_MROM_BLK_SZ;
	} else {
		pr_err("%s: SDIO: Invalid name\n", __func__);
		kfree(client_info);
		return ERR_PTR(-EINVAL);
	}
	client_info->cli_handle.client_data = &client_info->cli_data;

	INIT_LIST_HEAD(&client_info->ch_head);
	mutex_lock(&lock);
	list_add_tail(&client_info->cli_list, &cinfo_head);
	mutex_unlock(&lock);

	client_info->is_probed = 0;
	if ((sdio_ctxt) && (sdio_ctxt->curr_sw_mode)) {
		if ((sdio_ctxt->curr_sw_mode == QCN_SDIO_SW_MROM) &&
			(client_info->cli_handle.id > QCN_SDIO_CLI_ID_TTY)) {
			qcn_sdio_config(client_info);
			client_info->is_probed = !client_data->probe(
						&client_info->cli_handle);
			qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);
		}
	}

	return &client_info->cli_handle;
}
EXPORT_SYMBOL(sdio_al_register_client);

void sdio_al_deregister_client(struct sdio_al_client_handle *handle)
{
	struct qcn_sdio_ch_info	*ch_info = NULL;
	struct qcn_sdio_client_info *client_info = NULL;

	if (!handle) {
		pr_err("%s: SDIO: Invalid param\n", __func__);
		return;
	}

	client_info = container_of(handle, struct qcn_sdio_client_info,
								cli_handle);

	while (!list_empty(&client_info->ch_head)) {
		ch_info = list_first_entry(&client_info->ch_head,
					struct qcn_sdio_ch_info, ch_list);
		sdio_al_deregister_channel(&ch_info->ch_handle);
	}
	if (client_info->is_probed) {
		client_info->cli_data.remove(handle);
		client_info->is_probed = 0;
	}

	mutex_lock(&lock);
	list_del(&client_info->cli_list);
	kfree(client_info);
	mutex_unlock(&lock);
}
EXPORT_SYMBOL(sdio_al_deregister_client);

struct qcn_sdio_chan_nameid_mapping {
	const char *name;
	uint32_t id;
};

#ifdef CONFIG_DIAG_SDIO
static const struct qcn_sdio_chan_nameid_mapping nameid_mapping[] =
{
	{"SDIO_AL_TTY_CH0", QCN_SDIO_CH_0},
	{"SDIO_AL_WLAN_CH0", QCN_SDIO_CH_0},
	{"SDIO_AL_WLAN_CH1", QCN_SDIO_CH_1},
	{"SDIO_AL_QMI_CH0", QCN_SDIO_CH_2},
	{"SDIO_AL_DIAG_CH0", QCN_SDIO_CH_3},
	{NULL, QCN_SDIO_CH_MAX}
};
#else
static const struct qcn_sdio_chan_nameid_mapping nameid_mapping[] =
{
	{"SDIO_AL_TTY_CH0", QCN_SDIO_CH_0},
	{"SDIO_AL_WLAN_CH0", QCN_SDIO_CH_1},
	{"SDIO_AL_WLAN_CH1", QCN_SDIO_CH_2},
	{"SDIO_AL_QMI_CH0", QCN_SDIO_CH_3},
	{NULL, QCN_SDIO_CH_MAX}
};
#endif

struct sdio_al_channel_handle *sdio_al_register_channel(
		struct sdio_al_client_handle *client_handle,
		struct sdio_al_channel_data *channel_data)
{
	struct qcn_sdio_ch_info	*ch_info = NULL;
	struct qcn_sdio_client_info *client_info = NULL;
	int i = 0;
	bool is_valid = false;

	pr_err("sdio_al_register_channel for channel name:%s\n",
	       channel_data->name);
	if (!((channel_data) && (channel_data->name) && (client_handle) &&
	      (channel_data->client_data))) {
		pr_err("%s: SDIO: Invalid param\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	ch_info = kzalloc(sizeof(struct qcn_sdio_client_info), GFP_KERNEL);
	if (!ch_info)
		return ERR_PTR(-ENOMEM);

	memcpy(&ch_info->ch_data, channel_data,
	       sizeof(struct sdio_al_channel_data));

	while (nameid_mapping[i].name) {
		if (strcmp(channel_data->name, nameid_mapping[i].name)) {
			i++;
			continue;
		}

		is_valid = true;
		if (atomic_read(&sdio_ctxt->ch_status[nameid_mapping[i].id])
		    < 0)
			ch_info->ch_handle.channel_id = nameid_mapping[i].id;

		break;
	}

	if (!is_valid) {
		pr_err("%s: SDIO: Invalid CH name: %s\n", __func__,
		       channel_data->name);
		kfree(ch_info);
		return ERR_PTR(-EINVAL);
	}

	pr_err("%s: CH name %s id %u", __func__,
	       ch_info->ch_data.name, ch_info->ch_handle.channel_id);
	client_info = container_of(client_handle, struct qcn_sdio_client_info,
								cli_handle);
	ch_info->ch_handle.channel_data = &ch_info->ch_data;
	ch_info->chandle = &client_info->cli_handle;
	list_add_tail(&ch_info->ch_list, &client_info->ch_head);
	sdio_ctxt->ch[ch_info->ch_handle.channel_id] = ch_info;
	atomic_set(&sdio_ctxt->ch_status[ch_info->ch_handle.channel_id], 0);

	return &ch_info->ch_handle;
}
EXPORT_SYMBOL(sdio_al_register_channel);

void sdio_al_deregister_channel(struct sdio_al_channel_handle *ch_handle)
{
	int ret = 0;
	struct qcn_sdio_ch_info *ch_info = NULL;

	if (!ch_handle) {
		pr_err("%s: Error: Invalid Param\n", __func__);
		return;
	}

	do {
		ret = atomic_cmpxchg(
			&sdio_ctxt->ch_status[ch_handle->channel_id], 0, 1);
		if (ret) {
			if (ret == -1)
				return;

			usleep_range(1000, 1500);
		}
	} while (ret);

	ch_info = sdio_ctxt->ch[ch_handle->channel_id];
	if (ch_info) {
		list_del(&ch_info->ch_list);
		sdio_ctxt->ch[ch_handle->channel_id] = NULL;
		atomic_set(&sdio_ctxt->ch_status[ch_handle->channel_id], -1);
		kfree(ch_info);
	}
}
EXPORT_SYMBOL(sdio_al_deregister_channel);

void register_tx_bundle_size(int bundle_buf_size)
{
	sdio_ctxt->tx_bundle_buf_size = bundle_buf_size;
}
EXPORT_SYMBOL(register_tx_bundle_size);

int sdio_al_queue_transfer_async(struct sdio_al_channel_handle *handle,
		enum sdio_al_dma_direction dir,
		void *buf, size_t len, int priority, void *ctxt)
{
	struct qcn_sdio_rw_info *rw_req = NULL;
	u32 cid = QCN_SDIO_CH_MAX;

	if (!atomic_read(&xport_status))
		return -ENODEV;

	if (!handle) {
		pr_err("%s: Error: Invalid Param\n", __func__);
		return -EINVAL;
	}

	cid = handle->channel_id;

	if (IS_TX_INVALID(cid, dir))
		return -EINVAL;

	if (!(cid < QCN_SDIO_CH_MAX) &&
				(atomic_read(&sdio_ctxt->ch_status[cid]) < 0))
		return -EINVAL;

	if (dir == SDIO_AL_TX && atomic_read(&sdio_ctxt->free_list_count) <= 8)
		return -ENOMEM;

	rw_req = qcn_sdio_alloc_rw_req();
	if (!rw_req)
		return -ENOMEM;

	rw_req->cid = cid;
	rw_req->dir = dir;
	rw_req->buf = buf;
	rw_req->len = len;
	rw_req->ctxt = ctxt;

	qcn_sdio_add_rw_req(rw_req);
	queue_work(sdio_ctxt->qcn_sdio_wq, &sdio_ctxt->sdio_rw_w);

	return 0;
}
EXPORT_SYMBOL(sdio_al_queue_transfer_async);

int sdio_al_queue_transfer(struct sdio_al_channel_handle *ch_handle,
		enum sdio_al_dma_direction dir,
		void *buf, size_t len, int priority)
{
	int ret = 0;
	u32 cid = QCN_SDIO_CH_MAX;

	if (!atomic_read(&xport_status))
		return -ENODEV;

	if (!ch_handle) {
		pr_err("%s: SDIO: Invalid Param\n", __func__);
		return -EINVAL;
	}

	if (IS_TX_INVALID(ch_handle->channel_id, dir))
		return -EINVAL;

	/*
	 * DEAD CODE: This branch condition is logically contradictory and
	 * can never be true in practice.
	 *
	 * Condition: rw_wait_q is NOT empty  AND  wait_list_count == 0
	 */
	if (dir == SDIO_AL_RX && !list_empty(&sdio_ctxt->rw_wait_q) &&
				!atomic_read(&sdio_ctxt->wait_list_count)) {
		sdio_al_queue_transfer_async(ch_handle, dir, buf, len, true,
							(void *)(uintptr_t)len);
		pr_info("%s: switching to async\n", __func__);
		ret = 1;
	} else {
		cid = ch_handle->channel_id;

		if (!(cid < QCN_SDIO_CH_MAX))
			return -EINVAL;

		if (dir == SDIO_AL_RX) {
			if (!atomic_read(&sdio_ctxt->wait_list_count))
				ret = qcn_sdio_recv_buff(cid, buf, len);
			else {
				sdio_al_queue_transfer_async(ch_handle, dir,
					buf, len, true, (void *)(uintptr_t)len);
				pr_info("%s switching to async\n", __func__);
				ret = 1;
			}

			if (rx_dump)
				HEX_DUMP("SYNC_RECV: ", buf, len);
		} else if (dir == SDIO_AL_TX) {
			ret = qcn_sdio_send_buff(cid, buf, len);
			if (tx_dump)
				HEX_DUMP("SYNC_SEND: ", buf, len);
		} else
			ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(sdio_al_queue_transfer);

int sdio_al_meta_transfer(struct sdio_al_channel_handle *handle,
					unsigned int data, unsigned int trans)
{
	u32 cid = QCN_SDIO_CH_MAX;
	u8 event = 0;

	if (!atomic_read(&xport_status))
		return -ENODEV;

	if (!handle)
		return -EINVAL;

	cid = handle->channel_id;

	if (!(cid < QCN_SDIO_CH_MAX))
		return -EINVAL;

	event = (u8)((data & QCN_SDIO_HMETA_EVENT_BMSK) >>
						QCN_SDIO_HMETA_EVENT_SHFT);

	if (cid == QCN_SDIO_CH_0) {
		if ((event < QCN_SDIO_META_START_CH0) &&
					(event >= QCN_SDIO_META_START_CH1)) {
			return -EINVAL;
		}
	} else if (cid == QCN_SDIO_CH_1) {
		if ((event < QCN_SDIO_META_START_CH1) &&
					(event >= QCN_SDIO_META_START_CH2)) {
			return -EINVAL;
		}
	} else if (cid == QCN_SDIO_CH_2) {
		if ((event < QCN_SDIO_META_START_CH2) &&
					(event >= QCN_SDIO_META_START_CH3)) {
			return -EINVAL;
		}
	} else if (cid == QCN_SDIO_CH_3) {
		if ((event < QCN_SDIO_META_START_CH3) &&
					(event >= QCN_SDIO_META_END)) {
			return -EINVAL;
		}
	}

	return qcn_send_meta_info(event, data);
}
EXPORT_SYMBOL(sdio_al_meta_transfer);

int qcn_sdio_card_state(bool enable)
{
	int ret = 0;

	if (!current_host)
		return -ENODEV;

	mmc_try_claim_host(current_host, 2000);
	if (enable) {
		if (!atomic_read(&xport_status)) {
			ret = mmc_add_host(current_host);
			pr_err("QCN: Added mmc host");
		} else
			pr_err("QCN: Ignore add mmc host call");
		if (ret)
			pr_err("%s ret = %d\n", __func__, ret);
	} else {
		if (atomic_read(&xport_status)) {
			mmc_remove_host(current_host);
			pr_err("QCN: Removed mmc host");
		}else{
			ret = -EINVAL;
			pr_err("QCN: Can't remove mmc host\n");
		}
	}
	mmc_release_host(current_host);

	return ret;
}
EXPORT_SYMBOL(qcn_sdio_card_state);

#ifndef CONFIG_NAPIER_X86
static ssize_t qcn_card_state(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t count)
{
	int state = 0;

	if (sscanf(buf, "%du", &state) != 1)
		return -EINVAL;

	qcn_sdio_card_state(state);

	return count;
}
static DEVICE_ATTR(card_state, 0220, NULL, qcn_card_state);

static int qcn_create_sysfs(struct device *dev)
{
	int ret = 0;

	ret = device_create_file(dev, &dev_attr_card_state);
	if (ret) {
		pr_err("Failed to create device file, err = %d\n", ret);
		goto out;
	}

	return 0;
out:
	return ret;
}
#endif
#ifndef CONFIG_WLAN_CNSS_CORE
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("QCN Driver");
#endif
