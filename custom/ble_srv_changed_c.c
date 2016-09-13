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

#include "ble_srv_changed_c.h"
#include <stdlib.h>
#include "ble.h"
#include "nrf_log.h"

#include "sdk_common.h"

#define SVC_CH_LOG          NRF_LOG_INFO              /**< A debug logger macro that can be used in this file to do logging information over UART. */
#define CCCD_VALUE          0x0002                    /**< CCCD value to enable indication */

static bool             m_uuid_registered = false;    /**< Boolean telling whether GATT UUID is registered to ble_db_discovery, needed because the module is not re-entrant. */
static const ble_uuid_t m_svc_changed_uuid = {BLE_UUID_GATT, BLE_UUID_TYPE_BLE};    /**< Service changed indication */


/**@brief Function for handling the indication / notifications from server */
static void on_hvx (ble_srv_changed_c_t * const p_srv_changed_c,
                    const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    if (p_ble_gattc_evt->params.hvx.handle == p_srv_changed_c->srv_changed_char.handle_value)
    {
        if (p_srv_changed_c->p_handler != NULL)
        {
            ble_srv_changed_c_evt_t evt;
            ble_srv_changed_c_evt_handler_t evt_handler = (ble_srv_changed_c_evt_handler_t)p_srv_changed_c->p_handler;

            evt.evt_type    = BLE_SRV_CHANGED_C_EVT_SRV_CHANGED;
            evt.conn_handle = p_ble_gattc_evt->conn_handle;
            SVC_CH_LOG ("[SRV_CH] Service-changed indication.\n\r");
            evt_handler(&evt);
        }
    }
}


uint32_t ble_srv_changed_c_init(ble_srv_changed_c_t * const p_srv_changed_c,
                                ble_srv_changed_c_evt_handler_t evt_handler)
{
    uint32_t err_code = NRF_SUCCESS;
    VERIFY_PARAM_NOT_NULL(evt_handler);
    memset (p_srv_changed_c, 0, sizeof(ble_srv_changed_c_t));

    p_srv_changed_c->char_found  = false;
    p_srv_changed_c->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_srv_changed_c->p_handler   = (void *)evt_handler;

    if (!m_uuid_registered)
    {
        err_code = ble_db_discovery_evt_register(&m_svc_changed_uuid);
        if (err_code == NRF_SUCCESS)
        {
            m_uuid_registered = true;
            p_srv_changed_c->initialized = true;
        }
    }
    else
    {
        p_srv_changed_c->initialized = true;
    }
    return err_code;
}


uint32_t ble_srv_changed_c_enable_indication(ble_srv_changed_c_t * const p_srv_changed_c,
                                             bool enable)
{
    uint32_t err_code = NRF_SUCCESS;
    VERIFY_TRUE(p_srv_changed_c->char_found, NRF_ERROR_NOT_FOUND);
    VERIFY_TRUE(p_srv_changed_c->initialized, NRF_ERROR_INVALID_STATE);

    if (p_srv_changed_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    ble_gattc_write_params_t gattc_params;
    uint16_t cccd_val = (enable)? CCCD_VALUE: 0;

    gattc_params.handle   = p_srv_changed_c->cccd_handle;
    gattc_params.len      = 2;
    gattc_params.p_value  = (uint8_t *)&cccd_val;
    gattc_params.offset   = 0;
    gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;

    err_code = sd_ble_gattc_write(p_srv_changed_c->conn_handle, &gattc_params);
    return err_code;
}


void ble_srv_changed_c_on_db_disc_evt(ble_srv_changed_c_t * const p_srv_changed_c,
                                       ble_db_discovery_evt_t * p_evt)
{
    ble_srv_changed_c_evt_t evt;
    ble_gatt_db_char_t * p_chars;
    p_chars = p_evt->params.discovered_db.charateristics;

    evt.evt_type = BLE_SRV_CHANGED_C_EVT_CHAR_NOT_FOUND;

    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
    {
        // Find the handles of the service changed characteristics.
        for (uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            if (p_chars[i].characteristic.uuid.uuid == BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED &&
                p_chars[i].characteristic.char_props.indicate != 0)
            {
                memcpy (&p_srv_changed_c->srv_changed_char, &p_chars[i].characteristic, sizeof(ble_gattc_char_t));
                
                p_srv_changed_c->cccd_handle = p_chars[i].cccd_handle;
                p_srv_changed_c->char_found  = true;
                evt.evt_type                 = BLE_SRV_CHANGED_C_EVT_CHAR_FOUND;
                SVC_CH_LOG ("[SRV_CH] Service-changed characteristics found.\n\r");
                break;
            }
        }
    }

    if (p_srv_changed_c->p_handler != NULL)
    {
        ble_srv_changed_c_evt_handler_t evt_handler = (ble_srv_changed_c_evt_handler_t)p_srv_changed_c->p_handler;

        evt.conn_handle = p_srv_changed_c->conn_handle;
        evt_handler(&evt);
    }
}


void ble_srv_changed_c_on_ble_evt(ble_srv_changed_c_t * const p_srv_changed_c,
                                  const ble_evt_t * const p_ble_evt)
{
    VERIFY_PARAM_NOT_NULL_VOID(p_srv_changed_c);
    VERIFY_PARAM_NOT_NULL_VOID(p_ble_evt);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_srv_changed_c->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            p_srv_changed_c->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTC_EVT_HVX:
            on_hvx (p_srv_changed_c, &(p_ble_evt->evt.gattc_evt));
            break;

        default:
            break;
    }
}


