/**
 * \file LoRa.h
 * \brief
 * \author: Roman Garanin
 * \copyright (c)
 * \data Mar 14, 2025
 */
#ifndef INC_LORA_H_
#define INC_LORA_H_

#define _CRT_SECURE_NO_WARNINGS
#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"


#define LORA_CONFIG_MAGIC 0xab67

struct lora_state_s {

	uint32_t tx_ticks;

	/**
	 * The current RX frame counter value.
	 */
	uint16_t rx_frame_count;

	/**
	 * The current TX frame counter value.
	 */
	uint16_t tx_frame_count;

	/**
	 * ADR ACK counter
	 */
	uint16_t adr_ack_counter;
	/**
	 * The reload tx counter function pointer can be used to load tx counter values from non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	bool (*reload_tx_count)(uint16_t *count);

	/**
	 * The save tx counter  function pointer can be used to store tx count values in non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	void (*save_tx_count)(const uint16_t *count);

	/**
	 * The reload rx counter function pointer can be used to load rx counter values from non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	bool (*reload_rx_count)(uint16_t *count);

	/**
	 * The save rx counter  function pointer can be used to store rx count values in non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	void (*save_rx_count)(const uint16_t *count);

	/**
	 * The reload ADR ACK counter function pointer can be used to load ADR ACK counter values from non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	bool (*reload_adr_ack_counter)(uint16_t *count);

	/**
	 * The save ADR ACK counter function pointer can be used to store ADR ACK counter values in non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	void (*save_adr_ack_counter)(const uint16_t *count);

	/**
	 * Current random channel.
	 */
	uint8_t lora_random_ch;

	/**
	 * SNR.
	 */
	int8_t snr;

	/**
	 * Uplink acknowledged.
	 */
	bool uplink_acked; /**< */
};

typedef enum
{
	LORA_RECEIVE_MODE_NONE,
	LORA_RECEIVE_MODE_RX1_ONLY,
	LORA_RECEIVE_MODE_RX12,
} lora_receive_mode_t;

typedef enum
{
	LORATM,
	FSK
}lora_modulation_t;

struct lora_config_s {

	/**
	 * MAGIC
	 */
	uint16_t magic;
	/**
	 * The delay to the RX1 window.
	 */
	uint8_t rx1_delay;

	/**
	 * The power of transmission.
	 */
	int8_t power;

	/**
	 * The index of the power.
	 */
	uint8_t power_index;

	/**
	 * Spreading factor.
	 */
	uint8_t SF;

	/**
	 * The index of the spreading factor.
	 */
	uint8_t SF_index;

	/**
	 * Rx2 Spreading factor.
	 */
	uint8_t RX2_SF;

	/**
	 * BW
	 */
	uint8_t BW;

	/**
	 * Code rate (CR)
	 */
	uint8_t CR;

	/**
	 * Channel plan.
	 */
	uint8_t cp;

	/**
	 * The device address for the LoraWAN
	 */
	uint8_t device_address[4];

	/**
	 * The network session key for ABP activation with the LoraWAN
	 */
	uint8_t network_session_key[16];

	/**
	 * The application session key for ABP activation with the LoraWAN
	 */
	uint8_t application_session_key[16];

	/**
	 * The configuration of channels;
	 */
	uint32_t channels[24];

	/**
	 * Mask defining which channels are configured.
	 */
	uint32_t channel_mask;

	/**
	 * Rx2 frequency.
	 */
	uint32_t rx2_freq;

	/**
	 * ADR ACK limit.
	 */
	uint32_t adr_ack_limit;

	/**
	 * ADR ACK delay frames.
	 */
	uint32_t adr_ack_delay;

	/**
	 * Uplink frames confirm.
	 */
	bool uplink_confirmed;

	/**
	 * ADR Enable.
	 */
	bool adr;

	/**
	 * The receive mode to operate at.
	 */
	lora_receive_mode_t receive_mode;

    /**
     * The modulation type.
     */
	lora_modulation_t modulation_type;

	/**
	 * The frequency of the precision tick in Hz.
	 */
	uint32_t precision_tick_frequency;

	/**
	 * The +/- timing drift per second in nanoseconds.
	 */
	uint32_t precision_tick_drift_ns_per_s;

	/**
	 * The interval to send Lora packets.
	 */
	uint32_t interval;

};

extern struct lora_config_s default_config;

struct lora_s {

	struct lora_state_s state;

	struct lora_config_s config;
	/**
	 * The load config function pointer can be used to load config values from non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	bool (*reload_config)(struct lora_config_s *config);

	/**
	 * The save config function pointer can be used to store config values in non-volatile memory.
	 * Can be set to NULL to skip.
	 */
	void (*save_config)(const struct lora_config_s *config);

	/**
	 * Function that provides a random integer.
	 */
	uint8_t (*random_int)(uint8_t max);

	/**
	 * Function that returns the device's battery level.
	 */
	uint8_t (*get_battery_level)();

	/**
	 * Function provided that returns a precise tick for timing critical operations.
	 */
	uint32_t (*get_precision_tick)();

	/**
	 * Function that provides a precise sleep until a given tick count is reached.
	 */
	void (*precision_sleep_until)(uint32_t ticks_target);

	bool (*modem_set_freq)(uint32_t freq);

	bool (*modem_set_power)(int8_t power);

	bool (*modem_send)(uint8_t *payload_buf,
					  uint8_t sf,
					  uint8_t bw,
					  uint8_t cr,
	                  uint32_t payload_len,
	                  uint32_t frequency,
	                  uint32_t *tx_ticks);

    bool (*modem_receive)(uint32_t rx_target,
			              uint32_t rx_window_symbols,
					      uint8_t sf,
						  uint8_t bw,
						  uint8_t cr,
					      uint8_t *payload_buf,
						  uint32_t *payload_len,
	                      int8_t *snr);

    bool (*modem_send_fsk) (uint8_t *payload_buf,
    		                uint32_t frequency,
    		                uint32_t payload_len,
    				        uint32_t *tx_ticks);

    bool (*modem_receive_fsk)(uint8_t *payload_buf,
    				          uint32_t *payload_len);

    uint8_t phy_payload_buf[64];
    uint32_t phy_payload_len;
    uint32_t freq;

#ifdef DEBUG
	void (*debug_printf)(const char *format, ...);
#endif
};

extern struct lora_s lora;
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
							                     uint32_t *payload_len));
void lora_set_channel_plan();
bool lora_send_receive_cycle(const uint8_t *send_data, uint32_t send_data_length);

#ifdef __cplusplus
}
#endif
#endif /* INC_LORA_H_ */
