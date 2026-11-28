# RUÍDO — uma MIXTAPE de terror

RPG de exploração de salas e batalhas por turnos que roda em **três placas**:

| Placa | Pasta | Papel |
|---|---|---|
| **PEDAL** (ESP32-S3, Placa E) | `placa_s3_E/` | O controle: 5 botões, encoder EC11, LCD 16x2, OLED, buzzer, LED RGB, DHT11 |
| **AMP** (ESP32-S3, Placa V) | `placa_s3_V/` | O console: roda o jogo, telas 2.0" e 0.96", cartão SD, RTC |
| **CAIXA** (ESP32 DevKit V1) | `placa_devkit/` | Áudio 8 bits e rede (portal de Wi-Fi, hora, site) |

PEDAL e AMP conversam por **ESP-NOW** (o "idioma" fica em `comum/mensagem.h`).
AMP e CAIXA conversam por **fio (UART)**.
O mundo do jogo reage à sala real: hora (RTC), temperatura e umidade (DHT11).

## Estrutura
```
placa_s3_E/
  pedal/pedal.ino          projeto do PEDAL
  teste/                   testes desta placa (t3_placa_e)
placa_s3_V/
  amp/amp.ino              projeto do AMP
  teste/                   t0_sd, t1_placa_v, t2_audio, t3_placa_v_link, t4_mapa_v, t5_calibrar_telas
placa_devkit/
  caixa/                   projeto da CAIXA (começa depois do t6)
  teste/                   t6_audio_8bit
teste_geral/               raio-X de qualquer placa: chip, clock, memória, MACs, versões, energia, Wi-Fi
comum/mensagem.h           Mensagem do rádio, MACs e canal (AMP e PEDAL)
cartao_sd/                 conteúdo do micro SD do AMP (mapas, dados, sprites)
design.md                  documento de design completo (história, regras, telas, pinos)
CREDITOS.md                bibliotecas e materiais de terceiros
```

## Como gravar
Arduino IDE 2 · placa **ESP32S3 Dev Module** · Flash Size **16MB** · PSRAM **OPI PSRAM** ·
Partition Scheme **16M Flash (3MB APP/9.9MB FATFS)**.

Bibliotecas: LovyanGFX, ArduinoJson (v7), Adafruit SSD1306, LiquidCrystal I2C, DHT sensor library.

1. Copie o conteúdo de `cartao_sd/` para a raiz do cartão (FAT32) do AMP.
2. Grave `placa_s3_V/amp/amp.ino` na Placa V e `placa_s3_E/pedal/pedal.ino` na Placa E.
3. O LED das duas placas fica verde quando elas se conectam.
4. CAIXA: placa **ESP32 Dev Module** na IDE; por enquanto só o teste `t6_audio_8bit`.

Matriz de pinos e MACs: `design.md`, seção 13.

## Testes
| Teste | Placa | O que testa |
|---|---|---|
| `teste_geral` | qualquer | Chip, clock, memória, MACs, versões, último reset, desempenho, energia, Wi-Fi |
| `t0_sd` | AMP | Cartão SD, velocidade, pastas |
| `t1_placa_v` | AMP | Telas, SD, sprite na PSRAM, MAC |
| `t2_audio` | AMP | Tom, sirene e WAV pelo UDA1334A |
| `t3_placa_e` + `t3_placa_v_link` | PEDAL + AMP | Rádio nos dois sentidos, botões, encoder, OLED, DHT11 |
| `t4_mapa_v` | AMP | Mapa de salas com névoa |
| `t5_calibrar_telas` | AMP | Ajuste de offset e cores das telas |
| `t6_audio_8bit` | CAIXA | Sintetizador 8 bits pelo UDA1334A (I2S): músicas, efeitos e o "Ruído" |

Os testes são independentes (cada um tem a sua cópia da Mensagem). O projeto usa `comum/mensagem.h`.
