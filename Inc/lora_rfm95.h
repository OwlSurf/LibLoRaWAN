/**
 * \file lora_rfm95.h
 * \brief LoRaWAN MAC modem callbacks backed by the OwlSurf/rfm95 chip driver.
 */
#ifndef INC_LORA_RFM95_H_
#define INC_LORA_RFM95_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "rfm95.h"

/**
 * Bind the RFM95 chip handle used by the modem callbacks.
 *
 * After this call the global `lora` modem function pointers are set. If you
 * pass the `lora_rfm95_*` callbacks into `lora_init()`, call attach first so
 * the handle is stored; `lora_init()` will then keep those pointers.
 */
void lora_rfm95_attach(rfm95_handle_t *handle);

bool lora_rfm95_set_freq(uint32_t freq);
bool lora_rfm95_set_power(int8_t power);
bool lora_rfm95_send(uint8_t *payload_buf,
                     uint8_t sf,
                     uint8_t bw,
                     uint8_t cr,
                     uint32_t payload_len,
                     uint32_t frequency,
                     uint32_t *tx_ticks);
bool lora_rfm95_receive(uint32_t rx_target,
                        uint32_t rx_window_symbols,
                        uint8_t sf,
                        uint8_t bw,
                        uint8_t cr,
                        uint8_t *payload_buf,
                        uint32_t *payload_len,
                        int8_t *snr);
bool lora_rfm95_send_fsk(uint8_t *payload_buf,
                         uint32_t frequency,
                         uint32_t payload_len,
                         uint32_t *tx_ticks);
bool lora_rfm95_receive_fsk(uint8_t *payload_buf,
                            uint32_t *payload_len);

#ifdef __cplusplus
}
#endif

#endif /* INC_LORA_RFM95_H_ */
