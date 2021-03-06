package com.iotivity.service;

import android.content.Context;

public class RMInterface {

    static {
        // Load RI JNI interface
        System.loadLibrary("RMInterface");

        // Load CA JNI interface
        System.loadLibrary("CAInterface");
    }

    private com.iotivity.service.MainActivity mResponseListener = null;

    public native void setNativeResponseListener(Object listener);

    public native void RMInitialize(Context context);

    public native void RMTerminate();

    public native void RMStartListeningServer();

    public native void RMStartDiscoveryServer();

    public native void RMRegisterHandler();

    public native void RMFindResource(String uri);

    public native void RMSendRequest(String uri, String payload,
            int selectedNetwork, int isSecured, int msgType);

    public native void RMSendResponse(String uri, String payload,
            int selectedNetwork, int isSecured);

    public native void RMAdvertiseResource(String advertiseResource,
            int selectedNetwork);

    public native void RMSendNotification(String uri, String payload,
            int selectedNetwork, int isSecured);

    public native void RMSelectNetwork(int interestedNetwork);

    public native void RMHandleRequestResponse();

    public void setResponseListener(com.iotivity.service.MainActivity listener) {
        mResponseListener = listener;
        setNativeResponseListener(this);
    }

    private void OnResponseReceived(String subject, String receivedData) {
        if (null != mResponseListener) {
            mResponseListener.OnResponseReceived(subject, receivedData);
        }
    }

}
