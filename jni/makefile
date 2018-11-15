all:
	adb push ../libs/armeabi/libhello.so /data/local/tmp
	adb push ../libs/armeabi/myinject /data/local/tmp
	adb shell chmod 777 /data/local/tmp
	adb shell chmod 777 /data/local/tmp/libhello.so
	adb shell chmod 777 /data/local/tmp/myinject
	adb shell /data/local/tmp/myinject