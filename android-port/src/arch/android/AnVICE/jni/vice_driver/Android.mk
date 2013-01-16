LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vice_driver

MY_PATH2 := ../vice/

CG_SUBDIRS := \
$(MY_PATH2)/src/fsdevice \
$(MY_PATH2)/src/tape \
$(MY_PATH2)/src/imagecontents \
$(MY_PATH2)/src/fileio \
$(MY_PATH2)/src/serial \
$(MY_PATH2)/src/core \
$(MY_PATH2)/src/sounddrv/ \
$(MY_PATH2)/src/gfxoutputdrv \
$(MY_PATH2)/src/printerdrv \
$(MY_PATH2)/src/rs232drv/ \
$(MY_PATH2)/src/diskimage \

# Add more subdirs here, like src/subdir1 src/subdir2

MY_PATH := $(LOCAL_PATH)

CG_SRCDIR := $(LOCAL_PATH)

LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/include \
				$(LOCAL_PATH)/../sdl/include \
				$(LOCAL_PATH)/../locnet/include \
				$(LOCAL_PATH) \
				$(LOCAL_PATH)/../vice/src \
				$(LOCAL_PATH)/../vice/src/vdrive \
				$(LOCAL_PATH)/../vice/src/arch/sdl \
				$(LOCAL_PATH)/../vice/src/arch \
				$(LOCAL_PATH)/../vice/src
				
LOCAL_PATH := $(MY_PATH)

LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS)

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

#LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
# Uncomment to also add C sources
LOCAL_SRC_FILES += $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))
	
# LOCAL_STATIC_LIBRARIES := sdl_mixer sdl tremor stlport
#LOCAL_STATIC_LIBRARIES := locnet_al

#LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lsupc++ -ljnigraphics

#LOCAL_LDLIBS := -ldl -lsupc++ -ljnigraphics

LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

