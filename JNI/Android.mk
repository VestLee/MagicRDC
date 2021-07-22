LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	InlineHook/And64InlineHook.cpp \
	InlineHook/HookApi.cpp \
	InlineHook/inlineHook.cpp \
	InlineHook/relocate.cpp \
    Main.cpp

LOCAL_MODULE := OpenNativeLibrary
APP_STL := c++_shared
LOCAL_CFLAGS += -std=c++11
LOCAL_LDLIBS := -lm -ldl -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    libinject/inject.cpp \
    inject_main.cpp

LOCAL_MODULE := inject
#LOCAL_CFLAGS += -DDEBUG
LOCAL_LDLIBS := -ldl -llog

include $(BUILD_EXECUTABLE)