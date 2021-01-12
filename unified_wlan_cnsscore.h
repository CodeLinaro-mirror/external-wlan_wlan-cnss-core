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

#ifndef __UNIFIED_PLATFORM_DRIVER__
#define __UNIFIED_PLATFORM_DRIVER__

int ksb_init(void);
void ksb_exit(void);
int qmi_interface_init(void);
int diag_bridge_init(void);
void diag_bridge_exit(void);
int msm_ipc_router_init(void);
int msm_ipc_router_hsic_xprt_init(void);
int msm_ipc_router_sdio_xprt_init(void);
int ipc_router_mhi_xprt_init(void);
int diagchar_init(void);
void diagchar_exit(void);
int cnss_initialize(void);
void cnss_exit(void);
int mhi_init(void);
int qcn_sdio_init(void);
int qti_bridge_init(void);
void mhi_exit(void);
int cnss_utils_init(void);
void cnss_utils_exit(void);
void qti_bridge_exit(void);
void qcn_sdio_exit(void);
void qmi_interface_deinit(void);
void msm_ipc_router_hsic_xprt_deinit(void);
void ipc_router_mhi_xprt_deinit(void);
void msm_ipc_router_deinit(void);

#ifdef CONFIG_SINGLE_KO_FEATURE
int hdd_module_init(void);
void hdd_module_exit(void);
#endif

int wcnss_pre_alloc_init(void);
void wcnss_pre_alloc_exit(void);

#ifdef CONFIG_USE_CUSTOMIZED_DMA_MEM
void *cnss_get_plat_dev(void);
#endif

#endif
