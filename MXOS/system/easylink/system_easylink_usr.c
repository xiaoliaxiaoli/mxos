/**
 ******************************************************************************
 * @file    system_easylink.c
 * @author  William Xu
 * @version V1.0.0
 * @date    20-July-2015
 * @brief   This file provide the easylink function for quick provisioning and
 *          first time configuration.
 ******************************************************************************
 *
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

#include "mxos.h"
#include "system_internal.h"
#include "StringUtils.h"

#include "system.h"
#include "easylink_internal.h"

//#if (MXOS_WLAN_CONFIG_MODE == CONFIG_MODE_USER)

/******************************************************
 *               Function Declarations
 ******************************************************/
/* Perform easylink and connect to wlan */
static void easylink_usr_thread( uint32_t inContext );

/******************************************************
 *               Variables Definitions
 ******************************************************/
static mxos_semaphore_t easylink_connect_sem; /**< Used to suspend thread while connection. */
static mxos_thread_t easylink_usr_thread_handler = NULL;
static bool easylink_thread_force_exit = false;



/* MXOS callback when WiFi status is changed */
static void easylink_wifi_status_cb( WiFiEvent event, system_context_t * const inContext )
{
    require( inContext, exit );

    switch ( event )
    {
        case NOTIFY_STATION_UP:
            /* Connected to AP, means that the wlan configuration is right, update configuration in flash */
            inContext->flashContentInRam.mxosSystemConfig.configured = allConfigured;
            mxos_system_context_update( &inContext->flashContentInRam ); //Update Flash content
            mxos_rtos_set_semaphore( &easylink_connect_sem ); //Notify Easylink thread
            break;
        default:
            break;
    }
    exit:
    return;
}

static void easylink_usr_thread( uint32_t inContext )
{
    OSStatus err = kNoErr;
    system_context_t *Context = (system_context_t *) inContext;

    easylink_thread_force_exit = false;

    mxos_system_notify_register( mxos_notify_WIFI_STATUS_CHANGED, (void *)easylink_wifi_status_cb, (void *) Context );
    mxos_rtos_init_semaphore( &easylink_connect_sem, 1 );

    system_log("Start easylink user mode");
    mxos_system_delegate_config_will_start( );

    /* Developer should save the ssid/key to system_context_t and connect to AP.
     * NOTIFY_STATION_UP event will save the new ssid, key to flash and wake up the easylink routine */
    while ( mxos_rtos_get_semaphore( &easylink_connect_sem, 0 ) == kNoErr );
    err = mxos_rtos_get_semaphore( &easylink_connect_sem, MXOS_WAIT_FOREVER );

    /* Easylink force exit by user, clean and exit */
    if ( err != kNoErr && easylink_thread_force_exit ){
        mxosWlanSuspend( );
        system_log("EasyLink connection canceled by user");
        easylink_thread_force_exit = false;
        goto exit;
    }

    mxos_system_delegate_config_success( CONFIG_BY_USER );

exit:
    mxos_system_delegate_config_will_stop( );
    mxos_system_notify_remove( mxos_notify_WIFI_STATUS_CHANGED, (void *) easylink_wifi_status_cb );

    mxos_rtos_deinit_semaphore( &easylink_connect_sem );
    easylink_usr_thread_handler = NULL;
    mxos_rtos_delete_thread( NULL );
}

OSStatus mxos_easylink_usr( mxos_Context_t * const in_context, mxos_bool_t enable )
{
    OSStatus err = kUnknownErr;

    require_action( in_context, exit, err = kNotPreparedErr );

    easylink_remove_bonjour( INTERFACE_STA );

    /* easylink thread existed? stop! */
    if ( easylink_usr_thread_handler ) {
        /* easylink usr thread existed? stop! */
        system_log("EasyLink usr processing, force stop..");
        easylink_thread_force_exit = true;
        mxos_rtos_thread_force_awake( &easylink_usr_thread_handler );
        mxos_rtos_thread_join( &easylink_usr_thread_handler );
    }

    if ( enable == MXOS_TRUE ) {
        err = mxos_rtos_create_thread( &easylink_usr_thread_handler, MXOS_APPLICATION_PRIORITY, "EASYLINK USR",
                                       easylink_usr_thread, 0x1000, (mxos_thread_arg_t) in_context );
        require_noerr_string( err, exit, "ERROR: Unable to start the EasyLink usr thread." );

        /* Make sure easylink is already running, and waiting for sem trigger */
        mxos_rtos_delay_milliseconds( 100 );
    }

    exit:
    return err;
}

//#endif
