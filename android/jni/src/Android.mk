LOCAL_PATH := $(call my-dir)

#========prebuilt libs================
include $(LOCAL_PATH)/assimp.mk
include $(LOCAL_PATH)/libpng.mk
include $(LOCAL_PATH)/freetype.mk
include $(LOCAL_PATH)/sdl.mk
include $(LOCAL_PATH)/bulletSoftBody.mk
include $(LOCAL_PATH)/bulletDynamics.mk
include $(LOCAL_PATH)/bulletCollision.mk
include $(LOCAL_PATH)/linearMath.mk
#========end of prebuilt libs==========

include $(CLEAR_VARS)

COMMON := ../../../common/

## extra includes
LOCAL_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/
LOCAL_C_INCLUDES += $(ANDROID_TOOLCHAIN)/include/eigen3/
LOCAL_C_INCLUDES += $(ANDROID_SYSROOT)/include/
LOCAL_C_INCLUDES += $(COMMON)/

## jni module name
LOCAL_MODULE := main

LOCAL_SRC_FILES := 	SDL_android_main.c \
					$(COMMON)/main.cpp \
					$(COMMON)/gl_tools.cpp \
					$(COMMON)/data_utils.cpp \
					$(COMMON)/image.cpp \
					$(COMMON)/mesh.cpp 
					 

LOCAL_STATIC_LIBRARIES := sdl assimp libpng freetype bulletSoftBody bulletDynamics bulletCollision linearMath
LOCAL_LDLIBS 	:= -lz -lstdc++ -llog -lGLESv1_CM -lGLESv2 -lEGL -landroid
#LOCAL_LDLIBS 	:= -lz -lstdc++ -llog -lGLESv2 -lEGL -landroid

include $(BUILD_SHARED_LIBRARY)
#$(call import-module,SDL)LOCAL_PATH := $(call my-dir)
