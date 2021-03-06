#include <jni.h>
/* Header for class com_iotivity_service_RMInterface */

#ifndef _Included_com_iotivity_service_RMInterface
#define _Included_com_iotivity_service_RMInterface
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_setNativeResponseListener
  (JNIEnv *, jobject, jobject);
/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMInitialize
 * Signature: (Landroid/content/Context;)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMInitialize
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMTerminate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMTerminate
  (JNIEnv *, jobject);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMStartListeningServer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMStartListeningServer
  (JNIEnv *, jobject);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMStartDiscoveryServer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMStartDiscoveryServer
  (JNIEnv *, jobject);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMRegisterHandler
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMRegisterHandler
  (JNIEnv *, jobject);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMFindResource
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMFindResource
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMSendRequest
 * Signature: (Ljava/lang/String;III)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMSendRequest
  (JNIEnv *, jobject, jstring, jstring, jint, jint, jint);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMSendResponse
 * Signature: (Ljava/lang/String;III)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMSendResponse
  (JNIEnv *, jobject, jstring, jstring, jint, jint);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMAdvertiseResource
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMAdvertiseResource
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMSendNotification
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMSendNotification
  (JNIEnv *, jobject, jstring, jstring, jint, jint);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMSelectNetwork
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMSelectNetwork
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_iotivity_service_RMInterface
 * Method:    RMHandleRequestResponse
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_iotivity_service_RMInterface_RMHandleRequestResponse
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif

