/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************/

#include <bluetooth.h>
#include <bluetooth_type.h>
#include <bluetooth_product.h>

#include "cableserver.h"
#include <pthread.h>
#include "cacommon.h"
#include "caadapterutils.h"
#include <gio/gio.h>
#include "umutex.h"
#include "caqueueingthread.h"
#include "caadapterutils.h"
#include "camsgparser.h"
#include "cableutil.h"
#include "oic_string.h"
#include "oic_malloc.h"

/**
 * @def TZ_BLE_SERVER_TAG
 * @brief Logging tag for module name
 */
#define TZ_BLE_SERVER_TAG "TZ_BLE_GATT_SERVER"

/**
 * @def CA_BLE_SERVICE_UUID
 * @brief UUID of OIC service. This UUID is common across all platoform for LE transport.
 */
#define CA_BLE_SERVICE_UUID  "713D0000-503E-4C75-BA94-3148F18D941E"

/**
 * @def CA_BLE_READ_CHAR_UUID
 * @brief UUID of read characteristic. This UUID is common across all platoform for LE transport.
 */
#define CA_BLE_READ_CHAR_UUID "713D0002-503E-4C75-BA94-3148F18D941E"

/**
 * @def CA_BLE_WRITE_CHAR_UUID
 * @brief UUID of write characteristic. This UUID is common across all platoform for LE transport.
 */
#define CA_BLE_WRITE_CHAR_UUID "713D0003-503E-4C75-BA94-3148F18D941E"

/**
 * @def CA_BLE_INITIAL_BUF_SIZE
 * @brief Initial buffer size for Gatt Server.
 */
#define CA_BLE_INITIAL_BUF_SIZE 512

/**
 * @var g_gattSvcPath
 * @brief attribute handler for OIC server attribute.
 */
static char *g_gattSvcPath = NULL;

/**
 * @var g_gattReadCharPath
 * @brief attribute handler for readCharacteristic of OIC server
 */
static char *g_gattReadCharPath = NULL;

/**
 * @var g_gattWriteCharPath
 * @brief attribute handler for writeCharacteristic of OIC server
 */
static char *g_gattWriteCharPath = NULL;

/**
 * @var g_hAdvertiser
 * @brief handler for OIC advertiser.
 */
static bt_advertiser_h g_hAdvertiser = NULL;

/**
 * @var    g_bleServerDataReceivedCallback
 * @brief  Maintains the callback to be notified on receival of network packets from other
 *           BLE devices
 */
static CABLEServerDataReceivedCallback g_bleServerDataReceivedCallback = NULL;

/**
 * @var g_isBleGattServerStarted
 * @brief Boolean variable to keep the state of the GATTServer
 */
static bool g_isBleGattServerStarted = false;

/**
 * @var g_bleServerStateMutex
 * @brief Mutex to synchronize the calls to be done to the platform from GATTServer
 *           interfaces from different threads.
 */
static u_mutex g_bleServerStateMutex = NULL;

/**
 * @var g_bleCharacteristicMutex
 * @brief Mutex to synchronize writing operations on the characteristics.
 */
static  u_mutex g_bleCharacteristicMutex = NULL;

/**
 * @var g_bleServiceMutex
 * @brief  Mutex to synchronize to create the OIC service..
 */
static  u_mutex g_bleServiceMutex = NULL;

 /**
 * @var g_bleClientBDAddressMutex
 * @brief Mutex to synchronize the client BD Address update on server side.
 */
static u_mutex g_bleClientBDAddressMutex = NULL;


/**
 * @var g_bleReqRespCbMutex
 * @brief Mutex to synchronize access to the requestResponse callback to be called
 *           when the data needs to be sent from GATTClient.
 */
static  u_mutex g_bleReqRespCbMutex = NULL;

/**
 * @var g_bleServerThreadPoolMutex
 * @brief Mutex to synchronize the task to be pushed to thread pool.
 */
static u_mutex g_bleServerThreadPoolMutex = NULL;

/**
 * @struct gRemoteAddress
 * @brief Remote address of Gatt client
 *
 */
static char *g_remoteClientAddress = NULL;

/**
 * @var g_eventLoop
 * @brief gmainLoop to manage the threads to receive the callback from the platfrom.
 */
static GMainLoop *g_eventLoop = NULL;

/**
 * @var g_bleServerThreadPool
 * @brief reference to threadpool
 */
static u_thread_pool_t g_bleServerThreadPool = NULL;

void CABleGattServerConnectionStateChangedCb(int result, bool connected,
                                       const char *remoteAddress, void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "CABleGattConnectionStateChangedCb result[%d]", result);

    VERIFY_NON_NULL_VOID(remoteAddress, TZ_BLE_SERVER_TAG, "remote address is NULL");

    if (connected)
    {
        OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "Connected to [%s]", remoteAddress);

        u_mutex_lock(g_bleClientBDAddressMutex);
        g_remoteClientAddress = OICStrdup(remoteAddress);
        u_mutex_unlock(g_bleClientBDAddressMutex);

        VERIFY_NON_NULL_VOID(g_remoteClientAddress, TZ_BLE_SERVER_TAG, "Malloc failed");
    }
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CAStartBleGattServer()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    CAResult_t ret = CAInitGattServerMutexVariables();
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "CAInitGattServerMutexVariables failed!");
        CATerminateGattServerMutexVariables();
        return CA_SERVER_NOT_STARTED;
    }

    u_mutex_lock(g_bleServerThreadPoolMutex);
    if (NULL == g_bleServerThreadPool)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "g_bleServerThreadPool is NULL");
        u_mutex_unlock(g_bleServerThreadPoolMutex);
        return CA_STATUS_FAILED;
    }

    ret = u_thread_pool_add_task(g_bleServerThreadPool, CAStartBleGattServerThread,
                                 NULL);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "u_thread_pool_add_task failed with ret [%d]", ret);
        u_mutex_unlock(g_bleServerThreadPoolMutex);
        return CA_STATUS_FAILED;
    }

    u_mutex_unlock(g_bleServerThreadPoolMutex);
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CAStartBleGattServerThread(void *data)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");
    u_mutex_lock(g_bleServerStateMutex);
    if (true == g_isBleGattServerStarted)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "Gatt Server is already running");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    CAResult_t ret  =  CAInitBleGattService();
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "_bt_gatt_init_service failed");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    sleep(5); // Sleep is must because of the platform issue.

    char *serviceUUID = CA_BLE_SERVICE_UUID;

    ret  = CAAddNewBleServiceInGattServer(serviceUUID);
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "CAAddNewBleServiceInGattServer failed");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    char *charReadUUID = CA_BLE_READ_CHAR_UUID;
    char charReadValue[] = {33, 44, 55, 66}; // These are initial random values

    ret = CAAddNewCharacteristicsToGattServer(g_gattSvcPath, charReadUUID, charReadValue,
            CA_BLE_INITIAL_BUF_SIZE, false); // For Read Characteristics.
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "CAAddNewCharacteristicsToGattServer failed");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    char *charWriteUUID = CA_BLE_WRITE_CHAR_UUID;
    char charWriteValue[] = {33, 44, 55, 66}; // These are initial random values


    ret = CAAddNewCharacteristicsToGattServer(g_gattSvcPath, charWriteUUID, charWriteValue,
            CA_BLE_INITIAL_BUF_SIZE, true); // For Write Characteristics.
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "CAAddNewCharacteristicsToGattServer failed");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    ret = CARegisterBleServicewithGattServer(g_gattSvcPath);
    if (CA_STATUS_OK != ret )
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "CARegisterBleServicewithGattServer failed");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    int res = bt_gatt_set_connection_state_changed_cb(CABleGattServerConnectionStateChangedCb,
                                                          NULL);
    if (BT_ERROR_NONE != res)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                  "bt_gatt_set_connection_state_changed_cb Failed with return as [%s]",
                  CABTGetErrorMsg(res));
        return;
    }

    bt_adapter_le_create_advertiser(&g_hAdvertiser);
    if (NULL == g_hAdvertiser)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "g_hAdvertiser is NULL");
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    res = bt_adapter_le_start_advertising(g_hAdvertiser, NULL, NULL, NULL);
    if (BT_ERROR_NONE != res)
    {
        OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "bt_adapter_le_start_advertising failed with ret [%d] ",
                  res);
        u_mutex_unlock(g_bleServerStateMutex);
        CATerminateBleGattServer();
        return;
    }

    g_isBleGattServerStarted = true;

    u_mutex_unlock(g_bleServerStateMutex);

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG,
            "LE Server initialization complete.");

    GMainContext *thread_context = NULL;

    thread_context = g_main_context_new();

    g_eventLoop = g_main_loop_new(thread_context, FALSE);

    g_main_context_push_thread_default(thread_context);

    g_main_loop_run(g_eventLoop);

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CAStopBleGattServer()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    u_mutex_lock(g_bleServerStateMutex);

    if (false == g_isBleGattServerStarted)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "Gatt Server is not running to stop");

        u_mutex_unlock(g_bleServerStateMutex);
        return CA_STATUS_OK;
    }
    u_mutex_unlock(g_bleServerStateMutex);

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateBleGattServer()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    g_isBleGattServerStarted = false;

    u_mutex_lock(g_bleServerStateMutex);
    GMainContext  *context_event_loop = NULL;
    // Required for waking up the thread which is running in gmain loop
    if ( NULL != g_eventLoop)
    {
        context_event_loop = g_main_loop_get_context(g_eventLoop);
    }
    if (context_event_loop)
    {
        OIC_LOG_V(DEBUG,  TZ_BLE_SERVER_TAG, "g_eventLoop context %x", context_event_loop);
        g_main_context_wakeup(context_event_loop);

        // Kill g main loops and kill threads
        g_main_loop_quit(g_eventLoop);
    }
    else
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "g_eventLoop context is NULL");
    }

    if (NULL != g_hAdvertiser )
    {
        int ret = 0;
        ret  = bt_adapter_le_stop_advertising(g_hAdvertiser);
        if (0 != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                      "bt_adapter_le_stop_advertising failed with ret [%d]", ret);
        }

        ret = bt_adapter_le_destroy_advertiser(g_hAdvertiser);
        if (0 != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                      "bt_adapter_le_destroy_advertiser failed with ret [%d]", ret);
        }
        g_hAdvertiser = NULL;
    }

    CAResult_t res = CARemoveAllBleServicesFromGattServer();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "removeAllBleServicesFromGattServer failed");
    }

    res =  CADeInitBleGattService();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "_bt_gatt_deinit_service failed with ret [%d]", res);
    }

    u_mutex_lock(g_bleClientBDAddressMutex);
    OICFree(g_remoteClientAddress);
    u_mutex_unlock(g_bleClientBDAddressMutex);
    // free service Path(unique identifier for ble service)
    u_mutex_lock(g_bleServiceMutex);
    OICFree(g_gattSvcPath);
    g_gattSvcPath = NULL;
    u_mutex_unlock(g_bleServiceMutex);

    // freeing characteristics
    u_mutex_lock(g_bleCharacteristicMutex);
    OICFree(g_gattReadCharPath);
    g_gattReadCharPath = NULL;
    OICFree(g_gattWriteCharPath);
    g_gattWriteCharPath = NULL;
    u_mutex_unlock(g_bleCharacteristicMutex);

    u_mutex_unlock(g_bleServerStateMutex);

    // Terminating all mutex variables.
    CATerminateGattServerMutexVariables();
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CAInitGattServerMutexVariables()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");
    if (NULL == g_bleServerStateMutex)
    {
        g_bleServerStateMutex = u_mutex_new();
        if (NULL == g_bleServerStateMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "u_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleServiceMutex)
    {
        g_bleServiceMutex = u_mutex_new();
        if (NULL == g_bleServiceMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "u_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleCharacteristicMutex)
    {
        g_bleCharacteristicMutex = u_mutex_new();
        if (NULL == g_bleCharacteristicMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "u_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleReqRespCbMutex)
    {
        g_bleReqRespCbMutex = u_mutex_new();
        if (NULL == g_bleReqRespCbMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "u_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleClientBDAddressMutex)
    {
        g_bleClientBDAddressMutex = u_mutex_new();
        if (NULL == g_bleClientBDAddressMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "u_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    return CA_STATUS_OK;
}

void CATerminateGattServerMutexVariables()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");
    u_mutex_free(g_bleServerStateMutex);
    g_bleServerStateMutex = NULL;

    u_mutex_free(g_bleClientBDAddressMutex);
    g_bleClientBDAddressMutex = NULL;

    g_bleServerStateMutex = NULL;
    u_mutex_free(g_bleServiceMutex);
    g_bleServiceMutex = NULL;
    u_mutex_free(g_bleCharacteristicMutex);
    g_bleCharacteristicMutex = NULL;
    u_mutex_free(g_bleReqRespCbMutex);
    g_bleReqRespCbMutex = NULL;

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CAInitBleGattService()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    int ret =  _bt_gatt_init_service();
    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "_bt_gatt_deinit_service failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CADeInitBleGattService()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    int ret =  _bt_gatt_deinit_service();
    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "_bt_gatt_deinit_service failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CASetBleServerThreadPoolHandle(u_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");
    u_mutex_lock(g_bleServerThreadPoolMutex);
    g_bleServerThreadPool = handle;
    u_mutex_unlock(g_bleServerThreadPoolMutex);
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CAAddNewBleServiceInGattServer(const char *serviceUUID)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    VERIFY_NON_NULL(serviceUUID, NULL, "Param serviceUUID is NULL");

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "service uuid %s", serviceUUID);

    char *svcPath = NULL;

    int ret = bt_gatt_add_service(serviceUUID, &svcPath);
    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "bt_gatt_add_service failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    if (NULL != svcPath)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                  "AddNewBleServiceInGattServer ServicePath obtained is %s", svcPath);

        u_mutex_lock(g_bleServiceMutex);

        if (NULL != g_gattSvcPath)
        {
            OICFree(g_gattSvcPath);
            g_gattSvcPath = NULL;
        }
        g_gattSvcPath = svcPath;

        u_mutex_unlock(g_bleServiceMutex);
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CARemoveBleServiceFromGattServer(const char *svcPath)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    VERIFY_NON_NULL(svcPath, NULL, "Param svcPath is NULL");

    int ret = bt_gatt_remove_service(svcPath);

    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "bt_gatt_remove_service failed [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CARemoveAllBleServicesFromGattServer()
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");
    int ret = bt_gatt_delete_services();
    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "bt_gatt_delete_services  failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CABleGattRemoteCharacteristicWriteCb(char *charPath,
        unsigned char *charValue,
        int charValueLen, void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    if (NULL == charPath || NULL == charValue)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "Param callback values are NULL");
        return;
    }

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "charPath = [%s] charValue = [%s] len [%d]", charPath,
              charValue, charValueLen);

    char *data = (char *)OICMalloc(sizeof(char) * charValueLen);
    if (NULL == data)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "Malloc failed!");
        return;
    }

    strncpy(data, (char *)charValue, charValueLen);

    u_mutex_lock(g_bleReqRespCbMutex);
    if (NULL == g_bleServerDataReceivedCallback)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "gReqRespCallback is NULL!");
        u_mutex_unlock(g_bleReqRespCbMutex);
        OICFree(data);
        return;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "Sending data up !");
    u_mutex_lock(g_bleClientBDAddressMutex);
    uint32_t sentLength = 0;
    g_bleServerDataReceivedCallback(g_remoteClientAddress, OIC_BLE_SERVICE_ID,
                                     data, charValueLen, &sentLength);
    u_mutex_unlock(g_bleClientBDAddressMutex);

    u_mutex_unlock(g_bleReqRespCbMutex);

    OICFree(data);
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}

CAResult_t CARegisterBleServicewithGattServer(const char *svcPath)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    VERIFY_NON_NULL(svcPath, NULL, "Param svcPath is NULL");

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "svcPath:%s", svcPath);

    int ret = bt_gatt_register_service(svcPath, CABleGattRemoteCharacteristicWriteCb, NULL);

    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG, "bt_gatt_register_service failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAAddNewCharacteristicsToGattServer(const char *svcPath, const char *charUUID,
        const char *charValue, int charValueLen, bool read)
{

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    static const char *charFlags[] = {"read", "write", "notify", "indicate"};

    static const size_t flagLen = sizeof(charFlags) / sizeof(charFlags[0]);

    char *charPath = NULL;
    int ret = bt_gatt_add_characteristic(charUUID, charValue, charValueLen, charFlags, flagLen,
                  svcPath, &charPath);

    if (0 != ret || NULL == charPath)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                  "bt_gatt_add_characteristic  failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG,
              "bt_gatt_add_characteristic charPath obtained: %s", charPath);

    u_mutex_lock(g_bleCharacteristicMutex);

    if (!read)
    {
        if (NULL != g_gattReadCharPath)
        {
            OICFree(g_gattReadCharPath);
            g_gattReadCharPath = NULL;
        }
        g_gattReadCharPath = charPath;

    }
    else
    {
        if (NULL != g_gattWriteCharPath)
        {
            OICFree(g_gattWriteCharPath);
            g_gattWriteCharPath = NULL;
        }
        g_gattWriteCharPath = charPath;
    }

    u_mutex_unlock(g_bleCharacteristicMutex);

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CARemoveCharacteristicsFromGattServer(const char *charPath)
{
    ///TODO: There is no api provided in bluetooth.h for removing characteristics.
    return CA_STATUS_OK;
}

CAResult_t CAUpdateCharacteristicsInGattServer(const char *charValue,
        const uint32_t charValueLen)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    VERIFY_NON_NULL(charValue, NULL, "Param charValue is NULL");

    u_mutex_lock(g_bleCharacteristicMutex);

    if (NULL  == g_gattWriteCharPath)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "g_gattWriteCharPath is NULL");
        u_mutex_unlock(g_bleCharacteristicMutex);
        return CA_STATUS_FAILED;
    }

    char *data = (char *) OICMalloc(sizeof(char) * (charValueLen + 1));
    if (NULL == data)
    {
        OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "malloc failed!");
        u_mutex_unlock(g_bleCharacteristicMutex);
        return CA_STATUS_FAILED;
    }

    strncpy(data, charValue, charValueLen + 1);

    OIC_LOG_V(DEBUG, TZ_BLE_SERVER_TAG, "updating characteristics char [%s] data [%s] dataLen [%d]",
              (const char *)g_gattWriteCharPath, data, charValueLen);

    int ret =  bt_gatt_update_characteristic(g_gattWriteCharPath, data, charValueLen);
    if (0 != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_SERVER_TAG,
                  "bt_gatt_update_characteristic failed with return [%d]", ret);
        OICFree(data);
        data = NULL;
        u_mutex_unlock(g_bleCharacteristicMutex);
        return CA_STATUS_FAILED;
    }

    OICFree(data);
    u_mutex_unlock(g_bleCharacteristicMutex);

    OIC_LOG(ERROR, TZ_BLE_SERVER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CASetBLEReqRespServerCallback(CABLEServerDataReceivedCallback callback)
{
    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "IN");

    u_mutex_lock(g_bleReqRespCbMutex);

    g_bleServerDataReceivedCallback = callback;

    u_mutex_unlock(g_bleReqRespCbMutex);

    OIC_LOG(DEBUG, TZ_BLE_SERVER_TAG, "OUT");
}
