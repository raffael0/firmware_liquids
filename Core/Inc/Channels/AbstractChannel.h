#ifndef ABSTRACTCHANNEL_H
#define ABSTRACTCHANNEL_H

#include <can_houbolt/can_cmds.h>
#include <can_houbolt/cmds.h>

enum class ChannelStatus : int {

};

class AbstractChannel {
	public:
		AbstractChannel(CHANNEL_TYPE t, uint8_t channel_id);
		virtual ~AbstractChannel();

		CHANNEL_TYPE getChannelType() const;
		CHANNEL_STATUS getChannelStatus() const;
		uint8_t getChannelId() const;

		bool IsChannelType(CHANNEL_TYPE t) const;
		bool IsChannelId(uint8_t channel_id) const;

		virtual int init() = 0;
		virtual int reset() = 0;
		virtual int exec() = 0;

		virtual int prcMsg(uint8_t cmd_id, uint8_t *ret_data, uint8_t &ret_n);
		virtual int getSensorData(uint8_t *data, uint8_t &n) = 0;

	protected:
		virtual int setVar(uint8_t variable_id, int32_t data) = 0;
		virtual int getVar(uint8_t variable_id, int32_t *data) const = 0; //TODO: maybe return data param call by ref

	private:
		const CHANNEL_TYPE ch_type;
		const uint8_t ch_id;
		CHANNEL_STATUS ch_status;
};

#endif /*ABSTRACTCHANNEL_H*/
