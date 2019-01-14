ifneq ($(CONFIG_USB_QTI_KS_BRIDGE),)
     KBUILD_CPPFLAGS += -DCONFIG_USB_QTI_KS_BRIDGE
endif

ifneq ($(CONFIG_PCIE_EMULATION),)
     KBUILD_CPPFLAGS += -DCONFIG_PCIE_EMULATION
endif

ifneq ($(CONFIG_USB_EMULATION),)
     KBUILD_CPPFLAGS += -DCONFIG_USB_EMULATION
endif

ifneq ($(CONFIG_IPC_ROUTER),)
     KBUILD_CPPFLAGS += -DCONFIG_IPC_ROUTER
endif

ifneq ($(CONFIG_CNSS2_PCIE),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS2_PCIE
endif

ifneq ($(CONFIG_CNSS2_USB),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS2_USB
endif

ifneq ($(CONFIG_DIAG_IPC_BRIDGE),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_IPC_BRIDGE
endif

ifneq ($(CONFIG_DIAG_MHI),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_MHI
endif

ifneq ($(CONFIG_DIAG_HSIC),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_HSIC
endif

ifneq ($(CONFIG_MHI_XPRT),)
     KBUILD_CPPFLAGS += -DCONFIG_MHI_XPRT
endif

ifneq ($(CONFIG_HSIC_XPRT),)
     KBUILD_CPPFLAGS += -DCONFIG_HSIC_XPRT
endif

ifneq ($(CONFIG_IPC_ROUTER_SECURITY),)
     KBUILD_CPPFLAGS += -DCONFIG_IPC_ROUTER_SECURITY
endif

ifneq ($(CONFIG_MSM_MHI),)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_MHI
endif

ifneq ($(CONFIG_MSM_QMI_INTERFACE),)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_QMI_INTERFACE
endif

ifneq ($(CONFIG_MSM_DIAG_INTERFACE),)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_QMI_INTERFACE
endif

ifneq ($(CONFIG_CNSS2),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS2
endif

ifneq ($(CONFIG_CNSS2_DEBUG),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS2_DEBUG
endif

ifneq ($(CONFIG_QMI_ENCDEC),)
     KBUILD_CPPFLAGS += -DCONFIG_QMI_ENCDEC
endif

ifneq ($(CONFIG_QMI_ENCDEC_DEBUG),)
     KBUILD_CPPFLAGS += -DCONFIG_QMI_ENCDEC_DEBUG
endif

ifneq ($(CONFIG_NAPIER_X86),)
     KBUILD_CPPFLAGS += -DCONFIG_NAPIER_X86
endif

ifneq ($(CONFIG_WLAN_CNSS_CORE),)
    KBUILD_CPPFLAGS += -DCONFIG_WLAN_CNSS_CORE
endif

CDEFINES :=	-Wall\
		-Werror
KBUILD_CPPFLAGS += $(CDEFINES)

ifneq ($(CONFIG_WLAN_CNSS_CORE), y)
obj-$(CONFIG_USB_QTI_KS_BRIDGE) += ks_bridge/
obj-$(CONFIG_MSM_MHI) += mhi/
obj-$(CONFIG_IPC_ROUTER) += ipc_router/
obj-$(CONFIG_MHI_XPRT) += xprt/
obj-$(CONFIG_HSIC_XPRT) += hsic_xprt/
obj-$(CONFIG_MSM_QMI_INTERFACE) += qmi/
obj-$(CONFIG_MSM_DIAG_INTERFACE) += diag/
obj-$(CONFIG_CNSS2) += cnss2/
obj-$(CONFIG_DIAG_IPC_BRIDGE) += diag_ipc_bridge/

else

KS_BRIDGE_DIR := ks_bridge
MHI_DIR := mhi
IPC_ROUTER_DIR := ipc_router
XPRT_DIR := xprt
HSIC_XPRT_DIR := hsic_xprt
QMI_DIR := qmi
DIAG_DIR := diag
CNSS_DIR := cnss2
DIAG_IPC_BRIDGE_DIR := diag_ipc_bridge

INIT_OBJS := unified_wlan_cnsscore.o
INIT_INC := -I$(ROOTDIR)

ifneq ($(CONFIG_USB_QTI_KS_BRIDGE),)
	KS_BRIDGE_OBJS := $(KS_BRIDGE_DIR)/ks_bridge.o
	KS_BRIDGE_INC := -I$(KS_BRIDGE_DIR)/
endif

ifneq ($(CONFIG_MSM_MHI),)
	MHI_OBJS := $(MHI_DIR)/mhi_main.o                       \
				$(MHI_DIR)/mhi_iface.o          \
				$(MHI_DIR)/mhi_init.o           \
				$(MHI_DIR)/mhi_isr.o            \
				$(MHI_DIR)/mhi_mmio_ops.o       \
				$(MHI_DIR)/mhi_ring_ops.o       \
				$(MHI_DIR)/mhi_states.o         \
				$(MHI_DIR)/mhi_sys.o            \
				$(MHI_DIR)/mhi_bhi.o            \
				$(MHI_DIR)/mhi_pm.o             \
				$(MHI_DIR)/mhi_ssr.o            \
				$(MHI_DIR)/mhi_event.o

ifneq ($(CONFIG_NAPIER_X86),)
	MHI_OBJS += $(MHI_DIR)/mhi_fw_dump.o
endif
	MHI_INC := -I$(ROOTDIR)/$(MHI_DIR)
endif

ifneq ($(CONFIG_CNSS2),)
	CNSS_OBJS := $(CNSS_DIR)/main.o                           \
		     $(CNSS_DIR)/bus.o                            \
		     $(CNSS_DIR)/debug.o                          \
		     $(CNSS_DIR)/power.o                          \
		     $(CNSS_DIR)/qmi.o                            \
		     $(CNSS_DIR)/utils.o                          \
		     $(CNSS_DIR)/wlan_firmware_service_v01.o
ifeq ($(CONFIG_CNSS2_PCIE),y)
	CNSS_OBJS +=  $(CNSS_DIR)/pci.o
endif
ifeq ($(CONFIG_CNSS2_USB),y)
	CNSS_OBJS +=  $(CNSS_DIR)/usb.o
endif
	CNSS_INC := -I$(ROOTDIR)/$(CNSS_DIR)

endif

ifneq ($(CONFIG_MSM_DIAG_INTERFACE),)
	DIAG_OBJS := $(DIAG_DIR)/diagchar_core.o             \
		     $(DIAG_DIR)/diag_local.o                \
		     $(DIAG_DIR)/diagmem.o                   \
		     $(DIAG_DIR)/diagfwd_bridge.o            \
		     $(DIAG_DIR)/diag_nl.o

ifeq ($(CONFIG_DIAG_HSIC),y)
	DIAG_OBJS += $(DIAG_DIR)/diagfwd_hsic.o
endif
ifeq ($(CONFIG_DIAG_MHI),y)
	DIAG_OBJS += $(DIAG_DIR)/diagfwd_mhi.o
endif
	DIAG_INC := -I$(ROOTDIR)/$(DIAG_DIR)

endif

ifneq ($(CONFIG_DIAG_IPC_BRIDGE), )
	DIAG_IPC_BRIDGE_OBJS := $(DIAG_IPC_BRIDGE_DIR)/diag_ipc_bridge.o
endif

ifneq ($(CONFIG_IPC_ROUTER), )
	IPC_ROUTER_OBJS := $(IPC_ROUTER_DIR)/ipc_router_core.o          \
			   $(IPC_ROUTER_DIR)/ipc_router_socket.o        \
			   $(IPC_ROUTER_DIR)/ipc_router_security.o
	IPC_ROUTER_INC := -I$(ROOTDIR)/$(IPC_ROUTER_DIR)
endif

ifneq ($(CONFIG_HSIC_XPRT), )
	HSIC_XPERT_OBJS := $(HSIC_XPRT_DIR)/ipc_router_hsic_xprt.o
endif

ifneq ($(CONFIG_MHI_XPRT), )
	XPRT_OBJS := $(XPRT_DIR)/ipc_router_mhi_xprt.o
endif

ifneq ($(CONFIG_MSM_QMI_INTERFACE), )
	QMI_OBJS := $(QMI_DIR)/qmi_encdec.o        \
		    $(QMI_DIR)/qmi_interface.o
	QMI_INC := -I$(ROOTDIR)/$(QMI_DIR)
endif

OBJS := $(INIT_OBJS)                      \
	$(IPC_ROUTER_OBJS)                 \
	$(QMI_OBJS)                        \
	$(KS_BRIDGE_OBJS)                  \
	$(DIAG_IPC_BRIDGE_OBJS)            \
	$(XPRT_OBJS)                       \
	$(HSIC_XPERT_OBJS)                 \
	$(MHI_OBJS)                        \
	$(DIAG_OBJS)                       \
	$(CNSS_OBJS)                       \

INCS := $(INIT_INC)                     \
        $(CNSS_INC)                     \
        $(KS_BRIDGE_INC)                \
        $(MHI_INC)                      \
        $(DIAG_INC)                     \
        $(QMI_INC)                      \


cflags-y += $(INCS)
ccflags-y += -Os -I$(src)/inc -I$(src)/mhi

obj-$(WLAN_CNSSCORE) +=$(MODNAME).o
$(MODNAME)-y := $(OBJS)

endif
