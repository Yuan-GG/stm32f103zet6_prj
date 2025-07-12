#include <stdio.h>
#include "stm32f10x.h"
#include "timer.h"

static timer_elapsed_callback_t timer_elapsed_callback;
 
void timer_init(uint32_t period_ms){
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
  TIM_TimeBaseStruct.TIM_Prescaler = 72 - 1;      // 72MHz系统时钟频率，每个tick为1us
  TIM_TimeBaseStruct.TIM_Period = period_ms * 1000 - 1;
  TIM_TimeBaseStruct.TIM_ClockDivision = 0;
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void timer_start(void){
  TIM_Cmd(TIM2, ENABLE);
}

void timer_stop(void){
  TIM_Cmd(TIM2, DISABLE);
}

void timer_elapsed_register(timer_elapsed_callback_t cb){
  timer_elapsed_callback = cb;
}

void TIM2_IRQHandler(void){
  if(TIM_GetITStatus(TIM2, TIM_IT_Update)){
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    if(timer_elapsed_callback){
      timer_elapsed_callback();
    }
  }
}

/**
 * @brief  PWM输出，TIM3 CH1/2/3/4
 * @param  period_us
 * @return None
 */
void pwm_init(uint32_t period_us){
  GPIO_InitTypeDef GPIO_InitStructure;
  // ch1/2
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // ch3/4
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
  TIM_TimeBaseInitStructure.TIM_Prescaler     = 72 - 1;
  TIM_TimeBaseInitStructure.TIM_Period        = period_us - 1;
  TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse       = 0;
  TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);
  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM3, ENABLE);
}

/**
 * @brief  配置 PWM 占空比
 * @param  ch 通道x: 1/2/3/4
 * @param  duty_us 占空比，因为配置时基时使得1us/tick，以及重装载值为1ms，
 *                 所以0~1000对应0~100%
 * @return None
 */
void pwm_set_duty(uint32_t ch, uint32_t duty_us){
  switch (ch)
  {
    case 1:TIM_SetCompare1(TIM3, duty_us); break;
    case 2:TIM_SetCompare2(TIM3, duty_us); break;
    case 3:TIM_SetCompare3(TIM3, duty_us); break;
    case 4:TIM_SetCompare4(TIM3, duty_us); break;
    default:break;
  }
}

/**
 * @brief  使能 PWM 的TIM
 * @param  None
 * @return None
 */
void pwm_start(void){
  TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  除能 PWM 的TIM
 * @param  None
 * @return None
 */
void pwm_stop(void){
  TIM_Cmd(TIM3, DISABLE);
}
