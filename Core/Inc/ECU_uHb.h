#ifndef ECU_UHB_H
#define ECU_UHB_H

#include <Channels/ADCChannel.h>
#include <Channels/DigitalOutChannel.h>
#include <Channels/DigitalInChannel.h>
#include <Channels/PyroChannel.h>
#include <Channels/PressureControlChannel.h>
#include <Channels/ServoChannel.h>
#include <Channels/GenericChannel.h>
#include <Channels/RocketChannel.h>
#include <Can.h>
#include "../Modules/W25Qxx_Flash.h"
#include <Speaker.h>
#include <STRHAL.h>

class ECU_uHb: public GenericChannel
{
	public:
		ECU_uHb(uint32_t node_id, uint32_t fw_version, uint32_t refresh_divider);
		ECU_uHb(const ECU_uHb &other) = delete;
		ECU_uHb& operator=(const ECU_uHb &other) = delete;

		int init() override;
		//int reset() override;
		int exec() override;

		void testChannels();
		void testServo(ServoChannel &servo);

	private:
		STRHAL_GPIO_t ledRed, ledGreen;

		// Channels
		ADCChannel press_0, press_1, press_2, press_3, press_4, press_5;
		ADCChannel temp_0, temp_1, temp_2;
		ServoChannel servo_0, servo_1, servo_2;
		DigitalInChannel pyro0_cont, pyro1_cont, pyro2_cont;
		PyroChannel pyro_igniter0, pyro_igniter1, pyro_igniter2;
		DigitalOutChannel solenoid_0, solenoid_1;
		PressureControlChannel pressure_control;
		//RocketChannel rocket;

		Speaker speaker;
};

#endif /*ECU_UHB_H*/
