// ============================================================================
// RUÍDO — uma MIXTAPE de terror
// TESTE 0 · SÓ O CARTÃO SD (Placa V)
// Testa: ligação do módulo, velocidade do SPI, tipo e tamanho do cartão,
//        escrita e leitura, velocidade, e cria as pastas do jogo.
//
// Arduino IDE (Ferramentas):
//   Placa: ESP32S3 Dev Module · Flash Size: 16MB · PSRAM: OPI PSRAM
//   USB CDC On Boot: Enabled  (só se for usar a porta USB nativa para o Serial)
// Abra o Monitor Serial em 115200.
// ============================================================================

#include <SPI.h>
#include <SD.h>

// --- PINOS (matriz da Placa V) ----------------------------------------------
#define PINO_SCK   12   // = SCL das telas
#define PINO_MOSI  11   // = SDA das telas
#define PINO_MISO  13
#define SD_CS      14

#define T20_CS     10   // CS das telas: ficam em HIGH para não atrapalhar o SD
#define T09_CS      6
#define LED_PLACA  48

const char* PASTAS[] = { "/sprites", "/mapas", "/dados", "/save", "/log", "/som" };

// Tenta do mais rápido para o mais lento: mostra até onde a fiação aguenta
const uint32_t VELOCIDADES[] = { 20000000, 10000000, 4000000, 1000000 };

void led(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(LED_PLACA, r, g, b); }

const char* tipoCartao(uint8_t t) {
  switch (t) {
    case CARD_MMC:  return "MMC";
    case CARD_SD:   return "SDSC";
    case CARD_SDHC: return "SDHC/SDXC";
    default:        return "desconhecido";
  }
}

bool montarSD() {
  for (uint32_t freq : VELOCIDADES) {
    Serial.printf("  Tentando %lu MHz... ", (unsigned long)(freq / 1000000));
    if (SD.begin(SD_CS, SPI, freq)) {
      Serial.println("OK");
      return true;
    }
    Serial.println("falhou");
    SD.end();
    delay(100);
  }
  return false;
}

void listar(File pasta, uint8_t nivel) {
  File f = pasta.openNextFile();
  while (f) {
    for (uint8_t i = 0; i < nivel; i++) Serial.print("    ");
    if (f.isDirectory()) {
      Serial.printf("  [%s]\n", f.name());
      if (nivel < 1) listar(f, nivel + 1);
    } else {
      Serial.printf("  %s  (%u bytes)\n", f.name(), (unsigned)f.size());
    }
    f = pasta.openNextFile();
  }
}

bool testeEscritaLeitura() {
  const char* caminho = "/log/teste_sd.txt";
  File f = SD.open(caminho, FILE_WRITE);
  if (!f) { Serial.println("  Não abriu para escrever"); return false; }
  f.println("RUIDO - teste do SD - Placa V");
  f.printf("millis=%lu\n", (unsigned long)millis());
  f.close();

  f = SD.open(caminho, FILE_READ);
  if (!f) { Serial.println("  Não abriu para ler"); return false; }
  String primeira = f.readStringUntil('\n');
  f.close();
  primeira.trim();
  bool ok = primeira == "RUIDO - teste do SD - Placa V";
  Serial.printf("  Leu de volta: \"%s\" -> %s\n", primeira.c_str(), ok ? "OK" : "DIFERENTE");
  return ok;
}

void testeVelocidade() {
  const size_t TAM_BLOCO = 4096;
  const size_t TOTAL = 1024 * 1024;   // 1 MB
  static uint8_t bloco[TAM_BLOCO];
  for (size_t i = 0; i < TAM_BLOCO; i++) bloco[i] = i & 0xFF;

  File f = SD.open("/log/velocidade.bin", FILE_WRITE);
  if (!f) { Serial.println("  Não abriu o arquivo de velocidade"); return; }
  uint32_t t0 = millis();
  for (size_t n = 0; n < TOTAL; n += TAM_BLOCO) f.write(bloco, TAM_BLOCO);
  f.close();
  uint32_t tEsc = millis() - t0;

  f = SD.open("/log/velocidade.bin", FILE_READ);
  t0 = millis();
  while (f.available()) f.read(bloco, TAM_BLOCO);
  f.close();
  uint32_t tLei = millis() - t0;

  SD.remove("/log/velocidade.bin");
  Serial.printf("  Escrita: %.0f KB/s  |  Leitura: %.0f KB/s\n",
                1024.0f * 1000.0f / max<uint32_t>(tEsc, 1),
                1024.0f * 1000.0f / max<uint32_t>(tLei, 1));
  Serial.println("  (para o jogo, leitura acima de ~500 KB/s já está boa)");
}

void falhar(const char* motivo) {
  Serial.println();
  Serial.printf("RESULTADO: FALHOU — %s\n", motivo);
  Serial.println("Confira: cartão encaixado e em FAT32, VCC do módulo (5V se tiver regulador),");
  Serial.println("GND em comum, MISO no 13, CS no 14, e fios curtos.");
  while (true) { led(60, 0, 0); delay(300); led(0, 0, 0); delay(300); }   // vermelho piscando
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  led(0, 0, 40);   // azul: testando

  // Telas fora do barramento (se estiverem ligadas na protoboard)
  pinMode(T20_CS, OUTPUT); digitalWrite(T20_CS, HIGH);
  pinMode(T09_CS, OUTPUT); digitalWrite(T09_CS, HIGH);

  Serial.println();
  Serial.println("========== TESTE DO SD · PLACA V ==========");

  Serial.println("[1] Montando o cartão");
  SPI.begin(PINO_SCK, PINO_MISO, PINO_MOSI, SD_CS);
  if (!montarSD()) falhar("o cartão não respondeu em nenhuma velocidade");

  Serial.println("[2] Informações do cartão");
  uint8_t tipo = SD.cardType();
  if (tipo == CARD_NONE) falhar("nenhum cartão detectado");
  Serial.printf("  Tipo: %s\n", tipoCartao(tipo));
  Serial.printf("  Tamanho do cartão: %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
  Serial.printf("  Espaço total: %llu MB | usado: %llu MB\n",
                SD.totalBytes() / (1024ULL * 1024ULL), SD.usedBytes() / (1024ULL * 1024ULL));

  Serial.println("[3] Pastas do jogo");
  for (const char* p : PASTAS) {
    if (SD.exists(p)) Serial.printf("  %s já existe\n", p);
    else Serial.printf("  %s %s\n", p, SD.mkdir(p) ? "criada" : "NÃO foi criada");
  }

  Serial.println("[4] Escrita e leitura");
  if (!testeEscritaLeitura()) falhar("escrita ou leitura com erro");

  Serial.println("[5] Velocidade (1 MB)");
  testeVelocidade();

  Serial.println("[6] Conteúdo do cartão");
  File raiz = SD.open("/");
  listar(raiz, 0);

  Serial.println();
  Serial.println("RESULTADO: SD OK");
  led(0, 40, 0);   // verde fixo
}

void loop() {}
