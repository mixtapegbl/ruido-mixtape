// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 6 · ÁUDIO 8 BITS PELO UDA1334A (CAIXA · ESP32 DevKit V1 clássica)
//
// Um sintetizador estilo Game Boy / NES feito só com código: 3 vozes
// (pulso, triângulo e ruído), músicas curtas e efeitos. Não precisa de SD.
// O som é calculado em 8 bits (a "cara" de console antigo) e sai pelo
// módulo DAC UDA1334A por I2S, que entrega um sinal limpo para o amplificador.
//
// LIGAÇÃO DO UDA1334A (volume do amplificador no MÍNIMO antes de ligar):
//   BCLK  <- GPIO 27
//   WSEL  <- GPIO 14
//   DIN   <- GPIO 13
//   VIN   <- 3V3 da DevKit (aceita 3,3 a 5 V, NUNCA 12 V)
//   GND   <- GND da DevKit
//   Lout / Rout / AGND -> entrada do amplificador (ou o P2 do próprio módulo num fone)
//   Os outros pinos (SCLK, PLL, MUTE, DEEM, SF0, SF1) ficam desligados.
//
// Arduino IDE: placa "ESP32 Dev Module" · Monitor Serial 115200
//
// COMANDOS (uma letra e Enter):
//   1 tema   2 fábrica   3 batalha   4 level up   5 game over   p parar música
//   g golpe  c crítico   d dano      m moeda      a altar
//   w troca a onda da melodia (12,5% · 25% · 50% · triângulo)
//   + / -    volume geral
//   r        "Ruído" (Corrupção): 0 limpo → 1 → 2 → 3 bem destruído
//   h        ajuda
// ============================================================================

#include <Arduino.h>
#include <ESP_I2S.h>

#define I2S_BCLK    27
#define I2S_WSEL    14
#define I2S_DIN     13

#define TAXA        22050      // amostras por segundo (baixo de propósito: som de console antigo)
#define BLOCO       256        // amostras geradas por vez
#define FRAME_HZ    60         // o "relógio" das músicas, como nos consoles antigos

I2SClass i2s;

// --------------------------------------------------------------- VOZES
enum Onda : uint8_t { PULSO_12, PULSO_25, PULSO_50, TRIANGULO, RUIDO };

struct Voz {
  Onda     onda = PULSO_25;
  uint32_t fase = 0;
  uint32_t passo = 0;          // quanto a fase anda por amostra (define a nota)
  int16_t  volume = 0;         // 0 a 15
  int16_t  queda = 0;          // quanto o volume cai por frame (0 = segura)
  uint16_t lfsr = 0x7FFF;      // gerador do ruído
  uint8_t  ruidoContador = 0;
};

Voz vozes[3];                  // 0 = melodia (pulso), 1 = baixo (triângulo), 2 = bateria (ruído)
Voz efeito;                    // voz extra só para os efeitos, toca por cima da música
volatile int volumeGeral = 6;  // 0 a 10
volatile int nivelRuido = 0;   // 0 a 3 (a "Corrupção" no som)
volatile Onda ondaMelodia = PULSO_25;

uint32_t passoDaNota(int midi) {                    // nota MIDI → passo da fase
  if (midi <= 0) return 0;
  float freq = 440.0f * powf(2.0f, (midi - 69) / 12.0f);
  return (uint32_t)(freq * 4294967296.0f / TAXA);
}

void tocarNota(Voz& v, int midi, int volume, int queda) {
  v.passo = passoDaNota(midi);
  v.volume = volume;
  v.queda = queda;
  if (v.onda == RUIDO) v.passo = passoDaNota(midi > 0 ? midi : 90);
}

int8_t amostraDa(Voz& v) {
  if (v.volume <= 0 || v.passo == 0) return 0;
  v.fase += v.passo;
  int s;
  switch (v.onda) {
    case PULSO_12:  s = (v.fase < 0x20000000u) ? 127 : -127; break;
    case PULSO_25:  s = (v.fase < 0x40000000u) ? 127 : -127; break;
    case PULSO_50:  s = (v.fase < 0x80000000u) ? 127 : -127; break;
    case TRIANGULO: { uint32_t f = v.fase >> 24; s = (f < 128) ? (f * 2 - 127) : (383 - f * 2); break; }
    case RUIDO: default:
      if ((v.fase >> 28) != v.ruidoContador) {         // troca o valor do ruído no ritmo da "nota"
        v.ruidoContador = v.fase >> 28;
        uint16_t bit = ((v.lfsr >> 0) ^ (v.lfsr >> 1)) & 1;
        v.lfsr = (v.lfsr >> 1) | (bit << 14);
      }
      s = (v.lfsr & 1) ? 127 : -127;
  }
  return (int8_t)((s * v.volume) / 15);
}

void quedaDoFrame(Voz& v) {
  if (v.queda > 0 && v.volume > 0) { v.volume -= v.queda; if (v.volume < 0) v.volume = 0; }
}

// --------------------------------------------------------------- MÚSICAS
// Cada música tem 3 trilhas de passos: número = nota MIDI, 0 = segura a anterior, -1 = silêncio.
// Na trilha da bateria: 1 = caixa (ruído curto), 2 = prato (ruído agudo), 3 = bumbo (ruído grave).
struct Musica {
  const char* nome;
  uint8_t framesPorPasso;
  uint8_t passos;
  bool repete;
  const int8_t* melodia;
  const int8_t* baixo;
  const int8_t* bateria;
};

// 1 · TEMA: lento, menor, sombrio
const int8_t temaM[] = { 69,0,72,0, 76,0,74,0, 72,0,71,0, 69,0,0,-1,  64,0,67,0, 71,0,69,0, 67,0,64,0, 65,0,0,-1 };
const int8_t temaB[] = { 45,0,0,0, 0,0,0,0,   41,0,0,0, 0,0,0,0,     36,0,0,0, 0,0,0,0,     40,0,0,0, 0,0,0,0 };
const int8_t temaD[] = { 3,0,0,0, 0,0,0,0, 3,0,0,0, 0,0,2,0,  3,0,0,0, 0,0,0,0, 3,0,0,0, 0,0,2,0 };

// 2 · FÁBRICA: industrial, repetitivo, com "batidas de metal"
const int8_t fabM[] = { -1,0,0,0, 76,-1,76,-1, -1,0,0,0, 79,-1,77,-1,  -1,0,0,0, 76,-1,76,-1, -1,0,0,0, 74,0,72,-1 };
const int8_t fabB[] = { 40,-1,40,-1, 0,-1,43,-1, 40,-1,40,-1, 0,-1,38,-1,  40,-1,40,-1, 0,-1,43,-1, 40,-1,45,-1, 43,-1,38,-1 };
const int8_t fabD[] = { 3,0,1,0, 3,2,1,2, 3,0,1,0, 3,2,1,1,  3,0,1,0, 3,2,1,2, 3,0,1,0, 3,1,1,1 };

// 3 · BATALHA: rápida e tensa
const int8_t batM[] = { 69,71,72,74, 76,0,74,72, 71,0,69,71, 72,0,-1,0,  69,71,72,74, 77,0,76,74, 72,74,76,0, 81,0,-1,0 };
const int8_t batB[] = { 45,45,57,45, 45,45,57,45, 41,41,53,41, 41,41,53,41,  43,43,55,43, 43,43,55,43, 40,40,52,40, 44,44,56,44 };
const int8_t batD[] = { 3,2,1,2, 3,2,1,2, 3,2,1,2, 3,2,1,1,  3,2,1,2, 3,2,1,2, 3,2,1,2, 3,1,3,1 };

// 4 · LEVEL UP: curtinha, toca uma vez
const int8_t upM[] = { 72,76,79,84, 0,0,0,-1 };
const int8_t upB[] = { 48,52,55,60, 0,0,0,-1 };
const int8_t upD[] = { 2,0,2,0, 2,0,0,0 };

// 5 · GAME OVER: descendo, toca uma vez
const int8_t goM[] = { 76,0,74,0, 72,0,71,0, 69,0,68,0, 69,0,0,0, 0,0,0,-1 };
const int8_t goB[] = { 45,0,0,0, 44,0,0,0, 43,0,0,0, 33,0,0,0, 0,0,0,-1 };
const int8_t goD[] = { 3,0,0,0, 0,0,0,0, 0,0,0,0, 3,0,0,0, 0,0,0,0 };

const Musica MUSICAS[] = {
  { "Tema",      12, sizeof(temaM), true,  temaM, temaB, temaD },
  { "Fabrica",    7, sizeof(fabM),  true,  fabM,  fabB,  fabD  },
  { "Batalha",    5, sizeof(batM),  true,  batM,  batB,  batD  },
  { "Level up",   5, sizeof(upM),   false, upM,   upB,   upD   },
  { "Game over", 10, sizeof(goM),   false, goM,   goB,   goD   },
};

volatile int musicaAtual = -1;   // -1 = sem música
volatile int pedirMusica = -2;   // pedido vindo do Serial (-2 = nada)
int passoAtual = 0, framesNoPasso = 0;

void tocarPasso() {
  const Musica& m = MUSICAS[musicaAtual];
  int8_t n;
  n = m.melodia[passoAtual];
  if (n > 0) { vozes[0].onda = ondaMelodia; tocarNota(vozes[0], n, 10, 0); } else if (n < 0) vozes[0].volume = 0;
  n = m.baixo[passoAtual];
  if (n > 0) tocarNota(vozes[1], n, 15, 0); else if (n < 0) vozes[1].volume = 0;
  n = m.bateria[passoAtual];
  if (n == 1) tocarNota(vozes[2], 84, 9, 2);        // caixa
  if (n == 2) tocarNota(vozes[2], 100, 5, 3);       // prato
  if (n == 3) tocarNota(vozes[2], 60, 12, 3);       // bumbo
}

void avancarMusica() {
  if (musicaAtual < 0) return;
  const Musica& m = MUSICAS[musicaAtual];
  if (framesNoPasso == 0) tocarPasso();
  if (++framesNoPasso >= m.framesPorPasso) {
    framesNoPasso = 0;
    if (++passoAtual >= m.passos) {
      if (m.repete) passoAtual = 0;
      else { musicaAtual = -1; for (auto& v : vozes) v.volume = 0; }
    }
  }
}

// --------------------------------------------------------------- EFEITOS
// Um efeito é uma lista de (nota, frames); toca na voz extra, por cima da música.
struct PassoEfeito { int8_t nota; uint8_t frames; Onda onda; uint8_t volume; };
const PassoEfeito FX_GOLPE[]   = { {50, 2, RUIDO, 15}, {40, 4, RUIDO, 10}, {0, 0, RUIDO, 0} };
const PassoEfeito FX_CRITICO[] = { {84, 2, PULSO_12, 15}, {91, 2, PULSO_12, 15}, {96, 3, PULSO_12, 15}, {60, 6, RUIDO, 14}, {0, 0, RUIDO, 0} };
const PassoEfeito FX_DANO[]    = { {55, 3, PULSO_50, 15}, {49, 3, PULSO_50, 12}, {43, 6, PULSO_50, 9}, {0, 0, RUIDO, 0} };
const PassoEfeito FX_MOEDA[]   = { {83, 3, PULSO_25, 12}, {88, 8, PULSO_25, 12}, {0, 0, RUIDO, 0} };
const PassoEfeito FX_ALTAR[]   = { {72, 6, TRIANGULO, 15}, {76, 6, TRIANGULO, 15}, {79, 6, TRIANGULO, 15}, {84, 14, TRIANGULO, 15}, {0, 0, RUIDO, 0} };

const PassoEfeito* volatile efeitoPedido = nullptr;
const PassoEfeito* efeitoAtual = nullptr;
int efeitoFrames = 0;

void avancarEfeito() {
  if (!efeitoAtual) return;
  if (efeitoFrames == 0) {
    if (efeitoAtual->frames == 0) { efeitoAtual = nullptr; efeito.volume = 0; return; }
    efeito.onda = efeitoAtual->onda;
    tocarNota(efeito, efeitoAtual->nota, efeitoAtual->volume, 0);
  }
  if (++efeitoFrames >= efeitoAtual->frames) { efeitoFrames = 0; efeitoAtual++; }
}

// --------------------------------------------------------------- TAREFA DE ÁUDIO
void tarefaAudio(void*) {
  static int16_t buf[BLOCO * 2];                    // estéreo: esquerdo, direito, esquerdo...
  const int amostrasPorFrame = TAXA / FRAME_HZ;
  int contadorFrame = 0;
  uint8_t ultimaAmostra = 128;
  int seguraAmostra = 0;

  while (true) {
    if (pedirMusica != -2) {                          // pedidos do Serial
      musicaAtual = pedirMusica; pedirMusica = -2;
      passoAtual = 0; framesNoPasso = 0;
      for (auto& v : vozes) v.volume = 0;
      vozes[1].onda = TRIANGULO;
      vozes[2].onda = RUIDO;
    }
    if (efeitoPedido) { efeitoAtual = efeitoPedido; efeitoPedido = nullptr; efeitoFrames = 0; }

    int ruido = nivelRuido;
    int mestre = volumeGeral;
    for (int i = 0; i < BLOCO; i++) {
      if (++contadorFrame >= amostrasPorFrame) {      // 60 vezes por segundo
        contadorFrame = 0;
        avancarMusica();
        avancarEfeito();
        for (auto& v : vozes) quedaDoFrame(v);
        quedaDoFrame(efeito);
      }
      int soma = amostraDa(vozes[0]) + amostraDa(vozes[1]) + amostraDa(vozes[2]) / 2 + amostraDa(efeito);
      int s = (soma * mestre) / 20;                   // mistura e volume geral
      if (s > 127) s = 127; if (s < -127) s = -127;

      // "Ruído" da Corrupção: menos bits, menos amostras e chiado
      if (ruido > 0) {
        int bits = 8 - ruido * 2;                     // 6, 4 ou 2 bits
        s = (s >> (8 - bits)) << (8 - bits);
        if (random(100) < ruido * 3) s += random(-40 * ruido, 40 * ruido);
        if (s > 127) s = 127; if (s < -127) s = -127;
      }
      uint8_t amostra = (uint8_t)(s + 128);
      if (ruido >= 2) {                               // segura amostras: som "picotado" de rádio quebrado
        if (++seguraAmostra < ruido * 2) amostra = ultimaAmostra;
        else seguraAmostra = 0;
      }
      ultimaAmostra = amostra;
      int16_t saida = (int16_t)((amostra - 128) << 8);  // 8 bits -> 16 bits (o UDA1334A recebe 16)
      buf[i * 2]     = saida;                         // esquerdo
      buf[i * 2 + 1] = saida;                         // direito (mesmo som nos dois)
    }
    i2s.write((uint8_t*)buf, sizeof(buf));            // espera o DMA ter espaço
  }
}

// --------------------------------------------------------------- SERIAL
const char* NOMES_ONDA[] = { "pulso 12,5%", "pulso 25%", "pulso 50%", "triangulo" };

void ajuda() {
  Serial.println();
  Serial.println("Musicas:  1 tema  2 fabrica  3 batalha  4 level up  5 game over  p parar");
  Serial.println("Efeitos:  g golpe  c critico  d dano  m moeda  a altar");
  Serial.println("Som:      w onda da melodia  + / - volume  r ruido (corrupcao)  h ajuda");
  Serial.printf ("Agora:    volume %d/10 · onda %s · ruido %d/3\n", volumeGeral, NOMES_ONDA[ondaMelodia], nivelRuido);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("========== TESTE 6 · AUDIO 8 BITS · CAIXA ==========");
  Serial.println("Volume do amplificador no MINIMO!");

  i2s.setPins(I2S_BCLK, I2S_WSEL, I2S_DIN);
  bool ok = i2s.begin(I2S_MODE_STD, TAXA, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  Serial.printf("UDA1334A (BCLK %d, WSEL %d, DIN %d) a %d Hz: %s\n", I2S_BCLK, I2S_WSEL, I2S_DIN, TAXA, ok ? "OK" : "FALHOU");
  if (!ok) { Serial.println("Sem I2S: confira a placa (ESP32 Dev Module) e os pinos."); return; }

  vozes[0].onda = PULSO_25; vozes[1].onda = TRIANGULO; vozes[2].onda = RUIDO; efeito.onda = PULSO_25;
  xTaskCreatePinnedToCore(tarefaAudio, "audio", 4096, nullptr, 5, nullptr, 1);

  ajuda();
  pedirMusica = 0;                                    // toca o tema ao ligar
}

void loop() {
  if (!Serial.available()) { delay(5); return; }
  char c = Serial.read();
  switch (c) {
    case '1': case '2': case '3': case '4': case '5':
      pedirMusica = c - '1';
      Serial.printf("> Musica: %s\n", MUSICAS[c - '1'].nome); break;
    case 'p': pedirMusica = -1; Serial.println("> Musica parada"); break;
    case 'g': efeitoPedido = FX_GOLPE;   Serial.println("> Efeito: golpe");   break;
    case 'c': efeitoPedido = FX_CRITICO; Serial.println("> Efeito: critico"); break;
    case 'd': efeitoPedido = FX_DANO;    Serial.println("> Efeito: dano");    break;
    case 'm': efeitoPedido = FX_MOEDA;   Serial.println("> Efeito: moeda");   break;
    case 'a': efeitoPedido = FX_ALTAR;   Serial.println("> Efeito: altar");   break;
    case 'w': ondaMelodia = (Onda)((ondaMelodia + 1) % 4); Serial.printf("> Onda da melodia: %s\n", NOMES_ONDA[ondaMelodia]); break;
    case '+': if (volumeGeral < 10) volumeGeral++; Serial.printf("> Volume: %d/10\n", volumeGeral); break;
    case '-': if (volumeGeral > 0) volumeGeral--;  Serial.printf("> Volume: %d/10\n", volumeGeral); break;
    case 'r': nivelRuido = (nivelRuido + 1) % 4; Serial.printf("> Ruido (corrupcao): %d/3\n", nivelRuido); break;
    case '\n': case '\r': break;
    default: ajuda();
  }
}
