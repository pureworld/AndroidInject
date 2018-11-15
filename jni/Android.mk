LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := myinject
LOCAL_SRC_FILES := myinject.cpp

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE    := hello
LOCAL_SRC_FILES := hello.cpp
LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)