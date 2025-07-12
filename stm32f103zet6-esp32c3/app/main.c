#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "led.h"
#include "key.h"
#include "key_exti.h"
#include "esp_usart.h"
#include "st7735.h"
#include "mpu6050.h"
#include "swi2c.h"
#include "timer.h"
#include "rtc.h"
#include "esp_at.h"
#include "weather.h"
#include "main.h"


static const char *wifi_ssid = "H3C_401";
static const char *wifi_password = "xyp0622.";
static const char *weather_url = "https://api.seniverse.com/v3/weather/now.json?key=SskNx7A-3BrkJjutx&location=maoming&language=en&unit=c";


static void Board_LowLevel_Init(){
    /* Enable RCC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    
    /* Enable TIM2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    /* Enable USART2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    /* Enable USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* Enable SPI1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /* Enable AHB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    /* Enable RTC clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Enable BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();

    // LSE 默认关闭
    RCC_LSEConfig(RCC_LSE_ON);
    // LSE 不是说开就马上开了，还需要等RDY
    while(!RCC_GetFlagStatus(RCC_FLAG_LSERDY));

    /* Configure NVIC priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

int main(){
    Board_LowLevel_Init();
    
    led_init();
    rtc_init();

    st7735_init();
    st7735_fill_screen(ST7735_BLACK);

    delayms(2000);

    if (!esp_at_init())
    {
        while (1);
    }
    if (!esp_at_wifi_init())
    {
        while (1);
    }
    if (!esp_at_wifi_connect(wifi_ssid, wifi_password))
    {
        while (1);
    }

    bool weather_ok = false;
    bool sntp_ok = false;
    uint32_t t = 0;
    char str[64];

    while(1){
        t++;
        delayms(1000);

        rtc_date_t date;
        rtc_get_date(&date);
        snprintf(str, sizeof(str), "%04d-%02d-%02d", date.year, date.month, date.day);
        st7735_write_string(0, 0, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        snprintf(str, sizeof(str), "%02d:%02d:%02d", date.hour, date.minute, date.second);
        st7735_write_string(0, 16, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);

        // 1min请求一次
        if(!weather_ok || t % 60 == 0){
            const char *rsp;
            weather_ok = esp_at_http_get(weather_url, &rsp, NULL, 10000);
            weather_t weather;
            weather_parse(rsp, &weather);

            snprintf(str, sizeof(str), "%s", "maoming");
            st7735_write_string(0, 64, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
            snprintf(str, sizeof(str), "%s", weather.weather);
            st7735_write_string(0, 80, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
            snprintf(str, sizeof(str), "%s", weather.temperature);
            st7735_write_string(0, 96, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);

            if (!sntp_ok || t % 3600 == 0)
            {
                uint32_t ts;
                sntp_ok = esp_at_sntp_init();
                delayms(1000);
                esp_at_time_get(&ts);
                rtc_set_timestamp(ts + 8 * 60 * 60);
            }
        }
    }
}
