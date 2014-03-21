LOCAL_PATH := $(call my-dir)

#freetype
include $(CLEAR_VARS)
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libfreetype.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/freetype2/
include $(PREBUILT_STATIC_LIBRARY)
