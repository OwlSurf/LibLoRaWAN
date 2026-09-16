/**
 * \file lora_rfm95.c
 * \brief Modem callbacks for LibLoRaWAN on top of the OwlSurf/rfm95 chip API.
 *
 * LoRaWAN PHY: sync word 0x34, TX IQ normal, RX IQ inverted (Semtech AN1200.24).
 */

#include "lora_rfm95.h"
#include "LoRa.h"

#define LORA_RFM95_SYNC_WORD 0x34u

static rfm95_handle_t *s_rfm95;

void lora_rfm95_attach(rfm95_handle_t *handle)
{
	s_rfm95 = handle;

	lora.modem_set_freq = lora_rfm95_set_freq;
	lora.modem_set_power = lora_rfm95_set_power;
	lora.modem_send = lora_rfm95_send;
	lora.modem_receive = lora_rfm95_receive;
	lora.modem_send_fsk = lora_rfm95_send_fsk;
	lora.modem_receive_fsk = lora_rfm95_receive_fsk;

	if (s_rfm95 != NULL) {
		(void)rfm95_set_sync_word(s_rfm95, LORA_RFM95_SYNC_WORD);
	}
}

bool lora_rfm95_set_freq(uint32_t freq)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	return rfm95_set_frequency(s_rfm95, freq);
}

bool lora_rfm95_set_power(int8_t power)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	return rfm95_set_power(s_rfm95, power);
}

bool lora_rfm95_send(uint8_t *payload_buf,
                     uint8_t sf,
                     uint8_t bw,
                     uint8_t cr,
                     uint32_t payload_len,
                     uint32_t frequency,
                     uint32_t *tx_ticks)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	if (!rfm95_set_sync_word(s_rfm95, LORA_RFM95_SYNC_WORD)) {
		return false;
	}
	/* TX IQ normal (not inverted). */
	if (!rfm95_set_iq_inverted(s_rfm95, false)) {
		return false;
	}
	return rfm95_send(s_rfm95, payload_buf, sf, bw, cr, payload_len, frequency, tx_ticks);
}

bool lora_rfm95_receive(uint32_t rx_target,
                        uint32_t rx_window_symbols,
                        uint8_t sf,
                        uint8_t bw,
                        uint8_t cr,
                        uint8_t *payload_buf,
                        uint32_t *payload_len,
                        int8_t *snr)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	if (!rfm95_set_sync_word(s_rfm95, LORA_RFM95_SYNC_WORD)) {
		return false;
	}
	/* RX invert IQ for LoRaWAN downlink. */
	if (!rfm95_set_iq_inverted(s_rfm95, true)) {
		return false;
	}
	return rfm95_receive(s_rfm95, rx_target, rx_window_symbols, sf, bw, cr,
	                     payload_buf, payload_len, snr);
}

bool lora_rfm95_send_fsk(uint8_t *payload_buf,
                         uint32_t frequency,
                         uint32_t payload_len,
                         uint32_t *tx_ticks)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	return rfm95_send_fsk(s_rfm95, payload_buf, frequency, payload_len, tx_ticks);
}

bool lora_rfm95_receive_fsk(uint8_t *payload_buf, uint32_t *payload_len)
{
	if (s_rfm95 == NULL) {
		return false;
	}
	return rfm95_receive_fsk(s_rfm95, payload_buf, payload_len);
}
