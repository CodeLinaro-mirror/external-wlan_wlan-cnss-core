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
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <cnss2/main.h>
#include <cnss2/debug.h>
#include "cnss_module.h"
#include <cnss_prealloc/cnss_prealloc.h>
#ifdef CONFIG_WLAN_CNSS_CORE

#include "unified_wlan_cnsscore.h"

#define QCA6390_DEVICE_ID		0x1101

#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
static struct platform_device *s_plat_dev = NULL;

void *cnss_get_plat_dev(void)
{
	return s_plat_dev;
}

void *cnss_dma_alloc_coherent(struct device *dev, size_t size,
			      dma_addr_t *dma_handle, gfp_t flag)
{
	void *vaddr;
	struct platform_device *plat_dev = cnss_get_plat_dev();

	if (!plat_dev) {
		pr_err("platfrom driver is not registered!\n");
		return NULL;
	}

	dev = &plat_dev->dev;
	vaddr = dma_alloc_coherent(dev, size, dma_handle, flag);
	if (!vaddr) {
		pr_err("%s, alloc failed!\n", __func__);
		return NULL;
	}
	return vaddr;

}
cnss_export_symbol(cnss_dma_alloc_coherent);

void cnss_dma_free_coherent(struct device *dev, size_t size,
			    void *vaddr, dma_addr_t dma_handle)
{
	struct platform_device *plat_dev = cnss_get_plat_dev();

	if (!plat_dev) {
		pr_err("platfrom driver is not registered!\n");
		return;
	}

	dev = &plat_dev->dev;
	dma_free_coherent(dev, size, vaddr, dma_handle);

	return;
}
cnss_export_symbol(cnss_dma_free_coherent);

#else
void *cnss_dma_alloc_coherent(struct device *dev, size_t size,
			      dma_addr_t *dma_handle, gfp_t flag)
{
	void *vaddr;

#ifdef CONFIG_WCNSS_DMA_PRE_ALLOC
	vaddr = wcnss_dma_prealloc_get(size, dma_handle);
	if (vaddr)
		return vaddr;
#endif
	vaddr = dma_alloc_coherent(dev, size, dma_handle, flag);

#ifdef CONFIG_WCNSS_DMA_PRE_ALLOC
	wcnss_dma_prealloc_save(dev, size, vaddr, *dma_handle);
#endif
	return vaddr;
}
cnss_export_symbol(cnss_dma_alloc_coherent);

void cnss_dma_free_coherent(struct device *dev, size_t size,
			    void *vaddr, dma_addr_t dma_handle)
{
#ifdef CONFIG_WCNSS_DMA_PRE_ALLOC
	if (wcnss_dma_prealloc_put(size, vaddr, dma_handle))
		return;
#endif
	dma_free_coherent(dev, size, vaddr, dma_handle);
}
cnss_export_symbol(cnss_dma_free_coherent);
#endif

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

	/* cnss prealloc initialise */
	ret = wcnss_pre_alloc_init();
	if (ret){
		printk("%s: updrv: failed to pre alloc memory\n",__func__);
		goto fail14;
	}

	return 0;

fail14:
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

	wcnss_pre_alloc_exit();

#ifdef CONFIG_MSM_MHI
	mhi_exit();
#endif
}

static const struct platform_device_id cnss2_platform_id_table[] = {
	{ .name = "qca6390", .driver_data = QCA6390_DEVICE_ID, },
};

static const struct of_device_id cnss2_of_match_table[] = {
	{
		.compatible = "qcom,cnss2",
		.data = (void *)&cnss2_platform_id_table[0]},
	{ },
};
MODULE_DEVICE_TABLE(of, cnss2_of_match_table);

static int cnss2_probe(struct platform_device *plat_dev)
{
	int ret;
#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
	const struct of_device_id *of_id;

	printk("%s, plat_dev %p, dev %p\n",
		__func__, plat_dev, &plat_dev->dev);

	of_id = of_match_device(cnss2_of_match_table, &plat_dev->dev);
	if (!of_id || !of_id->data) {
		pr_err("Failed to find of match device!\n");
		return -ENODEV;
	}

	ret = of_reserved_mem_device_init(&plat_dev->dev);
	if (ret) {
		pr_err("%s,memory init fail:%d\n", __func__,ret);
		return -1;
	}
	s_plat_dev = plat_dev;
#endif
	ret = unified_pdrv_init();

	return ret;
}

static int cnss2_remove(struct platform_device *plat_dev)
{
	printk("%s, plat_dev %p\n",
		__func__, plat_dev);

	return 0;
}
#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
static struct platform_driver cnss2_platform_driver = {
	.probe  = cnss2_probe,
	.remove = cnss2_remove,
	.driver = {
		.name = "cnss2",
		.owner = THIS_MODULE,
		.of_match_table = cnss2_of_match_table,
	},
};
#endif
static int cnss2_module_init(void)
{
	int ret;
#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
	ret = platform_driver_register(&cnss2_platform_driver);
#else
	ret = cnss2_probe(NULL);
#endif
	if (ret)
		pr_err("register platform driver failed, ret = %d\n", ret);

	return ret;
}

static void cnss2_module_exit(void)
{
	unified_pdrv_deinit();
#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
	platform_driver_unregister(&cnss2_platform_driver);
#else
	cnss2_remove(NULL);
#endif
}

module_init(cnss2_module_init);
module_exit(cnss2_module_exit);
MODULE_DESCRIPTION("Unified Platform Driver");
MODULE_LICENSE("GPL v2");
#endif
