LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := linearMath
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libLinearMath.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/bullet
include $(PREBUILT_STATIC_LIBRARY)
