objs := mhi_main.o
objs += mhi_iface.o
objs += mhi_init.o
objs += mhi_isr.o
objs += mhi_mmio_ops.o
objs += mhi_ring_ops.o
objs += mhi_states.o
objs += mhi_sys.o
objs += mhi_bhi.o
objs += mhi_pm.o
objs += mhi_ssr.o
objs += mhi_event.o
ifneq ($(CONFIG_NAPIER_X86),)
objs += mhi_fw_dump.o
endif
obj-$(CONFIG_MSM_MHI) += mhi.o
mhi-y := $(objs)
ccflags-y += -I$(src)/../inc -I$(src)
