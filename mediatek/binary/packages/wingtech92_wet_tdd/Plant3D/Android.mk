#
$(info MAGE Money Plant android.mk)
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := Plant3D
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS

#LOCAL_SDK_VERSION := current


LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.ngin3d-static

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += CustomProperties

LOCAL_PROGUARD_ENABLED := custom
LOCAL_PROGUARD_SOURCE := javaclassfile
LOCAL_PROGUARD_FLAG_FILES := proguard.flags


LOCAL_JNI_SHARED_LIBRARIES := libja3m liba3m

LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res

#add for JPE begin
LOCAL_JAVASSIST_ENABLED := true
LOCAL_JAVASSIST_OPTIONS := $(LOCAL_PATH)/jpe.config
#end for JPE

ifeq ($(strip $(MTK_SIGNATURE_CUSTOMIZATION)),yes)
  LOCAL_CERTIFICATE := releasekey
else
  LOCAL_CERTIFICATE := testkey
endif
include $(BUILD_PREBUILT)
