/* Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "cnss2.h"
#include "qcn_sdio_al.h"
#include "main.h"
#include "sdio.h"
#include "debug.h"
#include "bus.h"

int cnss_sdio_call_driver_probe(struct cnss_sdio_data *sdio_priv)
{
	int ret = 0;
	struct cnss_plat_data *plat_priv = sdio_priv->plat_priv;

	if ((!sdio_priv->al_client_handle) ||
	    (!sdio_priv->al_client_handle->func)) {
		ret = -ENODEV;
		goto out;
	}

	if (!sdio_priv->ops) {
		cnss_pr_err("driver_ops is NULL\n");
		ret = -EINVAL;
		goto out;
	}

	if (test_bit(CNSS_DRIVER_RECOVERY, &plat_priv->driver_state) &&
	    test_bit(CNSS_DRIVER_PROBED, &plat_priv->driver_state)) {
		ret = sdio_priv->ops->reinit(sdio_priv->al_client_handle->func,
					     &sdio_priv->device_id);
		if (ret) {
			cnss_pr_err("Failed to reinit host driver, err = %d\n",
				    ret);
			goto out;
		}
		clear_bit(CNSS_DRIVER_RECOVERY, &plat_priv->driver_state);
	} else if (test_bit(CNSS_DRIVER_LOADING, &plat_priv->driver_state)) {
		ret = sdio_priv->ops->probe(sdio_priv->al_client_handle->func,
					    &sdio_priv->device_id);
		if (ret) {
			cnss_pr_err("Failed to probe host driver, err = %d\n",
				    ret);
			goto out;
		}
		clear_bit(CNSS_DRIVER_RECOVERY, &plat_priv->driver_state);
		clear_bit(CNSS_DRIVER_LOADING, &plat_priv->driver_state);
		set_bit(CNSS_DRIVER_PROBED, &plat_priv->driver_state);
	}

	return 0;

out:
	return ret;
}

int cnss_sdio_call_driver_remove(struct cnss_sdio_data *sdio_priv)
{
	struct cnss_plat_data *plat_priv = sdio_priv->plat_priv;

	if (test_bit(CNSS_IN_COLD_BOOT_CAL, &plat_priv->driver_state) ||
	    test_bit(CNSS_FW_BOOT_RECOVERY, &plat_priv->driver_state) ||
	    test_bit(CNSS_DRIVER_DEBUG, &plat_priv->driver_state)) {
		cnss_pr_dbg("Skip driver remove\n");
		return 0;
	}

	if (!sdio_priv->ops) {
		cnss_pr_err("driver_ops is NULL\n");
		return -EINVAL;
	}

	if (test_bit(CNSS_DRIVER_RECOVERY, &plat_priv->driver_state) &&
	    test_bit(CNSS_DRIVER_PROBED, &plat_priv->driver_state)) {
		cnss_pr_dbg("Recovery set after driver probed.Call shutdown\n");
		sdio_priv->ops->shutdown(sdio_priv->al_client_handle->func);
	} else if (test_bit(CNSS_DRIVER_UNLOADING, &plat_priv->driver_state)) {
		cnss_pr_dbg("driver_ops->remove\n");
		sdio_priv->ops->remove(sdio_priv->al_client_handle->func);
		clear_bit(CNSS_DRIVER_PROBED, &plat_priv->driver_state);
	}
	return 0;
}

int cnss_sdio_alloc_fw_mem(struct cnss_sdio_data *sdio_priv)
{
	struct cnss_plat_data *plat_priv = sdio_priv->plat_priv;
	struct cnss_fw_mem *fw_mem = plat_priv->fw_mem;
	int i;

	for (i = 0; i < plat_priv->fw_mem_seg_len; i++) {
		if (!fw_mem[i].va && fw_mem[i].size) {
			fw_mem[i].va = kzalloc(fw_mem[i].size, GFP_KERNEL);
			if (!fw_mem[i].va) {
				cnss_pr_err("Failed to allocate memory for FW, size: 0x%zx, type: %u\n",
					    fw_mem[i].size, fw_mem[i].type);

				goto alloc_fw_mem_err;
			}
			fw_mem[i].pa = virt_to_phys(fw_mem[i].va);

			cnss_pr_dbg("va %p pa %llx\n", fw_mem[i].va, fw_mem[i].pa);
		}
	}

	return 0;

alloc_fw_mem_err:
	cnss_sdio_free_fw_mem(sdio_priv);
	return -ENOMEM;
}

void cnss_sdio_free_fw_mem(struct cnss_sdio_data *sdio_priv)
{
	struct cnss_plat_data *plat_priv = sdio_priv->plat_priv;
	struct cnss_fw_mem *fw_mem = plat_priv->fw_mem;
	int i;

	for (i = 0; i < plat_priv->fw_mem_seg_len; i++) {
		if (fw_mem[i].va && fw_mem[i].size) {
			cnss_pr_dbg("Freeing memory for FW, va: 0x%pK, size: 0x%zx, type: %u\n",
				    fw_mem[i].va, fw_mem[i].size, fw_mem[i].type);
			kfree(fw_mem[i].va);
			fw_mem[i].va = NULL;
			fw_mem[i].pa = 0;
			fw_mem[i].size = 0;
			fw_mem[i].type = 0;
			fw_mem[i].pre_aligned = NULL;
			fw_mem[i].phys_addr = 0;
		}
	}

	plat_priv->fw_mem_seg_len = 0;
}

int cnss_sdio_force_fw_assert_hdlr(struct cnss_sdio_data *cnss_info)
{
	struct device *dev;
	if ((!cnss_info) ||
	    (!cnss_info->al_client_handle) ||
	    (!cnss_info->al_client_handle->func)) {
		cnss_pr_err("cnss_info is NULL\n");
		return -ENODEV;
	}
	dev = &cnss_info->al_client_handle->func->dev;
	return qcn_sdio_inject_sys_err_handle(dev);
}
/**
 * cnss_sdio_wlan_register_driver() - cnss wlan register API
 * @driver: sdio wlan driver interface from wlan driver.
 *
 * wlan sdio function driver uses this API to register callback
 * functions to cnss_sido platform driver. The callback will
 * be invoked by corresponding wrapper function of this cnss
 * platform driver.
 */
int cnss_sdio_wlan_register_driver(struct cnss_sdio_wlan_driver *driver_ops)
{
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);
	struct cnss_sdio_data *cnss_info;
	int ret = 0;

	if (!plat_priv) {
		cnss_pr_err("plat_priv is NULL\n");
		return -ENODEV;
	}

	cnss_info = plat_priv->bus_priv;
	if ((!cnss_info) ||
	    (!cnss_info->al_client_handle) ||
	    (!cnss_info->al_client_handle->func)) {
		cnss_pr_err("cnss_info is NULL\n");
		return -ENODEV;
	}

	if (cnss_info->ops) {
		cnss_pr_err("Driver has already registered\n");
		return -EEXIST;
	}
	cnss_info->ops = driver_ops;

	ret = cnss_driver_event_post(plat_priv,
				     CNSS_DRIVER_EVENT_REGISTER_DRIVER,
				     CNSS_EVENT_SYNC_UNINTERRUPTIBLE,
				     driver_ops);
	return ret;
}
EXPORT_SYMBOL(cnss_sdio_wlan_register_driver);

/**
 * cnss_sdio_wlan_unregister_driver() - cnss wlan unregister API
 * @driver: sdio wlan driver interface from wlan driver.
 *
 * wlan sdio function driver uses this API to detach it from cnss_sido
 * platform driver.
 */
void cnss_sdio_wlan_unregister_driver(struct cnss_sdio_wlan_driver *driver_ops)
{
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);

	if (!plat_priv) {
		cnss_pr_err("plat_priv is NULL\n");
		return;
	}

	cnss_driver_event_post(plat_priv, CNSS_DRIVER_EVENT_UNREGISTER_DRIVER,
			       CNSS_EVENT_SYNC_UNINTERRUPTIBLE, NULL);
}
EXPORT_SYMBOL(cnss_sdio_wlan_unregister_driver);

struct sdio_al_client_handle *cnss_sdio_wlan_get_sdio_al_client_handle(
				struct sdio_func *func)
{
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);
	struct cnss_sdio_data *cnss_info = plat_priv->bus_priv;

	return cnss_info->al_client_handle;
}
EXPORT_SYMBOL(cnss_sdio_wlan_get_sdio_al_client_handle);

struct sdio_al_channel_handle *cnss_sdio_wlan_register_sdio_al_channel(
			     struct sdio_al_channel_data *channel_data)
{
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);
	struct cnss_sdio_data *cnss_info = plat_priv->bus_priv;

	return sdio_al_register_channel(cnss_info->al_client_handle,
					channel_data);
}
EXPORT_SYMBOL(cnss_sdio_wlan_register_sdio_al_channel);

void cnss_sdio_wlan_unregister_sdio_al_channel(
				     struct sdio_al_channel_handle *ch_handle)
{
	sdio_al_deregister_channel(ch_handle);
}
EXPORT_SYMBOL(cnss_sdio_wlan_unregister_sdio_al_channel);

int cnss_sdio_register_driver_hdlr(struct cnss_sdio_data *cnss_info,
				   void *data)
{
	struct cnss_plat_data *plat_priv = cnss_info->plat_priv;
	int ret = 0;
	unsigned int timeout;

	set_bit(CNSS_DRIVER_LOADING, &plat_priv->driver_state);
	if (test_bit(CNSS_FW_READY, &plat_priv->driver_state)) {
		cnss_pr_info("CNSS SDIO driver register in FW_Ready state");
		cnss_sdio_call_driver_probe(cnss_info);
	} else if ((*cnss_get_qmi_bypass()) &&
		(cnss_info->al_client_handle->func)) {
		cnss_pr_info("qmi bypass enabled");
		cnss_sdio_call_driver_probe(cnss_info);
	} else {
		cnss_pr_info("Wait for FW_Ready");
		ret = cnss_power_on_device(plat_priv);
		if (ret) {
			cnss_pr_err("Failed to power on device, err = %d\n",
				    ret);
			return ret;
		}

		qcn_sdio_card_state(true);
		timeout = cnss_get_qmi_timeout();
		if (timeout) {
			mod_timer(&plat_priv->fw_boot_timer,
				  jiffies + msecs_to_jiffies(timeout << 1));
		}
	}
	return 0;
}

int cnss_sdio_unregister_driver_hdlr(struct cnss_sdio_data *cnss_info)
{
	struct cnss_plat_data *plat_priv = cnss_info->plat_priv;
	bool sdio_reset = 0;

	set_bit(CNSS_DRIVER_UNLOADING, &plat_priv->driver_state);
	cnss_sdio_call_driver_remove(cnss_info);
	cnss_request_bus_bandwidth(&plat_priv->plat_dev->dev,
				   CNSS_BUS_WIDTH_NONE);
	/*
	 * Block SDIO card reset while RDDM dump collection is in progress.
	 * fw_rddm_waiting_thread will perform the reset after RDDM completes.
	 */
	if (qcn_rddm_is_processing()) {
		pr_err("[%s:%d] rddm in processing, skip sdio card reset\n", __func__, __LINE__);
	} else {
		if(!qcn_sdio_card_state(false))
			sdio_reset = true;
	}

	cnss_power_off_device(plat_priv);
	clear_bit(CNSS_FW_READY, &plat_priv->driver_state);
	clear_bit(CNSS_DRIVER_UNLOADING, &plat_priv->driver_state);
	if (sdio_reset)
		qcn_sdio_card_state(true);

	cnss_info->ops = NULL;
	return 0;
}

int cnss_sdio_dev_powerup(struct cnss_sdio_data *cnss_info)
{
	struct cnss_plat_data *plat_priv = cnss_info->plat_priv;
	int ret = 0;
	unsigned int timeout;

	switch (plat_priv->device_id) {
	case QCN7605_SDIO_DEVICE_ID:
		ret = cnss_power_on_device(plat_priv);
		if (ret) {
			cnss_pr_err("Failed to power on device, err = %d\n",
				    ret);
			goto out;
		}

		qcn_sdio_card_state(true);
		timeout = cnss_get_qmi_timeout();
		mod_timer(&plat_priv->fw_boot_timer,
			  jiffies + msecs_to_jiffies(timeout >> 1));
		cnss_set_pin_connect_status(plat_priv);
		break;
	default:
		cnss_pr_err("Unknown device_id found: 0x%lx\n",
			    plat_priv->device_id);
		ret = -ENODEV;
	}
out:
	return ret;
}

int cnss_sdio_dev_shutdown(struct cnss_sdio_data *cnss_info)
{
	struct cnss_plat_data *plat_priv = cnss_info->plat_priv;

	cnss_sdio_call_driver_remove(cnss_info);
	cnss_request_bus_bandwidth(&plat_priv->plat_dev->dev,
				   CNSS_BUS_WIDTH_NONE);
	/*qcn_sdio_card_state(false);*/
	cnss_power_off_device(plat_priv);
	clear_bit(CNSS_FW_READY, &plat_priv->driver_state);
	clear_bit(CNSS_DRIVER_UNLOADING, &plat_priv->driver_state);

	return 0;
}

void cnss_sdio_fw_boot_timeout_hdlr(void *bus_priv)
{
    cnss_pr_err("Timeout waiting for FW ready indication\n");
}

static int cnss_sdio_probe(struct sdio_al_client_handle *pal_cli_handle)
{
	struct cnss_sdio_data *sdio_info = pal_cli_handle->client_priv;
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);

	sdio_info->device_id.class = pal_cli_handle->func->class;
	sdio_info->device_id.vendor = pal_cli_handle->func->vendor;
	sdio_info->device_id.device = pal_cli_handle->func->device;

	if (pal_cli_handle->func)
		cnss_pr_info("CNSS SDIO AL Probe for device Id: 0x%x\n",
			     pal_cli_handle->func->device);
	clear_bit(CNSS_DEV_REMOVED, &plat_priv->driver_state);
	plat_priv->device_id = pal_cli_handle->func->device;
	cnss_register_subsys(sdio_info->plat_priv);
	return 0;
}

static int cnss_sdio_remove(struct sdio_al_client_handle *pal_cli_handle)
{
	struct cnss_sdio_data *sdio_info = pal_cli_handle->client_priv;
	struct cnss_plat_data *plat_priv = sdio_info->plat_priv;

	cnss_sdio_free_fw_mem(sdio_info);

	if (pal_cli_handle->func)
		cnss_pr_err(
		"SDIO AL remove for device Id: 0x%x in driver state %lu\n",
		pal_cli_handle->func->device,
		plat_priv->driver_state);

	clear_bit(CNSS_FW_READY, &plat_priv->driver_state);
	set_bit(CNSS_DEV_REMOVED, &plat_priv->driver_state);

	cnss_unregister_subsys(plat_priv);

	return 0;
}

static int cnss_sdio_pm(struct sdio_al_client_handle *pal_cli_handle,
			 enum sdio_al_lpm_event event)
{
	struct cnss_sdio_data *sdio_info = pal_cli_handle->client_priv;
	struct sdio_func *func = sdio_info->al_client_handle->func;
	int ret = 0;

	if (!sdio_info->ops) {
		cnss_pr_warn("Ignore LPM event\n");
		return 0;
	}

	if (event == LPM_ENTER) {
		cnss_pr_info("Entering LPM\n");
		ret = sdio_info->ops->suspend(&func->dev);
	} else {
		cnss_pr_info("Exiting LPM\n");
		ret = sdio_info->ops->resume(&func->dev);
	}

	return ret;
}

int cnss_sdio_notify_fw_down(struct sdio_al_client_handle *pal_cli_handle)
{
	struct cnss_sdio_data *sdio_info = pal_cli_handle->client_priv;
	struct cnss_plat_data *plat_priv = sdio_info->plat_priv;

	if (!sdio_info || !sdio_info->ops) {
		cnss_pr_err("[%s:%d] ops is null\n", __func__, __LINE__);
		return 0;
	}

	set_bit(CNSS_DEV_ERR_NOTIFY, &plat_priv->driver_state);

	if (test_bit(CNSS_DRIVER_PROBED, &plat_priv->driver_state))
		sdio_info->ops->update_status(sdio_info->al_client_handle->func,
					      CNSS_FW_DOWN);

	return 0;
}

int cnss_sdio_is_device_down(struct device *dev)
{
	struct cnss_plat_data *plat_priv = cnss_bus_dev_to_plat_priv(NULL);

	if (!plat_priv) {
		cnss_pr_err("plat_priv is NULL\n");
		return -ENODEV;
	}

	return test_bit(CNSS_DEV_ERR_NOTIFY, &plat_priv->driver_state);
}
EXPORT_SYMBOL(cnss_sdio_is_device_down);

struct sdio_al_client_data al_cli_data = {
	.name = "SDIO_AL_CLIENT_WLAN",
	.probe = cnss_sdio_probe,
	.remove = cnss_sdio_remove,
	.lpm_notify_cb = cnss_sdio_pm,
};

int cnss_sdio_init(struct cnss_plat_data *plat_priv)
{
	struct cnss_sdio_data *sdio_info;
	struct sdio_al_client_handle *al_client_handle;
	int ret = 0;

	if (sdio_al_is_ready()) {
		cnss_pr_err("sdio_al not ready, defer probe\n");
		ret = -EPROBE_DEFER;
		goto out;
	}

	al_client_handle = sdio_al_register_client(&al_cli_data);
	if (!al_client_handle) {
		cnss_pr_err("sdio al registration failed!\n");
		ret = -ENODEV;
		goto out;
	}
#ifndef CONFIG_NAPIER_X86
	sdio_info = devm_kzalloc(&plat_priv->plat_dev->dev, sizeof(*sdio_info),
				 GFP_KERNEL);
#else
	sdio_info = kzalloc(sizeof(*sdio_info), GFP_KERNEL);
#endif
	if (!sdio_info) {
		ret = -ENOMEM;
		goto out;
	}
	al_client_handle->client_priv = sdio_info;
	sdio_info->al_client_handle = al_client_handle;
	sdio_info->plat_priv = plat_priv;
	plat_priv->bus_priv = sdio_info;

out:
	return ret;
}

int cnss_sdio_deinit(struct cnss_plat_data *plat_priv)
{
	struct cnss_sdio_data *sdio_info = plat_priv->bus_priv;

	sdio_al_deregister_client(sdio_info->al_client_handle);
	return 0;
}
