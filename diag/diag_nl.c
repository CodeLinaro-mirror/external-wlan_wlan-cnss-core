/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#include "diag_mux.h"
#include "diag_nl.h"

#include <net/sock.h>
#include <net/netlink.h>
#include <cnss_module.h>

static struct sock *srv_sock;

typedef struct sAniHdr {
	unsigned short type;
	unsigned short length;
} tAniHdr;

typedef struct sAniNlMsg {
	struct nlmsghdr nlh;
	int radio;
	tAniHdr wmsg;
} tAniNlHdr;

static int nl_srv_send_bcast(struct sk_buff *skb)
{
	int err = -1;
	int flags = GFP_KERNEL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	NETLINK_CB(skb).portid = 0;     /* sender's pid */
	NETLINK_CB(skb).dst_group = 0x01;    /* destination group */

	if (srv_sock) {
		err = netlink_broadcast(srv_sock, skb, 0, 0x01, flags);
		if ((err < 0) && (err != -ESRCH))
			dev_kfree_skb(skb);
	} else {
		dev_kfree_skb(skb);
	}

	return err;
}

static int generate_nl_msg(unsigned char *buf, size_t len)
{
#define WLAN_NL_CNSS_FW_MSG 29
	struct nlmsghdr *nlh;
	tAniNlHdr *wnl;
	size_t len_ext = sizeof(wnl->radio) + sizeof(wnl->wmsg);
	struct sk_buff *fw_skb = nlmsg_new(len + len_ext, GFP_KERNEL);

	if (!fw_skb) {
		pr_err("Fail to allocate\n");
		return -EINVAL;
	}

	nlh = nlmsg_put(fw_skb, 0, 0, WLAN_NL_CNSS_FW_MSG, len + len_ext, 0);
	if (nlh) {
		wnl = (tAniNlHdr *)nlh;
		wnl->radio = 0; /* To extend later */
		wnl->wmsg.type = 0; /* To extend later */
		wnl->wmsg.length = len;

		memcpy(nlmsg_data(nlh) + len_ext, buf, len);
		nl_srv_send_bcast(fw_skb);
	} else {
		kfree_skb(fw_skb);
		pr_err("Fail to put\n");
		return -EMSGSIZE;
	}

	return 0;
#undef WLAN_NL_CNSS_FW_MSG
}

void send_to_diag_app(int dev_id, unsigned char *data, size_t len)
{
	/* Check input params */
	if (!data || len > DIAG_MAX_HDLC_BUF_SIZE)
		return;

	/* Check channel open? */

	/* Check dev_id range? */

	/* If channel type equal to DIAG_DATA_TYPE? */

	/* transfer to user */
	if (generate_nl_msg(data, len) != 0)
		pr_err("Fail to send NL msg\n");
}

static void nl_srv_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	uint8_t *msg;

	nlh = (struct nlmsghdr *)skb->data;
	if (!nlh) {
		pr_err("%s: Netlink header null\n", __func__);
		return;
	}

	msg = NLMSG_DATA(nlh);

	/* send */
	diag_local_cmd_handler(msg);
}

int nl_srv_create(void)
{
#define NETLINK_CUSTOM_FW NETLINK_CUSTOM_FW_NUM
	int retcode = 0;
	struct netlink_kernel_cfg cfg = {
		.groups = 0x01,
		.input = nl_srv_rcv
	};

	srv_sock = netlink_kernel_create(&init_net, NETLINK_CUSTOM_FW, &cfg);
	if (!srv_sock) {
		pr_err("netlink_kernel_create failed\n");
		retcode = -1;
	}

	return retcode;
#undef NETLINK_CUSTOM_FW
}

void nl_srv_destroy(void)
{
	if (srv_sock)
		netlink_kernel_release(srv_sock);

	srv_sock = NULL;
}
