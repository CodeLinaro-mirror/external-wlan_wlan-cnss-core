objs += ipc_router_core.o
objs += ipc_router_socket.o
objs += ipc_router_security.o

obj-$(CONFIG_IPC_ROUTER) += ipc_router.o
ipc_router-y := $(objs)

ccflags-y += -I$(src)/../inc
