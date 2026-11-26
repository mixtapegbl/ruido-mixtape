// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 3 · PLACA V (console) recebendo a PLACA E por ESP-NOW
// Mostra na tela grande cada botão/giro que chega da Placa E, o clima do DHT11
// e o estado da conexão; responde mandando texto para o LCD 16x2 da Placa E.
// Grave junto com o t3_placa_e.ino na Placa E.
//
// Arduino IDE: ESP32S3 Dev Module · Flash 16MB · PSRAM OPI
// ============================================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// --- MATRIZ DE PINOS DA PLACA V ---------------------------------------------
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
#define LED_PLACA  48
#define SPI_FREQ_TELAS 27000000

#define CANAL_RADIO 1

// --- MENSAGEM (igual nas duas placas) ---------------------------------------
enum TipoMsg : uint8_t { MSG_PING = 1, MSG_BOTAO = 2, MSG_LCD = 3, MSG_CLIMA = 4 };
enum Botao   : uint8_t { B_MENU, B_CIMA, B_BAIXO, B_OK, B_VOLTAR, B_ENC_SW, B_ENC_GIRO };
enum Acao    : uint8_t { CLIQUE = 0, SEGUROU = 1 };

typedef struct __attribute__((packed)) {
  uint8_t  tipo;
  uint8_t  botao;
  uint8_t  acao;
  int8_t   giro;
  float    temperatura;
  float    umidade;
  char     linha1[17];
  char     linha2[17];
  uint32_t contador;
} Mensagem;

const char* NOMES[] = { "MENU", "CIMA", "BAIXO", "OK", "VOLTAR", "ENC OK", "ENC GIRO" };

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
LGFX_Sprite quadro(&grande);

// --- fila de eventos (o rádio chega numa tarefa separada; a tela desenha no loop)
#define TAM_FILA 16
Mensagem fila[TAM_FILA];
volatile uint8_t filaIni = 0, filaFim = 0;
portMUX_TYPE trava = portMUX_INITIALIZER_UNLOCKED;

uint8_t macPlacaE[6] = {0};
volatile bool conheceE = false;
volatile uint32_t ultimoContatoE = 0;
uint32_t pingsRecebidos = 0, botoesRecebidos = 0, perdidos = 0, ultimoContador = 0;
float temp = NAN, umid = NAN;
char registro[8][24];        // últimas 8 linhas mostradas na tela grande
uint8_t qtdRegistro = 0;
int posicaoEncoder = 0;

void ledPlaca(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(LED_PLACA, r, g, b); }

// --- ESP-NOW -------------------------------------------------------------------
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
void aoEnviar(const esp_now_send_info_t* info, esp_now_send_status_t status) {}
#else
void aoEnviar(const uint8_t* mac, esp_now_send_status_t status) {}
#endif

void aoReceber(const esp_now_recv_info_t* info, const uint8_t* dados, int tam) {
  if (tam != sizeof(Mensagem)) return;
  if (!conheceE) {                       // a primeira placa que falar vira a Placa E
    memcpy(macPlacaE, info->src_addr, 6);
    conheceE = true;
  } else if (memcmp(info->src_addr, macPlacaE, 6) != 0) return;
  ultimoContatoE = millis();
  portENTER_CRITICAL(&trava);
  uint8_t prox = (filaFim + 1) % TAM_FILA;
  if (prox != filaIni) { memcpy(&fila[filaFim], dados, sizeof(Mensagem)); filaFim = prox; }
  portEXIT_CRITICAL(&trava);
}

bool pegarDaFila(Mensagem& m) {
  bool tem = false;
  portENTER_CRITICAL(&trava);
  if (filaIni != filaFim) { memcpy(&m, &fila[filaIni], sizeof(Mensagem)); filaIni = (filaIni + 1) % TAM_FILA; tem = true; }
  portEXIT_CRITICAL(&trava);
  return tem;
}

bool peerAdicionado = false;
void garantirPeerE() {
  if (peerAdicionado || !conheceE) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, macPlacaE, 6);
  peer.channel = CANAL_RADIO;
  peer.encrypt = false;
  peerAdicionado = esp_now_add_peer(&peer) == ESP_OK;
  Serial.printf("[PLACA V] Placa E encontrada: %02X:%02X:%02X:%02X:%02X:%02X\n",
                macPlacaE[0], macPlacaE[1], macPlacaE[2], macPlacaE[3], macPlacaE[4], macPlacaE[5]);
  Serial.printf("uint8_t macPlacaE[] = {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X};\n",
                macPlacaE[0], macPlacaE[1], macPlacaE[2], macPlacaE[3], macPlacaE[4], macPlacaE[5]);
}

void enviarParaE(uint8_t tipo, const char* l1 = "", const char* l2 = "") {
  if (!peerAdicionado) return;
  Mensagem m = {};
  m.tipo = tipo;
  strncpy(m.linha1, l1, 16);
  strncpy(m.linha2, l2, 16);
  esp_now_send(macPlacaE, (uint8_t*)&m, sizeof(Mensagem));
}

void registrar(const char* txt) {
  if (qtdRegistro < 8) qtdRegistro++;
  for (int i = qtdRegistro - 1; i > 0; i--) strcpy(registro[i], registro[i - 1]);
  strncpy(registro[0], txt, 23);
  registro[0][23] = 0;
}

// --- telas -------------------------------------------------------------------
void desenharGrande(bool conectado) {
  quadro.fillScreen(TFT_BLACK);
  quadro.setTextDatum(top_center);
  quadro.setFont(&fonts::Font4);
  quadro.setTextColor(0xFC60);
  quadro.drawString("RUIDO", 120, 8);
  quadro.setFont(&fonts::Font2);
  quadro.setTextColor(conectado ? TFT_GREEN : TFT_RED);
  quadro.drawString(conectado ? "Placa E conectada" : "Esperando a Placa E...", 120, 40);

  quadro.setTextColor(TFT_LIGHTGREY);
  char linha[40];
  if (isnan(temp)) snprintf(linha, sizeof(linha), "Clima: sem leitura");
  else snprintf(linha, sizeof(linha), "Clima: %.1f C  %.0f%%", temp, umid);
  quadro.drawString(linha, 120, 62);
  snprintf(linha, sizeof(linha), "Encoder: %d", posicaoEncoder);
  quadro.drawString(linha, 120, 80);

  quadro.drawFastHLine(10, 102, 220, TFT_DARKGREY);
  quadro.setTextDatum(top_left);
  for (uint8_t i = 0; i < qtdRegistro; i++) {
    quadro.setTextColor(i == 0 ? TFT_WHITE : TFT_DARKGREY);
    quadro.setFont(i == 0 ? &fonts::Font4 : &fonts::Font2);
    quadro.drawString(registro[i], 14, i == 0 ? 112 : 122 + i * 20);
  }

  quadro.setFont(&fonts::Font0);
  quadro.setTextColor(TFT_DARKGREY);
  quadro.setTextDatum(bottom_center);
  snprintf(linha, sizeof(linha), "pings %lu  botoes %lu  perdidas %lu",
           (unsigned long)pingsRecebidos, (unsigned long)botoesRecebidos, (unsigned long)perdidos);
  quadro.drawString(linha, 120, 316);
  quadro.pushSprite(0, 0);
}

void desenharPequena(bool conectado) {
  pequena.fillScreen(TFT_BLACK);
  pequena.setTextDatum(middle_center);
  pequena.setFont(&fonts::Font2);
  pequena.setTextColor(conectado ? TFT_GREEN : TFT_RED, TFT_BLACK);
  pequena.drawString(conectado ? "CONECTADO" : "SEM SINAL", 80, 28);
  pequena.setFont(&fonts::Font0);
  pequena.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  char linha[24];
  snprintf(linha, sizeof(linha), "pings: %lu", (unsigned long)pingsRecebidos);
  pequena.drawString(linha, 80, 56);
}

// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(800);
  ledPlaca(0, 0, 40);

  grande.init(); grande.setRotation(0); grande.setBrightness(200);
  pequena.init(); pequena.setRotation(1);
  quadro.setPsram(true); quadro.setColorDepth(16); quadro.createSprite(240, 320);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL_RADIO, WIFI_SECOND_CHAN_NONE);
  bool ok = esp_now_init() == ESP_OK;
  esp_now_register_send_cb(aoEnviar);
  esp_now_register_recv_cb(aoReceber);
  Serial.printf("[PLACA V] ESP-NOW: %s (canal %d)\n", ok ? "OK" : "FALHOU", CANAL_RADIO);

  registrar(ok ? "Radio OK" : "Radio FALHOU");
  desenharGrande(false);
  desenharPequena(false);
  ledPlaca(40, 0, 0);
}

void loop() {
  uint32_t agora = millis();
  bool mudou = false;
  garantirPeerE();

  Mensagem m;
  while (pegarDaFila(m)) {
    if (ultimoContador && m.contador > ultimoContador + 1) perdidos += m.contador - ultimoContador - 1;
    ultimoContador = m.contador;

    if (m.tipo == MSG_PING) { pingsRecebidos++; continue; }
    if (m.tipo == MSG_CLIMA) { temp = m.temperatura; umid = m.umidade; mudou = true; continue; }
    if (m.tipo != MSG_BOTAO || m.botao > B_ENC_GIRO) continue;

    botoesRecebidos++;
    char txt[24], l1[17], l2[17];
    if (m.botao == B_ENC_GIRO) {
      posicaoEncoder += m.giro;
      snprintf(txt, sizeof(txt), "Encoder %s", m.giro > 0 ? "+1" : "-1");
      snprintf(l1, sizeof(l1), "Encoder: %d", posicaoEncoder);
    } else {
      snprintf(txt, sizeof(txt), "%s%s", NOMES[m.botao], m.acao == SEGUROU ? " (segurou)" : "");
      snprintf(l1, sizeof(l1), "Botao: %s", NOMES[m.botao]);
    }
    snprintf(l2, sizeof(l2), "V recebeu #%lu", (unsigned long)botoesRecebidos);
    registrar(txt);
    enviarParaE(MSG_LCD, l1, l2);         // resposta aparece no LCD da Placa E
    Serial.printf("<- %s\n", txt);
    mudou = true;
  }

  static uint32_t ultimoPing = 0;
  if (agora - ultimoPing >= 1000) { ultimoPing = agora; enviarParaE(MSG_PING); }

  static bool conectadoAntes = false;
  bool conectado = ultimoContatoE && (agora - ultimoContatoE < 3000);
  if (conectado != conectadoAntes) {
    conectadoAntes = conectado;
    ledPlaca(conectado ? 0 : 40, conectado ? 40 : 0, 0);
    registrar(conectado ? "Placa E conectou" : "Placa E sumiu");
    if (conectado) enviarParaE(MSG_LCD, "Conectado na V!", "Aperte um botao");
    Serial.println(conectado ? "== Placa E CONECTADA ==" : "== Placa E SEM SINAL ==");
    desenharPequena(conectado);
    mudou = true;
  }

  static uint32_t ultimoPequena = 0;
  if (agora - ultimoPequena >= 1000) { ultimoPequena = agora; desenharPequena(conectado); }

  if (mudou) desenharGrande(conectado);
}
