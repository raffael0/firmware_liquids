#include "channel_util.h"
#include "channels.h"
#include "main.h"
void ChannelUtil_DefaultMessageId(Can_MessageId_t *msg_id)
{
	msg_id->info.special_cmd = STANDARD_SPECIAL_CMD;
	msg_id->info.direction = NODE2MASTER_DIRECTION;
	//msg_id->info.direction = MASTER2NODE_DIRECTION; //TODO REMOVE: Just here for debugging
	msg_id->info.node_id = node.node_id;
	msg_id->info.priority = STANDARD_PRIORITY;
}

Result_t ChannelUtil_GetVariable(Channel_t *channel, GetMsg_t *get_msg, uint8_t response_cmd)
{
	Can_MessageId_t message_id =
	{ 0 };
	ChannelUtil_DefaultMessageId(&message_id);

	Can_MessageData_t data =
	{ 0 };

	data.bit.cmd_id = response_cmd;
	data.bit.info.channel_id = channel->id;
	data.bit.info.buffer = DIRECT_BUFFER;

	SetMsg_t *set_msg = (SetMsg_t*) &data.bit.data;
	uint16_t *var;
	switch(channel->type)
	{
		case CHANNEL_TYPE_ADC16:
			Adc16_VariableSelection(&channel->channel.adc16, get_msg->variable_id, channel->id);
			break;
		case CHANNEL_TYPE_ADC16_SINGLE:
			Adc16Single_VariableSelection(&channel->channel.adc16single, get_msg->variable_id, channel->id);
			break;
		case CHANNEL_TYPE_ADC24:
			Adc24_VariableSelection(&channel->channel.adc24, get_msg->variable_id);
			break;
		case CHANNEL_TYPE_DIGITAL_OUT:
			DigitalOut_VariableSelection(&channel->channel.digital_out, get_msg->variable_id, channel->id);
			break;
		case CHANNEL_TYPE_PNEUMATIC_VALVE:
			PneumaticValve_VariableSelection(&channel->channel.pneumatic_valve, get_msg->variable_id, channel->id);
			break;
		default:
			return OOF;
	}

	if (var == NULL)
		return OOF;
	set_msg->variable_id = get_msg->variable_id;
	set_msg->value = *var;

	//TODO @ANDI Move to Debug_PrintInt or something like that
	char serial_str[20] =
	{ 0 };
	//sprintf(serial_str,"%d: %d, ",channel->id, *var);
	sprintf(serial_str, "%d,", *var);
	Serial_PutString(serial_str);

	return Ui_SendCanMessage( MAIN_CAN_BUS, message_id, &data, sizeof(SetMsg_t));
}
