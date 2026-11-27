// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 1 · PLACA V (console)
// Testa: tela 2.0" + tela 0.96" (LovyanGFX), cartão SD no mesmo SPI,
//        PNG do SD, sprite na PSRAM, LED onboard e mostra o MAC da placa.
//
// Arduino IDE (Ferramentas):
//   Placa: ESP32S3 Dev Module · Flash Size: 16MB · PSRAM: OPI PSRAM
//   Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS)
//   USB CDC On Boot: Enabled  (só se for usar a porta USB nativa para o Serial)
// ============================================================================

#include <SPI.h>
#include <SD.h>            // tem que vir ANTES da LovyanGFX para o drawPngFile(SD, ...)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <esp_mac.h>

// --- MATRIZ DE PINOS DA PLACA V ---------------------------------------------
#define PINO_SCL   12   // SPI comum (SCLK / SCK)
#define PINO_SDA   11   // SPI comum (MOSI)
#define PINO_MISO  13   // só o SD usa

#define T20_CS     10
#define T20_DC      9
#define T20_RST     8
#define T20_BLK     7   // brilho por PWM

#define T09_CS      6
#define T09_DC      5
#define T09_RST     4

#define SD_CS      14
#define LED_PLACA  48

#define SPI_FREQ_TELAS 27000000   // 27 MHz: seguro em protoboard
#define SPI_FREQ_SD    20000000

// --- TELA 2.0" ST7789 (240x320) ---------------------------------------------
class TelaGrande : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
  lgfx::Light_PWM    _luz;
public:
  TelaGrande() {
    { auto c = _bus.config();
      c.spi_host   = SPI2_HOST;
      c.spi_mode   = 0;
      c.freq_write = SPI_FREQ_TELAS;
      c.freq_read  = 16000000;
      c.spi_3wire  = false;
      c.use_lock   = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = PINO_SCL;
      c.pin_mosi = PINO_SDA;
      c.pin_miso = PINO_MISO;
      c.pin_dc   = T20_DC;
      _bus.config(c);
      _panel.setBus(&_bus); }
    { auto c = _panel.config();
      c.pin_cs   = T20_CS;
      c.pin_rst  = T20_RST;
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;
      c.offset_x = 0;
      c.offset_y = 0;
      c.readable   = false;
      c.invert     = true;    // IPS: se as cores saírem invertidas, troque para false
      c.rgb_order  = false;   // se vermelho e azul trocarem, troque para true
      c.bus_shared = true;    // divide o SPI com a outra tela e o SD
      _panel.config(c); }
    { auto c = _luz.config();
      c.pin_bl = T20_BLK;
      c.invert = false;
      c.freq   = 12000;
      c.pwm_channel = 7;
      _luz.config(c);
      _panel.setLight(&_luz); }
    setPanel(&_panel);
  }
};

// --- TELA 0.96" ST7735S (80x160) --------------------------------------------
class TelaPequena : public lgfx::LGFX_Device {
  lgfx::Panel_ST7735S _panel;
  lgfx::Bus_SPI       _bus;
public:
  TelaPequena() {
    { auto c = _bus.config();
      c.spi_host   = SPI2_HOST;
      c.spi_mode   = 0;
      c.freq_write = SPI_FREQ_TELAS;
      c.freq_read  = 16000000;
      c.spi_3wire  = false;
      c.use_lock   = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = PINO_SCL;
      c.pin_mosi = PINO_SDA;
      c.pin_miso = PINO_MISO;
      c.pin_dc   = T09_DC;
      _bus.config(c);
      _panel.setBus(&_bus); }
    { auto c = _panel.config();
      c.pin_cs   = T09_CS;
      c.pin_rst  = T09_RST;
      c.pin_busy = -1;
      c.panel_width   = 80;
      c.panel_height  = 160;
      c.memory_width  = 132;
      c.memory_height = 162;
      c.offset_x = 24;        // valores do driver Adafruit MINI160x80; confira no t5_calibrar_telas
      c.offset_y = 0;
      c.readable   = false;
      c.invert     = false;
      c.rgb_order  = true;
      c.bus_shared = true;
      _panel.config(c); }
    setPanel(&_panel);
  }
};

TelaGrande  grande;
TelaPequena pequena;
LGFX_Sprite quadro(&grande);   // tela inteira desenhada na PSRAM

char macTexto[18];             // "A4:CB:8F:D4:AB:34"
bool sdOk = false;

// ----------------------------------------------------------------------------
void lerMac() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);   // é o MAC que o ESP-NOW usa
  snprintf(macTexto, sizeof(macTexto), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.println();
  Serial.println("==============================================");
  Serial.printf ("[PLACA V] MAC: %s\n", macTexto);
  Serial.println("Cole na Placa E:");
  Serial.printf ("uint8_t macPlacaV[] = {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X};\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("==============================================");
}

void led(uint8_t r, uint8_t g, uint8_t b) {
  rgbLedWrite(LED_PLACA, r, g, b);
}

// Passo 1: as duas telas ligando, cada uma com uma cor
void passo1_telas() {
  grande.init();
  grande.setRotation(0);
  grande.setBrightness(200);
  pequena.init();
  pequena.setRotation(1);       // deitada: 160x80

  grande.fillScreen(TFT_RED);   delay(300);
  grande.fillScreen(TFT_GREEN); delay(300);
  grande.fillScreen(TFT_BLUE);  delay(300);
  pequena.fillScreen(TFT_RED);   delay(300);
  pequena.fillScreen(TFT_GREEN); delay(300);
  pequena.fillScreen(TFT_BLUE);  delay(300);
  Serial.println("[1] Telas: se viu vermelho, verde e azul nas duas, OK");
}

// Passo 2: SD no mesmo SPI
void passo2_sd() {
  SPI.begin(PINO_SCL, PINO_MISO, PINO_SDA, SD_CS);
  sdOk = SD.begin(SD_CS, SPI, SPI_FREQ_SD);
  if (!sdOk) {
    Serial.println("[2] SD: FALHOU (cartão, fios do MISO/CS ou VCC do módulo)");
    return;
  }
  Serial.printf("[2] SD: OK, %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
  File raiz = SD.open("/");
  File f = raiz.openNextFile();
  while (f) {
    Serial.printf("     %s%s  (%u bytes)\n", f.name(), f.isDirectory() ? "/" : "", (unsigned)f.size());
    f = raiz.openNextFile();
  }
}

// Passo 3: PNG do SD com tempo medido
void passo3_png() {
  if (!sdOk || !SD.exists("/sprites/teste.png")) {
    Serial.println("[3] PNG: pulado (coloque /sprites/teste.png no cartão)");
    return;
  }
  grande.fillScreen(TFT_DARKGREY);
  uint32_t t0 = millis();
  grande.drawPngFile(SD, "/sprites/teste.png", 88, 128);   // 64x64 no centro
  uint32_t dt = millis() - t0;
  Serial.printf("[3] PNG 64x64: %u ms\n", (unsigned)dt);
  delay(1500);
}

// Passo 4: tela inteira no sprite (PSRAM) e envio de uma vez
bool passo4_sprite() {
  quadro.setPsram(true);
  quadro.setColorDepth(16);
  if (!quadro.createSprite(240, 320)) {
    Serial.println("[4] Sprite: FALHOU (PSRAM está como OPI nas Ferramentas?)");
    return false;
  }
  Serial.printf("[4] Sprite 240x320 na PSRAM: OK (PSRAM livre: %u KB)\n",
                (unsigned)(ESP.getFreePsram() / 1024));
  return true;
}

void desenharTelaGrande(uint32_t quadroNum, float fps) {
  quadro.fillScreen(TFT_BLACK);
  quadro.setTextColor(0xFC60);            // laranja MIXTAPE
  quadro.setTextDatum(top_center);
  quadro.setFont(&fonts::Font4);
  quadro.drawString("RUIDO", 120, 20);
  quadro.setFont(&fonts::Font2);
  quadro.setTextColor(TFT_LIGHTGREY);
  quadro.drawString("Placa V - teste 1", 120, 52);

  quadro.setTextColor(TFT_WHITE);
  quadro.drawString("MAC da placa:", 120, 100);
  quadro.setFont(&fonts::Font4);
  quadro.setTextColor(TFT_CYAN);
  quadro.drawString(macTexto, 120, 120);

  quadro.setFont(&fonts::Font2);
  quadro.setTextColor(sdOk ? TFT_GREEN : TFT_RED);
  quadro.drawString(sdOk ? "SD: OK" : "SD: falhou", 120, 170);

  // quadrado andando: se a tela inteira atualiza sem "linhas", o sprite funciona
  int x = (quadroNum * 4) % 208;
  quadro.fillRect(x, 220, 32, 32, 0xFC60);
  quadro.drawRect(0, 218, 240, 36, TFT_DARKGREY);

  quadro.setTextColor(TFT_LIGHTGREY);
  char linha[24];
  snprintf(linha, sizeof(linha), "%.1f quadros/s", fps);
  quadro.drawString(linha, 120, 270);

  quadro.pushSprite(0, 0);                 // manda a tela inteira de uma vez
}

void desenharTelaPequena() {
  pequena.fillScreen(TFT_BLACK);
  pequena.setTextColor(0xFC60, TFT_BLACK);
  pequena.setFont(&fonts::Font2);
  pequena.setTextDatum(top_left);
  pequena.drawString("PLACA V", 4, 2);
  pequena.setTextColor(TFT_WHITE, TFT_BLACK);
  pequena.setFont(&fonts::Font0);
  pequena.drawString("MAC:", 4, 26);
  pequena.drawString(macTexto, 4, 38);
  pequena.setTextColor(sdOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
  pequena.drawString(sdOk ? "SD OK" : "SD FALHOU", 4, 60);
}

// ----------------------------------------------------------------------------
bool spriteOk = false;

void setup() {
  Serial.begin(115200);
  delay(800);
  led(0, 0, 40);                // azul: iniciando

  lerMac();
  passo1_telas();
  passo2_sd();
  passo3_png();
  spriteOk = passo4_sprite();
  desenharTelaPequena();        // passo 5: a pequena fica fixa, a grande anima

  led(sdOk && spriteOk ? 0 : 40, sdOk && spriteOk ? 40 : 0, 0);   // verde = tudo OK, vermelho = algo falhou
}

void loop() {
  static uint32_t quadroNum = 0;
  static uint32_t ultimoFps = 0;
  static uint32_t contagem = 0;
  static float fps = 0;
  static uint32_t ultimoMacSerial = 0;

  if (spriteOk) {
    desenharTelaGrande(quadroNum++, fps);
    contagem++;
  }

  uint32_t agora = millis();
  if (agora - ultimoFps >= 1000) {
    fps = contagem * 1000.0f / (agora - ultimoFps);
    contagem = 0;
    ultimoFps = agora;
  }

  // repete o MAC no Serial a cada 10 s, caso o monitor tenha sido aberto depois
  if (agora - ultimoMacSerial >= 10000) {
    ultimoMacSerial = agora;
    Serial.printf("[PLACA V] MAC: %s | %.1f quadros/s\n", macTexto, fps);
  }
}
