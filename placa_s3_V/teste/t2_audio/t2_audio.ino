// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 2 · ÁUDIO (Placa V → UDA1334A → amplificador TPA3116D2)
// Etapa A: tom de teste gerado no código (não precisa de SD) → confere a fiação.
// Etapa B: toca /som/teste.wav do cartão SD (WAV PCM 16 bits, mono ou estéreo).
//
// COMECE COM O VOLUME DO AMPLIFICADOR NO MÍNIMO e vá subindo devagar.
//
// Comandos no Monitor Serial (115200, "Nova linha" ou "Nenhum final"):
//   t = tom de teste (lá 440 Hz, 2 s)     s = sirene (sobe e desce)
//   w = tocar /som/teste.wav              p = parar
//   + = mais volume                       - = menos volume
//   i = diagnóstico (leitura mais lenta do SD durante a música)
//
// v2: leitura do SD em blocos grandes (menos picotes), entrada e saída
//     suaves (fade) para não estalar ao começar/parar, e diagnóstico.
// ============================================================================

#include <SPI.h>
#include <SD.h>
#include <ESP_I2S.h>

// --- PINOS (matriz da Placa V) ----------------------------------------------
#define I2S_BCLK   15
#define I2S_WSEL   16
#define I2S_DIN    17

#define PINO_SCK   12
#define PINO_MOSI  11
#define PINO_MISO  13
#define SD_CS      14
#define T20_CS     10   // CS das telas em HIGH para não atrapalhar o SD
#define T09_CS      6
#define LED_PLACA  48

#define TAXA_TOM   44100
const char* ARQUIVO_WAV = "/som/teste.wav";

I2SClass i2s;
bool sdOk = false;
float volume = 0.30f;          // começa baixo (0.0 a 1.0)

enum Estado { PARADO, TOM, SIRENE, WAV };
Estado estado = PARADO;

// tom / sirene
float fase = 0;
uint32_t amostrasTom = 0;
uint32_t taxaAtual = 0;

// wav
#define BLOCO_WAV 8192         // bytes lidos do SD por vez (maior = menos picotes)
File wav;
uint32_t leituraMaisLentaUs = 0;
uint32_t leiturasLentas = 0;   // leituras que demoraram mais que o buffer do I2S
float ganhoFade = 0;           // 0 → 1 no começo, 1 → 0 ao parar
uint16_t wavCanais = 0;
uint32_t wavTaxa = 0;
uint32_t wavBytesRestantes = 0;

void led(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(LED_PLACA, r, g, b); }

bool configurarI2S(uint32_t taxa) {
  if (taxa == taxaAtual) return true;
  i2s.end();
  i2s.setPins(I2S_BCLK, I2S_WSEL, I2S_DIN);
  bool ok = i2s.begin(I2S_MODE_STD, taxa, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  taxaAtual = ok ? taxa : 0;
  Serial.printf("  I2S a %lu Hz: %s\n", (unsigned long)taxa, ok ? "OK" : "FALHOU");
  return ok;
}

// ---------------------------------------------------------------- TOM / SIRENE
void iniciarTom(Estado tipo) {
  pararTudo();
  if (!configurarI2S(TAXA_TOM)) return;
  fase = 0;
  amostrasTom = 0;
  estado = tipo;
  led(0, 0, 40);
  Serial.println(tipo == TOM ? "> Tom de 440 Hz (2 s)" : "> Sirene (3 s)");
}

void tocarTom() {
  static int16_t buf[256 * 2];                 // 256 amostras estéreo
  const uint32_t duracao = (estado == TOM ? 2 : 3) * TAXA_TOM;
  for (int i = 0; i < 256; i++) {
    float freq = 440.0f;
    if (estado == SIRENE) {                    // 300 Hz → 1200 Hz → 300 Hz
      float t = (float)(amostrasTom % TAXA_TOM) / TAXA_TOM;
      freq = 300.0f + 900.0f * (t < 0.5f ? t * 2 : (1 - t) * 2);
    }
    fase += 2.0f * PI * freq / TAXA_TOM;
    if (fase > 2.0f * PI) fase -= 2.0f * PI;
    int16_t s = (int16_t)(sinf(fase) * 32767.0f * volume);
    buf[i * 2]     = s;                        // esquerdo
    buf[i * 2 + 1] = s;                        // direito
    amostrasTom++;
  }
  i2s.write((uint8_t*)buf, sizeof(buf));
  if (amostrasTom >= duracao) pararTudo();
}

// ---------------------------------------------------------------------- WAV
uint32_t ler32(File& f) { uint8_t b[4]; f.read(b, 4); return b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t)b[3] << 24); }
uint16_t ler16(File& f) { uint8_t b[2]; f.read(b, 2); return b[0] | (b[1] << 8); }

void iniciarWav() {
  pararTudo();
  if (!sdOk) { Serial.println("> SD não montou: não dá para tocar o WAV"); return; }
  wav = SD.open(ARQUIVO_WAV, FILE_READ);
  if (!wav) { Serial.printf("> Não achei %s no cartão\n", ARQUIVO_WAV); return; }

  char id[5] = {0};
  wav.read((uint8_t*)id, 4); ler32(wav);
  char tipo[5] = {0}; wav.read((uint8_t*)tipo, 4);
  if (strcmp(id, "RIFF") || strcmp(tipo, "WAVE")) { Serial.println("> Não é um arquivo WAV"); wav.close(); return; }

  uint16_t formato = 0, bits = 0;
  wavBytesRestantes = 0;
  while (wav.available()) {                    // procura os blocos "fmt " e "data"
    char bloco[5] = {0};
    wav.read((uint8_t*)bloco, 4);
    uint32_t tam = ler32(wav);
    if (!strcmp(bloco, "fmt ")) {
      formato   = ler16(wav);
      wavCanais = ler16(wav);
      wavTaxa   = ler32(wav);
      ler32(wav); ler16(wav);
      bits      = ler16(wav);
      if (tam > 16) wav.seek(wav.position() + (tam - 16));
    } else if (!strcmp(bloco, "data")) {
      wavBytesRestantes = tam;
      break;
    } else {
      wav.seek(wav.position() + tam + (tam & 1));
    }
  }
  Serial.printf("> WAV: formato %u, %u canal(is), %lu Hz, %u bits, %lu bytes de som\n",
                formato, wavCanais, (unsigned long)wavTaxa, bits, (unsigned long)wavBytesRestantes);
  if (formato != 1 || bits != 16 || wavCanais < 1 || wavCanais > 2 || wavBytesRestantes == 0) {
    Serial.println("> Formato não suportado: exporte como WAV PCM 16 bits (mono ou estéreo)");
    wav.close();
    return;
  }
  if (!configurarI2S(wavTaxa)) { wav.close(); return; }
  leituraMaisLentaUs = 0;
  leiturasLentas = 0;
  ganhoFade = 0;
  estado = WAV;
  led(0, 40, 40);
}

void tocarWav() {
  static int16_t entrada[BLOCO_WAV / 2];
  static int16_t saida[BLOCO_WAV];            // espaço para mono → estéreo
  size_t pedir = min<uint32_t>(BLOCO_WAV, wavBytesRestantes);

  uint32_t t0 = micros();
  size_t lidos = wav.read((uint8_t*)entrada, pedir);
  uint32_t dt = micros() - t0;
  if (dt > leituraMaisLentaUs) leituraMaisLentaUs = dt;
  // o I2S guarda 6 x 240 quadros: ~32 ms a 44,1 kHz, ~65 ms a 22 kHz
  uint32_t limiteUs = (uint32_t)(1440.0f * 1000000.0f / wavTaxa);
  if (dt > limiteUs) leiturasLentas++;

  if (lidos == 0) { Serial.println("> Fim do WAV"); pararTudo(); return; }
  wavBytesRestantes -= lidos;

  size_t amostras = lidos / 2;
  size_t quadros = wavCanais == 1 ? amostras : amostras / 2;
  float passoFade = 1.0f / (wavTaxa * 0.02f);  // fade de 20 ms
  size_t n = 0;
  for (size_t q = 0; q < quadros; q++) {
    if (ganhoFade < 1.0f) ganhoFade = min(1.0f, ganhoFade + passoFade);
    float g = volume * ganhoFade;
    if (wavCanais == 1) {
      int16_t s = (int16_t)(entrada[q] * g);
      saida[n++] = s; saida[n++] = s;
    } else {
      saida[n++] = (int16_t)(entrada[q * 2] * g);
      saida[n++] = (int16_t)(entrada[q * 2 + 1] * g);
    }
  }
  i2s.write((uint8_t*)saida, n * 2);
  if (wavBytesRestantes == 0) { Serial.println("> Fim do WAV"); diagnostico(); pararTudo(); }
}

void diagnostico() {
  if (!wavTaxa) { Serial.println("> Toque o WAV primeiro (w)"); return; }
  uint32_t limiteUs = (uint32_t)(1440.0f * 1000000.0f / wavTaxa);
  Serial.printf("> Leitura mais lenta do SD: %.1f ms (limite antes de picotar: %.1f ms)\n",
                leituraMaisLentaUs / 1000.0f, limiteUs / 1000.0f);
  Serial.printf("> Leituras lentas demais: %lu %s\n", (unsigned long)leiturasLentas,
                leiturasLentas ? "→ os picotes vêm do SD" : "→ o SD não é a causa dos picotes");
}

// --------------------------------------------------------------------------
void pararTudo() {
  if (estado == WAV && wav) {
    // um bloco de silêncio antes de fechar, para não estalar
    static int16_t fim[256 * 2] = {0};
    if (taxaAtual) i2s.write((uint8_t*)fim, sizeof(fim));
    wav.close();
  }
  if (estado != PARADO) {
    static int16_t silencio[256 * 2] = {0};
    if (taxaAtual) i2s.write((uint8_t*)silencio, sizeof(silencio));
  }
  estado = PARADO;
  led(0, 40, 0);
}

void ajuda() {
  Serial.println();
  Serial.println("Comandos: t = tom 440 Hz | s = sirene | w = tocar WAV | p = parar | + / - = volume | i = diagnóstico");
  Serial.printf("Volume atual: %d%%\n", (int)(volume * 100));
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  led(0, 0, 40);

  pinMode(T20_CS, OUTPUT); digitalWrite(T20_CS, HIGH);
  pinMode(T09_CS, OUTPUT); digitalWrite(T09_CS, HIGH);

  Serial.println();
  Serial.println("========== TESTE DE ÁUDIO · PLACA V ==========");
  Serial.println("Volume do amplificador no MÍNIMO antes de começar!");

  SPI.begin(PINO_SCK, PINO_MISO, PINO_MOSI, SD_CS);
  sdOk = SD.begin(SD_CS, SPI, 20000000);
  Serial.printf("SD: %s\n", sdOk ? "OK" : "não montou (o tom de teste funciona mesmo assim)");
  if (sdOk) Serial.printf("%s: %s\n", ARQUIVO_WAV, SD.exists(ARQUIVO_WAV) ? "encontrado" : "não encontrado");

  ajuda();
  delay(500);
  iniciarTom(TOM);             // toca o tom uma vez sozinho ao ligar
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 't': iniciarTom(TOM); break;
      case 's': iniciarTom(SIRENE); break;
      case 'w': iniciarWav(); break;
      case 'p': pararTudo(); Serial.println("> Parado"); break;
      case 'i': diagnostico(); break;
      case '+': volume = min(1.0f, volume + 0.1f); Serial.printf("> Volume: %d%%\n", (int)(volume * 100 + 0.5f)); break;
      case '-': volume = max(0.0f, volume - 0.1f); Serial.printf("> Volume: %d%%\n", (int)(volume * 100 + 0.5f)); break;
      case '\n': case '\r': break;
      default: ajuda();
    }
  }

  if (estado == TOM || estado == SIRENE) tocarTom();
  else if (estado == WAV) tocarWav();
}
