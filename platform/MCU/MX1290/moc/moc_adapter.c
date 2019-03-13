#include "mxos_debug.h"
#include "mxos_common.h"
#include "moc_api.h"
#include "moc_api_sep.h"

static kernel_api_t _kernel_api;

static mxos_system_config_t* _system_config_get( void )
{
  static mxos_system_config_t cfg;
 // _kernel_api.os_apis->system_config(0, &cfg);
  printf("config get\r\n");
  return &cfg;
}

static void _system_config_set( mxos_system_config_t *cfg )
{
  //_kernel_api.os_apis->system_config(1, cfg);
  printf("config set");
}

#ifndef MXOS_DISABLE_STDIO

#ifndef STDIO_BUFFER_SIZE
#define STDIO_BUFFER_SIZE   64
#endif

static const platform_uart_config_t stdio_uart_config =
{
    .baud_rate = MXOS_STDIO_UART_BAUDRATE,
    .data_width = DATA_WIDTH_8BIT,
    .parity = NO_PARITY,
    .stop_bits = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
    .flags = 0,
};

static volatile ring_buffer_t stdio_rx_buffer;
static volatile uint8_t stdio_rx_data[STDIO_BUFFER_SIZE];

void init_debug_uart(void)
{
	ring_buffer_init( (ring_buffer_t*) &stdio_rx_buffer, (uint8_t*) stdio_rx_data, STDIO_BUFFER_SIZE );
	mxos_stdio_uart_init( &stdio_uart_config, (ring_buffer_t*) &stdio_rx_buffer );
}
#else
void init_debug_uart(void)
{
    return;
}
#endif

static void _mos_thread_yield(void)
{
   mos_thread_delay( 0 );
}


mxos_api_t *moc_adapter(new_mxos_api_t *new_mxos_api)
{
  static mxos_api_t mxos_api;
  new_mxos_api->mxos_api_get(API_VERSION_V1, &_kernel_api);
  
  /* automatically generated by python script */
  mxos_api.library_version = new_mxos_api->library_version;
  
  mxos_api.system_config_get = _system_config_get;
  mxos_api.system_config_set = _system_config_set;
  mxos_api.mxos_network_init = (void(*)())_kernel_api.os_apis->mxos_network_init;

  mxos_api.mos_thread_new = (int (*)(void **, uint8_t, char const *, void (*)(void *), uint32_t, void *))_kernel_api.os_apis->mos_thread_new;
  mxos_api.mos_thread_delete = _kernel_api.os_apis->mos_thread_delete;
  mxos_api.mos_thread_delete = _kernel_api.os_apis->mos_thread_delete;
  mxos_api.mos_thread_yield = _mos_thread_yield;
  mxos_api.mos_thread_suspend = _kernel_api.os_apis->mos_thread_suspend;
  mxos_api.mxos_rtos_suspend_all_thread = _kernel_api.os_apis->mxos_rtos_suspend_all_thread;
  mxos_api.mxos_rtos_resume_all_thread = (long(*)(void))_kernel_api.os_apis->mxos_rtos_resume_all_thread;
  mxos_api.mos_thread_join = _kernel_api.os_apis->mos_thread_join;
  mxos_api.mxos_rtos_thread_force_awake = _kernel_api.os_apis->mxos_rtos_thread_force_awake;
  mxos_api.mxos_rtos_is_current_thread = _kernel_api.os_apis->mxos_rtos_is_current_thread;
  mxos_api.mxos_thread_sleep = _kernel_api.os_apis->mxos_thread_sleep;
  mxos_api.mxos_thread_msleep = _kernel_api.os_apis->mxos_thread_msleep;
  mxos_api.mos_semphr_new = _kernel_api.os_apis->mos_semphr_new;
  mxos_api.mos_semphr_release = _kernel_api.os_apis->mos_semphr_release;
  mxos_api.mos_semphr_acquire = _kernel_api.os_apis->mos_semphr_acquire;
  mxos_api.mos_semphr_delete = _kernel_api.os_apis->mos_semphr_delete;
  mxos_api.mxos_rtos_init_mutex = _kernel_api.os_apis->mxos_rtos_init_mutex;
  mxos_api.mxos_rtos_lock_mutex = _kernel_api.os_apis->mxos_rtos_lock_mutex;
  mxos_api.mxos_rtos_unlock_mutex = _kernel_api.os_apis->mxos_rtos_unlock_mutex;
  mxos_api.mxos_rtos_deinit_mutex = _kernel_api.os_apis->mxos_rtos_deinit_mutex;
  mxos_api.mxos_rtos_init_queue = _kernel_api.os_apis->mxos_rtos_init_queue;
  mxos_api.mxos_rtos_push_to_queue = _kernel_api.os_apis->mxos_rtos_push_to_queue;
  mxos_api.mxos_rtos_pop_from_queue = _kernel_api.os_apis->mxos_rtos_pop_from_queue;
  mxos_api.mxos_rtos_deinit_queue = _kernel_api.os_apis->mxos_rtos_deinit_queue;
  mxos_api.mxos_rtos_is_queue_empty = _kernel_api.os_apis->mxos_rtos_is_queue_empty;
  mxos_api.mxos_rtos_is_queue_full = (int (*)(void * *))_kernel_api.os_apis->mxos_rtos_is_queue_full;
  mxos_api.mxos_get_time = _kernel_api.os_apis->mxos_get_time;
  mxos_api.mxos_init_timer = _kernel_api.os_apis->mxos_init_timer;
  mxos_api.mxos_start_timer = _kernel_api.os_apis->mxos_start_timer;
  mxos_api.mxos_stop_timer = _kernel_api.os_apis->mxos_stop_timer;
  mxos_api.mxos_reload_timer = _kernel_api.os_apis->mxos_reload_timer;
  mxos_api.mxos_deinit_timer = _kernel_api.os_apis->mxos_deinit_timer;
  mxos_api.mxos_is_timer_running = _kernel_api.os_apis->mxos_is_timer_running;
  mxos_api.mxos_create_event_fd = _kernel_api.os_apis->mxos_create_event_fd;
  mxos_api.mxos_delete_event_fd = _kernel_api.os_apis->mxos_delete_event_fd;
  mxos_api.SetTimer = NULL;
  mxos_api.SetTimer_uniq = NULL;
  mxos_api.UnSetTimer = NULL;
  mxos_api.mxos_memory_info = (mxosMemInfo_t* (*)( void ))_kernel_api.os_apis->mxos_memory_info;
  mxos_api.malloc = _kernel_api.os_apis->malloc;
  mxos_api.realloc = _kernel_api.os_apis->realloc;
  mxos_api.free = _kernel_api.os_apis->free;
  mxos_api.calloc = (void *(*)(int, int))_kernel_api.os_apis->calloc;
  mxos_api.heap_insert = _kernel_api.os_apis->heap_insert;
  
  mxos_api.socket = NULL;
  mxos_api.setsockopt = NULL;
  mxos_api.getsockopt = NULL;
  mxos_api.bind = NULL;
  mxos_api.connect = NULL;
  mxos_api.listen = NULL;
  mxos_api.accept = NULL;
  mxos_api.select = NULL;
  mxos_api.send = NULL;
  mxos_api.write = NULL;
  mxos_api.sendto = NULL;
  mxos_api.recv = NULL;
  mxos_api.read = NULL;
  mxos_api.recvfrom = NULL;
  mxos_api.close = NULL;
  mxos_api.inet_addr = NULL;
  mxos_api.inet_ntoa = NULL;
  mxos_api.gethostbyname = NULL;
  mxos_api.set_tcp_keepalive = NULL;
  mxos_api.get_tcp_keepalive = NULL;
  
  mxos_api.lwip_apis = _kernel_api.lwip_apis;
  
  mxos_api.ssl_set_cert = _kernel_api.ssl_crypto_apis->ssl_set_cert;
  mxos_api.ssl_connect = _kernel_api.ssl_crypto_apis->ssl_connect;
  mxos_api.ssl_accept = _kernel_api.ssl_crypto_apis->ssl_accept;
  mxos_api.ssl_send = _kernel_api.ssl_crypto_apis->ssl_send;
  mxos_api.ssl_recv = _kernel_api.ssl_crypto_apis->ssl_recv;
  mxos_api.ssl_close = _kernel_api.ssl_crypto_apis->ssl_close;
  mxos_api.set_ssl_client_version = _kernel_api.ssl_crypto_apis->set_ssl_client_version;
  mxos_api.ssl_nonblock_connect = _kernel_api.ssl_crypto_apis->ssl_nonblock_connect;
  mxos_api.ssl_set_using_nonblock = _kernel_api.ssl_crypto_apis->ssl_set_using_nonblock;
  mxos_api.ssl_pending = _kernel_api.ssl_crypto_apis->ssl_pending;
  mxos_api.ssl_get_error = _kernel_api.ssl_crypto_apis->ssl_get_error;
  mxos_api.ssl_set_client_cert = _kernel_api.ssl_crypto_apis->ssl_set_client_cert;
  mxos_api.ssl_connect_sni = _kernel_api.ssl_crypto_apis->ssl_connect_sni;
  
  mxos_api.InitMd5 = (void (*)(md5_context*))_kernel_api.ssl_crypto_apis->InitMd5;
  mxos_api.Md5Update = (void (*)(md5_context*,unsigned char*, int))_kernel_api.ssl_crypto_apis->Md5Update;
  mxos_api.Md5Final = (void (*)(md5_context*,uint8_t*))_kernel_api.ssl_crypto_apis->Md5Final;
  mxos_api.Md5Hash = _kernel_api.ssl_crypto_apis->Md5Hash;
  mxos_api.AesEncryptDirect = _kernel_api.ssl_crypto_apis->AesEncryptDirect;
  mxos_api.AesDecryptDirect = _kernel_api.ssl_crypto_apis->AesDecryptDirect;
  mxos_api.AesSetKeyDirect = _kernel_api.ssl_crypto_apis->AesSetKeyDirect;
  mxos_api.aes_encrypt = _kernel_api.ssl_crypto_apis->aes_encrypt;
  mxos_api.aes_decrypt = _kernel_api.ssl_crypto_apis->aes_decrypt;
  mxos_api.AesSetKey = _kernel_api.ssl_crypto_apis->AesSetKey;
  mxos_api.AesSetIV = _kernel_api.ssl_crypto_apis->AesSetIV;
  mxos_api.AesCbcEncrypt = _kernel_api.ssl_crypto_apis->AesCbcEncrypt;
  mxos_api.AesCbcDecrypt = _kernel_api.ssl_crypto_apis->AesCbcDecrypt;

  mxos_api.wlan_get_mac_address = _kernel_api.wifi_apis->wlan_get_mac_address;
  mxos_api.wlan_get_mac_address_by_interface = _kernel_api.wifi_apis->wlan_get_mac_address_by_interface;
  mxos_api.mxos_wlan_get_channel = _kernel_api.wifi_apis->mxos_wlan_get_channel;
  mxos_api.mxos_wlan_driver_version = _kernel_api.wifi_apis->mxos_wlan_driver_version;
  mxos_api.mxosWlanStart = _kernel_api.wifi_apis->mxosWlanStart;
  mxos_api.mxosWlanStartAdv = _kernel_api.wifi_apis->mxosWlanStartAdv;
  mxos_api.mxosWlanGetIPStatus = _kernel_api.wifi_apis->mxosWlanGetIPStatus;
  mxos_api.mxosWlanGetLinkStatus = _kernel_api.wifi_apis->mxosWlanGetLinkStatus;
  mxos_api.mxosWlanStartScan = (int(*)(void))_kernel_api.wifi_apis->mxosWlanStartScan;
  mxos_api.mxosWlanStartScanAdv = (int(*)(void))_kernel_api.wifi_apis->mxosWlanStartScanAdv;
  mxos_api.mxosWlanPowerOff = _kernel_api.wifi_apis->mxosWlanPowerOff;
  mxos_api.mxosWlanPowerOn = _kernel_api.wifi_apis->mxosWlanPowerOn;
  mxos_api.mxosWlanSuspend = _kernel_api.wifi_apis->mxosWlanSuspend;
  mxos_api.mxosWlanSuspendStation = _kernel_api.wifi_apis->mxosWlanSuspendStation;
  mxos_api.mxosWlanSuspendSoftAP = _kernel_api.wifi_apis->mxosWlanSuspendSoftAP;
  mxos_api.mxosWlanStartEasyLink = _kernel_api.wifi_apis->mxosWlanStartEasyLink;
  mxos_api.mxosWlanStartEasyLinkPlus = _kernel_api.wifi_apis->mxosWlanStartEasyLink;
  mxos_api.mxosWlanStopEasyLink = _kernel_api.wifi_apis->mxosWlanStopEasyLink;
  mxos_api.mxosWlanStopEasyLinkPlus = _kernel_api.wifi_apis->mxosWlanStopEasyLink;
  mxos_api.mxosWlanStartWPS = NULL;
  mxos_api.mxosWlanStopWPS = NULL;
  mxos_api.mxosWlanStartAirkiss = _kernel_api.wifi_apis->mxosWlanStartEasyLink;
  mxos_api.mxosWlanStopAirkiss = _kernel_api.wifi_apis->mxosWlanStopEasyLink;
  mxos_api.mxosWlanEnablePowerSave = _kernel_api.wifi_apis->mxosWlanEnablePowerSave;
  mxos_api.mxosWlanDisablePowerSave = _kernel_api.wifi_apis->mxosWlanDisablePowerSave;
  mxos_api.wifimgr_debug_enable = _kernel_api.wifi_apis->wifimgr_debug_enable;
  mxos_api.mxos_wlan_monitor_rx_type = _kernel_api.wifi_apis->mxos_wlan_monitor_rx_type;
  mxos_api.mxos_wlan_start_monitor = _kernel_api.wifi_apis->mxos_wlan_start_monitor;
  mxos_api.mxos_wlan_stop_monitor = _kernel_api.wifi_apis->mxos_wlan_stop_monitor;
  mxos_api.mxos_wlan_monitor_set_channel = _kernel_api.wifi_apis->mxos_wlan_monitor_set_channel;
  mxos_api.mxos_wlan_register_monitor_cb = _kernel_api.wifi_apis->mxos_wlan_register_monitor_cb;
  mxos_api.wlan_set_channel = _kernel_api.wifi_apis->wlan_set_channel;
  mxos_api.mxchip_active_scan = _kernel_api.wifi_apis->mxchip_active_scan;
  mxos_api.wifi_manage_custom_ie_add = _kernel_api.wifi_apis->wifi_manage_custom_ie_add;
  mxos_api.wifi_manage_custom_ie_delete = _kernel_api.wifi_apis->wifi_manage_custom_ie_delete;
  
  mxos_api.cli_init = _kernel_api.cli_apis->cli_init;
  mxos_api.cli_register_command = _kernel_api.cli_apis->cli_register_command;
  mxos_api.cli_unregister_command = _kernel_api.cli_apis->cli_unregister_command;
  mxos_api.wifistate_Command = _kernel_api.cli_apis->wifistate_Command;
  mxos_api.wifidebug_Command = _kernel_api.cli_apis->wifidebug_Command;
  mxos_api.wifiscan_Command = _kernel_api.cli_apis->wifiscan_Command;
  mxos_api.ifconfig_Command = _kernel_api.cli_apis->ifconfig_Command;
  mxos_api.arp_Command = _kernel_api.cli_apis->arp_Command;
  mxos_api.ping_Command = _kernel_api.cli_apis->ping_Command;
  mxos_api.dns_Command = _kernel_api.cli_apis->dns_Command;
  mxos_api.task_Command = _kernel_api.cli_apis->task_Command;
  mxos_api.socket_show_Command = _kernel_api.cli_apis->socket_show_Command;
  mxos_api.memory_show_Command = _kernel_api.cli_apis->memory_show_Command;
  mxos_api.memory_dump_Command = _kernel_api.cli_apis->memory_dump_Command;
  mxos_api.memory_set_Command = _kernel_api.cli_apis->memory_set_Command;
  mxos_api.memp_dump_Command = _kernel_api.cli_apis->memp_dump_Command;
  mxos_api.driver_state_Command = _kernel_api.cli_apis->driver_state_Command;
  mxos_api.iperf_Command = _kernel_api.cli_apis->iperf_Command;
  
  mxos_api.mxos_flash_get_info = _kernel_api.flash_apis->mxos_flash_get_info;
  mxos_api.mxos_flash_erase = _kernel_api.flash_apis->mxos_flash_erase;
  mxos_api.mxos_flash_write = _kernel_api.flash_apis->mxos_flash_write;
  mxos_api.mxos_flash_read = _kernel_api.flash_apis->mxos_flash_read;
  mxos_api.mxos_flash_enable_security = _kernel_api.flash_apis->mxos_flash_enable_security;
  mxos_api.mxos_gpio_init = _kernel_api.gpio_apis->mxos_gpio_init;
  mxos_api.mxos_gpio_deinit = _kernel_api.gpio_apis->mxos_gpio_deinit;
  mxos_api.mxos_gpio_output_high = _kernel_api.gpio_apis->mxos_gpio_output_high;
  mxos_api.mxos_gpio_output_low = _kernel_api.gpio_apis->mxos_gpio_output_low;
  mxos_api.mxos_gpio_output_toggle = _kernel_api.gpio_apis->mxos_gpio_output_toggle;
  mxos_api.mxos_gpio_input_get = _kernel_api.gpio_apis->mxos_gpio_input_get;
  mxos_api.mxos_gpio_enable_irq = _kernel_api.gpio_apis->mxos_gpio_enable_irq;
  mxos_api.mxos_gpio_disable_irq = _kernel_api.gpio_apis->mxos_gpio_disable_irq;
  mxos_api.mxos_uart_init = _kernel_api.uart_apis->mxos_uart_init;
  mxos_api.mxos_uart_deinit = _kernel_api.uart_apis->mxos_uart_deinit;
  mxos_api.mxos_uart_send = _kernel_api.uart_apis->mxos_uart_send;
  mxos_api.mxos_uart_recv = _kernel_api.uart_apis->mxos_uart_recv;
  mxos_api.mxos_uart_recvd_data_len = _kernel_api.uart_apis->mxos_uart_recvd_data_len;
  mxos_api.MxosUartPinRedirect = _kernel_api.uart_apis->MxosUartPinRedirect;
  
  mxos_api.pm_mcu_state = _kernel_api.ps_apis->pm_mcu_state;
  mxos_api.pm_wakeup_source = _kernel_api.ps_apis->pm_wakeup_source;
  mxos_api.wifi_off_mcu_standby = (void(*)(int))(_kernel_api.ps_apis->wifi_off_mcu_standby);
  mxos_api.mxos_mcu_powersave_config = _kernel_api.ps_apis->mxos_mcu_powersave_config;
  mxos_api.debug_putchar = _kernel_api.os_apis->debug_putchar;
  mxos_api.mxos_sys_reboot = _kernel_api.os_apis->mxos_sys_reboot;
  mxos_api.get_ali_key = NULL;
  mxos_api.get_ali_secret = NULL;
  mxos_api.mxos_rtc_init = _kernel_api.rtc_apis->mxos_rtc_init;
  mxos_api.mxos_rtc_get_time = _kernel_api.rtc_apis->mxos_rtc_get_time;
  mxos_api.mxos_rtc_set_time = _kernel_api.rtc_apis->mxos_rtc_set_time;
  mxos_api.localtime = _kernel_api.os_apis->localtime;
  mxos_api.asctime = _kernel_api.os_apis->asctime;
  mxos_api.wifi_set_country = _kernel_api.wifi_apis->wifi_set_country;
  mxos_api.switch_active_firmrware = NULL;
  mxos_api.last_reset_reason = _kernel_api.os_apis->last_reset_reason;
  mxos_api.aon_write = _kernel_api.os_apis->aon_write;
  mxos_api.aon_read = _kernel_api.os_apis->aon_read;
  mxos_api.ssl_get_fd = _kernel_api.ssl_crypto_apis->ssl_get_fd;
  mxos_api.get_random_sequence = _kernel_api.os_apis->get_random_sequence;
  mxos_api.ssl_set_loggingcb = _kernel_api.ssl_crypto_apis->ssl_loggingcb;
  mxos_api.wlan_inject_frame = _kernel_api.wifi_apis->wlan_inject_frame;
  mxos_api.wlan_rx_mgmt_indication = NULL;
  mxos_api.wlan_remain_on_channel = NULL;
  mxos_api.wifi_bridge_mode_enable = NULL;
  mxos_api.wifi_bridge_mode_disable = NULL;
  mxos_api.send_easylink_minus = _kernel_api.wifi_apis->send_easylink_minus;
  mxos_api.ssl_socket = (int(*)(void*))(_kernel_api.ssl_crypto_apis->ssl_get_fd);

  mxos_api.i2c_apis = _kernel_api.i2c_apis;
  mxos_api.spi_apis = _kernel_api.spi_apis;
  mxos_api.pwm_apis = _kernel_api.pwm_apis;
  mxos_api.wdg_apis = _kernel_api.wdg_apis;
  mxos_api.adc_apis = _kernel_api.adc_apis;
  mxos_api.gtimer_apis = _kernel_api.gtimer_apis;

  return &mxos_api;
}

int debug_putchar(char *ch, int len)
{
	return _kernel_api.os_apis->debug_putchar(ch, len);
}

int debug_gettchar(char *ch)
{
	return _kernel_api.os_apis->debug_getchar(ch);
}

int mxos_wlan_monitor_no_easylink(void)
{
	return _kernel_api.wifi_apis->mxos_wlan_monitor_no_easylink();
}

int mxos_wlan_monitor_with_easylink(void)
{
	return _kernel_api.wifi_apis->mxos_wlan_monitor_with_easylink();
}

int wlan_rx_mgnt_set(int enable, mgnt_handler_t cb)
{
	return _kernel_api.wifi_apis->wlan_rx_mgnt_set(enable, cb);
}

void autoconfig_start(int seconds, int mode)
{
	_kernel_api.wifi_apis->autoconfig_start(seconds, mode);
}

void wlan_set_softap_tdma(int value)
{
	_kernel_api.wifi_apis->wlan_set_softap_tdma(value);
}

int wifi_off_fastly(void)
{
    return _kernel_api.wifi_apis->wifi_off_fastly();
}

int mxos_wlan_easylink_uap_start(int timeout, char *ssid, char*key, int channel)
{
    return _kernel_api.wifi_apis->OpenEasylink_softap(timeout, ssid, key, channel);
}

void ssl_set_ecc(int enable)
{
    _kernel_api.ssl_crypto_apis->ssl_set_ecc(enable);
}

/* return 1=success; 0=fail*/
int disable_log_uart(void)
{
    return _kernel_api.uart_apis->disable_log_uart();
}

void mos_thread_resume(mos_thread_id_t thread)
{
    _kernel_api.os_apis->mos_thread_resume(&thread);
}

#define extra_apis _kernel_api.ssl_crypto_apis->extra_crypto_apis

#define EXTRA_CRYPTO_CHECK() if (EXTRA_CRYPTO_FLAG != _kernel_api.ssl_crypto_apis->extra_crypto_flag) return -1;
        
int  InitRng(CyaSSL_RNG* rng)
{
    EXTRA_CRYPTO_CHECK();
    return extra_apis->InitRng(rng);
}
int  RNG_GenerateBlock(CyaSSL_RNG* rng, byte* b, word32 sz)
{
    EXTRA_CRYPTO_CHECK();
    return extra_apis->RNG_GenerateBlock(rng, b, sz);
}
int  RNG_GenerateByte(CyaSSL_RNG* rng, byte* b)
{
    EXTRA_CRYPTO_CHECK();
    return extra_apis->RNG_GenerateByte(rng, b);
}
int FreeRng(CyaSSL_RNG* rng)
{
    EXTRA_CRYPTO_CHECK();
    return extra_apis->FreeRng(rng);
}


int  RsaPublicKeyDecode(const byte* input, word32* inOutIdx, RsaKey* key, word32 inSz)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->RsaPublicKeyDecode(input, inOutIdx, key, inSz);
}
int  InitRsaKey(RsaKey* key, void* ptr)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->InitRsaKey(key, ptr);
}
int  FreeRsaKey(RsaKey* key)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->FreeRsaKey(key);
}
int  RsaPublicEncrypt(const byte* in, word32 inLen, byte* out,
                         word32 outLen, RsaKey* key, CyaSSL_RNG* rng)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->RsaPublicEncrypt(in, inLen, out,
                         outLen, key, rng);
}
int  RsaSSL_Verify(const byte* in, word32 inLen, byte* out,
                      word32 outLen, RsaKey* key)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->RsaSSL_Verify(in, inLen, out, outLen, key);
}
int  RsaEncryptSize(RsaKey* key)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->RsaEncryptSize(key);
}

int InitSha256(Sha256* sha)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->InitSha256(sha);
}
int Sha256Update(Sha256* sha, const byte* data, word32 len)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->Sha256Update(sha, data, len);
}
int Sha256Final(Sha256* sha, byte* out)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->Sha256Final(sha, out);
}

int InitSha(Sha* sha)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->InitSha(sha);
}
int ShaUpdate(Sha* sha, const byte* data, word32 len)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->ShaUpdate(sha, data, len);
}
int ShaFinal(Sha* sha, byte* out)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->ShaFinal(sha, out);
}

int HmacSetKey(Hmac* hmac, int type, const byte* key, word32 keySz)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->HmacSetKey(hmac, type, key, keySz);
}
int HmacUpdate(Hmac* hmac, const byte* in, word32 sz)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->HmacUpdate(hmac, in, sz);
}
int HmacFinal(Hmac* hmac, byte* out)
{

    EXTRA_CRYPTO_CHECK();
    return extra_apis->HmacFinal(hmac, out);
}

void* ssl_connect_dtls(int fd, int calen, char*ca, int *errno)
{
    return _kernel_api.ssl_crypto_apis->ssl_connect_dtls(fd, calen, ca, errno);
}

void ssl_set_alpn_list(char*list)
{
    _kernel_api.ssl_crypto_apis->ssl_set_alpn_list(list);
}

int hardfault_get(char *msg, int len)
{
    return _kernel_api.os_apis->hardfault_get(msg, len);
}

int mxos_change_timer_period( mxos_timer_t* timer, uint32_t new_period )
{
    return _kernel_api.os_apis->mxos_change_timer_period(timer, new_period );
}

int mxos_init_once_timer( mxos_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg )
{
    return _kernel_api.os_apis->mxos_init_once_timer( timer, time_ms, function, arg );
}

merr_t MxosRtcSetalarm(time_t *time, rtc_irq_handler handler)
{
    return _kernel_api.rtc_apis->MxosRtcSetalarm( time, handler );
}

