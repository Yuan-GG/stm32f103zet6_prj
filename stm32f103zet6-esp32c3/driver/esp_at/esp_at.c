#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "esp_usart.h"
#include "esp_at.h"


#define RX_BUFFER_SIZE  4096
// 接收状态，OK: 接收完成； ERROR: 接收错误； 
//          FAIL：接收失败（可用在未开始接收前设置状态）； 
//          FULL：接收长度过长
#define RX_RESULT_OK    0
#define RX_RESULT_ERROR 1
#define RX_RESULT_FAIL  2
// #define RX_RESULT_FULL  3


static uint8_t at_rxBuf[RX_BUFFER_SIZE];
static uint32_t at_rxLen;                      // 从 0 开始
static bool at_rxReady;
static uint8_t at_rxResult;


static void On_USART_Received(uint8_t data){

    // 没有数据请求，不接收数据
    if(!at_rxReady){
        return;
    }

    // 接收数据，防止缓冲区溢出
    if(at_rxLen < RX_BUFFER_SIZE){
        at_rxBuf[at_rxLen++] = data;
        at_rxBuf[at_rxLen] = '\0'; // 保证字符串结尾
    } else {
        at_rxResult = RX_RESULT_FAIL;
        at_rxReady = false;
        return;
    }

    // 数据接收完毕判断
    if(data == '\n'){
        if(at_rxLen >= 2 && at_rxBuf[at_rxLen - 2] == '\r'){
            // 收到 OK
            if(at_rxLen >= 4 && !memcmp(&at_rxBuf[at_rxLen - 4], "OK\r\n", 4)){
                at_rxResult = RX_RESULT_OK;
                // 这里的 at_rxReady = false 指的是传输结束
                at_rxReady = false;
            }
            else if(at_rxLen >= 7 && !memcmp(&at_rxBuf[at_rxLen - 7], "ERROR\r\n", 7)) {
                at_rxResult = RX_RESULT_ERROR;
                at_rxReady = false;
            }
        }
    }
    
}

bool ESP_AT_Init(void){
    at_rxReady = false;

    ESP_USART2_Init();
    ESP_USART2_Receive_Register(On_USART_Received);

    return ESP_AT_Reset();
}

/**
  * @brief  esp发送at命令
  * @param  cmd: 命令，这里只用于发送，无需修改，所以const char*即可
  * @param  rsp：响应，这里既需要接收响应字符串（传字符串的地址-二级指针），又不希望更改字符（cosnt）
  * @param  length: 响应字符串长度
  * @param timeout: 超时时间
  * @retval None
  */
bool ESP_AT_Send_Cmd(const char* cmd, const char** rsp, uint32_t* length, uint32_t timeout){
    // 清空上次接收的状态
    at_rxLen = 0;
    at_rxReady = true;
    at_rxResult = RX_RESULT_FAIL;

    ESP_USART2_SendString(cmd);
    ESP_USART2_SendString("\r\n");
    
    while(at_rxReady && timeout--){
        Delayms(1);
    }

    at_rxReady = false;
    if(rsp){
        *rsp = (const char*)at_rxBuf;
    }
    if(length){
        *length = at_rxLen;
    }

    return at_rxResult == RX_RESULT_OK;
}

bool ESP_AT_Send_Data(const char* data, uint32_t length){

    ESP_USART2_SendData((uint8_t *)data, length);

    return true;
}

bool ESP_AT_Reset(void){
    // 复位esp32
    if (!ESP_AT_Send_Cmd("AT+RESTORE", NULL, NULL, 1000)){
        return false;
    }
    Delayms(2000);

    // 关闭回显
    if (!ESP_AT_Send_Cmd("ATE0", NULL, NULL, 1000)){
        return false;
    }

    // 关闭存储
    if(!ESP_AT_Send_Cmd("AT+SYSSTORE=0", NULL, NULL, 1000)){
        return false;
    }

    return true;
}

bool ESP_AT_Wifi_Init(void){
    // 设置为 station 模式
    if(!ESP_AT_Send_Cmd("AT+CWMODE=1", NULL, NULL, 1000)){
        return false;
    }
    return true;
}

bool ESP_AT_Wifi_Connect(const char* ssid, const char* pwd){
    char cmd[64];

    // 连接 wifi
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    if(!ESP_AT_Send_Cmd(cmd, NULL, NULL, 10000)){
        return false;
    }

    return true;
}

bool ESP_AT_Http_Get(const char* url, const char** rsp, uint32_t* length, uint32_t timeout){
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "AT+HTTPCGET=\"%s\"", url);
    if(!ESP_AT_Send_Cmd(cmd, rsp, length, 10000)){
        return false;
    }

    return true;
}

bool ESP_AT_SNTP_Init(void){
    // 设置为 SNTP 模式
    if(!ESP_AT_Send_Cmd("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000)){
        return false;
    }

    // 查询 SNTP 时间
    if(!ESP_AT_Send_Cmd("AT+CIPSNTPTIME?", NULL, NULL, 1000)){
        return false;
    }

    return true;
}

bool ESP_AT_Time_Get(uint32_t* timestamp){
    const char *rsp;
    uint32_t length;

    if(!ESP_AT_Send_Cmd("AT+SYSTIMESTAMP?", &rsp, &length, 1000)){
        return false;
    }

    char *sts = strstr(rsp, "+SYSTIMESTAMP:") + strlen("+SYSTIMESTAMP:");

    *timestamp = atoi(sts);
    return true;
}
