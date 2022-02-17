#ifndef SERVOCHANNEL_H
#define SERVOCHANNEL_H

#include "AbstractChannel.h"
#include "ADCChannel.h"

#include <can_houbolt/channels/servo_channel_def.h>
#include <LID.h>

class ServoChannel : public AbstractChannel {
	public:
		ServoChannel(uint8_t channel_id, const LID_TIM_TimerId_t &pwm_timer, const LID_TIM_ChannelId_t &control, const ADCChannel &feedbackChannel, const ADCChannel &currentChannel, const LID_GPIO_t &led_o);

		int init() override;
		int reset() override;
		int exec() override;

		int prcMsg(uint8_t cmd_id, uint8_t variable_id, uint32_t data, uint32_t &ret) override;
		int getSensorData(uint8_t *data, uint8_t &n) override;


		int move();

		int setTargetPos(uint16_t pos);
		uint16_t getTargetPos() const;

		uint16_t getPos() const;

		static constexpr uint16_t SERVO_PWM_FREQ = 50;

	protected:
		int setVar(uint8_t variable_id, uint32_t data) override;
		int getVar(uint8_t variable_id, uint32_t &data) const override;

	private:
		LID_TIM_TimerId_t pwm_tim;
		LID_TIM_ChannelId_t ctrl_chid;
		LID_TIM_PWM_Channel_t pwm_ch;

		const ADCChannel &fdbkCh;
		const ADCChannel &currCh;
		LID_GPIO_t led_o;

		uint32_t t_pos;
		uint32_t c_pos;

		uint32_t start_pos = 0;
		uint32_t end_pos = 360;
};

#endif /*SERVOCHANNEL_H*/
