objs += diagchar_core.o
objs += diag_local.o
objs += diagmem.o
objs += diagfwd_bridge.o
objs += diag_nl.o

obj-$(CONFIG_MSM_DIAG_INTERFACE) += diag.o

diag-y := $(objs)
diag-$(CONFIG_DIAG_MHI) += diagfwd_mhi.o
diag-$(CONFIG_DIAG_HSIC) += diagfwd_hsic.o
diag-$(CONFIG_DIAG_SDIO) += diagfwd_sdio.o

ccflags-y += -I$(src)/../inc
