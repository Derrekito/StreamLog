AWK		:= awk
DF		:= df
MKDIR	:= mkdir -p
RM 		:= rm -f
AR 		?= ar
SHELL		?= /bin/bash
hostname	:= $(shell uname -n)

# Host architecture and size
HOST_ARCH	:= $(shell uname -m)
HOST_SIZE	:= $(shell uname -m | sed -e "s/x86_64/64/" -e "s/armv7l/32/" -e "s/aarch64/64/")

# Target architecture and size
TARGET_SIZE	?= $(HOST_SIZE)
TARGET_ARCH	?= $(HOST_ARCH)

define echo_build
	@echo "building " $(1)
endef