LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vice

CG_SUBDIRS := .

# Add more subdirs here, like src/subdir1 src/subdir2

MY_PATH := $(LOCAL_PATH)

CG_SRCDIR := $(LOCAL_PATH)

LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/include \
				$(LOCAL_PATH)/../sdl/include \
				$(LOCAL_PATH)/../vice/src/ \
				$(LOCAL_PATH)/../vice/src/ \
				$(LOCAL_PATH)/../vice/src/arch/sdl \
				$(LOCAL_PATH)

LOCAL_PATH := $(MY_PATH)

LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS)

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))

LOCAL_STATIC_LIBRARIES := locnet_al vice_main vice_x64 vice_common vice_driver vice_temp
#LOCAL_LDLIBS := -ljnigraphics
LOCAL_LDLIBS := -lz -llog


MY_PATH2 := C:/Projects/Eclipse/C64/AnVICE/jni/application/vice/vice

LOCAL_LDFLAGS := \
	-LC:/Projects/Eclipse/C64/AnVICE/obj/local/armeabi	\
	-L$(MY_PATH2)/src/arch/sdl	\
	-L$(MY_PATH2)/src/fsdevice \
	-L$(MY_PATH2)/src/tape \
	-L$(MY_PATH2)/src/imagecontents \
	-L$(MY_PATH2)/src/fileio \
	-L$(MY_PATH2)/src/serial \
	-L$(MY_PATH2)/src/core \
	-L$(MY_PATH2)/src/sounddrv \
	-L$(MY_PATH2)/src/gfxoutputdrv \
	-L$(MY_PATH2)/src/printerdrv \
	-L$(MY_PATH2)/src/rs232drv \
	-L$(MY_PATH2)/src/diskimage \
	-L$(MY_PATH2)/src/vicii \
	-L$(MY_PATH2)/src/raster \
	-L$(MY_PATH2)/src/rtc \
	-L$(MY_PATH2)/src/video \
	-L$(MY_PATH2)/src/c64 \
	-L$(MY_PATH2)/src/c64/cart \
	-L$(MY_PATH2)/src/drive/iec \
	-L$(MY_PATH2)/src/drive/iecieee \
	-L$(MY_PATH2)/src/drive/iec/c64exp \
	-L$(MY_PATH2)/src/drive/ieee \
	-L$(MY_PATH2)/src/drive \
	-L$(MY_PATH2)/src/iecbus \
	-L$(MY_PATH2)/src/parallel \
	-L$(MY_PATH2)/src/vdrive \
	-L$(MY_PATH2)/src/sid \
	-L$(MY_PATH2)/src/monitor \
	-Wl,--start-group -lvice_main -lvice_x64 -lvice_common -lvice_driver \
	-lvice_temp \
	-Wl,--end-group 

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

