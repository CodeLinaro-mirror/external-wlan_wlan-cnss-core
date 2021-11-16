ifeq ($(CONFIG_SINGLE_KO_FEATURE),y)

    CONFIG_WLAN_CNSS_CORE:=y

    ## it will affect performance for high latency interface
    ## by default, it is disabled
    CONFIG_MSM_DIAG_INTERFACE?=n

    CONFIG_MSM_QMI_INTERFACE:=y
    CONFIG_IPC_ROUTER:=y
    CONFIG_IPC_ROUTER_SECURITY:=y
    CONFIG_QMI_ENCDEC:=y
    CONFIG_QMI_ENCDEC_DEBUG:=y
    CONFIG_CNSS2:=y
    CONFIG_CNSS2_DEBUG:=y
    CONFIG_NAPIER_X86:=y

    ifeq ($(INTERFACE_TYPE), PCIE)
        CONFIG_MHI_XPRT:=y
        CONFIG_DIAG_MHI:=y
        CONFIG_CNSS2_PCIE:=y
        CONFIG_MSM_MHI:=y
        ifeq ($(EMULATION_BUILD),1)
            CONFIG_PCIE_EMULATION:=y
        endif
    endif

    ifeq ($(INTERFACE_TYPE), USB)
        CONFIG_HSIC_XPRT:=y
        CONFIG_USB_QTI_KS_BRIDGE:=y
        CONFIG_DIAG_HSIC:=y
        CONFIG_DIAG_IPC_BRIDGE:=y
        CONFIG_CNSS2_USB:=y

        ifeq ($(EMULATION_BUILD),1)
            CONFIG_USB_EMULATION:=y
        endif
    endif

    ifeq ($(INTERFACE_TYPE), SDIO)
        CONFIG_SDIO_XPRT:=y
        CONFIG_QCN:=y
        CONFIG_DIAG_SDIO:=y
        CONFIG_QTI_SDIO_CLIENT:=y
        CONFIG_CNSS2_SDIO:=y
    endif

    KBUILD_CPPFLAGS += -DCONFIG_SINGLE_KO_FEATURE

endif

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

ifneq ($(CONFIG_CNSS2_SDIO),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS2_SDIO
endif

ifneq ($(CONFIG_DIAG_IPC_BRIDGE),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_IPC_BRIDGE
endif

ifneq ($(CONFIG_DIAG_MHI),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_MHI
endif

ifneq ($(CONFIG_QCN),)
     KBUILD_CPPFLAGS += -DCONFIG_QCN
endif

ifneq ($(CONFIG_DIAG_HSIC),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_HSIC
endif

ifneq ($(CONFIG_DIAG_SDIO),)
     KBUILD_CPPFLAGS += -DCONFIG_DIAG_SDIO
endif

ifneq ($(CONFIG_MHI_XPRT),)
     KBUILD_CPPFLAGS += -DCONFIG_MHI_XPRT
endif

ifneq ($(CONFIG_HSIC_XPRT),)
     KBUILD_CPPFLAGS += -DCONFIG_HSIC_XPRT
endif

ifneq ($(CONFIG_SDIO_XPRT),)
     KBUILD_CPPFLAGS += -DCONFIG_SDIO_XPRT
endif

ifneq ($(CONFIG_IPC_ROUTER_SECURITY),)
     KBUILD_CPPFLAGS += -DCONFIG_IPC_ROUTER_SECURITY
endif

ifeq ($(CONFIG_USE_CUSTOMIZED_DMA_MEM), y)
    KBUILD_CPPFLAGS += -DCONFIG_USE_CUSTOMIZED_DMA_MEM
endif

ifeq ($(CONFIG_WCNSS_DMA_PRE_ALLOC), y)
    KBUILD_CPPFLAGS += -DCONFIG_WCNSS_DMA_PRE_ALLOC
endif

ifneq ($(CONFIG_MSM_MHI),)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_MHI
endif

ifneq ($(CONFIG_MSM_QMI_INTERFACE),)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_QMI_INTERFACE
endif

ifeq ($(CONFIG_MSM_DIAG_INTERFACE), y)
     KBUILD_CPPFLAGS += -DCONFIG_MSM_DIAG_INTERFACE
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

ifneq ($(CONFIG_QTI_SDIO_CLIENT),)
     KBUILD_CPPFLAGS += -DCONFIG_QTI_SDIO_CLIENT
endif

ifneq ($(CONFIG_NAPIER_X86),)
     KBUILD_CPPFLAGS += -DCONFIG_NAPIER_X86
endif

ifneq ($(CONFIG_CNSS_QCA6390),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS_QCA6390
endif

ifneq ($(CONFIG_CNSS_QCA6490),)
     KBUILD_CPPFLAGS += -DCONFIG_CNSS_QCA6490
endif

ifneq ($(CONFIG_WLAN_CNSS_CORE),)
    KBUILD_CPPFLAGS += -DCONFIG_WLAN_CNSS_CORE
endif

ifneq ($(CONFIG_CNSS_UTILS),)
    KBUILD_CPPFLAGS += -DCONFIG_CNSS_UTILS
endif

ifneq ($(CONFIG_WCNSS_SKB_PRE_ALLOC),)
    KBUILD_CPPFLAGS += -DCONFIG_WCNSS_MEM_PRE_ALLOC
	KBUILD_CPPFLAGS += -DCONFIG_WCNSS_SKB_PRE_ALLOC
else
	ifneq ($(CONFIG_WCNSS_MEM_PRE_ALLOC),)
		KBUILD_CPPFLAGS += -DCONFIG_WCNSS_MEM_PRE_ALLOC
	endif
endif

ifneq ($(CONFIG_ONE_MSI_VECTOR),)
     KBUILD_CPPFLAGS += -DCONFIG_ONE_MSI_VECTOR
endif

ifneq ($(CONFIG_CNSS_DISABLE_EXPORT_SYMBOL),)
	KBUILD_CPPFLAGS += -DCNSS_DISABLE_EXPORT_SYMBOL
endif

ifneq ($(CONFIG_MULTI_CARD),)
	KBUILD_CPPFLAGS += -DMULTI_CARD
	ifneq ($(CONFIG_PCIE_SSID),)
		KBUILD_CPPFLAGS += -DPCIE_SSID=$(CONFIG_PCIE_SSID)
	endif
	ifneq ($(CONFIG_CUSTOM_FW_NL_PROTO),)
		KBUILD_CPPFLAGS += -DCUSTOM_FW_NL_PROTO=$(CONFIG_CUSTOM_FW_NL_PROTO)
	endif
	ifneq ($(CONFIG_IPC_SOCKET_FAMILY),)
		KBUILD_CPPFLAGS += -DIPC_SOCKET_FAMILY=$(CONFIG_IPC_SOCKET_FAMILY)
	endif
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
obj-$(CONFIG_SDIO_XPRT) += sdio_xprt/
obj-$(CONFIG_MSM_QMI_INTERFACE) += qmi/
obj-$(CONFIG_MSM_DIAG_INTERFACE) += diag/
obj-$(CONFIG_CNSS2) += cnss2/
obj-$(CONFIG_DIAG_IPC_BRIDGE) += diag_ipc_bridge/
obj-$(CONFIG_QTI_SDIO_CLIENT) += qti_sdio_client/
obj-$(CONFIG_QCN) += qcn/
obj-$(CONFIG_CNSS_UTILS) += cnss_utils/
obj-$(CONFIG_WCNSS_MEM_PRE_ALLOC) += cnss_prealloc/
else

ifneq ($(CONFIG_SINGLE_KO_FEATURE),)
    CNSS_CORE_BASE=../wlan-cnss-core
else
    CNSS_CORE_BASE=.
endif


KS_BRIDGE_DIR := $(CNSS_CORE_BASE)/ks_bridge
MHI_DIR := $(CNSS_CORE_BASE)/mhi
IPC_ROUTER_DIR := $(CNSS_CORE_BASE)/ipc_router
XPRT_DIR := $(CNSS_CORE_BASE)/xprt
HSIC_XPRT_DIR := $(CNSS_CORE_BASE)/hsic_xprt
SDIO_XPRT_DIR := $(CNSS_CORE_BASE)/sdio_xprt
QMI_DIR := $(CNSS_CORE_BASE)/qmi
DIAG_DIR := $(CNSS_CORE_BASE)/diag
CNSS_DIR := $(CNSS_CORE_BASE)/cnss2
DIAG_IPC_BRIDGE_DIR := $(CNSS_CORE_BASE)/diag_ipc_bridge
QTI_SDIO_CLIENT_DIR := $(CNSS_CORE_BASE)/qti_sdio_client
QCN_DIR := $(CNSS_CORE_BASE)/qcn
CNSS_UTILS_DIR := $(CNSS_CORE_BASE)/cnss_utils
CNSS_PREALLOC_DIR := $(CNSS_CORE_BASE)/cnss_prealloc
MULTI_CARD_DIR := $(CNSS_CORE_BASE)/multi_card

INIT_OBJS := $(CNSS_CORE_BASE)/unified_wlan_cnsscore.o
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

ifneq ($(CONFIG_QCN),)
        QCN_OBJS := $(QCN_DIR)/qcn_sdio.o
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
ifeq ($(CONFIG_CNSS2_SDIO),y)
	CNSS_OBJS +=  $(CNSS_DIR)/sdio.o
endif

	CNSS_INC := -I$(CNSS_DIR)
endif

ifeq ($(CONFIG_MSM_DIAG_INTERFACE), y)
	DIAG_OBJS := $(DIAG_DIR)/diagchar_core.o             \
		     $(DIAG_DIR)/diag_local.o                \
		     $(DIAG_DIR)/diagmem.o                   \
		     $(DIAG_DIR)/diagfwd_bridge.o            \
		     $(DIAG_DIR)/diag_nl.o

ifeq ($(CONFIG_DIAG_HSIC),y)
	DIAG_OBJS += $(DIAG_DIR)/diagfwd_hsic.o
endif
ifeq ($(CONFIG_DIAG_SDIO),y)
        DIAG_OBJS += $(DIAG_DIR)/diagfwd_sdio.o
endif
ifeq ($(CONFIG_DIAG_MHI),y)
	DIAG_OBJS += $(DIAG_DIR)/diagfwd_mhi.o
endif
	DIAG_INC := -I$(DIAG_DIR)
endif

ifneq ($(CONFIG_DIAG_IPC_BRIDGE), )
	DIAG_IPC_BRIDGE_OBJS := $(DIAG_IPC_BRIDGE_DIR)/diag_ipc_bridge.o
endif

ifneq ($(CONFIG_QTI_SDIO_CLIENT), )
        QTI_SDIO_CLIENT_OBJS := $(QTI_SDIO_CLIENT_DIR)/qti_sdio_client.o
endif

ifneq ($(CONFIG_IPC_ROUTER), )
	IPC_ROUTER_OBJS := $(IPC_ROUTER_DIR)/ipc_router_core.o          \
			   $(IPC_ROUTER_DIR)/ipc_router_socket.o        \
			   $(IPC_ROUTER_DIR)/ipc_router_security.o
	IPC_ROUTER_INC := -I$(IPC_ROUTER_DIR)
endif

ifneq ($(CONFIG_HSIC_XPRT), )
	HSIC_XPERT_OBJS := $(HSIC_XPRT_DIR)/ipc_router_hsic_xprt.o
endif

ifneq ($(CONFIG_SDIO_XPRT), )
        SDIO_XPRT_OBJS := $(SDIO_XPRT_DIR)/ipc_router_sdio_xprt.o
endif

ifneq ($(CONFIG_MHI_XPRT), )
	XPRT_OBJS := $(XPRT_DIR)/ipc_router_mhi_xprt.o
endif

ifneq ($(CONFIG_MSM_QMI_INTERFACE), )
	QMI_OBJS := $(QMI_DIR)/qmi_encdec.o        \
		    $(QMI_DIR)/qmi_interface.o
	QMI_INC := -I$(QMI_DIR)
endif

ifneq ($(CONFIG_CNSS_UTILS), )
	CNSS_UTILS_OBJS := $(CNSS_UTILS_DIR)/cnss_utils.o
	CNSS_UTILS_INC := -I$(CNSS_UTILS_DIR)
endif

CNSS_PREALLOC_OBJS := $(CNSS_PREALLOC_DIR)/cnss_prealloc.o
CNSS_PREALLOC_INC := $(CNSS_PREALLOC_DIR)

ifneq ($(CONFIG_MULTI_CARD),)
	MULTI_CARD_OBJS := $(MULTI_CARD_DIR)/multi_card.o
	MULTI_CARD_INC := $(MULTI_CARD_DIR)
endif

ifneq ($(CONFIG_SINGLE_KO_FEATURE),)
OBJS += $(INIT_OBJS)
else
OBJS := $(INIT_OBJS)
endif

OBJS += $(IPC_ROUTER_OBJS)                 \
	$(QMI_OBJS)                        \
	$(KS_BRIDGE_OBJS)                  \
	$(DIAG_IPC_BRIDGE_OBJS)            \
	$(XPRT_OBJS)                       \
	$(HSIC_XPERT_OBJS)                 \
	$(SDIO_XPRT_OBJS)                  \
	$(MHI_OBJS)                        \
	$(QCN_OBJS)                        \
	$(QTI_SDIO_CLIENT_OBJS)            \
	$(DIAG_OBJS)                       \
	$(CNSS_OBJS)                       \
	$(CNSS_UTILS_OBJS)                 \
	$(CNSS_PREALLOC_OBJS)              \
	$(MULTI_CARD_OBJS)

ifneq ($(CONFIG_SINGLE_KO_FEATURE),)
INCS += $(INIT_INC)
else
INCS := $(INIT_INC)
endif

INCS += $(CNSS_INC)                     \
        $(KS_BRIDGE_INC)                \
        $(MHI_INC)                      \
        $(DIAG_INC)                     \
        $(QMI_INC)                      \
        $(CNSS_UTILS_INC)               \
        $(CNSS_PREALLOC_INC)            \
        $(MULTI_CARD_INC)


cflags-y += $(INCS)
ccflags-y += -Os -I$(src)/$(CNSS_CORE_BASE)/inc -I$(src)/$(CNSS_CORE_BASE)/mhi -I$(ROOTDIR)

obj-$(WLAN_CNSSCORE) +=$(MODNAME).o
$(MODNAME)-y := $(OBJS)

endif
