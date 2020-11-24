
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := x264
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/armeabi-v7a/libx264.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

include $(PREBUILT_STATIC_LIBRARY)

