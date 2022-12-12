ifeq ($(CONFIG_CNSS_OUT_OF_TREE),y)
KBUILD_CPPFLAGS += -DCONFIG_CNSS_OUT_OF_TREE
endif

ifeq ($(CONFIG_CNSS2_DEBUG),y)
KBUILD_CPPFLAGS += -DCONFIG_CNSS2_DEBUG
endif

ifeq ($(CONFIG_CNSS2_QMI),y)
KBUILD_CPPFLAGS += -DCONFIG_CNSS2_QMI
endif

ifeq ($(CONFIG_CNSS2_X86),y)
KBUILD_CPPFLAGS += -DCONFIG_CNSS2_X86
endif

ifeq ($(CONFIG_ONE_MSI_VECTOR),y)
KBUILD_CPPFLAGS += -DCONFIG_ONE_MSI_VECTOR
endif

ifeq ($(CONFIG_DUMP_FW_TO_FILE),y)
KBUILD_CPPFLAGS += -DCONFIG_DUMP_FW_TO_FILE
endif

# CONFIG_CNSS_PLAT_IPC_QMI_SVC should never be "y" here since it
# can be only compiled as a module from out-of-kernel-tree source.
ifeq ($(CONFIG_CNSS_PLAT_IPC_QMI_SVC),m)
KBUILD_CPPFLAGS += -DCONFIG_CNSS_PLAT_IPC_QMI_SVC
endif

ifneq ($(CONFIG_PCIE_EMULATION),)
    KBUILD_CPPFLAGS += -DCONFIG_PCIE_EMULATION
endif

ifneq ($(CONFIG_MHI_BUS),)
	KBUILD_CPPFLAGS += -DCONFIG_MHI_BUS
endif

ifneq ($(CONFIG_MHI_BUS_PCI_GENERIC),)
	KBUILD_CPPFLAGS += -DCONFIG_MHI_BUS_PCI_GENERIC
endif

ifneq ($(CONFIG_QRTR),)
	KBUILD_CPPFLAGS += -DCONFIG_QRTR
endif

ifneq ($(CONFIG_QRTR_MHI),)
	KBUILD_CPPFLAGS += -DCONFIG_QRTR_MHI
endif

ifneq ($(CONFIG_MHI_BUS_DEBUG),)
	KBUILD_CPPFLAGS += -DCONFIG_MHI_BUS_DEBUG
endif

ifneq ($(CONFIG_WLAN_CNSS_CORE),)
    KBUILD_CPPFLAGS += -DCONFIG_WLAN_CNSS_CORE
endif

ifneq ($(CONFIG_WLAN_CNSS_CORE), y)
ifeq ($(CONFIG_FULL_CORE_TECH),y)
obj-$(CONFIG_QRTR) += qrtr/
obj-$(CONFIG_QRTR_MHI) += qrtr/
obj-$(CONFIG_MHI_BUS) += mhi/
obj-$(CONFIG_QCOM_QMI_HELPERS) += qti/
endif
obj-$(CONFIG_CNSS2) += cnss2/
obj-$(CONFIG_CNSS_GENL) += cnss_genl/
obj-$(CONFIG_WCNSS_MEM_PRE_ALLOC) += cnss_prealloc/
obj-y += cnss_utils/

else
CNSS_CORE_BASE=.
QRTR_DIR := $(CNSS_CORE_BASE)/qrtr
MHI_DIR := $(CNSS_CORE_BASE)/mhi/core
QTI_DIR := $(CNSS_CORE_BASE)/qti
CNSS_DIR := $(CNSS_CORE_BASE)/cnss2
CNSS_UTILS_DIR := $(CNSS_CORE_BASE)/cnss_utils
CNSS_PREALLOC_DIR := $(CNSS_CORE_BASE)/cnss_prealloc

INIT_OBJS := $(CNSS_CORE_BASE)/unified_wlan_cnsscore.o
INIT_INC := -I$(ROOTDIR)

ifneq ($(CONFIG_QRTR),)
	QRTR_OBJS := $(QRTR_DIR)/qrtr.o                                         \
				 $(QRTR_DIR)/ns.o
ifeq ($(CONFIG_QRTR_SMD),y)
	QRTR_OBJS +=  $(QRTR_DIR)/smd.o
endif
ifeq ($(CONFIG_QRTR_TUN),y)
	QRTR_OBJS +=  $(QRTR_DIR)/tun.o
endif
ifeq ($(CONFIG_QRTR_MHI),y)
	QRTR_OBJS +=  $(QRTR_DIR)/mhi.o
endif
	QRTR_INC := -I$(ROOTDIR)/$(QRTR_DIR)
endif

ifneq ($(CONFIG_MHI_BUS),)
	MHI_OBJS := $(MHI_DIR)/init.o                                            \
				$(MHI_DIR)/main.o                                       \
				$(MHI_DIR)/pm.o                                         \
				$(MHI_DIR)/boot.o
ifeq ($(CONFIG_MHI_BUS_DEBUG),y)
	MHI_OBJS += $(MHI_DIR)/debugfs.o
endif
	MHI_INC := -I$(ROOTDIR)/$(MHI_DIR)
endif

ifneq ($(CONFIG_QCOM_QMI_HELPERS), )
	QMI_HELPERS_OBJS := $(QTI_DIR)/qmi_encdec.o                            \
	                    $(QTI_DIR)/qmi_interface.o
	QMI_HELPERS_INC := -I$(QTI_DIR)
endif

ifneq ($(CONFIG_CNSS2),)
	CNSS_OBJS := $(CNSS_DIR)/main.o                                         \
		         $(CNSS_DIR)/bus.o                                      \
		         $(CNSS_DIR)/debug.o                                    \
		         $(CNSS_DIR)/power.o                                    \
			     $(CNSS_DIR)/genl.o                                     \
			     $(CNSS_DIR)/coredump.o
ifeq ($(CONFIG_CNSS2_PCIE),y)
	CNSS_OBJS += $(CNSS_DIR)/pci.o
endif
ifeq ($(CONFIG_CNSS2_QMI),y)
	CNSS_OBJS += $(CNSS_DIR)/qmi.o                                          \
		         $(CNSS_DIR)/coexistence_service_v01.o                   \
			     $(CNSS_DIR)/ip_multimedia_subsystem_private_service_v01.o
endif
	CNSS_INC := -I$(CNSS_DIR)
endif

ifneq ($(CONFIG_CNSS_UTILS), )
	CNSS_UTILS_OBJS := $(CNSS_UTILS_DIR)/cnss_utils.o
ifeq ($(CONFIG_CNSS_QMI_SVC),y)
	CNSS_UTILS_OBJS += $(CNSS_UTILS_DIR)/wlan_firmware_service_v01.o     \
                       $(CNSS_UTILS_DIR)/device_management_service_v01.o
endif
ifeq ($(CONFIG_CNSS_PLAT_IPC_QMI_SVC),y)
	CNSS_UTILS_OBJS += $(CNSS_UTILS_DIR)/cnss_plat_ipc_qmi.o             \
                       $(CNSS_UTILS_DIR)/cnss_plat_ipc_service_v01.o
endif
	CNSS_UTILS_INC := -I$(CNSS_UTILS_DIR)
endif

CNSS_PREALLOC_OBJS := $(CNSS_PREALLOC_DIR)/cnss_prealloc.o
CNSS_PREALLOC_INC := $(CNSS_PREALLOC_DIR)

OBJS := $(INIT_OBJS)
OBJS += $(QRTR_OBJS)                                                        \
	    $(MHI_OBJS)                                                     \
	    $(QMI_HELPERS_OBJS)                                             \
	    $(CNSS_OBJS)                                                    \
	    $(CNSS_UTILS_OBJS)                                              \
	    $(CNSS_PREALLOC_OBJS)

INCS := $(INIT_INC)

INCS += $(QRTR_INC)                                                         \
        $(MHI_INC)                                                      \
        $(QMI_HELPERS_INC)                                              \
        $(CNSS_INC)                                                     \
        $(CNSS_UTILS_INC)                                               \
        $(CNSS_PREALLOC_INC)

cflags-y += $(INCS)
ccflags-y += -Os -I$(src)/$(CNSS_CORE_BASE)/inc -I$(src)/$(CNSS_CORE_BASE)/cnss_utils -I$(ROOTDIR)

obj-$(WLAN_CNSSCORE) +=$(MODNAME).o
$(MODNAME)-y := $(OBJS)

endif
