# RUÍDO — uma MIXTAPE de terror · Documento de Design (rascunho v0.2)

> Montado a partir das nossas conversas. Tudo marcado com **[A DECIDIR]** ainda está em aberto.
> Os itens que você deixou para o Claude estão marcados com **(proposta do Claude)**. Se não gostar, troque.
> Edite à vontade: o documento é seu.

---

## 1. Ideia em uma frase
RPG de exploração de salas e batalhas por turnos, jogado em duas ESP32-S3: uma é o controle com os diálogos, a outra é o "console" com as telas. O mundo reage à sala real (hora, temperatura, luz).

---

## 2. Mundo e história
- **Tema:** cyberpunk industrial, terror psicológico e um toque gótico, com momentos de humor.
- **Conceito central:** o **Ruído**, uma força sonora que corrompe pessoas e máquinas numa cidade industrial. A inspiração é o clima de *Amplificador*; a história e os personagens são nossos.
- **Metamorfose** (de *A Metamorfose*, obra em domínio público): usar habilidades corrompidas deixa o personagem mais forte, mas o transforma aos poucos. Os NPCs passam a reagir com medo.
- **Referências de clima:** Blasphemous, Castlevania SotN e Overlord (gótico, masmorras); Ranma ½ (humor nos NPCs).
- **Tom:** adulto, com violência e humor malicioso (piadas de duplo sentido nos NPCs), mas sem cenas explícitas.
- **Vilão (proposta do Claude [ok]):** **O Maestro**. Era o engenheiro-chefe da cidade e construiu o Grande Amplificador no Núcleo para "afinar" as pessoas e deixar todo mundo produtivo e obediente. Ele ouviu a frequência pura e virou o próprio Ruído. Cada chefe é alguém que ele "afinou".
- **Começo:** o herói acorda numa linha de montagem desligada, com um fone de ouvido quebrado que o protege do Ruído (por pouco tempo).

### 2.1 História por área (proposta do Claude)
| Área | O que acontece | NPCs | Fita escondida (final secreto) |
|---|---|---|---|
| 1 Fábrica | Operários "afinados" trabalham sem parar; a chefe de turno virou a Esteira-Mãe | **Vovó Graxa**: mecânica velha, loja, piadas de duplo sentido com ferramentas ("quer ver minha chave de boca, querido?") | No vestiário trancado (Chave Azul, achada na própria área) |
| 2 Esgoto | Quem fugiu do Ruído vive aqui; a Boca do Bueiro está engolindo os refugiados | **Toninho Tampa**: vendedor que mora num cano, medroso e exagerado | Deixada pela Boca do Bueiro ao ser derrotada |
| 3 Rádio Abandonada | Daqui saiu a primeira transmissão do Ruído; o Locutor Fantasma conta o passado da cidade | **Locutor Fantasma** (inimigo e narrador); a **Ratazana Rainha** reaparece cobrando "aluguel" | Na cabine de transmissão, só aparece **à noite** (RTC) |
| 4 Catedral do Ruído | Um culto adora o Ruído como música divina; a Soprano Cega foi a primeira voz "afinada" pelo Maestro | **Irmã Dó-Ré**: freira que desertou do culto, ajuda escondida | No confessionário, só aparece se você **não usou Ruído** na área 4 |
| 5 Núcleo | O Grande Amplificador; o Eco de Você e o Maestro | — | Na sala do Amplificador, depois de vencer o Eco de Você |

- **Lado B (final secreto):** o Maestro construiu o Amplificador para **cancelar** o barulho da cidade e deixar as pessoas descansarem. O cancelamento saiu errado e virou o Ruído. Ele não é só um vilão: é um engenheiro que tentou ajudar e falhou.
- **Finais (proposta do Claude[ok]), decididos pela Corrupção no fim do jogo:**
  - **Corrupção < 50 — "Silêncio":** você desliga o Amplificador e a cidade acorda.
  - **Corrupção ≥ 50 — "Nova Regência":** você derrota o Maestro e toma o lugar dele.
  - **Secreto — "Lado B":** você zera com Corrupção 0 e acha as 5 fitas escondidas. Revela quem o Maestro era antes do Ruído.

---

## 3. Personagem
- Jogo de 1 jogador, com um herói só.
- **Atributos:** Vida, Mana, Ataque, Defesa, Velocidade, Corrupção (0–100).
- **Nível máximo:** 20.
- **Classes (3, escolhidas no início):**
| Classe | Nome no mundo do Ruído | Forte em | Fraco em | Elemento natural | Diferencial |
|---|---|---|---|---|---|
| Guerreiro | **Britador** (martelo de sucata aquecido) | Vida, Defesa | Velocidade, Mana | Fogo | Defesa ativa reduz mais dano (−70% em vez de −50%) |
| Mago | **Tecnomante** (luvas de bobina) | Mana, Ataque mágico | Vida, Defesa | Elétrico | Habilidades custam 20% menos mana |
| Ladino | **Pirata de Sinal** (lâminas e rádio pirata) | Velocidade, Crítico | Vida | Som | Zona de crítico da barra de precisão é 2x maior |
- **Nome do herói:** o jogador digita no início (até 8 letras, para caber no 16x2).
- Mais classes podem entrar depois, sem mudar o resto do sistema.

### 3.1 Atributos base (proposta do Claude), no nível 1 e o ganho por nível
| Classe | Vida | Mana | Ataque | Defesa | Velocidade |
|---|---|---|---|---|---|
| Britador | 40 (+8) | 10 (+2) | 8 (+2) | 6 (+2) | 3 (+1) |
| Tecnomante | 25 (+5) | 30 (+5) | 7 (+2) | 3 (+1) | 4 (+1) |
| Pirata de Sinal | 30 (+6) | 15 (+3) | 7 (+2) | 4 (+1) | 7 (+2) |
- Mais **+1 ponto livre por nível** para Ataque, Defesa ou Velocidade.
- O Tecnomante compensa a vida baixa com habilidades mais fortes e mais baratas.
- **Esses números são o ponto de partida; vão ser ajustados nos testes da etapa 5.**

---

## 4. Mapa (estilo Darkest Dungeon)
- **5 áreas:** Fábrica → Esgoto → Rádio Abandonada → Catedral do Ruído → Núcleo.
- 15 a 20 salas por área, ligadas por corredores, numa grade com movimento cima/baixo/esquerda/direita.
- Salas não exploradas aparecem **cinza** (névoa).
- A tela 2.0" só redesenha quando o jogador muda de sala.
- **O que pode ter numa sala:** vazio, inimigo visível, baú, altar de save, porta trancada, chave, NPC, chefe.
- **Batalhas:** começam com inimigos visíveis no mapa (sem batalha aleatória).

---

## 5. Batalha (inspiração Undertale)
| Problema comum | Solução |
|---|---|
| Errar o ataque por azar | Todo ataque acerta; a **barra de precisão** define Normal ou Crítico |
| Tomar dano do nada | A **intenção do inimigo** aparece antes do seu turno |
| Só apertar "Atacar" cansa | Barra de precisão + **defesa ativa** (apertar no tempo certo reduz o dano) |

- **Menu:** Atacar / Habilidade / Item / Fugir.
- **Barra de precisão:** um cursor corre no **OLED** da Placa E (128 pixels, liso); OK perto do centro = crítico. Sem OLED, cai para a linha de baixo do 16x2.
- **Intenção do inimigo:** ícone grande no **OLED** (espada, escudo ou caveira); o 16x2 fica com o texto.
- **Defesa ativa:** o OLED mostra o "AGORA!" no tempo certo; o buzzer dá o sinal.

### 5.1 Fórmula de dano (proposta do Claude)
```
dano = Ataque × Poder × (20 / (20 + Defesa_do_alvo)) × Elemento × Clima × Precisão × Variação
```
- **Poder:** 1,0 no Atacar comum; cada habilidade tem o seu (tabela da seção 7.1).
- **Defesa:** quanto mais defesa, menos dano, mas nunca chega a zero (com Defesa 20 o dano cai pela metade).
- **Elemento:** ×1,5 (forte), ×0,5 (fraco), ×1,0 (neutro), ×1,25 (Ruído). Veja as fraquezas abaixo.
- **Clima:** os modificadores da seção 10.2.
- **Precisão:** Normal ×1,0 · **Crítico ×1,5**.
- **Variação:** sorteio entre ×0,9 e ×1,1, só para os números não ficarem sempre iguais (o golpe sempre acerta).
- Dano mínimo = 1. Arredondar para inteiro no final.
- **Defesa ativa:** perfeita = −50% (Britador −70%) · boa = −25% · errou = sem redução.
- **Quem age primeiro:** maior Velocidade. Empate = o jogador.
- **Fugir:** 100% de chance contra inimigos comuns, mas custa 10% da Sucata que você carrega. Não dá para fugir de chefes.

**Inimigos base por área** (os detalhes de cada um mudam em cima disso)
| Área | Vida | Ataque | Defesa | Sucata ao vencer |
|---|---|---|---|---|
| 1 Fábrica | 22 | 6 | 3 | 5 |
| 2 Esgoto | 55 | 12 | 6 | 15 |
| 3 Rádio | 100 | 20 | 10 | 35 |
| 4 Catedral | 170 | 30 | 15 | 65 |
| 5 Núcleo | 260 | 42 | 21 | 110 |
- **Chefe:** Vida ×6, Ataque ×1,3 e Sucata ×10 do inimigo da área.

- **Fraquezas por tipo (proposta do Claude [ok]):**
  - Cada inimigo tem um **corpo**: **Máquina**, **Carne** ou **Espectro**.
  - Cada golpe tem um **elemento**:

| Elemento | Forte contra (×1,5) | Fraco contra (×0,5) |
|---|---|---|
| Elétrico | Máquina | Espectro |
| Fogo | Carne | Máquina |
| Som | Espectro | Carne |
| **Ruído** (habilidades sombrias) | todos (×1,25) | — · custa **+5 de Corrupção** por uso |

  - Isso fecha um triângulo: cada classe brilha numa área e sofre em outra, e o Ruído vira a "tentação" de quando o jogador aperta.
  - **Clima da sala (DHT11):** afeta o dano dos elementos. As regras completas, com vantagens e desvantagens, estão na seção 10.

---

## 6. Inimigos e chefes
- 6 inimigos por área, mais variações de cor (palette swap): cerca de **45 inimigos**.
- **5 chefes**, um por área; cada um libera uma habilidade nova.
- **Tamanhos:** tiles 16x16, inimigos 64x64, chefes 96x96.
- **Lista de inimigos (proposta do Claude [ok]):** 30 inimigos base + 15 variantes = **45**
  - **Variantes noturnas (RTC, das 22h às 6h):** 3 inimigos por área ganham uma versão de cor escura, +30% de vida e o dobro de XP.

**Área 1 — Fábrica** (maioria Máquina)
| Inimigo | Corpo | Detalhe |
|---|---|---|
| Rato-Fio | Carne | Rápido, rói seus itens (rouba 1 poção) |
| Operário Oco | Carne | Lento, bate forte; repete "meta batida, meta batida" |
| Prensa Viva | Máquina | Carrega 1 turno e esmaga (intenção: caveira) |
| Drone de Solda | Máquina | Queima: dano por 2 turnos |
| Parafuso Gritante | Máquina | Grito que reduz sua Defesa |
| Capataz de Graxa | Carne | Chama um Operário Oco se você demorar |
| **Chefe: Esteira-Mãe** | Máquina | Muda de "velocidade" a cada 2 turnos: a barra de precisão acelera |

**Área 2 — Esgoto** (maioria Carne)
| Inimigo | Corpo | Detalhe |
|---|---|---|
| Sanguessuga de Cobre | Carne | Rouba vida |
| Crocodilo de Lata | Máquina | Morde e prende: você perde 1 turno |
| Lodo Faminto | Carne | Divide em 2 quando leva crítico |
| Pescador Cego | Carne | Só ataca se você usar Som |
| Fungo-Eco | Espectro | Repete o último golpe que você usou |
| Ratazana Rainha | Carne | Cura os outros ratos; NPC cômica antes da luta |
| **Chefe: Boca do Bueiro** | Carne | Engole seus itens; ao vencer, você recupera tudo em dobro |

**Área 3 — Rádio Abandonada** (maioria Espectro)
| Inimigo | Corpo | Detalhe |
|---|---|---|
| Locutor Fantasma | Espectro | Narra o seu próximo golpe e o esquiva se você repetir |
| Estática Viva | Espectro | Embaralha o texto do 16x2 por 1 turno |
| Fita Rebobinada | Máquina | Desfaz o dano do último turno (1 vez) |
| Antena Enforcada | Máquina | Única Máquina da área: castiga quem só trouxe Som (Tecnomante brilha aqui) |
| Ouvinte Sem Rosto | Espectro | Aumenta a sua Corrupção ao acertar |
| Jingle Assassino | Espectro | Inimigo cômico: canta propagandas absurdas; a melodia toca no buzzer |
| **Chefe: A Última Transmissão** | Espectro | A batalha é um programa ao vivo: "ligações" do público ajudam ou atrapalham |

**Área 4 — Catedral do Ruído** (Carne e Espectro)
| Inimigo | Corpo | Detalhe |
|---|---|---|
| Coroinha Afinado | Carne | Fraco sozinho, forte em trio |
| Freira Metrônomo | Carne | Ataca no ritmo: a defesa ativa precisa seguir o tempo do buzzer |
| Órgão de Ossos | Espectro | Muito resistente; cada nota tocada é um ataque |
| Penitente Amplificado | Carne | Causa dano em si mesmo para bater mais forte |
| Sino Rachado | Máquina | Atordoa: o próximo turno não tem barra de precisão |
| Diácono do Feedback | Espectro | Devolve 50% do dano de Ruído |
| **Chefe: A Soprano Cega** | Espectro | Só "vê" pelo som: usar Som contra ela é arriscado (ela te acha) |

**Área 5 — Núcleo** (todos os tipos)
| Inimigo | Corpo | Detalhe |
|---|---|---|
| Cabo-Serpente | Máquina | Enrola: −1 de Velocidade por turno |
| Guardião Subwoofer | Máquina | Escudo que só cai com crítico |
| Eco de Você | Espectro | Copia a sua classe e os seus golpes |
| Fusível Kamikaze | Máquina | Explode no 3º turno (intenção: caveira com contagem) |
| Mixer Carniceiro | Carne | Mistura 2 elementos por golpe |
| Coral de Alto-Falantes | Máquina | Fica mais forte para cada inimigo derrotado na área |
| **Chefe final: O Maestro** | todos | 2 fases; na segunda ele usa o mesmo Ruído que você usou durante o jogo |

---

## 7. Progressão
- Ganha XP nas batalhas e sobe de nível até o 20.
- Habilidades novas vêm dos chefes; a Corrupção libera as versões "sombrias".
- **Curva de XP (proposta do Claude):**
  - XP para passar do nível N para o N+1 = **5 × N²** (nível 1→2: 5 XP · 10→11: 500 XP · 19→20: 1.805 XP).
  - XP por inimigo: Área 1 = 10 · Área 2 = 30 · Área 3 = 70 · Área 4 = 130 · Área 5 = 220. Chefe = 10× o inimigo da área.
  - Fazendo ~12 lutas por área, o jogador chega perto do **nível 18** no Maestro. O nível 20 fica para quem explorar tudo e caçar as variantes noturnas.
  - A cada nível: +Vida, +Mana e +1 ponto para distribuir entre Ataque, Defesa e Velocidade.
- **Equipamentos (proposta do Claude):** 3 espaços: **Arma**, **Armadura** e **Amuleto**.
  - Um "tier" novo por área (5 tiers), encontrado em baús ou deixado pelos chefes.
  - **Amuletos** mexem com a Corrupção. Exemplos: o *Fone Remendado* reduz a Corrupção ganha em 50%; o *Diapasão Negro* dá +40% de dano de Ruído e +2 de Corrupção por turno.
- **Corrupção:** sobe com habilidades de Ruído; os altares podem baixar 10 pontos uma vez por área. Acima de 50, alguns NPCs fogem e os preços das lojas sobem.

### 7.1 Habilidades (proposta do Claude)
Cada classe começa com 1 habilidade e ganha mais 1 por chefe (chefes 1 a 4). As de **Ruído** liberam pela Corrupção (≥ 34 e ≥ 67) e custam +5 de Corrupção por uso, além da mana.

**Britador (Fogo)**
| Habilidade | Libera | Mana | Poder | Efeito |
|---|---|---|---|---|
| Martelada Rubra | início | 4 | 1,4 Fogo | — |
| Muralha de Sucata | Chefe 1 | 6 | — | A próxima defesa ativa é perfeita automaticamente |
| Fornalha | Chefe 2 | 8 | 1,2 Fogo | Queima: dano por 2 turnos |
| Fúria de Turno | Chefe 3 | 5 | — | +40% de Ataque e −20% de Defesa por 2 turnos |
| Terremoto Industrial | Chefe 4 | 14 | 2,2 Fogo | A barra de precisão corre 2x mais rápido |
| Grito do Britador | Corrupção 34 | 8 | 1,8 Ruído | Atordoa o inimigo por 1 turno |
| Coração de Pistão | Corrupção 67 | 10 | — | Cura 30% da vida, mas a vida máxima cai 10% até o fim da luta |

**Tecnomante (Elétrico)** (já com 20% menos mana da classe)
| Habilidade | Libera | Mana | Poder | Efeito |
|---|---|---|---|---|
| Arco Voltaico | início | 4 | 1,5 Elétrico | — |
| Sobrecarga | Chefe 1 | 8 | 1,0 Elétrico | −30% de Defesa do inimigo por 3 turnos |
| Campo Magnético | Chefe 2 | 7 | — | −40% de dano recebido de Máquinas por 3 turnos |
| Reboot | Chefe 3 | 12 | — | Cura 35% da vida |
| Tempestade de Bobina | Chefe 4 | 16 | 2,5 Elétrico | — |
| Estática Mental | Corrupção 34 | 9 | — | 50% de chance de o inimigo atacar a si mesmo no próximo turno |
| Curto-Circuito Total | Corrupção 67 | 15 | 3,0 Ruído | Custa também 30% da sua vida atual |

**Pirata de Sinal (Som)**
| Habilidade | Libera | Mana | Poder | Efeito |
|---|---|---|---|---|
| Lâmina de Frequência | início | 3 | 1,3 Som | — |
| Interceptar | Chefe 1 | 5 | — | Mostra a intenção dos próximos 2 turnos e rouba Sucata |
| Eco Duplo | Chefe 2 | 7 | 0,8 Som ×2 | Dois golpes, cada um com a sua barra de precisão |
| Sumir do Sinal | Chefe 3 | 6 | — | Esquiva garantida no próximo ataque |
| Pirataria em Massa | Chefe 4 | 13 | 1,8 Som | Copia o bônus que o inimigo tiver |
| Faixa Proibida | Corrupção 34 | 7 | 2,0 Ruído | Medo: −30% de Ataque do inimigo por 2 turnos |
| Silêncio Absoluto | Corrupção 67 | 12 | — | O inimigo perde 1 turno inteiro |

### 7.2 Itens e Sucata (proposta do Claude)
- **Moeda:** Sucata (S$). Os inimigos deixam Sucata ao morrer (tabela da seção 5.1).
- **Lojas:** Vovó Graxa (áreas 1 e 3) e Toninho Tampa (áreas 2 e 4). A área 5 não tem loja.
- Máximo de **9 de cada item** (cabe em 1 dígito no 16x2).

| Item | Efeito | Preço |
|---|---|---|
| Óleo de Reparo | Cura 30% da vida | 10 |
| Bateria | Recupera 30% da mana | 12 |
| Óleo Grosso | Cura 70% da vida (à venda a partir da área 3) | 40 |
| Antiestática | Remove queima, atordoamento e medo | 15 |
| Granada de Feedback | 40 de dano de Som, ignora Defesa | 30 |
| Fita Limpa | −10 de Corrupção (rara, só em baús) | não vende |
- Preços +20% com Interferência ou com Corrupção acima de 50 (não acumula: no máximo +20%).

### 7.3 Metamorfose (proposta do Claude)
| Corrupção | Estágio | Visual do herói (2.0") | Efeito no jogo |
|---|---|---|---|
| 0–33 | **Humano** | Normal | — |
| 34–66 | **Afinado** | Olhos com estática, fios saindo da pele | Libera a 1ª habilidade de Ruído; alguns NPCs ficam desconfiados |
| 67–100 | **Ressoante** | Meio máquina, alto-falante no peito | Libera a 2ª habilidade de Ruído; NPCs fogem; lojas +20% |
- São **9 sprites de herói** (3 classes × 3 estágios).

---

## 7.5 Um console só: o papel de cada placa
As placas funcionam como **um console só** (desde o Raio-X, com uma terceira: a **CAIXA**, ESP32 DevKit V1, que cuida do som 8 bits e do Wi-Fi e fala com o AMP por fio; veja a seção 13). A **Placa V é o cérebro** e a **Placa E é o controle com a caixa de diálogo**. O estado do jogo existe só na V: a E nunca decide nada sozinha, ela mostra o que a V manda e avisa o que o jogador fez. Assim as duas nunca ficam "discordando".

**Placa V (Receptor): o console**
- Roda o jogo inteiro: mapa, salas, névoa, batalha, inimigos, XP, inventário e Corrupção.
- Desenha a tela 2.0" (mapa e sala; inimigo e golpes) e a 0.96" (vidas, mana e status).
- Lê e grava no SD: save, log da partida, sprites e mapas.
- Lê o RTC (dia/noite, hora do save).
- Decide **o que** a Placa E mostra, toca e acende.
- Opcional: portal Wi-Fi / página web com os status.

**Placa E (Master): o controle e a caixa de diálogo**
- Lê os 5 botões (com debounce e "segurar 2 s") e envia para a V.
- Mostra no 16x2 as duas linhas que a V mandar, com os ícones e animações locais (setas, espadas, transição de combate).
- Toca no buzzer a melodia ou o efeito que a V pedir.
- Acende o LED RGB conforme a vida do jogador (verde, amarelo, vermelho).
- Lê o DHT11 a cada 30 s e envia o clima para a V.
- **Mini-jogos de tempo rodam na própria E:** a barra de precisão e a defesa ativa precisam de reação na hora. A E anima a barra no 16x2, mede onde o jogador apertou e devolve só o resultado (ex.: "posição 7 de 15"). Assim nenhum atraso do rádio atrapalha o tempo.

**Mensagens (ESP-NOW, nos dois sentidos)**
| De → Para | Mensagem | Conteúdo |
|---|---|---|
| E → V | BOTAO | qual botão, e se foi clique ou segurou |
| E → V | RESULTADO | resultado da barra de precisão / da defesa ativa |
| E → V | CLIMA | temperatura e umidade |
| V → E | LCD | linha 1, linha 2, grupo de ícones (exploração / batalha / transição) |
| V → E | MINIJOGO | começar a barra ou a defesa, com a velocidade |
| V → E | SOM | número da melodia ou do efeito |
| V → E | LED | cor da vida do jogador |
| as duas | PING | a cada 1 s, para o LED onboard mostrar se estão conectadas |

**Se o rádio cair:** a V pausa o jogo e mostra "Controle desconectado" na 2.0"; a E mostra `SEM SINAL` no 16x2 e o LED onboard fica vermelho. Quando volta, o jogo continua de onde parou.

---

## 8. Telas
| Tela | Explorando | Em batalha |
|---|---|---|
| **LCD 16x2** (Placa E) | Linha 1: saídas (`< [^] >`) · linha 2: o acontecimento | Menu (Atacar/Habilidade/Item/Fugir) e o texto do turno (`CRITICO!`) |
| **OLED 0.96"** (Placa E) | Status da conexão, hora do RTC, clima | **Barra de precisão**, **intenção do inimigo**, **defesa ativa** |
| **2.0" ST7789** (Placa V) | Mapa de salas / sala atual | Inimigo e animações dos golpes |
| **0.96" ST7735** (Placa V) | Vida, Mana, **Corrupção**, XP, nível, Sucata, nome da área | Vida do inimigo (em cima) + sua vida, mana e Corrupção (embaixo) |
| **LED onboard** (as duas) | Verde = conectado · azul piscando = mensagem · vermelho = sem link · amarelo = portal Wi-Fi | igual |

### 8.1 Barra de Corrupção (0.96")
- Fica logo abaixo da Vida e da Mana, com a sigla `RUI`.
- A cor muda aos poucos com o valor: **cinza** (0) → **ferrugem** `#B7410E` (34) → **sangue** `#8A0303` (67 em diante).
- Acima de 67: a borda da tela pulsa em vermelho e, de vez em quando, a 0.96" dá um "glitch" (linhas tremidas por um instante).
- Ícones pequenos ao lado das barras para os efeitos ativos: queima, atordoado, medo, Interferência.

### 8.2 Telas iniciais (proposta do Claude)
1. **Abertura:** "MIXTAPE apresenta" com um jingle curto.
2. **Título:** "RUÍDO" na 2.0" com estática animada; no 16x2, `>Continuar` / ` Novo jogo`.
3. **3 slots de save.** No 16x2: `Slot 1 Nv12 A3` (nível e área).
4. **Escolha de classe:** a 2.0" mostra o sprite e os atributos; o 16x2 mostra o nome da classe. Cima/Baixo trocam e OK confirma.
5. **Nome do herói (até 8 letras):** o 16x2 mostra `Nome: GABRI_`. Cima/Baixo trocam a letra (A–Z e espaço), OK vai para a próxima letra, Voltar apaga e **segurar OK** termina.
6. **Prólogo:** 3 telas curtas de história, avançando com OK.

### 8.3 Game over animado (nas três telas)
- **2.0":** o herói se desfaz em estática; aparece **"FIM DA FITA"**. Depois vem "REBOBINANDO..." com uma fita cassete voltando, e o mapa "anda para trás" até o altar.
- **0.96":** as barras esvaziam e uma linha de batimento cardíaco vira reta.
- **16x2:** a cortina de pixels desce, aparece `FIM DA FITA` e depois `OK: rebobinar`.
- **Buzzer:** melodia descendo e distorcendo. **LED RGB:** vermelho apagando devagar.

**Regras técnicas:**
- O LCD tem só **8 caracteres personalizados** por vez; organizar em grupos (exploração / batalha / transição).
- Nada de `delay()`: animações controladas com `millis()`.
- Telas desenhadas primeiro na memória (LovyanGFX + PSRAM) e enviadas de uma vez, sem o efeito "TV de tubo".
- Na 0.96": sem `fillScreen` a cada quadro, só redesenhar o que mudou.

---

## 9. Controles (Placa E)
A case tem **5 botões em linha** abaixo do LCD; o jogo funciona só com eles, sem hardware novo.

| Botão (esq. → dir.) | GPIO | Explorando | Em batalha |
|---|---|---|---|
| 1 · Menu | 1 | Abrir menu (itens, status, mapa) | — |
| 2 · Cima | 2 | Escolher a **próxima saída** da sala | Opção anterior |
| 3 · Baixo | 4 | Escolher a **saída anterior** | Próxima opção |
| 4 · OK | 5 | Ir pela saída escolhida / interagir | Confirmar / acertar a barra / defender |
| 5 · Voltar | 6 | Cancelar (segurar 2 s = mutar o som) | Voltar no menu |

**Movimento sem botões de esquerda/direita (proposta do Claude):**
- No estilo Darkest Dungeon, você anda de sala em sala, não pixel a pixel. Cada sala tem no máximo 4 saídas.
- O 16x2 mostra só as saídas que existem, por exemplo `Saidas: ^ > v`, com a escolhida piscando (seta animada).
- Cima/Baixo **passam pelas saídas válidas** e OK anda. São 2 cliques no máximo para qualquer direção.
- **Layout do 16x2 na exploração (decidido):**
  - **Linha 1:** só as saídas, com a escolhida entre colchetes e piscando, por exemplo `< ^ > [v]`.
  - **Linha 2:** o acontecimento, bem resumido (até 16 letras), por exemplo `Cheiro de oleo` ou `Rato-Fio a vista`.
  - Exemplo numa encruzilhada:
```
< ^ > [v]
Vozes ao norte..
```

- **Encoder rotativo com clique (EC11/KY-040, decidido):** girar troca a saída ou a opção do menu (1 clique = 1 opção); apertar o eixo = OK. Os 5 botões continuam funcionando; o encoder só deixa tudo mais rápido.
- O encoder de motor de HD fica para um projeto futuro.

---

## 10. Mundo real dentro do jogo
- **RTC DS1302:** dia/noite seguindo a hora real (variantes noturnas das 22h às 6h) e data/hora em cada save.
- **DHT11:** temperatura e umidade da sala viram o clima do jogo.

### 10.1 Faixas de clima
Faixas largas, porque o DHT11 tem erro de ~±2 °C e ~±5% de umidade. Para trocar de faixa, a média precisa passar **2 unidades além** do limite (histerese); assim o clima não fica ligando e desligando.

- **Temperatura:** Frio < 20 °C · Ameno 20–28 °C · Quente > 28 °C
- **Umidade:** Seco < 40% · Normal 40–70% · Úmido > 70%

### 10.2 Efeitos: todo clima tem vantagem e desvantagem
| Clima | Vantagem do jogador | Desvantagem do jogador | Visual (2.0") |
|---|---|---|---|
| Frio | Defesa +10% | Fogo −20% | Hálito / gelo nas bordas |
| Quente | Fogo +20% | Inimigos de Carne +15% de ataque | Faíscas, ar tremendo |
| Úmido | Elétrico +20% | Você leva +20% de dano Elétrico | Goteiras, névoa |
| Seco | Som +10% | Máquinas +15% de defesa | Poeira |
| Frio + Úmido | — | — | **Chuva** (só visual) |
| Ameno + Normal | — | — | Neutro (também é o clima de quando o sensor falha) |

### 10.3 Proteção contra trapaça
**Princípio:** nenhum clima é sempre melhor, então mexer no sensor não compensa.
1. **Média lenta:** o jogo usa a média dos últimos 10 minutos, não a leitura do momento.
2. **Saltos impossíveis são descartados:** mais de 3 °C ou 15% de umidade em 1 minuto = leitura ignorada (o valor anterior continua).
3. **Clima fixo por área:** decidido ao **entrar na área**, e fica igual até sair. Mexer no sensor no meio da batalha não muda nada.
4. **Sensor com erro ou desconectado:** clima neutro (Ameno + Normal), sem vantagem.

### 10.4 Ruído de verdade × trapaça (não punir quem joga certo)
O DS1302 e o DHT11 dão leituras erradas de vez em quando, mesmo com tudo ligado certo (fio comprido, mau contato, ruído elétrico). **Uma leitura estranha nunca é punição: é só descartada em silêncio.**
- **RTC:** ler 2 vezes seguidas e só aceitar se as duas baterem; rejeitar valores impossíveis (minuto > 59, mês 0, ano < 2026, tudo 0 ou tudo 165).
- **RTC:** uma diferença pequena (até 2 minutos) em relação ao relógio interno do ESP é tolerada e corrigida sem aviso.
- **RTC:** com Wi-Fi ligado, a hora é acertada pela internet (NTP) e o RTC é corrigido.
- **RTC:** o ajuste do relógio fica só num menu escondido, fora do jogo.
- **DHT11:** leitura `NaN` ou erro = descartar e usar a anterior.
- **Só conta como trapaça um padrão:** **3 manipulações confirmadas em 10 minutos**, com leituras válidas (que passaram na checagem dupla). Exemplos: o relógio volta no tempo, pula várias horas, ou a temperatura salta e volta várias vezes.

### 10.5 A punição: "Interferência"
Quando a trapaça é confirmada, aparece `O Ruido te viu..` no 16x2, com um som distorcido no buzzer, e o jogador recebe a **Interferência**:
- **Só coisas negativas:** −25% de XP e lojas 20% mais caras por 15 minutos de jogo.
- **Não mexe na Corrupção.** Como a Corrupção também dá poder (o dano de Ruído e o final "Nova Regência"), ganhar Corrupção trapaceando viraria uma recompensa.
- **Não acumula:** se já estiver com Interferência, o tempo só reinicia; no máximo 1 aplicação por área.
- Fica registrada no log da partida no SD (para você conferir se o sistema está punindo injustamente durante os testes).

---

## 11. Save, Wi-Fi e cartão SD
- **Cada slot tem 2 arquivos de save:**
  - `altar.json`: o estado salvo no último altar. É para onde o jogador volta quando morre.
  - `atual.json`: salvo automaticamente ao entrar em cada sala. Serve só para **desligar o console e continuar depois** de onde parou.
- **Altares** salvam o `altar.json` e recuperam a vida e a mana.
- **Wi-Fi:** portal cativo; sem rede salva, a placa cria a rede `MIXTAPE-RPG` e abre uma página para digitar nome e senha. Lembrar de alinhar o canal do ESP-NOW.

### 11.1 Morte (decidido)
- O jogador volta ao **último altar**, com o estado de quando salvou lá.
- Os inimigos vencidos depois do altar **voltam** (os chefes vencidos não voltam).
- O **XP e a Sucata ganhos depois do altar** ficam numa **Fita Perdida** na sala onde ele morreu, mais **25% da Sucata** que ele tinha no altar. Se tinha subido de nível depois do altar, volta para o nível do altar.
- **Chegando vivo na sala da Fita Perdida:** recupera **75%** do que estava nela (a perda é os outros 25%).
- **Morrer de novo antes de recuperar:** a fita antiga some para sempre e uma nova fica no novo lugar.
- **Nunca se perde:** itens, chaves e o mapa já revelado (a névoa continua aberta).

### 11.2 Formatos de arquivo (proposta do Claude)
**Regra de ouro:** todos os números do jogo ficam em arquivos **JSON no SD**, não no código. Assim, para ajustar o balanço (vida de inimigo, preço, dano), é só editar o arquivo no PC, **sem recompilar**. Biblioteca: **ArduinoJson**, usada só na Placa V.

```
/dados/classes.json       atributos base e ganho por nível (seção 3.1)
/dados/habilidades.json   seção 7.1
/dados/inimigos.json      seções 5.1 e 6
/dados/itens.json         seção 7.2
/mapas/area1.json ... area5.json
/save/slot1/altar.json  /save/slot1/atual.json   (e slot2, slot3)
/log/partida_001.csv      histórico da partida, com hora do RTC
/sprites/*.png            imagens (seção 15)
/som/*.json               melodias do buzzer (seção 12)
/som/*.wav                músicas, se usar o alto-falante (seção 12)
```

**Exemplo de mapa** (`/mapas/area1.json`): cada sala tem posição na grade, o que tem nela e as saídas.
```json
{
  "area": 1, "nome": "Fabrica", "inicio": 0,
  "salas": [
    { "id": 0, "x": 0, "y": 2, "tipo": "altar",   "saidas": { "L": 1 } },
    { "id": 1, "x": 1, "y": 2, "tipo": "inimigo", "inimigo": "rato_fio",
      "texto": "Cheiro de oleo", "saidas": { "O": 0, "N": 2, "L": 3 } },
    { "id": 2, "x": 1, "y": 1, "tipo": "bau", "item": "oleo_reparo", "saidas": { "S": 1 } }
  ]
}
```

**Exemplo de save** (`altar.json`):
```json
{
  "nome": "GABRIEL", "classe": "britador", "nivel": 5, "xp": 140,
  "vida": 72, "mana": 18, "corrupcao": 12, "sucata": 85,
  "area": 1, "sala": 0, "salas_vistas": [0, 1, 2, 3],
  "itens": { "oleo_reparo": 3, "bateria": 1 }, "chaves": ["azul"],
  "chefes": [], "fitas": [], "fita_perdida": null,
  "hora_save": "2026-11-23T20:40:00"
}
```

**Mensagens do ESP-NOW:** não usar JSON aqui (é lento e grande). Usar uma `struct` binária com o 1º byte dizendo o tipo (BOTAO, LCD, SOM...) e o resto com os dados. O limite é **250 bytes** por mensagem; uma mensagem de LCD (2 linhas de 16 letras + ícones) usa uns 40.

---

## 12. Som e música
**Atenção:** o buzzer **não toca MP3 nem WAV**. Ele só toca uma nota por vez (som de videogame antigo, tipo Game Boy). Para música de verdade é preciso um amplificador e um alto-falante. Por isso o som tem 2 caminhos:

### 12.1 Buzzer (já temos, Placa E)
- Efeitos: golpe, crítico, dano, defesa perfeita, level up, abrir porta, game over.
- Melodias de 1 voz, guardadas como **lista de notas em JSON** no SD: `/som/area1.json` → `{"bpm": 120, "notas": [["E4", 4], ["G4", 8], ["P", 8], ...]}` (`P` = pausa; o número é a duração: 4 = semínima, 8 = colcheia).
- A V lê o arquivo e manda as notas para a E tocar. Corrupção alta = notas levemente desafinadas de propósito.

### 12.2 Alto-falante (decidido, Placa V)
- **Amplificador: TPA3116D2 (XH-M552)**, estéreo, com controle de volume e alto-falante próprio; testado de 5 V a 24 V.
- **Atenção:** esse amplificador tem **entrada analógica** (como a saída de fone de um celular), e o ESP32-S3 **não tem saída de áudio analógica (DAC)**. Por isso entra um **DAC I2S UDA1334A** (Saravati, Santa Ifigênia) no meio:
```
ESP32-S3 (Placa V) --I2S--> UDA1334A --Lout/Rout--> TPA3116D2 --> alto-falante
```
- **Pinos do UDA1334A:** BCLK ← GPIO 15 · WSEL ← GPIO 16 · DIN ← GPIO 17 · **VIN ← 3V3 da Placa V** (o regulador da placa limpa o ruído do boost; VIN aceita 3,3 a 5 V, **nunca 12 V**) · GND. Os outros pinos (MUTE, PLL, SF0, SF1, DEEM) ficam desligados. Ele também tem saída P2, útil para fone ou outros projetos.
- **Ligação:** Lout, Rout e AGND do UDA1334A vão na entrada do amplificador. **GND do amplificador, do UDA1334A e da Placa V em comum.**
- **Alimentação (decidido): duas baterias separadas.**
  - **Células 18650 + boost de 12 V → só o amplificador.**
  - **LiPo 3,7 V 3000 mAh + boost de 5 V + carregador → as duas placas** (e, pelo 3V3 da Placa V, o UDA1334A).
  - **GND entre os dois sistemas num ponto só:** o AGND do UDA1334A no GND da entrada do amplificador. Nenhum outro fio de GND entre as baterias (dois caminhos = laço = zumbido).
  - O boost de 5 V precisa aguentar **pelo menos 1 A**; o carregador da LiPo deve ter **proteção** (TP4056 com DW01).
- **Teste de áudio (`t2_audio`):** tom, sirene e WAV funcionando; o SD **não** causa os estalos (leitura mais lenta 6 ms de 65 ms de folga). Os estalos que sobraram são de alimentação, e as baterias separadas + capacitores resolvem.
- **Volume:** o TPA3116 é de 50 W; começar com o potenciômetro baixo para não estourar o alto-falante.
- **Plano B, sem comprar nada:** saída PDM do próprio ESP32-S3 num pino + um filtro RC (resistor de 1 kΩ + capacitor de 10 nF) direto na entrada do amplificador. Funciona, mas o som tem mais chiado.
- Toca as músicas direto do SD com a biblioteca **ESP32-audioI2S**. Formato recomendado: **WAV 16 kHz mono** (leve para o ESP) ou MP3 de 128 kbps.
- O buzzer continua na E para os efeitos rápidos (cliques do menu e da barra de precisão).
- Com isso, as músicas geradas por IA podem ser testadas no próprio ESP.

### 12.3 Palavras-chave para gerar as músicas
Para cada faixa, pedir **"loop instrumental de 60 segundos, sem voz, que emende do fim para o começo"**, mais o estilo:
| Faixa | Palavras-chave |
|---|---|
| Título | dark synthwave, industrial, cassette tape hiss, slow, ominous, 90 BPM |
| Área 1 Fábrica | industrial, metal percussion, machine rhythm, factory ambience, 110 BPM |
| Área 2 Esgoto | dark ambient, dripping water, deep bass drone, slow, 70 BPM |
| Área 3 Rádio | lo-fi, detuned radio, static, distant old jingle, eerie, 85 BPM |
| Área 4 Catedral | gothic choir pad, pipe organ, distorted bells, 75 BPM |
| Área 5 Núcleo | aggressive industrial, glitch, heavy distortion, 130 BPM |
| Batalha comum | fast chiptune rock, tense, 150 BPM |
| Chefe | epic industrial metal, organ, choir, 160 BPM |
| Maestro | orchestral + noise, broken symphony, 140 BPM |
| Loja da Vovó Graxa | playful jazz, funny, old radio, 100 BPM |
| Game over | descending, tape slowing down, sad, 10 segundos |

- **Para o buzzer**, o pedido muda: "melodia de 1 voz em JSON, no formato `[nota, duração]`, até 32 notas, em loop".

---

## 13. Hardware e pinos (matriz nova, montagem do zero)
### Endereços (MAC) das placas: confirmados nos testes
| Placa | MAC | No código |
|---|---|---|
| **Placa V** (console) | `A4:CB:8F:D4:AB:34` | `uint8_t macPlacaV[] = {0xA4, 0xCB, 0x8F, 0xD4, 0xAB, 0x34};` |
| **Placa E** (controle) | `A4:CB:8F:D3:4F:00` | `uint8_t macPlacaE[] = {0xA4, 0xCB, 0x8F, 0xD3, 0x4F, 0x00};` |
| **CAIXA** (DevKit V1, nova) | `8C:94:DF:46:7A:90` | `uint8_t macCaixa[] = {0x8C, 0x94, 0xDF, 0x46, 0x7A, 0x90};` (fala por UART; o MAC só importa se um dia usar ESP-NOW) |
| DevKit V1 antiga (fora da CAIXA) | `E8:6B:EA:DD:09:A0` | `{0xE8, 0x6B, 0xEA, 0xDD, 0x09, 0xA0}` · mesmo chip e mesmos resultados do `teste_geral` |

Canal do ESP-NOW: **1**, por enquanto.
- O roteador troca de canal sozinho (no Raio-X da S3 estava no 10; no `teste_geral` da CAIXA, no 1). Por isso **o canal do roteador nunca entra fixo no código**: só a CAIXA usa o Wi-Fi, e ela segue o roteador sozinha.
- Como a CAIXA fala com o AMP por fio, o ESP-NOW do PEDAL e do AMP pode ficar num canal escolhido por nós. O canal 1 está cheio na casa (4 redes no `teste_geral`); se o rodapé do teste 3 mostrar mensagens perdidas, trocar para um canal vazio (ex.: 4, 8 ou 9). **Evitar o 13**: é o canal do hotspot POCO_X7_Pro (visto no `teste_geral` da CAIXA nova, a -32 dBm).

**Regras usadas:** o SPI da Placa V usa os pinos nativos do ESP32-S3 (11, 12 e 13), os mais rápidos; o I2C da Placa E usa os pinos padrão (8 e 9); todo botão usa `INPUT_PULLUP`, com a outra perna no GND.

### Placa V (console)
```
TELA 2.0" ST7789
  VCC  -> 3V3
  GND  -> GND
  SCL  -> GPIO 12   (SPI SCLK, comum)
  SDA  -> GPIO 11   (SPI MOSI, comum)
  CS   -> GPIO 10
  DC   -> GPIO 9
  RES  -> GPIO 8
  BLK  -> GPIO 7    (brilho por PWM: fade do game over)

TELA 0.96" ST7735
  VCC  -> 3V3
  GND  -> GND
  SCL  -> GPIO 12   (SPI SCLK, comum)
  SDA  -> GPIO 11   (SPI MOSI, comum)
  CS   -> GPIO 6
  DC   -> GPIO 5
  RES  -> GPIO 4
  BLK  -> 3V3       (sempre ligada)

MICRO SD
  VCC  -> 5V ou 3V3 (5V se o módulo tiver regulador; 3V3 se não tiver)
  GND  -> GND
  SCK  -> GPIO 12   (SPI SCLK, comum)
  MOSI -> GPIO 11   (SPI MOSI, comum)
  MISO -> GPIO 13   (só o SD usa)
  CS   -> GPIO 14

UART com a CAIXA (o DAC e o som foram para a CAIXA)
  TX   -> GPIO 39   (vai no RX2 = GPIO 16 da CAIXA)
  RX   -> GPIO 40   (vem do TX2 = GPIO 17 da CAIXA)
  GND  -> GND da CAIXA (o ÚNICO fio de GND entre o lado da lógica e o lado do áudio)

RTC DS1302
  VCC  -> 3V3
  GND  -> GND
  CLK  -> GPIO 18
  DAT  -> GPIO 21
  RST  -> GPIO 38

LED ONBOARD (status da conexão)
  -> GPIO 48

LIVRES: 1, 2, 15, 16, 17, 41, 42, 47
```

### Placa E (controle)
```
LCD 16x2 I2C (0x27)
  VCC  -> 5V
  GND  -> GND
  SDA  -> GPIO 8    (I2C, comum)
  SCL  -> GPIO 9    (I2C, comum)

OLED 0.96" I2C SSD1306 (0x3C) — em uso, testado
  VCC  -> 3V3
  GND  -> GND
  SDA  -> GPIO 8    (I2C, comum)
  SCL  -> GPIO 9    (I2C, comum)

BOTÕES (outra perna no GND)
  MENU   -> GPIO 1
  CIMA   -> GPIO 2
  BAIXO  -> GPIO 4
  OK     -> GPIO 5
  VOLTAR -> GPIO 6

ENCODER EC11 (sem placa, com clique) — não usa VCC
  Lado de 3 pinos:
    A (ponta)   -> GPIO 10
    C (meio)    -> GND
    B (ponta)   -> GPIO 11
  Lado de 2 pinos (botão do eixo):
    pino 1      -> GPIO 12
    pino 2      -> GND
  (pull-up interno do ESP; as abas de metal laterais são só fixação)
  Se for um KY-040 com placa: CLK -> 10 · DT -> 11 · SW -> 12 · + -> 3V3 · GND -> GND

DHT11
  VCC  -> 3V3
  GND  -> GND
  DATA -> GPIO 7    (10 kΩ para o 3V3 se o sensor não for em módulo)

BUZZER
  +    -> GPIO 13
  -    -> GND

LED RGB (cátodo comum, 220 Ω em cada cor)
  R    -> GPIO 14
  G    -> GPIO 21
  B    -> GPIO 47
  COMUM -> GND

LED ONBOARD (status da conexão)
  -> GPIO 48

LIVRES: 15, 16, 17, 18, 38, 39, 40, 41, 42
```

### CAIXA (ESP32 DevKit V1 clássica): áudio e rede
```
UART com o AMP (Serial2)
  RX2  -> GPIO 16   (vem do TX = GPIO 39 do AMP)
  TX2  -> GPIO 17   (vai no RX = GPIO 40 do AMP)
  GND  -> GND do AMP

SOM: DAC I2S UDA1334A (teste 6)
  BCLK -> GPIO 27 · WSEL -> GPIO 14 · DIN -> GPIO 13 · VIN -> 3V3 da CAIXA · GND
  Lout / Rout / AGND -> entrada do amplificador · SCLK, PLL, MUTE, DEEM, SF0, SF1 desligados
  (o som é calculado em 8 bits e enviado em 16 bits; GPIO 25 e 26 ficam livres)

LED RGB (cátodo comum: GND + 3 cores, 220 Ω em cada cor)
  R -> GPIO 32 · G -> GPIO 33 · B -> GPIO 4 · comum -> GND

LED DA PLACA -> GPIO 2 (azul)
ALIMENTAÇÃO  -> VIN <- 5V das células (cabo vermelho do pacote)

LIVRES: 5, 18, 19, 21, 22, 23, 25, 26 · só entrada: 34, 35, 36, 39
NUNCA USAR: 6 a 11 (flash) · 1 e 3 (USB/Serial) · cuidado no boot: 0, 2, 12, 15
```
- **`teste_geral` (28/11), CAIXA nova (`8C:94:DF:46:7A:90`) e DevKit antiga deram o mesmo resultado:** ESP32-D0WD-V3 rev 3.1, 2 núcleos a 240 MHz, **4 MB de flash, sem PSRAM**, 280 KB de RAM livre (maior bloco 107 KB), passou no teste de energia, float 11,5x mais rápido que double. App de até 1,28 MB na partição padrão: se a CAIXA (Wi-Fi + site + som) passar disso, escolher **"No OTA (2MB APP/2MB SPIFFS)"** em Partition Scheme.
- **Sem cartão SD** (só existe um, e ele fica no AMP): a música da CAIXA é **gerada por código**, nota por nota, num sintetizador de 3 vozes (pulso, triângulo e ruído). Cabe na memória e tem a cara de console antigo.
- **Wi-Fi na CAIXA:** portal que pede nome e senha da rede, hora pela internet e site de status. Como ela fala com o AMP por fio, pode seguir o canal do roteador (que muda sozinho) sem mexer no ESP-NOW (canal 1).
- **"Ruído" no som:** a CAIXA reduz bits, segura amostras e coloca chiado conforme a Corrupção sobe.

**Nunca usar nas duas placas S3:** 0, 3, 45, 46 (boot) · 19, 20 (USB) · 26–32 (memória flash) · **33–37 (PSRAM da N16R8)** · 43, 44 (Serial).

**Cuidado com o LCD:** o módulo I2C dele fica ligado em 5V e tem resistores de pull-up para os 5V, então o SDA e o SCL chegam no ESP com 5V. Na prática costuma funcionar, mas o ESP32 é de 3,3V. O jeito seguro é tirar os 2 resistores de pull-up do módulo do LCD (os do OLED, em 3,3V, já bastam) ou usar um conversor de nível lógico.

**Arduino IDE:** Flash 16MB · Partition com app grande · PSRAM OPI

---

## 14. Ordem de desenvolvimento (sugestão)
1. ✅ Teste da LovyanGFX: telas, SD e sprite na PSRAM (`t1_placa_v`, ~21 quadros/s a 27 MHz)
2. ✅ ESP-NOW nos dois sentidos + LED de status (`t3_placa_e` + `t3_placa_v_link`)
3. ✅ Botões, encoder EC11, DHT11 e LCD chegando na V (`t3`)
   ✅ Extra: SD (`t0_sd`) e áudio pelo UDA1334A (`t2_audio`); SD não causa picotes (6 ms de 65 ms)
4. Mapa de salas com névoa
5. Batalha simples (1 inimigo, barra de precisão)
6. Save no SD
7. Conteúdo: áreas, inimigos, história
8. Extras: RTC, DHT11, encoder, portal Wi-Fi

---

## 15. Arte e sprites
### 15.1 Formato
- **Desenhar e guardar em PNG** (fundo transparente). A LovyanGFX desenha PNG e BMP direto do SD (`drawPngFile` / `drawBmpFile`), então os seus testes em BMP continuam valendo.
- **Se algum PNG ficar lento** (sprites grandes ou animação), converter para `.bin` em RGB565 com um script Python no PC: carrega instantâneo, sem decodificar. Decidir isso na etapa 1, medindo o tempo.
- **Tamanhos:** tiles 16x16 · inimigos 64x64 · chefes 96x96 · herói 48x48 no mapa e 64x64 na batalha.
- **Paleta fixa de 32 cores** para tudo combinar: tons de ferrugem, azul-petróleo, cinza industrial, vermelho sangue e o **laranja MIXTAPE `#FF9800`** como cor de destaque. O site **Lospec** tem paletas prontas de pixel art.
- **Programas gratuitos:** LibreSprite (PC) ou Piskel (navegador). O Aseprite é pago, mas é o padrão da pixel art.

### 15.2 De onde vêm os sprites
- **Pacotes com licença livre:** Kenney (tudo CC0), OpenGameArt e itch.io filtrando por **CC0** ou **CC-BY**. CC-BY exige crédito: anotar tudo num arquivo `CREDITOS.md`.
- **Imagem aleatória do Google não serve:** quase toda imagem tem dono, e o projeto vai para o GitHub público e para eventos. Usar como **referência** para desenhar o seu é tranquilo; copiar o sprite, não.
- **Desenhados por nós:** o herói, os chefes e os NPCs principais (Vovó Graxa, Toninho Tampa, Irmã Dó-Ré). São eles que dão a cara do jogo.

### 15.3 O protagonista (proposta do Claude)
- **Marca registrada:** um **fone de ouvido grande e remendado com fita isolante laranja** (`#FF9800`), ligado por um cabo enrolado a um **walkman velho** preso na cintura. É de onde vem a proteção contra o Ruído e é onde ele ouve as fitas da história.
- **Em comum entre as classes:** tons de azul-petróleo, remendos e um crachá da fábrica com o número apagado.
- **Rosto:** meio escondido pelo fone e por uma gola alta; só os olhos aparecem bem, o que facilita mostrar a Metamorfose.
- **O protagonista muda conforme a classe (decidido).** Os três compartilham só o fone laranja e o walkman; o resto é próprio de cada um:
  - **Britador:** corpo grande e pesado, avental de couro de fundição, braços enfaixados, martelo de sucata com a cabeça incandescente.
  - **Tecnomante:** magro, jaleco de técnico cheio de bolsos, óculos de solda na testa, luvas de bobina de cobre soltando faíscas.
  - **Pirata de Sinal:** ágil, jaqueta curta com patches, capuz, duas lâminas curtas e uma antena de rádio dobrável nas costas.
- **Metamorfose:** **Afinado** = olhos com chiado de estática e fios saindo do pescoço · **Ressoante** = braço de metal enferrujado e um alto-falante rasgando o peito. O fone laranja **fica sempre**, cada vez mais rachado.


---

## 16. Lista de componentes
**Legenda:** ✅ já tem · 🛒 comprar agora · ⏳ depois / opcional

### Placa V (console)
| Status | Componente | Qtd | Observação |
|---|---|---|---|
| ✅ | ESP32-S3 N16R8 | 1 | |
| ✅ | Display 2.0" ST7789 (240x320) | 1 | |
| ✅ | Display 0.96" ST7735 (160x80) | 1 | |
| ✅ | Módulo micro SD + cartão 32 GB | 1 | |
| ✅ | RTC DS1302 | 1 | conferir se tem a bateria |
| 🛒 | Bateria CR2032 | 1 | para o RTC não perder a hora desligado |
| 🛒 | DAC I2S UDA1334A | 1 | Saravati |
| ✅ | Amplificador TPA3116D2 (XH-M552) + alto-falante | 1 | |
| 🛒 | Cabo P2-P2 ou fios blindados | 1 | do DAC até o amplificador; fio curto evita chiado |

### Placa E (controle)
| Status | Componente | Qtd | Observação |
|---|---|---|---|
| ✅ | ESP32-S3 | 1 | |
| ✅ | LCD 16x2 com módulo I2C | 1 | |
| ✅ | Botões na case | 5 | |
| ✅ | Buzzer | 1 | |
| ✅ | LED RGB cátodo comum | 1 | |
| ✅ | DHT11 | 1 | |
| 🛒 | Resistores de 220 Ω | 3 | um por cor do LED RGB, se ainda não tiver |
| ✅ | Encoder EC11 com clique | 1 | o reserva, em uso; falta só o knob |
| 🛒 | Resistores de 10 kΩ | 4 | pull-up do encoder (o KY-040 já vem com eles) |
| ✅ | Display OLED I2C 0.96" (SSD1306) | 1 | em uso: barra de precisão e intenção do inimigo |
| ⏳ | Encoder com motor de HD + LM339 | 1 | projeto futuro; o LM339 já foi comprado |
| ⏳ | Encoder óptico 600 PPR (LPD3806) | 1 | caro (~R$ 150+); só se o EC11 não agradar |

### Geral
| Status | Componente | Qtd | Observação |
|---|---|---|---|
| 🛒 | Jumpers macho-fêmea e fêmea-fêmea | 1 kit | |
| 🛒 | Capacitores de 100 µF e 100 nF | 4 de cada | perto da alimentação das telas e do amplificador, contra ruído |
| ⏳ | Placa perfurada | 2 | para soldar a versão final no lugar da protoboard |
| ⏳ | Fonte 12 V para o amplificador | 1 | opcional; soa melhor que 5 V. Só o GND é ligado nas placas |
