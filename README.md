# README - Documentação de Comentários do Sistema da Câmara (`main.c`)

> Este documento foi gerado automaticamente a partir da extração completa de todos os comentários presentes no código-fonte original de `main.c` antes de sua remoção.

## Sumário

- [Visão Geral e Arquitetura do Sistema](#visão-geral-e-arquitetura-do-sistema) (1 comentários)
- [1. Limites e Tipos (Layout e Estruturas de Dados)](#1-limites-e-tipos-layout-e-estruturas-de-dados) (26 comentários)
- [2. Dados de Apoio (Constantes, Nomes e Utilitários)](#2-dados-de-apoio-constantes-nomes-e-utilitários) (3 comentários)
- [3. Medidas em Caracteres Visíveis (UTF-8)](#3-medidas-em-caracteres-visíveis-utf-8) (4 comentários)
- [4. Paginação](#4-paginação) (5 comentários)
- [5. Quebra de Linha e Alinhamento](#5-quebra-de-linha-e-alinhamento) (5 comentários)
- [6. Cabeçalho e Rodapé](#6-cabeçalho-e-rodapé) (5 comentários)
- [7. Tabela em Caixa](#7-tabela-em-caixa) (18 comentários)
- [8. Renderização](#8-renderização) (3 comentários)
- [9. Leitura da Base (Listas Encadeadas)](#9-leitura-da-base-listas-encadeadas) (5 comentários)
- [10. Console: Avisos, Telas e Leitura Segura](#10-console-avisos-telas-e-leitura-segura) (11 comentários)
- [11. Nomes de Arquivo](#11-nomes-de-arquivo) (4 comentários)
- [12. Ações sobre Blocos (Lista Encadeada)](#12-ações-sobre-blocos-lista-encadeada) (17 comentários)
- [13. Arquivo de Metadados (.meta)](#13-arquivo-de-metadados-meta) (6 comentários)
- [14. Salvar e Abrir Documentos](#14-salvar-e-abrir-documentos) (8 comentários)
- [15. Menu do Editor](#15-menu-do-editor) (3 comentários)
- [16. Base de Dados: Geração Aleatória e Exclusão](#16-base-de-dados-geração-aleatória-e-exclusão) (15 comentários)
- [17. Função Principal (main) e Menu Inicial](#17-função-principal-main-e-menu-inicial) (4 comentários)

---

## Visão Geral e Arquitetura do Sistema

### Comentário #1 (Linhas 1–45)
* `Código associado:` `#include <stdio.h>`*

```text
 =====================================================================
   main.c - Sistema da Camara: base de dados + editor de relatorio por blocos

   O QUE FAZ
     1) BASE DE DADOS: 5 arquivos .txt (vereadores, reunioes, projetos,
        presencas, votacoes), lidos para LISTAS ENCADEADAS. O menu inicial
        gera dados aleatorios e apaga dados da base.
     2) EDITOR DE RELATORIO: o usuario junta BLOCOS (texto, tabela, quebra
        de pagina) numa LISTA ENCADEADA (BlocoDocumento). O programa quebra
        as linhas, alinha, desenha tabelas em caixa, pagina e grava
        NOME.txt. A estrutura vai em NOME.meta, para editar depois.
     Veja PLANO_PAGINACAO.md e GUIA_APRESENTACAO.md.

   COMPILAR / RODAR
     gcc-o main main.c        (rodar na pasta dos .txt)
     O arquivo e UTF-8; no Windows o console e trocado para UTF-8 no main().

   MAPA DO ARQUIVO (cada parte usa so as anteriores)
     1  Limites e tipos        constantes, listas encadeadas, Cabecalho, Render
     2  Dados de apoio         listas de nomes/temas e funcoes basicas de arquivo
     3  Medidas UTF-8          largura VISIVEL (nao bytes)
     4  Paginacao              emitir_linha / quebrar_pagina
     5  Quebra de linha        emitir_paragrafo (+ alinhamento)
     6  Cabecalho e rodape
     7  Tabela em caixa
     8  Renderizacao           junta tudo: renderizar() percorre a lista de blocos
     9  Leitura da base        carregar_* / liberar_* (listas encadeadas)
     10 Console                avisos, limpar tela, leitura segura
     11 Nomes de arquivo       sanear_nome
     12 Acoes sobre blocos     adicionar, inserir quebra, remover, editar
     13 Arquivo .meta          salvar_meta / carregar_meta / autosalvar
     14 Salvar e abrir         salvar_documento / abrir_documento
     15 Menu do editor         editar_documento
     16 Base de dados          gerar dados aleatorios / apagar
     17 main                   menu inicial

   IDEIAS-CHAVE (para explicar)
     - Tudo que vai para a tela/arquivo passa por emitir_linha(): ela e
       o UNICO lugar que conta linhas e troca de pagina.
     - O mesmo codigo que imprime tambem CONTA: com Render.out == NULL
       nada e escrito, so se mede (usado para validar o tamanho do texto).
     - Tamanhos sempre em caracteres visiveis, nunca strlen (acentos).
     - Toda alteracao e gravada na hora (autosave) em rascunho.meta ou
       NOME.meta: se o programa fechar, nada se perde.
   =====================================================================
```

---

## 1. Limites e Tipos (Layout e Estruturas de Dados)

### Comentário #2 (Linha 59)
* `Código associado:` `#define LARGURA_LINHA 70    /* colunas VISIVEIS por linha */`*

>  ===== 1. LIMITES E TIPOS (unico lugar para ajustar o layout) =====

### Comentário #3 (Linha 60)
* `Código associado:` `#define LINHAS_POR_PAGINA 24`*

>  colunas VISIVEIS por linha

### Comentário #4 (Linha 63)
* `Código associado:` `#define LINHAS_RODAPE 6     /* quantas linhas emitir_rodape() usa */`*

>  quantas linhas emitir_cabecalho() usa

### Comentário #5 (Linha 64)
* `Código associado:` `- no maximo LINHAS_POR_PAGINA linhas ja contando a linha em branco final`*

>  quantas linhas emitir_rodape() usa

### Comentário #6 (Linhas 66–68)
* `Código associado:` `#define TAM_TEXTO (LARGURA_LINHA * (LINHAS_POR_PAGINA - 1))`*

```text
 Uma caixa de texto nunca pode ser maior que uma pagina:
   - no maximo LINHAS_POR_PAGINA linhas ja contando a linha em branco final
   - e no maximo TAM_TEXTO caracteres visiveis
```

### Comentário #7 (Linha 70)
* `Código associado:` `#define LINHA_BUF (LARGURA_LINHA * 4 + 16) /* uma linha impressa, em bytes */`*

>  bytes: acentos UTF-8 ocupam 2

### Comentário #8 (Linha 71)
* `Código associado:` `#define MAX_COLS 6`*

>  uma linha impressa, em bytes

### Comentário #9 (Linha 75)
* `Código associado:` `separador + 1 linha de dados + borda final */`*

>  menor largura para a qual uma coluna encolhe

### Comentário #10 (Linhas 77–78)
* `Código associado:` `#define LINHAS_MIN_TABELA 6`*

```text
 uma tabela so comeca numa pagina se couber: titulo + borda + cabecalho +
   separador + 1 linha de dados + borda final
```

### Comentário #11 (Linha 80)
* `Código associado:` `tabela pode mostrar. O bloco de tabela guarda so o numero da fonte. */`*

>  caracteres do texto mostrados na lista de blocos

### Comentário #12 (Linhas 82–83)
* `Código associado:` `typedef struct`*

```text
 Fonte = um arquivo de dados (.txt com campos separados por ';') que uma
   tabela pode mostrar. O bloco de tabela guarda so o numero da fonte.
```

### Comentário #13 (Linha 101)
* `Código associado:` `#define IDX_VER 0`*

>  indices (0..4) de cada fonte em FONTES[]

### Comentário #14 (Linha 108)
* `Código associado:` `typedef struct Vereador`*

>  ---- registros da base: uma LISTA ENCADEADA para cada arquivo ----

### Comentário #15 (Linha 158)
* `Código associado:` `enum TipoBloco`*

>  ---- blocos do documento: LISTA ENCADEADA ----

### Comentário #16 (Linha 160)
* `Código associado:` `enum TipoBloco`*

>  tipos de bloco e de alinhamento (os numeros tambem vao para o .meta)

### Comentário #17 (Linhas 175–178)
* `Código associado:` `typedef struct BlocoDocumento`*

```text
 Um no da lista = um pedaco do documento. Campos usados por tipo:
   BLOCO_TEXTO  -> conteudo_texto, alinhamento
   BLOCO_TABELA -> fonte_tabela (1..5, indice em FONTES[]), colunas (1 = mostrar)
   BLOCO_QUEBRA -> nenhum
```

### Comentário #18 (Linha 190)
* `Código associado:` `typedef struct`*

>  dados da faixa de cima do relatorio

### Comentário #19 (Linha 197)
* `Código associado:` `} Cabecalho;`*

>  0 = valores padrao, usuario ainda nao preencheu

### Comentário #20 (Linha 200)
* `Código associado:` `typedef struct`*

>  "caneta" da renderizacao: onde escrever e em que ponto da pagina estamos

### Comentário #21 (Linha 203)
* `Código associado:` `int pagina;`*

>  NULL = modo contagem: so conta linhas, nao imprime

### Comentário #22 (Linha 205)
* `Código associado:` `} Render;`*

>  linhas ja usadas na pagina atual

### Comentário #23 (Linha 209)
* `Código associado:` `typedef struct`*

>  nome do arquivo enquanto o documento nao tem nome

### Comentário #24 (Linha 211)
* `Código associado:` `typedef struct`*

>  tudo que pertence a um documento aberto

### Comentário #25 (Linha 214)
* `Código associado:` `int nb;               /* quantos blocos a lista tem */`*

>  primeiro bloco da lista (NULL = documento vazio)

### Comentário #26 (Linha 215)
* `Código associado:` `Cabecalho cab;`*

>  quantos blocos a lista tem

### Comentário #27 (Linha 217)
* `Código associado:` `} Doc;`*

>  vazio = ainda sem nome (grava em rascunho.meta)

---

## 2. Dados de Apoio (Constantes, Nomes e Utilitários)

### Comentário #28 (Linha 220)
* `Código associado:` `const char *NOMES_BASE[] = {`*

>  ===== 2. DADOS DE APOIO =====

### Comentário #29 (Linha 222)
* `Código associado:` `const char *NOMES_BASE[] = {`*

>  listas

### Comentário #30 (Linha 387)
* `Código associado:` `char* extrair_campo(char **resto_da_linha) {`*

>  carregamento de dados

---

## 3. Medidas em Caracteres Visíveis (UTF-8)

### Comentário #31 (Linhas 407–409)
* `Código associado:` `static int larg_visual_n(const char *s, int nbytes)`*

```text
 ===== 3. MEDIDAS EM CARACTERES VISIVEIS (UTF-8) =====
   strlen() conta bytes: "ã" valeria 2 e desalinharia tudo. Aqui conta-se
   so os bytes que iniciam um caractere (os de continuacao sao 10xxxxxx).
```

### Comentário #32 (Linha 411)
* `Código associado:` `static int larg_visual_n(const char *s, int nbytes)`*

>  largura visivel dos primeiros nbytes bytes de s

### Comentário #33 (Linha 426)
* `Código associado:` `static int bytes_ate(const char *s, int nchars)`*

>  quantos bytes ocupam os primeiros nchars caracteres de s

### Comentário #34 (Linha 443)
* `Código associado:` `static void cortar_visual(char *s, int max_chars)`*

>  trunca s para no maximo max_chars caracteres visiveis (sem partir um acento)

---

## 4. Paginação

### Comentário #35 (Linhas 449–451)
* `Código associado:` `static void linha_crua(Render *r, const char *s)`*

```text
 ===== 4. PAGINACAO =====
   Regra unica: a pagina tem LINHAS_POR_PAGINA linhas. Quando enche, a
   proxima linha abre uma pagina nova (quebrar_pagina).
```

### Comentário #36 (Linha 453)
* `Código associado:` `static void linha_crua(Render *r, const char *s)`*

>  escreve sem checar limite (so para fechar tabela antes de quebrar)

### Comentário #37 (Linha 460)
* `Código associado:` `static void quebrar_pagina(Render *r, const char *motivo)`*

>  escreve a linha "---- fim da pagina N [motivo] ----" e zera o contador

### Comentário #38 (Linhas 479–480)
* `Código associado:` `static void emitir_linha(Render *r, const char *s)`*

```text
 toda linha normal passa por aqui: quebra a pagina sozinha no limite.
   Linha em branco nunca fica sozinha no topo nem forca uma quebra.
```

### Comentário #39 (Linha 483)
* `Código associado:` `{`*

>  modo contagem

---

## 5. Quebra de Linha e Alinhamento

### Comentário #40 (Linha 498)
* `Código associado:` `static void emitir_alinhado(Render *r, const char *s, int alinha)`*

>  ===== 5. QUEBRA DE LINHA + ALINHAMENTO =====

### Comentário #41 (Linha 500)
* `Código associado:` `static void emitir_alinhado(Render *r, const char *s, int alinha)`*

>  uma linha ja quebrada, com espacos a esquerda conforme o alinhamento

### Comentário #42 (Linhas 516–519)
* `Código associado:` `static void emitir_paragrafo(Render *r, const char *texto, int alinha)`*

```text
 Quebra o texto em linhas de ate LARGURA_LINHA caracteres, SEM partir
   palavras (a palavra que nao cabe vai inteira para a proxima linha; so
   uma palavra maior que a linha toda e cortada). O alinhamento e aplicado
   DEPOIS de quebrar, linha a linha. Termina com uma linha em branco.
```

### Comentário #43 (Linha 538)
* `Código associado:` `{`*

>  palavra maior que a linha: corta

### Comentário #44 (Linha 574)
* `Código associado:` `}`*

>  linha em branco entre paragrafos

---

## 6. Cabeçalho e Rodapé

### Comentário #45 (Linha 577)
* `Código associado:` `static void emitir_barra(Render *r, char c)`*

>  ===== 6. CABECALHO E RODAPE (mesma largura do resto) =====

### Comentário #46 (Linha 579)
* `Código associado:` `static void emitir_barra(Render *r, char c)`*

>  linha de LARGURA_LINHA repeticoes de c (ex.: "=====")

### Comentário #47 (Linha 588)
* `Código associado:` `static void emitir_texto_curto(Render *r, const char *fmt, ...)`*

>  printf de uma linha so, cortada em LARGURA_LINHA para nunca estourar

### Comentário #48 (Linhas 610–611)
* `Código associado:` `static void emitir_rodape(Render *r)`*

```text
 o rodape e sempre a ultima coisa; se nao couber inteiro, vai para uma
   pagina nova (nunca fica dividido)
```

### Comentário #49 (Linha 614)
* `Código associado:` `quebrar_pagina(r, "automática");`*

>  rodape inteiro na mesma pagina

---

## 7. Tabela em Caixa

### Comentário #50 (Linhas 629–631)
* `Código associado:` `typedef char Linha[MAX_COLS][TAM_CEL]; /* um registro do arquivo: ate MAX_COLS campos */`*

```text
 ===== 7. TABELA EM CAIXA =====
   Passos de emitir_tabela(): ler o arquivo -> calcular larguras das colunas
   -> desenhar, repetindo o cabecalho quando a tabela muda de pagina.
```

### Comentário #51 (Linha 633)
* `Código associado:` `typedef struct`*

>  um registro do arquivo: ate MAX_COLS campos

### Comentário #52 (Linha 635)
* `Código associado:` `typedef struct`*

>  quais colunas aparecem e com que largura (em caracteres visiveis)

### Comentário #53 (Linha 638)
* `Código associado:` `int idx[MAX_COLS];    /* idx[i] = coluna do arquivo que e a i-esima da tabela */`*

>  colunas ativas

### Comentário #54 (Linha 639)
* `Código associado:` `int larg[MAX_COLS];`*

>  idx[i] = coluna do arquivo que e a i-esima da tabela

### Comentário #55 (Linha 643)
* `Código associado:` `static void montar_borda(const Layout *L, char c, char *dest)`*

>  "+----+----+" (c = '-') ou "+====+====+" (c = '=', separa o cabecalho)

### Comentário #56 (Linha 657)
* `Código associado:` `static int escrever_celula(char *dest, const char *t, int w)`*

>  escreve a celula com exatamente w colunas visiveis (preenche ou corta com "...")

### Comentário #57 (Linha 686)
* `Código associado:` `static void montar_linha_dados(const Layout *L, char cel[][TAM_CEL], char *dest)`*

>  "| a | b |" com cada celula ajustada a sua largura

### Comentário #58 (Linhas 701–703)
* `Código associado:` `static int ler_arquivo(const char *caminho, Linha **out)`*

```text
 le o arquivo ';' para um vetor alocado dinamicamente (cresce com realloc:
   sem limite fixo de linhas). Ignora linhas vazias e comentarios '#'.
   Devolve o numero de linhas, ou -1 se nao abriu. O chamador faz free(*out).
```

### Comentário #59 (Linha 726)
* `Código associado:` `snprintf(dados[n][c], TAM_CEL, "%s", extrair_campo(&resto));`*

>  mesma funcao que os carregar_* usam

### Comentário #60 (Linhas 737–740)
* `Código associado:` `static Layout calcular_layout(const BlocoDocumento *b, Linha *dados, int n)`*

```text
 Larguras: comeca pela maior celula de cada coluna ativa (cabecalho
   incluso). Se a tabela passar de LARGURA_LINHA, tira 1 caractere por vez
   da coluna mais larga, ate caber (ou chegar em LARG_MIN_COL). O texto
   que sobra e cortado com "..." por escrever_celula().
```

### Comentário #61 (Linha 761)
* `Código associado:` `for (int i = 0; i < L.n; i++)`*

>  '|' inicial + (espaco + dado + espaco + '|') por coluna

### Comentário #62 (Linha 771)
* `Código associado:` `L.larg[maior]--;`*

>  nao da para encolher mais

### Comentário #63 (Linhas 777–779)
* `Código associado:` `static void emitir_topo_tabela(Render *r, const BlocoDocumento *b, const char *borda,`*

```text
 titulo + borda + cabecalho + separador. continuacao=1 muda o titulo.
   E chamada no inicio da tabela E em cada nova pagina: por isso o
   cabecalho se repete.
```

### Comentário #64 (Linhas 810–811)
* `Código associado:` `if (r->linhas + LINHAS_MIN_TABELA > LINHAS_POR_PAGINA)`*

```text
 nunca deixa so o cabecalho no fim da pagina: se nao cabem pelo menos
       LINHAS_MIN_TABELA linhas, a tabela inteira comeca na proxima
```

### Comentário #65 (Linha 818)
* `Código associado:` `if (r->linhas + 2 > LINHAS_POR_PAGINA)`*

>  precisa sobrar espaco para a linha + a borda de fechamento

### Comentário #66 (Linha 821)
* `Código associado:` `quebrar_pagina(r, "automática");`*

>  fecha a caixa nesta pagina

### Comentário #67 (Linha 823)
* `Código associado:` `}`*

>  REPETE o cabecalho

---

## 8. Renderização

### Comentário #68 (Linhas 833–835)
* `Código associado:` `static void renderizar(FILE *out, const BlocoDocumento *head, int marcas, const Cabecalho *cab)`*

```text
 ===== 8. RENDERIZACAO =====
   Percorre os blocos na ordem e chama o emissor de cada tipo. E a mesma
   funcao para tela, arquivo .txt e pre-visualizacao.
```

### Comentário #69 (Linha 837)
* `Código associado:` `static void renderizar(FILE *out, const BlocoDocumento *head, int marcas, const Cabecalho *cab)`*

>  marcas=1 escreve "[B n]" antes de cada bloco (pre-visualizacao)

### Comentário #70 (Linha 847)
* `Código associado:` `quebrar_pagina(&r, "manual");`*

>  nao gera pagina em branco se ja acabou de quebrar

---

## 9. Leitura da Base (Listas Encadeadas)

### Comentário #71 (Linha 861)
* `Código associado:` `Vereador *carregar_vereadores(const char *caminho)`*

>  ===== 9. LEITURA DA BASE (listas encadeadas) =====

### Comentário #72 (Linha 930)
* `Código associado:` `copiar_string(novo->hora, token, sizeof(novo->hora));`*

>  faltava ler a hora: a pauta recebia "16:00"

### Comentário #73 (Linha 974)
* `Código associado:` `token = extrair_campo(&resto);`*

>  faltavam autor, data e categoria: o "status" recebia o ID do autor

### Comentário #74 (Linha 1018)
* `Código associado:` `novo->id_presenca = atoi(token);`*

>  faltava o ID da presenca: tudo ficava deslocado

### Comentário #75 (Linha 1087)
* `Código associado:` `void liberar_vereadores(Vereador *head)`*

>  liberação de memória

---

## 10. Console: Avisos, Telas e Leitura Segura

### Comentário #76 (Linha 1144)
* `Código associado:` `static char g_aviso[800];`*

>  ===== 10. CONSOLE: avisos, tela e leitura segura =====

### Comentário #77 (Linha 1146)
* `Código associado:` `static char g_aviso[800];`*

>  mensagens que precisam sobreviver ao "apagar tela": aparecem acima do menu

### Comentário #78 (Linha 1149)
* `Código associado:` `static void aviso(const char *fmt, ...)`*

>  guarda a mensagem (printf) para o proximo mostrar_aviso()

### Comentário #79 (Linha 1162)
* `Código associado:` `static void mostrar_aviso(void)`*

>  imprime e esvazia as mensagens guardadas

### Comentário #80 (Linha 1181)
* `Código associado:` `static void descartar_resto_da_linha(void)`*

>  se o usuario digitou mais que o buffer, joga fora o resto da linha

### Comentário #81 (Linha 1197)
* `Código associado:` `invalida" nos menus) e 0 no fim da entrada (g_eof), que os menus tratam`*

>  1 quando a entrada acabou (Ctrl+Z / Ctrl+D): os menus encerram

### Comentário #82 (Linhas 1199–1201)
* `Código associado:` `static int ler_int(void)`*

```text
 le um numero de uma linha. Devolve -1 se nao for numero (cai em "opcao
   invalida" nos menus) e 0 no fim da entrada (g_eof), que os menus tratam
   como "sair" para nao ficarem presos em laco infinito.
```

### Comentário #83 (Linha 1208)
* `Código associado:` `}`*

>  fim da entrada: trata como "sair"

### Comentário #84 (Linha 1217)
* `Código associado:` `static void ler_linha(const char *prompt, char *dest, size_t tam)`*

>  le uma linha curta; vazio mantem o valor atual de dest

### Comentário #85 (Linha 1231)
* `Código associado:` `tmp[i] = '/';`*

>  '|' separa campos no .meta

### Comentário #86 (Linha 1236)
* `Código associado:` `static void configurar_cabecalho(Cabecalho *c)`*

>  tela de preenchimento do cabecalho do relatorio

---

## 11. Nomes de Arquivo

### Comentário #87 (Linha 1255)
* `Código associado:` `(U+00C0..U+00FF); '?' = sem equivalente */`*

>  ===== 11. NOMES DE ARQUIVO =====

### Comentário #88 (Linhas 1257–1258)
* `Código associado:` `static char letra_sem_acento(unsigned char b2)`*

```text
 letra sem acento para o 2o byte de um caractere UTF-8 que comeca com 0xC3
   (U+00C0..U+00FF); '?' = sem equivalente
```

### Comentário #89 (Linhas 1269–1270)
* `Código associado:` `static int sanear_nome(char *nome)`*

```text
 deixa so letras, numeros, '_' e '-' (acentos viram a letra base, espaco vira '_').
   devolve o tamanho final; 0 = nada aproveitavel
```

### Comentário #90 (Linha 1298)
* `Código associado:` `static void caminho_de(const char *nome, const char *ext, char *dest, size_t tam)`*

>  "NOME" + "meta" -> "NOME.meta"; nome vazio usa o do rascunho

---

## 12. Ações sobre Blocos (Lista Encadeada)

### Comentário #91 (Linhas 1312–1315)
* `Código associado:` `static int obter_proximo_id_bloco(const BlocoDocumento *head)`*

```text
 ===== 12. ACOES SOBRE BLOCOS (lista encadeada) =====
   Cada acao (novo_texto, nova_tabela, ...) devolve 1 se MUDOU o documento
   e 0 se nao; o menu usa isso para decidir se faz o autosave.
   Posicoes (pos) sao sempre 1-based, como o usuario ve na tela.
```

### Comentário #92 (Linha 1317)
* `Código associado:` `static int obter_proximo_id_bloco(const BlocoDocumento *head)`*

>  proximo ID livre: maior ID da lista + 1

### Comentário #93 (Linha 1327)
* `Código associado:` `static BlocoDocumento *bloco_na_pos(BlocoDocumento *head, int pos)`*

>  no na posicao pos (1..nb), ou NULL

### Comentário #94 (Linha 1335)
* `Código associado:` `static BlocoDocumento *criar_bloco(int tipo)`*

>  aloca um no zerado do tipo pedido (a lista so recebe o no depois de pronto)

### Comentário #95 (Linha 1344)
* `Código associado:` `static void listar_blocos(const Doc *d)`*

>  topo do menu do editor: uma linha por bloco

### Comentário #96 (Linhas 1376–1377)
* `Código associado:` `static void inserir(Doc *d, int pos, BlocoDocumento *novo)`*

```text
 liga o no na posicao pos (1..nb+1): pos == nb+1 acrescenta no fim.
   O no ganha um ID novo.
```

### Comentário #97 (Linha 1395)
* `Código associado:` `static int contar_linhas_texto(const char *texto, int alinha)`*

>  quantas linhas o texto ocupa depois da quebra, ja com a linha em branco final

### Comentário #98 (Linha 1403)
* `Código associado:` `static int ler_texto_bloco(BlocoDocumento *b)`*

>  le o texto do bloco; so aceita o que cabe em uma pagina. Linha vazia cancela.

### Comentário #99 (Linha 1416)
* `Código associado:` `tmp[len] = '\0';`*

>  sem \n: a linha era maior que tmp

### Comentário #100 (Linha 1418)
* `Código associado:` `descartar_resto_da_linha();`*

>  descarta o resto para nao contaminar o proximo prompt

### Comentário #101 (Linha 1442)
* `Código associado:` `static int montar_texto(BlocoDocumento *b)`*

>  pede alinhamento + texto para o no b. devolve 0 se cancelado

### Comentário #102 (Linha 1452)
* `Código associado:` `static int montar_tabela(BlocoDocumento *b)`*

>  pede fonte + colunas para o no b. devolve 0 se cancelado

### Comentário #103 (Linha 1515)
* `Código associado:` `static int nova_quebra(Doc *d)`*

>  quebra manual: vira um bloco, como sempre foi no main.c (BLOCO_QUEBRA)

### Comentário #104 (Linha 1536)
* `Código associado:` `static int remover(Doc *d)`*

>  desliga o no da lista e o libera

### Comentário #105 (Linhas 1566–1567)
* `Código associado:` `static int editar_alinhamento(BlocoDocumento *b, int pos)`*

```text
 muda SO o alinhamento de um bloco de texto, mantendo o texto.
   Recusa se, com o novo alinhamento, o texto deixar de caber numa pagina.
```

### Comentário #106 (Linhas 1594–1596)
* `Código associado:` `static int editar(Doc *d)`*

```text
 refaz um bloco existente (texto: so alinhamento OU texto novo; tabela:
   nova fonte/colunas). O no novo substitui o conteudo do antigo mantendo
   ID e posicao na lista. Se cancelar, o bloco original e mantido.
```

### Comentário #107 (Linha 1645)
* `Código associado:` `free(novo);`*

>  o endereco de 'atual' nao muda: o no anterior continua apontando para ele

---

## 13. Arquivo de Metadados (.meta)

### Comentário #108 (Linhas 1651–1658)
* `Código associado:` `static int salvar_meta(const char *caminho, const Doc *d)`*

```text
 ===== 13. ARQUIVO .meta =====
   Texto simples, um bloco por trecho:
     # CABECALHO|municipio|data|sessao|tipo      (so se configurado)
     # BLOCO_INICIO|ID:n|TIPO:1|ALINHA:a         + linha do texto
     # BLOCO_INICIO|ID:n|TIPO:2|FONTE:f|COLS:1,0,1,...
     # BLOCO_INICIO|ID:n|TIPO:3                  (quebra de pagina)
     # BLOCO_FIM
   Mesmo formato do main.c (que ignora as linhas que nao conhece).
```

### Comentário #109 (Linha 1665)
* `Código associado:` `if (d->cab.configurado)`*

>  linha extra: o main.c ignora tudo que nao comeca com "# BLOCO_INICIO"

### Comentário #110 (Linha 1666)
* `Código associado:` `if (d->cab.configurado)`*

>  so grava se o usuario configurou: um rascunho com cabecalho padrao nao conta como configurado

### Comentário #111 (Linhas 1683–1684)
* `Código associado:` `static void autosalvar(const Doc *d)`*

```text
 grava a cada alteracao: se o programa travar, nada se perde.
   Documento sem nome vai para rascunho.meta; com nome, para NOME.meta.
```

### Comentário #112 (Linhas 1695–1698)
* `Código associado:` `static int carregar_meta(const char *caminho, Doc *d)`*

```text
 le o .meta para dentro de d (que deve estar vazio): cada bloco vira um no
   anexado ao FIM da lista (guarda-se o ultimo no em 'tail', sem percorrer a
   lista toda). Texto grande demais e cortado (com aviso) em vez de estourar
   o buffer. Devolve 0 se nao abriu.
```

### Comentário #113 (Linha 1758)
* `Código associado:` `{`*

>  bloco corrompido: ignora

---

## 14. Salvar e Abrir Documentos

### Comentário #114 (Linha 1783)
* `Código associado:` `Devolve 0 se o usuario cancelou (nome vazio num documento ainda sem nome). */`*

>  ===== 14. SALVAR E ABRIR =====

### Comentário #115 (Linhas 1785–1786)
* `Código associado:` `static int pedir_nome(const Doc *d, char novo[NOME_MAX])`*

```text
 pergunta o nome do documento e o deixa "limpo" em novo[].
   Devolve 0 se o usuario cancelou (nome vazio num documento ainda sem nome).
```

### Comentário #116 (Linha 1800)
* `Código associado:` `if (sanear_nome(tmp) == 0 || strcmp(tmp, RASCUNHO) == 0)`*

>  vazio mantem o nome atual; sem nome = cancela

### Comentário #117 (Linhas 1811–1812)
* `Código associado:` `static void salvar_documento(Doc *d)`*

```text
 Fluxo de "Salvar": cabecalho (se faltar) -> mostra o documento final ->
   ENTER -> nome -> confirma sobrescrita -> grava NOME.txt + NOME.meta
```

### Comentário #118 (Linha 1815)
* `Código associado:` `configurar_cabecalho(&d->cab);`*

>  como no main.c: pergunta uma vez ao gerar

### Comentário #119 (Linha 1853)
* `Código associado:` `}`*

>  o rascunho virou documento com nome

### Comentário #120 (Linha 1861)
* `Código associado:` `static int listar_metas(char nomes[][NOME_MAX])`*

>  lista os *.meta da pasta atual (sem a extensao)

### Comentário #121 (Linha 1883)
* `Código associado:` `static void documento_vazio(Doc *d)`*

>  esvazia o documento: LIBERA a lista de blocos antes de zerar a struct

---

## 15. Menu do Editor

### Comentário #122 (Linhas 1941–1943)
* `Código associado:` `static void editar_documento(Doc *d)`*

```text
 ===== 15. MENU DO EDITOR =====
   Cada volta do laco: limpa a tela -> lista os blocos -> mostra avisos ->
   le a opcao -> executa -> se mudou algo, AUTOSAVE (obrigatorio).
```

### Comentário #123 (Linha 1945)
* `Código associado:` `static void editar_documento(Doc *d)`*

>  devolve quando o usuario escolhe Sair (volta ao menu inicial)

### Comentário #124 (Linha 1995)
* `Código associado:` `}`*

>  a cada alteracao, obrigatoriamente

---

## 16. Base de Dados: Geração Aleatória e Exclusão

### Comentário #125 (Linhas 2000–2005)
* `Código associado:` `#define N_ARR(a) ((int)(sizeof(a) / sizeof((a)[0])))`*

```text
 ===== 16. BASE DE DADOS: GERAR ALEATORIOS E APAGAR =====
   Os 5 arquivos .txt (FONTES[]) sao a "base". Aqui se cria dados de teste
   e se esvazia a base. Cada gerador le as LISTAS ENCADEADAS (carregar_*) do
   que ja existe. Ordem de dependencia: vereadores e reunioes -> projetos
   (precisam de autor) -> presencas (reuniao x vereador) -> votacoes
   (so votam os vereadores presentes).
```

### Comentário #126 (Linha 2008)
* `Código associado:` `#define MAX_REU 100`*

>  presença = reuniões x vereadores; votação = projetos x vereadores

### Comentário #127 (Linha 2012)
* `Código associado:` `static const int ORDEM_DADOS[5] = {IDX_VER, IDX_REU, IDX_PROJ, IDX_PRES, IDX_VOT};`*

>  bytes da tabela reuniao x vereador

### Comentário #128 (Linha 2014)
* `Código associado:` `static const int ORDEM_DADOS[5] = {IDX_VER, IDX_REU, IDX_PROJ, IDX_PRES, IDX_VOT};`*

>  ordem de exibição (índices em FONTES): quem vem antes alimenta quem vem depois

### Comentário #129 (Linha 2017)
* `Código associado:` `static long aleat(long n)`*

>  no Windows RAND_MAX = 32767: junta duas chamadas para sortear em faixas maiores

### Comentário #130 (Linha 2025)
* `Código associado:` `static long id_unico(long lo, long hi)`*

>  sorteia um ID em [lo, hi] que ainda não saiu; reinicie com memset(g_usado)

### Comentário #131 (Linha 2041)
* `Código associado:` `static int contar_registros(int idx)`*

>  quantos registros a lista do arquivo idx tem hoje (le o arquivo para a lista)

### Comentário #132 (Linha 2065)
* `Código associado:` `static int pedir_qtd(const char *rotulo, int max)`*

>  pede uma quantidade em 1..max; 0 = cancelou/inválido

### Comentário #133 (Linhas 2082–2083)
* `Código associado:` `static int gerar_vereadores(int n)`*

```text
 geradores: devolvem o nº de registros gravados, -1 se não abriu o arquivo,
   -2 se falta a base da qual dependem, -3 se a combinação é grande demais
```

### Comentário #134 (Linha 2121)
* `Código associado:` `static int gerar_projetos(int n)`*

>  o autor de cada projeto e um vereador sorteado da LISTA de vereadores

### Comentário #135 (Linha 2153)
* `Código associado:` `static int gerar_presencas(void)`*

>  uma linha para cada par reuniao x vereador (percorre as duas listas)

### Comentário #136 (Linhas 2190–2193)
* `Código associado:` `static int gerar_votacoes_completas_otimizada(const char *caminho_saida, Reuniao *lista_reunioes,`*

```text
 Cada projeto e votado numa reuniao sorteada, so pelos vereadores PRESENTES
   nela. Para saber "esse vereador estava presente?" em O(1) monta-se uma
   tabela plana [reuniao][vereador] a partir da lista de presencas
   (indice = reuniao * colunas + vereador).
```

### Comentário #137 (Linha 2254)
* `Código associado:` `static int gerar_votacoes(void)`*

>  carrega as listas, gera as votacoes e libera tudo

### Comentário #138 (Linha 2336)
* `Código associado:` `if (!qtd || !confirmar_substituir(idx))`*

>  presenças/votações: derivadas das outras bases

### Comentário #139 (Linha 2389)
* `Código associado:` `if (f)`*

>  esvazia, mantém o arquivo

---

## 17. Função Principal (main) e Menu Inicial

### Comentário #140 (Linha 2402)
* `Código associado:` `int main(void)`*

>  ===== 17. MAIN: MENU INICIAL =====

### Comentário #141 (Linha 2406)
* `Código associado:` `#ifdef _WIN32`*

>  semente: dados aleatorios diferentes a cada execucao

### Comentário #142 (Linha 2412)
* `Código associado:` `for (;;)`*

>  comeca zerada: head == NULL

### Comentário #143 (Linha 2449)
* `Código associado:` `return 0;`*

>  free geral da lista de blocos

---
