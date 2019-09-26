/* Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
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

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kmemleak.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/ratelimit.h>
#include <linux/platform_device.h>
#ifdef USB_QCOM_DIAG_BRIDGE
#include <linux/smux.h>
#endif
#include "diag_mux.h"
#include "diagfwd_bridge.h"

#ifdef CONFIG_DIAG_MHI
#include "diagfwd_mhi.h"
#endif
#ifdef CONFIG_DIAG_HSIC
#include "diagfwd_hsic.h"
#endif
#ifdef CONFIG_DIAG_SDIO
#include "diagfwd_sdio.h"
#endif
#include "diag_nl.h"

#define BRIDGE_TO_MUX(x)	(x + DIAG_MUX_BRIDGE_BASE)

struct diagfwd_bridge_info bridge_info[NUM_REMOTE_DEV] = {
	{
		.id = DIAGFWD_MDM,
		.type = DIAG_DATA_TYPE,
		.name = "MDM",
		.inited = 0,
		.ctxt = 0,
		.dev_ops = NULL,
		.dci_read_ptr = NULL,
		.dci_read_buf = NULL,
		.dci_read_len = 0,
		.dci_wq = NULL,
	},
};

int diagfwd_bridge_register(int id, int ctxt, struct diag_remote_dev_ops *ops)
{
	struct diagfwd_bridge_info *ch = NULL;

	if (!ops) {
		pr_err("diag: Invalid pointers ops: %pK ctxt: %d\n", ops, ctxt);
		return -EINVAL;
	}

	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;

	ch = &bridge_info[id];
	ch->ctxt = ctxt;
	ch->dev_ops = ops;
	
	switch (ch->type) {
	case DIAG_DATA_TYPE:
		break;
	default:
		pr_err("diag: Invalid channel type %d in %s\n", ch->type,
		       __func__);
		return -EINVAL;
	}
	return 0;
}


int diagfwd_bridge_init()
{
	int err = 0;

	/* Create NL srv */
	nl_srv_create();

#ifdef CONFIG_DIAG_MHI
	err = diag_mhi_init();
#endif
#ifdef CONFIG_DIAG_HSIC
	err = diag_hsic_init();
#endif
#ifdef CONFIG_DIAG_SDIO
       err = diag_sdio_init();
#endif

	if (err)
		goto fail;
	return 0;

fail:
	pr_err("diag: Unable to initialze diagfwd bridge, err: %d\n", err);
	return err;
}


int diagfwd_bridge_write(int id, unsigned char *buf, int len)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	if (bridge_info[id].dev_ops && bridge_info[id].dev_ops->write) {
		return bridge_info[id].dev_ops->write(bridge_info[id].ctxt,
						      buf, len, 0);
	}
	return 0;
}


void diagfwd_bridge_exit()
{
#ifdef CONFIG_DIAG_MHI
	diag_mhi_exit();
#endif
#ifdef CONFIG_DIAG_HSIC
	diag_hsic_exit();
#endif
#ifdef CONFIG_DIAG_SDIO
       diag_sdio_exit();
#endif

	/* Destroy NL srv */
	nl_srv_destroy();
}

int diag_remote_dev_read_done(int id, unsigned char *buf, int len)
{
	int err = 0;
	struct diagfwd_bridge_info *ch = NULL;

	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;

	/* send to net link */
	send_to_diag_app(id, buf, len);

	ch = &bridge_info[id];
	if (ch->type == DIAG_DATA_TYPE) {
		/* process the incoming buffer, forwar to netlink, do nothing here  */
	 /*	err = diag_local_rx_process(id, buf, len, id); */
		/* free back receiving buffer, queue free to MHI  */
		if (ch->dev_ops && ch->dev_ops->queue_read)
			ch->dev_ops->fwd_complete(id, buf, len, ch->ctxt);
		return err;
	}

	return 0;
}

int diag_remote_dev_write_done(int id, unsigned char *buf, int len, int ctxt)
{
	int err = 0;
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;

	if (bridge_info[id].type == DIAG_DATA_TYPE) {
		if (buf == driver->hdlc_encode_buf)
			driver->hdlc_encode_buf_len = 0;
		
		err = diag_local_send_done(id);
	} 

	return err;
}

int diag_remote_dev_open(int id)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	bridge_info[id].inited = 1;
	
	return 0;
}

void diag_remote_dev_close(int id)
{
	return;
}
