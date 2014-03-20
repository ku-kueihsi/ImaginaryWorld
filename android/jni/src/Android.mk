LOCAL_PATH := $(call my-dir)

#bullet
include $(CLEAR_VARS)
LOCAL_MODULE := bullet
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libBulletSoftBody.a
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libBulletDynamics.a
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libBulletCollision.a
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libLinearMath.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/bullet
include $(PREBUILT_STATIC_LIBRARY)

#assimp
include $(CLEAR_VARS)
LOCAL_MODULE := assimp
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libassimp.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/assimp
include $(PREBUILT_STATIC_LIBRARY)

#libpng
include $(CLEAR_VARS)
LOCAL_MODULE := libpng
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libpng.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/
include $(PREBUILT_STATIC_LIBRARY)

#SDL
include $(CLEAR_VARS)
LOCAL_MODULE := SDL
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libSDL2.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/SDL2/
include $(PREBUILT_STATIC_LIBRARY)

#freetype2
include $(CLEAR_VARS)
LOCAL_MODULE := freetype2
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libfreetype.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/freetype2/
include $(PREBUILT_STATIC_LIBRARY)

#bullet
include $(CLEAR_VARS)
LOCAL_MODULE := bullet
LOCAL_SRC_FILES := $(ANDROID_TOOLCHAIN)/lib/libBulletCollision.a $(ANDROID_TOOLCHAIN)/lib/libBulletSoftBody.a $(ANDROID_TOOLCHAIN)/lib/libConvexDecomposition.a $(ANDROID_TOOLCHAIN)/lib/libLinearMath.a $(ANDROID_TOOLCHAIN)/lib/libBulletDynamics.a $(ANDROID_TOOLCHAIN)/lib/libBulletWorldImporter.a $(ANDROID_TOOLCHAIN)/lib/libGIMPACTUtils.a $(ANDROID_TOOLCHAIN)/lib/libBulletFileLoader.a $(ANDROID_TOOLCHAIN)/lib/libBulletXmlWorldImporter.a $(ANDROID_TOOLCHAIN)/lib/libHACD.a
LOCAL_EXPORT_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/bullet/
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

COMMON := ../../../common/

LOCAL_C_INCLUDES := $(ANDROID_TOOLCHAIN)/include/
LOCAL_C_INCLUDES += $(ANDROID_TOOLCHAIN)/include/eigen3/
LOCAL_C_INCLUDES += $(ANDROID_SYSROOT)/include/
LOCAL_C_INCLUDES += $(COMMON)/

LOCAL_MODULE := main

LOCAL_SRC_FILES := 	SDL_android_main.c \
					$(COMMON)/main.cpp \
					$(COMMON)/gl_tools.cpp \
					$(COMMON)/data_utils.cpp \
					$(COMMON)/image.cpp \
					$(COMMON)/mesh.cpp 
					 

LOCAL_STATIC_LIBRARIES := SDL assimp libpng freetype2 bullet
LOCAL_LDLIBS 	:= -lz -lstdc++ -llog -lGLESv1_CM -lGLESv2 -lEGL -landroid

include $(BUILD_SHARED_LIBRARY)
#$(call import-module,SDL)LOCAL_PATH := $(call my-dir)
