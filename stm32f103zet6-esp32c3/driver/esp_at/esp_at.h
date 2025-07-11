#ifndef __ESP_AT_H__
#define __ESP_AT_H__


#include <stdbool.h>
#include <stdint.h>


bool ESP_AT_Init(void);
bool ESP_AT_Send_Cmd(const char* cmd, const char** rsp, uint32_t* length, uint32_t timeout);
bool ESP_AT_Send_Data(const char* data, uint32_t length);

bool ESP_AT_Reset(void);

bool ESP_AT_Wifi_Init(void);
bool ESP_AT_Wifi_Connect(const char* ssid, const char* pwd);
bool ESP_AT_Http_Get(const char* url, const char** rsp, uint32_t* length, uint32_t timeout);
bool ESP_AT_Time_Get(uint32_t* timestamp);
bool ESP_AT_SNTP_Init(void);

#endif /* __ESP_AT_H__ */
