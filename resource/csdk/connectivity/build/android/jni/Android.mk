#define build type
BUILD = debug

PROJECT_ROOT_PATH			?= ../..
PROJECT_API_PATH			= $(PROJECT_ROOT_PATH)/api
PROJECT_INC_PATH			= $(PROJECT_ROOT_PATH)/inc
PROJECT_SRC_PATH			= $(PROJECT_ROOT_PATH)/src
PROJECT_COMMON_PATH			= $(PROJECT_ROOT_PATH)/common
PROJECT_COMMON_INC_PATH			= $(PROJECT_COMMON_PATH)/inc
PROJECT_COMMON_SRC_PATH			= $(PROJECT_COMMON_PATH)/src
PROJECT_LIB_PATH			= $(PROJECT_ROOT_PATH)/lib
PROJECT_EXTERNAL_PATH			= $(PROJECT_ROOT_PATH)/external/inc
DTLS_LIB 				= $(PROJECT_LIB_PATH)/tinydtls
GLIB_PATH				= $(PROJECT_LIB_PATH)/android/glib-2.40.2

#Modify below values to enable/disable the Adapter
#Suffix "NO_" to disable given adapter
EDR		= EDR_ADAPTER
WIFI		= WIFI_ADAPTER
LE		= LE_ADAPTER
ETHERNET	= NO_ETHERNET_ADAPTER

#Add Pre processor definitions
DEFINE_FLAG =  -DWITH_POSIX -D__ANDROID__ -D__WITH_DTLS__
DEFINE_FLAG += -D$(EDR) -D$(LE) -D$(WIFI) -D$(ETHERNET)

#Add Debug flags here
DEBUG_FLAG 	= -DTB_LOG
#DEBUG_FLAG 	+= -DADB_SHELL

BUILD_FLAG.debug	= $(DEFINE_FLAG) $(DEBUG_FLAG)
BUILD_FLAG.release	=	$(DEFINE_FLAG)
BUILD_FLAG = $(BUILD_FLAG.$(BUILD))

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include glib

include $(CLEAR_VARS)
LOCAL_PATH = $(PROJECT_LIB_PATH)/android
LOCAL_MODULE = Glib
LOCAL_SRC_FILES := libglib-2.40.2.so
LOCAL_EXPORT_C_INCLUDES = $(GLIB_PATH) \
                          $(GLIB_PATH)/glib

include $(PREBUILT_SHARED_LIBRARY)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include glibthread

include $(CLEAR_VARS)
LOCAL_PATH = $(PROJECT_LIB_PATH)/android
LOCAL_MODULE = GLibThread
LOCAL_SRC_FILES := libgthread-2.40.2.so
LOCAL_EXPORT_C_INCLUDES = $(GLIB_PATH) \
                          $(GLIB_PATH)/glib

include $(PREBUILT_SHARED_LIBRARY)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#Build TinyDtls

include $(CLEAR_VARS)
include $(DTLS_LIB)/Android.mk

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#Build CACommon

include $(CLEAR_VARS)

#Build Common Libraries
LOCAL_PATH = $(PROJECT_COMMON_SRC_PATH)
LOCAL_MODULE = CACommon
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

LOCAL_SHARED_LIBRARIES = Glib GLibThread

LOCAL_CFLAGS = -D__ANDROID__ $(DEBUG_FLAG)

LOCAL_C_INCLUDES = $(PROJECT_COMMON_INC_PATH)
LOCAL_C_INCLUDES += $(PROJECT_API_PATH)

LOCAL_SRC_FILES	=	oic_logger.c \
					oic_console_logger.c logger.c oic_malloc.c \
					uarraylist.c uqueue.c oic_string.c \
					uthreadpool.c umutex.c

include $(BUILD_STATIC_LIBRARY)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#Build CACoap

include $(CLEAR_VARS)

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_PATH = $(PROJECT_LIB_PATH)/libcoap-4.1.1
LOCAL_MODULE = CACoap
LOCAL_EXPORT_C_INCLUDES = $(PROJECT_LIB_PATH)/libcoap-4.1.1
LOCAL_CFLAGS = -DWITH_POSIX
LOCAL_SRC_FILES	= pdu.c net.c debug.c encode.c uri.c coap_list.c resource.c hashkey.c \
					str.c option.c async.c subscribe.c block.c

include $(BUILD_STATIC_LIBRARY)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#Build CA

#Relative path to LOCAL_PATH (PROJECT_SRC_PATH)
LOCAL_PLATFORM 				= android

ENET_ADAPTER_PATH			= ethernet_adapter/$(LOCAL_PLATFORM)
EDR_ADAPTER_PATH			= bt_edr_adapter/$(LOCAL_PLATFORM)
LE_ADAPTER_PATH				= bt_le_adapter/$(LOCAL_PLATFORM)
WIFI_ADAPTER_PATH			= wifi_adapter/$(LOCAL_PLATFORM)
ADAPTER_UTILS				= adapter_util

include $(CLEAR_VARS)

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_PATH = $(PROJECT_SRC_PATH)
LOCAL_MODULE = CA

LOCAL_STATIC_LIBRARIES = CACommon CACoap TinyDtls

LOCAL_C_INCLUDES = $(PROJECT_API_PATH)
LOCAL_C_INCLUDES += $(PROJECT_COMMON_INC_PATH)
LOCAL_C_INCLUDES += $(PROJECT_INC_PATH)
LOCAL_C_INCLUDES += $(PROJECT_LIB_PATH)/libcoap-4.1.1
LOCAL_C_INCLUDES += $(PROJECT_EXTERNAL_PATH)
LOCAL_C_INCLUDES += $(DTLS_LIB)

LOCAL_CFLAGS += $(BUILD_FLAG)

LOCAL_SRC_FILES	= \
					caconnectivitymanager.c caremotehandler.c cainterfacecontroller.c \
					camessagehandler.c canetworkconfigurator.c caprotocolmessage.c \
					caretransmission.c caqueueingthread.c \
					$(ADAPTER_UTILS)/caadapternetdtls.c $(ADAPTER_UTILS)/caadapterutils.c \
					$(ADAPTER_UTILS)/camsgparser.c \
					$(LE_ADAPTER_PATH)/caleadapter.c $(LE_ADAPTER_PATH)/caleclient.c \
					$(LE_ADAPTER_PATH)/caleserver.c $(LE_ADAPTER_PATH)/caleutils.c \
					$(LE_ADAPTER_PATH)/calenwmonitor.c \
					bt_edr_adapter/caedradapter.c $(EDR_ADAPTER_PATH)/caedrutils.c \
					$(EDR_ADAPTER_PATH)/caedrclient.c $(EDR_ADAPTER_PATH)/caedrserver.c \
					$(EDR_ADAPTER_PATH)/caedrnwmonitor.c \
					wifi_adapter/cawifiadapter.c $(WIFI_ADAPTER_PATH)/cawifiserver.c \
					$(WIFI_ADAPTER_PATH)/cawificlient.c $(WIFI_ADAPTER_PATH)/cawifinwmonitor.c \

include $(BUILD_STATIC_LIBRARY)

