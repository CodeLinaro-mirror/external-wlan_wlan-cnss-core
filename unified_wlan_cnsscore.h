/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __UNIFIED_PLATFORM_DRIVER__
#define __UNIFIED_PLATFORM_DRIVER__

#ifdef CONFIG_WLAN_CNSS_CORE
int mhi_init(void);
void mhi_exit(void);
int qrtr_proto_init(void);
void qrtr_proto_fini(void);
int qrtr_mhi_init(void);
void qrtr_mhi_deinit(void);
int wlfw_init(void);
void wlfw_deinit(void);
int cnss_utils_init(void);
void cnss_utils_exit(void);
int cnss_prealloc_init(void);
void cnss_prealloc_exit(void);
int cnss_initialize(void);
void cnss_exit(void);
#endif /* CONFIG_WLAN_CNSS_CORE */
#endif /* __UNIFIED_PLATFORM_DRIVER__ */
