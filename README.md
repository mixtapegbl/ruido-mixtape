# RUÍDO — uma MIXTAPE de terror

RPG de exploração de salas e batalhas por turnos que roda em **duas ESP32-S3**:

| Placa | Pasta | Papel |
|---|---|---|
| **AMP** (Placa V) | `amp/` | O console: roda o jogo, telas 2.0" e 0.96", cartão SD, áudio (UDA1334A → TPA3116D2), RTC |
| **PEDAL** (Placa E) | `pedal/` | O controle: 5 botões, encoder EC11, LCD 16x2, OLED, buzzer, LED RGB, DHT11 |

As duas conversam por **ESP-NOW**; o "idioma" do rádio fica em `comum/mensagem.h`.
O mundo do jogo reage à sala real: hora (RTC), temperatura e umidade (DHT11).

## Estrutura
```
amp/amp.ino          Placa V (console)
pedal/pedal.ino      Placa E (controle)
comum/mensagem.h     Mensagem do rádio, MACs e canal (usado pelas duas)
cartao_sd/           Conteúdo do micro SD (mapas, dados, sprites, som)
testes/              Testes de hardware t0 a t5
design.md            Documento de design completo (história, regras, telas, pinos)
CREDITOS.md          Bibliotecas e materiais de terceiros
```

## Como gravar
Arduino IDE 2 · placa **ESP32S3 Dev Module** · Flash Size **16MB** · PSRAM **OPI PSRAM** ·
Partition Scheme **16M Flash (3MB APP/9.9MB FATFS)**.

Bibliotecas: LovyanGFX, ArduinoJson (v7), Adafruit SSD1306, LiquidCrystal I2C, DHT sensor library.

1. Copie o conteúdo de `cartao_sd/` para a raiz do cartão (FAT32).
2. Grave `amp/amp.ino` na Placa V e `pedal/pedal.ino` na Placa E.
3. O LED das duas placas fica verde quando elas se conectam.

Matriz de pinos e MACs: `design.md`, seção 13.

## Testes
| Teste | Placa | O que testa |
|---|---|---|
| `t0_sd` | V | Cartão SD, velocidade, pastas |
| `t1_placa_v` | V | Telas, SD, sprite na PSRAM, MAC |
| `t2_audio` | V | Tom, sirene e WAV pelo UDA1334A |
| `t3_placa_e` + `t3_placa_v_link` | E + V | Rádio nos dois sentidos, botões, encoder, OLED, DHT11 |
| `t4_mapa_v` | V | Mapa de salas com névoa |
| `t5_calibrar_telas` | V | Ajuste de offset e cores das telas |

Os testes são independentes (cada um tem a sua cópia da Mensagem). O jogo de verdade usa `comum/mensagem.h`.
