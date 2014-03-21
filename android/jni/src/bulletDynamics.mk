LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := bulletDynamics
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libBulletDynamics.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/bullet
include $(PREBUILT_STATIC_LIBRARY)
