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

#include "bus.h"
#include "debug.h"
#include "pci.h"
#include "usb.h"
#include "sdio.h"

enum cnss_dev_bus_type cnss_get_dev_bus_type(struct device *dev)
{
	if (!dev)
		return CNSS_BUS_NONE;

	if (!dev->bus)
		return CNSS_BUS_NONE;

	if (memcmp(dev->bus->name, "pci", 3) == 0)
		return CNSS_BUS_PCI;
	else if (memcmp(dev->bus->name, "usb", 3) == 0)
		return CNSS_BUS_USB;
	else if (memcmp(dev->bus->name, "sdio", 4) == 0)
		return CNSS_BUS_SDIO;
	else
		return CNSS_BUS_NONE;
}

enum cnss_dev_bus_type cnss_get_bus_type(unsigned long device_id)
{
	switch (device_id) {
	case QCA6174_DEVICE_ID:
	case QCA6290_EMULATION_DEVICE_ID:
	case QCA6290_DEVICE_ID:
	case QCA6390_DEVICE_ID:
	case QCA6490_DEVICE_ID:
	case QCN7605_DEVICE_ID:
		return CNSS_BUS_PCI;
	case QCN7605_COMPOSITE_DEVICE_ID:
	case QCN7605_STANDALONE_DEVICE_ID:
	case QCN7605_VER20_STANDALONE_DEVICE_ID:
	case QCN7605_VER20_COMPOSITE_DEVICE_ID:
		return CNSS_BUS_USB;
	case QCN7605_SDIO_DEVICE_ID:
		return CNSS_BUS_SDIO;
	default:
		cnss_pr_err("Unknown device_id: 0x%lx\n", device_id);
		return CNSS_BUS_NONE;
	}
}

bool cnss_bus_req_mem_ind_valid(struct cnss_plat_data *plat_priv)
{
	enum cnss_dev_bus_type bus_type = cnss_get_bus_type(plat_priv->device_id);
	
	if (bus_type == CNSS_BUS_USB || bus_type == CNSS_BUS_SDIO)
		return false;
	else
		return true;
}

void *cnss_bus_dev_to_bus_priv(struct device *dev)
{
	if (!dev)
		return NULL;

	switch (cnss_get_dev_bus_type(dev)) {
	case CNSS_BUS_PCI:
		return cnss_get_pci_priv(to_pci_dev(dev));
	case CNSS_BUS_USB:
		return cnss_get_usb_priv(to_usb_interface(dev));
	default:
		return NULL;
	}
}

struct cnss_plat_data *cnss_bus_dev_to_plat_priv(struct device *dev)
{
	void *bus_priv;

	if (!dev)
		return cnss_get_plat_priv(NULL);

	bus_priv = cnss_bus_dev_to_bus_priv(dev);
	if (!bus_priv)
		return NULL;

	switch (cnss_get_dev_bus_type(dev)) {
	case CNSS_BUS_PCI:
		return cnss_pci_priv_to_plat_priv(bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_priv_to_plat_priv(bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_get_plat_priv(NULL);
	default:
		return NULL;
	}
}

int cnss_bus_init(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_init(plat_priv);
	case CNSS_BUS_USB:
		return cnss_usb_init(plat_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_init(plat_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

void cnss_bus_deinit(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		cnss_pci_deinit(plat_priv);
		break;
	case CNSS_BUS_USB:
		cnss_usb_deinit(plat_priv);
		break;
	case CNSS_BUS_SDIO:
		cnss_sdio_deinit(plat_priv);
		break;
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
	}
	return;
}

int cnss_bus_load_m3(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_load_m3(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_free_m3(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		cnss_pci_free_m3_mem(plat_priv->bus_priv);
		return 0;
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_alloc_fw_mem(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_alloc_fw_mem(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_get_wake_irq(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_get_wake_msi(plat_priv->bus_priv);
	default:
		return -EINVAL;
	}
}

int cnss_bus_force_fw_assert_hdlr(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_force_fw_assert_hdlr(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
void cnss_bus_fw_boot_timeout_hdlr(struct timer_list *t)
#else
void cnss_bus_fw_boot_timeout_hdlr(unsigned long data)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
	struct cnss_plat_data *plat_priv = from_timer(plat_priv, t, fw_boot_timer);
#else
	struct cnss_plat_data *plat_priv = (struct cnss_plat_data *)data;
#endif
	if (!plat_priv)
		return;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_fw_boot_timeout_hdlr(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_fw_boot_timeout_hdlr(plat_priv->bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_fw_boot_timeout_hdlr(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return;
	}
}

void cnss_bus_collect_dump_info(struct cnss_plat_data *plat_priv)
{
	int ret;

	if (!plat_priv)
		return;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		ret = cnss_pci_set_mhi_state(plat_priv->bus_priv,
					     CNSS_MHI_RDDM);
		if (ret) {
			cnss_pr_err("Failed to complete RDDM, err = %d\n", ret);
			cnss_dump_fw_sram_to_file(plat_priv);
			cnss_pci_dump_fw_remote_mem_to_file(plat_priv->bus_priv);
			cnss_pci_dump_fw_paging_to_file(plat_priv->bus_priv);
			break;
		}
		return cnss_pci_collect_dump_info(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return;
	}
}

int cnss_bus_call_driver_probe(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_call_driver_probe(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_call_driver_probe(plat_priv->bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_call_driver_probe(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_call_driver_remove(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_call_driver_remove(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_call_driver_remove(plat_priv->bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_call_driver_remove(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_dev_powerup(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_dev_powerup(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_dev_powerup(plat_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_dev_powerup(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_dev_shutdown(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_dev_shutdown(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_dev_shutdown(plat_priv->bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_dev_shutdown(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_dev_crash_shutdown(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_dev_crash_shutdown(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

#ifndef CONFIG_NAPIER_X86
int cnss_bus_dev_ramdump(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_dev_ramdump(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}
#endif

int cnss_bus_register_driver_hdlr(struct cnss_plat_data *plat_priv, void *data)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_register_driver_hdlr(plat_priv->bus_priv, data);
	case CNSS_BUS_USB:
		return cnss_usb_register_driver_hdlr(plat_priv->bus_priv, data);
	case CNSS_BUS_SDIO:
		return cnss_sdio_register_driver_hdlr(plat_priv->bus_priv,
						      data);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_unregister_driver_hdlr(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_unregister_driver_hdlr(plat_priv->bus_priv);
	case CNSS_BUS_USB:
		return cnss_usb_unregister_driver_hdlr(plat_priv->bus_priv);
	case CNSS_BUS_SDIO:
		return cnss_sdio_unregister_driver_hdlr(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_call_driver_modem_status(struct cnss_plat_data *plat_priv,
				      int modem_current_status)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_call_driver_modem_status(plat_priv->bus_priv,
							 modem_current_status);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_recovery_update_status(struct cnss_plat_data *plat_priv)
{
	if (!plat_priv)
		return -ENODEV;

	switch (plat_priv->bus_type) {
	case CNSS_BUS_PCI:
		return cnss_pci_recovery_update_status(plat_priv->bus_priv);
	default:
		cnss_pr_err("Unsupported bus type: %d\n",
			    plat_priv->bus_type);
		return -EINVAL;
	}
}

int cnss_bus_fw_sram_dump_to_file(struct cnss_plat_data *plat_priv,
		uint32_t fw_sram_start,
		uint32_t fw_sram_end,
		const char *fw_sram_dump_path)
{
	int ret = 0;

	if (!plat_priv) {
		cnss_pr_err("plat_priv is NULL\n");
		return -ENODEV;
	}

	switch (cnss_get_bus_type(plat_priv->device_id)) {
		case CNSS_BUS_PCI:
			ret = cnss_pci_fw_sram_dump_to_file(
					plat_priv->bus_priv,
					fw_sram_start,
					fw_sram_end,
					fw_sram_dump_path);
			break;
		case CNSS_BUS_SDIO:
		case CNSS_BUS_USB:
		default:
			ret = -ENOTSUPP;
			break;
	}

	return ret;
}
