KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

interface_type ?= pcie
emulation_build ?= 0
unified_driver ?= 0
unified_prealloc ?= 0

ifeq ($(unified_driver), 1)
M?= $(shell pwd)
KBUILD_OPTIONS += ROOTDIR=$(shell cd $(KERNEL_SRC); readlink -e $(M))
KBUILD_OPTIONS += MODNAME?=wlan_cnss_core_$(interface_type)
KBUILD_OPTIONS += WLAN_CNSSCORE=m
KBUILD_OPTIONS += CONFIG_WLAN_CNSS_CORE=y
KBUILD_OPTIONS += CONFIG_MSM_DIAG_INTERFACE=y CONFIG_MSM_QMI_INTERFACE=y CONFIG_IPC_ROUTER=y CONFIG_IPC_ROUTER_SECURITY=y \
                  CONFIG_QMI_ENCDEC=y CONFIG_QMI_ENCDEC_DEBUG=y CONFIG_CNSS2=y CONFIG_CNSS2_DEBUG=y CONFIG_NAPIER_X86=y   \
                  CONFIG_CNSS_UTILS=y

ifeq ($(interface_type), pcie)
KBUILD_OPTIONS += CONFIG_MHI_XPRT=y CONFIG_DIAG_MHI=y CONFIG_CNSS2_PCIE=y CONFIG_MSM_MHI=y
ifeq ($(emulation_build), 1)
KBUILD_OPTIONS += CONFIG_PCIE_EMULATION=y
endif
endif
ifeq ($(interface_type), usb)
KBUILD_OPTIONS += CONFIG_HSIC_XPRT=y CONFIG_USB_QTI_KS_BRIDGE=y CONFIG_DIAG_HSIC=y CONFIG_DIAG_IPC_BRIDGE=y CONFIG_CNSS2_USB=y
ifeq ($(emulation_build), 1)
KBUILD_OPTIONS += CONFIG_USB_EMULATION=y
endif
endif
ifeq ($(interface_type), sdio)
KBUILD_OPTIONS += CONFIG_SDIO_XPRT=y CONFIG_QCN=y CONFIG_DIAG_SDIO=y CONFIG_QTI_SDIO_CLIENT=y CONFIG_CNSS2_SDIO=y
endif


else #unified_driver 0

KBUILD_OPTIONS +=CONFIG_MSM_DIAG_INTERFACE=m CONFIG_MSM_QMI_INTERFACE=m CONFIG_IPC_ROUTER=m CONFIG_IPC_ROUTER_SECURITY=y \
                 CONFIG_QMI_ENCDEC=y CONFIG_QMI_ENCDEC_DEBUG=y CONFIG_CNSS2=m CONFIG_CNSS2_DEBUG=y CONFIG_NAPIER_X86=y   \
                 CONFIG_CNSS_UTILS=m

ifeq ($(interface_type), pcie)
KBUILD_OPTIONS += CONFIG_MHI_XPRT=m CONFIG_DIAG_MHI=y CONFIG_CNSS2_PCIE=y CONFIG_MSM_MHI=m
ifeq ($(emulation_build), 1)
KBUILD_OPTIONS += CONFIG_PCIE_EMULATION=y
endif
endif
ifeq ($(interface_type), usb)
KBUILD_OPTIONS += CONFIG_HSIC_XPRT=m CONFIG_USB_QTI_KS_BRIDGE=m CONFIG_DIAG_HSIC=y CONFIG_DIAG_IPC_BRIDGE=m CONFIG_CNSS2_USB=y
ifeq ($(emulation_build), 1)
KBUILD_OPTIONS += CONFIG_USB_EMULATION=y
endif
endif
ifeq ($(interface_type), sdio)
KBUILD_OPTIONS += CONFIG_SDIO_XPRT=m CONFIG_QCN=m CONFIG_DIAG_SDIO=y CONFIG_QTI_SDIO_CLIENT=m CONFIG_CNSS2_SDIO=y
endif
endif #unified_driver end

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) clean
