// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE GERAL · o "raio-X" de qualquer placa do projeto
// Funciona nas três: PEDAL e AMP (ESP32-S3) e CAIXA (ESP32 DevKit V1).
//
// Mostra: modelo, núcleos, clock, memória (flash, RAM, PSRAM), MACs, versões
// do core e do ESP-IDF, motivo da última reinicialização, temperatura (só S3),
// desempenho da CPU e da memória, e testes de energia e de Wi-Fi.
//
// Comandos no Monitor Serial (115200):
//   i = informações da placa       b = desempenho (CPU e memória)
//   e = teste de energia           w = busca de redes Wi-Fi
//   t = medir tensão num pino      l = piscar o LED da placa
//   h = ajuda
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_idf_version.h>

// --- Medir tensão (comando "t") ------------------------------------------------
// Ligue um divisor de tensão: 5V ── 10 kΩ ── PINO ── 10 kΩ ── GND
// (o pino nunca pode receber mais de 3,3 V; com dois resistores iguais, 5 V viram 2,5 V)
#if CONFIG_IDF_TARGET_ESP32
  #define PINO_TENSAO   34      // DevKit V1: pino só de entrada, bom para isso
  #define LED_PLACA      2
#else
  #define PINO_TENSAO   15      // ESP32-S3: livre no AMP e no PEDAL (ADC2: só funciona com o Wi-Fi desligado)
  #define LED_PLACA     48      // LED RGB da placa
#endif
#define DIVISOR       2.0f      // 10k + 10k = metade; ajuste se usar outros resistores

const char* motivoReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "ligou na energia (normal)";
    case ESP_RST_SW:        return "reinicio pelo codigo (ESP.restart)";
    case ESP_RST_PANIC:     return "TRAVOU (erro no codigo / panic)";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:       return "TRAVOU (watchdog: alguma parte ficou presa)";
    case ESP_RST_BROWNOUT:  return "FALTA DE ENERGIA (brownout: a tensao caiu demais)";
    case ESP_RST_DEEPSLEEP: return "acordou do deep sleep";
    case ESP_RST_EXT:       return "botao de reset";
    case ESP_RST_USB:       return "reinicio pela USB";
    default:                return "desconhecido";
  }
}

void macLinha(const char* nome, esp_mac_type_t tipo) {
  uint8_t m[6];
  if (esp_read_mac(m, tipo) != ESP_OK) return;
  Serial.printf("  %-10s %02X:%02X:%02X:%02X:%02X:%02X\n", nome, m[0], m[1], m[2], m[3], m[4], m[5]);
}

void informacoes() {
  esp_chip_info_t chip;
  esp_chip_info(&chip);
  uint8_t sta[6];
  esp_read_mac(sta, ESP_MAC_WIFI_STA);

  Serial.println();
  Serial.println("================= INFORMACOES DA PLACA =================");
  Serial.printf ("Chip:           %s rev %d · %d nucleo(s)\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf ("Clock:          CPU %lu MHz · cristal %lu MHz · APB %lu MHz\n",
                 (unsigned long)getCpuFrequencyMhz(), (unsigned long)getXtalFrequencyMhz(), (unsigned long)(getApbFrequency() / 1000000));
  Serial.printf ("Radios:         %s%s%s\n",
                 (chip.features & CHIP_FEATURE_WIFI_BGN) ? "Wi-Fi " : "",
                 (chip.features & CHIP_FEATURE_BLE) ? "BLE " : "",
                 (chip.features & CHIP_FEATURE_BT) ? "Bluetooth-Classico" : "");
  Serial.printf ("Flash:          %lu MB a %lu MHz\n",
                 (unsigned long)(ESP.getFlashChipSize() / (1024 * 1024)), (unsigned long)(ESP.getFlashChipSpeed() / 1000000));
  Serial.printf ("PSRAM:          %lu KB (livre %lu KB)%s\n",
                 (unsigned long)(ESP.getPsramSize() / 1024), (unsigned long)(ESP.getFreePsram() / 1024),
                 ESP.getPsramSize() ? "" : "  <- sem PSRAM (ou PSRAM desligada nas Ferramentas)");
  Serial.printf ("RAM interna:    %lu KB · livre %lu KB · maior bloco %lu KB · minimo ja visto %lu KB\n",
                 (unsigned long)(ESP.getHeapSize() / 1024), (unsigned long)(ESP.getFreeHeap() / 1024),
                 (unsigned long)(ESP.getMaxAllocHeap() / 1024), (unsigned long)(ESP.getMinFreeHeap() / 1024));
  Serial.printf ("Programa:       %lu KB usados · %lu KB livres para o app\n",
                 (unsigned long)(ESP.getSketchSize() / 1024), (unsigned long)(ESP.getFreeSketchSpace() / 1024));
  Serial.printf ("Core Arduino:   %d.%d.%d · ESP-IDF %s\n",
                 ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH, esp_get_idf_version());
  Serial.printf ("Ultimo reset:   %s\n", motivoReset(esp_reset_reason()));
#if SOC_TEMP_SENSOR_SUPPORTED
  Serial.printf ("Temperatura:    %.1f C (sensor interno, aproximado)\n", temperatureRead());
#else
  Serial.println("Temperatura:    esta placa nao tem sensor interno confiavel");
#endif
  Serial.printf ("Ligada ha:      %lu s\n", (unsigned long)(millis() / 1000));
  Serial.println("MACs:");
  macLinha("Wi-Fi STA", ESP_MAC_WIFI_STA);
  macLinha("Wi-Fi AP", ESP_MAC_WIFI_SOFTAP);
  macLinha("Bluetooth", ESP_MAC_BT);
  Serial.printf ("Para o codigo (ESP-NOW): {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                 sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
  Serial.println("========================================================");
}

// --- Desempenho ------------------------------------------------------------------
void desempenho() {
  Serial.println();
  Serial.println("================= DESEMPENHO =================");
  const uint32_t N = 2000000;

  volatile uint32_t ai = 1;
  uint32_t t0 = micros();
  for (uint32_t i = 0; i < N; i++) ai = ai * 1103515245u + 12345u;
  float tInt = (micros() - t0) / 1e6f;

  volatile float af = 1.0f;
  t0 = micros();
  for (uint32_t i = 0; i < N; i++) af = af * 1.0000001f + 0.5f;
  float tFloat = (micros() - t0) / 1e6f;

  volatile double ad = 1.0;
  t0 = micros();
  for (uint32_t i = 0; i < N / 4; i++) ad = ad * 1.0000001 + 0.5;
  float tDouble = (micros() - t0) / 1e6f * 4;

  Serial.printf("Inteiro 32 bits:  %.2f milhoes/s\n", N / tInt / 1e6f);
  Serial.printf("Float:            %.2f milhoes/s\n", N / tFloat / 1e6f);
  Serial.printf("Double:           %.2f milhoes/s  (float e %.1fx mais rapido)\n", N / tDouble / 1e6f, tDouble / tFloat);

  const size_t TAM = 64 * 1024;
  uint8_t* a = (uint8_t*)malloc(TAM);
  uint8_t* b = (uint8_t*)malloc(TAM);
  if (a && b) {
    t0 = micros();
    for (int r = 0; r < 20; r++) memcpy(b, a, TAM);
    float s = (micros() - t0) / 1e6f;
    Serial.printf("Copiar RAM interna: %.1f MB/s\n", (TAM * 20) / s / 1e6f);
  }
  free(a); free(b);
  if (ESP.getPsramSize()) {
    uint8_t* pa = (uint8_t*)ps_malloc(TAM);
    uint8_t* pb = (uint8_t*)ps_malloc(TAM);
    if (pa && pb) {
      t0 = micros();
      for (int r = 0; r < 20; r++) memcpy(pb, pa, TAM);
      float s = (micros() - t0) / 1e6f;
      Serial.printf("Copiar PSRAM:       %.1f MB/s\n", (TAM * 20) / s / 1e6f);
    }
    free(pa); free(pb);
  }
  Serial.println("==============================================");
}

// --- Energia ---------------------------------------------------------------------
// Coloca a placa no consumo mais alto (CPU a 100% + rádio transmitindo) por 10 s.
// Se a fonte ou a bateria não aguentar, a placa reinicia; ao voltar, o "Ultimo reset"
// mostra FALTA DE ENERGIA (brownout).
volatile bool cargaLigada = false;
void tarefaCarga(void*) {
  volatile float x = 1.0f;
  while (cargaLigada) { for (int i = 0; i < 10000; i++) x = x * 1.0001f + 0.1f; vTaskDelay(1); }
  vTaskDelete(nullptr);
}

void energia() {
  Serial.println();
  Serial.println("================= TESTE DE ENERGIA =================");
  Serial.println("10 s com a CPU no maximo e o Wi-Fi transmitindo (consumo de pico).");
  Serial.println("Se a placa reiniciar agora, a alimentacao nao aguenta o pico.");
  delay(200);
  cargaLigada = true;
  xTaskCreatePinnedToCore(tarefaCarga, "carga0", 2048, nullptr, 1, nullptr, 0);
  if (ESP.getChipCores() > 1) xTaskCreatePinnedToCore(tarefaCarga, "carga1", 2048, nullptr, 1, nullptr, 1);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  uint32_t fim = millis() + 10000;
  int buscas = 0;
  while (millis() < fim) { WiFi.scanNetworks(); buscas++; Serial.print("."); }
  cargaLigada = false;
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
  Serial.printf("\nPassou! %d buscas de Wi-Fi com a CPU no maximo, sem reiniciar.\n", buscas);
  Serial.println("A alimentacao aguenta o pico desta placa.");
  Serial.println("====================================================");
}

// --- Wi-Fi -----------------------------------------------------------------------
void buscarRedes() {
  Serial.println();
  Serial.println("Buscando redes...");
  WiFi.mode(WIFI_STA);
  uint32_t t0 = millis();
  int n = WiFi.scanNetworks();
  Serial.printf("%d redes em %.1f s:\n", n, (millis() - t0) / 1000.0f);
  for (int i = 0; i < n && i < 15; i++)
    Serial.printf("  %-24s canal %2d  %4d dBm %s\n", WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i),
                  WiFi.RSSI(i) > -60 ? "(otimo)" : WiFi.RSSI(i) > -75 ? "(bom)" : "(fraco)");
  Serial.println("Lembrete: o ESP-NOW precisa estar no MESMO canal da rede que a placa usar.");
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
}

// --- Tensão ----------------------------------------------------------------------
void medirTensao() {
  analogReadResolution(12);
  uint32_t soma = 0;
  for (int i = 0; i < 64; i++) soma += analogReadMilliVolts(PINO_TENSAO);
  float vPino = soma / 64.0f / 1000.0f;
  Serial.printf("\nPino %d: %.3f V  ->  entrada estimada: %.2f V (divisor %.1fx)\n", PINO_TENSAO, vPino, vPino * DIVISOR, DIVISOR);
  Serial.println("Sem divisor ligado, o valor nao significa nada. Meca com a placa trabalhando para ver a queda.");
}

// --- LED -------------------------------------------------------------------------
void piscarLed() {
  Serial.println("\nPiscando o LED da placa...");
#if CONFIG_IDF_TARGET_ESP32
  pinMode(LED_PLACA, OUTPUT);
  for (int i = 0; i < 6; i++) { digitalWrite(LED_PLACA, i % 2 == 0); delay(250); }
  digitalWrite(LED_PLACA, LOW);
#else
  const uint8_t cores[][3] = { {40, 0, 0}, {0, 40, 0}, {0, 0, 40}, {40, 40, 40}, {0, 0, 0} };
  for (auto& c : cores) { rgbLedWrite(LED_PLACA, c[0], c[1], c[2]); delay(400); }
#endif
}

void ajuda() {
  Serial.println();
  Serial.println("i = informacoes   b = desempenho   e = energia   w = Wi-Fi   t = tensao   l = LED   h = ajuda");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  informacoes();
  ajuda();
}

void loop() {
  if (!Serial.available()) { delay(10); return; }
  switch (Serial.read()) {
    case 'i': informacoes(); break;
    case 'b': desempenho(); break;
    case 'e': energia(); break;
    case 'w': buscarRedes(); break;
    case 't': medirTensao(); break;
    case 'l': piscarLed(); break;
    case '\n': case '\r': break;
    default: ajuda();
  }
}
