// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// comum/mensagem.h · o "idioma" do rádio entre o AMP (Placa V) e o PEDAL (Placa E)
//
// AMP e PEDAL incluem ESTE arquivo:  #include "../../comum/mensagem.h"
// Mudou algo aqui? Grave de novo nas DUAS placas, senão elas param de se entender
// (o receptor descarta mensagens com tamanho diferente).
// ============================================================================
#pragma once
#include <stdint.h>

// --- Endereços (MAC) confirmados nos testes ---------------------------------
static const uint8_t macPlacaV[6] = {0xA4, 0xCB, 0x8F, 0xD4, 0xAB, 0x34};   // AMP (console)
static const uint8_t macPlacaE[6] = {0xA4, 0xCB, 0x8F, 0xD3, 0x4F, 0x00};   // PEDAL (controle)

#define CANAL_RADIO 1   // até o Wi-Fi entrar (design.md, seção 11)

// --- Tipos de mensagem --------------------------------------------------------
enum TipoMsg : uint8_t {
  MSG_PING  = 1,   // as duas, a cada 1 s: mantém o LED de conexão verde
  MSG_BOTAO = 2,   // PEDAL -> AMP: botão, clique/segurou, giro do encoder
  MSG_LCD   = 3,   // AMP -> PEDAL: as duas linhas do LCD 16x2
  MSG_CLIMA = 4    // PEDAL -> AMP: temperatura e umidade do DHT11
};

enum Botao : uint8_t { B_MENU, B_CIMA, B_BAIXO, B_OK, B_VOLTAR, B_ENC_SW, B_ENC_GIRO };
enum Acao  : uint8_t { CLIQUE = 0, SEGUROU = 1 };

// --- A mensagem (igual nas duas placas) --------------------------------------
typedef struct __attribute__((packed)) {
  uint8_t  tipo;          // TipoMsg
  uint8_t  botao;         // Botao
  uint8_t  acao;          // Acao
  int8_t   giro;          // encoder: +1 / -1
  float    temperatura;
  float    umidade;
  char     linha1[17];    // LCD: 16 letras + fim de texto
  char     linha2[17];
  uint32_t contador;      // sobe a cada envio: mostra mensagens perdidas
} Mensagem;
