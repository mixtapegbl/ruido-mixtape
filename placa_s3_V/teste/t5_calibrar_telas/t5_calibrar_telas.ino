// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 5 · CALIBRAR AS TELAS DA PLACA V (LovyanGFX)
// Ajusta ao vivo, pelo Monitor Serial, o deslocamento e as cores das telas.
// Quando estiver certo, digite "v" e copie os valores para os outros testes.
//
// Valores iniciais da 0.96" copiados do driver Adafruit INITR_MINI160x80,
// que funcionou no seu teste: offset_x 24 · offset_y 0 · sem inversão · RGB.
//
// O QUE CONFERIR:
//   Tela 0.96": a moldura branca aparece INTEIRA nos 4 lados, sem faixa de lixo,
//               e as barras estão na cor escrita embaixo de cada uma.
//   Tela 2.0" : as barras estão na cor escrita.
//
// Comandos (Monitor Serial, 115200):
//   Tela 0.96":  a / d = offset_x -1 / +1     w / s = offset_y -1 / +1
//                i = liga/desliga inversão     r = troca RGB <-> BGR
//   Tela 2.0" :  I = liga/desliga inversão     R = troca RGB <-> BGR
//   v = mostra os valores prontos para colar no código
// ============================================================================

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define PINO_SCL   12
#define PINO_SDA   11
#define PINO_MISO  13
#define T20_CS     10
#define T20_DC      9
#define T20_RST     8
#define T20_BLK     7
#define T09_CS      6
#define T09_DC      5
#define T09_RST     4
#define SD_CS      14
#define SPI_FREQ_TELAS 27000000

// valores que vão sendo ajustados
int  peqOffX = 24, peqOffY = 0;
bool peqInvert = false, peqRgb = true;
bool grdInvert = true,  grdRgb = false;

class TelaGrande : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
  lgfx::Light_PWM    _luz;
public:
  TelaGrande() {
    { auto c = _bus.config();
      c.spi_host = SPI2_HOST; c.spi_mode = 0;
      c.freq_write = SPI_FREQ_TELAS; c.freq_read = 16000000;
      c.spi_3wire = false; c.use_lock = true; c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = PINO_SCL; c.pin_mosi = PINO_SDA; c.pin_miso = PINO_MISO; c.pin_dc = T20_DC;
      _bus.config(c); _panel.setBus(&_bus); }
    { auto c = _luz.config(); c.pin_bl = T20_BLK; c.invert = false; c.freq = 12000; c.pwm_channel = 7;
      _luz.config(c); _panel.setLight(&_luz); }
    ajustar();
    setPanel(&_panel);
  }
  void ajustar() {
    auto c = _panel.config();
    c.pin_cs = T20_CS; c.pin_rst = T20_RST; c.pin_busy = -1;
    c.panel_width = 240; c.panel_height = 320;
    c.offset_x = 0; c.offset_y = 0;
    c.readable = false; c.bus_shared = true;
    c.invert = grdInvert; c.rgb_order = grdRgb;
    _panel.config(c);
  }
};

class TelaPequena : public lgfx::LGFX_Device {
  lgfx::Panel_ST7735S _panel;
  lgfx::Bus_SPI       _bus;
public:
  TelaPequena() {
    { auto c = _bus.config();
      c.spi_host = SPI2_HOST; c.spi_mode = 0;
      c.freq_write = SPI_FREQ_TELAS; c.freq_read = 16000000;
      c.spi_3wire = false; c.use_lock = true; c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = PINO_SCL; c.pin_mosi = PINO_SDA; c.pin_miso = PINO_MISO; c.pin_dc = T09_DC;
      _bus.config(c); _panel.setBus(&_bus); }
    ajustar();
    setPanel(&_panel);
  }
  void ajustar() {
    auto c = _panel.config();
    c.pin_cs = T09_CS; c.pin_rst = T09_RST; c.pin_busy = -1;
    c.panel_width = 80; c.panel_height = 160;
    c.memory_width = 132; c.memory_height = 162;
    c.offset_x = peqOffX; c.offset_y = peqOffY;
    c.readable = false; c.bus_shared = true;
    c.invert = peqInvert; c.rgb_order = peqRgb;
    _panel.config(c);
  }
};

TelaGrande  grande;
TelaPequena pequena;

void desenharPequena() {
  pequena.ajustar();
  pequena.init();
  pequena.setRotation(1);                 // deitada: 160x80
  pequena.fillScreen(TFT_BLACK);
  pequena.drawRect(0, 0, 160, 80, TFT_WHITE);     // moldura: tem que aparecer inteira
  pequena.drawRect(1, 1, 158, 78, TFT_DARKGREY);
  pequena.fillRect(0, 0, 6, 6, TFT_WHITE);         // cantos
  pequena.fillRect(154, 0, 6, 6, TFT_WHITE);
  pequena.fillRect(0, 74, 6, 6, TFT_WHITE);
  pequena.fillRect(154, 74, 6, 6, TFT_WHITE);

  pequena.fillRect(12, 10, 40, 34, TFT_RED);
  pequena.fillRect(60, 10, 40, 34, TFT_GREEN);
  pequena.fillRect(108, 10, 40, 34, TFT_BLUE);
  pequena.setFont(&fonts::Font0);
  pequena.setTextColor(TFT_WHITE);
  pequena.setTextDatum(top_center);
  pequena.drawString("VERM", 32, 48);
  pequena.drawString("VERDE", 80, 48);
  pequena.drawString("AZUL", 128, 48);
  char linha[32];
  snprintf(linha, sizeof(linha), "x%d y%d %s %s", peqOffX, peqOffY, peqInvert ? "inv" : "normal", peqRgb ? "RGB" : "BGR");
  pequena.drawString(linha, 80, 62);
}

void desenharGrande() {
  grande.ajustar();
  grande.init();
  grande.setRotation(0);
  grande.setBrightness(200);
  grande.fillScreen(TFT_BLACK);
  grande.drawRect(0, 0, 240, 320, TFT_WHITE);
  grande.fillRect(16, 40, 64, 160, TFT_RED);
  grande.fillRect(88, 40, 64, 160, TFT_GREEN);
  grande.fillRect(160, 40, 64, 160, TFT_BLUE);
  grande.setFont(&fonts::Font2);
  grande.setTextColor(TFT_WHITE);
  grande.setTextDatum(top_center);
  grande.drawString("VERMELHO", 48, 208);
  grande.drawString("VERDE", 120, 208);
  grande.drawString("AZUL", 192, 208);
  grande.setTextColor(0xFC60);
  grande.drawString("laranja MIXTAPE", 120, 240);
  grande.setTextColor(TFT_LIGHTGREY);
  char linha[32];
  snprintf(linha, sizeof(linha), "2.0: %s %s", grdInvert ? "inv" : "normal", grdRgb ? "RGB" : "BGR");
  grande.drawString(linha, 120, 280);
  grande.drawString("Calibrando...", 120, 12);
}

void mostrarValores() {
  Serial.println();
  Serial.println("===== COLE NOS OUTROS TESTES =====");
  Serial.println("Tela 0.96\" (TelaPequena):");
  Serial.printf ("  c.offset_x = %d;\n  c.offset_y = %d;\n  c.invert = %s;\n  c.rgb_order = %s;\n",
                 peqOffX, peqOffY, peqInvert ? "true" : "false", peqRgb ? "true" : "false");
  Serial.println("Tela 2.0\" (TelaGrande):");
  Serial.printf ("  c.invert = %s;\n  c.rgb_order = %s;\n", grdInvert ? "true" : "false", grdRgb ? "true" : "false");
  Serial.println("==================================");
}

void ajuda() {
  Serial.println();
  Serial.println("0.96\": a/d = x-/x+  w/s = y-/y+  i = inverter  r = RGB/BGR");
  Serial.println("2.0\" : I = inverter  R = RGB/BGR      v = mostrar valores");
  Serial.printf ("Agora: 0.96\" x%d y%d %s %s | 2.0\" %s %s\n", peqOffX, peqOffY,
                 peqInvert ? "inv" : "normal", peqRgb ? "RGB" : "BGR",
                 grdInvert ? "inv" : "normal", grdRgb ? "RGB" : "BGR");
}

void setup() {
  Serial.begin(115200);
  delay(1200);
  pinMode(SD_CS, OUTPUT); digitalWrite(SD_CS, HIGH);   // SD fora do barramento
  desenharGrande();
  desenharPequena();
  Serial.println("\n===== CALIBRAR TELAS · PLACA V =====");
  ajuda();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  bool peq = true, grd = false;
  switch (c) {
    case 'a': peqOffX--; break;
    case 'd': peqOffX++; break;
    case 'w': peqOffY--; break;
    case 's': peqOffY++; break;
    case 'i': peqInvert = !peqInvert; break;
    case 'r': peqRgb = !peqRgb; break;
    case 'I': grdInvert = !grdInvert; peq = false; grd = true; break;
    case 'R': grdRgb = !grdRgb; peq = false; grd = true; break;
    case 'v': mostrarValores(); return;
    case '\n': case '\r': return;
    default: ajuda(); return;
  }
  if (grd) desenharGrande();
  if (peq) desenharPequena();
  ajuda();
}
