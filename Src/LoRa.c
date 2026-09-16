/**
 * \file LoRa.c
 * \brief
 * \author: Roman Garanin
 * \copyright (c)
 * \data Mar 14, 2025
 */



#include "LoRa.h"
#include "lib/ideetron/Encrypt_V31.h"

#include <assert.h>
#include <string.h>

#define APP_KEY 0x5f, 0x06, 0x0b, 0x48, \
                0x31, 0xb5, 0xdb, 0x3a, \
                0xfc, 0xbc, 0xe0, 0xed, \
                0x1c, 0x5a, 0x03, 0x7e

#define NET_KEY 0x4b, 0xed, 0x83, 0x59, \
                0x54, 0x78, 0x91, 0x40, \
                0xd5, 0x4b, 0xb8, 0xe3, \
                0xe7, 0x7a, 0xa1, 0xff

struct lora_s lora = {
		.config = {0},
		.state = {
			.adr_ack_counter = 0,
			.tx_frame_count = 0,
			.rx_frame_count = 0,
			.uplink_acked = false,
		}
};

struct lora_config_s default_config = {
		.magic                         = LORA_CONFIG_MAGIC,
		.rx1_delay                     = 0,
		.power                         = 20,
		.power_index                   = 0,
		.SF                            = 0xC0,
		.SF_index                      = 0,
		.RX2_SF                        = 0xC0,
		.BW                            = 0x70,
		.CR                            = 0x02,
		.rx2_freq                      = 868300000,
		.channel_mask                  = 0,
		.cp                            = 0,
		.application_session_key       = { APP_KEY },
		.network_session_key           = { NET_KEY },
		.device_address                = {0},
		.receive_mode                  = LORA_RECEIVE_MODE_NONE,
		.modulation_type               = LORATM,
	    .precision_tick_frequency      = 1000,
	    .precision_tick_drift_ns_per_s = 5000,
		.uplink_confirmed              = false,
		.adr                           = false,
		.adr_ack_limit                 = 10,
		.adr_ack_delay                 = 10
};

/** Frequency plan RU868 */
static uint32_t RU868 [] = {
		/** Default (mandatory) channels.
		 *  10% duty cycle or LBT mode.*/
		//868900000, /**< MultiSF 125 kHz 100 mW */
		//869100000, /**< MultiSF 125 kHz 100 mW */
		/** Additional channels.
		 *  Use within airports (airfields) is prohibited.
		 *  0.1% duty cycle or LBT mode. */
		//864100000, /**< MultiSF 125 kHz 25 mW */
		//864300000, /**< MultiSF 125 kHz 25 mW */
		//864500000, /**< MultiSF 125 kHz 25 mW */
		//864700000, /**< MultiSF 125 kHz 25 mW */
		//864900000, /**< MultiSF 125 kHz 25 mW */
		/** Use within airports (airfields) is prohibited.
		 *  1% duty cycle or LBT mode. */
		866100000, /**< MultiSF 125 kHz 25 mW */
		866300000, /**< MultiSF 125 kHz 25 mW */
		866500000, /**< MultiSF 125 kHz 25 mW */
		866700000, /**< MultiSF 125 kHz 25 mW */
		866900000, /**< MultiSF 125 kHz 25 mW */
		867100000, /**< MultiSF 125 kHz 25 mW */
		867300000, /**< MultiSF 125 kHz 25 mW */
		867500000, /**< MultiSF 125 kHz 25 mW */
		//867700000, /**< MultiSF 125 kHz 25 mW */
		//867900000  /**< MultiSF 125 kHz 25 mW */
};

static uint8_t RU868_power [] = {
		17, // 0 Max EIRP
		15, // 1 Max EIRP – 2dB
		13, // 2 Max EIRP – 4dB
		11, // 3 Max EIRP – 6dB
		9,  // 4 Max EIRP – 8dB
		7,  // 5 Max EIRP – 10dB
		5,  // 6 Max EIRP – 12dB
		3,  // 7 Max EIRP – 14dB
};

static uint8_t RU868_SF [] = {
		0xC0, // SF12
		0xB0, // SF11
		0xA0, // SF10
		0x90, // SF9
		0x80, // SF8
		0x70, // SF7
		0x70, // SF7
		0,    // Not implemented. FSK: 50 kbps
		0,0,0,0,0,0, //RFU
		0 //ignore that field and keep the current parameter values
};

static uint8_t RU868_BW [] = {
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x80, // BW_250
		0,    // Not implemented. FSK: 50 kbps
		0,0,0,0,0,0, //RFU
		0 //ignore that field and keep the current parameter values
};

static uint32_t RU_LoraSTD = 864600000; /** SF7 250 kHz  25 mW*/
static uint32_t RU_FSK =     864800000; /** FSK 250 kHz, 50kbps 25 mW */
static uint32_t RU_RX2 =     869100000; /** SF12 125 kHz 25 mW  */

/** Frequency plan EU863 */
static uint32_t EU863 [] = {
		/** Default (mandatory) channels.*/
		863100000, /**< MultiSF 125 kHz */
		863300000, /**< MultiSF 125 kHz */
		863500000, /**< MultiSF 125 kHz */
		/** Additional channels. */
		867100000,
		867300000,
		867500000,
		867700000,
		867900000
};


static uint32_t EU863_870 [] = {
		/** Default (mandatory) channels.*/
		868100000, /**< MultiSF 125 kHz */
		868300000, /**< MultiSF 125 kHz */
		868500000, /**< MultiSF 125 kHz */
		/** Additional channels. */
		867100000,
		867300000,
		867500000,
		867700000,
		867900000
};

static uint8_t EU863_870_power [] = {
		17, // 0 Max EIRP
		15, // 1 Max EIRP – 2dB
		13, // 2 Max EIRP – 4dB
		11, // 3 Max EIRP – 6dB
		9,  // 4 Max EIRP – 8dB
		7,  // 5 Max EIRP – 10dB
		5,  // 6 Max EIRP – 12dB
		3,  // 7 Max EIRP – 14dB
};

static uint8_t EU863_870_SF [] = {
		0xC0, // SF12
		0xB0, // SF11
		0xA0, // SF10
		0x90, // SF9
		0x80, // SF8
		0x70, // SF7
		0x70, // SF7
		0,    // Not implemented. FSK: 50 kbps
		0,0,0,0,0,0, //RFU
		0 //ignore that field and keep the current parameter values
};

static uint8_t EU863_870_BW [] = {
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x70, // BW_125
		0x80, // BW_250
		0,    // Not implemented. FSK: 50 kbps
		0,0,0,0,0,0, //RFU
		0 //ignore that field and keep the current parameter values
};

static bool lora_send();
static bool lora_receive();
static void select_random_channel();
static void lora_set_channel(struct lora_s *lora, uint8_t channel_index, uint32_t frequency);
static void lora_set_dr(uint8_t dr_pwr);
static void calculate_rx_timings(struct lora_s *lora,
		                         uint32_t bw,
								 uint8_t sf,
								 uint32_t tx_ticks,
                                 uint32_t *rx_target,
								 uint32_t *rx_window_symbols);

static void encode_phy_payload(struct lora_s *lora,
                                 uint8_t payload_buf[64],
                                 const uint8_t *frame_payload,
                                 size_t frame_payload_length,
                                 uint8_t port);

static bool decode_phy_payload(struct lora_s *lora,
                               uint8_t payload_buf[64],
                               uint8_t payload_length,
                               uint8_t **decoded_frame_payload_ptr,
                               uint8_t *decoded_frame_payload_length,
                               uint8_t *frame_port);

static bool process_mac_commands(const uint8_t *frame_payload,
                                 size_t frame_payload_length,
								 uint8_t answer_buffer[51],
								 uint8_t *answer_buffer_length,
                                 int8_t snr);

static void config_load_default();

void lora_init (uint8_t  (*pp_random_int)(uint8_t max),
				uint8_t  (*get_battery_level)(),
		        uint32_t (*pp_get_precision_tick)(),
				void     (*pp_save_config)(const struct lora_config_s *config),
				bool     (*pp_reload_config)(struct lora_config_s *config),
				void     (*pp_sleep_until)(uint32_t ms),
				void     (*pp_save_tx_counter)(const uint16_t *count),
				bool     (*pp_reload_tx_counter)(uint16_t *count),
				void     (*pp_save_rx_counter)(const uint16_t *count),
				bool     (*pp_reload_rx_counter)(uint16_t *count),
				void     (*pp_save_adr_ack_counter)(const uint16_t *count),
				bool     (*pp_reload_adr_ack_counter)(uint16_t *count),
				bool     (*pp_modem_set_freq)(uint32_t),
				bool     (*pp_modem_set_power)(int8_t power),
				bool     (*pp_modem_send)(uint8_t *payload_buf,
								      uint8_t sf,
									  uint8_t bw,
									  uint8_t cr,
				                      uint32_t payload_len,
				                      uint32_t frequency,
				                      uint32_t *tx_ticks),

			    bool     (*pp_modem_receive)(uint32_t rx_target,
						                 uint32_t rx_window_symbols,
										 uint8_t sf,
										 uint8_t bw,
										 uint8_t cr,
										 uint8_t *payload_buf,
										 uint32_t *payload_len,
				                         int8_t *snr),

				bool     (*pp_modem_send_fsk) (uint8_t *payload_buf,
					                           uint32_t frequency,
					                           uint32_t payload_len,
							                   uint32_t *tx_ticks),

				bool     (*pp_modem_receive_fsk)(uint8_t *payload_buf,
							                     uint32_t *payload_len))
{
	lora.random_int            = pp_random_int;
    lora.get_precision_tick    = pp_get_precision_tick;
    lora.save_config           = pp_save_config;
    lora.reload_config         = pp_reload_config;
    lora.precision_sleep_until = pp_sleep_until;

    lora.modem_set_freq        = pp_modem_set_freq;
    lora.modem_set_power       = pp_modem_set_power;
    lora.modem_send            = pp_modem_send;
    lora.modem_receive         = pp_modem_receive;
    lora.modem_send_fsk        = pp_modem_send_fsk;
    lora.modem_receive_fsk     = pp_modem_receive_fsk;

	lora.state.save_tx_count   = pp_save_tx_counter;
	lora.state.reload_tx_count = pp_reload_tx_counter;
	lora.state.save_rx_count   = pp_save_rx_counter;
	lora.state.reload_rx_count = pp_reload_rx_counter;

	lora.state.save_adr_ack_counter  = pp_save_adr_ack_counter;
	lora.state.reload_adr_ack_counter = pp_reload_adr_ack_counter;

	// If there is reload function or the reload was unsuccessful or the magic does not match restore default.
	if (lora.reload_config == NULL ||
	    !lora.reload_config(&lora.config) ||
	    lora.config.magic != LORA_CONFIG_MAGIC) {
		config_load_default();
	}

	lora_set_channel_plan();

	if ( lora.state.reload_tx_count ) {
		lora.state.reload_tx_count (&(lora.state.tx_frame_count));
	}

	if ( lora.state.reload_rx_count ) {
		lora.state.reload_rx_count (&(lora.state.rx_frame_count));
	}

	if ( lora.state.reload_adr_ack_counter) {
		lora.state.reload_adr_ack_counter (&(lora.state.adr_ack_counter));
	}
}

bool lora_send_receive_cycle(const uint8_t *send_data, uint32_t send_data_length)
{

	// Clear phy payload buffer to reuse for the down-link message.
	memset(lora.phy_payload_buf, 0, sizeof(lora.phy_payload_buf));
	lora.phy_payload_len = 0;

	// Build the up-link phy payload.
	encode_phy_payload(&lora, lora.phy_payload_buf, send_data, send_data_length, 1);
	select_random_channel();

	if ( !lora_send() ) {
		return false;
	}

	// Only receive if configured to do so.
	if (lora.config.receive_mode != LORA_RECEIVE_MODE_NONE) {

		// Try receiving a down-link.
		if ( !lora_receive() ) {
			//Put modem to sleep here.
/*
 *	If no Class A downlink frame is received within the ext ADR_ACK_DELAY uplinks
 *	(i.e., after a total of ADR_ACK_LIMIT + ADR_ACK_DELAY transmitted frames),
 *	the end-device SHALL try to regain connectivity by first setting the TX power to the default power,
 *	then switching to the next lower data rate that provides a longer radio range.
 *	The end-device SHALL further lower its data rate step by step every time ADR_ACK_DELAY uplink frames are transmitted.
 *	Once the end-device has reached the default data rate,
 *	and transmitted for ADR_ACK_DELAY uplinks with ADRACKReq=1 without receiving a downlink,
 *	it SHALL re-enable all default uplink frequency channels and reset NbTrans to its default value of 1.
 */
			if ( lora.state.adr_ack_counter >= lora.config.adr_ack_limit + lora.config.adr_ack_delay) {

				if (lora.config.SF_index == 0  ) {
					lora.state.adr_ack_counter = lora.state.adr_ack_counter - lora.config.adr_ack_delay;
					if ( lora.state.save_adr_ack_counter ) {
						{
							lora.state.save_adr_ack_counter(&(lora.state.adr_ack_counter));
						}
					}

					return false;
				}

				/* Set default power */
				lora.config.power_index = 0;
				lora_set_dr((lora.config.power_index & 0x0F) | (lora.config.SF_index << 4));
				lora.config.power = 20;
				lora.modem_set_power(lora.config.power );
				lora.config.SF_index--;

				lora.state.adr_ack_counter = lora.state.adr_ack_counter - lora.config.adr_ack_delay;
				if ( lora.state.save_adr_ack_counter ) {
					{
						lora.state.save_adr_ack_counter(&(lora.state.adr_ack_counter));
					}
				}

			}
			if (NULL != lora.save_config){
				lora.save_config(&lora.config);
			}
			return false;
		}

		// Any RX payload was received.
		if (lora.phy_payload_len != 0) {

			uint8_t *frame_payload;
			uint8_t frame_payload_len = 0;
			uint8_t frame_port;

			// Try decoding the frame payload.
			if (decode_phy_payload(&lora,
					               lora.phy_payload_buf,
					               lora.phy_payload_len,
								   &frame_payload,
								   &frame_payload_len,
			                       &frame_port)) {

				// Process Mac Commands
				if (frame_port == 0) {

					uint8_t mac_response_data[51] = {0};
					uint8_t mac_response_len = 0;

					if ( process_mac_commands(frame_payload,
											  frame_payload_len,
											  mac_response_data,
					                          &mac_response_len,
											  lora.state.snr)
							                  &&
											  mac_response_len != 0 ) {
						// Build the up-link phy payload.
						bool tmp_confirmed = lora.config.uplink_confirmed;
						bool tmp_ack = lora.state.uplink_acked;
						lora.config.uplink_confirmed = false;
						encode_phy_payload(&lora,
								           lora.phy_payload_buf,
										   mac_response_data,
						                   mac_response_len,
										   0);
						if ( !lora_send() ) {
							return false;
						}
						lora.config.uplink_confirmed = tmp_confirmed;
						lora.state.uplink_acked = tmp_ack;
					}

				} else {
					// Don't process application messages for now!
				}
			}
		}
	}
	/* A Class A downlink frame received following an uplink frame SHALL reset the ADRACKCnt counter.
	 * Upon receipt of any Class A downlink, the end-device SHALL clear the ADRACKReq bit.
	 */
	if ( lora.state.adr_ack_counter >= lora.config.adr_ack_limit ) {
		lora.state.adr_ack_counter = 0;
		if (NULL != lora.state.save_adr_ack_counter) {
			lora.state.save_adr_ack_counter(&(lora.state.adr_ack_counter));
		}
	}
	return true;
}


static void calculate_rx_timings(struct lora_s *lora,
		                         uint32_t bw,
								 uint8_t sf,
								 uint32_t tx_ticks,
                                 uint32_t *rx_target,
								 uint32_t *rx_window_symbols)
{
	volatile int32_t symbol_rate_ns = (int32_t)(((2 << (sf - 1)) * 1000000) / bw);

	volatile int32_t rx_timing_error_ns = (int32_t)(lora->config.precision_tick_drift_ns_per_s * lora->config.rx1_delay);
	volatile int32_t rx_window_ns = 2 * symbol_rate_ns + 2 * rx_timing_error_ns;
	volatile int32_t rx_offset_ns = 4 * symbol_rate_ns - (rx_timing_error_ns / 2);
	volatile int32_t rx_offset_ticks = (int32_t)(((int64_t)rx_offset_ns * (int64_t)lora->config.precision_tick_frequency) / 1000000);
	//*rx_target = tx_ticks + handle->precision_tick_frequency * handle->config.rx1_delay + rx_offset_ticks;
	*rx_target = lora->config.precision_tick_frequency * lora->config.rx1_delay + rx_offset_ticks;
	*rx_window_symbols = rx_window_ns / symbol_rate_ns;
}

static void encode_phy_payload(struct lora_s *lora,
                                 uint8_t payload_buf[64],
                                 const uint8_t *frame_payload,
                                 size_t frame_payload_length,
                                 uint8_t port)
{
	size_t payload_len = 0;

	// 64 bytes is maximum size of FIFO
	assert(frame_payload_length + 4 + 9 <= 64);

	if ( (true  == lora->config.uplink_confirmed) && (lora->config.receive_mode != LORA_RECEIVE_MODE_NONE) ) {
		payload_buf[0] = 0x80; // MAC Header Uplink confirmed
	} else {
		payload_buf[0] = 0x40; // MAC Header Uplink uconfirmed
	}
	payload_buf[1] = lora->config.device_address[3];
	payload_buf[2] = lora->config.device_address[2];
	payload_buf[3] = lora->config.device_address[1];
	payload_buf[4] = lora->config.device_address[0];
	if ( true == lora->config.adr) {
		payload_buf[5] = 0x80;  // ADR Enable
		if (lora->state.adr_ack_counter >= lora->config.adr_ack_limit) { // Set ADRACKReq.
			payload_buf[5] |= (0x01 << 6);
		}
	} else {
		payload_buf[5] = 0x00;  //
	}
	payload_buf[6] = (lora->state.tx_frame_count & 0x00ffu);
	payload_buf[7] = ((uint16_t)(lora->state.tx_frame_count >> 8u) & 0x00ffu);
	payload_buf[8] = port; // Frame Port
	payload_len += 9;

	// Encrypt payload in place in payload_buf.
	memcpy(payload_buf + payload_len, frame_payload, frame_payload_length);
	if (port == 0) {
		Encrypt_Payload(payload_buf + payload_len,
				        frame_payload_length,
						lora->state.tx_frame_count,
		                0,
						lora->config.network_session_key,
						lora->config.device_address);
	} else {
		Encrypt_Payload(payload_buf + payload_len,
                        frame_payload_length,
						lora->state.tx_frame_count,
		                0,
						lora->config.application_session_key,
						lora->config.device_address);
	}
	payload_len += frame_payload_length;

	// Calculate MIC and copy to last 4 bytes of the payload_buf.
	uint8_t mic[4];
	Calculate_MIC(payload_buf,
			      mic,
				  payload_len,
				  lora->state.tx_frame_count,
				  0,
	              lora->config.network_session_key,
				  lora->config.device_address);
	for (uint8_t i = 0; i < 4; i++) {
		payload_buf[payload_len + i] = mic[i];
	}
	payload_len += 4;

	lora->phy_payload_len =  payload_len;
}

static bool decode_phy_payload(struct lora_s *lora,
                               uint8_t payload_buf[64],
                               uint8_t payload_length,
                               uint8_t **decoded_frame_payload_ptr,
                               uint8_t *decoded_frame_payload_length,
                               uint8_t *frame_port)
{
	// Only unconfirmed down-links are supported for now.
	if (payload_buf[0] != 0x60) {
		return false;
	}

	// Does the device address match?
	if (payload_buf[1] != lora->config.device_address[3] ||
		payload_buf[2] != lora->config.device_address[2] ||
	    payload_buf[3] != lora->config.device_address[1] ||
		payload_buf[4] != lora->config.device_address[0]) {
		return false;
	}

	uint8_t frame_control = payload_buf[5];

	if ( true == lora->config.uplink_confirmed ) {
		if ( 0xA0 &  frame_control) {
			lora->state.uplink_acked = true;
		}
	}

	uint8_t frame_opts_length = frame_control & 0x0f;
	uint16_t rx_frame_count = (payload_buf[7] << 8) | payload_buf[6];

	// Check if rx frame count is valid and if so, update accordingly.
	if (rx_frame_count < lora->state.rx_frame_count) {
		return false;
	}

	lora->state.rx_frame_count = rx_frame_count;

	uint8_t check_mic[4];

	Calculate_MIC(payload_buf,
                  check_mic,
                  payload_length - 4,
                  rx_frame_count, 1,
                  lora->config.network_session_key,
                  lora->config.device_address);

	if (memcmp(check_mic, &payload_buf[payload_length - 4], 4) != 0) {
		return false;
	}

	if (payload_length - 12 - frame_opts_length == 0) {
		*frame_port = 0;
		*decoded_frame_payload_ptr = &payload_buf[8];
		*decoded_frame_payload_length = frame_opts_length;

	} else {
		*frame_port = payload_buf[8];

		uint8_t frame_payload_start = 9 + frame_opts_length;
		uint8_t frame_payload_end = payload_length - 4;
		uint8_t frame_payload_length = frame_payload_end - frame_payload_start;

		if (*frame_port == 0) {
			Encrypt_Payload(&payload_buf[frame_payload_start],
                            frame_payload_length,
                            rx_frame_count,
                            1,
                            lora->config.network_session_key,
                            lora->config.device_address);
		} else {
			Encrypt_Payload(&payload_buf[frame_payload_start],
                            frame_payload_length,
                            rx_frame_count,
                            1,
                            lora->config.application_session_key,
                            lora->config.device_address);
		}

		*decoded_frame_payload_ptr = &payload_buf[frame_payload_start];
		*decoded_frame_payload_length = frame_payload_length;
	}

	return true;
}

static bool lora_send()
{
	// Send the requested up-link.

#ifdef DEBUG
	lora.debug_printf("LoRa:\r\n"
			          "\tDevice address: %08lx\r\n"
	        		  "\tFreq: %d Hz\r\n"
	                  "\tTx Frame count:%ld\r\n"
	                  "\tTx Frame size: %d\r\n"
			          "\tTx pwr: %d\r\n"
			          "\tSF: %d\r\n"
					  "\tADR_ACK_Cnt: %d\r\n"

	                  ,(uint32_t)(lora.config.device_address[3]       |
	                              lora.config.device_address[2] << 8  |
	                              lora.config.device_address[1] << 16 |
	                              lora.config.device_address[0] << 24 )
	    			  ,lora.freq
	                  ,lora.state.tx_frame_count
	                  ,lora.phy_payload_len
					  ,lora.config.power
					  ,lora.config.SF>>4
					  ,lora.state.adr_ack_counter);
#endif

	if (!lora.modem_send(lora.phy_payload_buf,
			             lora.config.SF,
						 lora.config.BW,
						 lora.config.CR,
						 lora.phy_payload_len,
						 lora.freq,
						 &lora.state.tx_ticks)) {
		return false;
	} else {
		lora.state.tx_frame_count++;
		if ( lora.state.save_tx_count ) {
			lora.state.save_tx_count(&(lora.state.tx_frame_count));
		}
		lora.state.adr_ack_counter++;
		if ( lora.state.save_adr_ack_counter ) {
			lora.state.save_adr_ack_counter(&(lora.state.adr_ack_counter));
		}
	}
	lora.state.uplink_acked = false;
	return true;
}

static bool lora_receive()
{
	// Clear phy payload buffer to reuse for the down-link message.
	memset(lora.phy_payload_buf, 0, sizeof(lora.phy_payload_buf));
	lora.phy_payload_len = 0;
	uint32_t rx1_target, rx1_window_symbols;
	calculate_rx_timings(&lora, 125000, (lora.config.SF >> 4), lora.state.tx_ticks, &rx1_target, &rx1_window_symbols);
	//rx1_window_symbols = 0xFF;
	assert(rx1_window_symbols <= 0x3ff);

	bool received = lora.modem_receive(rx1_target,
			                           rx1_window_symbols,
									   lora.config.SF,
									   lora.config.BW,
									   lora.config.CR,
									   lora.phy_payload_buf,
									   &lora.phy_payload_len,
									   &lora.state.snr);
	if ( true == received ) {
		if (lora.state.save_rx_count) {
			lora.state.save_rx_count(&lora.state.rx_frame_count);
		}
		return true;
	}

	//Set freq for receive in rx2 869525000 and sf 12

	uint32_t rx2_target, rx2_window_symbols;
	if ( !lora.modem_set_freq(lora.config.rx2_freq)) {
		return false;
	}
	calculate_rx_timings(&lora, 125000, (lora.config.SF >> 4), lora.state.tx_ticks, &rx2_target, &rx2_window_symbols);
	received = lora.modem_receive(rx2_target,
			                      rx2_window_symbols,
					              lora.config.RX2_SF,
					              lora.config.BW,
					              lora.config.CR,
					              lora.phy_payload_buf,
					              &lora.phy_payload_len,
					              &lora.state.snr);
	if ( true == received ) {
		lora.state.rx_frame_count++;
		if (lora.state.save_rx_count) {
			lora.state.save_rx_count(&lora.state.rx_frame_count);
		}
		return true;
	} else {
		return false;
	}

}

static void  select_random_channel()
{
	uint8_t channel_count = 0;

	for (uint8_t i = 0; i < 16; i++) {
		if (lora.config.channel_mask & (1 << i)) {
			channel_count++;
		}
	}

	uint8_t random_channel = lora.random_int(channel_count);

	for (uint8_t i = 0; i < 16; i++) {
		if (lora.config.channel_mask & (1 << i)) {
			if (random_channel == 0) {
				lora.state.lora_random_ch = i;
				lora.freq = lora.config.channels[lora.state.lora_random_ch];
				return;
			} else {
				random_channel--;
			}
		}
	}
	lora.state.lora_random_ch = 0;
	lora.freq = lora.config.channels[lora.state.lora_random_ch];
	return;
}

static void lora_set_channel(struct lora_s *lora, uint8_t channel_index, uint32_t frequency)
{
	assert(channel_index < 24);
	lora->config.channels[channel_index] = frequency;
	lora->config.channel_mask |= (1 << channel_index);
}

static void lora_set_dr(uint8_t dr_pwr)
{
	lora.config.power_index = dr_pwr & 0x0F;
	lora.config.SF_index = dr_pwr >> 4;
	if ( lora.config.power_index == 0x0F && lora.config.SF_index == 0x0F ) {
		return;
	}
	switch (lora.config.cp)
	{
	case 0: //RU
		lora.modem_set_power(RU868_power[lora.config.power_index]);
		lora.config.power = RU868_power[lora.config.power_index];
		lora.config.SF = RU868_SF[lora.config.SF_index];
		lora.config.BW = RU868_BW[lora.config.SF_index];
		break;
	case 1: //EN
		lora.modem_set_power(EU863_870_power[lora.config.power_index]);
		lora.config.power = EU863_870_power[lora.config.power_index];
		lora.config.SF = EU863_870_SF[lora.config.SF_index];
		lora.config.BW = EU863_870_BW[lora.config.SF_index];
		break;
	}
}

static bool process_mac_commands(const uint8_t *frame_payload,
                                 size_t frame_payload_length,
								 uint8_t answer_buffer[51],
								 uint8_t *answer_buffer_length,
                                 int8_t snr)
{
	uint8_t index = 0;
	uint8_t answer_index = 0;

	while (index < frame_payload_length) {
		switch (frame_payload[index++])
		{
			case 0x01: // ResetConf
			{
				if (index > frame_payload_length) {

					return false;
				}

				index += 1;
				break;
			}
			case 0x02: // LinkCheckReq
			{
				if ((index + 1) > frame_payload_length) {
					return false;
				}

				index += 2;
				break;
			}
			case 0x03: // LinkADRReq
			{
				if ((index + 3) > frame_payload_length) {
					return false;
				}

				lora_set_dr(frame_payload[index++]);

				uint32_t mask;
				mask &= (0xFFFF & (uint32_t)frame_payload[index++]);
				mask &= (0xFFFF & ((uint32_t)frame_payload[index++]<<8));

				uint8_t redundency = frame_payload[index++];

				uint8_t nm_trans = redundency & 0x0F;
				uint8_t mask_ctrl = (redundency & 0x70) >> 4;

				if (mask_ctrl == 0){
					lora.config.channel_mask = mask;
				}

				answer_buffer[answer_index++] = 0x03;
				answer_buffer[answer_index++] = 0x07;

				break;
			}
			case 0x04: // DutyCycleReq
			{
				if (index >= frame_payload_length) {
					return false;
				}

				index += 1;
				break;
			}
			case 0x05: // RXParamSetupReq
			{
				if ((index + 4) > frame_payload_length) {
					return false;
				}
				if ((answer_index + 2) > 51) {
					return false;
				}

				uint8_t dl_settings = frame_payload[index++];
				uint8_t frequency_lsb = frame_payload[index++];
				uint8_t frequency_msb = frame_payload[index++];
				uint8_t frequency_hsb = frame_payload[index++];
				uint32_t frequency = (frequency_lsb | (frequency_msb << 8) | (frequency_hsb << 16)) * 100;

				lora.config.rx2_freq = frequency;

				answer_buffer[answer_index++] = 0x05;
				answer_buffer[answer_index++] = 0b0000111;
				break;
			}
			case 0x06: // DevStatusReq
			{
				if ((answer_index + 3) > 51) {
					return false;
				}

				uint8_t margin = (uint8_t)(snr & 0x1f);
				uint8_t battery_level = lora.get_battery_level == NULL ? 0xff : lora.get_battery_level();

				answer_buffer[answer_index++] = 0x06;
				answer_buffer[answer_index++] = battery_level;
				answer_buffer[answer_index++] = margin;
				break;
			}
			case 0x07: // NewChannelReq
			{
				if ((index + 4) > frame_payload_length) {
					return false;
				}
				if ((answer_index + 2) > 51) {
					return false;
				}

				uint8_t channel_index = frame_payload[index++];
				uint8_t frequency_lsb = frame_payload[index++];
				uint8_t frequency_msb = frame_payload[index++];
				uint8_t frequency_hsb = frame_payload[index++];
				uint8_t min_max_dr = frame_payload[index++];

				uint32_t frequency = (frequency_lsb | (frequency_msb << 8) | (frequency_hsb << 16)) * 100;
				uint8_t min_dr = min_max_dr & 0x0f;
				uint8_t max_dr = (min_max_dr >> 4) & 0x0f;

				if (channel_index >= 3) {
					lora_set_channel(&lora, channel_index, frequency);
				}

				bool dr_supports_125kHz_SF7 = min_dr <= 5 || max_dr >= 5;

				answer_buffer[answer_index++] = 0x07;
				answer_buffer[answer_index++] = 0x01 | (dr_supports_125kHz_SF7 << 1);
				break;
			}
			case 0x08: // RXTimingSetupReq
			{
				if (index >= frame_payload_length) {
					return false;
				}
				if ((answer_index + 2) > 51) {
					return false;
				}

				lora.config.rx1_delay = frame_payload[index++] & 0xf;
				if (lora.config.rx1_delay == 0) {
					lora.config.rx1_delay = 1;
				}

				answer_buffer[answer_index++] = 0x08;
				break;
			}
			case 0x09: // TxParamSetupReq
			{
				if (index >= frame_payload_length) {
					return false;
				}

				break;
			}
			case 0x0a: // DlChannelReq
			{
				if ((index + 4) > frame_payload_length) {
					return false;
				}

				break;
			}
			case 0x0b: // RekeyConf
			{
				if (index >= frame_payload_length) {
					return false;
				}

				break;
			}
			case 0x0c: // ADRParamSetupReq
			{
				if (index >= frame_payload_length) {
					return false;
				}

				break;
			}
			case 0x0d: // DeviceTimeReq
			{
				break;
			}
		}
	}
	*answer_buffer_length = answer_index;
	lora.save_config(&lora.config);
	return true;
}

void  lora_set_channel_plan()
{
	lora.config.channel_mask = 0;

	memset(lora.config.channels, 0, sizeof(uint32_t) * 24);
	switch (lora.config.cp)
	{
	case 0:
		for (uint16_t i = 0; i < sizeof (RU868) / sizeof(uint32_t); i++)
		{
			lora_set_channel(&lora, i, RU868[i]);
		}
		break;
	case 1:
		for (uint16_t i = 0; i < sizeof (EU863_870) / sizeof(uint32_t); i++)
		{
			lora_set_channel(&lora, i, EU863_870[i]);
		}
		break;
	}
}

static void config_load_default()
{
	lora.config.magic                         = default_config.magic;
	lora.config.rx1_delay                     = default_config.rx1_delay;
	lora.config.power                         = default_config.power;
	lora.config.power_index                   = default_config.power_index;
	lora.config.channel_mask                  = default_config.channel_mask;
	lora.config.SF                            = default_config.SF;
	lora.config.SF_index                      = default_config.SF_index;
	lora.config.rx1_delay                     = default_config.rx1_delay;
	lora.config.rx2_freq                      = default_config.rx2_freq;
	lora.config.RX2_SF                        = default_config.RX2_SF;
	lora.config.BW                            = default_config.BW;
	lora.config.CR                            = default_config.CR;
	lora.config.cp                            = default_config.cp;
	lora.config.receive_mode                  = default_config.receive_mode;
    lora.config.precision_tick_frequency      = default_config.precision_tick_frequency;
    lora.config.precision_tick_drift_ns_per_s = default_config.precision_tick_drift_ns_per_s;
	lora.config.uplink_confirmed              = default_config.uplink_confirmed;
	lora.config.adr                           = default_config.adr;
	lora.config.adr_ack_delay                 = default_config.adr_ack_delay;
	lora.config.adr_ack_limit                 = default_config.adr_ack_limit;
	lora.config.interval                      = default_config.interval;


	memcpy(lora.config.application_session_key,
		   default_config.application_session_key,
		   sizeof(default_config.application_session_key));
	memcpy(lora.config.network_session_key,
		   default_config.network_session_key,
		   sizeof(default_config.network_session_key));
	memcpy(lora.config.device_address,
		   default_config.device_address,
		   sizeof(default_config.device_address));

	if (NULL != lora.save_config){
		lora.save_config(&lora.config);
	}

}

