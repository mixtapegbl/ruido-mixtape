// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// PEDAL · Placa E (controle) — botões, encoder, LCD, OLED, DHT11 e som
// Testa: LCD 16x2, OLED 0.96" (SSD1306), 5 botões (clique e segurar), encoder EC11
//        com clique, buzzer, LED RGB, DHT11, LED onboard e o rádio nos dois sentidos.
// Grave o AMP na Placa V. Ponto de partida: teste 3.
//
// Arduino IDE: ESP32S3 Dev Module · Flash 16MB · PSRAM OPI
// Bibliotecas: "LiquidCrystal I2C" (Frank de Brabander), "DHT sensor library" (Adafruit)
//             e "Adafruit SSD1306" (instala junto a "Adafruit GFX")
// ============================================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- MATRIZ DE PINOS DA PLACA E ---------------------------------------------
#define I2C_SDA     8
#define I2C_SCL     9
#define BTN_MENU    1
#define BTN_CIMA    2
#define BTN_BAIXO   4
#define BTN_OK      5
#define BTN_VOLTAR  6
#define PINO_DHT    7
#define ENC_A      10   // EC11: pino da ponta (lado de 3 pinos)
#define ENC_B      11   // EC11: pino da outra ponta
#define ENC_SW     12   // EC11: um dos 2 pinos do botão (o outro vai no GND)
                        // EC11: pino do MEIO (C) vai no GND · não usa VCC
#define TRANSICOES_POR_CLIQUE 4   // se precisar de 2 cliques para andar 1, troque para 2
#define BUZZER     13
#define LED_R      14
#define LED_G      21
#define LED_B      47
#define LED_PLACA  48

#include "../../comum/mensagem.h"   // Mensagem, MACs e canal do rádio

const char* NOMES[] = { "MENU", "CIMA", "BAIXO", "OK", "VOLTAR", "ENC OK", "ENC GIRO" };

// --- OBJETOS -----------------------------------------------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);   // endereço 0x3C
bool oledOk = false;
DHT dht(PINO_DHT, DHT11);

// o que aparece no OLED
char oledBotao[12] = "---";
int  oledEncoder = 0;
float oledTemp = NAN, oledUmid = NAN;
bool oledConectado = false;
bool oledMudou = true;

volatile uint32_t ultimoContatoV = 0;   // millis da última mensagem recebida da V
volatile bool chegouLcd = false;
Mensagem lcdRecebido;
uint32_t contadorEnvio = 0;

// --- BOTÕES: debounce + segurar 2 s ----------------------------------------
struct EstadoBotao {
  uint8_t pino;
  bool apertado = false;
  bool segurouEnviado = false;
  uint32_t desde = 0;
  uint32_t ultimaMudanca = 0;
};
EstadoBotao botoes[] = { {BTN_MENU}, {BTN_CIMA}, {BTN_BAIXO}, {BTN_OK}, {BTN_VOLTAR}, {ENC_SW} };
const uint8_t QTD_BOTOES = sizeof(botoes) / sizeof(botoes[0]);

// --- ENCODER -----------------------------------------------------------------
volatile int encPassos = 0;
volatile uint8_t encUltimo = 0;
void IRAM_ATTR lerEncoder() {
  // tabela de quadratura: ignora ruído e conta 1 passo por "clique"
  static const int8_t tabela[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
  static int8_t acumulado = 0;
  uint8_t agora = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);
  acumulado += tabela[(encUltimo << 2) | agora];
  encUltimo = agora;
  if (acumulado >= TRANSICOES_POR_CLIQUE)  { encPassos++; acumulado = 0; }
  if (acumulado <= -TRANSICOES_POR_CLIQUE) { encPassos--; acumulado = 0; }
}

// --- LEDs e som ---------------------------------------------------------------
void ledRgb(bool r, bool g, bool b) { digitalWrite(LED_R, r); digitalWrite(LED_G, g); digitalWrite(LED_B, b); }
void ledPlaca(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(LED_PLACA, r, g, b); }
void bip(uint16_t freq, uint16_t ms) { tone(BUZZER, freq, ms); }

// --- ESP-NOW -------------------------------------------------------------------
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
void aoEnviar(const esp_now_send_info_t* info, esp_now_send_status_t status) {}
#else
void aoEnviar(const uint8_t* mac, esp_now_send_status_t status) {}
#endif

void aoReceber(const esp_now_recv_info_t* info, const uint8_t* dados, int tam) {
  if (tam != sizeof(Mensagem)) return;
  if (memcmp(info->src_addr, macPlacaV, 6) != 0) return;   // só aceita a Placa V
  ultimoContatoV = millis();
  const Mensagem* m = (const Mensagem*)dados;
  if (m->tipo == MSG_LCD) { memcpy(&lcdRecebido, m, sizeof(Mensagem)); chegouLcd = true; }
}

void enviar(Mensagem& m) {
  m.contador = ++contadorEnvio;
  esp_now_send(macPlacaV, (uint8_t*)&m, sizeof(Mensagem));
}

void enviarBotao(uint8_t botao, uint8_t acao, int8_t giro = 0) {
  Mensagem m = {};
  m.tipo = MSG_BOTAO; m.botao = botao; m.acao = acao; m.giro = giro;
  enviar(m);
  if (botao == B_ENC_GIRO) oledEncoder += giro;
  else snprintf(oledBotao, sizeof(oledBotao), "%s%s", NOMES[botao], acao == SEGUROU ? "*" : "");
  oledMudou = true;
  Serial.printf("-> %s %s\n", NOMES[botao], botao == B_ENC_GIRO ? (giro > 0 ? "+1" : "-1") : (acao == SEGUROU ? "(segurou)" : ""));
}

bool iniciarRadio() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL_RADIO, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) return false;
  esp_now_register_send_cb(aoEnviar);
  esp_now_register_recv_cb(aoReceber);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, macPlacaV, 6);
  peer.channel = CANAL_RADIO;
  peer.encrypt = false;
  return esp_now_add_peer(&peer) == ESP_OK;
}

// --- OLED ---------------------------------------------------------------------
void desenharOled() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("RUIDO  Placa E");
  oled.setCursor(92, 0);
  oled.print(oledConectado ? "LINK" : "----");
  if (oledConectado) oled.fillCircle(122, 3, 3, SSD1306_WHITE); else oled.drawCircle(122, 3, 3, SSD1306_WHITE);
  oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  oled.setCursor(0, 14);
  oled.print("Ultimo botao:");
  oled.setTextSize(2);
  oled.setCursor(0, 25);
  oled.print(oledBotao);

  oled.setTextSize(1);
  oled.setCursor(0, 45);
  oled.printf("Encoder: %d", oledEncoder);
  oled.setCursor(0, 56);
  if (isnan(oledTemp)) oled.print("Clima: sem leitura");
  else oled.printf("Clima: %.1fC  %.0f%%", oledTemp, oledUmid);
  oled.display();
}

// ----------------------------------------------------------------------------
void mostrarMac() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char txt[18];
  snprintf(txt, sizeof(txt), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("==============================================");
  Serial.printf ("[PLACA E] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf ("uint8_t macPlacaE[] = {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X};\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("==============================================");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("PLACA E  MAC:");
  lcd.setCursor(0, 1); lcd.print(txt);
}

void setup() {
  Serial.begin(115200);
  delay(1200);

  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
  ledRgb(0, 0, 1);
  ledPlaca(0, 0, 40);
  for (uint8_t i = 0; i < QTD_BOTOES; i++) pinMode(botoes[i].pino, INPUT_PULLUP);
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  encUltimo = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);
  attachInterrupt(digitalPinToInterrupt(ENC_A), lerEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), lerEncoder, CHANGE);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  lcd.init();
  lcd.backlight();
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.printf("OLED: %s\n", oledOk ? "OK" : "não encontrado (confira SDA 8, SCL 9 e endereço 0x3C)");
  if (oledOk) { oled.clearDisplay(); oled.display(); }
  dht.begin();

  mostrarMac();
  bip(1200, 80);
  delay(2500);

  bool ok = iniciarRadio();
  Serial.printf("ESP-NOW: %s (canal %d)\n", ok ? "OK" : "FALHOU", CANAL_RADIO);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(ok ? "Radio OK" : "Radio FALHOU");
  lcd.setCursor(0, 1); lcd.print("Procurando V...");
}

void loop() {
  uint32_t agora = millis();

  // --- botões ---
  for (uint8_t i = 0; i < QTD_BOTOES; i++) {
    EstadoBotao& b = botoes[i];
    bool lido = digitalRead(b.pino) == LOW;
    if (lido != b.apertado && agora - b.ultimaMudanca > 30) {   // debounce 30 ms
      b.ultimaMudanca = agora;
      b.apertado = lido;
      if (lido) { b.desde = agora; b.segurouEnviado = false; }
      else if (!b.segurouEnviado) { enviarBotao(i, CLIQUE); bip(1800, 30); }
    }
    if (b.apertado && !b.segurouEnviado && agora - b.desde >= 2000) {
      b.segurouEnviado = true;
      enviarBotao(i, SEGUROU);
      bip(900, 120);
    }
  }

  // --- encoder ---
  noInterrupts(); int passos = encPassos; encPassos = 0; interrupts();
  while (passos > 0) { enviarBotao(B_ENC_GIRO, CLIQUE, +1); bip(2400, 8); passos--; }
  while (passos < 0) { enviarBotao(B_ENC_GIRO, CLIQUE, -1); bip(2000, 8); passos++; }

  // --- ping e clima ---
  static uint32_t ultimoPing = 0, ultimoClima = 0;
  if (agora - ultimoPing >= 1000) {
    ultimoPing = agora;
    Mensagem m = {}; m.tipo = MSG_PING; enviar(m);
  }
  if (agora - ultimoClima >= 5000) {        // no jogo vai ser a cada 30 s
    ultimoClima = agora;
    Mensagem m = {}; m.tipo = MSG_CLIMA;
    m.temperatura = dht.readTemperature();
    m.umidade     = dht.readHumidity();
    enviar(m);
    oledTemp = m.temperatura; oledUmid = m.umidade; oledMudou = true;
    Serial.printf("-> CLIMA %.1f C  %.0f %%\n", m.temperatura, m.umidade);
  }

  // --- texto do LCD vindo da V ---
  if (chegouLcd) {
    chegouLcd = false;
    lcd.setCursor(0, 0); lcd.printf("%-16s", lcdRecebido.linha1);
    lcd.setCursor(0, 1); lcd.printf("%-16s", lcdRecebido.linha2);
  }

  // --- status da conexão: LED onboard + LED RGB ---
  static bool conectadoAntes = false;
  bool conectado = ultimoContatoV && (agora - ultimoContatoV < 3000);
  if (conectado != conectadoAntes) {
    conectadoAntes = conectado;
    oledConectado = conectado; oledMudou = true;
    if (conectado) { ledPlaca(0, 40, 0); ledRgb(0, 1, 0); bip(1500, 60); Serial.println("== CONECTADO na Placa V =="); }
    else {
      ledPlaca(40, 0, 0); ledRgb(1, 0, 0); Serial.println("== SEM SINAL da Placa V ==");
      lcd.setCursor(0, 0); lcd.print("   SEM SINAL    ");
      lcd.setCursor(0, 1); lcd.print("Ligue a Placa V ");
    }
  }

  // --- OLED (no máximo 20 vezes por segundo) ---
  static uint32_t ultimoOled = 0;
  if (oledMudou && agora - ultimoOled >= 50) {
    ultimoOled = agora;
    oledMudou = false;
    desenharOled();
  }
}
