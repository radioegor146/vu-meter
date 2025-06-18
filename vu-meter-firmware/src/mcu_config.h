#ifndef MCU_CONFIG_H_
#define MCU_CONFIG_H_

#include <ch32x035.h>

#define INPUT_L_PORT GPIOA
#define INPUT_L_PIN GPIO_Pin_0
#define INPUT_R_PORT GPIOA
#define INPUT_R_PIN GPIO_Pin_1

#define SCK_PORT GPIOA
#define SCK_PIN GPIO_Pin_5
#define MOSI_PORT GPIOA
#define MOSI_PIN GPIO_Pin_7
#define LATCH_PORT GPIOB
#define LATCH_PIN GPIO_Pin_1

#endif