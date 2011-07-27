LOCAL_PATH:= $(call my-dir)

PPPOE_VERSION="\"3.0\""

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/pppoe.c \
                   src/if.c \
                   src/debug.c \
                   src/common.c \
                   src/ppp.c \
                   src/discovery.c
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE = pppoe
LOCAL_CFLAGS := -DVERSION=$(PPPOE_VERSION)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/pppoe-sniff.c \
                   src/if.c \
                   src/debug.c \
                   src/common.c 
LOCAL_C_INCLUDES := $(KERNEL_HEADERS) 
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE = pppoe-sniff
LOCAL_CFLAGS := -DVERSION=$(PPPOE_VERSION) 
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/relay.c \
                   src/if.c \
                   src/debug.c \
                   src/common.c 
LOCAL_C_INCLUDES := $(KERNEL_HEADERS) 
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE = pppoe-relay
LOCAL_CFLAGS := -DVERSION=$(PPPOE_VERSION) 
include $(BUILD_EXECUTABLE)

