/* Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __MULTI_CARD_H__
#define __MULTI_CARD_H__

#define CNSS_LOCK_PM_SEM_ALIAS_DECLARE(suffix) \
void cnss_lock_pm_sem_##suffix(struct device *dev);

#define CNSS_LOCK_PM_SEM_DECLARE(pcie_ssid) CNSS_LOCK_PM_SEM_ALIAS_DECLARE(pcie_ssid)

CNSS_LOCK_PM_SEM_DECLARE(PCIE_SSID)

#define CNSS_RELEASE_PM_SEM_ALIAS_DECLARE(suffix) \
void cnss_release_pm_sem_##suffix(struct device *dev);

#define CNSS_RELEASE_PM_SEM_DECLARE(pcie_ssid) CNSS_RELEASE_PM_SEM_ALIAS_DECLARE(pcie_ssid)

CNSS_RELEASE_PM_SEM_DECLARE(PCIE_SSID)

#define CNSS_GET_FW_FILES_FOR_TARGET_ALIAS_DECLARE(suffix) \
int cnss_get_fw_files_for_target_##suffix(struct device *dev, \
					  struct cnss_fw_files *pfw_files, \
					  u32 target_type, u32 target_version);

#define CNSS_GET_FW_FILES_FOR_TARGET_DECLARE(pcie_ssid) CNSS_GET_FW_FILES_FOR_TARGET_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_FW_FILES_FOR_TARGET_DECLARE(PCIE_SSID)

#define CNSS_REQUEST_BUS_BANDWIDTH_ALIAS_DECLARE(suffix) \
int cnss_request_bus_bandwidth_##suffix(struct device *dev, int bandwidth);

#define CNSS_REQUEST_BUS_BANDWIDTH_DECLARE(pcie_ssid) CNSS_REQUEST_BUS_BANDWIDTH_ALIAS_DECLARE(pcie_ssid)

CNSS_REQUEST_BUS_BANDWIDTH_DECLARE(PCIE_SSID)

#define CNSS_GET_PLATFORM_CAP_ALIAS_DECLARE(suffix) \
int cnss_get_platform_cap_##suffix(struct device *dev, struct cnss_platform_cap *cap);

#define CNSS_GET_PLATFORM_CAP_DECLARE(pcie_ssid) CNSS_GET_PLATFORM_CAP_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_PLATFORM_CAP_DECLARE(PCIE_SSID)

#define CNSS_REQUEST_PM_QOS_ALIAS_DECLARE(suffix) \
void cnss_request_pm_qos_##suffix(struct device *dev, u32 qos_val);

#define CNSS_REQUEST_PM_QOS_DECLARE(pcie_ssid) CNSS_REQUEST_PM_QOS_ALIAS_DECLARE(pcie_ssid)

CNSS_REQUEST_PM_QOS_DECLARE(PCIE_SSID)

#define CNSS_REMOVE_PM_QOS_ALIAS_DECLARE(suffix) \
void cnss_remove_pm_qos_##suffix(struct device *dev);

#define CNSS_REMOVE_PM_QOS_DECLARE(pcie_ssid) CNSS_REMOVE_PM_QOS_ALIAS_DECLARE(pcie_ssid)

CNSS_REMOVE_PM_QOS_DECLARE(PCIE_SSID)

#define CNSS_WLAN_ENABLE_ALIAS_DECLARE(suffix) \
int cnss_wlan_enable_##suffix(struct device *dev, \
			      struct cnss_wlan_enable_cfg *config, \
			      enum cnss_driver_mode mode, \
			      const char *host_version);

#define CNSS_WLAN_ENABLE_DECLARE(pcie_ssid) CNSS_WLAN_ENABLE_ALIAS_DECLARE(pcie_ssid)

CNSS_WLAN_ENABLE_DECLARE(PCIE_SSID)

#define CNSS_WLAN_DISABLE_ALIAS_DECLARE(suffix) \
int cnss_wlan_disable_##suffix(struct device *dev, enum cnss_driver_mode mode);

#define CNSS_WLAN_DISABLE_DECLARE(pcie_ssid) CNSS_WLAN_DISABLE_ALIAS_DECLARE(pcie_ssid)

CNSS_WLAN_DISABLE_DECLARE(PCIE_SSID)

#define CNSS_ATHDIAG_READ_ALIAS_DECLARE(suffix) \
int cnss_athdiag_read_##suffix(struct device *dev, u32 offset, u32 mem_type, \
			       u32 data_len, u8 *output);

#define CNSS_ATHDIAG_READ_DECLARE(pcie_ssid) CNSS_ATHDIAG_READ_ALIAS_DECLARE(pcie_ssid)

CNSS_ATHDIAG_READ_DECLARE(PCIE_SSID)

#define CNSS_ATHDIAG_WRITE_ALIAS_DECLARE(suffix) \
int cnss_athdiag_write_##suffix(struct device *dev, u32 offset, u32 mem_type, \
				u32 data_len, u8 *input);

#define CNSS_ATHDIAG_WRITE_DECLARE(pcie_ssid) CNSS_ATHDIAG_WRITE_ALIAS_DECLARE(pcie_ssid)

CNSS_ATHDIAG_WRITE_DECLARE(PCIE_SSID)

#define CNSS_SET_FW_LOG_MODE_ALIAS_DECLARE(suffix) \
int cnss_set_fw_log_mode_##suffix(struct device *dev, u8 fw_log_mode);

#define CNSS_SET_FW_LOG_MODE_DECLARE(pcie_ssid) CNSS_SET_FW_LOG_MODE_ALIAS_DECLARE(pcie_ssid)

CNSS_SET_FW_LOG_MODE_DECLARE(PCIE_SSID)

#define CNSS_POWER_UP_ALIAS_DECLARE(suffix) \
int cnss_power_up_##suffix(struct device *dev);

#define CNSS_POWER_UP_DECLARE(pcie_ssid) CNSS_POWER_UP_ALIAS_DECLARE(pcie_ssid)

CNSS_POWER_UP_DECLARE(PCIE_SSID)

#define CNSS_POWER_DOWN_ALIAS_DECLARE(suffix) \
int cnss_power_down_##suffix(struct device *dev);

#define CNSS_POWER_DOWN_DECLARE(pcie_ssid) CNSS_POWER_DOWN_ALIAS_DECLARE(pcie_ssid)

CNSS_POWER_DOWN_DECLARE(PCIE_SSID)

#define CNSS_IDLE_RESTART_ALIAS_DECLARE(suffix) \
int cnss_idle_restart_##suffix(struct device *dev);

#define CNSS_IDLE_RESTART_DECLARE(pcie_ssid) CNSS_IDLE_RESTART_ALIAS_DECLARE(pcie_ssid)

CNSS_IDLE_RESTART_DECLARE(PCIE_SSID)

#define CNSS_IDLE_SHUTDOWN_ALIAS_DECLARE(suffix) \
int cnss_idle_shutdown_##suffix(struct device *dev);

#define CNSS_IDLE_SHUTDOWN_DECLARE(pcie_ssid) CNSS_IDLE_SHUTDOWN_ALIAS_DECLARE(pcie_ssid)

CNSS_IDLE_SHUTDOWN_DECLARE(PCIE_SSID)

#define CNSS_GET_VIRT_RAMDUMP_MEM_ALIAS_DECLARE(suffix) \
void *cnss_get_virt_ramdump_mem_##suffix(struct device *dev, unsigned long *size);

#define CNSS_GET_VIRT_RAMDUMP_MEM_DECLARE(pcie_ssid) CNSS_GET_VIRT_RAMDUMP_MEM_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_VIRT_RAMDUMP_MEM_DECLARE(PCIE_SSID)

#define CNSS_DEVICE_CRASHED_ALIAS_DECLARE(suffix) \
void cnss_device_crashed_##suffix(struct device *dev);

#define CNSS_DEVICE_CRASHED_DECLARE(pcie_ssid) CNSS_DEVICE_CRASHED_ALIAS_DECLARE(pcie_ssid)

CNSS_DEVICE_CRASHED_DECLARE(PCIE_SSID)

#define CNSS_SELF_RECOVERY_ALIAS_DECLARE(suffix) \
int cnss_self_recovery_##suffix(struct device *dev, \
				enum cnss_recovery_reason reason);

#define CNSS_SELF_RECOVERY_DECLARE(pcie_ssid) CNSS_SELF_RECOVERY_ALIAS_DECLARE(pcie_ssid)

CNSS_SELF_RECOVERY_DECLARE(PCIE_SSID)

#define CNSS_SCHEDULE_RECOVERY_ALIAS_DECLARE(suffix) \
void cnss_schedule_recovery_##suffix(struct device *dev, \
				     enum cnss_recovery_reason reason);

#define CNSS_SCHEDULE_RECOVERY_DECLARE(pcie_ssid) CNSS_SCHEDULE_RECOVERY_ALIAS_DECLARE(pcie_ssid)

CNSS_SCHEDULE_RECOVERY_DECLARE(PCIE_SSID)

#define CNSS_FORCE_FW_ASSERT_ALIAS_DECLARE(suffix) \
int cnss_force_fw_assert_##suffix(struct device *dev);

#define CNSS_FORCE_FW_ASSERT_DECLARE(pcie_ssid) CNSS_FORCE_FW_ASSERT_ALIAS_DECLARE(pcie_ssid)

CNSS_FORCE_FW_ASSERT_DECLARE(PCIE_SSID)

#define CNSS_FORCE_COLLECT_RDDM_ALIAS_DECLARE(suffix) \
int cnss_force_collect_rddm_##suffix(struct device *dev);

#define CNSS_FORCE_COLLECT_RDDM_DECLARE(pcie_ssid) CNSS_FORCE_COLLECT_RDDM_ALIAS_DECLARE(pcie_ssid)

CNSS_FORCE_COLLECT_RDDM_DECLARE(PCIE_SSID)

#define CNSS_QMI_SEND_GET_ALIAS_DECLARE(suffix) \
int cnss_qmi_send_get_##suffix(struct device *dev);

#define CNSS_QMI_SEND_GET_DECLARE(pcie_ssid) CNSS_QMI_SEND_GET_ALIAS_DECLARE(pcie_ssid)

CNSS_QMI_SEND_GET_DECLARE(PCIE_SSID)

#define CNSS_QMI_SEND_PUT_ALIAS_DECLARE(suffix) \
int cnss_qmi_send_put_##suffix(struct device *dev);

#define CNSS_QMI_SEND_PUT_DECLARE(pcie_ssid) CNSS_QMI_SEND_PUT_ALIAS_DECLARE(pcie_ssid)

CNSS_QMI_SEND_PUT_DECLARE(PCIE_SSID)

#define CNSS_QMI_SEND_ALIAS_DECLARE(suffix) \
int cnss_qmi_send_##suffix(struct device *dev, int type, void *cmd, \
			   int cmd_len, void *cb_ctx, \
			   int (*cb)(void *ctx, void *event, int event_len));

#define CNSS_QMI_SEND_DECLARE(pcie_ssid) CNSS_QMI_SEND_ALIAS_DECLARE(pcie_ssid)

CNSS_QMI_SEND_DECLARE(PCIE_SSID)

#define CNSS_PCI_IS_DEVICE_DOWN_ALIAS_DECLARE(suffix) \
int cnss_pci_is_device_down_##suffix(struct device *dev);

#define CNSS_PCI_IS_DEVICE_DOWN_DECLARE(pcie_ssid) CNSS_PCI_IS_DEVICE_DOWN_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_IS_DEVICE_DOWN_DECLARE(PCIE_SSID)

#define CNSS_PCI_LOCK_REG_WINDOW_ALIAS_DECLARE(suffix) \
void cnss_pci_lock_reg_window_##suffix(struct device *dev, unsigned long *flags);

#define CNSS_PCI_LOCK_REG_WINDOW_DECLARE(pcie_ssid) CNSS_PCI_LOCK_REG_WINDOW_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_LOCK_REG_WINDOW_DECLARE(PCIE_SSID)

#define CNSS_PCI_UNLOCK_REG_WINDOW_ALIAS_DECLARE(suffix) \
void cnss_pci_unlock_reg_window_##suffix(struct device *dev, unsigned long *flags);

#define CNSS_PCI_UNLOCK_REG_WINDOW_DECLARE(pcie_ssid) CNSS_PCI_UNLOCK_REG_WINDOW_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_UNLOCK_REG_WINDOW_DECLARE(PCIE_SSID)

#define CNSS_PCI_PREVENT_L1_ALIAS_DECLARE(suffix) \
int cnss_pci_prevent_l1_##suffix(struct device *dev);

#define CNSS_PCI_PREVENT_L1_DECLARE(pcie_ssid) CNSS_PCI_PREVENT_L1_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_PREVENT_L1_DECLARE(PCIE_SSID)

#define CNSS_PCI_ALLOW_L1_ALIAS_DECLARE(suffix) \
void cnss_pci_allow_l1_##suffix(struct device *dev);

#define CNSS_PCI_ALLOW_L1_DECLARE(pcie_ssid) CNSS_PCI_ALLOW_L1_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_ALLOW_L1_DECLARE(PCIE_SSID)

#define CNSS_PCI_LINK_DOWN_ALIAS_DECLARE(suffix) \
int cnss_pci_link_down_##suffix(struct device *dev);

#define CNSS_PCI_LINK_DOWN_DECLARE(pcie_ssid) CNSS_PCI_LINK_DOWN_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_LINK_DOWN_DECLARE(PCIE_SSID)

#define CNSS_WLAN_REGISTER_DRIVER_ALIAS_DECLARE(suffix) \
int cnss_wlan_register_driver_##suffix(struct cnss_wlan_driver *driver_ops);

#define CNSS_WLAN_REGISTER_DRIVER_DECLARE(pcie_ssid) CNSS_WLAN_REGISTER_DRIVER_ALIAS_DECLARE(pcie_ssid)

CNSS_WLAN_REGISTER_DRIVER_DECLARE(PCIE_SSID)

#define CNSS_WLAN_UNREGISTER_DRIVER_ALIAS_DECLARE(suffix) \
void cnss_wlan_unregister_driver_##suffix(struct cnss_wlan_driver *driver_ops);

#define CNSS_WLAN_UNREGISTER_DRIVER_DECLARE(pcie_ssid) CNSS_WLAN_UNREGISTER_DRIVER_ALIAS_DECLARE(pcie_ssid)

CNSS_WLAN_UNREGISTER_DRIVER_DECLARE(PCIE_SSID)

#define CNSS_PCI_IS_DRV_CONNECTED_ALIAS_DECLARE(suffix) \
int cnss_pci_is_drv_connected_##suffix(struct device *dev);

#define CNSS_PCI_IS_DRV_CONNECTED_DECLARE(pcie_ssid) CNSS_PCI_IS_DRV_CONNECTED_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_IS_DRV_CONNECTED_DECLARE(PCIE_SSID)

#define CNSS_WLAN_PM_CONTROL_ALIAS_DECLARE(suffix) \
int cnss_wlan_pm_control_##suffix(struct device *dev, bool vote);

#define CNSS_WLAN_PM_CONTROL_DECLARE(pcie_ssid) CNSS_WLAN_PM_CONTROL_ALIAS_DECLARE(pcie_ssid)

CNSS_WLAN_PM_CONTROL_DECLARE(PCIE_SSID)

#define CNSS_AUTO_SUSPEND_ALIAS_DECLARE(suffix) \
int cnss_auto_suspend_##suffix(struct device *dev);

#define CNSS_AUTO_SUSPEND_DECLARE(pcie_ssid) CNSS_AUTO_SUSPEND_ALIAS_DECLARE(pcie_ssid)

CNSS_AUTO_SUSPEND_DECLARE(PCIE_SSID)

#define CNSS_AUTO_RESUME_ALIAS_DECLARE(suffix) \
int cnss_auto_resume_##suffix(struct device *dev);

#define CNSS_AUTO_RESUME_DECLARE(pcie_ssid) CNSS_AUTO_RESUME_ALIAS_DECLARE(pcie_ssid)

CNSS_AUTO_RESUME_DECLARE(PCIE_SSID)

#define CNSS_PCI_FORCE_WAKE_REQUEST_ALIAS_DECLARE(suffix) \
int cnss_pci_force_wake_request_##suffix(struct device *dev);

#define CNSS_PCI_FORCE_WAKE_REQUEST_DECLARE(pcie_ssid) CNSS_PCI_FORCE_WAKE_REQUEST_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_FORCE_WAKE_REQUEST_DECLARE(PCIE_SSID)

#define CNSS_PCI_IS_DEVICE_AWAKE_ALIAS_DECLARE(suffix) \
int cnss_pci_is_device_awake_##suffix(struct device *dev);

#define CNSS_PCI_IS_DEVICE_AWAKE_DECLARE(pcie_ssid) CNSS_PCI_IS_DEVICE_AWAKE_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_IS_DEVICE_AWAKE_DECLARE(PCIE_SSID)

#define CNSS_PCI_FORCE_WAKE_RELEASE_ALIAS_DECLARE(suffix) \
int cnss_pci_force_wake_release_##suffix(struct device *dev);

#define CNSS_PCI_FORCE_WAKE_RELEASE_DECLARE(pcie_ssid) CNSS_PCI_FORCE_WAKE_RELEASE_ALIAS_DECLARE(pcie_ssid)

CNSS_PCI_FORCE_WAKE_RELEASE_DECLARE(PCIE_SSID)

#define CNSS_GET_SOC_INFO_ALIAS_DECLARE(suffix) \
int cnss_get_soc_info_##suffix(struct device *dev, struct cnss_soc_info *info);

#define CNSS_GET_SOC_INFO_DECLARE(pcie_ssid) CNSS_GET_SOC_INFO_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_SOC_INFO_DECLARE(PCIE_SSID)

#define CNSS_SMMU_GET_MAPPING_ALIAS_DECLARE(suffix) \
struct dma_iommu_mapping *cnss_smmu_get_mapping_##suffix(struct device *dev);

#define CNSS_SMMU_GET_MAPPING_DECLARE(pcie_ssid) CNSS_SMMU_GET_MAPPING_ALIAS_DECLARE(pcie_ssid)

CNSS_SMMU_GET_MAPPING_DECLARE(PCIE_SSID)

#define CNSS_SMMU_MAP_ALIAS_DECLARE(suffix) \
int cnss_smmu_map_##suffix(struct device *dev, \
			   phys_addr_t paddr, uint32_t *iova_addr, size_t size);

#define CNSS_SMMU_MAP_DECLARE(pcie_ssid) CNSS_SMMU_MAP_ALIAS_DECLARE(pcie_ssid)

CNSS_SMMU_MAP_DECLARE(PCIE_SSID)

#define CNSS_GET_USER_MSI_ASSIGNMENT_ALIAS_DECLARE(suffix) \
int cnss_get_user_msi_assignment_##suffix(struct device *dev, char *user_name, \
					  int *num_vectors, u32 *user_base_data, \
					  u32 *base_vector);

#define CNSS_GET_USER_MSI_ASSIGNMENT_DECLARE(pcie_ssid) CNSS_GET_USER_MSI_ASSIGNMENT_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_USER_MSI_ASSIGNMENT_DECLARE(PCIE_SSID)

#define CNSS_GET_MSI_IRQ_ALIAS_DECLARE(suffix) \
int cnss_get_msi_irq_##suffix(struct device *dev, unsigned int vector);

#define CNSS_GET_MSI_IRQ_DECLARE(pcie_ssid) CNSS_GET_MSI_IRQ_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_MSI_IRQ_DECLARE(PCIE_SSID)

#define CNSS_GET_MSI_ADDRESS_ALIAS_DECLARE(suffix) \
void cnss_get_msi_address_##suffix(struct device *dev, u32 *msi_addr_low, \
				   u32 *msi_addr_high);

#define CNSS_GET_MSI_ADDRESS_DECLARE(pcie_ssid) CNSS_GET_MSI_ADDRESS_ALIAS_DECLARE(pcie_ssid)

CNSS_GET_MSI_ADDRESS_DECLARE(PCIE_SSID)

#define CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_ALIAS_DECLARE(suffix) \
int cnss_utils_set_wlan_unsafe_channel_##suffix(struct device *dev, \
						u16 *unsafe_ch_list, u16 ch_count);

#define CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_DECLARE(pcie_ssid) CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_DECLARE(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_ALIAS_DECLARE(suffix) \
int cnss_utils_get_wlan_unsafe_channel_##suffix(struct device *dev, \
						u16 *unsafe_ch_list, \
						u16 *ch_count, u16 buf_len);

#define CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_DECLARE(pcie_ssid) CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_DECLARE(PCIE_SSID)

#define CNSS_UTILS_WLAN_SET_DFS_NOL_ALIAS_DECLARE(suffix) \
int cnss_utils_wlan_set_dfs_nol_##suffix(struct device *dev, \
					 const void *info, u16 info_len);

#define CNSS_UTILS_WLAN_SET_DFS_NOL_DECLARE(pcie_ssid) CNSS_UTILS_WLAN_SET_DFS_NOL_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_WLAN_SET_DFS_NOL_DECLARE(PCIE_SSID)

#define CNSS_UTILS_WLAN_GET_DFS_NOL_ALIAS_DECLARE(suffix) \
int cnss_utils_wlan_get_dfs_nol_##suffix(struct device *dev, \
					 void *info, u16 info_len);

#define CNSS_UTILS_WLAN_GET_DFS_NOL_DECLARE(pcie_ssid) CNSS_UTILS_WLAN_GET_DFS_NOL_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_WLAN_GET_DFS_NOL_DECLARE(PCIE_SSID)

#define CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_ALIAS_DECLARE(suffix) \
void cnss_utils_increment_driver_load_cnt_##suffix(struct device *dev);

#define CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_DECLARE(pcie_ssid) CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_DECLARE(PCIE_SSID)

#define CNSS_UTILS_GET_DRIVER_LOAD_CNT_ALIAS_DECLARE(suffix) \
int cnss_utils_get_driver_load_cnt_##suffix(struct device *dev);

#define CNSS_UTILS_GET_DRIVER_LOAD_CNT_DECLARE(pcie_ssid) CNSS_UTILS_GET_DRIVER_LOAD_CNT_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_GET_DRIVER_LOAD_CNT_DECLARE(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_MAC_ADDRESS_ALIAS_DECLARE(suffix) \
u8 *cnss_utils_get_wlan_mac_address_##suffix(struct device *dev, uint32_t *num);

#define CNSS_UTILS_GET_WLAN_MAC_ADDRESS_DECLARE(pcie_ssid) CNSS_UTILS_GET_WLAN_MAC_ADDRESS_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_GET_WLAN_MAC_ADDRESS_DECLARE(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_ALIAS_DECLARE(suffix) \
u8 *cnss_utils_get_wlan_derived_mac_address_##suffix( \
                       struct device *dev, uint32_t *num);

#define CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_DECLARE(pcie_ssid) CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_ALIAS_DECLARE(pcie_ssid)

CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_DECLARE(PCIE_SSID)

#define CNSS_SMMU_GET_DOMAIN_ALIAS_DECLARE(suffix) \
struct iommu_domain *cnss_smmu_get_domain_##suffix(struct device *dev);

#define CNSS_SMMU_GET_DOMAIN_DECLARE(pcie_ssid) CNSS_SMMU_GET_DOMAIN_ALIAS_DECLARE(pcie_ssid)

CNSS_SMMU_GET_DOMAIN_DECLARE(PCIE_SSID)
#endif
