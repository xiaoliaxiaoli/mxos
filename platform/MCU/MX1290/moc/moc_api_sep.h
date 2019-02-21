#include "mxos_security.h"

enum {
	API_VERSION_V1 = 1,
	API_VERSION_MAX,
};

typedef void* mxos_event;
typedef void (*ssl_log_cb)(const int logLevel, const char *const logMessage);

#define EXTRA_CRYPTO_FLAG 0xEC000001

typedef struct {
    int  (*InitRng)(CyaSSL_RNG*);
    int  (*RNG_GenerateBlock)(CyaSSL_RNG*, byte*, word32 sz);
    int  (*RNG_GenerateByte)(CyaSSL_RNG*, byte*);
    int  (*FreeRng)(CyaSSL_RNG*);

    int  (*RsaPublicKeyDecode)(const byte* input, word32* inOutIdx, RsaKey*, word32);
    int  (*InitRsaKey)(RsaKey* key, void*);
    int  (*FreeRsaKey)(RsaKey* key);
    int  (*RsaPublicEncrypt)(const byte* in, word32 inLen, byte* out,
                             word32 outLen, RsaKey* key, CyaSSL_RNG* rng);
    int  (*RsaSSL_Verify)(const byte* in, word32 inLen, byte* out,
                          word32 outLen, RsaKey* key);
    int  (*RsaEncryptSize)(RsaKey* key);

    int (*InitSha256)(Sha256*);
    int (*Sha256Update)(Sha256*, const byte*, word32);
    int (*Sha256Final)(Sha256*, byte*);

    int (*InitSha)(Sha*);
    int (*ShaUpdate)(Sha*, const byte*, word32);
    int (*ShaFinal)(Sha*, byte*);

    int (*HmacSetKey)(Hmac*, int type, const byte* key, word32 keySz);
    int (*HmacUpdate)(Hmac*, const byte*, word32);
    int (*HmacFinal)(Hmac*, byte*);

}extra_crypto_api_t;

typedef void (*mgnt_handler_t)(char *buf, int buf_len);

typedef struct {
	/* OS Layer*/
	int (*system_config)(int type, void *value);/* system configuration */
	int (*mxchipInit)();
	OSStatus (*mxos_rtos_create_thread)( mxos_thread_t* thread, uint8_t priority, const char* name, mxos_thread_function_t function, uint32_t stack_size, uint32_t arg );
	OSStatus (*mxos_rtos_delete_thread)( mxos_thread_t* thread );
	void (*mxos_rtos_suspend_thread)(mxos_thread_t* thread);
	void (*mxos_rtos_suspend_all_thread)(void);
	long (*mxos_rtos_resume_all_thread)(void);
	OSStatus (*mxos_rtos_thread_join)( mxos_thread_t* thread );
	OSStatus (*mxos_rtos_thread_force_awake)( mxos_thread_t* thread );
	bool (*mxos_rtos_is_current_thread)( mxos_thread_t* thread );
	void (*mxos_thread_sleep)(uint32_t seconds);
	void (*mxos_thread_msleep)(uint32_t milliseconds);
	OSStatus (*mxos_rtos_init_semaphore)( mxos_semaphore_t* semaphore, int count );
	OSStatus (*mxos_rtos_set_semaphore)( mxos_semaphore_t* semaphore );
	OSStatus (*mxos_rtos_get_semaphore)( mxos_semaphore_t* semaphore, uint32_t timeout_ms );
	OSStatus (*mxos_rtos_deinit_semaphore)( mxos_semaphore_t* semaphore );
	OSStatus (*mxos_rtos_init_mutex)( mxos_mutex_t* mutex );
	OSStatus (*mxos_rtos_lock_mutex)( mxos_mutex_t* mutex );
	OSStatus (*mxos_rtos_unlock_mutex)( mxos_mutex_t* mutex );
	OSStatus (*mxos_rtos_deinit_mutex)( mxos_mutex_t* mutex );
	OSStatus (*mxos_rtos_init_queue)( mxos_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages );
	OSStatus (*mxos_rtos_push_to_queue)( mxos_queue_t* queue, void* message, uint32_t timeout_ms );
	OSStatus (*mxos_rtos_pop_from_queue)( mxos_queue_t* queue, void* message, uint32_t timeout_ms );
	OSStatus (*mxos_rtos_deinit_queue)( mxos_queue_t* queue );
	bool (*mxos_rtos_is_queue_empty)( mxos_queue_t* queue );
	bool (*mxos_rtos_is_queue_full)( mxos_queue_t* queue );
	uint32_t (*mxos_get_time)(void);
	OSStatus (*mxos_init_timer)( mxos_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg );
	OSStatus (*mxos_start_timer)( mxos_timer_t* timer );
	OSStatus (*mxos_stop_timer)( mxos_timer_t* timer );
	OSStatus (*mxos_reload_timer)( mxos_timer_t* timer );
	OSStatus (*mxos_deinit_timer)( mxos_timer_t* timer );
	bool (*mxos_is_timer_running)( mxos_timer_t* timer );
	int (*mxos_create_event_fd)(mxos_event handle);
	int (*mxos_delete_event_fd)(int fd);

	/* memory management*/
	struct mxchip_mallinfo* (*mxos_memory_info)(void);
	void* (*malloc)(size_t size); // malloc
	void* (*realloc)(void* pv, size_t size); // realloc
	void (*free)(void* pv);     //free
	void* (*calloc)(size_t nmemb, size_t size);     // calloc
	void (*heap_insert)(uint8_t *pv, int len);

	void (*get_random_sequence)(unsigned char *buf, unsigned int size);
	int (*last_reset_reason)(void);
	int (*aon_write)( uint32_t offset, uint8_t* in ,uint32_t len);
	int (*aon_read )( uint32_t offset, uint8_t* out, uint32_t len);

	/* uitls */
	int (*debug_putchar)(char *ch, int len);
	int (*debug_getchar)(char *ch);
	void (*MxosSystemReboot)( void );

	struct tm* (*localtime)(const time_t * time);
	char * (*asctime)(const struct tm *tm);

    void (*mxos_rtos_resume_thread)(mxos_thread_t* thread);
    int (*hardfault_get)(char *msg, int len);
    int (*mxos_init_once_timer)( mxos_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg );
    int (*mxos_change_timer_period)( mxos_timer_t* timer, uint32_t new_period );
} os_api_v1_t;

typedef struct {
	/* SSL */
	void (*ssl_set_cert)(const char *_cert_pem, const char *private_key_pem);
	void* (*ssl_connect)(int fd, int calen, char*ca, int *errno); 
	void* (*ssl_accept)(int fd); 
	int (*ssl_send)(void* ssl, char *data, int len);
	int (*ssl_recv)(void* ssl, char *data, int len);
	int (*ssl_close)(void* ssl);
	void (*set_ssl_client_version)(int version);

	int (*ssl_pending)(void* ssl);
	int (*ssl_get_error)(void* ssl, int ret);
	void (*ssl_set_using_nonblock)(void* ssl, int nonblock);
	int (*ssl_get_fd)(const void* ssl);
	int (*ssl_loggingcb)(ssl_log_cb f);
	
	/*crypto*/
	void (*InitMd5)(md5_context*md5);
	void (*Md5Update)(md5_context* md5, const uint8_t* data, uint32_t len);
	void (*Md5Final)(md5_context* md5, uint8_t* hash);
	int (*Md5Hash)(const uint8_t* data, uint32_t len, uint8_t* hash);
	void (*AesEncryptDirect)(Aes* aes, uint8_t* out, const uint8_t* in);
	void (*AesDecryptDirect)(Aes* aes, uint8_t* out, const uint8_t* in);
	int (*AesSetKeyDirect)(Aes* aes, const uint8_t* key, uint32_t len,
                                const uint8_t* iv, int dir);
	int (*aes_encrypt)(int sz, const char * key, const char * in, char * out);
	int (*aes_decrypt)(int sz, const char * key, const char * in, char * out);
	int  (*AesSetKey)(Aes* aes, const uint8_t* key, uint32_t len,
                              const uint8_t* iv, int dir);
	int  (*AesSetIV)(Aes* aes, const uint8_t* iv);
	int  (*AesCbcEncrypt)(Aes* aes, uint8_t* out,
                                  const uint8_t* in, uint32_t sz);
	int  (*AesCbcDecrypt)(Aes* aes, uint8_t* out,
                                  const uint8_t* in, uint32_t sz);
	void* (*ssl_nonblock_connect)(int fd, int calen, char*ca, int *errno, int timeout);
	void (*ssl_set_client_cert)(const char *_cert_pem, const char *private_key_pem);
	void* (*ssl_connect_sni)(int fd, int calen, char*ca, char *sni_servername, int *errno);
    void (*ssl_set_ecc)(int enable);

    uint32_t extra_crypto_flag;
    extra_crypto_api_t *extra_crypto_apis; 

    void* (*ssl_connect_dtls)(int fd, int calen, char*ca, int *errno);
    void (*ssl_set_alpn_list)(char*list);
} ssl_crypto_api_v1_t;

typedef struct {
	/* WIFI MGR */
	int (*wlan_get_mac_address)(unsigned char *dest);
	int (*wlan_get_mac_address_by_interface)(wlan_if_t wlan_if, unsigned char *dest);
	int (*wlan_driver_version)( char* version, int length );
	OSStatus (*mxosWlanStart)(network_InitTypeDef_st* inNetworkInitPara);
	OSStatus (*mxosWlanStartAdv)(network_InitTypeDef_adv_st* inNetworkInitParaAdv);
	OSStatus (*mxosWlanGetIPStatus)(IPStatusTypedef *outNetpara, WiFi_Interface inInterface);
	OSStatus (*mxosWlanGetLinkStatus)(LinkStatusTypeDef *outStatus);
	void (*mxosWlanStartScan)(void);
	void (*mxosWlanStartScanAdv)(void);
	OSStatus (*mxosWlanPowerOff)(void);
	OSStatus (*mxosWlanPowerOn)(void);
	OSStatus (*mxosWlanSuspend)(void);
	OSStatus (*mxosWlanSuspendStation)(void);
	OSStatus (*mxosWlanSuspendSoftAP)(void);
	OSStatus (*mxosWlanStartEasyLink)(int inTimeout);
	OSStatus (*mxosWlanStopEasyLink)(void);
	void (*mxosWlanEnablePowerSave)(void);
	void (*mxosWlanDisablePowerSave)(void); 
	void (*wifimgr_debug_enable)(bool enable);
	int (*mxos_wlan_monitor_rx_type)(int type);
	int (*mxos_wlan_start_monitor)(void);
	int (*mxos_wlan_stop_monitor)(void);
	int (*mxos_wlan_monitor_set_channel)(int channel);
	void (*mxos_wlan_register_monitor_cb)(monitor_cb_t fn);
	void (*wlan_set_channel)(int channel);
	int (*mxchip_active_scan)(char*ssid, int is_adv);
	int (*send_easylink_minus)(uint32_t ip, char *ssid, char *key)	;
	int (*mxos_wlan_get_channel)(void);
    OSStatus (*wifi_manage_custom_ie_add)(wlan_if_t wlan_if, uint8_t *custom_ie, uint32_t len);
    OSStatus (*wifi_manage_custom_ie_delete)(wlan_if_t wlan_if);
    int (*wlan_inject_frame)(const uint8_t *buff, size_t len);
	int (*mxos_wlan_monitor_no_easylink)(void);
	int (*wifi_set_country)(int country_code);
	int (*wlan_rx_mgnt_set)(int enable, mgnt_handler_t cb);
	void (*autoconfig_start)(int seconds, int mode);
    void (*wlan_set_softap_tdma)(int value);
    int (*wifi_off_fastly)(void);
    int (*OpenEasylink_softap)(int timeout, char *ssid, char*key, int channel);
    int (*mxos_wlan_monitor_with_easylink)(void);
} wifi_api_v1_t;

typedef struct {
	/* CLI APIs */
	int (*cli_init)(void);
	int (*cli_register_command)(const struct cli_command *command);
	int (*cli_unregister_command)(const struct cli_command *command);
	void (*wifistate_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*wifidebug_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*wifiscan_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*ifconfig_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*arp_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*ping_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*dns_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*task_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*socket_show_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*memory_show_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*memory_dump_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*memory_set_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*memp_dump_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*driver_state_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
	void (*iperf_Command)(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
} cli_api_v1_t;


typedef struct {
	mxos_logic_partition_t* (*MxosFlashGetInfo)( mxos_partition_t inPartition );
	OSStatus (*MxosFlashErase)(mxos_partition_t inPartition, uint32_t off_set, uint32_t size);
	OSStatus (*MxosFlashWrite)( mxos_partition_t inPartition, volatile uint32_t* off_set, uint8_t* inBuffer ,uint32_t inBufferLength);
	OSStatus (*MxosFlashRead)( mxos_partition_t inPartition, volatile uint32_t* off_set, uint8_t* outBuffer, uint32_t inBufferLength);
	OSStatus (*MxosFlashEnableSecurity)( mxos_partition_t partition, uint32_t off_set, uint32_t size );
} flash_api_t;

typedef struct {
	OSStatus (*MxosGpioInitialize)( mxos_gpio_t gpio, mxos_gpio_config_t configuration );
	OSStatus (*MxosGpioFinalize)( mxos_gpio_t gpio );
	OSStatus (*MxosGpioOutputHigh)( mxos_gpio_t gpio );
	OSStatus (*MxosGpioOutputLow)( mxos_gpio_t gpio );
	OSStatus (*MxosGpioOutputTrigger)( mxos_gpio_t gpio );
	bool (*MxosGpioInputGet)( mxos_gpio_t gpio );
	OSStatus (*MxosGpioEnableIRQ)( mxos_gpio_t gpio, mxos_gpio_irq_trigger_t trigger, mxos_gpio_irq_handler_t handler, void* arg );
	OSStatus (*MxosGpioDisableIRQ)( mxos_gpio_t gpio );
} gpio_api_t;

typedef struct {
	OSStatus (*MxosUartInitialize)( mxos_uart_t uart, const mxos_uart_config_t* config, ring_buffer_t* optional_rx_buffer );
	OSStatus (*MxosUartFinalize)( mxos_uart_t uart );
	OSStatus (*MxosUartSend)( mxos_uart_t uart, const void* data, uint32_t size );
	OSStatus (*MxosUartRecv)( mxos_uart_t uart, void* data, uint32_t size, uint32_t timeout );
	uint32_t (*MxosUartGetLengthInBuffer)( mxos_uart_t uart ); 
	void     (*MxosUartPinRedirect)(mxos_uart_t uart);
    int (*disable_log_uart)(void);
} uart_api_t;

typedef void (*rtc_irq_handler)(void);

typedef struct {
	void (*MxosRtcInitialize)(void);
	OSStatus (*MxosRtcGetTime)(time_t *time);
	OSStatus (*MxosRtcSetTime)(time_t time);
    OSStatus (*MxosRtcSetalarm)(time_t *time, rtc_irq_handler handler);
} rtc_api_t;

typedef struct {
	/* Power management*/
	int (*pm_mcu_state)(power_state_t state, uint32_t time_dur);
	int (*pm_wakeup_source)(uint8_t wake_source);
	void (*wifi_off_mcu_standby)(uint32_t seconds);
	void (*MxosMcuPowerSaveConfig)( int enable );
} power_save_api_t;

typedef os_api_v1_t os_api_t;
typedef ssl_crypto_api_v1_t ssl_crypto_api_t;
typedef wifi_api_v1_t wifi_api_t;
typedef cli_api_v1_t cli_api_t;

/* API type define */
typedef struct 
{
	os_api_t *os_apis;
	lwip_api_t *lwip_apis;
	ssl_crypto_api_t *ssl_crypto_apis;
	wifi_api_t *wifi_apis;
	cli_api_t *cli_apis;

    flash_api_t *flash_apis;
	gpio_api_t *gpio_apis;
	uart_api_t *uart_apis;
	i2c_api_t *i2c_apis;
	spi_api_t *spi_apis;
	pwm_api_t *pwm_apis;
	rtc_api_t *rtc_apis;
	wdg_api_t *wdg_apis;
	adc_api_t *adc_apis;
	power_save_api_t *ps_apis;
	gtimer_api_t *gtimer_apis;
} kernel_api_v1_t;

typedef kernel_api_v1_t kernel_api_t;

typedef struct new_mxos_api_struct
{
	char *library_version;

	int (*mxos_api_get)(int version, void *kernel_apis);
} new_mxos_api_t;

mxos_api_t *moc_adapter(new_mxos_api_t *new_mxos_api);