/**
 ******************************************************************************
 * @file    config_server.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-May-2014
 * @brief   Local TCP server for mxos device configuration
 ******************************************************************************
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
#include "mxos_board.h"
#include "easylink_internal.h"

#include "SocketUtils.h"
#include "HTTPUtils.h"
#include "StringUtils.h"
#include "CheckSumUtils.h"

#if MXOS_CONFIG_SERVER_ENABLE

#define kCONFIGURLRead          "/config-read"
#define kCONFIGURLWrite         "/config-write"
#define kCONFIGURLWriteByUAP    "/config-write-uap"  /* Don't reboot but connect to AP immediately */
#define kCONFIGURLOTA           "/OTA"

#define kMIMEType_MXCHIP_OTA    "application/ota-stream"

typedef struct _configContext_t{
  uint32_t offset;
  bool     isFlashLocked;
  CRC16_Context crc16_contex;
} configContext_t;

extern merr_t     ConfigIncommingJsonMessage( int fd, const char *input, bool *need_reboot, mxos_Context_t * const inContext );
extern json_object* ConfigCreateReportJsonMessage( mxos_Context_t * const inContext );

static void localConfiglistener_thread(void* arg);
static void localConfig_thread(void* arg);
static merr_t _LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, system_context_t * const inContext);
static merr_t onReceivedData(struct _HTTPHeader_t * httpHeader, uint32_t pos, uint8_t * data, size_t len, void * userContext );
static void onClearHTTPHeader(struct _HTTPHeader_t * httpHeader, void * userContext );
static config_server_uap_configured_cb _uap_configured_cb = NULL;

bool is_config_server_established = false;

/* Defined in uAP config mode */
extern merr_t ConfigIncommingJsonMessageUAP( int fd, const uint8_t *input, size_t size, system_context_t * const inContext );
extern system_context_t* sys_context;

static mos_semphr_id_t close_listener_sem = NULL, close_client_sem[ MAX_TCP_CLIENT_PER_SERVER ] = { NULL };

WEAK void config_server_delegate_report( json_object *app_menu, mxos_Context_t *in_context )
{
  UNUSED_PARAMETER(app_menu);
  UNUSED_PARAMETER(in_context);
  return;
}

WEAK void config_server_delegate_recv( const char *key, json_object *value, bool *need_reboot, mxos_Context_t *in_context )
{
  UNUSED_PARAMETER(key);
  UNUSED_PARAMETER(value);
  UNUSED_PARAMETER(need_reboot);
  UNUSED_PARAMETER(in_context);
  return;  
}

void config_server_set_uap_cb( config_server_uap_configured_cb callback )
{
    _uap_configured_cb = callback;
}

merr_t config_server_start ( void )
{
  int i = 0;
  merr_t err = kNoErr;
  
  require( sys_context, exit );
  
  if( is_config_server_established )
    return kNoErr;

  is_config_server_established = true;

  close_listener_sem = NULL;
  for (; i < MAX_TCP_CLIENT_PER_SERVER; i++)
    close_client_sem[ i ] = NULL;
  require_action(mos_thread_new( MXOS_APPLICATION_PRIORITY, "Config Server", localConfiglistener_thread, 
  STACK_SIZE_LOCAL_CONFIG_SERVER_THREAD, NULL ) != NULL, exit, err = kGeneralErr);
  
  mxos_thread_msleep(200);

exit:
  return err;
}

merr_t config_server_stop( void )
{
  int i = 0;
  merr_t err = kNoErr;

  if( !is_config_server_established )
    return kNoErr;

  for (; i < MAX_TCP_CLIENT_PER_SERVER; i++){
    if( close_client_sem[ i ] != NULL )
      mos_semphr_release(close_client_sem[ i ] );
  }
  mxos_thread_msleep(50);

  if( close_listener_sem != NULL )
    mos_semphr_release(close_listener_sem );

  mxos_thread_msleep(500);
  is_config_server_established = false;
  
  return err;
}

void localConfiglistener_thread(void * arg)
{
  merr_t err = kUnknownErr;
  int j;
  struct sockaddr_in addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int localConfiglistener_fd = -1;
  int close_listener_fd = -1;

  close_listener_sem = mos_semphr_new(1);
  close_listener_fd = mxos_create_event_fd( close_listener_sem );

  /*Establish a TCP server fd that accept the tcp clients connections*/ 
  localConfiglistener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  require_action(IsValidSocket( localConfiglistener_fd ), exit, err = kNoResourcesErr );
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr =INADDR_ANY;
  addr.sin_port = htons(MXOS_CONFIG_SERVER_PORT);
  err = bind(localConfiglistener_fd, (struct sockaddr *)&addr, sizeof(addr));
  require_noerr( err, exit );

  err = listen(localConfiglistener_fd, 1);
  require_noerr( err, exit );

  system_log("Config Server established at port: %d, fd: %d", MXOS_CONFIG_SERVER_PORT, localConfiglistener_fd);
  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(localConfiglistener_fd, &readfds);
    FD_SET(close_listener_fd, &readfds);
    select( Max(localConfiglistener_fd, close_listener_fd) + 1, &readfds, NULL, NULL, NULL);

    /* Check close requests */
    if(FD_ISSET(close_listener_fd, &readfds)){
      mos_semphr_acquire(close_listener_sem, 0 );
      goto exit;
    }

    /* Check tcp connection requests */
    if(FD_ISSET(localConfiglistener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_in);
      j = accept(localConfiglistener_fd, (struct sockaddr *)&addr, (socklen_t *)&sockaddr_t_size);
      if ( IsValidSocket( j ) ) {
        strcpy(ip_address,inet_ntoa( addr.sin_addr ));
        system_log("Config Client %s:%d connected, fd: %d", ip_address, addr.sin_port, j);
        if(NULL ==  mos_thread_new( MXOS_APPLICATION_PRIORITY, "Config Clients", localConfig_thread, STACK_SIZE_LOCAL_CONFIG_CLIENT_THREAD, (void *)j) )
          SocketClose(&j);
      }
    }
  }

exit:
    if( close_listener_sem != NULL ){
      mxos_delete_event_fd( close_listener_fd );
      mos_semphr_delete(close_listener_sem );
      close_listener_sem = NULL;
    };
    system_log("Exit: Config listener exit with err = %d", err);
    SocketClose( &localConfiglistener_fd );
    is_config_server_established = false;
    mos_thread_delete(NULL);
    return;
}

void localConfig_thread(void* arg)
{
  merr_t err = kNoErr;
  int clientFd = (int)arg;
  int clientFdIsSet;
  int close_sem_index;
  fd_set readfds;
  struct timeval t;
  HTTPHeader_t *httpHeader = NULL;
  int close_client_fd = -1;
  configContext_t httpContext = {0, false};

  for( close_sem_index = 0; close_sem_index < MAX_TCP_CLIENT_PER_SERVER; close_sem_index++ ){
    if( close_client_sem[close_sem_index] == NULL )
      break;
  }

  if( close_sem_index == MAX_TCP_CLIENT_PER_SERVER){
    mos_thread_delete(NULL);
    return;
  }

  close_client_sem[close_sem_index] = mos_semphr_new(1);
  close_client_fd = mxos_create_event_fd( close_client_sem[close_sem_index] );

  httpHeader = HTTPHeaderCreateWithCallback( 512, onReceivedData, onClearHTTPHeader, &httpContext );
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );

  t.tv_sec = 60;
  t.tv_usec = 0;
  system_log("Free memory %d bytes", mxos_get_mem_info()->free_memory) ; 

  while(1){
    FD_ZERO(&readfds);
    FD_SET(clientFd, &readfds);
    FD_SET(close_client_fd, &readfds);
    clientFdIsSet = 0;

    if(httpHeader->len == 0){
      require(select( Max(clientFd, close_client_fd) + 1 , &readfds, NULL, NULL, &t) >= 0, exit);
      clientFdIsSet = FD_ISSET(clientFd, &readfds);
    }

    /* Check close requests */
    if(FD_ISSET(close_client_fd, &readfds)){
      mos_semphr_acquire(close_client_sem[close_sem_index], 0 );
      err = kConnectionErr;
      goto exit;
    }    
  
    if(clientFdIsSet||httpHeader->len){
      err = SocketReadHTTPHeader( clientFd, httpHeader );

      switch ( err )
      {
        case kNoErr:
          // Read the rest of the HTTP body if necessary
          //do{
          err = SocketReadHTTPBody( clientFd, httpHeader );
          
          if(httpHeader->dataEndedbyClose == true){
            err = _LocalConfigRespondInComingMessage( clientFd, httpHeader, sys_context );
            require_noerr(err, exit);
            err = kConnectionErr;
            goto exit;
          }else{
            require_noerr(err, exit);
            err = _LocalConfigRespondInComingMessage( clientFd, httpHeader, sys_context );
            require_noerr(err, exit);
          }

          HTTPHeaderClear( httpHeader );
        break;

        case EWOULDBLOCK:
            // NO-OP, keep reading
        break;

        case kNoSpaceErr:
          system_log("ERROR: Cannot fit HTTPHeader.");
          goto exit;
        
        case kConnectionErr:
          // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
          system_log("ERROR: Connection closed.");
          goto exit;

        default:
          system_log("ERROR: HTTP Header parse internal error: %d", err);
          goto exit;
      }
    }
  }

exit:
  system_log("Exit: Client exit with err = %d", err);
  SocketClose(&clientFd);

  if( close_client_sem[close_sem_index] != NULL )
  {
    mxos_delete_event_fd( close_client_fd );
    mos_semphr_delete(close_client_sem[close_sem_index] );
    close_client_sem[close_sem_index] = NULL;
  };

  HTTPHeaderDestory( &httpHeader );
  mos_thread_delete(NULL);
  return;
}

static merr_t onReceivedData(struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext )
{
  merr_t err = kUnknownErr;
  const char *    value;
  size_t          valueSize;
  configContext_t *context = (configContext_t *)inUserContext;
  mxos_logic_partition_t* ota_partition = mhal_flash_get_info( MXOS_PARTITION_OTA_TEMP );

  err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
  if(err == kNoErr && strnicmpx( value, valueSize, kMIMEType_MXCHIP_OTA ) == 0){

    if( ota_partition->partition_owner == MXOS_FLASH_NONE ){
      system_log("OTA storage is not exist");
      return kUnsupportedErr;
    }

    if(inPos==0 && inLen==0)
        return err;

     if(inPos == 0){
       context->offset = 0x0;
       CRC16_Init( &context->crc16_contex );
       mos_mutex_lock(sys_context->flashContentInRam_mutex); //We are write the Flash content, no other write is possible
       context->isFlashLocked = true;
       err = mhal_flash_erase( MXOS_PARTITION_OTA_TEMP, 0x0, ota_partition->partition_length);
       require_noerr(err, flashErrExit);
       err = mhal_flash_write( MXOS_PARTITION_OTA_TEMP, &context->offset, (uint8_t *)inData, inLen);
       require_noerr(err, flashErrExit);
       CRC16_Update( &context->crc16_contex, inData, inLen);
     }else{
       err = mhal_flash_write( MXOS_PARTITION_OTA_TEMP, &context->offset, (uint8_t *)inData, inLen);
       require_noerr(err, flashErrExit);
       CRC16_Update( &context->crc16_contex, inData, inLen);
     }
  }
  else{
    return kUnsupportedErr;
  }

  if(err!=kNoErr)  system_log("onReceivedData");
  return err;

flashErrExit:
  mos_mutex_unlock(sys_context->flashContentInRam_mutex);
  return err;
}

static void onClearHTTPHeader(struct _HTTPHeader_t * inHeader, void * inUserContext )
{
  UNUSED_PARAMETER(inHeader);
  configContext_t *context = (configContext_t *)inUserContext;

  if(context->isFlashLocked == true){
    mos_mutex_unlock(sys_context->flashContentInRam_mutex);
    context->isFlashLocked = false;
  }
 }

merr_t _LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, system_context_t * const inContext)
{
  merr_t err = kUnknownErr;
  const char *  json_str;
  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;
  json_object* report = NULL, *config = NULL;
  bool need_reboot = false;
  uint16_t crc;
  configContext_t *http_context = (configContext_t *)inHeader->userContext;
  mxos_logic_partition_t* ota_partition = mhal_flash_get_info( MXOS_PARTITION_OTA_TEMP );
  char name[50];
  IPStatusTypedef ip;
  mwifi_get_ip(&ip, INTERFACE_STA);

  json_object *sectors, *sector = NULL;

  if(HTTPHeaderMatchURL( inHeader, kCONFIGURLRead ) == kNoErr){    
    //report = ConfigCreateReportJsonMessage( inContext );

    mos_mutex_lock(inContext->flashContentInRam_mutex);
    snprintf(name, 50, "%s(%c%c%c%c%c%c)",MODEL, 
                                          inContext->mxosStatus.mac[9],  inContext->mxosStatus.mac[10], 
                                          inContext->mxosStatus.mac[12], inContext->mxosStatus.mac[13],
                                          inContext->mxosStatus.mac[15], inContext->mxosStatus.mac[16]);
    report = json_object_new_object();
    require_action(report, exit, err = kNoMemoryErr);

    sectors = json_object_new_array();
    require( sectors, exit );

    json_object_object_add(report, "T", json_object_new_string("Current Configuration"));
    json_object_object_add(report, "N", json_object_new_string(name));
    json_object_object_add(report, "C", sectors);

    json_object_object_add(report, "PO", json_object_new_string(PROTOCOL));
    json_object_object_add(report, "HD", json_object_new_string(HARDWARE_REVISION));
    json_object_object_add(report, "FW", json_object_new_string(FIRMWARE_REVISION));
    json_object_object_add(report, "RF", json_object_new_string(inContext->mxosStatus.rf_version));

#if MXOS_CONFIG_SERVER_REPORT_SYSTEM_DATA
    /*Sector 1*/
    sector = json_object_new_array();
    require( sector, exit );
    err = config_server_create_sector(sectors, "MXOS SYSTEM",    sector);
    require_noerr(err, exit);

      /*name cell*/
      err = config_server_create_string_cell(sector, "Device Name",   inContext->flashContentInRam.mxosSystemConfig.name,               "RW", NULL);
      require_noerr(err, exit);

      //RF power save switcher cell
      err = config_server_create_bool_cell(sector, "RF power save",   inContext->flashContentInRam.mxosSystemConfig.rfPowerSaveEnable,  "RW");
      require_noerr(err, exit);

      //MCU power save switcher cell
      err = config_server_create_bool_cell(sector, "MCU power save",  inContext->flashContentInRam.mxosSystemConfig.mcuPowerSaveEnable, "RW");
      require_noerr(err, exit);

      /*SSID cell*/
      err = config_server_create_string_cell(sector, "Wi-Fi",         inContext->flashContentInRam.mxosSystemConfig.ssid,     "RW", NULL);
      require_noerr(err, exit);
      /*PASSWORD cell*/
      err = config_server_create_string_cell(sector, "Password",      inContext->flashContentInRam.mxosSystemConfig.user_key, "RW", NULL);
      require_noerr(err, exit);
      /*DHCP cell*/
      err = config_server_create_bool_cell(sector, "DHCP",            inContext->flashContentInRam.mxosSystemConfig.dhcpEnable,   "RW");
      require_noerr(err, exit);
      /*Local cell*/
      err = config_server_create_string_cell(sector, "IP address",  ip.ip,   "RW", NULL);
      require_noerr(err, exit);
      /*Netmask cell*/
      err = config_server_create_string_cell(sector, "Net Mask",    ip.mask,   "RW", NULL);
      require_noerr(err, exit);
      /*Gateway cell*/
      err = config_server_create_string_cell(sector, "Gateway",     ip.gate,   "RW", NULL);
      require_noerr(err, exit);
      /*DNS server cell*/
      err = config_server_create_string_cell(sector, "DNS Server",  ip.dns, "RW", NULL);
      require_noerr(err, exit);
#endif
      
    /*Sector 2*/
    sector = json_object_new_array();
    require( sector, exit );
    err = config_server_create_sector(sectors, "APPLICATION",    sector);
    require_noerr(err, exit);

    config_server_delegate_report( sector, &inContext->flashContentInRam );

    mos_mutex_unlock(inContext->flashContentInRam_mutex);

    json_str = json_object_to_json_string(report);
    require_action( json_str, exit, err = kNoMemoryErr );
    system_log("Send config object=%s", json_str);
    err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_JSON, strlen(json_str), &httpResponse, &httpResponseLen );
    require_noerr( err, exit );
    require( httpResponse, exit );
    err = SocketSend( fd, httpResponse, httpResponseLen );
    require_noerr( err, exit );
    err = SocketSend( fd, (uint8_t *)json_str, strlen(json_str) );
    require_noerr( err, exit );
    system_log("Current configuration sent");
    goto exit;
  }
  else if(HTTPHeaderMatchURL( inHeader, kCONFIGURLWrite ) == kNoErr){
    if(inHeader->contentLength > 0){
      system_log("Recv new configuration, apply");

      err =  CreateSimpleHTTPOKMessage( &httpResponse, &httpResponseLen );
      require_noerr( err, exit );
      require( httpResponse, exit );
      err = SocketSend( fd, httpResponse, httpResponseLen );
      require_noerr( err, exit );

      config = json_tokener_parse(inHeader->extraDataPtr);
      require_action(config, exit, err = kUnknownErr);
      system_log("Recv config object=%s", json_object_to_json_string(config));
      mos_mutex_lock(inContext->flashContentInRam_mutex);
      json_object_object_foreach(config, key, val) {
        if(!strcmp(key, "Device Name")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.name, json_object_get_string(val), maxNameLen);
          need_reboot = true;
        }else if(!strcmp(key, "RF power save")){
          inContext->flashContentInRam.mxosSystemConfig.rfPowerSaveEnable = json_object_get_boolean(val);
          need_reboot = true;
        }else if(!strcmp(key, "MCU power save")){
          inContext->flashContentInRam.mxosSystemConfig.mcuPowerSaveEnable = json_object_get_boolean(val);
          need_reboot = true;
        }else if(!strcmp(key, "Wi-Fi")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.ssid, json_object_get_string(val), maxSsidLen);
          inContext->flashContentInRam.mxosSystemConfig.channel = 0;
          memset(inContext->flashContentInRam.mxosSystemConfig.bssid, 0x0, 6);
          inContext->flashContentInRam.mxosSystemConfig.security = SECURITY_TYPE_AUTO;
          memcpy(inContext->flashContentInRam.mxosSystemConfig.key, inContext->flashContentInRam.mxosSystemConfig.user_key, maxKeyLen);
          inContext->flashContentInRam.mxosSystemConfig.keyLength = inContext->flashContentInRam.mxosSystemConfig.user_keyLength;
          need_reboot = true;
        }else if(!strcmp(key, "Password")){
          inContext->flashContentInRam.mxosSystemConfig.security = SECURITY_TYPE_AUTO;
          strncpy(inContext->flashContentInRam.mxosSystemConfig.key, json_object_get_string(val), maxKeyLen);
          strncpy(inContext->flashContentInRam.mxosSystemConfig.user_key, json_object_get_string(val), maxKeyLen);
          inContext->flashContentInRam.mxosSystemConfig.keyLength = strlen(inContext->flashContentInRam.mxosSystemConfig.key);
          inContext->flashContentInRam.mxosSystemConfig.user_keyLength = strlen(inContext->flashContentInRam.mxosSystemConfig.key);
          need_reboot = true;
        }else if(!strcmp(key, "DHCP")){
          inContext->flashContentInRam.mxosSystemConfig.dhcpEnable   = json_object_get_boolean(val);
          need_reboot = true;
        }else if(!strcmp(key, "IP address")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.localIp, json_object_get_string(val), maxIpLen);
          need_reboot = true;
        }else if(!strcmp(key, "Net Mask")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.netMask, json_object_get_string(val), maxIpLen);
          need_reboot = true;
        }else if(!strcmp(key, "Gateway")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.gateWay, json_object_get_string(val), maxIpLen);
          need_reboot = true;
        }else if(!strcmp(key, "DNS Server")){
          strncpy(inContext->flashContentInRam.mxosSystemConfig.dnsServer, json_object_get_string(val), maxIpLen);
          need_reboot = true;
        }else{
          config_server_delegate_recv( key, val, &need_reboot, &inContext->flashContentInRam );
        }
      }
      mos_mutex_unlock(inContext->flashContentInRam_mutex);

      json_object_put(config);

      inContext->flashContentInRam.mxosSystemConfig.configured = allConfigured;
      mxos_system_context_update( &inContext->flashContentInRam );

      if( need_reboot == true ){
        mxos_system_power_perform( &inContext->flashContentInRam, eState_Software_Reset );
      }
    }
    goto exit;
  }
  else if(HTTPHeaderMatchURL( inHeader, kCONFIGURLWriteByUAP ) == kNoErr){
      uint32_t easylinkIndentifier = 0;

      require( inHeader->contentLength > 0, exit );

      system_log( "Recv new configuration from uAP, apply and connect to AP" );

      inContext->flashContentInRam.mxosSystemConfig.easyLinkByPass = EASYLINK_BYPASS_NO;

      config = json_tokener_parse(inHeader->extraDataPtr);
      require_action(config, exit, err = kUnknownErr);
      system_log("Recv config object from uap =%s", json_object_to_json_string(config));
      mos_mutex_lock(inContext->flashContentInRam_mutex);

      json_object_object_foreach( config, key, val ) {
          if ( !strcmp( key, "SSID" ) ) {
              strncpy( inContext->flashContentInRam.mxosSystemConfig.ssid, json_object_get_string( val ), maxSsidLen );
              inContext->flashContentInRam.mxosSystemConfig.channel = 0;
              memset( inContext->flashContentInRam.mxosSystemConfig.bssid, 0x0, 6 );
              inContext->flashContentInRam.mxosSystemConfig.security = SECURITY_TYPE_AUTO;
              memcpy( inContext->flashContentInRam.mxosSystemConfig.key,
                      inContext->flashContentInRam.mxosSystemConfig.user_key, maxKeyLen );
              inContext->flashContentInRam.mxosSystemConfig.keyLength = inContext->flashContentInRam.mxosSystemConfig.user_keyLength;
          }
          else if ( !strcmp( key, "PASSWORD" ) ) {
              inContext->flashContentInRam.mxosSystemConfig.security = SECURITY_TYPE_AUTO;
              strncpy( inContext->flashContentInRam.mxosSystemConfig.key, json_object_get_string( val ), maxKeyLen );
              strncpy( inContext->flashContentInRam.mxosSystemConfig.user_key, json_object_get_string( val ), maxKeyLen );
              inContext->flashContentInRam.mxosSystemConfig.keyLength = strlen( inContext->flashContentInRam.mxosSystemConfig.key );
              inContext->flashContentInRam.mxosSystemConfig.user_keyLength = strlen( inContext->flashContentInRam.mxosSystemConfig.key );
              memcpy( inContext->flashContentInRam.mxosSystemConfig.key,
                      inContext->flashContentInRam.mxosSystemConfig.user_key, maxKeyLen );
              inContext->flashContentInRam.mxosSystemConfig.keyLength = inContext->flashContentInRam.mxosSystemConfig.user_keyLength;
          }
          else if ( !strcmp( key, "DHCP" ) ) {
              inContext->flashContentInRam.mxosSystemConfig.dhcpEnable = json_object_get_boolean( val );
          }
          else if ( !strcmp( key, "IDENTIFIER" ) ) {
              easylinkIndentifier = (uint32_t) json_object_get_int( val );
          }
          else if ( !strcmp( key, "IP" ) ) {
              strncpy( inContext->flashContentInRam.mxosSystemConfig.localIp, json_object_get_string( val ), maxIpLen );
          }
          else if ( !strcmp( key, "NETMASK" ) ) {
              strncpy( inContext->flashContentInRam.mxosSystemConfig.netMask, json_object_get_string( val ), maxIpLen );
          }
          else if ( !strcmp( key, "GATEWAY" ) ) {
              strncpy( inContext->flashContentInRam.mxosSystemConfig.gateWay, json_object_get_string( val ), maxIpLen );
          }
          else if ( !strcmp( key, "DNS1" ) ) {
              strncpy( inContext->flashContentInRam.mxosSystemConfig.dnsServer, json_object_get_string( val ), maxIpLen );
          }
      }
      mos_mutex_unlock(inContext->flashContentInRam_mutex);
      json_object_put( config );

      err = CreateSimpleHTTPOKMessage( &httpResponse, &httpResponseLen );
      require_noerr( err, exit );

      err = SocketSend( fd, httpResponse, httpResponseLen );
      require_noerr( err, exit );

      if ( _uap_configured_cb ) {
          mos_thread_delay( 1000 );
          _uap_configured_cb( easylinkIndentifier );
      }
  }
  else if(HTTPHeaderMatchURL( inHeader, kCONFIGURLOTA ) == kNoErr && ota_partition->partition_owner != MXOS_FLASH_NONE){
    if(inHeader->contentLength > 0){
      system_log("Receive OTA data!");
      CRC16_Final( &http_context->crc16_contex, &crc);
      memset(&inContext->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
#ifdef CONFIG_MX108
      inContext->flashContentInRam.bootTable.dst_adr = 0x13200;
      inContext->flashContentInRam.bootTable.src_adr = ota_partition->partition_start_addr;
      inContext->flashContentInRam.bootTable.siz = inHeader->contentLength;
      inContext->flashContentInRam.bootTable.crc = crc;
#else
      inContext->flashContentInRam.bootTable.length = inHeader->contentLength;
      inContext->flashContentInRam.bootTable.start_address = ota_partition->partition_start_addr;
      inContext->flashContentInRam.bootTable.type = 'A';
      inContext->flashContentInRam.bootTable.upgrade_type = 'U';
      inContext->flashContentInRam.bootTable.crc = crc;
#endif
      mxos_system_context_update( &inContext->flashContentInRam );
      SocketClose( &fd );
      mxos_system_power_perform( &inContext->flashContentInRam, eState_Software_Reset );
      mxos_thread_sleep( MXOS_WAIT_FOREVER );
    }
    goto exit;
  }
  else{
    return kNotFoundErr;
  };

 exit:
  if(inHeader->persistent == false)  //Return an err to close socket and exit the current thread
    err = kConnectionErr;
  if(httpResponse)  free(httpResponse);
  if(report)        json_object_put(report);
  if(config)        json_object_put(config);

  return err;

}
#endif
