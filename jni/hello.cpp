#include <android/log.h>

#define TAG "inject"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

__attribute__((constructor)) void OnLoad() {
	LOGD("OnLoad\n");
}

__attribute__((destructor)) void OnUnload() {
	LOGD("OnUnload\n");
}
