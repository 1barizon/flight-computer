---
name: datasheet-telemetria-foguetes
description: Lê o datasheet (e, quando houver, o manual de referência / register map) de um componente eletrônico e produz um resumo técnico voltado a quatro tarefas de uma equipe de foguetes - (1) montagem de hardware do sistema de telemetria, (2) criação de footprint e símbolo no KiCad, (3) desenvolvimento de biblioteca bare-metal em C e (4) leitura de registradores do componente. Use sempre que o usuário enviar um datasheet de sensor, MCU, rádio, GNSS, memória, regulador, etc. para o sistema de telemetria.
---

# Skill: Resumo de Datasheet – Telemetria de Foguetes

## Contexto do projeto

Equipe de foguetes construindo um **sistema de telemetria**. O resumo gerado serve a quatro finalidades:

| Finalidade                        | O que precisa do resumo                                                                                                                                          |
| --------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Hardware**                      | Alimentação, pinagem, circuito típico, componentes externos, encapsulamento, layout, térmica, limites de operação no ambiente do foguete                         |
| **Software (registradores em C)** | Interface de comunicação, **mapa completo de registradores**, campos de bits, sequência de inicialização, formato dos dados, fórmulas de conversão, temporização |
| **Projeto PCB (KiCad)**           | **Dimensões mecânicas, land pattern, posição do pino 1, tabela de pads, dados de máscara/stencil/vias térmicas, tipos elétricos dos pinos para o símbolo**       |
| **Biblioteca bare-metal**         | Requisitos da camada de portabilidade (I²C/SPI/GPIO/delay), estrutura de arquivos, API mínima, tipos de dados, modelo de execução e tratamento de falhas         |

O resumo deve permitir que alguém **monte o hardware, crie o footprint e o símbolo no KiCad e escreva o driver bare-metal em C sem precisar reabrir o PDF** para os pontos críticos.

---

## Regras gerais (obrigatórias)

1. **Nunca invente valores, endereços ou bits.** Se não está no datasheet, escreva `Não informado no datasheet`. Em registradores, um valor errado gera bug silencioso, por isso é preferível deixar vazio a chutar.
2. **Cite a origem de tudo**: página, tabela ou figura (ex.: `[p. 42, Tab. 18]`).
3. **Todo valor numérico tem unidade e condição de teste.** Ex.: `Iq = 45 µA (VDD = 3,3 V, TA = 25 °C)`.
4. **Endereços e valores em hexadecimal** com prefixo `0x`; máscaras de bits em binário quando ajudar (`0b0011_1000`).
5. **Preserve os nomes originais** de registradores, campos e pinos (ex.: `CTRL_REG1`, `ODR[3:0]`, `nCS`). A biblioteca em C deve poder usar os mesmos nomes do datasheet.
6. **Separe sempre**: _Absolute Maximum Ratings_ (limite de destruição) × _Recommended Operating Conditions_ × _Electrical Characteristics_ (desempenho garantido).
7. **Diferencie Min/Max (garantido) de Typ (indicativo).** Para dimensionar, use Min/Max.
8. **Sinalize conflitos** entre seções, variantes do part number ou versões do documento, sem escolher um lado.
9. **Identifique a revisão** do datasheet. Se existir errata ou manual de referência separado (comum em MCUs), avise e diga se foi fornecido.
10. **Seja conciso**, mas **completo nos registradores**: nesta skill, o mapa de registradores NÃO deve ser resumido, e sim extraído por inteiro (veja a Seção 9).
11. **Dimensões mecânicas sempre em mm**, com min/nom/max e a página do desenho. Se o datasheet usar polegadas ou mils, converter e indicar o valor original. Um footprint errado em 0,1 mm pode inutilizar uma placa, então nunca arredonde, estime ou deduza uma dimensão que não esteja no documento.
12. **Nunca invente um land pattern.** Se o datasheet não traz o padrão de solda recomendado, diga `Land pattern não fornecido` e entregue as dimensões mecânicas para que o footprint seja calculado (IPC-7351), marcando cada valor como **"extraído"** ou **"calculado"**.

---

## Fluxo de trabalho

1. **Reconhecimento:** capa, descrição, diagrama de blocos, tabela de variantes/part numbers, índice. Identifique o **tipo de componente**.
2. **Interface:** descubra como o componente conversa com o host (I²C, SPI, UART, analógico etc.).
3. **Extração hardware:** Seções 1 a 7 e 12 a 14 do "Conteúdo a extrair".
4. **Extração software:** Seções 8 a 11 (registradores, inicialização, dados, temporização).
5. **Footprint e símbolo (KiCad):** Seções 16 e 17. Localize os desenhos mecânicos (_Package Outline_, _Mechanical Data_, _Land Pattern_, _Recommended PCB Footprint_, _Tape and Reel_).
6. **Biblioteca bare-metal:** Seção 18, organizando o que já foi extraído na Parte B em uma arquitetura portável.
7. **Contexto foguete:** avalie o componente contra os requisitos da Seção 15.
8. **Verificação cruzada:** endereços, bits e pinos batem entre texto, tabelas e figuras? A pinagem do footprint bate com a do símbolo e com o diagrama de pinos? As dimensões do desenho mecânico batem com o land pattern? Valores batem entre variantes?
9. **Saída:** gere o resumo no **Modelo de saída** ao final.

---

## Conteúdo a extrair

### PARTE A – HARDWARE

#### 1. Identificação

- Fabricante, part number completo, família, revisão/data do datasheet
- Categoria (IMU, barômetro, GNSS, rádio, MCU, memória, regulador...)
- Status (Active, NRND, EOL, Preliminary)
- Descrição em 1 a 3 frases, sem marketing

#### 2. Especificações-chave

Tabela: **parâmetro | símbolo | min | typ | max | unidade | condição | página**

- Tensão de alimentação (e tensões de I/O, se separadas)
- Consumo (operação, standby, sleep, picos de TX/RX/boot)
- Faixa de temperatura de operação
- Desempenho principal do tipo de componente (ver Módulos por tipo)

#### 3. Limites absolutos (Absolute Maximum Ratings)

- Tensão máxima por pino/grupo, corrente máxima, temperatura de junção e de armazenamento
- **Limites de choque mecânico** (ex.: 10 000 g) e vibração, se informados
- ESD (HBM, CDM)

#### 4. Condições recomendadas de operação

- Faixas de tensão, temperatura, e **sequência de power-up/power-down**

#### 5. Pinagem

Tabela: **pino nº | nome | tipo (I/O/PWR/NC) | função | observações**
Destacar:

- Alimentação, terra, referência
- Pinos de comunicação (SDA, SCL, MOSI, MISO, SCK, nCS, TX, RX...)
- Pinos de **seleção de modo/endereço** (ex.: SDO/SA0 que define o endereço I²C, CS/SPI select)
- Interrupção (INT1/INT2, DRDY), reset, boot, enable
- **Tolerância de nível lógico** (aceita 5 V? 3,3 V apenas?)
- Pinos que **não podem ficar flutuando** e pinos NC/reservados (o que fazer com eles)

#### 6. Aplicação típica e componentes externos

- Esquema recomendado (descrever)
- Capacitores de desacoplamento, bypass, resistores de pull-up (valor típico para I²C na velocidade usada), indutores, cristais (frequência, carga, tolerância)
- Fórmulas de dimensionamento fornecidas
- Nota de aplicação ou placa de avaliação referenciada

#### 7. Térmica e proteções

- θJA / θJC, necessidade de pad térmico e vias
- Proteções (sobrecorrente, sobretemperatura, UVLO, brown-out) e comportamento ao atuar
- Pinos de fault/flag/power-good

### PARTE B – SOFTWARE / BIBLIOTECA EM C

#### 8. Interface de comunicação (como falar com o chip)

**I²C**

- Endereço de 7 bits (e como alterá-lo via pino); mostrar também o byte de escrita/leitura (8 bits) para evitar confusão
- Velocidades suportadas (Standard 100 kHz, Fast 400 kHz, Fast+ 1 MHz...)
- Formato da transação: escrita de registrador, leitura de registrador, **leitura em burst**, **auto-incremento de endereço** (como habilitar; algum bit de controle?)
- Suporte a clock stretching, restart (repeated start) obrigatório?

**SPI**

- **Modo SPI** (CPOL/CPHA ou Mode 0/1/2/3), frequência máxima de SCK
- Ordem dos bits (MSB/LSB first)
- Formato do comando: bit de R/W (qual bit e qual valor significa leitura), bit de auto-incremento, tamanho do endereço, byte(s) dummy
- Comportamento do nCS (mantido baixo durante burst?)
- Modo 3-fios vs. 4-fios

**UART / outros**

- Baud rate, formato do frame, protocolo de comandos (AT, binário, NMEA, UBX etc.), checksums

**Níveis lógicos:** VIH, VIL, VOH, VOL

#### 9. Mapa de registradores (SEÇÃO CRÍTICA)

Extrair **todos** os registradores documentados. Se o datasheet for extenso (centenas de registradores), extrair todos os de configuração, controle, status e dados, e listar os demais como "reservados/de fábrica".

**9.1 Tabela-resumo do mapa**

| Endereço | Nome     | Acesso (R / W / R/W / RC / W1C) | Valor de reset | Descrição curta   | Pág.  |
| -------- | -------- | ------------------------------- | -------------- | ----------------- | ----- |
| 0x0F     | WHO_AM_I | R                               | 0x6C           | ID do dispositivo | p. 30 |

**9.2 Detalhe de cada registrador relevante (campos de bits)**

Para cada registrador, uma tabela:

**`NOME_REG` – Endereço `0x__` – Reset `0x__` – Acesso `R/W`**

| Bits  | Campo | Acesso | Reset  | Descrição e valores possíveis                             |
| ----- | ----- | ------ | ------ | --------------------------------------------------------- |
| [7:4] | ODR   | R/W    | 0b0000 | `0000` = power-down; `0001` = 12,5 Hz; `0010` = 26 Hz ... |
| [3]   | BDU   | R/W    | 0      | 0 = atualização contínua; 1 = bloqueia até ler MSB e LSB  |
| [2:0] | —     | —      | 000    | Reservado (manter 0)                                      |

Regras para esta extração:

- Informar **posição exata dos bits** (`[7:4]`), não só o nome.
- Listar **todos os valores codificados** de cada campo (a tabela de enumeração completa).
- Indicar **bits reservados** e **o valor que deve ser escrito** neles (muitos chips exigem escrever 0 ou manter o valor de reset).
- Indicar o **tipo de acesso especial**: bits que **limpam ao ler** (RC), **limpam ao escrever 1** (W1C), **auto-limpantes** (ex.: SW_RESET), **somente leitura**, **protegidos** (precisam de chave de desbloqueio).
- Marcar registradores **de 16/24/32 bits** distribuídos em vários endereços e a **ordem dos bytes** (LSB em endereço menor ou maior).
- Marcar **registradores com efeito colateral** (ler STATUS limpa a interrupção; ler o registrador de dados reinicia o DRDY).
- Marcar **registradores em bancos/páginas** (ex.: troca de banco por `BANK_SEL`) e como alternar.

**9.3 Registradores de dados (saída de medição)**

- Endereços, tamanho (bits), formato (signed/unsigned, complemento de dois, alinhamento à esquerda/direita, justificação), ordem dos bytes (endianness)
- Endereço inicial recomendado para **leitura em burst** de todos os eixos/canais em uma só transação
- Mecanismo de **coerência dos dados** (BDU, shadow registers, FIFO)

#### 10. Sequência de inicialização e operação

Extrair, passo a passo, na ordem:

1. Tempo de **power-on** até o chip responder (ex.: 10 ms)
2. **Reset** (por pino ou por software: qual registrador/bit/valor) e tempo de espera após o reset
3. **Verificação de identidade** (WHO_AM_I / CHIP_ID: endereço e valor esperado)
4. Configurações mínimas obrigatórias para entrar em operação (registradores e valores, na ordem exigida)
5. **Como iniciar medições** (modo contínuo, single-shot, trigger) e **como pará-las**
6. **Como saber que há dado novo** (DRDY, bit de status, interrupção, polling)
7. **Modos de energia** (power-down, low-power, normal, high-performance), transição entre eles e tempos de transição
8. **Autoteste (self-test)**, se existir: como acionar e valores esperados
9. **Calibração** exigida (offset, ganho) e como é feita

Se o datasheet trouxer **exemplo de código ou pseudocódigo de inicialização**, reproduzir a sequência de valores.

#### 11. Dados, conversão e temporização

**Conversão de unidades**

- Sensibilidade / LSB por faixa de escala (ex.: ±16 g → 0,488 mg/LSB), em tabela
- Fórmulas de conversão de valor bruto (raw) para unidade física
- **Coeficientes de calibração/compensação** armazenados em NVM/ROM do chip (ex.: trim parameters de barômetros): endereços, tamanhos, signed/unsigned e **o algoritmo de compensação completo** (copiar as fórmulas, passo a passo, com os tipos de dados exigidos: int32, int64, float)
- Offset de temperatura, fórmula de temperatura interna

**Temporização**

- Frequência de saída (ODR) e opções disponíveis
- Tempo de conversão, tempo de estabilização, tempo de warm-up
- Tempos mínimos entre transações, setup/hold de nCS, tempo mínimo de nCS em nível alto
- Atrasos obrigatórios (delay após comando, após mudança de modo)

**FIFO / buffers**

- Tamanho, modos (bypass, FIFO, stream, trigger), como configurar, como ler, formato das amostras, watermark, flags de overflow

**Interrupções**

- Fontes, mapeamento nos pinos INT, polaridade (active high/low), push-pull/open-drain, latched/pulsed, como limpar

### PARTE C – MONTAGEM, ENCAPSULAMENTO E AMBIENTE

#### 12. Encapsulamento e montagem

- Tipos disponíveis, dimensões, pitch, pad térmico exposto
- Footprint recomendado e stencil, perfil de reflow, **MSL** (dados completos para criação do footprint na Seção 16)
- Marcação e orientação (pino 1)
- Observações para **soldagem manual** (viável? exige ar quente?)

#### 13. Layout e integração mecânica

- Posição dos capacitores de desacoplamento, plano de terra, roteamento de sinais sensíveis
- Para sensores inerciais/pressão: orientação dos eixos (desenhar o sistema de coordenadas), **posição no PCB**, distância de fontes de ruído, fixação mecânica, **porta de pressão** (barômetros: ventilação, vedação, proteção contra luz e contaminação)
- Para rádio/GNSS: antena, casamento de impedância, plano de terra, isolamento de ruído digital

#### 14. Informações de pedido

- Part numbers, sufixos (temperatura, tensão, embalagem), fornecedores indicados, quantidade por carretel

### PARTE D – AVALIAÇÃO PARA O AMBIENTE DO FOGUETE

#### 15. Adequação ao uso em foguete

Avaliar o componente contra as condições típicas de voo e informar **ATENDE / NÃO ATENDE / NÃO INFORMADO** para cada item, com justificativa e página:

| Item                   | O que verificar                                                                                                                                                                                                                                                                                              |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Aceleração**         | Faixa de medição (acelerômetro/IMU): a faixa máxima cobre a aceleração do motor? Satura? (ex.: ±16 g pode saturar no boost de foguetes de alta potência)                                                                                                                                                     |
| **Giroscópio**         | Faixa em °/s suficiente para a taxa de rotação (spin) do foguete?                                                                                                                                                                                                                                            |
| **Choque e vibração**  | Limite de choque (g) e vibração suportados; sensibilidade de cristais/MEMS a vibração                                                                                                                                                                                                                        |
| **Temperatura**        | Faixa operacional cobre aquecimento da aviônica, sol na rampa e frio em altitude? Deriva com temperatura                                                                                                                                                                                                     |
| **Pressão / altitude** | Faixa de pressão cobre do nível do solo ao apogeu (ex.: 300 a 1100 hPa ≈ até ~9 km)? Comportamento em despressurização rápida e **efeito de transição transônica** nas leituras de pressão                                                                                                                   |
| **Taxa de amostragem** | ODR suficiente para a dinâmica do voo? Latência de saída                                                                                                                                                                                                                                                     |
| **GNSS**               | **Limites de altitude e velocidade** (restrições tipo COCOM: muitos módulos param de fornecer posição acima de ~18 km e/ou ~515 m/s), existência de **modo "airborne" / dinâmica alta** (ex.: Airborne <4g ou <1g), tempo de aquisição (cold/hot start), taxa de atualização máxima, constelações suportadas |
| **Rádio / RF**         | Frequência, potência TX, sensibilidade RX, alcance esperado, sensibilidade a **interferência do próprio foguete**, **adequação regulatória** (ANATEL no Brasil: faixas e potências permitidas)                                                                                                               |
| **Alimentação**        | Compatibilidade com a bateria (LiPo 1S/2S), queda de tensão sob corrente de pico, brown-out, consumo total no orçamento de energia, comportamento com tensão decaindo                                                                                                                                        |
| **Níveis lógicos**     | Compatibilidade de tensão com o MCU (3,3 V × 5 V × 1,8 V) e necessidade de level shifter                                                                                                                                                                                                                     |
| **Robustez**           | Proteções, comportamento em reset/queda de energia, watchdog, retenção de dados (memórias)                                                                                                                                                                                                                   |
| **Massa e tamanho**    | Dimensões e peso do componente / módulo                                                                                                                                                                                                                                                                      |
| **Disponibilidade**    | Ciclo de vida, disponibilidade em módulo breakout vs. SMD puro                                                                                                                                                                                                                                               |

### PARTE E – FOOTPRINT E SÍMBOLO (KiCad)

**Objetivo:** ao final, uma pessoa deve conseguir desenhar o footprint e o símbolo no KiCad sem reabrir o PDF. Os dados vêm dos desenhos mecânicos do datasheet (_Package Outline_, _Mechanical Data_, _Land Pattern_, _Recommended PCB Footprint_, _Soldering Footprint_, _Tape and Reel_).

#### 16. Dados para criação do footprint

**16.1 Identificação do encapsulamento**

- Nome do encapsulamento conforme o fabricante (ex.: `QFN-24 4x4`, `LGA-14 2,5x3`, `SOIC-8 (3,9 mm)`) e código do desenho (drawing no.)
- Código JEDEC / padrão (ex.: MO-220, MS-012), quando informado
- Tipo (QFN, DFN, LGA, SOIC, TSSOP, TQFP, BGA, WLCSP, módulo com castelations, THT...) e **número de pinos/pads**
- Se o part number tem **mais de um encapsulamento**, indicar qual desenho vale para qual sufixo
- Página do desenho no PDF

**16.2 Dimensões mecânicas do corpo**
Tabela: **símbolo | descrição | min | nom | max | mm | pág.**

- Comprimento e largura do corpo (D, E) e, para terminais gull-wing, o **span total incluindo terminais** (D1/E1)
- Altura total (A), standoff (A1), espessura do terminal (A3/c)
- **Pitch (e)**, largura do terminal (b), comprimento do terminal (L)
- **Pad térmico exposto** (D2 × E2) e sua posição
- BGA/WLCSP: matriz (linhas × colunas), pitch, diâmetro da esfera, **bolas ausentes**, nomenclatura das linhas (A, B, C... sem I, O, Q, S, X, Z quando aplicável)
- Tolerâncias gerais do desenho (ex.: ±0,05 mm) e referências de datum

**16.3 Pino 1, numeração e vista do desenho**

- **Posição do pino 1** e o indicador usado (ponto, chanfro, entalhe, canto cortado)
- Sentido da numeração (padrão: anti-horário, visto de cima)
- **Vista do desenho: TOP ou BOTTOM?** QFN, LGA, BGA e módulos costumam ter o desenho mostrado **visto de baixo**; sinalizar isso explicitamente para evitar footprint espelhado
- Pinos/pads **ausentes ou pulados** na numeração
- Pads NC, pads de fábrica e o que fazer com eles
- **Pad exposto**: qual número recebe (ex.: pad 25 ou `EP`) e a que sinal deve ser ligado (GND? flutuante?)

**16.4 Land pattern recomendado (padrão de solda)**
Se existir, extrair:

- Tamanho de cada pad (X × Y), pitch, posição (distância centro a centro entre linhas de pads, ex.: 5,40 mm entre fileiras)
- Pad exposto: dimensão e posição
- **Abertura da máscara de solda** (expansão ou valor absoluto; pad SMD × NSMD)
- **Stencil / pasta**: espessura sugerida, tamanho das aberturas, **janelas no pad térmico** (divisão em quadrados, % de cobertura)
- **Vias térmicas**: quantidade, diâmetro, passo, se devem ser tented/preenchidas
- **Zonas de keep-out**: sem cobre/trilhas sob o componente, área de antena, entrada de pressão, sensor óptico
- Courtyard / distância mínima a outros componentes, se citada
- Página e figura

Se **não existir**, escrever claramente `Land pattern não fornecido` e fornecer todas as dimensões da Seção 16.2 para cálculo pelo padrão IPC-7351 (indicar o nível de densidade, se o datasheet citar).

**16.5 Dados de montagem e fabricação**

- Perfil de reflow, temperatura de pico, **MSL**
- Orientação na fita (pino 1 / quadrante), largura da fita e passo (opcional)
- Solda manual viável? (QFN/LGA com pad térmico normalmente exigem ar quente)
- Cuidados: sensibilidade a flux, limpeza, underfill, proibição de ultrassom

**16.6 Modelo 3D**

- Altura máxima, dimensões e cor/marcação
- Se o fabricante informa a disponibilidade de modelo STEP e onde obtê-lo

**16.7 Tabela de pads para o KiCad**
Gerar quando houver land pattern ou dimensões suficientes:

| Pad nº | Sinal | Tipo (smd / thru_hole / np_thru_hole) | Forma (rect / roundrect / oval / circle) | X (mm) | Y (mm) | Tam. X (mm) | Tam. Y (mm) | Camadas | Origem do dado (extraído / calculado) |
| ------ | ----- | ------------------------------------- | ---------------------------------------- | ------ | ------ | ----------- | ----------- | ------- | ------------------------------------- |

Convenções:

- Origem (0,0) no **centro do corpo** (ou centro do pad exposto, para componentes simétricos)
- **No KiCad, o eixo Y cresce para baixo**: o pino 1 no canto superior esquerdo tem Y negativo
- Unidades em mm, grade de 0,05 mm ou mais fina
- Pads SMD em `F.Cu`, `F.Mask`, `F.Paste`; pad térmico com a janela de pasta informada
- Pad calculado: mostrar a conta (ex.: `X = (5,40 / 2) = 2,70 mm`)

**16.8 Camadas e convenções do footprint**

- `F.Fab`: contorno do corpo e marcação do pino 1
- `F.SilkS`: marcador do pino 1 e contorno, **sem sobrepor pads**
- `F.CrtYd`: courtyard com folga conforme política da equipe (a biblioteca oficial do KiCad usa 0,25 mm na maioria dos casos)
- Nome seguindo o padrão do KiCad, ex.: `QFN-24-1EP_4x4mm_P0.5mm_EP2.7x2.7mm`
- **Antes de criar, verificar se um footprint equivalente já existe na biblioteca oficial do KiCad** e comparar as dimensões com as do datasheet

#### 17. Dados para criação do símbolo (KiCad)

**17.1 Tabela de pinos do símbolo**

| Pino nº | Nome | Tipo elétrico (KiCad) | Grupo/lado sugerido | Funções alternativas | Observações |
| ------- | ---- | --------------------- | ------------------- | -------------------- | ----------- |

Tipos elétricos do KiCad: `Input`, `Output`, `Bidirectional`, `Tri-state`, `Passive`, `Power input`, `Power output`, `Open collector`, `Open emitter`, `Unconnected`.

**17.2 Regras**

- Pinos de alimentação repetidos (ex.: 4 × VDD): listar todos os números de pino com o mesmo nome
- Pinos NC → `Unconnected`; pad exposto → incluir como pino, indicando a ligação exigida
- Agrupamento sugerido: alimentação (cima), GND (baixo), entradas (esquerda), saídas e comunicação (direita)
- Componentes grandes (MCU): sugerir divisão em **múltiplas unidades** (alimentação, portas GPIO, periféricos)
- Se houver **mais de uma variante de pinagem** (por package ou sufixo), indicar se precisa de símbolo separado ou apenas de filtro de footprint

**17.3 Propriedades do símbolo**

- `MPN` (part number completo), `Manufacturer`, `Datasheet` (URL ou nome do PDF/revisão), `Description`, `ki_keywords`
- `Footprint` (nome do footprint sugerido) e `ki_fp_filters`
- Códigos de fornecedor (Digi-Key, Mouser, LCSC), se aparecerem no documento

### PARTE F – BIBLIOTECA BARE-METAL

**Objetivo:** organizar o que foi extraído na Parte B em uma **biblioteca C independente de HAL, RTOS e fabricante do MCU**, portável entre as placas da equipe. Regras: C99, **sem alocação dinâmica (`malloc`)**, sem dependência de bibliotecas de terceiros, sem acesso direto a periféricos do MCU dentro do driver.

#### 18. Dados e requisitos para a biblioteca bare-metal

**18.1 Camada de portabilidade (o que o host precisa fornecer)**
Listar as funções que o usuário da biblioteca deve implementar para o MCU escolhido, e extrair do datasheet os requisitos de cada uma:

- `read(reg, buf, len)` / `write(reg, buf, len)` sobre I²C, SPI ou UART: velocidade máxima do barramento, modo SPI, necessidade de _repeated start_, tamanho máximo de transferência, dummy bytes
- `cs_write(level)` e requisitos de temporização do chip select
- `delay_ms(ms)` e/ou `delay_us(us)`: **menor atraso exigido pelo datasheet** (define se basta milissegundo ou se é preciso microssegundo)
- `gpio_read(int)` para DRDY/INT, e se requer interrupção externa
- Fonte de **tempo (timestamp)**, se a medição ou a fusão de dados exigir

**18.2 Estrutura de arquivos sugerida**

- `dev.h`: API pública, tipos, enums de configuração
- `dev_regs.h`: endereços, máscaras e posições dos bits **gerados a partir do mapa de registradores da Seção 11** (`DEV_REG_<NOME>`, `<REG>_<CAMPO>_Pos`, `<REG>_<CAMPO>_Msk`)
- `dev.c`: implementação
- `dev_port.h`: declaração da camada de portabilidade (18.1)
- Enums para valores codificados (ex.: `DEV_ODR_26HZ`, `DEV_FS_16G`), nomeados como no datasheet

**18.3 API mínima a ser derivada do datasheet**

- `dev_init()`, `dev_reset()`, `dev_check_id()`
- `dev_config()` com uma `struct` tipada de configuração (faixa, ODR, filtros, modo de energia)
- `dev_start()` / `dev_stop()` / `dev_set_power_mode()`
- `dev_data_ready()`
- `dev_read_raw()` e `dev_read_converted()` (indicar unidade de saída)
- `dev_read_fifo()`, `dev_irq_handler()`, `dev_self_test()`, `dev_calibrate()`, se o componente tiver
- Indicar, para cada função, **quais registradores e sequências** da Seção 11/12 ela usa

**18.4 Tipos de dados e conversões**

- Largura exigida para os cálculos (int16, int32, **int64** em algoritmos de compensação)
- Sinal e endianness: converter byte a byte, **sem cast de buffer para `struct`** (evita problema de alinhamento e de ordem dos bytes)
- Sugerir saída em **ponto fixo** (mg, mdps, Pa, centi-°C) e informar se o algoritmo do datasheet exige ponto flutuante, para decidir conforme o MCU tenha ou não FPU

**18.5 Modelo de execução**

- Operação **bloqueante** ou **não bloqueante**; polling, interrupção ou DMA
- O que pode ser feito dentro da ISR (idealmente, apenas sinalizar com flag)
- Tempo máximo de bloqueio aceitável no _loop_ de voo
- Regras para **barramento compartilhado** entre vários dispositivos (exclusão mútua, troca de chip select)

**18.6 Tratamento de erros e robustez**

- Códigos de retorno sugeridos: `OK`, `ERR_COMM`, `ERR_ID`, `ERR_TIMEOUT`, `ERR_PARAM`, `ERR_NOT_READY`, `ERR_OVERFLOW`
- Timeouts de espera por conversão ou por flag
- Mecanismos de detecção do chip: CRC/checksum, bit de erro, watchdog interno, contador de amostras
- **Procedimento de recuperação** (reset por software, reinicialização completa, religar a alimentação), para que uma falha no sensor **não trave o loop de voo**
- Comportamento do chip em brown-out e em queda de alimentação

**18.7 Testes de bancada**

- Valor esperado de `WHO_AM_I`
- Resultado esperado do self-test (faixas de aceitação)
- Leituras de sanidade em repouso (ex.: aceleração ≈ 1 g no eixo vertical, pressão ≈ 101 kPa ao nível do mar, temperatura próxima da ambiente)
- Sequência para validar comunicação sem depender do restante do sistema

**18.8 Recursos necessários no MCU host**

- Instância de I²C/SPI/UART, pinos de GPIO para CS/INT/RESET/EN
- Entrada com interrupção externa (EXTI), canal de DMA (opcional)
- Resistores de pull-up e níveis de tensão (cruzar com a Seção 6/7)

---

## Módulos específicos por tipo de componente

Acrescentar os pontos abaixo, conforme o tipo.

### IMU / acelerômetro / giroscópio / magnetômetro

- Faixas de escala selecionáveis e sensibilidade por faixa
- ODR disponíveis, largura de banda do filtro (LPF/HPF), filtros digitais configuráveis
- Ruído (densidade espectral), offset de zero-g/zero-rate e deriva com temperatura
- Eixos e **sentido positivo de cada eixo** (diagrama), alinhamento
- Magnetômetro: faixa, calibração hard/soft-iron, set/reset
- FIFO, interrupções (free-fall, wake-up, data-ready, tap), autoteste
- Sensor de temperatura interno

### Barômetro / sensor de pressão

- Faixa de pressão, resolução, precisão absoluta e relativa, ruído (em Pa ou m de altitude)
- **Oversampling / filtro IIR**, modos de energia, tempo de conversão por configuração
- Coeficientes de calibração e **algoritmo de compensação (completo)**
- Fórmula de pressão → altitude (barométrica) se fornecida
- Sensibilidade à luz, umidade, contaminação, porta e vedação

### GNSS (GPS, GLONASS, Galileo, BeiDou)

- Constelações, bandas (L1, L5), sensibilidade (aquisição/tracking), tempo de TTFF (cold/warm/hot)
- Taxa de atualização máxima, precisão (CEP)
- **Protocolo**: NMEA, UBX ou outro, lista de mensagens principais e como configurar (comandos, checksums)
- **Modos dinâmicos** (airborne), **limites de altitude e velocidade**
- Pino de PPS, bateria de backup (V_BCKP), antena ativa/passiva, alimentação da antena

### Rádio / LoRa / módulos RF

- Frequência, modulação, taxa de dados, potência TX (dBm), sensibilidade RX (dBm), orçamento de enlace
- Parâmetros configuráveis (SF, BW, CR no LoRa), tamanho máximo do pacote, preâmbulo, sync word
- Registradores de configuração, FIFO TX/RX, estados (sleep, standby, TX, RX), IRQs, tempo de transição entre estados
- Antena, balun, casamento de impedância, filtros, requisitos de layout RF
- Consumo em TX/RX, **corrente de pico** (influencia o regulador e os capacitores)

### Microcontroladores / SoCs

- Núcleo, clock máximo, Flash/RAM
- Periféricos (quantidade e instâncias): UART, SPI, I²C, ADC, timers, DMA, USB, CAN
- **Multiplexação de pinos (alternate functions)**: mapear quais pinos suportam cada periférico usado na telemetria
- Modos de baixo consumo, fontes de clock, watchdog, brown-out reset
- Debug (SWD/JTAG), bootloader, opções de boot
- Corrente máxima por pino e por porta
- **Nota:** o _Reference Manual_ (separado do datasheet) contém os registradores; se não foi fornecido, avisar claramente.

### Memória (Flash SPI, EEPROM, cartão SD, FRAM)

- Capacidade, organização (páginas, setores, blocos), interface e comandos (opcodes)
- Tempo de escrita/apagamento, endurance (ciclos), retenção
- Proteção contra escrita, registrador de status (WIP, WEL), sequência de escrita segura
- Velocidade máxima, requisito de sequência de power-up

### Reguladores e gerenciamento de energia

- VIN/VOUT, corrente máx, corrente quiescente, dropout (LDO) ou eficiência (chaveado)
- Capacitores/indutor exigidos, estabilidade, soft-start, enable, power-good
- Comportamento em curto-circuito, proteção térmica
- Ripple e ruído (importante para sensores analógicos e RF)

### Sensores adicionais (temperatura, corrente, tensão, umidade)

- Grandeza, faixa, precisão, resolução, tempo de resposta
- Saída analógica (curva de transferência, offset, ganho) ou digital (registradores)

---

## Alertas e pegadinhas (sempre verificar)

**Hardware**

- [ ] Valores Typ tratados como garantidos
- [ ] Especificação válida só a 25 °C
- [ ] Parâmetros diferentes entre variantes/sufixos do part number
- [ ] Pinos que não toleram 5 V ou que não podem ficar flutuando
- [ ] Sequência de alimentação obrigatória
- [ ] Capacitor externo específico exigido para estabilidade
- [ ] Pad térmico que precisa ser ligado a um potencial específico
- [ ] MSL alto (cuidado no armazenamento e na soldagem)
- [ ] Componente Preliminary, NRND ou EOL

**Software / registradores**

- [ ] Endereço I²C de 7 bits × 8 bits (erro clássico de deslocamento)
- [ ] Bit de R/W e de auto-incremento no SPI (posição e polaridade)
- [ ] Ordem dos bytes (endianness) e sinal (complemento de dois)
- [ ] Bits reservados que precisam ser mantidos em 0 ou no valor de reset
- [ ] Registradores com **efeito colateral na leitura** (limpam flags)
- [ ] Escrita em registrador que só tem efeito após **desligar o modo de operação** (ex.: precisa estar em standby para configurar)
- [ ] Atrasos obrigatórios após reset ou mudança de modo
- [ ] Configuração que muda a **sensibilidade** (a conversão para unidade depende da faixa de escala escolhida)
- [ ] Algoritmo de compensação que exige **tipos de 32/64 bits** específicos (overflow em int16/int32)
- [ ] Registradores diferentes entre **variantes** do mesmo chip (ex.: revisões de silício)
- [ ] Diferenças entre datasheet, errata e manual de referência

**Footprint / KiCad**

- [ ] Desenho mecânico visto de **baixo** (BOTTOM) em QFN/LGA/BGA: risco de footprint espelhado
- [ ] Pino 1 e sentido da numeração conferidos com o diagrama de pinos
- [ ] Pad exposto: número atribuído e sinal ao qual deve ser ligado
- [ ] Dimensões em polegadas/mils convertidas corretamente para mm
- [ ] Datasheet com **mais de um encapsulamento** ou variantes de dimensões (ex.: 0,5 mm × 0,4 mm de pitch)
- [ ] Land pattern recomendado não fornecido: footprint calculado, não extraído
- [ ] Stencil com janelas no pad térmico e vias térmicas (tented/preenchidas) especificados
- [ ] Zonas de keep-out (antena, porta de pressão, sensor) definidas
- [ ] Pads/pinos NC, reservados e de fábrica identificados no símbolo
- [ ] Footprint equivalente já existente na biblioteca oficial do KiCad: dimensões conferem?

**Bare-metal**

- [ ] A biblioteca depende de algum recurso do MCU que não esteja abstraído na camada de portabilidade
- [ ] Atrasos exigidos menores que 1 ms (precisam de delay em µs)
- [ ] Algoritmo de compensação que exige int64 ou ponto flutuante
- [ ] Chamadas bloqueantes dentro de ISR ou do loop de voo
- [ ] Ausência de timeout ou de recuperação de falha de comunicação
- [ ] Barramento compartilhado sem regra de acesso

**Foguete**

- [ ] Saturação de faixa de medição durante o boost
- [ ] Limites de altitude/velocidade do GNSS
- [ ] Interferência do rádio no GNSS e nos sensores
- [ ] Queda de tensão da bateria sob picos de TX
- [ ] Barômetro afetado por transição transônica ou por vazamento de pressão na baia de aviônica

---

## Níveis de detalhe

Se o usuário não especificar, use **Completo** (esta skill é voltada ao desenvolvimento de driver).

| Nível          | Conteúdo                                                                                                                       |
| -------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| **Rápido**     | Identificação, função, 5 especificações-chave, interface e endereço, WHO_AM_I, encapsulamento, adequação ao foguete (resumida) |
| **Hardware**   | Partes A, C e D (sem registradores)                                                                                            |
| **Software**   | Parte B completa (interface, registradores, inicialização, conversão, temporização) + alertas de software                      |
| **Footprint**  | Seções 1, 5, 9 (encapsulamento), 16 e 17 + alertas de footprint                                                                |
| **Bare-metal** | Parte B completa + Parte F + alertas de software e bare-metal                                                                  |
| **Completo**   | Todas as partes + módulo do tipo de componente + alertas                                                                       |

---

## Modelo de saída

Use esta estrutura (omita o que não existir, mas **nunca omita** "Alertas" e "Informações não encontradas"):

````markdown
# Resumo de Datasheet – [PART NUMBER] (Telemetria de Foguetes)

**Fabricante:** [..]
**Documento:** [código] – Rev. [X] ([data])
**Categoria:** [..]
**Status:** [Active / NRND / EOL / Preliminary]
**Documentos adicionais necessários:** [Reference Manual / Errata / App Notes: fornecido? sim/não]

## 1. Visão geral

[1 a 3 frases.]

## 2. Veredito rápido para o foguete

[Atende / Atende com ressalvas / Não atende] – [2 a 3 linhas com o motivo principal.]

---

# PARTE A – HARDWARE

## 3. Especificações-chave

| Parâmetro | Símbolo | Min | Typ | Max | Unid. | Condição | Pág. |
| --------- | ------- | --- | --- | --- | ----- | -------- | ---- |

## 4. Limites absolutos (NÃO exceder)

| Parâmetro | Valor | Pág. |
| --------- | ----- | ---- |

## 5. Condições recomendadas de operação

- ...

## 6. Pinagem

| Pino | Nome | Tipo | Função | Observações |
| ---- | ---- | ---- | ------ | ----------- |

## 7. Circuito de aplicação típico

- **Componentes externos obrigatórios:** [..]
- **Pull-ups / terminações / cristal:** [..]
- **Fórmulas de dimensionamento:** [..]

## 8. Térmica e proteções

- ...

## 9. Encapsulamento, montagem e layout

- Encapsulamento / footprint / pitch / MSL: [..]
- Orientação dos eixos / posição no PCB / antena / porta de pressão: [..]
- Pedido (part numbers e sufixos): [..]

---

# PARTE B – SOFTWARE / BIBLIOTECA EM C

## 10. Interface de comunicação

- **Tipo:** [I²C / SPI / UART]
- **Endereço I²C (7 bits):** 0x__ (alternativo: 0x__ via pino [..])
- **Modo SPI / velocidade máx.:** [..]
- **Formato da transação:** [bit R/W, auto-incremento, dummy bytes]
- **Níveis lógicos:** [..]

## 11. Mapa de registradores

### 11.1 Tabela-resumo

| Endereço | Nome | Acesso | Reset | Descrição | Pág. |
| -------- | ---- | ------ | ----- | --------- | ---- |

### 11.2 Detalhe por registrador

#### `NOME_REG` – 0x__ – Reset 0x__ – R/W

| Bits | Campo | Acesso | Reset | Descrição / valores |
| ---- | ----- | ------ | ----- | ------------------- |

[Repetir para cada registrador relevante.]

### 11.3 Registradores de dados

- Endereços, tamanho, formato (signed/unsigned), endianness, endereço inicial para burst, coerência (BDU)

## 12. Sequência de inicialização

1. Aguardar [..] ms após power-on
2. Reset: [registrador/bit/valor] → aguardar [..] ms
3. Ler `[WHO_AM_I]` (0x__) → esperado `0x__`
4. Configurar: [registrador] = 0x__ ([significado])
5. Habilitar medição: [..]
6. Verificar dado pronto: [DRDY / status / interrupção]

## 13. Conversão de dados e temporização

- **Sensibilidade por faixa:** [tabela]
- **Fórmulas raw → unidade física:** [..]
- **Coeficientes de calibração (endereços e tipos):** [..]
- **Algoritmo de compensação:** [passo a passo, com tipos de dados]
- **ODR / tempos de conversão / atrasos obrigatórios:** [..]
- **FIFO e interrupções:** [..]

## 14. Esqueleto sugerido para a biblioteca em C

(Opcional, a pedido do usuário. Gerar apenas com valores extraídos do datasheet.)

```c
/* Endereços */
#define DEV_I2C_ADDR        0x__
#define REG_WHO_AM_I        0x__
#define WHO_AM_I_VALUE      0x__

/* Campos de bits */
#define CTRL1_ODR_Pos       4U
#define CTRL1_ODR_Msk       (0x0FU << CTRL1_ODR_Pos)

/* Protótipos */
int dev_read_reg(uint8_t reg, uint8_t *buf, uint16_t len);
int dev_write_reg(uint8_t reg, const uint8_t *buf, uint16_t len);
int dev_init(void);
```
````

---

# PARTE D – FOGUETE

## 15. Adequação ao ambiente de voo

| Item | Veredito (Atende / Não atende / Não informado) | Justificativa e página |
| ---- | ---------------------------------------------- | ---------------------- |

---

# PARTE E – FOOTPRINT E SÍMBOLO (KiCad)

## 16. Dados para o footprint

### 16.1 Encapsulamento

- **Nome / tipo / nº de pinos:** [..]
- **Código JEDEC / drawing no.:** [..]
- **Desenho na página:** [..]
- **Pino 1:** [posição e indicador]
- **Vista do desenho:** [TOP / BOTTOM]
- **Pad exposto:** [nº, dimensão, ligação exigida]

### 16.2 Dimensões mecânicas

| Símbolo | Descrição | Min | Nom | Max | mm  | Pág. |
| ------- | --------- | --- | --- | --- | --- | ---- |

### 16.3 Land pattern recomendado

- **Pads (tamanho / pitch / posição):** [..]
- **Pad térmico:** [..]
- **Máscara de solda:** [..]
- **Stencil / pasta (janelas):** [..]
- **Vias térmicas:** [..]
- **Keep-out:** [..]
- **Land pattern não fornecido?** [sim/não; se sim, indicar que o footprint será calculado por IPC-7351]

### 16.4 Tabela de pads para o KiCad

| Pad nº | Sinal | Tipo | Forma | X (mm) | Y (mm) | Tam. X | Tam. Y | Camadas | Origem (extraído/calculado) |
| ------ | ----- | ---- | ----- | ------ | ------ | ------ | ------ | ------- | --------------------------- |

### 16.5 Convenções e montagem

- **Nome sugerido do footprint:** [..]
- **Camadas (Fab, SilkS, CrtYd):** [..]
- **MSL / reflow / solda manual:** [..]
- **Modelo 3D (altura, STEP disponível?):** [..]

## 17. Dados para o símbolo

| Pino nº | Nome | Tipo elétrico (KiCad) | Grupo/lado | Funções alternativas | Observações |
| ------- | ---- | --------------------- | ---------- | -------------------- | ----------- |

**Propriedades:** MPN: [..] | Fabricante: [..] | Datasheet: [..] | Footprint sugerido: [..] | Códigos de fornecedor: [..]

---

# PARTE F – BIBLIOTECA BARE-METAL

## 18. Requisitos da biblioteca bare-metal

### 18.1 Camada de portabilidade

| Função exigida do host               | Requisito do datasheet                           |
| ------------------------------------ | ------------------------------------------------ |
| `read(reg, buf, len)` / `write(...)` | [velocidade, modo, repeated start, tamanho máx.] |
| `cs_write(level)`                    | [tempos de CS]                                   |
| `delay_ms` / `delay_us`              | [menor atraso exigido]                           |
| `gpio_read(int)` / EXTI              | [DRDY/INT, polaridade]                           |

### 18.2 Arquivos e nomenclatura

- `dev.h`, `dev.c`, `dev_regs.h`, `dev_port.h`
- Prefixos: `DEV_REG_<NOME>`, `<REG>_<CAMPO>_Pos/_Msk`, enums `DEV_<CAMPO>_<VALOR>`

### 18.3 API mínima

| Função                 | Registradores / sequência usados | Observações        |
| ---------------------- | -------------------------------- | ------------------ |
| `dev_init()`           | [..]                             | [..]               |
| `dev_config()`         | [..]                             | [..]               |
| `dev_read_raw()`       | [..]                             | [..]               |
| `dev_read_converted()` | [..]                             | [unidade de saída] |

### 18.4 Tipos, conversões e execução

- **Tipos exigidos:** [int16/int32/int64/float]
- **Endianness e sinal:** [..]
- **Modelo de execução:** [bloqueante / não bloqueante; polling / IRQ / DMA]
- **Barramento compartilhado:** [..]

### 18.5 Erros, recuperação e testes

- **Timeouts e códigos de erro:** [..]
- **Recuperação de falha:** [..]
- **Testes de bancada (WHO_AM_I, self-test, leituras de sanidade):** [..]
- **Recursos do MCU host:** [instância de barramento, GPIOs, EXTI, DMA]

## 19. Alertas e pontos de atenção

- ⚠️ [alerta com página]

## 20. Informações não encontradas

- [o que foi procurado e não consta no documento fornecido]

```

---

## Checklist final (antes de responder)

- [ ] Todos os valores têm **unidade, condição e página**?
- [ ] O **mapa de registradores** está completo, com **bits, reset, tipo de acesso e valores codificados**?
- [ ] O **endereço do dispositivo** está informado em 7 e 8 bits (I²C) ou o formato de comando está claro (SPI)?
- [ ] A **sequência de inicialização** está na ordem correta, com atrasos?
- [ ] As **fórmulas de conversão** e o **algoritmo de compensação** estão completos e com tipos de dados?
- [ ] Verifiquei **endianness, sinal e alinhamento** dos dados?
- [ ] A seção de **adequação ao foguete** foi preenchida, incluindo limites de faixa, GNSS e RF?
- [ ] Footprint: dimensões mecânicas em mm com **min/nom/max e página**?
- [ ] Footprint: **pino 1, vista do desenho (TOP/BOTTOM) e pad exposto** definidos, sem risco de espelhamento?
- [ ] Footprint: land pattern **extraído** ou, se ausente, explicitamente marcado como **calculado**?
- [ ] Footprint: tabela de pads com coordenadas, tamanhos e camadas (Y positivo para baixo no KiCad)?
- [ ] Símbolo: **tipo elétrico de cada pino** e propriedades (MPN, Datasheet, Footprint) preenchidos?
- [ ] Bare-metal: camada de portabilidade, API mínima, tipos, modelo de execução e **recuperação de falhas** descritos?
- [ ] Nada foi inventado? O que faltou está em "Informações não encontradas"?
- [ ] Registradores ausentes por o datasheet remeter a outro documento (ex.: Reference Manual) foram sinalizados?

---

## Como usar esta skill

Envie este arquivo como instrução junto com o PDF do datasheet (e o Reference Manual, se existir), e peça, por exemplo:

> Use a skill `datasheet-telemetria-foguetes` no datasheet anexo. Nível: Completo. Componente: [função na telemetria, ex.: barômetro para detectar apogeu]. MCU host: [ex.: STM32F4, 3,3 V]. Bateria: [ex.: LiPo 2S]. Aceleração máxima esperada: [ex.: 12 g]. Altitude de apogeu: [ex.: 3000 m].

Quanto mais contexto de missão (aceleração, altitude, bateria, MCU, interface livre), mais preciso será o veredito da Seção 15 e mais útil o esqueleto de código.

**Para footprint e símbolo no KiCad:** garanta que o PDF contenha as páginas de *Package Outline* / *Land Pattern* (às vezes ficam em um documento separado, como "Package Information" ou "Mechanical Drawing"). Se não estiverem no arquivo enviado, o LLM deve avisar em vez de deduzir as dimensões.

**Dica para datasheets muito longos:** se o LLM não conseguir ler o documento inteiro, peça a extração em rodadas: primeiro Partes A, C, D (hardware e foguete), depois Parte B e F (registradores e biblioteca bare-metal), informando as páginas do mapa de registradores, e por fim Parte E (footprint e símbolo), informando as páginas dos desenhos mecânicos.
```
