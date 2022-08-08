//#ifndef _WEBSERVER_H_
//#define _WEBSERVER_H_
//
//#include <stdbool.h>
//#include <sys/socket.h>
//#include <sys/param.h>
//#include <netinet/in.h>
//#include <esp_log.h>
//#include <esp_err.h>
//
//#include <esp_http_server.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//
//
//void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
//void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
//httpd_handle_t start_webserver(void);
//
//
//#ifdef __cplusplus
//}
//#endif
//
//#endif /* ! _WEBSERVER_H_ */

esp_err_t init_spiffs(void);