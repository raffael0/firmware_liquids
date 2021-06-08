#ifndef HARDWARE_BOARD_CONFIG_H_
#define HARDWARE_BOARD_CONFIG_H_

#define LCB 1
#define IOB 2

#define BOARD IOB

#if BOARD == LCB
#include "lcb.h"
#define MAIN_CAN_BUS LCB_MAIN_CAN_BUS
#elif BOARD == IOB
#include "iob.h"
#define MAIN_CAN_BUS IOB_MAIN_CAN_BUS
#endif
#define DEBUG_CAN_BUS 1



#define SPI_TIMEOUT_VALUE 1000
#define ADS_TIMEOUT_VALUE SPI_TIMEOUT_VALUE

#define ADS_nDRDY_Pin LL_GPIO_PIN_13
#define ADS_nDRDY_GPIO_Port GPIOC
#define ADS_nRESET_Pin LL_GPIO_PIN_12
#define ADS_nRESET_GPIO_Port GPIOC


//GPIO3
#define EN_GPIO_Port		GPIOC
#define EN_Pin				LL_GPIO_PIN_13

//GPIO8
#define STATUS_GPIO_Port	GPIOA
#define STATUS_Pin			LL_GPIO_PIN_10

//GPIO9
#define FAULT_GPIO_Port		GPIOE
#define FAULT_Pin			LL_GPIO_PIN_10

//GPIO2
#define CSN_CTR_GPIO_Port	GPIOC
#define CSN_CTR_Pin			LL_GPIO_PIN_12

//GPIO10
#define CSN_DRV_GPIO_Port	GPIOE
#define CSN_DRV_Pin			LL_GPIO_PIN_15

//GPIO4
#define CSN_ENC0_GPIO_Port	GPIOE
#define CSN_ENC0_Pin		LL_GPIO_PIN_0

//GPIO5
#define CSN_ENC1_GPIO_Port	GPIOE
#define CSN_ENC1_Pin		LL_GPIO_PIN_1

//GPIO6
#define CSN_ENC2_GPIO_Port	GPIOE
#define CSN_ENC2_Pin		LL_GPIO_PIN_3

//GPIO7
#define CSN_ENC3_GPIO_Port	GPIOE
#define CSN_ENC3_Pin		LL_GPIO_PIN_7




#define PIN_SPEAKER_N_Pin LL_GPIO_PIN_8
#define PIN_SPEAKER_N_GPIO_Port GPIOE
#define PIN_SPEAKER_Pin LL_GPIO_PIN_9
#define PIN_SPEAKER_GPIO_Port GPIOE
#define BUTTON_Pin LL_GPIO_PIN_11
#define BUTTON_GPIO_Port GPIOB
#define LED_STATUS_1_Pin LL_GPIO_PIN_8
#define LED_STATUS_1_GPIO_Port GPIOD
#define LED_STATUS_2_Pin LL_GPIO_PIN_9
#define LED_STATUS_2_GPIO_Port GPIOD
#define LED_DEBUG_Pin LL_GPIO_PIN_10
#define LED_DEBUG_GPIO_Port GPIOD

#endif
