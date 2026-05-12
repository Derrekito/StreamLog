ifeq ($(aarch64),1)
	GCC_VERSION := 9
	TARGET_ARCH := aarch64

	AR := $(TARGET_ARCH)-linux-gnu-ar
	CXX := $(TARGET_ARCH)-linux-gnu-g++-$(GCC_VERSION)
endif