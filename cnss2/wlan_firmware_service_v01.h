/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef WLAN_FIRMWARE_SERVICE_V01_H
#define WLAN_FIRMWARE_SERVICE_V01_H

#ifdef CONFIG_NAPIER_X86
#include "qmi_encdec.h"
#include "msm_qmi_interface.h"
#else
#include <linux/qmi_encdec.h>
#include <soc/qcom/msm_qmi_interface.h>
#endif

#define FW_MEM_SEG_INDEX_0	0

#define WLFW_SERVICE_ID_V01 0x45
#define WLFW_SERVICE_VERS_V01 0x01
#define QMI_WLFW_DEVICE_INFO_RESP_V01 0x004C

#define QMI_WLFW_BDF_DOWNLOAD_REQ_V01 0x0025
#define QMI_WLFW_FW_MEM_READY_IND_V01 0x0037
#define QMI_WLFW_QDSS_TRACE_CONFIG_DOWNLOAD_REQ_V01 0x0044
#define QMI_WLFW_GET_INFO_REQ_V01 0x004A
#define QMI_WLFW_INITIATE_CAL_UPDATE_IND_V01 0x002A
#define QMI_WLFW_CAL_DONE_IND_V01 0x003E
#define QMI_WLFW_M3_DUMP_UPLOAD_REQ_IND_V01 0x004D
#define QMI_WLFW_HOST_CAP_REQ_V01 0x0034
#define QMI_WLFW_DYNAMIC_FEATURE_MASK_RESP_V01 0x003B
#define QMI_WLFW_M3_INFO_REQ_V01 0x003C
#define QMI_WLFW_CAP_REQ_V01 0x0024
#define QMI_WLFW_FW_INIT_DONE_IND_V01 0x0038
#define QMI_WLFW_CAL_REPORT_REQ_V01 0x0026
#define QMI_WLFW_RESPOND_GET_INFO_IND_V01 0x004B
#define QMI_WLFW_M3_INFO_RESP_V01 0x003C
#define QMI_WLFW_CAL_UPDATE_RESP_V01 0x0029
#define QMI_WLFW_M3_DUMP_UPLOAD_DONE_RESP_V01 0x004E
#define QMI_WLFW_CAL_DOWNLOAD_RESP_V01 0x0027
#define QMI_WLFW_XO_CAL_IND_V01 0x003D
#define QMI_WLFW_INI_RESP_V01 0x002F
#define QMI_WLFW_CAL_REPORT_RESP_V01 0x0026
#define QMI_WLFW_QDSS_TRACE_MEM_INFO_REQ_V01 0x0040
#define QMI_WLFW_QDSS_TRACE_REQ_MEM_IND_V01 0x003F
#define QMI_WLFW_SHUTDOWN_RESP_V01 0x0043
#define QMI_WLFW_MAC_ADDR_RESP_V01 0x0033
#define QMI_WLFW_INITIATE_CAL_DOWNLOAD_IND_V01 0x0028
#define QMI_WLFW_HOST_CAP_RESP_V01 0x0034
#define QMI_WLFW_MSA_READY_IND_V01 0x002B
#define QMI_WLFW_ATHDIAG_WRITE_RESP_V01 0x0031
#define QMI_WLFW_EXIT_POWER_SAVE_REQ_V01 0x0050
#define QMI_WLFW_WLAN_MODE_REQ_V01 0x0022
#define QMI_WLFW_IND_REGISTER_REQ_V01 0x0020
#define QMI_WLFW_WLAN_CFG_RESP_V01 0x0023
#define QMI_WLFW_QDSS_TRACE_MODE_REQ_V01 0x0045
#define QMI_WLFW_REQUEST_MEM_IND_V01 0x0035
#define QMI_WLFW_QDSS_TRACE_CONFIG_DOWNLOAD_RESP_V01 0x0044
#define QMI_WLFW_SOC_WAKE_RESP_V01 0x004F
#define QMI_WLFW_REJUVENATE_IND_V01 0x0039
#define QMI_WLFW_DYNAMIC_FEATURE_MASK_REQ_V01 0x003B
#define QMI_WLFW_ATHDIAG_WRITE_REQ_V01 0x0031
#define QMI_WLFW_WLAN_MODE_RESP_V01 0x0022
#define QMI_WLFW_RESPOND_MEM_REQ_V01 0x0036
#define QMI_WLFW_GET_INFO_RESP_V01 0x004A
#define QMI_WLFW_QDSS_TRACE_MODE_RESP_V01 0x0045
#define QMI_WLFW_PIN_CONNECT_RESULT_IND_V01 0x002C
#define QMI_WLFW_FW_READY_IND_V01 0x0021
#define QMI_WLFW_QDSS_TRACE_SAVE_IND_V01 0x0041
#define QMI_WLFW_QDSS_TRACE_MEM_INFO_RESP_V01 0x0040
#define QMI_WLFW_MSA_READY_RESP_V01 0x002E
#define QMI_WLFW_QDSS_TRACE_DATA_REQ_V01 0x0042
#define QMI_WLFW_CAL_UPDATE_REQ_V01 0x0029
#define QMI_WLFW_INI_REQ_V01 0x002F
#define QMI_WLFW_BDF_DOWNLOAD_RESP_V01 0x0025
#define QMI_WLFW_REJUVENATE_ACK_RESP_V01 0x003A
#define QMI_WLFW_MSA_INFO_RESP_V01 0x002D
#define QMI_WLFW_MSA_READY_REQ_V01 0x002E
#define QMI_WLFW_QDSS_TRACE_DATA_RESP_V01 0x0042
#define QMI_WLFW_M3_DUMP_UPLOAD_DONE_REQ_V01 0x004E
#define QMI_WLFW_CAP_RESP_V01 0x0024
#define QMI_WLFW_SOC_WAKE_REQ_V01 0x004F
#define QMI_WLFW_REJUVENATE_ACK_REQ_V01 0x003A
#define QMI_WLFW_ATHDIAG_READ_RESP_V01 0x0030
#define QMI_WLFW_SHUTDOWN_REQ_V01 0x0043
#define QMI_WLFW_VBATT_REQ_V01 0x0032
#define QMI_WLFW_DEVICE_INFO_REQ_V01 0x004C
#define QMI_WLFW_MAC_ADDR_REQ_V01 0x0033
#define QMI_WLFW_EXIT_POWER_SAVE_RESP_V01 0x0050
#define QMI_WLFW_RESPOND_MEM_RESP_V01 0x0036
#define QMI_WLFW_VBATT_RESP_V01 0x0032
#define QMI_WLFW_MSA_INFO_REQ_V01 0x002D
#define QMI_WLFW_QDSS_TRACE_FREE_IND_V01 0x0046
#define QMI_WLFW_CAL_DOWNLOAD_REQ_V01 0x0027
#define QMI_WLFW_ATHDIAG_READ_REQ_V01 0x0030
#define QMI_WLFW_WLAN_CFG_REQ_V01 0x0023
#define QMI_WLFW_IND_REGISTER_RESP_V01 0x0020
#define QMI_WLFW_PCIE_GEN_SWITCH_REQ_V01 0x0053
#define QMI_WLFW_PCIE_GEN_SWITCH_RESP_V01 0x0053
#define QMI_WLFW_QDSS_MEM_READY_IND_V01 0x0052

#define QMI_WLFW_MAX_NUM_MEMORY_REGIONS_V01 2
#define QMI_WLFW_MAX_NUM_MEM_SEG_V01 32
#define QMI_WLFW_MAX_NUM_CAL_V01 5
#define QMI_WLFW_MAX_DATA_SIZE_V01 6144
#define QMI_WLFW_FUNCTION_NAME_LEN_V01 128
#define QMI_WLFW_MAX_NUM_CE_V01 12
#define QMI_WLFW_MAX_TIMESTAMP_LEN_V01 32
#define QMI_WLFW_MAX_ATHDIAG_DATA_SIZE_V01 6144
#define QMI_WLFW_MAX_NUM_GPIO_V01 32
#define QMI_WLFW_MAX_BUILD_ID_LEN_V01 128
#define QMI_WLFW_MAX_NUM_MEM_CFG_V01 2
#define QMI_WLFW_MAX_STR_LEN_V01 16
#define QMI_WLFW_MAX_NUM_SHADOW_REG_V01 24
#define QMI_WLFW_MAC_ADDR_SIZE_V01 6
#define QMI_WLFW_MAX_NUM_SHADOW_REG_V2_V01 36
#define QMI_WLFW_MAX_NUM_SVC_V01 24
#define QMI_WLFW_MAX_PLATFORM_NAME_LEN_V01 64
#define QMI_WLFW_MAX_HOST_DDR_RANGE_SIZE_V01 3
#define QMI_WLFW_MAX_MLO_CHIP_V01 3
#define QMI_WLFW_MAX_NUM_MLO_LINKS_PER_CHIP_V01 2
#define QMI_WLFW_MAX_NUM_GPIO_INFO_V01 20
#define QMI_WLFW_MLO_V2_CHP_V01 4
#define QMI_WLFW_MAX_ADJ_CHIP_V01 2
#define QMI_WLFW_MAX_DEV_MEM_NUM_V01 4
#define QMI_WLFW_PMU_PARAMS_MAX_V01 16
#define QMI_WLFW_PMU_PIN_NAME_MAX_LEN_V01 32

enum wlfw_driver_mode_enum_v01 {
	WLFW_DRIVER_MODE_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_MISSION_V01 = 0,
	QMI_WLFW_FTM_V01 = 1,
	QMI_WLFW_EPPING_V01 = 2,
	QMI_WLFW_WALTEST_V01 = 3,
	QMI_WLFW_OFF_V01 = 4,
	QMI_WLFW_CCPM_V01 = 5,
	QMI_WLFW_QVIT_V01 = 6,
	QMI_WLFW_CALIBRATION_V01 = 7,
	QMI_WLFW_FTM_CALIBRATION_V01 = 10,
	WLFW_DRIVER_MODE_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_cal_temp_id_enum_v01 {
	WLFW_CAL_TEMP_ID_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_CAL_TEMP_IDX_0_V01 = 0,
	QMI_WLFW_CAL_TEMP_IDX_1_V01 = 1,
	QMI_WLFW_CAL_TEMP_IDX_2_V01 = 2,
	QMI_WLFW_CAL_TEMP_IDX_3_V01 = 3,
	QMI_WLFW_CAL_TEMP_IDX_4_V01 = 4,
	WLFW_CAL_TEMP_ID_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_pipedir_enum_v01 {
	WLFW_PIPEDIR_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_PIPEDIR_NONE_V01 = 0,
	QMI_WLFW_PIPEDIR_IN_V01 = 1,
	QMI_WLFW_PIPEDIR_OUT_V01 = 2,
	QMI_WLFW_PIPEDIR_INOUT_V01 = 3,
	WLFW_PIPEDIR_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_mem_type_enum_v01 {
	WLFW_MEM_TYPE_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_MEM_TYPE_MSA_V01 = 0,
	QMI_WLFW_MEM_TYPE_DDR_V01 = 1,
	QMI_WLFW_MEM_BDF_V01 = 2,
	QMI_WLFW_MEM_M3_V01 = 3,
	QMI_WLFW_MEM_CAL_V01 = 4,
	QMI_WLFW_MEM_DPD_V01 = 5,
	QMI_WLFW_MEM_QDSS_V01 = 6,
	WLFW_MEM_TYPE_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_qdss_trace_mode_enum_v01 {
	WLFW_QDSS_TRACE_MODE_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_QDSS_TRACE_OFF_V01 = 0,
	QMI_WLFW_QDSS_TRACE_ON_V01 = 1,
	WLFW_QDSS_TRACE_MODE_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_soc_wake_enum_v01 {
	WLFW_SOC_WAKE_ENUM_MIN_VAL_V01 = INT_MIN,
	QMI_WLFW_WAKE_REQUEST_V01 = 0,
	QMI_WLFW_WAKE_RELEASE_V01 = 1,
	WLFW_SOC_WAKE_ENUM_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_pcie_gen_speed_v01 {
	WLFW_PCIE_GEN_SPEED_MIN_VAL_V01 = INT_MIN,
	QMI_PCIE_GEN_SPEED_INVALID_V01 = 0,
	QMI_PCIE_GEN_SPEED_1_V01 = 1,
	QMI_PCIE_GEN_SPEED_2_V01 = 2,
	QMI_PCIE_GEN_SPEED_3_V01 = 3,
	WLFW_PCIE_GEN_SPEED_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_host_build_type_v01 {
	WLFW_HOST_BUILD_TYPE_MIN_VAL_V01 = INT_MIN,
	QMI_HOST_BUILD_TYPE_UNSPECIFIED_V01 = 0,
	QMI_HOST_BUILD_TYPE_PRIMARY_V01 = 1,
	QMI_HOST_BUILD_TYPE_SECONDARY_V01 = 2,
	WLFW_HOST_BUILD_TYPE_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_rd_card_chain_cap_v01 {
	WLFW_RD_CARD_CHAIN_CAP_MIN_VAL_V01 = INT_MIN,
	WLFW_RD_CARD_CHAIN_CAP_UNSPECIFIED_V01 = 0,
	WLFW_RD_CARD_CHAIN_CAP_1x1_V01 = 1,
	WLFW_RD_CARD_CHAIN_CAP_2x2_V01 = 2,
	WLFW_RD_CARD_CHAIN_CAP_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_bdf_dnld_method_v01 {
	WLFW_BDF_DNLD_METHOD_MIN_VAL_V01 = INT_MIN,
	WLFW_DIRECT_BDF_COPY_V01 = 0,
	WLFW_SEND_BDF_OVER_QMI_V01 = 1,
	WLFW_BDF_DNLD_METHOD_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_he_channel_width_cap_v01 {
	WLFW_HE_CHANNEL_WIDTH_CAP_MIN_VAL_V01 = INT_MIN,
	WLFW_PHY_HE_CHANNEL_WIDTH_CAP_UNSPECIFIED_V01 = 0,
	WLFW_PHY_HE_CHANNEL_WIDTH_CAP_80MHZ_V01 = 1,
	WLFW_PHY_HE_CHANNEL_WIDTH_CAP_160MHZ_V01 = 2,
	WLFW_HE_CHANNEL_WIDTH_CAP_MAX_VAL_V01 = INT_MAX,
};

enum wlfw_phy_qam_cap_v01 {
	WLFW_PHY_QAM_CAP_MIN_VAL_V01 = INT_MIN,
	WLFW_PHY_QAM_CAP_UNSPECIFIED_V01 = 0,
	WLFW_PHY_QAM_CAP_1K_V01 = 1,
	WLFW_PHY_QAM_CAP_4K_V01 = 2,
	WLFW_PHY_QAM_CAP_MAX_VAL_V01 = INT_MAX,
};

struct wlfw_host_ddr_range_s_v01 {
	u64 start;
	u64 size;
};

struct mlo_chip_info_s_v01 {
	u8 chip_id;
	u8 num_local_links;
	u8 hw_link_id[QMI_WLFW_MAX_NUM_MLO_LINKS_PER_CHIP_V01];
	u8 valid_mlo_link_id[QMI_WLFW_MAX_NUM_MLO_LINKS_PER_CHIP_V01];
};

struct wlfw_host_pcie_link_info_s_v01 {
	u32 pci_link_speed;
	u32 pci_link_width;
};

struct mlo_chip_v2_info_s_v01 {
	struct mlo_chip_info_s_v01 mlo_chip_info;
	u8 adj_mlo_num_chips;
	struct mlo_chip_info_s_v01 adj_mlo_chip_info[QMI_WLFW_MAX_ADJ_CHIP_V01];
};

struct wlfw_dev_mem_info_s_v01 {
	u64 start;
	u64 size;
};

struct wlfw_pmu_param_v01 {
	u8 pin_name[QMI_WLFW_PMU_PIN_NAME_MAX_LEN_V01];
	u32 wake_volt_valid;
	u32 wake_volt;
	u32 sleep_volt_valid;
	u32 sleep_volt;
};

struct wlfw_pmu_cfg_v01 {
	u32 pmu_param_len;
	struct wlfw_pmu_param_v01 pmu_param[QMI_WLFW_PMU_PARAMS_MAX_V01];
};

struct wlchip_serial_id_v01 {
	u32 serial_id_msb;
	u32 serial_id_lsb;
};

#define QMI_WLFW_CE_ATTR_FLAGS_V01 ((u32)0x00)
#define QMI_WLFW_CE_ATTR_NO_SNOOP_V01 ((u32)0x01)
#define QMI_WLFW_CE_ATTR_BYTE_SWAP_DATA_V01 ((u32)0x02)
#define QMI_WLFW_CE_ATTR_SWIZZLE_DESCRIPTORS_V01 ((u32)0x04)
#define QMI_WLFW_CE_ATTR_DISABLE_INTR_V01 ((u32)0x08)
#define QMI_WLFW_CE_ATTR_ENABLE_POLL_V01 ((u32)0x10)

#define QMI_WLFW_ALREADY_REGISTERED_V01 ((u64)0x01ULL)
#define QMI_WLFW_FW_READY_V01 ((u64)0x02ULL)
#define QMI_WLFW_MSA_READY_V01 ((u64)0x04ULL)
#define QMI_WLFW_FW_MEM_READY_V01 ((u64)0x08ULL)
#define QMI_WLFW_FW_INIT_DONE_V01 ((u64)0x10ULL)

#define QMI_WLFW_FW_REJUVENATE_V01 ((u64)0x01ULL)

#define QMI_WLFW_HW_XPA_V01 ((u64)0x01ULL)
#define QMI_WLFW_CBC_FILE_DOWNLOAD_V01 ((u64)0x02ULL)

#define QMI_WLFW_HOST_PCIE_GEN_SWITCH_V01 ((u64)0x01ULL)

struct wlfw_ce_tgt_pipe_cfg_s_v01 {
	u32 pipe_num;
	enum wlfw_pipedir_enum_v01 pipe_dir;
	u32 nentries;
	u32 nbytes_max;
	u32 flags;
};

struct wlfw_ce_svc_pipe_cfg_s_v01 {
	u32 service_id;
	enum wlfw_pipedir_enum_v01 pipe_dir;
	u32 pipe_num;
};

struct wlfw_shadow_reg_cfg_s_v01 {
	u16 id;
	u16 offset;
};

struct wlfw_shadow_reg_v2_cfg_s_v01 {
	u32 addr;
};

struct wlfw_rri_over_ddr_cfg_s_v01 {
	u32 base_addr_low;
	u32 base_addr_high;
};

struct wlfw_msi_cfg_s_v01 {
	u16 ce_id;
	u16 msi_vector;
};

struct wlfw_memory_region_info_s_v01 {
	u64 region_addr;
	u32 size;
	u8 secure_flag;
};

struct wlfw_mem_cfg_s_v01 {
	u64 offset;
	u32 size;
	u8 secure_flag;
};

struct wlfw_mem_seg_s_v01 {
	u32 size;
	enum wlfw_mem_type_enum_v01 type;
	u32 mem_cfg_len;
	struct wlfw_mem_cfg_s_v01 mem_cfg[QMI_WLFW_MAX_NUM_MEM_CFG_V01];
};

struct wlfw_mem_seg_resp_s_v01 {
	u64 addr;
	u32 size;
	enum wlfw_mem_type_enum_v01 type;
	u8 restore;
};

struct wlfw_rf_chip_info_s_v01 {
	u32 chip_id;
	u32 chip_family;
};

struct wlfw_rf_board_info_s_v01 {
	u32 board_id;
};

struct wlfw_soc_info_s_v01 {
	u32 soc_id;
};

struct wlfw_fw_version_info_s_v01 {
	u32 fw_version;
	char fw_build_timestamp[QMI_WLFW_MAX_TIMESTAMP_LEN_V01 + 1];
};

struct wlfw_ind_register_req_msg_v01 {
	u8 fw_ready_enable_valid;
	u8 fw_ready_enable;
	u8 initiate_cal_download_enable_valid;
	u8 initiate_cal_download_enable;
	u8 initiate_cal_update_enable_valid;
	u8 initiate_cal_update_enable;
	u8 msa_ready_enable_valid;
	u8 msa_ready_enable;
	u8 pin_connect_result_enable_valid;
	u8 pin_connect_result_enable;
	u8 client_id_valid;
	u32 client_id;
	u8 request_mem_enable_valid;
	u8 request_mem_enable;
	u8 fw_mem_ready_enable_valid;
	u8 fw_mem_ready_enable;
	u8 fw_init_done_enable_valid;
	u8 fw_init_done_enable;
	u8 rejuvenate_enable_valid;
	u32 rejuvenate_enable;
	u8 xo_cal_enable_valid;
	u8 xo_cal_enable;
	u8 cal_done_enable_valid;
	u8 cal_done_enable;
	u8 qdss_trace_req_mem_enable_valid;
	u8 qdss_trace_req_mem_enable;
	u8 qdss_trace_save_enable_valid;
	u8 qdss_trace_save_enable;
	u8 qdss_trace_free_enable_valid;
	u8 qdss_trace_free_enable;
	u8 respond_get_info_enable_valid;
	u8 respond_get_info_enable;
	u8 m3_dump_upload_req_enable_valid;
	u8 m3_dump_upload_req_enable;
	u8 wfc_call_twt_config_enable_valid;
	u8 wfc_call_twt_config_enable;
	u8 qdss_mem_ready_enable_valid;
	u8 qdss_mem_ready_enable;
};

#define WLFW_IND_REGISTER_REQ_MSG_V01_MAX_MSG_LEN 82
extern struct elem_info wlfw_ind_register_req_msg_v01_ei[];

struct wlfw_ind_register_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 fw_status_valid;
	u64 fw_status;
};

#define WLFW_IND_REGISTER_RESP_MSG_V01_MAX_MSG_LEN 18
extern struct elem_info wlfw_ind_register_resp_msg_v01_ei[];

struct wlfw_fw_ready_ind_msg_v01 {
	char placeholder;
};

#define WLFW_FW_READY_IND_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_fw_ready_ind_msg_v01_ei[];

struct wlfw_msa_ready_ind_msg_v01 {
	u8 hang_data_addr_offset_valid;
	u32 hang_data_addr_offset;
	u8 hang_data_length_valid;
	u16 hang_data_length;
};

#define WLFW_MSA_READY_IND_MSG_V01_MAX_MSG_LEN 12
extern struct elem_info wlfw_msa_ready_ind_msg_v01_ei[];

struct wlfw_pin_connect_result_ind_msg_v01 {
	u8 pwr_pin_result_valid;
	u32 pwr_pin_result;
	u8 phy_io_pin_result_valid;
	u32 phy_io_pin_result;
	u8 rf_pin_result_valid;
	u32 rf_pin_result;
};

#define WLFW_PIN_CONNECT_RESULT_IND_MSG_V01_MAX_MSG_LEN 21
extern struct elem_info wlfw_pin_connect_result_ind_msg_v01_ei[];

struct wlfw_wlan_mode_req_msg_v01 {
	enum wlfw_driver_mode_enum_v01 mode;
	u8 hw_debug_valid;
	u8 hw_debug;
};

#define WLFW_WLAN_MODE_REQ_MSG_V01_MAX_MSG_LEN 11
extern struct elem_info wlfw_wlan_mode_req_msg_v01_ei[];

struct wlfw_wlan_mode_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_WLAN_MODE_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_wlan_mode_resp_msg_v01_ei[];

struct wlfw_wlan_cfg_req_msg_v01 {
	u8 host_version_valid;
	char host_version[QMI_WLFW_MAX_STR_LEN_V01 + 1];
	u8 tgt_cfg_valid;
	u32 tgt_cfg_len;
	struct wlfw_ce_tgt_pipe_cfg_s_v01 tgt_cfg[QMI_WLFW_MAX_NUM_CE_V01];
	u8 svc_cfg_valid;
	u32 svc_cfg_len;
	struct wlfw_ce_svc_pipe_cfg_s_v01 svc_cfg[QMI_WLFW_MAX_NUM_SVC_V01];
	u8 shadow_reg_valid;
	u32 shadow_reg_len;
	struct wlfw_shadow_reg_cfg_s_v01
		shadow_reg[QMI_WLFW_MAX_NUM_SHADOW_REG_V01];
	u8 shadow_reg_v2_valid;
	u32 shadow_reg_v2_len;
	struct wlfw_shadow_reg_v2_cfg_s_v01
		shadow_reg_v2[QMI_WLFW_MAX_NUM_SHADOW_REG_V2_V01];
	u8 rri_over_ddr_cfg_valid;
	struct wlfw_rri_over_ddr_cfg_s_v01 rri_over_ddr_cfg;
	u8 msi_cfg_valid;
	u32 msi_cfg_len;
	struct wlfw_msi_cfg_s_v01 msi_cfg[QMI_WLFW_MAX_NUM_CE_V01];
};

#define WLFW_WLAN_CFG_REQ_MSG_V01_MAX_MSG_LEN 866
extern struct elem_info wlfw_wlan_cfg_req_msg_v01_ei[];

struct wlfw_wlan_cfg_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_WLAN_CFG_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_wlan_cfg_resp_msg_v01_ei[];

struct wlfw_cap_req_msg_v01 {
	char placeholder;
};

#define WLFW_CAP_REQ_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_cap_req_msg_v01_ei[];

struct wlfw_cap_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 chip_info_valid;
	struct wlfw_rf_chip_info_s_v01 chip_info;
	u8 board_info_valid;
	struct wlfw_rf_board_info_s_v01 board_info;
	u8 soc_info_valid;
	struct wlfw_soc_info_s_v01 soc_info;
	u8 fw_version_info_valid;
	struct wlfw_fw_version_info_s_v01 fw_version_info;
	u8 fw_build_id_valid;
	char fw_build_id[QMI_WLFW_MAX_BUILD_ID_LEN_V01 + 1];
	u8 num_macs_valid;
	u8 num_macs;
	u8 voltage_mv_valid;
	u32 voltage_mv;
	u8 time_freq_hz_valid;
	u32 time_freq_hz;
	u8 otp_version_valid;
	u32 otp_version;
	u8 eeprom_caldata_read_timeout_valid;
	u32 eeprom_caldata_read_timeout;
	u8 fw_caps_valid;
	u64 fw_caps;
	u8 rd_card_chain_cap_valid;
	enum wlfw_rd_card_chain_cap_v01 rd_card_chain_cap;
	u8 dev_mem_info_valid;
	struct wlfw_dev_mem_info_s_v01 dev_mem_info[QMI_WLFW_MAX_DEV_MEM_NUM_V01];
	u8 foundry_name_valid;
	char foundry_name[QMI_WLFW_MAX_STR_LEN_V01 + 1];
	u8 hang_data_addr_offset_valid;
	u32 hang_data_addr_offset;
	u8 hang_data_length_valid;
	u16 hang_data_length;
	u8 bdf_dnld_method_valid;
	enum wlfw_bdf_dnld_method_v01 bdf_dnld_method;
	u8 hwid_bitmap_valid;
	u8 hwid_bitmap;
	u8 ol_cpr_cfg_valid;
	struct wlfw_pmu_cfg_v01 ol_cpr_cfg;
	u8 regdb_mandatory_valid;
	u8 regdb_mandatory;
	u8 regdb_support_valid;
	u8 regdb_support;
	u8 rxgainlut_support_valid;
	u8 rxgainlut_support;
	u8 he_channel_width_cap_valid;
	enum wlfw_he_channel_width_cap_v01 he_channel_width_cap;
	u8 phy_qam_cap_valid;
	enum wlfw_phy_qam_cap_v01 phy_qam_cap;
	u8 serial_id_valid;
	struct wlchip_serial_id_v01 serial_id;
};
#define WLFW_CAP_RESP_MSG_V01_MAX_MSG_LEN 1171
extern struct elem_info wlfw_cap_resp_msg_v01_ei[];

struct wlfw_bdf_download_req_msg_v01 {
	u8 valid;
	u8 file_id_valid;
	enum wlfw_cal_temp_id_enum_v01 file_id;
	u8 total_size_valid;
	u32 total_size;
	u8 seg_id_valid;
	u32 seg_id;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_DATA_SIZE_V01];
	u8 end_valid;
	u8 end;
	u8 bdf_type_valid;
	u8 bdf_type;
};

#define WLFW_BDF_DOWNLOAD_REQ_MSG_V01_MAX_MSG_LEN 6182
extern struct elem_info wlfw_bdf_download_req_msg_v01_ei[];

struct wlfw_bdf_download_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 host_bdf_data_valid;
	u64 host_bdf_data;
};

#define WLFW_BDF_DOWNLOAD_RESP_MSG_V01_MAX_MSG_LEN 18
extern struct elem_info wlfw_bdf_download_resp_msg_v01_ei[];

struct wlfw_cal_report_req_msg_v01 {
	u32 meta_data_len;
	enum wlfw_cal_temp_id_enum_v01 meta_data[QMI_WLFW_MAX_NUM_CAL_V01];
	u8 xo_cal_data_valid;
	u8 xo_cal_data;
	u8 cal_remove_supported_valid;
	u8 cal_remove_supported;
};

#define WLFW_CAL_REPORT_REQ_MSG_V01_MAX_MSG_LEN 32
extern struct elem_info wlfw_cal_report_req_msg_v01_ei[];

struct wlfw_cal_report_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_CAL_REPORT_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_cal_report_resp_msg_v01_ei[];

struct wlfw_initiate_cal_download_ind_msg_v01 {
	enum wlfw_cal_temp_id_enum_v01 cal_id;
	u8 total_size_valid;
	u32 total_size;
	u8 cal_data_location_valid;
	u32 cal_data_location;
};

#define WLFW_INITIATE_CAL_DOWNLOAD_IND_MSG_V01_MAX_MSG_LEN 21
extern struct elem_info wlfw_initiate_cal_download_ind_msg_v01_ei[];

struct wlfw_cal_download_req_msg_v01 {
	u8 valid;
	u8 file_id_valid;
	enum wlfw_cal_temp_id_enum_v01 file_id;
	u8 total_size_valid;
	u32 total_size;
	u8 seg_id_valid;
	u32 seg_id;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_DATA_SIZE_V01];
	u8 end_valid;
	u8 end;
	u8 cal_data_location_valid;
	u32 cal_data_location;
};

#define WLFW_CAL_DOWNLOAD_REQ_MSG_V01_MAX_MSG_LEN 6185
extern struct elem_info wlfw_cal_download_req_msg_v01_ei[];

struct wlfw_cal_download_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_CAL_DOWNLOAD_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_cal_download_resp_msg_v01_ei[];

struct wlfw_initiate_cal_update_ind_msg_v01 {
	enum wlfw_cal_temp_id_enum_v01 cal_id;
	u32 total_size;
	u8 cal_data_location_valid;
	u32 cal_data_location;
};

#define WLFW_INITIATE_CAL_UPDATE_IND_MSG_V01_MAX_MSG_LEN 21
extern struct elem_info wlfw_initiate_cal_update_ind_msg_v01_ei[];

struct wlfw_cal_update_req_msg_v01 {
	enum wlfw_cal_temp_id_enum_v01 cal_id;
	u32 seg_id;
};

#define WLFW_CAL_UPDATE_REQ_MSG_V01_MAX_MSG_LEN 14
extern struct elem_info wlfw_cal_update_req_msg_v01_ei[];

struct wlfw_cal_update_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 file_id_valid;
	enum wlfw_cal_temp_id_enum_v01 file_id;
	u8 total_size_valid;
	u32 total_size;
	u8 seg_id_valid;
	u32 seg_id;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_DATA_SIZE_V01];
	u8 end_valid;
	u8 end;
	u8 cal_data_location_valid;
	u32 cal_data_location;
};

#define WLFW_CAL_UPDATE_RESP_MSG_V01_MAX_MSG_LEN 6188
extern struct elem_info wlfw_cal_update_resp_msg_v01_ei[];

struct wlfw_msa_info_req_msg_v01 {
	u64 msa_addr;
	u32 size;
};

#define WLFW_MSA_INFO_REQ_MSG_V01_MAX_MSG_LEN 18
extern struct elem_info wlfw_msa_info_req_msg_v01_ei[];

struct wlfw_msa_info_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u32 mem_region_info_len;
	struct wlfw_memory_region_info_s_v01
		mem_region_info[QMI_WLFW_MAX_NUM_MEMORY_REGIONS_V01];
};

#define WLFW_MSA_INFO_RESP_MSG_V01_MAX_MSG_LEN 37
extern struct elem_info wlfw_msa_info_resp_msg_v01_ei[];

struct wlfw_msa_ready_req_msg_v01 {
	char placeholder;
};

#define WLFW_MSA_READY_REQ_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_msa_ready_req_msg_v01_ei[];

struct wlfw_msa_ready_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_MSA_READY_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_msa_ready_resp_msg_v01_ei[];

struct wlfw_ini_req_msg_v01 {
	u8 enablefwlog_valid;
	u8 enablefwlog;
};

#define WLFW_INI_REQ_MSG_V01_MAX_MSG_LEN 4
extern struct elem_info wlfw_ini_req_msg_v01_ei[];

struct wlfw_ini_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_INI_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_ini_resp_msg_v01_ei[];

struct wlfw_athdiag_read_req_msg_v01 {
	u32 offset;
	u32 mem_type;
	u32 data_len;
};

#define WLFW_ATHDIAG_READ_REQ_MSG_V01_MAX_MSG_LEN 21
extern struct elem_info wlfw_athdiag_read_req_msg_v01_ei[];

struct wlfw_athdiag_read_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_ATHDIAG_DATA_SIZE_V01];
};

#define WLFW_ATHDIAG_READ_RESP_MSG_V01_MAX_MSG_LEN 6156
extern struct elem_info wlfw_athdiag_read_resp_msg_v01_ei[];

struct wlfw_athdiag_write_req_msg_v01 {
	u32 offset;
	u32 mem_type;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_ATHDIAG_DATA_SIZE_V01];
};

#define WLFW_ATHDIAG_WRITE_REQ_MSG_V01_MAX_MSG_LEN 6163
extern struct elem_info wlfw_athdiag_write_req_msg_v01_ei[];

struct wlfw_athdiag_write_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_ATHDIAG_WRITE_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_athdiag_write_resp_msg_v01_ei[];

struct wlfw_vbatt_req_msg_v01 {
	u64 voltage_uv;
};

#define WLFW_VBATT_REQ_MSG_V01_MAX_MSG_LEN 11
extern struct elem_info wlfw_vbatt_req_msg_v01_ei[];

struct wlfw_vbatt_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_VBATT_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_vbatt_resp_msg_v01_ei[];

struct wlfw_mac_addr_req_msg_v01 {
	u8 mac_addr_valid;
	u8 mac_addr[QMI_WLFW_MAC_ADDR_SIZE_V01];
};

#define WLFW_MAC_ADDR_REQ_MSG_V01_MAX_MSG_LEN 9
extern struct elem_info wlfw_mac_addr_req_msg_v01_ei[];

struct wlfw_mac_addr_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_MAC_ADDR_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_mac_addr_resp_msg_v01_ei[];

#define WLFW_HOST_CAP_INTERNAL_SLEEPCLOCK_MASK 0x02
struct wlfw_host_cap_req_msg_v01 {
	u8 num_clients_valid;
	u32 num_clients;
	u8 wake_msi_valid;
	u32 wake_msi;
	u8 gpios_valid;
	u32 gpios_len;
	u32 gpios[QMI_WLFW_MAX_NUM_GPIO_V01];
	u8 nm_modem_valid;
	u8 nm_modem;
	u8 bdf_support_valid;
	u8 bdf_support;
	u8 bdf_cache_support_valid;
	u8 bdf_cache_support;
	u8 m3_support_valid;
	u8 m3_support;
	u8 m3_cache_support_valid;
	u8 m3_cache_support;
	u8 cal_filesys_support_valid;
	u8 cal_filesys_support;
	u8 cal_cache_support_valid;
	u8 cal_cache_support;
	u8 cal_done_valid;
	u8 cal_done;
	u8 mem_bucket_valid;
	u32 mem_bucket;
	u8 mem_cfg_mode_valid;
	u8 mem_cfg_mode;
	u8 cal_duration_valid;
	u16 cal_duration;
	u8 platform_name_valid;
	char platform_name[QMI_WLFW_MAX_PLATFORM_NAME_LEN_V01 + 1];
	u8 ddr_range_valid;
	struct wlfw_host_ddr_range_s_v01 ddr_range[QMI_WLFW_MAX_HOST_DDR_RANGE_SIZE_V01];
	u8 host_build_type_valid;
	enum wlfw_host_build_type_v01 host_build_type;
	u8 mlo_capable_valid;
	u8 mlo_capable;
	u8 mlo_chip_id_valid;
	u16 mlo_chip_id;
	u8 mlo_group_id_valid;
	u8 mlo_group_id;
	u8 max_mlo_peer_valid;
	u16 max_mlo_peer;
	u8 mlo_num_chips_valid;
	u8 mlo_num_chips;
	u8 mlo_chip_info_valid;
	struct mlo_chip_info_s_v01 mlo_chip_info[QMI_WLFW_MAX_MLO_CHIP_V01];
	u8 feature_list_valid;
	u64 feature_list;
	u8 num_wlan_clients_valid;
	u16 num_wlan_clients;
	u8 num_wlan_vaps_valid;
	u8 num_wlan_vaps;
	u8 wake_msi_addr_valid;
	u32 wake_msi_addr;
	u8 wlan_enable_delay_valid;
	u32 wlan_enable_delay;
	u8 ddr_type_valid;
	u32 ddr_type;
	u8 gpio_info_valid;
	u32 gpio_info_len;
	u32 gpio_info[QMI_WLFW_MAX_NUM_GPIO_INFO_V01];
	u8 fw_ini_cfg_support_valid;
	u8 fw_ini_cfg_support;
	u8 mlo_chip_v2_info_valid;
	struct mlo_chip_v2_info_s_v01 mlo_chip_v2_info[QMI_WLFW_MLO_V2_CHP_V01];
	u8 pcie_link_info_valid;
	struct wlfw_host_pcie_link_info_s_v01 pcie_link_info;
};
#define WLFW_HOST_CAP_REQ_MSG_V01_MAX_MSG_LEN 581
extern struct elem_info wlfw_host_cap_req_msg_v01_ei[];

struct wlfw_host_cap_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_HOST_CAP_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_host_cap_resp_msg_v01_ei[];

struct wlfw_request_mem_ind_msg_v01 {
	u32 mem_seg_len;
	struct wlfw_mem_seg_s_v01 mem_seg[QMI_WLFW_MAX_NUM_MEM_SEG_V01];
};

#define WLFW_REQUEST_MEM_IND_MSG_V01_MAX_MSG_LEN 1124
extern struct elem_info wlfw_request_mem_ind_msg_v01_ei[];

struct wlfw_respond_mem_req_msg_v01 {
	u32 mem_seg_len;
	struct wlfw_mem_seg_resp_s_v01 mem_seg[QMI_WLFW_MAX_NUM_MEM_SEG_V01];
};

#define WLFW_RESPOND_MEM_REQ_MSG_V01_MAX_MSG_LEN 548
extern struct elem_info wlfw_respond_mem_req_msg_v01_ei[];

struct wlfw_respond_mem_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_RESPOND_MEM_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_respond_mem_resp_msg_v01_ei[];

struct wlfw_fw_mem_ready_ind_msg_v01 {
	char placeholder;
};

#define WLFW_FW_MEM_READY_IND_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_fw_mem_ready_ind_msg_v01_ei[];

struct wlfw_fw_init_done_ind_msg_v01 {
	char placeholder;
};

#define WLFW_FW_INIT_DONE_IND_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_fw_init_done_ind_msg_v01_ei[];

struct wlfw_rejuvenate_ind_msg_v01 {
	u8 cause_for_rejuvenation_valid;
	u8 cause_for_rejuvenation;
	u8 requesting_sub_system_valid;
	u8 requesting_sub_system;
	u8 line_number_valid;
	u16 line_number;
	u8 function_name_valid;
	char function_name[QMI_WLFW_FUNCTION_NAME_LEN_V01 + 1];
};

#define WLFW_REJUVENATE_IND_MSG_V01_MAX_MSG_LEN 144
extern struct elem_info wlfw_rejuvenate_ind_msg_v01_ei[];

struct wlfw_rejuvenate_ack_req_msg_v01 {
	char placeholder;
};

#define WLFW_REJUVENATE_ACK_REQ_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_rejuvenate_ack_req_msg_v01_ei[];

struct wlfw_rejuvenate_ack_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_REJUVENATE_ACK_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_rejuvenate_ack_resp_msg_v01_ei[];

struct wlfw_dynamic_feature_mask_req_msg_v01 {
	u8 mask_valid;
	u64 mask;
};

#define WLFW_DYNAMIC_FEATURE_MASK_REQ_MSG_V01_MAX_MSG_LEN 11
extern struct elem_info wlfw_dynamic_feature_mask_req_msg_v01_ei[];

struct wlfw_dynamic_feature_mask_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 prev_mask_valid;
	u64 prev_mask;
	u8 curr_mask_valid;
	u64 curr_mask;
};

#define WLFW_DYNAMIC_FEATURE_MASK_RESP_MSG_V01_MAX_MSG_LEN 29
extern struct elem_info wlfw_dynamic_feature_mask_resp_msg_v01_ei[];

struct wlfw_m3_info_req_msg_v01 {
	u64 addr;
	u32 size;
};

#define WLFW_M3_INFO_REQ_MSG_V01_MAX_MSG_LEN 18
extern struct elem_info wlfw_m3_info_req_msg_v01_ei[];

struct wlfw_m3_info_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_M3_INFO_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_m3_info_resp_msg_v01_ei[];

struct wlfw_xo_cal_ind_msg_v01 {
	u8 xo_cal_data;
};

#define WLFW_XO_CAL_IND_MSG_V01_MAX_MSG_LEN 4
extern struct elem_info wlfw_xo_cal_ind_msg_v01_ei[];

struct wlfw_cal_done_ind_msg_v01 {
	char placeholder;
};

#define WLFW_CAL_DONE_IND_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_cal_done_ind_msg_v01_ei[];

struct wlfw_device_info_req_msg_v01 {
	char placeholder;
};

#define WLFW_DEVICE_INFO_REQ_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_device_info_req_msg_v01_ei[];

struct wlfw_device_info_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
	u8 bar_addr_valid;
	u64 bar_addr;
	u8 bar_size_valid;
	u32 bar_size;
};

#define WLFW_DEVICE_INFO_RESP_MSG_V01_MAX_MSG_LEN 25
extern struct elem_info wlfw_device_info_resp_msg_v01_ei[];

struct wlfw_m3_dump_upload_req_ind_msg_v01 {
	u32 pdev_id;
	u64 addr;
	u64 size;
};

#define WLFW_M3_DUMP_UPLOAD_REQ_IND_MSG_V01_MAX_MSG_LEN 29
extern struct elem_info wlfw_m3_dump_upload_req_ind_msg_v01_ei[];

struct wlfw_m3_dump_upload_done_req_msg_v01 {
	u32 pdev_id;
	u32 status;
};

#define WLFW_M3_DUMP_UPLOAD_DONE_REQ_MSG_V01_MAX_MSG_LEN 14
extern struct elem_info wlfw_m3_dump_upload_done_req_msg_v01_ei[];

struct wlfw_m3_dump_upload_done_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_M3_DUMP_UPLOAD_DONE_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_m3_dump_upload_done_resp_msg_v01_ei[];

struct wlfw_soc_wake_req_msg_v01 {
	u8 wake_valid;
	enum wlfw_soc_wake_enum_v01 wake;
};

#define WLFW_SOC_WAKE_REQ_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_soc_wake_req_msg_v01_ei[];

struct wlfw_soc_wake_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};
#define WLFW_SOC_WAKE_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_soc_wake_resp_msg_v01_ei[];

struct wlfw_exit_power_save_req_msg_v01 {
	char placeholder;
};

#define WLFW_EXIT_POWER_SAVE_REQ_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_exit_power_save_req_msg_v01_ei[];

struct wlfw_exit_power_save_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_EXIT_POWER_SAVE_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_exit_power_save_resp_msg_v01_ei[];

struct wlfw_wfc_call_twt_config_ind_msg_v01 {
	u8 twt_sta_start_valid;
	u64 twt_sta_start;
	u8 twt_sta_int_valid;
	u16 twt_sta_int;
	u8 twt_sta_upo_valid;
	u16 twt_sta_upo;
	u8 twt_sta_sp_valid;
	u16 twt_sta_sp;
	u8 twt_sta_dl_valid;
	u16 twt_sta_dl;
	u8 twt_sta_config_changed_valid;
	u8 twt_sta_config_changed;
};

#define WLFW_WFC_CALL_TWT_CONFIG_IND_MSG_V01_MAX_MSG_LEN 35
extern struct elem_info wlfw_wfc_call_twt_config_ind_msg_v01_ei[];

struct wlfw_qdss_mem_ready_ind_msg_v01 {
	char placeholder;
};

#define WLFW_QDSS_MEM_READY_IND_MSG_V01_MAX_MSG_LEN 0
extern struct elem_info wlfw_qdss_mem_ready_ind_msg_v01_ei[];

struct wlfw_pcie_gen_switch_req_msg_v01 {
	enum wlfw_pcie_gen_speed_v01 pcie_speed;
};

#define WLFW_PCIE_GEN_SWITCH_REQ_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_pcie_gen_switch_req_msg_v01_ei[];

struct wlfw_pcie_gen_switch_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};

#define WLFW_PCIE_GEN_SWITCH_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_pcie_gen_switch_resp_msg_v01_ei[];

struct wlfw_qdss_trace_config_download_req_msg_v01 {
	u8 total_size_valid;
	u32 total_size;
	u8 seg_id_valid;
	u32 seg_id;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_WLFW_MAX_DATA_SIZE_V01];
	u8 end_valid;
	u8 end;
};
#define WLFW_QDSS_TRACE_CONFIG_DOWNLOAD_REQ_MSG_V01_MAX_MSG_LEN 6167
extern struct elem_info wlfw_qdss_trace_config_download_req_msg_v01_ei[];

struct wlfw_qdss_trace_config_download_resp_msg_v01 {
	struct qmi_response_type_v01 resp;
};
#define WLFW_QDSS_TRACE_CONFIG_DOWNLOAD_RESP_MSG_V01_MAX_MSG_LEN 7
extern struct elem_info wlfw_qdss_trace_config_download_resp_msg_v01_ei[];
#endif
