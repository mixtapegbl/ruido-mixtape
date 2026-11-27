# CAIXA · ESP32 DevKit V1 (clássica)

Placa do **áudio e da rede** do RUÍDO. O código do projeto (`caixa.ino`) começa
depois que o teste `teste/t6_audio_8bit` funcionar na bancada.

O que ela vai fazer:
- **Som 8 bits gerado por código** (sintetizador estilo Game Boy) no DAC interno, GPIO 25.
  Não precisa de cartão SD: as músicas são notas, ocupam poucos bytes.
- **Efeitos por cima da música** e o **"Ruído" da Corrupção** aplicado ao som em tempo real.
- **Wi-Fi**: portal que pede nome e senha da rede, hora pela internet e site com os status.
- Conversa com o **AMP por fio (UART)**, então pode usar o canal do roteador sem atrapalhar o ESP-NOW.

Matriz de pinos: `design.md`, seção 13.
