/* Copyright (c) 2011-2019, The Linux Foundation. All rights reserved.
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
#include <linux/init.h>

#ifdef CONFIG_WLAN_CNSS_CORE

#include "unified_wlan_cnsscore.h"

static int unified_pdrv_init(void)
{
	int ret;

	/* mhi Registration */
#ifdef CONFIG_MSM_MHI
	ret = mhi_init();
	if (ret){
		printk("%s: updrv: failed to register ks_bridge\n",__func__);
		goto fail;
	}
#endif
#ifdef CONFIG_USB_QTI_KS_BRIDGE
	/* ks_brige Registration */
	ret = ksb_init();
	if (ret){
		printk("%s: updrv: failed to register ks_bridge\n",__func__);
		goto fail1;
	}
#endif
#ifdef CONFIG_QCN
	ret = qcn_sdio_init();
	if (ret){
                printk("%s: updrv: failed to register qcn_sdio\n",__func__);
                goto fail2;
        }

#endif
#ifdef CONFIG_QTI_SDIO_CLIENT
	ret = qti_bridge_init();
	if (ret){
                printk("%s: updrv: failed to register qti_bridge\n",__func__);
                goto fail3;
        }

#endif
	/* ipc_router Registration */
	ret = msm_ipc_router_init();
	if (ret){
		printk("%s: updrv: failed to register ipc_router\n",__func__);
		goto fail4;
	}
	/* QMI Registration */
	ret = qmi_interface_init();
	if (ret){
		printk("%s: updrv: failed to register qmi\n",__func__);
		goto fail5;
	}
	/* ipc_brigde Registration */
#ifdef CONFIG_DIAG_IPC_BRIDGE
	ret = diag_bridge_init();
	if (ret){
		printk("%s: updrv: failed to register ipc_bridge\n",__func__);
		goto fail6;
	}
#endif
	/* ipc_router_mhi_xprt Registration */
#ifdef CONFIG_MHI_XPRT
	ret = ipc_router_mhi_xprt_init();
	if (ret){
		printk("%s: updrv: failed to register ipc_router_mhi_xprt (ipc_xprt)\n",__func__);
		goto fail7;
	}
#endif
	/* ipc_router_hsic_xprt Registration */
#ifdef CONFIG_HSIC_XPRT
	ret = msm_ipc_router_hsic_xprt_init();
	if (ret){
		printk("%s: updrv: failed to register ipc_router_hsic_xprt\n",__func__);
		goto fail8;
	}
#endif
#ifdef CONFIG_SDIO_XPRT
	ret = msm_ipc_router_sdio_xprt_init();
	if (ret){
                printk("%s: updrv: failed to register ipc_router_sdio_xprt\n",__func__);
                goto fail9;
        }

#endif
	/* cnss Registration */
	ret = cnss_initialize();
	if (ret){
		printk("%s: updrv: failed to register cnss\n",__func__);
		goto fail10;
	}

#ifdef CONFIG_MSM_DIAG_INTERFACE
	/* diag Registration */
	ret = diagchar_init();
	if (ret){
		printk("%s: updrv: failed to register diag\n",__func__);
		goto fail11;
	}
#endif

#ifdef CONFIG_CNSS_UTILS
	/* cnss utils Registration */
	ret = cnss_utils_init();
	if (ret){
		printk("%s: updrv: failed to register diag\n",__func__);
		goto fail12;
	}
#endif

#ifdef CONFIG_SINGLE_KO_FEATURE
	ret = hdd_module_init();
	if (ret) {
		printk("%s: updrv: failed to register hdd module\n",__func__);
		goto fail13;
	}
#endif

#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
	/* cnss prealloc initialise */
	ret = wcnss_pre_alloc_init();
	if (ret){
		printk("%s: updrv: failed to pre alloc memory\n",__func__);
		goto fail14;
	}
#endif
	return 0;

#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
fail14:
#endif
#ifdef CONFIG_SINGLE_KO_FEATURE
	hdd_module_exit();
#endif
#ifdef CONFIG_SINGLE_KO_FEATURE
fail13:
#ifdef CONFIG_CNSS_UTILS
	cnss_utils_exit();
#endif
#endif

#ifdef CONFIG_CNSS_UTILS
fail12:
#ifdef CONFIG_MSM_DIAG_INTERFACE
	diagchar_exit();
#endif
#endif

#ifdef CONFIG_MSM_DIAG_INTERFACE
fail11:
#endif
	cnss_exit();
fail10:
#ifdef CONFIG_SDIO_XPRT
fail9:
#endif
#ifdef CONFIG_HSIC_XPRT
	msm_ipc_router_hsic_xprt_deinit();
fail8:
#endif
#ifdef CONFIG_MHI_XPRT
	ipc_router_mhi_xprt_deinit();
#endif
#ifdef CONFIG_MHI_XPRT
fail7:
#endif
#ifdef CONFIG_DIAG_IPC_BRIDGE
	diag_bridge_exit(); /* ipc_bridge  */
fail6:
#endif
	qmi_interface_deinit();
#ifdef CONFIG_DIAG_IPC_BRIDGE	
	diag_bridge_exit();
#endif
fail5:
	msm_ipc_router_deinit();
fail4:
#ifdef CONFIG_QTI_SDIO_CLIENT
	qti_bridge_exit();
fail3:
#endif
#ifdef CONFIG_QCN
	qcn_sdio_exit();
fail2:
#endif
#ifdef CONFIG_USB_QTI_KS_BRIDGE
	ksb_exit();
fail1:
#endif
#ifdef CONFIG_MSM_MHI
	mhi_exit();
fail:
#endif
	return ret;
}

static void unified_pdrv_deinit(void)
{
#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
	wcnss_pre_alloc_exit();
#endif
#ifdef CONFIG_SINGLE_KO_FEATURE
	hdd_module_exit();
#endif

#ifdef CONFIG_CNSS_UTILS
	cnss_utils_exit();
#endif
#ifdef CONFIG_MSM_DIAG_INTERFACE
	diagchar_exit();
#endif
	cnss_exit();
#ifdef CONFIG_DIAG_IPC_BRIDGE
	diag_bridge_exit(); /* ipc_bridge  */
#endif
#ifdef CONFIG_HSIC_XPRT
	msm_ipc_router_hsic_xprt_deinit();
#endif
#ifdef CONFIG_MHI_XPRT
	ipc_router_mhi_xprt_deinit();
#endif
	qmi_interface_deinit();
	msm_ipc_router_deinit();
#ifdef CONFIG_USB_QTI_KS_BRIDGE
	ksb_exit();
#endif
#ifdef CONFIG_MSM_MHI
	mhi_exit();
#endif
}

module_init(unified_pdrv_init);
module_exit(unified_pdrv_deinit);
MODULE_DESCRIPTION("Unified Platform Driver");
MODULE_LICENSE("GPL v2");
#endif
