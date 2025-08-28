/* Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
#include <linux/version.h>
#include "qcn_sdio.h"
#ifdef CONFIG_WLAN_CNSS_CORE
#include "unified_wlan_cnsscore.h"
#endif

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

int qcn_sw_mode_change(enum qcn_sdio_sw_mode mode);
int reset_thread(void *data);
static void qcn_set_host_clock(unsigned int hz);

static struct mmc_host *current_host;

#define HEX_DUMP(mode, buf, len)				\
	print_hex_dump(KERN_ERR, mode, 2, 32, 4, buf,		\
			dump_len > len ? len : dump_len, 0)

struct qcn_sdio {
	enum qcn_sdio_sw_mode curr_sw_mode;
	struct sdio_func *func;
	const struct sdio_device_id *id;
	struct qcn_sdio_ch_info *ch[QCN_SDIO_CH_MAX];
	atomic_t ch_status[QCN_SDIO_CH_MAX];
	spinlock_t lock_free_q;
	spinlock_t lock_wait_q;
	u32 rx_addr_base;
	u32 tx_addr_base;
	u8 rx_cnum_base;
	u8 tx_cnum_base;
	struct qcn_sdio_rw_info rw_req_info[QCN_SDIO_RW_REQ_MAX];
	struct list_head rw_free_q;
	struct list_head rw_wait_q;
	atomic_t free_list_count;
	atomic_t wait_list_count;
	struct workqueue_struct *qcn_sdio_wq;
	struct work_struct sdio_rw_w;
	struct dentry *dbg_dentry;
#ifdef CONFIG_LPM
	bool low_power_disabled;
	atomic_t suspended;
#endif
	bool wake_irq_enable;
	int wake_irq_nr;
	int wake_irq_flag;
	int wake_irq_pending;
};

static struct qcn_sdio *sdio_ctxt;
struct completion client_probe_complete;
static struct mutex lock;
static struct list_head cinfo_head;
static atomic_t status;
static atomic_t xport_status;
static spinlock_t async_lock;
static struct task_struct *reset_task;

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

#define VALID_IRQ(irq_nr)	(irq_nr > 0 ? true : false)

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
	spin_lock(&sdio_ctxt->lock_free_q);
	list_add_tail(&rw_req->list, &sdio_ctxt->rw_free_q);
	atomic_inc(&sdio_ctxt->free_list_count);
	spin_unlock(&sdio_ctxt->lock_free_q);
}

static void qcn_sdio_purge_rw_buff(void)
{
	struct qcn_sdio_rw_info *rw_req = NULL;

	while (!list_empty(&sdio_ctxt->rw_wait_q)) {
		rw_req = list_first_entry(&sdio_ctxt->rw_wait_q,
						struct qcn_sdio_rw_info, list);
		list_del(&rw_req->list);
		qcn_sdio_free_rw_req(rw_req);
	}
}

void qcn_sdio_client_probe_complete(int id)
{
	complete(&client_probe_complete);
}
EXPORT_SYMBOL(qcn_sdio_client_probe_complete);

static struct qcn_sdio_rw_info *qcn_sdio_alloc_rw_req(void)
{
	struct qcn_sdio_rw_info *rw_req = NULL;

	spin_lock(&sdio_ctxt->lock_free_q);
	if (list_empty(&sdio_ctxt->rw_free_q)) {
		spin_unlock(&sdio_ctxt->lock_free_q);
		return rw_req;
	}

	rw_req = list_first_entry(&sdio_ctxt->rw_free_q,
						struct qcn_sdio_rw_info, list);
	list_del(&rw_req->list);
	atomic_dec(&sdio_ctxt->free_list_count);
	spin_unlock(&sdio_ctxt->lock_free_q);

	return rw_req;
}

static void qcn_sdio_add_rw_req(struct qcn_sdio_rw_info *rw_req)
{
	spin_lock(&sdio_ctxt->lock_wait_q);
	list_add_tail(&rw_req->list, &sdio_ctxt->rw_wait_q);
	atomic_inc(&sdio_ctxt->wait_list_count);
	spin_unlock(&sdio_ctxt->lock_wait_q);
}

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

static int qcn_send_meta_info(u8 event, u32 data)
{
	int ret = 0;
	u32 i = 0;
	u32 value = 0;
	u8 temp = 0;

	value =	META_INFO(event, data);

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

	return ret;
}

static int qcn_read_crq_info(void)
{
	int ret = 0;
	u32 i = 0;
	u32 temp = 0;
	u32 data = 0;
	u32 len = 0;
	u8 cid = 0;

	struct sdio_al_channel_handle *ch_handle = NULL;

	sdio_claim_host(sdio_ctxt->func);
	if (sdio_ctxt->curr_sw_mode < QCN_SDIO_SW_SBL) {
		for (i = 0; i < 4; i++) {
			temp = sdio_readb(sdio_ctxt->func,
						(SDIO_QCN_CRQ_PULL + i), &ret);
			temp = temp << (i * 8);
			data |= temp;
		}
	} else {
		data = sdio_readl(sdio_ctxt->func, SDIO_QCN_CRQ_PULL, &ret);
	}

	sdio_release_host(sdio_ctxt->func);
	if (ret)
		return ret;

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

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_set_block_size(sdio_ctxt->func,
				  cinfo->cli_handle.block_size);

	if (ret) {
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}

	data = SDIO_QCN_CONFIG_QE_MASK;

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
	return ret;
}


int qcn_sw_mode_change(enum qcn_sdio_sw_mode mode)
{
	struct qcn_sdio_client_info *cinfo = NULL;
	struct qcn_sdio_ch_info *chinfo = NULL;
	int ret = 0;

	if (!(mode) && !(mode < QCN_SDIO_SW_MAX))
		return -EINVAL;

	pr_info("%s: curr_sw_mode %d new mode %d\n",
		__func__, sdio_ctxt->curr_sw_mode, mode);
	if (sdio_ctxt->curr_sw_mode == mode)
		return 0;

	if (mode == QCN_SDIO_SW_RDDM) {
		if (current_host && current_host->ios.clock &&
		    current_host->ios.clock > 100000000) {
			pr_info("Try to reduce the frequency\n");
			qcn_set_host_clock(50000000);
		}
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
			if (((cinfo->cli_handle.id == QCN_SDIO_CLI_ID_WLAN) ||
			     (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_QMI) ||
			     (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_TTY) ||
			     (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_DIAG)) &&
			     (mode == QCN_SDIO_SW_MROM)) {
				qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT,
						(u32)(mode | QCN_SDIO_MAJOR_VER
						| QCN_SDIO_MINOR_VER));
				cinfo->cli_handle.block_size =
							QCN_SDIO_MROM_BLK_SZ;
				cinfo->cli_handle.func = sdio_ctxt->func;
				qcn_sdio_config(cinfo);
				cinfo->is_probed = !cinfo->cli_data.probe(
							&cinfo->cli_handle);
				qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT,
									(u32)0);
				pr_err("Calling client probe\n");
			}
		}
		mutex_unlock(&lock);
		break;
	case QCN_SDIO_SW_RESET:
		ret = wait_for_completion_timeout(&client_probe_complete,
							msecs_to_jiffies(3000));
		if (!ret)
			pr_err("Timeout waiting for clients\n");
		fallthrough;
	case QCN_SDIO_SW_MROM:
		mutex_lock(&lock);
		list_for_each_entry(cinfo, &cinfo_head, cli_list) {
			while (!list_empty(&cinfo->ch_head)) {
				chinfo = list_first_entry(&cinfo->ch_head,
					      struct qcn_sdio_ch_info, ch_list);
				if (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_TTY)
					sdio_al_deregister_channel(&chinfo->ch_handle);
			}
			cinfo->cli_handle.func = NULL;


			if (cinfo->is_probed) {
				if (cinfo->cli_handle.id == QCN_SDIO_CLI_ID_TTY) {
					cinfo->cli_data.remove(&cinfo->cli_handle);
					cinfo->is_probed = 0;
				}
			}

			if ((cinfo->cli_handle.id == QCN_SDIO_CLI_ID_TTY) &&
						   (mode <= QCN_SDIO_SW_MROM)) {
				qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT,
						(u32)(mode | QCN_SDIO_MAJOR_VER
						| QCN_SDIO_MINOR_VER));
				cinfo->cli_handle.block_size =
							QCN_SDIO_TTY_BLK_SZ;
				cinfo->cli_handle.func = sdio_ctxt->func;
				qcn_sdio_config(cinfo);
				cinfo->is_probed = !cinfo->cli_data.probe(
							&cinfo->cli_handle);
				qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT,
									(u32)0);
				break;
			}
		}
		mutex_unlock(&lock);
		break;
	default:
		pr_err("Invalid mode\n");
	}

	driver_state = mode;
	sdio_ctxt->curr_sw_mode = mode;
	if (sdio_ctxt->curr_sw_mode == QCN_SDIO_SW_RDDM) {
		char *uevent[2];
		uevent[0] = envp[QCN_SDIO_SW_RDDM];
		uevent[1] = NULL;
		kobject_uevent_env(&sdio_ctxt->func->dev.kobj, KOBJ_CHANGE, uevent);
	}
	return 0;
}

static int qcn_read_meta_info(void)
{
	int ret = 0;
	u32 i = 0;
	u32 data = 0;
	u32 temp = 0;

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
	qcn_sdio_card_state(false);
	qcn_sdio_card_state(true);
	kthread_stop(reset_task);
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

#ifdef OOB_WAKEUP

#define WAKE_IRQ_NAME "oob-wake"

static irqreturn_t qcn_sdio_wake_irq_handler(int irq, void *func)
{
	pr_info("%s: wake IRQ %d triggered\n", __func__, irq);

	disable_irq_nosync(sdio_ctxt->wake_irq_nr);

	/* TODO - IRQ service */

	return IRQ_HANDLED;
}

static int qcn_sdio_wake_irq_init(struct sdio_func *func)
{
	struct device *sdio_dev = &func->dev;
	int ret = 0;

	if (sdio_dev->of_node) {
		sdio_ctxt->wake_irq_nr = of_irq_get_byname(sdio_dev->of_node, WAKE_IRQ_NAME);
		if (!VALID_IRQ(sdio_ctxt->wake_irq_nr)) {
			dev_err(sdio_dev, "Failed to get IRQ %s\n", WAKE_IRQ_NAME);
			ret = -ENODEV;
		} else {
			dev_info(sdio_dev, "wake IRQ: number %d\n", sdio_ctxt->wake_irq_nr);
			sdio_ctxt->wake_irq_flag =
				irq_get_trigger_type(sdio_ctxt->wake_irq_nr);
			if (!sdio_ctxt->wake_irq_flag) {
				/* Fall back to default, if not provided */
				sdio_ctxt->wake_irq_flag = IRQF_TRIGGER_LOW;
			}
			sdio_ctxt->wake_irq_flag |= IRQF_ONESHOT;
			dev_info(sdio_dev, "wake IRQ: %s level trigger\n",
				 sdio_ctxt->wake_irq_flag & IRQF_TRIGGER_HIGH ?
				 "high" : "low");

			ret = devm_request_threaded_irq(sdio_dev, sdio_ctxt->wake_irq_nr,
							NULL,
							qcn_sdio_wake_irq_handler,
							sdio_ctxt->wake_irq_flag,
							WAKE_IRQ_NAME,
							func);
			if (ret) {
				dev_err(sdio_dev, "Failed to request IRQ: %d\n", ret);
				return ret;
			}

			ret = enable_irq_wake(sdio_ctxt->wake_irq_nr);
			if (ret) {
				dev_err(sdio_dev, "Failed to enable_irq_wake %d\n", ret);
				return ret;
			}
			disable_irq_wake(sdio_ctxt->wake_irq_nr);

			device_init_wakeup(sdio_dev, true);
		}
	} else {
		dev_err(sdio_dev, "of_node not found!\n");
		ret = -ENODEV;
	}

	return ret;
}

static void qcn_sdio_wake_irq_deinit(struct sdio_func *func)
{
	if (VALID_IRQ(sdio_ctxt->wake_irq_nr))
		device_init_wakeup(&func->dev, false);
}
#else
static int qcn_sdio_wake_irq_init(struct sdio_func *func)
{
	return 0;
}
static void qcn_sdio_wake_irq_deinit(struct sdio_func *func)
{
}
#endif

static void qcn_set_host_clock(unsigned int hz)
{
	if (current_host->ios.clock <= hz)
		return;

	pr_info("%s: %u hz", __func__, hz);
	sdio_claim_host(sdio_ctxt->func);
	current_host->ios.clock = hz;
	if (current_host->ops->set_ios)
		current_host->ops->set_ios(current_host, &current_host->ios);
	sdio_release_host(sdio_ctxt->func);
}

static void qcn_sdio_irq_handler(struct sdio_func *func)
{
	u8 data = 0;
	int ret = 0;

	sdio_claim_host(sdio_ctxt->func);
	data = sdio_readb(sdio_ctxt->func, SDIO_QCN_IRQ_STATUS, &ret);
	if (ret) {
		sdio_release_host(sdio_ctxt->func);

		pr_err("%s: IRQ status read error ret = %d\n", __func__, ret);

		if (current_host && current_host->ios.clock &&
		    current_host->ios.clock > 100000000) {
			pr_info("Try to reduce the frequency\n");
			qcn_set_host_clock(50000000);
			return;
		}

		ret = qcn_sdio_reset();
		if (ret)
			pr_err("Failed to run qcn_sdio_reset thread\n");

		return;
	}
	sdio_release_host(sdio_ctxt->func);

	if (data & SDIO_QCN_IRQ_CRQ_READY_MASK) {
		qcn_read_crq_info();
	} else if (data & SDIO_QCN_IRQ_LOCAL_MASK) {
		qcn_read_meta_info();
	} else if (data & SDIO_QCN_IRQ_EN_SYS_ERR_MASK) {
		sdio_claim_host(sdio_ctxt->func);
		sdio_writeb(sdio_ctxt->func, (u8)SDIO_QCN_IRQ_CLR_SYS_ERR_MASK,
				SDIO_QCN_IRQ_CLR, NULL);
		sdio_release_host(sdio_ctxt->func);
		pr_err("%s: sys_err interrupt triggered\n", __func__);
	} else if (data & SDIO_QCN_IRQ_EN_UNDERFLOW_MASK) {
		sdio_claim_host(sdio_ctxt->func);
		sdio_writeb(sdio_ctxt->func,
					(u8)SDIO_QCN_IRQ_CLR_UNDERFLOW_MASK,
					SDIO_QCN_IRQ_CLR, NULL);
		sdio_release_host(sdio_ctxt->func);
		pr_err("%s: underflow interrupt triggered\n", __func__);
	} else if (data & SDIO_QCN_IRQ_EN_OVERFLOW_MASK) {
		sdio_claim_host(sdio_ctxt->func);
		sdio_writeb(sdio_ctxt->func, (u8)SDIO_QCN_IRQ_CLR_OVERFLOW_MASK,
				SDIO_QCN_IRQ_CLR, NULL);
		sdio_release_host(sdio_ctxt->func);
		pr_err("%s: overflow interrupt triggered\n", __func__);
	} else if (data & SDIO_QCN_IRQ_EN_CH_MISMATCH_MASK) {
		sdio_claim_host(sdio_ctxt->func);
		sdio_writeb(sdio_ctxt->func,
					(u8)SDIO_QCN_IRQ_CLR_CH_MISMATCH_MASK,
					SDIO_QCN_IRQ_CLR, NULL);
		sdio_release_host(sdio_ctxt->func);
		pr_err("%s: channel mismatch interrupt triggered\n", __func__);
	} else {
		pr_err("%s: Unknown interrupt: 0x%02x\n", __func__, data);
		sdio_claim_host(sdio_ctxt->func);
		sdio_writeb(sdio_ctxt->func, (u8)data, SDIO_QCN_IRQ_CLR, NULL);
		sdio_release_host(sdio_ctxt->func);
	}
}

static int qcn_sdio_send_buff(u32 cid, void *buff, size_t len)
{
	int ret = 0;

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_writesb(sdio_ctxt->func,
			(sdio_ctxt->tx_addr_base + (cid * (u32)4)), buff, len);

	if (ret)
		qcn_send_io_abort();

	sdio_release_host(sdio_ctxt->func);

	return ret;
}

static int qcn_sdio_recv_buff(u32 cid, void *buff, size_t len)
{
	int ret = 0;

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_readsb(sdio_ctxt->func, buff,
			(sdio_ctxt->rx_addr_base + (cid * (u32)4)), len);

	if (ret)
		qcn_send_io_abort();

	sdio_release_host(sdio_ctxt->func);

	return ret;
}

static void qcn_sdio_rw_work(struct work_struct *work)
{
	int ret = 0;
	struct qcn_sdio_rw_info *rw_req = NULL;
	struct sdio_al_xfer_result *result = NULL;
	struct sdio_al_channel_handle *ch_handle = NULL;

	while (1) {
		spin_lock(&sdio_ctxt->lock_wait_q);
		if (list_empty(&sdio_ctxt->rw_wait_q)) {
			spin_unlock(&sdio_ctxt->lock_wait_q);
			break;
		}
		rw_req = list_first_entry(&sdio_ctxt->rw_wait_q,
						struct qcn_sdio_rw_info, list);
		list_del(&rw_req->list);
		spin_unlock(&sdio_ctxt->lock_wait_q);

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

		ch_handle = &sdio_ctxt->ch[rw_req->cid]->ch_handle;
		result = &sdio_ctxt->ch[rw_req->cid]->result;
		result->xfer_status = ret;
		result->buf_addr = rw_req->buf;
		result->xfer_len = rw_req->len;
		if (rw_req->dir)
			sdio_ctxt->ch[rw_req->cid]->ch_data.dl_xfer_cb(
					ch_handle, result, rw_req->ctxt);
		else
			sdio_ctxt->ch[rw_req->cid]->ch_data.ul_xfer_cb(
					ch_handle, result, rw_req->ctxt);
		atomic_set(&sdio_ctxt->ch_status[rw_req->cid], 0);
		qcn_sdio_free_rw_req(rw_req);
		atomic_dec(&sdio_ctxt->wait_list_count);
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

static int qcn_sdio_suspend(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	u8 value = 0;
	int ret = 0, suspended;

	if (sdio_ctxt->low_power_disabled) {
		pr_err("Low power has been disabled\n");
		return 0;
	}

	if (sdio_ctxt->wake_irq_pending) {
		dev_err(dev, "wake IRQ pending\n");
		return -EBUSY;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 0, 1);
	if (suspended) {
		pr_info("Already suspended\n");
		return 0;
	}

	dev_info(dev, "Notify client to suspend");
	if ((ret = qcn_sdio_lpm_notify_client(LPM_ENTER)) != 0)
		dev_err(dev, "Client failed to suspend: %d", ret);

	pr_info("%s: func %d curr_sw_mode=%d\n", __func__,
		func->num, sdio_ctxt->curr_sw_mode);

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

	if (VALID_IRQ(sdio_ctxt->wake_irq_nr)) {
		dev_info(dev, "enable_irq_wake");
		enable_irq_wake(sdio_ctxt->wake_irq_nr);
		sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
	}

out:
	sdio_release_host(func);

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

	if (VALID_IRQ(sdio_ctxt->wake_irq_nr)) {
		dev_info(dev, "disable_irq_wake\n");
		disable_irq_wake(sdio_ctxt->wake_irq_nr);
		sdio_ctxt->wake_irq_pending = 0;
	}

	suspended = atomic_cmpxchg(&sdio_ctxt->suspended, 1, 0);
	if (!suspended) {
		pr_info("Already resumed\n");
		return 0;
	}

	qcn_send_meta_info(QCN_SDIO_DOORBELL_HEVENT, (u32)0);

	dev_info(dev, "Notify client to resume");
	if ((ret = qcn_sdio_lpm_notify_client(LPM_EXIT)) != 0)
		dev_err(dev, "Client failed to resume: %d", ret);

	dev_info(dev, "resume exit\n");
	return ret;
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

	sdio_claim_host(func);
	sdio_writel(func, value, SDIO_QCN_HRQ_PUSH, &ret);
	sdio_release_host(func);

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

	ret = qcn_sdio_wake_irq_init(func);
	if (ret)
		goto err;

	sdio_ctxt->qcn_sdio_wq = create_singlethread_workqueue("qcn_sdio");
	if (!sdio_ctxt->qcn_sdio_wq) {
		pr_err("%s: Error: SDIO create wq\n", __func__);
		goto err;
	}

	for (ret = 0; ret < QCN_SDIO_CH_MAX; ret++) {
		sdio_ctxt->ch[ret] = NULL;
		atomic_set(&sdio_ctxt->ch_status[ret], -1);
	}

	spin_lock_init(&sdio_ctxt->lock_free_q);
	spin_lock_init(&sdio_ctxt->lock_wait_q);
	spin_lock_init(&async_lock);
	INIT_WORK(&sdio_ctxt->sdio_rw_w, qcn_sdio_rw_work);
	INIT_LIST_HEAD(&sdio_ctxt->rw_free_q);
	INIT_LIST_HEAD(&sdio_ctxt->rw_wait_q);

	for (ret = 0; ret < QCN_SDIO_RW_REQ_MAX; ret++)
		qcn_sdio_free_rw_req(&sdio_ctxt->rw_req_info[ret]);

	sdio_claim_host(sdio_ctxt->func);
	ret = sdio_enable_func(sdio_ctxt->func);
	if (ret) {
		pr_err("%s: Error:%d SDIO enable func\n", __func__, ret);
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}
	ret = sdio_claim_irq(sdio_ctxt->func, qcn_sdio_irq_handler);
	if (ret) {
		pr_err("%s: Error:%d SDIO claim irq\n", __func__, ret);
		sdio_release_host(sdio_ctxt->func);
		goto err;
	}

	qcn_enable_async_irq(true);
	sdio_release_host(sdio_ctxt->func);

	if (qcn_read_meta_info()) {
		pr_err("%s: Error: SDIO Config\n", __func__);
		qcn_send_meta_info((u8)QCN_SDIO_SW_MODE_HEVENT, (u32)0);
	}

	current_host = func->card->host;

	if (!retune) {
		pr_info("%s Probing driver with retune disabled\n", __func__);
		mmc_retune_disable(current_host);
	}

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

	qcn_sdio_debugfs_destroy(sdio_ctxt);
	atomic_set(&xport_status, 0);

#ifndef CONFIG_NAPIER_X86
	sdio_claim_host(sdio_ctxt->func);
	qcn_enable_async_irq(false);
	sdio_release_host(sdio_ctxt->func);
#endif

	qcn_sdio_purge_rw_buff();

	destroy_workqueue(sdio_ctxt->qcn_sdio_wq);
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

	sdio_claim_host(sdio_ctxt->func);
	sdio_release_irq(sdio_ctxt->func);
	sdio_release_host(sdio_ctxt->func);

	qcn_sdio_wake_irq_deinit(func);

	kfree(sdio_ctxt);
	sdio_ctxt = NULL;
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
		destroy_workqueue(sdio_ctxt->qcn_sdio_wq);
		sdio_claim_host(sdio_ctxt->func);
		sdio_release_irq(sdio_ctxt->func);
		sdio_release_host(sdio_ctxt->func);
		kfree(sdio_ctxt);
		sdio_ctxt = NULL;
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

	if (dir == SDIO_AL_RX)
		spin_lock(&async_lock);

	qcn_sdio_add_rw_req(rw_req);
	queue_work(sdio_ctxt->qcn_sdio_wq, &sdio_ctxt->sdio_rw_w);

	if (dir == SDIO_AL_RX)
		spin_unlock(&async_lock);

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
	} else if (atomic_read(&xport_status)) {
		mmc_remove_host(current_host);
		pr_err("QCN: Removed mmc host");
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
