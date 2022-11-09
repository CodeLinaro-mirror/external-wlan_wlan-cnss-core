// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#ifdef CONFIG_WLAN_CNSS_CORE

#include "unified_wlan_cnsscore.h"

static int unified_pdrv_init(void)
{
	int ret;

#ifdef CONFIG_MHI_BUS
	/* mhi Registration */
	ret = mhi_init();
	if (ret) {
		printk("%s: updrv: failed to register mhi\n", __func__);
		goto fail;
	}
#endif

#ifdef CONFIG_QRTR
	/* qrtr Registration */
	ret = qrtr_proto_init();
	if (ret) {
		printk("%s: updrv: failed to register qrtr\n", __func__);
		goto fail1;
	}
#endif

#ifdef CONFIG_QRTR_MHI
	ret = qrtr_mhi_init();
	if (ret) {
		printk("%s: updrv: failed to register qrtr-mhi\n", __func__);
		goto fail2;
	}
#endif

#ifdef CONFIG_CNSS_QMI_SVC
	ret = wlfw_init();
	if (ret) {
		printk("%s: updrv: failed to wlfw_init\n", __func__);
		goto fail3;
	}
#endif

#ifdef CONFIG_CNSS_UTILS
	/* cnss utils Registration */
	ret = cnss_utils_init();
	if (ret) {
		printk("%s: updrv: failed to register diag\n", __func__);
		goto fail4;
	}
#endif

	/* cnss Registration */
	ret = cnss_initialize();
	if (ret) {
		printk("%s: updrv: failed to register cnss\n", __func__);
		goto fail5;
	}

	/* cnss prealloc initialise */
	ret = cnss_prealloc_init();
	if (ret) {
		printk("%s: updrv: failed to pre alloc memory\n", __func__);
		goto fail6;
	}

	return 0;

fail6:
	cnss_exit();
fail5:
#ifdef CONFIG_CNSS_UTILS
	cnss_utils_exit();
fail4:
#endif
#ifdef CONFIG_CNSS_QMI_SVC
	wlfw_deinit();
fail3:
#endif
#ifdef CONFIG_QRTR_MHI
	qrtr_mhi_deinit();
fail2:
#endif
#ifdef CONFIG_QRTR
	qrtr_proto_fini();
fail1:
#endif
#ifdef CONFIG_MHI_BUS
	mhi_exit();
fail:
#endif
	return ret;
}

static void unified_pdrv_deinit(void)
{
	cnss_prealloc_exit();
	cnss_exit();
#ifdef CONFIG_CNSS_UTILS
	cnss_utils_exit();
#endif
#ifdef CONFIG_CNSS_QMI_SVC
	wlfw_deinit();
#endif
#ifdef CONFIG_QRTR_MHI
	qrtr_mhi_deinit();
#endif
#ifdef CONFIG_QRTR
	qrtr_proto_fini();
#endif
#ifdef CONFIG_MHI_BUS
	mhi_exit();
#endif
}

module_init(unified_pdrv_init);
module_exit(unified_pdrv_deinit);
MODULE_DESCRIPTION("Unified Platform Driver");
MODULE_LICENSE("GPL v2");
#endif
