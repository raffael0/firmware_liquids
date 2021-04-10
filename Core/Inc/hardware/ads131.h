#ifndef ADS131_H_
#define ADS131_H_

#include "main.h"
#include "cmds.h"
#include <stdint.h>

#define ADS131_NO_NEW_DATA 0xFFFFFFFF

typedef enum
{
	ADS131_ID = 0,
	ADS131_STATUS,
	ADS131_MODE,
	ADS131_CLOCK,
	ADS131_GAIN1,
	ADS131_GAIN2,
	ADS131_CFG,
	ADS131_THRSHLD_MSB,
	ADS131_THRSHLD_LSB,
	ADS131_CH0_CFG,
	ADS131_CH0_OCAL_MSB,
	ADS131_CH0_OCAL_LSB,
	ADS131_CH0_GCAL_MSB,
	ADS131_CH0_GCAL_LSB,
	ADS131_CH1_CFG,
	ADS131_CH1_OCAL_MSB,
	ADS131_CH1_OCAL_LSB,
	ADS131_CH1_GCAL_MSB,
	ADS131_CH1_GCAL_LSB,
	ADS131_CH2_CFG,
	ADS131_CH2_OCAL_MSB,
	ADS131_CH2_OCAL_LSB,
	ADS131_CH2_GCAL_MSB,
	ADS131_CH2_GCAL_LSB,
	ADS131_CH3_CFG,
	ADS131_CH3_OCAL_MSB,
	ADS131_CH3_OCAL_LSB,
	ADS131_CH3_GCAL_MSB,
	ADS131_CH3_GCAL_LSB,
	ADS131_CH4_CFG,
	ADS131_CH4_OCAL_MSB,
	ADS131_CH4_OCAL_LSB,
	ADS131_CH4_GCAL_MSB,
	ADS131_CH4_GCAL_LSB,
	ADS131_CH5_CFG,
	ADS131_CH5_OCAL_MSB,
	ADS131_CH5_OCAL_LSB,
	ADS131_CH5_GCAL_MSB,
	ADS131_CH5_GCAL_LSB,
	ADS131_CH6_CFG,
	ADS131_CH6_OCAL_MSB,
	ADS131_CH6_OCAL_LSB,
	ADS131_CH6_GCAL_MSB,
	ADS131_CH6_GCAL_LSB,
	ADS131_CH7_CFG,
	ADS131_CH7_OCAL_MSB,
	ADS131_CH7_OCAL_LSB,
	ADS131_CH7_GCAL_MSB,
	ADS131_CH7_GCAL_LSB,
	ADS131_REGMAP_CRC,
	ADS131_RESERVED
} ADS131_REG;

typedef enum
{					 // RESPONSE
	ADS131_CMD_NULL = 0x0000,					 // Status register
	ADS131_CMD_RESET = 0X0011,					 // 0xFF28
	ADS131_CMD_STANDBY = 0X0022,					 // 0x0022
	ADS131_CMD_WAKEUP = 0X0033,					 // 0x0033
	ADS131_CMD_LOCK = 0X0555,					 // 0x0555
	ADS131_CMD_UNLOCK = 0X0655,					 // 0x0655
	ADS131_CMD_RREG = 0XA000,					 //	(Cmd) and data
	ADS131_CMD_WREG = 0X6000 // Cmd and data

} ADS131_CMD;

#define ADS131_RREG_OPCODE(n, a) (ADS131_CMD_RREG | ((a & 0x3F) << 7) | (n & 0x7F))
#define ADS131_WREG_OPCODE(n, a) (ADS131_CMD_WREG | ((a & 0x3F) << 7) | (n & 0x7F))

Result_t Ads131_Init(void);
Result_t Ads131_WriteRegister(uint32_t reg, uint32_t value);
Result_t Ads131_ReadRegisters(uint32_t reg, uint32_t n, int32_t data[]);
Result_t Ads131_UpdateData(void);
int32_t  Ads131_GetData(uint8_t ch);
uint32_t Ads131_GetStatus(void);

#endif
