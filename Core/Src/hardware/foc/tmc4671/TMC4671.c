/*
 * TMC4671.c
 *
 *  Created on: 30.09.2016
 *      Author: ed
 */
#include "foc/tmc4671/TMC4671.h"
#include "foc/swdriver.h"
#include "spi.h"
#include "serial.h"

#define STATE_NOTHING_TO_DO    0
#define STATE_START_INIT       1
#define STATE_WAIT_INIT_TIME   2
#define STATE_ESTIMATE_OFFSET  3

// spi access
uint32_t tmc4671_readInt(uint8_t address)
{
	uint8_t txData[5] = {0};
	txData[0] = TMC_ADDRESS(address);
	txData[1] = 0;
	txData[2] = 0;
	txData[3] = 0;
	txData[4] = 0;

	uint8_t rxData[5] = {0};

	swdriver_setCsnController( false);
	if(SPI_Transmit_Receive(swdriver.spi, txData, rxData, 5) != NOICE) Serial_PrintString("fack");
	swdriver_setCsnController(true);

	return (uint32_t)((rxData[1] << 24) | (rxData[2] << 16) | (rxData[3] << 8) | (rxData[4] << 0));
}

void tmc4671_writeInt(uint8_t address, int32_t value)
{
	uint8_t data[5] = {0};
	data[0] = address | 0x80;
	data[1] = 0xFF & (value >> 24);
	data[2] = 0xFF & (value >> 16);
	data[3] = 0xFF & (value >> 8);
	data[4] = 0xFF & (value >> 0);

	swdriver_setCsnController(false);
	if(SPI_Transmit_Receive(swdriver.spi, data, 0, 5) != NOICE) Serial_PrintString("fack");
	swdriver_setCsnController(true);
}

/*
void tmc4671_writeInt_nonBlocking(uint8_t address, int32_t value) //TODO: make every write non blocking and just wait for previous transaction done?
{

	int32_t data[5] = {0};
	data[0] = address | 0x80;
	data[1] = 0xFF & (value >> 24);
	data[2] = 0xFF & (value >> 16);
	data[3] = 0xFF & (value >> 8);
	data[4] = 0xFF & (value >> 0);

	swdriver_setCsnController(false);

	//TODO FIX HAL_SPI_Transmit_IT(swdriver.SPI, data, 5);

}
*/
//TODO FIX
/*
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) //FIXME: pfusch
{
	swdriver_setCsnController(true);
	swdriver_setCsnDriver(true);
	swdriver_setCsnEncoder(true);
}
*/
uint16_t tmc4671_readRegister16BitValue(uint8_t address, uint8_t channel)
{
	int32_t registerValue = tmc4671_readInt(address);

	// read one channel
	switch (channel)
	{
		case BIT_0_TO_15:
			return (registerValue & 0xFFFF);
		default:
		case BIT_16_TO_31:
			return ((registerValue >> 16) & 0xFFFF);
	}
	return 0;
}

void tmc4671_writeRegister16BitValue(uint8_t address, uint8_t channel, uint16_t value)
{
	// read actual register content
	int32_t registerValue = tmc4671_readInt(address);

	// update one channel
	switch (channel)
	{
		case BIT_0_TO_15:
			registerValue &= 0xFFFF0000;
			registerValue |= value;
			break;
		default:
		case BIT_16_TO_31:
			registerValue &= 0x0000FFFF;
			registerValue |= (value << 16);
			break;
	}
	// write the register
	tmc4671_writeInt(address, registerValue);
}

void tmc4671_switchToMotionMode(uint8_t mode)
{
	// switch motion mode
	uint32_t actualModeRegister = tmc4671_readInt(TMC4671_MODE_RAMP_MODE_MOTION);
	actualModeRegister &= 0xFFFFFF00;
	actualModeRegister |= mode;
	tmc4671_writeInt(TMC4671_MODE_RAMP_MODE_MOTION, actualModeRegister);
}

void tmc4671_setTargetTorque_raw(int32_t targetTorque)
{
	tmc4671_switchToMotionMode(TMC4671_MOTION_MODE_TORQUE);
	tmc4671_writeRegister16BitValue(TMC4671_PID_TORQUE_FLUX_TARGET, BIT_16_TO_31, targetTorque);
}

int32_t tmc4671_getTargetTorque_raw(void)
{
	// remember last set index
	uint32_t lastIndex = tmc4671_readInt(TMC4671_INTERIM_ADDR);

	// get value
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, 0);
	int32_t value = (int32_t) tmc4671_readInt(TMC4671_INTERIM_DATA);

	// reset last set index
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, lastIndex);
	return value;
}

int32_t tmc4671_getActualTorque_raw(void)
{
	return (int16_t) tmc4671_readRegister16BitValue(TMC4671_PID_TORQUE_FLUX_ACTUAL, BIT_16_TO_31);
}

int32_t tmc4671_getActualRampTorque_raw(void)
{
	// no ramp implemented
	return 0;
}

void tmc4671_setTargetTorque_mA(uint16_t torqueMeasurementFactor, int32_t targetTorque)
{
	tmc4671_switchToMotionMode(TMC4671_MOTION_MODE_TORQUE);
	tmc4671_writeRegister16BitValue(TMC4671_PID_TORQUE_FLUX_TARGET, BIT_16_TO_31, (targetTorque * 256) / (int32_t) torqueMeasurementFactor);
}

int32_t tmc4671_getTargetTorque_mA(uint16_t torqueMeasurementFactor)
{
	return (tmc4671_getTargetTorque_raw() * (int32_t) torqueMeasurementFactor) / 256;
}

int32_t tmc4671_getActualTorque_mA(uint16_t torqueMeasurementFactor)
{
	return (tmc4671_getActualTorque_raw() * (int32_t) torqueMeasurementFactor) / 256;
}

int32_t tmc4671_getTargetTorqueFluxSum_mA(uint16_t torqueMeasurementFactor)
{
	// remember last set index
	uint32_t lastIndex = tmc4671_readInt(TMC4671_INTERIM_ADDR);

	// get target torque value
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, 0);
	int32_t torque = (int32_t) tmc4671_readInt(TMC4671_INTERIM_DATA);

	// get target flux value
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, 1);
	int32_t flux = (int32_t) tmc4671_readInt(TMC4671_INTERIM_DATA);

	// reset last set index
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, lastIndex);

	return (((int32_t) flux + (int32_t) torque) * (int32_t) torqueMeasurementFactor) / 256;
}

int32_t tmc4671_getActualTorqueFluxSum_mA(uint16_t torqueMeasurementFactor)
{
	int32_t registerValue = tmc4671_readInt(TMC4671_PID_TORQUE_FLUX_ACTUAL);
	int16_t flux = (registerValue & 0xFFFF);
	int16_t torque = ((registerValue >> 16) & 0xFFFF);
	return (((int32_t) flux + (int32_t) torque) * (int32_t) torqueMeasurementFactor) / 256;
}

int32_t tmc4671_getActualRampTorque_mA(uint16_t torqueMeasurementFactor)
{
	// no ramp implemented
	UNUSED(torqueMeasurementFactor);
	return 0;
}

void tmc4671_setTargetFlux_raw(int32_t targetFlux)
{
	// do not change the MOTION_MODE here! target flux can also be used during velocity and position modes
	tmc4671_writeRegister16BitValue(TMC4671_PID_TORQUE_FLUX_TARGET, BIT_0_TO_15, targetFlux);
}

int32_t tmc4671_getTargetFlux_raw(void)
{
	// remember last set index
	uint32_t lastIndex = tmc4671_readInt(TMC4671_INTERIM_ADDR);

	// get value
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, 1);
	int32_t value = (int32_t) tmc4671_readInt(TMC4671_INTERIM_DATA);

	// reset last set index
	tmc4671_writeInt(TMC4671_INTERIM_ADDR, lastIndex);
	return value;
}

int32_t tmc4671_getActualFlux_raw(void)
{
	return (int16_t) tmc4671_readRegister16BitValue(TMC4671_PID_TORQUE_FLUX_ACTUAL, BIT_0_TO_15);
}

void tmc4671_setTargetFlux_mA(uint16_t torqueMeasurementFactor, int32_t targetFlux)
{
	// do not change the MOTION_MODE here! target flux can also be used during velocity and position modes
	tmc4671_writeRegister16BitValue(TMC4671_PID_TORQUE_FLUX_TARGET, BIT_0_TO_15, (targetFlux * 256) / (int32_t) torqueMeasurementFactor);
}

int32_t tmc4671_getTargetFlux_mA(uint16_t torqueMeasurementFactor)
{
	return (tmc4671_getTargetFlux_raw() * (int32_t) torqueMeasurementFactor) / 256;
}

int32_t tmc4671_getActualFlux_mA(uint16_t torqueMeasurementFactor)
{
	return (tmc4671_getActualFlux_raw() * (int32_t) torqueMeasurementFactor) / 256;
}

void tmc4671_setTorqueFluxLimit_mA(uint16_t torqueMeasurementFactor, int32_t max)
{
	tmc4671_writeRegister16BitValue(TMC4671_PID_TORQUE_FLUX_LIMITS, BIT_0_TO_15, (max * 256) / (int32_t) torqueMeasurementFactor);
}

int32_t tmc4671_getTorqueFluxLimit_mA(uint16_t torqueMeasurementFactor)
{
	return ((int32_t) tmc4671_readRegister16BitValue(TMC4671_PID_TORQUE_FLUX_LIMITS, BIT_0_TO_15) * (int32_t) torqueMeasurementFactor) / 256;
}

void tmc4671_setTargetVelocity(int32_t targetVelocity)
{
	tmc4671_switchToMotionMode(TMC4671_MOTION_MODE_VELOCITY);
	tmc4671_writeInt(TMC4671_PID_VELOCITY_TARGET, targetVelocity);
}

int32_t tmc4671_getTargetVelocity(void)
{
	return (int32_t) tmc4671_readInt(TMC4671_PID_VELOCITY_TARGET);
}

int32_t tmc4671_getActualVelocity(void)
{
	return (int32_t) tmc4671_readInt(TMC4671_PID_VELOCITY_ACTUAL);
}

int32_t tmc4671_getActualRampVelocity(void)
{
	// no ramp implemented
	return 0;
}

void tmc4671_setAbsolutTargetPosition(int32_t targetPosition)
{
	tmc4671_switchToMotionMode(TMC4671_MOTION_MODE_POSITION);
	tmc4671_writeInt(TMC4671_PID_POSITION_TARGET, targetPosition);
}

void tmc4671_setRelativeTargetPosition(int32_t relativePosition)
{
	tmc4671_switchToMotionMode(TMC4671_MOTION_MODE_POSITION);
	// determine actual position and add relative position ticks
	tmc4671_writeInt(TMC4671_PID_POSITION_TARGET, (int32_t) tmc4671_readInt(TMC4671_PID_POSITION_ACTUAL) + relativePosition);
}

int32_t tmc4671_getTargetPosition(void)
{
	return (int32_t) tmc4671_readInt(TMC4671_PID_POSITION_TARGET);
}

void tmc4671_setActualPosition(int32_t actualPosition)
{
	tmc4671_writeInt(TMC4671_PID_POSITION_ACTUAL, actualPosition);
}

int32_t tmc4671_getActualPosition(void)
{
	return (int32_t) tmc4671_readInt(TMC4671_PID_POSITION_ACTUAL);
}

int32_t tmc4671_getActualRampPosition(void)
{
	// no ramp implemented
	return 0;
}

// encoder initialization
void tmc4671_doEncoderInitializationMode0(uint8_t *initState, uint16_t initWaitTime, uint16_t *actualInitWaitTime, uint16_t startVoltage)
{
	static uint16_t last_Phi_E_Selection = 0;
	static uint32_t last_UQ_UD_EXT = 0;
	static int16_t last_PHI_E_EXT = 0;

	switch (*initState)
	{
		case STATE_NOTHING_TO_DO:
			*actualInitWaitTime = 0;
			break;
		case STATE_START_INIT: // started by writing 1 to initState

			// save actual set values for PHI_E_SELECTION, UQ_UD_EXT, and PHI_E_EXT
			last_Phi_E_Selection = (uint16_t) tmc4671_readRegister16BitValue(TMC4671_PHI_E_SELECTION, BIT_0_TO_15);
			last_UQ_UD_EXT = (uint32_t) tmc4671_readInt(TMC4671_UQ_UD_EXT);
			last_PHI_E_EXT = (int16_t) tmc4671_readRegister16BitValue(TMC4671_PHI_E_EXT, BIT_0_TO_15);

			// set ABN_DECODER_PHI_E_OFFSET to zero
			tmc4671_writeRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, BIT_16_TO_31, 0);

			// select phi_e_ext
			tmc4671_writeRegister16BitValue(TMC4671_PHI_E_SELECTION, BIT_0_TO_15, 1);

			// set an initialization voltage on UD_EXT (to the flux, not the torque!)
			tmc4671_writeRegister16BitValue(TMC4671_UQ_UD_EXT, BIT_16_TO_31, 0);
			tmc4671_writeRegister16BitValue(TMC4671_UQ_UD_EXT, BIT_0_TO_15, startVoltage);

			// set the "zero" angle
			tmc4671_writeRegister16BitValue(TMC4671_PHI_E_EXT, BIT_0_TO_15, 0);

			*initState = STATE_WAIT_INIT_TIME;
			break;
		case STATE_WAIT_INIT_TIME:
			// wait until initialization time is over (until no more vibration on the motor)
			(*actualInitWaitTime)++;
			if (*actualInitWaitTime >= initWaitTime)
			{
				// set internal encoder value to zero
				tmc4671_writeInt(TMC4671_ABN_DECODER_COUNT, 0);

				// switch back to last used UQ_UD_EXT setting
				tmc4671_writeInt(TMC4671_UQ_UD_EXT, last_UQ_UD_EXT);

				// set PHI_E_EXT back to last value
				tmc4671_writeRegister16BitValue(TMC4671_PHI_E_EXT, BIT_0_TO_15, last_PHI_E_EXT);

				// switch back to last used PHI_E_SELECTION setting
				tmc4671_writeRegister16BitValue(TMC4671_PHI_E_SELECTION, BIT_0_TO_15, last_Phi_E_Selection);

				// go to next state
				*initState = STATE_ESTIMATE_OFFSET;
			}
			break;
		case STATE_ESTIMATE_OFFSET:
			// you can do offset estimation here (wait for N-Channel if available and save encoder value)

			// go to ready state
			*initState = 0;
			break;
		default:
			*initState = 0;
			break;
	}
}

int16_t tmc4671_getS16CircleDifference(int16_t newValue, int16_t oldValue)
{
	return (newValue - oldValue);
}

void tmc4671_doEncoderInitializationMode2(uint8_t *initState, uint16_t *actualInitWaitTime)
{
	static int16_t hall_phi_e_old = 0;
	static int16_t hall_phi_e_new = 0;
	static int16_t actual_coarse_offset = 0;

	switch (*initState)
	{
		case STATE_NOTHING_TO_DO:
			*actualInitWaitTime = 0;
			break;
		case STATE_START_INIT: // started by writing 1 to initState
			// turn hall_mode interpolation off (read, clear bit 8, write back)
			tmc4671_writeInt(TMC4671_HALL_MODE, tmc4671_readInt(TMC4671_HALL_MODE) & 0xFFFFFEFF);

			// set ABN_DECODER_PHI_E_OFFSET to zero
			tmc4671_writeRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, BIT_16_TO_31, 0);

			// read actual hall angle
			hall_phi_e_old = (int16_t) tmc4671_readRegister16BitValue(TMC4671_HALL_PHI_E_INTERPOLATED_PHI_E, BIT_0_TO_15);

			// read actual abn_decoder angle and compute difference to actual hall angle
			actual_coarse_offset = tmc4671_getS16CircleDifference(hall_phi_e_old, (int16_t) tmc4671_readRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M, BIT_16_TO_31));

			// set ABN_DECODER_PHI_E_OFFSET to actual hall-abn-difference, to use the actual hall angle for coarse initialization
			tmc4671_writeRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, BIT_16_TO_31, actual_coarse_offset);

			*initState = STATE_WAIT_INIT_TIME;
			break;
		case STATE_WAIT_INIT_TIME:
			// read actual hall angle
			hall_phi_e_new = (int16_t) tmc4671_readRegister16BitValue(TMC4671_HALL_PHI_E_INTERPOLATED_PHI_E, BIT_0_TO_15);

			// wait until hall angle changed
			if (hall_phi_e_old != hall_phi_e_new)
			{
				// estimated value = old value + diff between old and new (handle int16_t overrun)
				int16_t hall_phi_e_estimated = hall_phi_e_old + tmc4671_getS16CircleDifference(hall_phi_e_new, hall_phi_e_old) / 2;

				// read actual abn_decoder angle and consider last set abn_decoder_offset
				int16_t abn_phi_e_actual = (int16_t) tmc4671_readRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M, BIT_16_TO_31) - actual_coarse_offset;

				// set ABN_DECODER_PHI_E_OFFSET to actual estimated angle - abn_phi_e_actual difference
				tmc4671_writeRegister16BitValue(TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, BIT_16_TO_31, tmc4671_getS16CircleDifference(hall_phi_e_estimated, abn_phi_e_actual));

				// go to ready state
				*initState = 0;
			}
			break;
		default:
			*initState = 0;
			break;
	}
}

void tmc4671_checkEncderInitialization(uint32_t actualSystick, uint8_t initMode, uint8_t *initState, uint16_t initWaitTime, uint16_t *actualInitWaitTime, uint16_t startVoltage)
{
	// use the systick as 1ms timer for encoder initialization
	static uint32_t lastSystick = 0;
	if (actualSystick != lastSystick)
	{
		// needs timer to use the wait time
		if (initMode == 0)
		{
			tmc4671_doEncoderInitializationMode0(initState, initWaitTime, actualInitWaitTime, startVoltage);
		}
		lastSystick = actualSystick;
	}

	// needs no timer
	if (initMode == 2)
	{
		tmc4671_doEncoderInitializationMode2(initState, actualInitWaitTime);
	}
}

void tmc4671_periodicJob(uint32_t actualSystick, uint8_t initMode, uint8_t *initState, uint16_t initWaitTime, uint16_t *actualInitWaitTime, uint16_t startVoltage)
{
	tmc4671_checkEncderInitialization(actualSystick, initMode, initState, initWaitTime, actualInitWaitTime, startVoltage);
}

void tmc4671_startEncoderInitialization(uint8_t mode, uint8_t *initMode, uint8_t *initState)
{
	// allow only a new initialization if no actual initialization is running
	if (*initState == STATE_NOTHING_TO_DO)
	{
		if (mode == 0) // estimate offset
		{
			// set mode
			*initMode = 0;

			// start initialization
			*initState = 1;
		}
		else if (mode == 2) // use hall sensor signals
		{
			// set mode
			*initMode = 2;

			// start initialization
			*initState = 1;
		}
	}
}

void tmc4671_updatePhiSelectionAndInitialize(uint8_t actualPhiESelection, uint8_t desiredPhiESelection, uint8_t initMode, uint8_t *initState)
{
	if (actualPhiESelection != desiredPhiESelection)
	{
		tmc4671_writeInt(TMC4671_PHI_E_SELECTION, desiredPhiESelection);

		switch (desiredPhiESelection)
		{
			case 3:
				tmc4671_startEncoderInitialization(initMode, &initMode, initState);
				break;
			default:
				break;
		}
	}
}

// =====

void tmc4671_disablePWM(void)
{
	tmc4671_writeInt(TMC4671_PWM_SV_CHOP, 0);
}

uint8_t tmc4671_getMotorType(void)
{
	return FIELD_GET(tmc4671_readInt(TMC4671_MOTOR_TYPE_N_POLE_PAIRS), TMC4671_MOTOR_TYPE_MASK, TMC4671_MOTOR_TYPE_SHIFT);
}

void tmc4671_setMotorType(uint8_t motorType)
{
	TMC4671_FIELD_UPDATE(TMC4671_MOTOR_TYPE_N_POLE_PAIRS, TMC4671_MOTOR_TYPE_MASK, TMC4671_MOTOR_TYPE_SHIFT, motorType);
}

uint8_t tmc4671_getPolePairs(void)
{
	return FIELD_GET(tmc4671_readInt(TMC4671_MOTOR_TYPE_N_POLE_PAIRS), TMC4671_N_POLE_PAIRS_MASK, TMC4671_N_POLE_PAIRS_SHIFT);
}

void tmc4671_setPolePairs(uint8_t polePairs)
{
	TMC4671_FIELD_UPDATE(TMC4671_MOTOR_TYPE_N_POLE_PAIRS, TMC4671_N_POLE_PAIRS_MASK, TMC4671_N_POLE_PAIRS_SHIFT, polePairs);
}

uint16_t tmc4671_getAdcI0Offset(void)
{
	return FIELD_GET(tmc4671_readInt(TMC4671_ADC_I0_SCALE_OFFSET), TMC4671_ADC_I0_OFFSET_MASK, TMC4671_ADC_I0_OFFSET_SHIFT);
}

void tmc4671_setAdcI0Offset(uint16_t offset)
{
	TMC4671_FIELD_UPDATE(TMC4671_ADC_I0_SCALE_OFFSET, TMC4671_ADC_I0_OFFSET_MASK, TMC4671_ADC_I0_OFFSET_SHIFT, offset);
}

uint16_t tmc4671_getAdcI1Offset(void)
{
	return FIELD_GET(tmc4671_readInt(TMC4671_ADC_I1_SCALE_OFFSET), TMC4671_ADC_I1_OFFSET_MASK, TMC4671_ADC_I1_OFFSET_SHIFT);
}

void tmc4671_setAdcI1Offset(uint16_t offset)
{
	TMC4671_FIELD_UPDATE(TMC4671_ADC_I1_SCALE_OFFSET, TMC4671_ADC_I1_OFFSET_MASK, TMC4671_ADC_I1_OFFSET_SHIFT, offset);
}

void tmc4671_setTorqueFluxPI(uint16_t pParameter, uint16_t iParameter)
{
	tmc4671_writeInt(TMC4671_PID_FLUX_P_FLUX_I, ((uint32_t) pParameter << 16) | (uint32_t) iParameter);
	tmc4671_writeInt(TMC4671_PID_TORQUE_P_TORQUE_I, ((uint32_t) pParameter << 16) | (uint32_t) iParameter);
}

void tmc4671_setVelocityPI(uint16_t pParameter, uint16_t iParameter)
{
	tmc4671_writeInt(TMC4671_PID_VELOCITY_P_VELOCITY_I, ((uint32_t) pParameter << 16) | (uint32_t) iParameter);
}

void tmc4671_setPositionPI(uint16_t pParameter, uint16_t iParameter)
{
	tmc4671_writeInt(TMC4671_PID_POSITION_P_POSITION_I, ((uint32_t) pParameter << 16) | (uint32_t) iParameter);
}

int32_t tmc4671_readFieldWithDependency(uint8_t reg, uint8_t dependsReg, uint32_t dependsValue, uint32_t mask, uint8_t shift)
{
	// remember old depends value
	uint32_t lastDependsValue = tmc4671_readInt(dependsReg);

	// set needed depends value
	tmc4671_writeInt(dependsReg, dependsValue);
	uint32_t value = FIELD_GET(tmc4671_readInt(reg), mask, shift);

	// set old depends value
	tmc4671_writeInt(dependsReg, lastDependsValue);
	return value;
}
