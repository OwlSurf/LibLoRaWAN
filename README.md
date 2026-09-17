# LibLoRaWAN

LoRaWAN MAC for end devices. Radio PHY is provided by the host project via modem function pointers — this library has no built-in chip driver (RFM95, SX126x, etc.).

## Layout

| Path | Role |
| --- | --- |
| `Inc/LoRa.h`, `Src/LoRa.c` | LoRaWAN MAC |
| `lib/ideetron/` | Ideetron AES-128 + LoRaWAN encrypt/MIC (`Encrypt_V31.h` is included by `LoRa.c`) |

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Modem integration

Modem callbacks default to stubs that return `false`. Bind your chip driver after (or before) `lora_init()`:

```c
lora_init(/* platform callbacks… */);
lora_modem_init(my_set_freq,
                my_set_power,
                my_send,
                my_receive,
                my_send_fsk,
                my_receive_fsk);
```

Pass `NULL` for any slot to leave the stub in place. Typedefs (`lora_modem_set_freq_fn`, …) match the signatures expected by the MAC.

Expected PHY behaviour for LoRaWAN (implement in your driver):

- Sync word `0x34`
- TX IQ normal
- RX IQ inverted

## License

- MAC: project license / original LoRa sources
- `lib/ideetron/`: GNU LGPL v3 or later (Ideetron B.V.), see `lib/ideetron/LICENSE`
