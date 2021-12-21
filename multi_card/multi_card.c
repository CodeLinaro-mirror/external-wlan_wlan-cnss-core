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

#include "cnss2/cnss2.h"
#include "cnss_utils/cnss_utils.h"

#define CNSS_LOCK_PM_SEM_ALIAS_EXPORT(suffix) \
void cnss_lock_pm_sem_##suffix(struct device *dev) \
{ \
	return cnss_lock_pm_sem(dev); \
} \
EXPORT_SYMBOL(cnss_lock_pm_sem_##suffix);

#define CNSS_LOCK_PM_SEM_EXPORT(pcie_ssid) CNSS_LOCK_PM_SEM_ALIAS_EXPORT(pcie_ssid)

CNSS_LOCK_PM_SEM_EXPORT(PCIE_SSID)

#define CNSS_RELEASE_PM_SEM_ALIAS_EXPORT(suffix) \
void cnss_release_pm_sem_##suffix(struct device *dev) \
{ \
	return cnss_release_pm_sem(dev); \
} \
EXPORT_SYMBOL(cnss_release_pm_sem_##suffix);

#define CNSS_RELEASE_PM_SEM_EXPORT(pcie_ssid) CNSS_RELEASE_PM_SEM_ALIAS_EXPORT(pcie_ssid)

CNSS_RELEASE_PM_SEM_EXPORT(PCIE_SSID)

#define CNSS_GET_FW_FILES_FOR_TARGET_ALIAS_EXPORT(suffix) \
int cnss_get_fw_files_for_target_##suffix(struct device *dev, \
					  struct cnss_fw_files *pfw_files, \
					  u32 target_type, u32 target_version) \
{ \
       return cnss_get_fw_files_for_target(dev, \
					   pfw_files, \
					   target_type, target_version); \
} \
EXPORT_SYMBOL(cnss_get_fw_files_for_target_##suffix);

#define CNSS_GET_FW_FILES_FOR_TARGET_EXPORT(pcie_ssid) CNSS_GET_FW_FILES_FOR_TARGET_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_FW_FILES_FOR_TARGET_EXPORT(PCIE_SSID)

#define CNSS_REQUEST_BUS_BANDWIDTH_ALIAS_EXPORT(suffix) \
int cnss_request_bus_bandwidth_##suffix(struct device *dev, int bandwidth) \
{ \
	return cnss_request_bus_bandwidth(dev, bandwidth); \
} \
EXPORT_SYMBOL(cnss_request_bus_bandwidth_##suffix);

#define CNSS_REQUEST_BUS_BANDWIDTH_EXPORT(pcie_ssid) CNSS_REQUEST_BUS_BANDWIDTH_ALIAS_EXPORT(pcie_ssid)

CNSS_REQUEST_BUS_BANDWIDTH_EXPORT(PCIE_SSID)

#define CNSS_GET_PLATFORM_CAP_ALIAS_EXPORT(suffix) \
int cnss_get_platform_cap_##suffix(struct device *dev, struct cnss_platform_cap *cap) \
{ \
	return cnss_get_platform_cap(dev, cap); \
} \
EXPORT_SYMBOL(cnss_get_platform_cap_##suffix);

#define CNSS_GET_PLATFORM_CAP_EXPORT(pcie_ssid) CNSS_GET_PLATFORM_CAP_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_PLATFORM_CAP_EXPORT(PCIE_SSID)

#define CNSS_REQUEST_PM_QOS_ALIAS_EXPORT(suffix) \
void cnss_request_pm_qos_##suffix(struct device *dev, u32 qos_val) \
{ \
	return cnss_request_pm_qos(dev, qos_val); \
} \
EXPORT_SYMBOL(cnss_request_pm_qos_##suffix);

#define CNSS_REQUEST_PM_QOS_EXPORT(pcie_ssid) CNSS_REQUEST_PM_QOS_ALIAS_EXPORT(pcie_ssid)

CNSS_REQUEST_PM_QOS_EXPORT(PCIE_SSID)

#define CNSS_REMOVE_PM_QOS_ALIAS_EXPORT(suffix) \
void cnss_remove_pm_qos_##suffix(struct device *dev) \
{ \
	return cnss_remove_pm_qos(dev); \
} \
EXPORT_SYMBOL(cnss_remove_pm_qos_##suffix);

#define CNSS_REMOVE_PM_QOS_EXPORT(pcie_ssid) CNSS_REMOVE_PM_QOS_ALIAS_EXPORT(pcie_ssid)

CNSS_REMOVE_PM_QOS_EXPORT(PCIE_SSID)

#define CNSS_WLAN_ENABLE_ALIAS_EXPORT(suffix) \
int cnss_wlan_enable_##suffix(struct device *dev, \
			      struct cnss_wlan_enable_cfg *config, \
			      enum cnss_driver_mode mode, \
			      const char *host_version) \
{ \
       return cnss_wlan_enable(dev, \
			       config, \
			       mode, \
			       host_version); \
} \
EXPORT_SYMBOL(cnss_wlan_enable_##suffix);

#define CNSS_WLAN_ENABLE_EXPORT(pcie_ssid) CNSS_WLAN_ENABLE_ALIAS_EXPORT(pcie_ssid)

CNSS_WLAN_ENABLE_EXPORT(PCIE_SSID)

#define CNSS_WLAN_DISABLE_ALIAS_EXPORT(suffix) \
int cnss_wlan_disable_##suffix(struct device *dev, enum cnss_driver_mode mode) \
{ \
	return cnss_wlan_disable(dev, mode); \
} \
EXPORT_SYMBOL(cnss_wlan_disable_##suffix);

#define CNSS_WLAN_DISABLE_EXPORT(pcie_ssid) CNSS_WLAN_DISABLE_ALIAS_EXPORT(pcie_ssid)

CNSS_WLAN_DISABLE_EXPORT(PCIE_SSID)

#define CNSS_ATHDIAG_READ_ALIAS_EXPORT(suffix) \
int cnss_athdiag_read_##suffix(struct device *dev, u32 offset, u32 mem_type, \
			       u32 data_len, u8 *output) \
{ \
	return cnss_athdiag_read(dev, offset, mem_type, \
				 data_len, output); \
} \
EXPORT_SYMBOL(cnss_athdiag_read_##suffix);

#define CNSS_ATHDIAG_READ_EXPORT(pcie_ssid) CNSS_ATHDIAG_READ_ALIAS_EXPORT(pcie_ssid)

CNSS_ATHDIAG_READ_EXPORT(PCIE_SSID)

#define CNSS_ATHDIAG_WRITE_ALIAS_EXPORT(suffix) \
int cnss_athdiag_write_##suffix(struct device *dev, u32 offset, u32 mem_type, \
				u32 data_len, u8 *input) \
{ \
	return cnss_athdiag_write(dev, offset, mem_type, data_len, input); \
} \
EXPORT_SYMBOL(cnss_athdiag_write_##suffix);

#define CNSS_ATHDIAG_WRITE_EXPORT(pcie_ssid) CNSS_ATHDIAG_WRITE_ALIAS_EXPORT(pcie_ssid)

CNSS_ATHDIAG_WRITE_EXPORT(PCIE_SSID)

#define CNSS_SET_FW_LOG_MODE_ALIAS_EXPORT(suffix) \
int cnss_set_fw_log_mode_##suffix(struct device *dev, u8 fw_log_mode) \
{ \
	return cnss_set_fw_log_mode(dev, fw_log_mode); \
} \
EXPORT_SYMBOL(cnss_set_fw_log_mode_##suffix);

#define CNSS_SET_FW_LOG_MODE_EXPORT(pcie_ssid) CNSS_SET_FW_LOG_MODE_ALIAS_EXPORT(pcie_ssid)

CNSS_SET_FW_LOG_MODE_EXPORT(PCIE_SSID)

#define CNSS_POWER_UP_ALIAS_EXPORT(suffix) \
int cnss_power_up_##suffix(struct device *dev) \
{ \
	return cnss_power_up(dev); \
} \
EXPORT_SYMBOL(cnss_power_up_##suffix);

#define CNSS_POWER_UP_EXPORT(pcie_ssid) CNSS_POWER_UP_ALIAS_EXPORT(pcie_ssid)

CNSS_POWER_UP_EXPORT(PCIE_SSID)

#define CNSS_POWER_DOWN_ALIAS_EXPORT(suffix) \
int cnss_power_down_##suffix(struct device *dev) \
{ \
	return cnss_power_down(dev); \
} \
EXPORT_SYMBOL(cnss_power_down_##suffix);

#define CNSS_POWER_DOWN_EXPORT(pcie_ssid) CNSS_POWER_DOWN_ALIAS_EXPORT(pcie_ssid)

CNSS_POWER_DOWN_EXPORT(PCIE_SSID)

#define CNSS_IDLE_RESTART_ALIAS_EXPORT(suffix) \
int cnss_idle_restart_##suffix(struct device *dev) \
{ \
	return cnss_idle_restart(dev); \
} \
EXPORT_SYMBOL(cnss_idle_restart_##suffix);

#define CNSS_IDLE_RESTART_EXPORT(pcie_ssid) CNSS_IDLE_RESTART_ALIAS_EXPORT(pcie_ssid)

CNSS_IDLE_RESTART_EXPORT(PCIE_SSID)

#define CNSS_IDLE_SHUTDOWN_ALIAS_EXPORT(suffix) \
int cnss_idle_shutdown_##suffix(struct device *dev) \
{ \
	return cnss_idle_shutdown(dev); \
} \
EXPORT_SYMBOL(cnss_idle_shutdown_##suffix);

#define CNSS_IDLE_SHUTDOWN_EXPORT(pcie_ssid) CNSS_IDLE_SHUTDOWN_ALIAS_EXPORT(pcie_ssid)

CNSS_IDLE_SHUTDOWN_EXPORT(PCIE_SSID)

#define CNSS_GET_VIRT_RAMDUMP_MEM_ALIAS_EXPORT(suffix) \
void *cnss_get_virt_ramdump_mem_##suffix(struct device *dev, unsigned long *size) \
{ \
	return cnss_get_virt_ramdump_mem(dev, size); \
} \
EXPORT_SYMBOL(cnss_get_virt_ramdump_mem_##suffix);

#define CNSS_GET_VIRT_RAMDUMP_MEM_EXPORT(pcie_ssid) CNSS_GET_VIRT_RAMDUMP_MEM_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_VIRT_RAMDUMP_MEM_EXPORT(PCIE_SSID)

#define CNSS_DEVICE_CRASHED_ALIAS_EXPORT(suffix) \
void cnss_device_crashed_##suffix(struct device *dev) \
{ \
	return cnss_device_crashed(dev); \
} \
EXPORT_SYMBOL(cnss_device_crashed_##suffix);

#define CNSS_DEVICE_CRASHED_EXPORT(pcie_ssid) CNSS_DEVICE_CRASHED_ALIAS_EXPORT(pcie_ssid)

CNSS_DEVICE_CRASHED_EXPORT(PCIE_SSID)

#define CNSS_SELF_RECOVERY_ALIAS_EXPORT(suffix) \
int cnss_self_recovery_##suffix(struct device *dev, \
				enum cnss_recovery_reason reason) \
{ \
	return cnss_self_recovery(dev, reason); \
} \
EXPORT_SYMBOL(cnss_self_recovery_##suffix);

#define CNSS_SELF_RECOVERY_EXPORT(pcie_ssid) CNSS_SELF_RECOVERY_ALIAS_EXPORT(pcie_ssid)

CNSS_SELF_RECOVERY_EXPORT(PCIE_SSID)

#define CNSS_SCHEDULE_RECOVERY_ALIAS_EXPORT(suffix) \
void cnss_schedule_recovery_##suffix(struct device *dev, \
				     enum cnss_recovery_reason reason) \
{ \
	return cnss_schedule_recovery(dev, reason); \
} \
EXPORT_SYMBOL(cnss_schedule_recovery_##suffix);

#define CNSS_SCHEDULE_RECOVERY_EXPORT(pcie_ssid) CNSS_SCHEDULE_RECOVERY_ALIAS_EXPORT(pcie_ssid)

CNSS_SCHEDULE_RECOVERY_EXPORT(PCIE_SSID)

#define CNSS_FORCE_FW_ASSERT_ALIAS_EXPORT(suffix) \
int cnss_force_fw_assert_##suffix(struct device *dev) \
{ \
	return cnss_force_fw_assert(dev); \
} \
EXPORT_SYMBOL(cnss_force_fw_assert_##suffix);

#define CNSS_FORCE_FW_ASSERT_EXPORT(pcie_ssid) CNSS_FORCE_FW_ASSERT_ALIAS_EXPORT(pcie_ssid)

CNSS_FORCE_FW_ASSERT_EXPORT(PCIE_SSID)

#define CNSS_FORCE_COLLECT_RDDM_ALIAS_EXPORT(suffix) \
int cnss_force_collect_rddm_##suffix(struct device *dev) \
{ \
	return cnss_force_collect_rddm(dev); \
} \
EXPORT_SYMBOL(cnss_force_collect_rddm_##suffix);

#define CNSS_FORCE_COLLECT_RDDM_EXPORT(pcie_ssid) CNSS_FORCE_COLLECT_RDDM_ALIAS_EXPORT(pcie_ssid)

CNSS_FORCE_COLLECT_RDDM_EXPORT(PCIE_SSID)

#define CNSS_QMI_SEND_GET_ALIAS_EXPORT(suffix) \
int cnss_qmi_send_get_##suffix(struct device *dev) \
{ \
	return cnss_qmi_send_get(dev); \
} \
EXPORT_SYMBOL(cnss_qmi_send_get_##suffix);

#define CNSS_QMI_SEND_GET_EXPORT(pcie_ssid) CNSS_QMI_SEND_GET_ALIAS_EXPORT(pcie_ssid)

CNSS_QMI_SEND_GET_EXPORT(PCIE_SSID)

#define CNSS_QMI_SEND_PUT_ALIAS_EXPORT(suffix) \
int cnss_qmi_send_put_##suffix(struct device *dev) \
{ \
	return cnss_qmi_send_put(dev); \
} \
EXPORT_SYMBOL(cnss_qmi_send_put_##suffix);

#define CNSS_QMI_SEND_PUT_EXPORT(pcie_ssid) CNSS_QMI_SEND_PUT_ALIAS_EXPORT(pcie_ssid)

CNSS_QMI_SEND_PUT_EXPORT(PCIE_SSID)

#define CNSS_QMI_SEND_ALIAS_EXPORT(suffix) \
int cnss_qmi_send_##suffix(struct device *dev, int type, void *cmd, \
			   int cmd_len, void *cb_ctx, \
			   int (*cb)(void *ctx, void *event, int event_len)) \
{ \
	return cnss_qmi_send(dev, type, cmd, \
			     cmd_len, cb_ctx, \
			     cb); \
} \
EXPORT_SYMBOL(cnss_qmi_send_##suffix);

#define CNSS_QMI_SEND_EXPORT(pcie_ssid) CNSS_QMI_SEND_ALIAS_EXPORT(pcie_ssid)

CNSS_QMI_SEND_EXPORT(PCIE_SSID)

#define CNSS_PCI_IS_DEVICE_DOWN_ALIAS_EXPORT(suffix) \
int cnss_pci_is_device_down_##suffix(struct device *dev) \
{ \
	return cnss_pci_is_device_down(dev); \
} \
EXPORT_SYMBOL(cnss_pci_is_device_down_##suffix);

#define CNSS_PCI_IS_DEVICE_DOWN_EXPORT(pcie_ssid) CNSS_PCI_IS_DEVICE_DOWN_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_IS_DEVICE_DOWN_EXPORT(PCIE_SSID)

#define CNSS_PCI_LOCK_REG_WINDOW_ALIAS_EXPORT(suffix) \
void cnss_pci_lock_reg_window_##suffix(struct device *dev, unsigned long *flags) \
{ \
	return cnss_pci_lock_reg_window(dev, flags); \
} \
EXPORT_SYMBOL(cnss_pci_lock_reg_window_##suffix);

#define CNSS_PCI_LOCK_REG_WINDOW_EXPORT(pcie_ssid) CNSS_PCI_LOCK_REG_WINDOW_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_LOCK_REG_WINDOW_EXPORT(PCIE_SSID)

#define CNSS_PCI_UNLOCK_REG_WINDOW_ALIAS_EXPORT(suffix) \
void cnss_pci_unlock_reg_window_##suffix(struct device *dev, unsigned long *flags) \
{ \
	return cnss_pci_unlock_reg_window(dev, flags); \
} \
EXPORT_SYMBOL(cnss_pci_unlock_reg_window_##suffix);

#define CNSS_PCI_UNLOCK_REG_WINDOW_EXPORT(pcie_ssid) CNSS_PCI_UNLOCK_REG_WINDOW_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_UNLOCK_REG_WINDOW_EXPORT(PCIE_SSID)

#define CNSS_PCI_PREVENT_L1_ALIAS_EXPORT(suffix) \
int cnss_pci_prevent_l1_##suffix(struct device *dev) \
{ \
	return cnss_pci_prevent_l1(dev); \
} \
EXPORT_SYMBOL(cnss_pci_prevent_l1_##suffix);

#define CNSS_PCI_PREVENT_L1_EXPORT(pcie_ssid) CNSS_PCI_PREVENT_L1_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_PREVENT_L1_EXPORT(PCIE_SSID)

#define CNSS_PCI_ALLOW_L1_ALIAS_EXPORT(suffix) \
void cnss_pci_allow_l1_##suffix(struct device *dev) \
{ \
	return cnss_pci_allow_l1(dev); \
} \
EXPORT_SYMBOL(cnss_pci_allow_l1_##suffix);

#define CNSS_PCI_ALLOW_L1_EXPORT(pcie_ssid) CNSS_PCI_ALLOW_L1_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_ALLOW_L1_EXPORT(PCIE_SSID)

#define CNSS_PCI_LINK_DOWN_ALIAS_EXPORT(suffix) \
int cnss_pci_link_down_##suffix(struct device *dev) \
{ \
	return cnss_pci_link_down(dev); \
} \
EXPORT_SYMBOL(cnss_pci_link_down_##suffix);

#define CNSS_PCI_LINK_DOWN_EXPORT(pcie_ssid) CNSS_PCI_LINK_DOWN_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_LINK_DOWN_EXPORT(PCIE_SSID)

#define CNSS_WLAN_REGISTER_DRIVER_ALIAS_EXPORT(suffix) \
int cnss_wlan_register_driver_##suffix(struct cnss_wlan_driver *driver_ops) \
{ \
	return cnss_wlan_register_driver(driver_ops); \
} \
EXPORT_SYMBOL(cnss_wlan_register_driver_##suffix);

#define CNSS_WLAN_REGISTER_DRIVER_EXPORT(pcie_ssid) CNSS_WLAN_REGISTER_DRIVER_ALIAS_EXPORT(pcie_ssid)

CNSS_WLAN_REGISTER_DRIVER_EXPORT(PCIE_SSID)

#define CNSS_WLAN_UNREGISTER_DRIVER_ALIAS_EXPORT(suffix) \
void cnss_wlan_unregister_driver_##suffix(struct cnss_wlan_driver *driver_ops) \
{ \
	return cnss_wlan_unregister_driver(driver_ops); \
} \
EXPORT_SYMBOL(cnss_wlan_unregister_driver_##suffix);

#define CNSS_WLAN_UNREGISTER_DRIVER_EXPORT(pcie_ssid) CNSS_WLAN_UNREGISTER_DRIVER_ALIAS_EXPORT(pcie_ssid)

CNSS_WLAN_UNREGISTER_DRIVER_EXPORT(PCIE_SSID)

#define CNSS_PCI_IS_DRV_CONNECTED_ALIAS_EXPORT(suffix) \
int cnss_pci_is_drv_connected_##suffix(struct device *dev) \
{ \
	return cnss_pci_is_drv_connected(dev); \
} \
EXPORT_SYMBOL(cnss_pci_is_drv_connected_##suffix);

#define CNSS_PCI_IS_DRV_CONNECTED_EXPORT(pcie_ssid) CNSS_PCI_IS_DRV_CONNECTED_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_IS_DRV_CONNECTED_EXPORT(PCIE_SSID)

#define CNSS_WLAN_PM_CONTROL_ALIAS_EXPORT(suffix) \
int cnss_wlan_pm_control_##suffix(struct device *dev, bool vote) \
{ \
	return cnss_wlan_pm_control(dev, vote); \
} \
EXPORT_SYMBOL(cnss_wlan_pm_control_##suffix);

#define CNSS_WLAN_PM_CONTROL_EXPORT(pcie_ssid) CNSS_WLAN_PM_CONTROL_ALIAS_EXPORT(pcie_ssid)

CNSS_WLAN_PM_CONTROL_EXPORT(PCIE_SSID)

#define CNSS_AUTO_SUSPEND_ALIAS_EXPORT(suffix) \
int cnss_auto_suspend_##suffix(struct device *dev) \
{ \
	return cnss_auto_suspend(dev); \
} \
EXPORT_SYMBOL(cnss_auto_suspend_##suffix);

#define CNSS_AUTO_SUSPEND_EXPORT(pcie_ssid) CNSS_AUTO_SUSPEND_ALIAS_EXPORT(pcie_ssid)

CNSS_AUTO_SUSPEND_EXPORT(PCIE_SSID)

#define CNSS_AUTO_RESUME_ALIAS_EXPORT(suffix) \
int cnss_auto_resume_##suffix(struct device *dev) \
{ \
	return cnss_auto_resume(dev); \
} \
EXPORT_SYMBOL(cnss_auto_resume_##suffix);

#define CNSS_AUTO_RESUME_EXPORT(pcie_ssid) CNSS_AUTO_RESUME_ALIAS_EXPORT(pcie_ssid)

CNSS_AUTO_RESUME_EXPORT(PCIE_SSID)

#define CNSS_PCI_FORCE_WAKE_REQUEST_ALIAS_EXPORT(suffix) \
int cnss_pci_force_wake_request_##suffix(struct device *dev) \
{ \
	return cnss_pci_force_wake_request(dev); \
} \
EXPORT_SYMBOL(cnss_pci_force_wake_request_##suffix);

#define CNSS_PCI_FORCE_WAKE_REQUEST_EXPORT(pcie_ssid) CNSS_PCI_FORCE_WAKE_REQUEST_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_FORCE_WAKE_REQUEST_EXPORT(PCIE_SSID)

#define CNSS_PCI_IS_DEVICE_AWAKE_ALIAS_EXPORT(suffix) \
int cnss_pci_is_device_awake_##suffix(struct device *dev) \
{ \
	return cnss_pci_is_device_awake(dev); \
} \
EXPORT_SYMBOL(cnss_pci_is_device_awake_##suffix);

#define CNSS_PCI_IS_DEVICE_AWAKE_EXPORT(pcie_ssid) CNSS_PCI_IS_DEVICE_AWAKE_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_IS_DEVICE_AWAKE_EXPORT(PCIE_SSID)

#define CNSS_PCI_FORCE_WAKE_RELEASE_ALIAS_EXPORT(suffix) \
int cnss_pci_force_wake_release_##suffix(struct device *dev) \
{ \
	return cnss_pci_force_wake_release(dev); \
} \
EXPORT_SYMBOL(cnss_pci_force_wake_release_##suffix);

#define CNSS_PCI_FORCE_WAKE_RELEASE_EXPORT(pcie_ssid) CNSS_PCI_FORCE_WAKE_RELEASE_ALIAS_EXPORT(pcie_ssid)

CNSS_PCI_FORCE_WAKE_RELEASE_EXPORT(PCIE_SSID)

#define CNSS_GET_SOC_INFO_ALIAS_EXPORT(suffix) \
int cnss_get_soc_info_##suffix(struct device *dev, struct cnss_soc_info *info) \
{ \
	return cnss_get_soc_info(dev, info); \
} \
EXPORT_SYMBOL(cnss_get_soc_info_##suffix);

#define CNSS_GET_SOC_INFO_EXPORT(pcie_ssid) CNSS_GET_SOC_INFO_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_SOC_INFO_EXPORT(PCIE_SSID)

#define CNSS_SMMU_GET_MAPPING_ALIAS_EXPORT(suffix) \
struct dma_iommu_mapping *cnss_smmu_get_mapping_##suffix(struct device *dev) \
{ \
	return cnss_smmu_get_mapping(dev); \
} \
EXPORT_SYMBOL(cnss_smmu_get_mapping_##suffix);

#define CNSS_SMMU_GET_MAPPING_EXPORT(pcie_ssid) CNSS_SMMU_GET_MAPPING_ALIAS_EXPORT(pcie_ssid)

CNSS_SMMU_GET_MAPPING_EXPORT(PCIE_SSID)

#define CNSS_SMMU_MAP_ALIAS_EXPORT(suffix) \
int cnss_smmu_map_##suffix(struct device *dev, \
			   phys_addr_t paddr, uint32_t *iova_addr, size_t size) \
{ \
	return cnss_smmu_map(dev, \
			     paddr, iova_addr, size); \
} \
EXPORT_SYMBOL(cnss_smmu_map_##suffix);

#define CNSS_SMMU_MAP_EXPORT(pcie_ssid) CNSS_SMMU_MAP_ALIAS_EXPORT(pcie_ssid)

CNSS_SMMU_MAP_EXPORT(PCIE_SSID)

#define CNSS_GET_USER_MSI_ASSIGNMENT_ALIAS_EXPORT(suffix) \
int cnss_get_user_msi_assignment_##suffix(struct device *dev, char *user_name, \
					  int *num_vectors, u32 *user_base_data, \
					  u32 *base_vector) \
{ \
	return cnss_get_user_msi_assignment(dev, user_name, \
					    num_vectors, user_base_data, \
					    base_vector); \
} \
EXPORT_SYMBOL(cnss_get_user_msi_assignment_##suffix);

#define CNSS_GET_USER_MSI_ASSIGNMENT_EXPORT(pcie_ssid) CNSS_GET_USER_MSI_ASSIGNMENT_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_USER_MSI_ASSIGNMENT_EXPORT(PCIE_SSID)

#define CNSS_GET_MSI_IRQ_ALIAS_EXPORT(suffix) \
int cnss_get_msi_irq_##suffix(struct device *dev, unsigned int vector) \
{ \
	return cnss_get_msi_irq(dev, vector); \
} \
EXPORT_SYMBOL(cnss_get_msi_irq_##suffix);

#define CNSS_GET_MSI_IRQ_EXPORT(pcie_ssid) CNSS_GET_MSI_IRQ_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_MSI_IRQ_EXPORT(PCIE_SSID)

#define CNSS_GET_MSI_ADDRESS_ALIAS_EXPORT(suffix) \
void cnss_get_msi_address_##suffix(struct device *dev, u32 *msi_addr_low, \
				   u32 *msi_addr_high) \
{ \
	return cnss_get_msi_address(dev, msi_addr_low, \
				    msi_addr_high); \
} \
EXPORT_SYMBOL(cnss_get_msi_address_##suffix);

#define CNSS_GET_MSI_ADDRESS_EXPORT(pcie_ssid) CNSS_GET_MSI_ADDRESS_ALIAS_EXPORT(pcie_ssid)

CNSS_GET_MSI_ADDRESS_EXPORT(PCIE_SSID)

#define CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_ALIAS_EXPORT(suffix) \
int cnss_utils_set_wlan_unsafe_channel_##suffix(struct device *dev, \
						u16 *unsafe_ch_list, u16 ch_count) \
{ \
	return cnss_utils_set_wlan_unsafe_channel(dev, \
						  unsafe_ch_list, ch_count); \
} \
EXPORT_SYMBOL(cnss_utils_set_wlan_unsafe_channel_##suffix);

#define CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_EXPORT(pcie_ssid) CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_SET_WLAN_UNSAFE_CHANNEL_EXPORT(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_ALIAS_EXPORT(suffix) \
int cnss_utils_get_wlan_unsafe_channel_##suffix(struct device *dev, \
						u16 *unsafe_ch_list, \
						u16 *ch_count, u16 buf_len) \
{ \
	return cnss_utils_get_wlan_unsafe_channel(dev, \
						  unsafe_ch_list, \
						  ch_count, buf_len); \
} \
EXPORT_SYMBOL(cnss_utils_get_wlan_unsafe_channel_##suffix);

#define CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_EXPORT(pcie_ssid) CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_GET_WLAN_UNSAFE_CHANNEL_EXPORT(PCIE_SSID)

#define CNSS_UTILS_WLAN_SET_DFS_NOL_ALIAS_EXPORT(suffix) \
int cnss_utils_wlan_set_dfs_nol_##suffix(struct device *dev, \
					 const void *info, u16 info_len) \
{ \
	return cnss_utils_wlan_set_dfs_nol(dev, info, info_len); \
} \
EXPORT_SYMBOL(cnss_utils_wlan_set_dfs_nol_##suffix);

#define CNSS_UTILS_WLAN_SET_DFS_NOL_EXPORT(pcie_ssid) CNSS_UTILS_WLAN_SET_DFS_NOL_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_WLAN_SET_DFS_NOL_EXPORT(PCIE_SSID)

#define CNSS_UTILS_WLAN_GET_DFS_NOL_ALIAS_EXPORT(suffix) \
int cnss_utils_wlan_get_dfs_nol_##suffix(struct device *dev, \
					 void *info, u16 info_len) \
{ \
	return cnss_utils_wlan_get_dfs_nol(dev, info, info_len); \
} \
EXPORT_SYMBOL(cnss_utils_wlan_get_dfs_nol_##suffix);

#define CNSS_UTILS_WLAN_GET_DFS_NOL_EXPORT(pcie_ssid) CNSS_UTILS_WLAN_GET_DFS_NOL_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_WLAN_GET_DFS_NOL_EXPORT(PCIE_SSID)

#define CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_ALIAS_EXPORT(suffix) \
void cnss_utils_increment_driver_load_cnt_##suffix(struct device *dev) \
{ \
	return cnss_utils_increment_driver_load_cnt(dev); \
} \
EXPORT_SYMBOL(cnss_utils_increment_driver_load_cnt_##suffix);

#define CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_EXPORT(pcie_ssid) CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_INCREMENT_DRIVER_LOAD_CNT_EXPORT(PCIE_SSID)

#define CNSS_UTILS_GET_DRIVER_LOAD_CNT_ALIAS_EXPORT(suffix) \
int cnss_utils_get_driver_load_cnt_##suffix(struct device *dev) \
{ \
	return cnss_utils_get_driver_load_cnt(dev); \
} \
EXPORT_SYMBOL(cnss_utils_get_driver_load_cnt_##suffix);

#define CNSS_UTILS_GET_DRIVER_LOAD_CNT_EXPORT(pcie_ssid) CNSS_UTILS_GET_DRIVER_LOAD_CNT_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_GET_DRIVER_LOAD_CNT_EXPORT(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_MAC_ADDRESS_ALIAS_EXPORT(suffix) \
u8 *cnss_utils_get_wlan_mac_address_##suffix(struct device *dev, uint32_t *num) \
{ \
	return cnss_utils_get_wlan_mac_address(dev, num); \
} \
EXPORT_SYMBOL(cnss_utils_get_wlan_mac_address_##suffix);

#define CNSS_UTILS_GET_WLAN_MAC_ADDRESS_EXPORT(pcie_ssid) CNSS_UTILS_GET_WLAN_MAC_ADDRESS_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_GET_WLAN_MAC_ADDRESS_EXPORT(PCIE_SSID)

#define CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_ALIAS_EXPORT(suffix) \
u8 *cnss_utils_get_wlan_derived_mac_address_##suffix( \
			struct device *dev, uint32_t *num) \
{ \
	return cnss_utils_get_wlan_derived_mac_address(dev, num); \
} \
EXPORT_SYMBOL(cnss_utils_get_wlan_derived_mac_address_##suffix);

#define CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_EXPORT(pcie_ssid) CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_ALIAS_EXPORT(pcie_ssid)

CNSS_UTILS_GET_WLAN_DERIVED_MAC_ADDRESS_EXPORT(PCIE_SSID)

#define CNSS_SMMU_GET_DOMAIN_ALIAS_EXPORT(suffix) \
struct iommu_domain *cnss_smmu_get_domain_##suffix(struct device *dev) \
{ \
	return cnss_smmu_get_domain(dev); \
} \
EXPORT_SYMBOL(cnss_smmu_get_domain_##suffix);

#define CNSS_SMMU_GET_DOMAIN_EXPORT(pcie_ssid) CNSS_SMMU_GET_DOMAIN_ALIAS_EXPORT(pcie_ssid)

CNSS_SMMU_GET_DOMAIN_EXPORT(PCIE_SSID)
