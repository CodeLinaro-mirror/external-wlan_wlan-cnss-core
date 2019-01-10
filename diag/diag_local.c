/* Copyright (c) 2014-2017, The Linux Foundation. All rights reserved.
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
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/ratelimit.h>


#include "diagchar.h"
#include "diagfwd.h"
#include "diag_mux.h"
#include "diag_ipc_logging.h"
#include "diagmem.h"
#include "diag_masks.h"

#define MSG_SSID_WLAN       4500
#define MSG_SSID_WLAN_LAST  4583
#define MASK_LOW_LEVEL      0x1F



int diag_local_send_done(int proc)
{
   /* process send done completion */
   /* free write buffuer to pool?  */
    DIAG_LOG(DIAG_DEBUG_BRIDGE,"diag: %s enter.\n", __func__);
    return 0;
}


/* send local data */
static int diag_local_write(void *buf, int len)
{
	int err = 0;
	uint8_t retry_count = 0;
	uint8_t max_retries = 3;

	if (!buf)
		return -EINVAL;

	if (len <= 0) {
		pr_err("diag: In %s, invalid len: %d", __func__, len);
		return -EBADMSG;
	}


	do {
		if (driver->hdlc_encode_buf_len == 0)
			break;
		usleep_range(10000, 10100);
		retry_count++;
	} while (retry_count < max_retries);

	if (driver->hdlc_encode_buf_len != 0)
		return -EAGAIN;


        if (DIAG_MAX_HDLC_BUF_SIZE < len) {
	        pr_err("diag: Dropping packet, HDLC encoded packet payload size crosses buffer limit. Current payload size %d\n",
		      len);
		return -EBADMSG;
	}

	driver->hdlc_encode_buf_len = len;
	memcpy(driver->hdlc_encode_buf, buf, len);
	
	err = diagfwd_bridge_write(DIAGFWD_MDM, driver->hdlc_encode_buf,
				   driver->hdlc_encode_buf_len);
	if (err) {
		pr_err_ratelimited("diag: Error writing packet to bridge DIAGFWD_MDM, err: %d\n",
				    err);
		driver->hdlc_encode_buf_len = 0;
	}

	return err;
}

#if 0
int diag_local_enable_log(void)
{
/* try to create msg enable cmd and call diag_local_write to send */

   struct diag_msg_build_mask_t *req = NULL;
   int req_len = 0;
   int ret = 0;
   int range = MSG_SSID_WLAN_LAST - MSG_SSID_WLAN + 1;
   int i = 0;
   uint32_t *mask_ptr = NULL;

   req = diagmem_alloc(driver,DIAG_MAX_REQ_SIZE,POOL_TYPE_COPY);
   if(!req)
   	return -EINVAL;

    req_len = sizeof(struct diag_msg_build_mask_t) + (range * sizeof (uint32_t));
    mask_ptr = (uint32_t *) (req + 1);
   

   req->cmd_code   = DIAG_CMD_MSG_CONFIG;
   req->sub_cmd    = DIAG_CMD_OP_SET_MSG_MASK;
   req->ssid_first = MSG_SSID_WLAN;
   req->ssid_last   = MSG_SSID_WLAN_LAST;
   req->padding = 0;
   req->status = 0;

   for(i = 0; i < range; i ++)
   	mask_ptr[i] = MASK_LOW_LEVEL;


   /* start to send */
   ret = diag_local_write(req, req_len, 0);

   diagmem_free(driver, req, POOL_TYPE_COPY);
   
   return ret;
}
#endif

int diag_local_cmd_handler(void *buf)
{
   int ret = 0;
   struct dbglog_slot *slot = (struct dbglog_slot *)buf;
   
	switch (slot->diag_type) {
	case DIAG_TYPE_FW_MSG: /* cmd to onfigure */
		if (slot->length <= 0) {
			pr_err("%s: invliad cmd len \n", __func__);
			return -1;
		}

		printk("%s: diag_type_fw_msg cmd len is 0x%x.\n",
			   __func__, slot->length);

		ret = diag_local_write(slot->payload, slot->length); /* has done hdlc encode in user app */
		break;
	default:
		pr_err("Unknown cmd[%d] error\n",
						slot->diag_type);
		break;
	}

   return ret;
}




