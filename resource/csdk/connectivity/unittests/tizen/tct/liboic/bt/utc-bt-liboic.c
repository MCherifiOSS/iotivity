//#include <glib.h>
#include "assert.h"
#include "interfaceHeaders/cacommon.h"
#include "interfaceHeaders/caadapterinterface.h"
#include "interfaceHeaders/caedradapter.h"

//& set: Liboic

//Bt callbacks

CALocalConnectivity_t* localBtEndpoint = NULL;
CARemoteEndpoint_t remoteMulticastEndpoint;
CARemoteEndpoint_t remoteEndpoint[10]; /* 10 RemoteEndpoints are currently kept */
char remoteIPAddress[CA_IPADDR_SIZE] = "192.168.1.8";  //Change the Corresponding IP address during testing.
char localIPAddress[CA_IPADDR_SIZE] = {0};
int localPort = -1;
int32_t serverId;
// Global bt Addr. Change this before testing.
char btAddr[CA_MACADDR_SIZE] = "BC:79:AD:E6:BC:F6";
char* serviceUUID = "12341234-1C25-481F-9DFB-59193D238280";
//static GMainLoop *mainloop;
//int gMainloopStatus = 0;

//Hardcoded values to Test

char coapData[500] = "{\"oc:\[{href\":\"/a/light\",\"ref\":{\"power\":\"20\",\"state\":\"true\"}}\]}";

typedef struct ConnectivityHandlerList {
    CAConnectivityType_t type;
    CAConnectivityHandler_t handler;
    struct ConnectivityHandlerList* nextHandler;
} ConnectivityHandlerList;

CAConnectivityHandler_t* gConnectivityHandlers = NULL;

void storeInterfaceCallbacks(CAConnectivityHandler_t* newHandler)
{
    printf("\nstoreInterfaceCallbacks Entry in Sample");
	#if 0
    newHandler->nextHandler = NULL;

    CAConnectivityHandler* tempCAConnectivityHandlers = gCAConnectivityHandlers;

    if (!tempCAConnectivityHandlers) {
        gCAConnectivityHandlers = newHandler;
        return;
    }
    while (tempCAConnectivityHandlers->nextHandler) {
        tempCAConnectivityHandlers = tempCAConnectivityHandlers->nextHandler;
    }

    tempCAConnectivityHandlers->nextHandler = newHandler;
	#endif
    printf("\nstoreInterfaceCallbacks Exit in Sample");
}

void interfaceRegisterCallback(CAConnectivityHandler_t handler,
                               CAConnectivityType_t connType)
{
    printf("interfaceRegisterCallback Entry in Sample\n");
    ConnectivityHandlerList* newConnectivityHandler = (ConnectivityHandlerList*) malloc(sizeof(ConnectivityHandlerList));
    if (NULL == newConnectivityHandler)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newConnectivityHandler->type = connType;
    newConnectivityHandler->handler = handler;
    storeInterfaceCallbacks(newConnectivityHandler);
    printf("interfaceRegisterCallback Exit in Sample\n");
}

void requestResponseHandler(CARemoteEndpoint_t* object, void* data)
{
    printf("\nrequestResponseHandler Entry in Sample");
    if (object == NULL || data == NULL) {
        printf("\nNULL Object");
        return;
    }

    if (object->addressInfo.BT.btMacAddress)
        printf("\nData Received from %s\n", object->addressInfo.BT.btMacAddress);
    if (data)
        printf("\nReceived Data : %s \n", (char*)data);
    /*
    gMainloopStatus = 1;
    if (mainloop)
    {
    g_main_loop_quit(mainloop);
    mainloop = NULL;
    }
    */
    printf("\nrequestResponseHandler Exit in Sample");
}


void networkInterfaceCallback(CALocalConnectivity_t* localEndPoint, CANetworkStatus_t networkConnectivityState)
{
    printf("\nnetworkInterfaceCallback Entry in Sample");
    if (localEndPoint == NULL) {
        printf("\nNULL Object");
        return;
    }

    if (localEndPoint->addressInfo.BT.btMacAddress)
        printf("\n Interface Address%s\n", localEndPoint->addressInfo.BT.btMacAddress);

    if (networkConnectivityState == CA_INTERFACE_UP)
        printf("\n BT Status is UP");
    else
        printf("\n BT Status is DOWN");
    printf("\nnetworkInterfaceCallback Exit in Sample");
}

void utc_liboic_startup(void)
{

}

void utc_liboic_cleanup(void)
{

}


int bt_tc_001_intializeBT_p(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    error = CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
    assert_eq(error, CA_STATUS_OK);
	CATerminateEDR();
    return 0;
}

int bt_tc_002_intializeBT_n(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    error = CAInitializeEDR(NULL, NULL, NULL);
    assert_eq(error, CA_STATUS_INVALID_PARAM);
    return 0;
}

int bt_tc_003_getBTInterfaceInformation_p(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    //Startup
    CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
	uint32_t size = 0;

    error = CAGetEDRInterfaceInformation(&localBtEndpoint,&size);
    assert_eq(error, CA_STATUS_OK);

	CATerminateEDR();
    // Now, check local endpoint information
    //assert_neq(localBtEndpoint->endpoint_info, NULL);
    //assert_neq(localBtEndpoint->endpoint_info->address, NULL);
    return 0;
}

int bt_tc_004_getBTInterfaceInformation_n(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    error = CAGetEDRInterfaceInformation(NULL,NULL);
    assert_eq(error, CA_STATUS_INVALID_PARAM);
    return 0;
}

int bt_tc_005_startAdapter_p(void)
{
    CAResult_t error = CA_STATUS_FAILED;
	CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
	error = CAStartEDR();
    assert_eq(error, CA_STATUS_OK);
	CAStopEDR();
	CATerminateEDR();
    return 0;
}

int bt_tc_006_sendBTUnicastData_p(void)
{
    int dataSize = 0;
    int ret = -1;
    int ref = 0;

    //Startup
    CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
	CAStartEDR();
	CAStartEDRListeningServer();

	CARemoteEndpoint_t endpoint;
	endpoint.connectivityType = CA_EDR;
    strncpy(endpoint.addressInfo.BT.btMacAddress, btAddr, CA_MACADDR_SIZE-1);
    endpoint.addressInfo.BT.btMacAddress[CA_MACADDR_SIZE-1] = '\0';
    endpoint.resourceUri = NULL;
    ret = CASendEDRUnicastData(&endpoint, coapData, strlen(coapData) + 1);
    sleep(5);

    //Cleanup
    CAStopEDR();
    CATerminateEDR();

    assert_eq(ret, strlen(coapData) + 1);
    return 0;
}

int bt_tc_007_sendBTUnicastData_n(void)
{
    int dataSize = 0;
    int ret = -1;
	CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
    ret = CASendEDRUnicastData(NULL, NULL, NULL);
    CATerminateEDR();
    assert_eq(ret, 0);
    return 0;
}

int bt_tc_008_sendBTMulticastDataToAll_p(void)
{
    int dataSize = 0;
    int ret = 0;
	CAStartEDRListeningServer();
    ret = CASendEDRMulticastData(coapData, strlen(coapData) + 1);
    assert_eq(ret, strlen(coapData) + 1);
	CAStopEDR();
    CATerminateEDR();
    return 0;
}

int bt_tc_009_sendBTMulticastDataToAll_n(void)
{
    int dataSize = 0;
    int ret = -1;
	CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
    ret = CASendEDRMulticastData(NULL, NULL);
	CATerminateEDR();
    assert_eq(ret, 0);
    return 0;
}

int bt_tc_010_StartListeningServer_p(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    int ref = 0;
    //Startup
    CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);

    error = CAStartEDRListeningServer();
    assert_eq(error, CA_STATUS_OK);
    //Cleanup
    CAStopEDR();
    CATerminateEDR();
    return 0;
}

int bt_tc_011_StopRfCommServer_p(void)
{
    CAResult_t error = CA_STATUS_FAILED;
    int ref = 0;
    //Startup
    CAInitializeEDR(interfaceRegisterCallback, requestResponseHandler, networkInterfaceCallback);
    CAStartEDRListeningServer();

    error = CAStopEDR();
    assert_eq(error, CA_STATUS_OK);
    //Cleanup
    CATerminateEDR();
    return 0;
}