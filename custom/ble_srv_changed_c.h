/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */


#ifndef BLE_SRV_CHANGED_H__
#define BLE_SRV_CHANGED_H__

#include <stdint.h>
#include "ble_gattc.h"
#include "ble.h"
#include "nrf_error.h"
#include "ble_srv_common.h"
#include "ble_db_discovery.h"


/**@brief   Type of the service changed client event.
 */
typedef enum
{
    BLE_SRV_CHANGED_C_EVT_CHAR_FOUND,           /**< Event indicating that the service changed indication has been found. */
    BLE_SRV_CHANGED_C_EVT_CHAR_NOT_FOUND,       /**< Event indicating that the service changed indication has NOT been found. */
    BLE_SRV_CHANGED_C_EVT_SRV_CHANGED,          /**< Event indicating that service changed indication has been received. */
    BLE_SRV_CHANGED_C_EVT_ERR_RSP,              /**< Error when sending response. */
} ble_srv_changed_c_evt_type_t;


/**@brief   Structure for holding the information related to the service changed indication at the server.
 *
 * @details This module identifies a remote database. Use one instance of this structure per 
 *          connection.
 *
 * @warning This structure must be zero-initialized.
 */
typedef struct
{
    ble_gattc_char_t srv_changed_char;  /**< Information of the service changed characteristics. */
    bool             initialized;       /**< Boolean telling whether the context has been initialized or not. */
    bool             char_found;        /**< Boolean telling whether service changed indication has been found.*/
    uint16_t         cccd_handle;       /**< Information related to the service changed indication discovered. */
    uint16_t         conn_handle;       /**< Active connection handle */
    void           * p_handler;         /**< Pointer to event handler function */
} ble_srv_changed_c_t;


/**@brief   Structure containing the event from the service changed client module to the application.
 */
typedef struct
{
    ble_srv_changed_c_evt_type_t evt_type;      /**< Type of event. */
    uint16_t                     conn_handle;   /**< Handle of the connection for which this event has occurred. */
} ble_srv_changed_c_evt_t;


/**@brief   Service changed handler type. */
typedef void (* ble_srv_changed_c_evt_handler_t)(ble_srv_changed_c_evt_t * p_evt);


/**@brief     Function for initializing the service changed client module.
 *
 * @param[in,out] p_srv_changed_c   Pointer to service changed client structure.
 * @param[in]     evt_handler       Event handler to be called by the service changed client
 *                                  module when any related event occurs.
 *
 * @retval  NRF_SUCCESS             On successful initialization.
 * @retval  NRF_ERROR_NULL          If the handler was NULL.
 */
uint32_t ble_srv_changed_c_init(ble_srv_changed_c_t * const p_srv_changed_c,
                                ble_srv_changed_c_evt_handler_t evt_handler);


/**@brief Function for enabling remote indication.
 *
 * @param[in,out] p_srv_changed_c   Pointer to service changed client structure.
 * @param[in]     enable            True to enable service changed remte indication, false to disable.
 *
 * @retval  NRF_SUCCESS             Operation success.
 */
uint32_t ble_srv_changed_c_enable_indication(ble_srv_changed_c_t * const p_srv_changed_c,
                                             bool enable);


/**@brief     Function for handling events from the database discovery module.
 *
 * @details   This function will handle an event from the database discovery module, and determine
 *            if it relates to the discovery of service changed characteristics at the peer. If so,
 *            it will call the application's event handler indicating that service changed
 *            characteristics has been discovered at the peer.
 *
 * @param[in,out] p_srv_changed_c   Pointer to the service change client structure.
 * @param[in] p_evt                 Pointer to the event received from the database discovery module.
 */
 void ble_srv_changed_c_on_db_disc_evt(ble_srv_changed_c_t * const p_srv_changed_c,
                                       ble_db_discovery_evt_t * p_evt);

                                
/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in,out] p_srv_changed_c   Pointer to the service changed client structure.
 * @param[in]     p_ble_evt         Pointer to the BLE event received.
 */
void ble_srv_changed_c_on_ble_evt(ble_srv_changed_c_t * const p_srv_changed_c,
                                  const ble_evt_t * const p_ble_evt);

#endif // BLE_SRV_CHANGED_H__

/** @} */
