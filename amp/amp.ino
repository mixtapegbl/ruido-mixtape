// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// AMP · Placa V (console) — o jogo roda aqui
// Ponto de partida: teste 4 (mapa de salas com névoa). Grave o PEDAL na Placa E.
//
// Controles (Placa E):
//   CIMA / BAIXO / girar o encoder -> escolhe a saída
//   OK / apertar o encoder         -> anda pela saída escolhida
//   MENU                           -> +10 de Corrupção (teste da barra)
//   VOLTAR (segurar 2 s)           -> recomeça o mapa
//
// Mapa: lê /mapas/area1.json do SD. Sem SD, usa o mapa embutido abaixo.
// Arduino IDE: ESP32S3 Dev Module · Flash 16MB · PSRAM OPI
// Biblioteca: ArduinoJson (Benoit Blanchon), versão 7
// ============================================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
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
#define SD_CS      14
#define LED_PLACA  48
#define SPI_FREQ_TELAS 27000000

#include "../comum/mensagem.h"   // Mensagem, MACs e canal do rádio

// --- MAPA EMBUTIDO (igual ao /mapas/area1.json) -----------------------------
const char MAPA_PADRAO[] PROGMEM = R"json(
{
  "area": 1,
  "nome": "Fabrica",
  "inicio": 0,
  "salas": [
    { "id": 0, "x": 2, "y": 4, "tipo": "inicio",  "nome": "Linha de montagem", "texto": "Esteira parada",   "saidas": { "n": 1 } },
    { "id": 1, "x": 2, "y": 3, "tipo": "vazio",   "nome": "Corredor de graxa", "texto": "Cheiro de oleo",   "saidas": { "s": 0, "o": 2, "l": 3, "n": 5 } },
    { "id": 2, "x": 1, "y": 3, "tipo": "inimigo", "nome": "Deposito",          "texto": "Rato-Fio a vista", "saidas": { "l": 1, "n": 4 } },
    { "id": 3, "x": 3, "y": 3, "tipo": "bau",     "nome": "Almoxarifado",      "texto": "Um bau velho",     "saidas": { "o": 1 } },
    { "id": 4, "x": 1, "y": 2, "tipo": "altar",   "nome": "Capela do turno",   "texto": "Altar de sucata",  "saidas": { "s": 2, "n": 6 } },
    { "id": 5, "x": 2, "y": 2, "tipo": "porta",   "nome": "Portao de carga",   "texto": "Portao aberto",    "saidas": { "s": 1, "n": 7 }, "tranca": true },
    { "id": 6, "x": 1, "y": 1, "tipo": "chave",   "nome": "Vestiario",         "texto": "Armario aberto",   "saidas": { "s": 4 } },
    { "id": 7, "x": 2, "y": 1, "tipo": "vazio",   "nome": "Esteira norte",     "texto": "Vozes ao norte..", "saidas": { "s": 5, "n": 8, "l": 9 } },
    { "id": 8, "x": 2, "y": 0, "tipo": "chefe",   "nome": "Sala da Esteira",   "texto": "ESTEIRA-MAE!",     "saidas": { "s": 7 } },
    { "id": 9, "x": 3, "y": 1, "tipo": "inimigo", "nome": "Prensas",           "texto": "Prensa Viva!",     "saidas": { "o": 7 } }
  ]
}
)json";

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

// --- CORES (RGB565) -------------------------------------------------------------
#define COR_LARANJA   0xFC60   // MIXTAPE #FF9800
#define COR_NEVOA     0x2965   // sala vista mas não visitada
#define COR_CORREDOR  0x52AA
#define COR_FUNDO     0x0841

// --- ESTRUTURA DO MAPA ---------------------------------------------------------
#define MAX_SALAS 32
enum Dir : uint8_t { D_OESTE, D_NORTE, D_LESTE, D_SUL };   // ordem das saídas no LCD
const char SIMBOLO[4] = { '<', '^', '>', 'v' };
const char* CHAVE_DIR[4] = { "o", "n", "l", "s" };

struct Sala {
  int8_t x, y;
  char tipo[10];
  char nome[24];
  char texto[17];
  int8_t saida[4];           // id da sala vizinha ou -1
  bool tranca;
  bool visitada;
  bool resolvida;            // baú aberto, chave pega, etc.
};
Sala salas[MAX_SALAS];
uint8_t qtdSalas = 0;
char nomeArea[24] = "";
int8_t salaAtual = 0, salaInicio = 0;
uint8_t saidaEscolhida = 0;  // índice dentro da lista de saídas existentes

// --- JOGADOR (valores de teste) -------------------------------------------------
int vida = 40, vidaMax = 40, mana = 10, manaMax = 10, corrupcao = 0, sucata = 0;
bool temChave = false;
char evento[17] = "";        // linha 2 do LCD

bool sdOk = false;
bool mapaDoSd = false;

// --- RÁDIO ---------------------------------------------------------------------
#define TAM_FILA 16
Mensagem fila[TAM_FILA];
volatile uint8_t filaIni = 0, filaFim = 0;
portMUX_TYPE trava = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t ultimoContatoE = 0;

void ledPlaca(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(LED_PLACA, r, g, b); }

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
void aoEnviar(const esp_now_send_info_t* info, esp_now_send_status_t status) {}
#else
void aoEnviar(const uint8_t* mac, esp_now_send_status_t status) {}
#endif

void aoReceber(const esp_now_recv_info_t* info, const uint8_t* dados, int tam) {
  if (tam != sizeof(Mensagem) || memcmp(info->src_addr, macPlacaE, 6) != 0) return;
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

void enviarParaE(uint8_t tipo, const char* l1 = "", const char* l2 = "") {
  Mensagem m = {};
  m.tipo = tipo;
  strncpy(m.linha1, l1, 16);
  strncpy(m.linha2, l2, 16);
  esp_now_send(macPlacaE, (uint8_t*)&m, sizeof(Mensagem));
}

bool iniciarRadio() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL_RADIO, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) return false;
  esp_now_register_send_cb(aoEnviar);
  esp_now_register_recv_cb(aoReceber);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, macPlacaE, 6);
  peer.channel = CANAL_RADIO;
  peer.encrypt = false;
  return esp_now_add_peer(&peer) == ESP_OK;
}

// --- CARREGAR O MAPA -----------------------------------------------------------
bool lerMapa(JsonDocument& doc) {
  qtdSalas = 0;
  strlcpy(nomeArea, doc["nome"] | "Area", sizeof(nomeArea));
  salaInicio = doc["inicio"] | 0;
  for (JsonObject s : doc["salas"].as<JsonArray>()) {
    int id = s["id"] | -1;
    if (id < 0 || id >= MAX_SALAS) continue;
    Sala& sala = salas[id];
    sala.x = s["x"] | 0;
    sala.y = s["y"] | 0;
    strlcpy(sala.tipo,  s["tipo"]  | "vazio", sizeof(sala.tipo));
    strlcpy(sala.nome,  s["nome"]  | "",      sizeof(sala.nome));
    strlcpy(sala.texto, s["texto"] | "",      sizeof(sala.texto));
    for (int d = 0; d < 4; d++) sala.saida[d] = s["saidas"][CHAVE_DIR[d]] | -1;
    sala.tranca = s["tranca"] | false;
    sala.visitada = false;
    sala.resolvida = false;
    if (id + 1 > qtdSalas) qtdSalas = id + 1;
  }
  return qtdSalas > 0;
}

void carregarMapa() {
  JsonDocument doc;
  mapaDoSd = false;
  if (sdOk && SD.exists("/mapas/area1.json")) {
    File f = SD.open("/mapas/area1.json");
    DeserializationError erro = deserializeJson(doc, f);
    f.close();
    if (!erro && lerMapa(doc)) { mapaDoSd = true; Serial.println("[MAPA] /mapas/area1.json do SD"); return; }
    Serial.printf("[MAPA] Erro no JSON do SD: %s -> usando o mapa embutido\n", erro.c_str());
  }
  deserializeJson(doc, MAPA_PADRAO);
  lerMapa(doc);
  Serial.println("[MAPA] Mapa embutido");
}

// --- LÓGICA --------------------------------------------------------------------
uint8_t listarSaidas(uint8_t dirs[4]) {
  uint8_t n = 0;
  for (uint8_t d = 0; d < 4; d++) if (salas[salaAtual].saida[d] >= 0) dirs[n++] = d;
  return n;
}

bool salaVista(uint8_t id) {           // visitada ou vizinha de uma visitada
  if (salas[id].visitada) return true;
  for (uint8_t i = 0; i < qtdSalas; i++)
    if (salas[i].visitada)
      for (uint8_t d = 0; d < 4; d++) if (salas[i].saida[d] == id) return true;
  return false;
}

void entrarNaSala(int8_t id) {
  Sala& s = salas[id];
  bool primeira = !s.visitada;
  salaAtual = id;
  s.visitada = true;
  saidaEscolhida = 0;
  strlcpy(evento, s.texto, sizeof(evento));

  if (!strcmp(s.tipo, "bau") && !s.resolvida) { s.resolvida = true; sucata += 10; strlcpy(evento, "Bau: +10 Sucata", sizeof(evento)); }
  else if (!strcmp(s.tipo, "chave") && !s.resolvida) { s.resolvida = true; temChave = true; strlcpy(evento, "Pegou a chave!", sizeof(evento)); }
  else if (!strcmp(s.tipo, "altar")) { vida = vidaMax; mana = manaMax; strlcpy(evento, "Vida/mana cheias", sizeof(evento)); }
  else if (!strcmp(s.tipo, "porta") && primeira) { strlcpy(evento, "Chave usada!", sizeof(evento)); }
  else if (!strcmp(s.tipo, "inimigo") && !s.resolvida) { /* batalha: etapa 5 */ }

  Serial.printf("[SALA %d] %s | %s\n", id, s.nome, evento);
}

void tentarAndar() {
  uint8_t dirs[4];
  uint8_t n = listarSaidas(dirs);
  if (!n) return;
  int8_t destino = salas[salaAtual].saida[dirs[saidaEscolhida % n]];
  Sala& d = salas[destino];
  if (d.tranca && !d.visitada && !temChave) { strlcpy(evento, "Porta trancada!", sizeof(evento)); return; }
  entrarNaSala(destino);
}

void recomecar() {
  for (uint8_t i = 0; i < qtdSalas; i++) { salas[i].visitada = false; salas[i].resolvida = false; }
  vida = vidaMax; mana = manaMax; corrupcao = 0; sucata = 0; temChave = false;
  entrarNaSala(salaInicio);
}

// --- TEXTO DO LCD -----------------------------------------------------------------
void montarLcd(char* l1, char* l2) {
  uint8_t dirs[4];
  uint8_t n = listarSaidas(dirs);
  char* p = l1;
  for (uint8_t i = 0; i < n; i++) {
    bool sel = (i == saidaEscolhida % n);
    *p++ = sel ? '[' : ' ';
    *p++ = SIMBOLO[dirs[i]];
    *p++ = sel ? ']' : ' ';
  }
  *p = 0;
  strlcpy(l2, evento, 17);
}

void enviarLcd() {
  char l1[17], l2[17];
  montarLcd(l1, l2);
  enviarParaE(MSG_LCD, l1, l2);
}

// --- DESENHO: TELA GRANDE (MAPA) ---------------------------------------------------
uint16_t corDoTipo(const Sala& s) {
  if (!strcmp(s.tipo, "inicio"))  return TFT_LIGHTGREY;
  if (!strcmp(s.tipo, "inimigo")) return 0xB104;          // vermelho ferrugem
  if (!strcmp(s.tipo, "bau"))     return s.resolvida ? 0x6B4D : 0xDDA0;
  if (!strcmp(s.tipo, "altar"))   return 0x2D7A;          // verde-água
  if (!strcmp(s.tipo, "chave"))   return s.resolvida ? 0x6B4D : TFT_YELLOW;
  if (!strcmp(s.tipo, "porta"))   return 0x9A60;          // marrom
  if (!strcmp(s.tipo, "chefe"))   return 0x780F;          // roxo
  return 0x4A69;                                          // vazio
}

int px(int8_t x) { return 24 + x * 48; }
int py(int8_t y) { return 24 + y * 48; }

void desenharMapa() {
  quadro.fillScreen(TFT_BLACK);
  quadro.fillRect(0, 0, 240, 240, COR_FUNDO);
  for (int i = 0; i <= 240; i += 48) {                     // grade discreta
    quadro.drawFastVLine(i, 0, 240, 0x1082);
    quadro.drawFastHLine(0, i, 240, 0x1082);
  }

  uint8_t dirs[4];
  uint8_t n = listarSaidas(dirs);
  int8_t destinoSel = n ? salas[salaAtual].saida[dirs[saidaEscolhida % n]] : -1;

  // corredores (só entre salas que o jogador já enxerga)
  for (uint8_t i = 0; i < qtdSalas; i++) {
    if (!salaVista(i)) continue;
    for (uint8_t d = 0; d < 4; d++) {
      int8_t j = salas[i].saida[d];
      if (j < 0 || j < i || !salaVista(j)) continue;
      bool sel = (i == salaAtual && j == destinoSel) || (j == salaAtual && i == destinoSel);
      uint16_t cor = sel ? COR_LARANJA : COR_CORREDOR;
      int x1 = px(salas[i].x), y1 = py(salas[i].y), x2 = px(salas[j].x), y2 = py(salas[j].y);
      quadro.drawLine(x1, y1, x2, y2, cor);
      if (x1 == x2) { quadro.drawLine(x1 - 1, y1, x2 - 1, y2, cor); quadro.drawLine(x1 + 1, y1, x2 + 1, y2, cor); }
      else          { quadro.drawLine(x1, y1 - 1, x2, y2 - 1, cor); quadro.drawLine(x1, y1 + 1, x2, y2 + 1, cor); }
    }
  }

  // salas
  quadro.setFont(&fonts::Font2);
  quadro.setTextDatum(middle_center);
  for (uint8_t i = 0; i < qtdSalas; i++) {
    if (!salaVista(i)) continue;                          // névoa total: nem aparece
    int cx = px(salas[i].x), cy = py(salas[i].y);
    if (salas[i].visitada) {
      quadro.fillRoundRect(cx - 15, cy - 15, 30, 30, 4, corDoTipo(salas[i]));
    } else {
      quadro.fillRoundRect(cx - 15, cy - 15, 30, 30, 4, COR_NEVOA);
      quadro.setTextColor(0x7BEF);
      quadro.drawString("?", cx, cy);
    }
    if (i == destinoSel) quadro.drawRoundRect(cx - 17, cy - 17, 34, 34, 5, COR_LARANJA);
    if (i == salaAtual) {                                 // o herói
      quadro.drawRoundRect(cx - 17, cy - 17, 34, 34, 5, TFT_WHITE);
      quadro.fillCircle(cx, cy, 7, COR_LARANJA);
      quadro.drawCircle(cx, cy, 7, TFT_BLACK);
    }
  }

  // painel de baixo
  Sala& s = salas[salaAtual];
  quadro.setTextDatum(top_left);
  quadro.setFont(&fonts::Font2);
  quadro.setTextColor(COR_LARANJA);
  char linha[40];
  snprintf(linha, sizeof(linha), "Area 1 - %s", nomeArea);
  quadro.drawString(linha, 8, 246);
  quadro.setFont(&fonts::Font4);
  quadro.setTextColor(TFT_WHITE);
  quadro.drawString(s.nome, 8, 264);
  quadro.setFont(&fonts::Font2);
  quadro.setTextColor(TFT_LIGHTGREY);
  quadro.drawString(evento, 8, 292);
  quadro.setTextDatum(top_right);
  quadro.setTextColor(0x632C);
  quadro.drawString(mapaDoSd ? "mapa: SD" : "mapa: embutido", 232, 246);
  if (temChave) { quadro.setTextColor(TFT_YELLOW); quadro.drawString("chave", 232, 292); }

  quadro.pushSprite(0, 0);
}

// --- DESENHO: TELA PEQUENA (STATUS) -------------------------------------------------
uint16_t corCorrupcao() {
  if (corrupcao >= 67) return 0x8800;    // vermelho sangue
  if (corrupcao >= 34) return 0xB208;    // ferrugem
  return 0x8410;                         // cinza
}

void barra(int y, const char* rotulo, int valor, int maximo, uint16_t cor, bool chiado) {
  pequena.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  pequena.setFont(&fonts::Font0);
  pequena.setTextDatum(middle_left);
  pequena.drawString(rotulo, 2, y + 5);
  int largura = 112;
  int cheio = maximo > 0 ? (valor * largura) / maximo : 0;
  pequena.drawRect(34, y, largura + 2, 11, TFT_DARKGREY);
  pequena.fillRect(35, y + 1, cheio, 9, cor);
  pequena.fillRect(35 + cheio, y + 1, largura - cheio, 9, TFT_BLACK);
  if (chiado)                                           // estática na barra de Corrupção
    for (int k = 0; k < 10; k++) pequena.drawPixel(35 + random(cheio > 0 ? cheio : 1), y + 1 + random(9), TFT_WHITE);
}

void desenharStatus() {
  barra(4,  "VIDA", vida, vidaMax, 0x2D7A, false);
  barra(20, "MANA", mana, manaMax, 0x339F, false);
  barra(36, "CORR", corrupcao, 100, corCorrupcao(), corrupcao >= 67);
  pequena.setTextDatum(middle_left);
  pequena.setTextColor(COR_LARANJA, TFT_BLACK);
  char linha[28];
  snprintf(linha, sizeof(linha), "Sucata %-4d  Corr %3d  ", sucata, corrupcao);
  pequena.drawString(linha, 2, 62);
}

// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(800);
  ledPlaca(0, 0, 40);

  grande.init(); grande.setRotation(0); grande.setBrightness(200);
  pequena.init(); pequena.setRotation(1); pequena.fillScreen(TFT_BLACK);
  quadro.setPsram(true); quadro.setColorDepth(16); quadro.createSprite(240, 320);

  SPI.begin(PINO_SCL, PINO_MISO, PINO_SDA, SD_CS);
  sdOk = SD.begin(SD_CS, SPI, 20000000);
  Serial.printf("[SD] %s\n", sdOk ? "OK" : "não montou");

  carregarMapa();
  recomecar();

  bool ok = iniciarRadio();
  Serial.printf("[RADIO] ESP-NOW %s\n", ok ? "OK" : "FALHOU");
  desenharMapa();
  desenharStatus();
  ledPlaca(40, 0, 0);
}

void loop() {
  uint32_t agora = millis();
  bool mudou = false;

  Mensagem m;
  while (pegarDaFila(m)) {
    if (m.tipo != MSG_BOTAO) continue;
    uint8_t dirs[4];
    uint8_t n = listarSaidas(dirs);
    switch (m.botao) {
      case B_CIMA:     if (n) saidaEscolhida = (saidaEscolhida + n - 1) % n; break;
      case B_BAIXO:    if (n) saidaEscolhida = (saidaEscolhida + 1) % n; break;
      case B_ENC_GIRO: if (n) saidaEscolhida = (saidaEscolhida + n + (m.giro > 0 ? 1 : -1)) % n; break;
      case B_OK:
      case B_ENC_SW:   if (m.acao == CLIQUE) tentarAndar(); break;
      case B_MENU:     if (m.acao == CLIQUE) { corrupcao = min(100, corrupcao + 10); snprintf(evento, sizeof(evento), "Corrupcao %d", corrupcao); } break;
      case B_VOLTAR:   if (m.acao == SEGUROU) { recomecar(); strlcpy(evento, "Mapa reiniciado", sizeof(evento)); } break;
    }
    mudou = true;
  }

  // ping + status da conexão
  static uint32_t ultimoPing = 0;
  if (agora - ultimoPing >= 1000) { ultimoPing = agora; enviarParaE(MSG_PING); }
  static bool conectadoAntes = false;
  bool conectado = ultimoContatoE && (agora - ultimoContatoE < 3000);
  if (conectado != conectadoAntes) {
    conectadoAntes = conectado;
    ledPlaca(conectado ? 0 : 40, conectado ? 40 : 0, 0);
    Serial.println(conectado ? "== Placa E CONECTADA ==" : "== Placa E SEM SINAL ==");
    if (conectado) mudou = true;            // manda o LCD atual assim que conectar
  }

  if (mudou) {
    desenharMapa();
    desenharStatus();
    enviarLcd();
  }

  // a barra de Corrupção "chia" sozinha quando está alta
  static uint32_t ultimoChiado = 0;
  if (corrupcao >= 67 && agora - ultimoChiado >= 150) { ultimoChiado = agora; desenharStatus(); }
}
