KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
KBUILD_OPTIONS +=CONFIG_MSM_DIAG_INTERFACE=m CONFIG_MSM_QMI_INTERFACE=m CONFIG_IPC_ROUTER=m CONFIG_IPC_ROUTER_SECURITY=y \
                 CONFIG_QMI_ENCDEC=y CONFIG_QMI_ENCDEC_DEBUG=y CONFIG_CNSS2=m CONFIG_CNSS2_DEBUG=y CONFIG_NAPIER_X86=y

interface_type ?= pcie
emulation_build ?= 0

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

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
