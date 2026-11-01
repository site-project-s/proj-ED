#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define LARGURA_LINHA 70
#define LINHAS_POR_PAGINA 24

#define LINHAS_CABECALHO 6
#define LINHAS_RODAPE 6

#define TAM_TEXTO (LARGURA_LINHA * (LINHAS_POR_PAGINA - 1))
#define TAM_BUF_TEXTO (TAM_TEXTO * 2 + 1)
#define LINHA_BUF (LARGURA_LINHA * 4 + 16)

#define MAX_COLS 6
#define TAM_CEL 128
#define LARG_MIN_COL 6

#define LINHAS_MIN_TABELA 6
#define PREVIA_TEXTO 35

typedef struct
{
    const char *titulo;
    const char *arquivo;
    int n_cols;
    const char *nomes[MAX_COLS];
} Fonte;

static const Fonte FONTES[] = {
    {"Vereadores", "OsCapaPreta.txt", 4, {"ID_VEREADOR", "NOME", "PARTIDO", "EMAIL"}},
    {"Reuniões", "reunioes.txt", 4, {"ID_REUNIAO", "DATA_REUNIAO", "HORA", "PAUTA"}},
    {"Votações", "votacao.txt", 5, {"ID_VOTACAO", "ID_PROJETO", "ID_REUNIAO", "ID_VEREADOR", "VOTO"}},
    {"Projetos de Lei", "projetos.txt", 6, {"ID_PROJ", "TITULO", "ID_AUTOR", "DATA_MOSTRA", "CATEGORIA", "STATUS"}},
    {"Presenças", "presenca.txt", 4, {"ID_PRESENCA", "ID_REUNIAO", "ID_VEREADOR", "STATUS_PRESENCA"}},
};
#define N_FONTES 5

#define IDX_VER 0
#define IDX_REU 1
#define IDX_VOT 2
#define IDX_PROJ 3
#define IDX_PRES 4

typedef struct Vereador
{
    int id_vereador;
    char nome[60];
    char partido[15];
    char email[50];
    struct Vereador *prox;
} Vereador;

typedef struct Reuniao
{
    int id_reuniao;
    char data_reuniao[11];
    char hora[6];
    char pauta[100];
    struct Reuniao *prox;
} Reuniao;

typedef struct Projeto
{
    int id_proj;
    int id_autor;
    char titulo[100];
    char data_mostra[11];
    char categoria[30];
    char status[20];
    struct Projeto *prox;
} Projeto;

typedef struct Presenca
{
    int id_presenca;
    int id_reuniao;
    int id_vereador;
    char status_presenca[15];
    struct Presenca *prox;
} Presenca;

typedef struct Votacao
{
    int id_votacao;
    int id_projeto;
    int id_reuniao;
    int id_vereador;
    char voto[15];
    struct Votacao *prox;
} Votacao;

enum TipoBloco
{
    BLOCO_TEXTO = 1,
    BLOCO_TABELA = 2,
    BLOCO_QUEBRA = 3
};

enum Alinhamento
{
    ALINHA_ESQUERDA = 1,
    ALINHA_CENTRO = 2,
    ALINHA_DIREITA = 3
};

typedef struct BlocoDocumento
{
    int id;
    int tipo;
    int alinhamento;
    char conteudo_texto[TAM_BUF_TEXTO];
    int fonte_tabela;
    int colunas[MAX_COLS];
    struct BlocoDocumento *prox;
} BlocoDocumento;

typedef struct
{
    char municipio[60];
    char data[12];
    char tipo[30];
    int sessao;
    int configurado;
} Cabecalho;

typedef struct
{
    FILE *out;
    int pagina;
    int linhas;
} Render;

#define NOME_MAX 48
#define RASCUNHO "rascunho"

typedef struct
{
    BlocoDocumento *head;
    int nb;
    Cabecalho cab;
    char nome[NOME_MAX];
} Doc;

const char *NOMES_BASE[] = {
    "Arthur", "Oliveira", "Patriota",
    "Lemos", "Conceicao", "Emidio",
    "Araujo", "Andrey", "Assis",
    "Correia", "Cayo", "Lins",
    "Mathaus", "Mickael", "D'Silva",
    "Chaves", "Santiago", "Ferreira",
    "Santos", "Silver", "Prata",
    "Contes", "Mike", "Azevedo",
    "Junior", "Descassa", "Sentiu",
    "Lenicia", "Joao", "Mario",
    "Carlos", "Silva", "Santos",
    "Souza", "Pereira", "Lima",
    "Ana", "Beatriz", "Lucas",
    "Leticia", "Mandraque", "Jonas",
    "INSIRA_NOME_BRABO"};
const int TOTAL_NOMES = sizeof(NOMES_BASE) / sizeof(NOMES_BASE[0]);

const char *PARTIDOS[] = {
    "PIC", "PICHICA", "PIQB",
    "PIF", "TPM", "PF",
    "PSCRR", "NOVO", "ANTIGO",
    "PSC", "PNET", "PP",
    "FON"};
const int TOTAL_PARTIDOS = sizeof(PARTIDOS) / sizeof(PARTIDOS[0]);

const char *DOMINIOS[] = {
    "camara.leg.br",
    "gmail.com",
    "outlook.com",
    "ic.ufal.br",
    "lves.com.br",
    "camara.gov.br",
    "fbi.gov.us",
    "hacker.tor.br",
    "empty.none.go"};
const int TOTAL_DOMINIOS = sizeof(DOMINIOS) / sizeof(DOMINIOS[0]);

const char *ACOES_BASE[] = {
    "Institucionalizacao", "Regulamentacao",
    "Memificacao", "Fomento",
    "Proibicao", "Padronizacao",
    "Valorizacao", "Nacionalizacao",
    "Precarizacao", "Clusteriacao",
    "Oficializacao", "Consideracao",
    "Perdao", "Informe",
    "Ementa", "Documentalizacao"};
const int TOTAL_ACOES = sizeof(ACOES_BASE) / sizeof(ACOES_BASE[0]);

const char *TEMAS_BASE[] = {
    "Saude", "Violencia", "Representacao",
    "Patriotismo", "Panificacao", "Cambio",
    "Educacao", "Cultura", "Criptomoedas",
    "Gastronomia", "Seguranca", "CyberSeguranca",
    "Residuos", "Polica"};
const int TOTAL_TEMAS = sizeof(TEMAS_BASE) / sizeof(TEMAS_BASE[0]);

const char *CATEGORIAS[] = {
    "Saude", "Educacao", "Infraestrutura",
    "Meio Ambiente", "Seguranca", "Financas",
    "Migueh", "Inutil", "Trapaceio", "Manobra Legal",
    "Greve"};

const int TOTAL_CATEGORIAS = sizeof(CATEGORIAS) / sizeof(CATEGORIAS[0]);

const char *STATUS_PROJ[] = {
    "Em Tramitacao",
    "Aprovado",
    "Rejeitado",
    "Arquivado"};
const int TOTAL_STATUS_PROJ = sizeof(STATUS_PROJ) / sizeof(STATUS_PROJ[0]);

const char *STATUS_PRESENCA[] = {
    "presente",
    "ausente",
    "justificado"};
const int TOTAL_PRESENCA = sizeof(STATUS_PRESENCA) / sizeof(STATUS_PRESENCA[0]);

const char *VOTOS[] = {
    "Favoravel",
    "Contrario",
    "Abstencao"};
const int TOTAL_VOTOS = sizeof(VOTOS) / sizeof(VOTOS[0]);

void copiar_string(char *pra_ca, const char *daqui, size_t tamanho_max)
{
    if (!pra_ca || !daqui || tamanho_max == 0)
        return;
    strncpy(pra_ca, daqui, tamanho_max - 1);
    pra_ca[tamanho_max - 1] = '\0';
}

void sortear_tema_complexo(char *destino, size_t tam_max, const char *prefixo)
{
    int acao = rand() % TOTAL_ACOES;
    int tema1 = rand() % TOTAL_TEMAS;
    int tema2 = rand() % TOTAL_TEMAS;

    if (prefixo && strlen(prefixo) > 0)
    {
        snprintf(destino, tam_max, "%s %s de %s de %s", prefixo, ACOES_BASE[acao], TEMAS_BASE[tema1], TEMAS_BASE[tema2]);
    }
    else
    {
        snprintf(destino, tam_max, "%s de %s de %s", ACOES_BASE[acao], TEMAS_BASE[tema1], TEMAS_BASE[tema2]);
    }
}

char* extrair_campo(char **resto_da_linha) {
    if (*resto_da_linha == NULL) return "";

    char *inicio = *resto_da_linha;
    char *ponto_virgula = strchr(inicio, ';');

    if (ponto_virgula != NULL) {
        *ponto_virgula = '\0';
        *resto_da_linha = ponto_virgula + 1;
    } else {
        *resto_da_linha = NULL;
    }

    return inicio;
}

static int larg_visual_n(const char *s, int nbytes)
{
    int n = 0;
    for (int i = 0; i < nbytes && s[i]; i++)
        if (((unsigned char)s[i] & 0xC0) != 0x80)
            n++;
    return n;
}

static int larg_visual(const char *s)
{
    return larg_visual_n(s, (int)strlen(s));
}

static int bytes_ate(const char *s, int nchars)
{
    int i = 0, n = 0;
    while (s[i])
    {
        if (((unsigned char)s[i] & 0xC0) != 0x80)
        {
            if (n == nchars)
                break;
            n++;
        }
        i++;
    }
    return i;
}

static void cortar_visual(char *s, int max_chars)
{
    s[bytes_ate(s, max_chars)] = '\0';
}

static void linha_crua(Render *r, const char *s)
{
    fprintf(r->out, "%s\n", s);
    r->linhas++;
}

static void quebrar_pagina(Render *r, const char *motivo)
{
    char rot[64], marca[LINHA_BUF];
    snprintf(rot, sizeof(rot), " fim da página %d [%s] ", r->pagina, motivo);
    int n = larg_visual(rot);
    int esq = (LARGURA_LINHA - n) / 2;
    int pos = 0;
    for (int i = 0; i < esq; i++)
        marca[pos++] = '-';
    pos += snprintf(marca + pos, sizeof(marca) - pos, "%s", rot);
    for (int i = esq + n; i < LARGURA_LINHA; i++)
        marca[pos++] = '-';
    marca[pos] = '\0';
    fprintf(r->out, "%s\n\n", marca);
    r->pagina++;
    r->linhas = 0;
}

static void emitir_linha(Render *r, const char *s)
{
    if (!r->out)
    {
        r->linhas++;
        return;
    }
    if (s[0] == '\0')
    {
        if (r->linhas == 0 || r->linhas >= LINHAS_POR_PAGINA)
            return;
    }
    else if (r->linhas >= LINHAS_POR_PAGINA)
        quebrar_pagina(r, "automática");
    linha_crua(r, s);
}

static void emitir_alinhado(Render *r, const char *s, int alinha)
{
    char buf[LINHA_BUF + LARGURA_LINHA];
    int len = larg_visual(s);
    int esp = 0;
    if (alinha == ALINHA_DIREITA)
        esp = LARGURA_LINHA - len;
    else if (alinha == ALINHA_CENTRO)
        esp = (LARGURA_LINHA - len) / 2;
    if (esp < 0)
        esp = 0;
    snprintf(buf, sizeof(buf), "%*s%s", esp, "", s);
    emitir_linha(r, buf);
}

static void emitir_paragrafo(Render *r, const char *texto, int alinha)
{
    char linha[LINHA_BUF] = "";
    int len_b = 0, len_v = 0;
    const char *p = texto;

    while (*p)
    {
        while (*p == ' ')
            p++;
        if (!*p)
            break;
        const char *ini = p;
        while (*p && *p != ' ')
            p++;
        int tam_b = (int)(p - ini);
        int tam_v = larg_visual_n(ini, tam_b);

        while (tam_v > LARGURA_LINHA)
        {
            if (len_v > 0)
            {
                emitir_alinhado(r, linha, alinha);
                linha[0] = '\0';
                len_b = len_v = 0;
            }
            int corte = bytes_ate(ini, LARGURA_LINHA);
            char pedaco[LINHA_BUF];
            memcpy(pedaco, ini, corte);
            pedaco[corte] = '\0';
            emitir_alinhado(r, pedaco, alinha);
            ini += corte;
            tam_b -= corte;
            tam_v = larg_visual_n(ini, tam_b);
        }

        if (len_v > 0 && len_v + 1 + tam_v > LARGURA_LINHA)
        {
            emitir_alinhado(r, linha, alinha);
            linha[0] = '\0';
            len_b = len_v = 0;
        }
        if (len_v > 0)
        {
            linha[len_b++] = ' ';
            len_v++;
        }
        memcpy(linha + len_b, ini, tam_b);
        len_b += tam_b;
        len_v += tam_v;
        linha[len_b] = '\0';
    }
    if (len_v > 0)
        emitir_alinhado(r, linha, alinha);
    emitir_linha(r, "");
}

static void emitir_barra(Render *r, char c)
{
    char b[LARGURA_LINHA + 1];
    memset(b, c, LARGURA_LINHA);
    b[LARGURA_LINHA] = '\0';
    emitir_linha(r, b);
}

static void emitir_texto_curto(Render *r, const char *fmt, ...)
{
    char buf[LINHA_BUF];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    cortar_visual(buf, LARGURA_LINHA);
    emitir_linha(r, buf);
}

static void emitir_cabecalho(Render *r, const Cabecalho *c)
{
    emitir_barra(r, '=');
    emitir_texto_curto(r, " CÂMARA MUNICIPAL DE %s", c->municipio);
    emitir_texto_curto(r, " RELATÓRIO DO FORMULÁRIO LEGISLATIVO");
    emitir_texto_curto(r, " Sessão Nº: %02d (%s) | Data: %s", c->sessao, c->tipo, c->data);
    emitir_barra(r, '=');
    emitir_linha(r, "");
}

static void emitir_rodape(Render *r)
{
    if (r->linhas + LINHAS_RODAPE > LINHAS_POR_PAGINA)
        quebrar_pagina(r, "automática");

    char data[16];
    time_t agora = time(NULL);
    strftime(data, sizeof(data), "%d/%m/%Y", localtime(&agora));

    emitir_linha(r, "");
    emitir_barra(r, '=');
    emitir_texto_curto(r, " Documento gerado pelo Sistema Legislativo em C");
    emitir_texto_curto(r, " Assinatura do Responsável: ______________________");
    emitir_texto_curto(r, " Data de emissão: %s", data);
    emitir_barra(r, '=');
}

typedef char Linha[MAX_COLS][TAM_CEL];

typedef struct
{
    int n;
    int idx[MAX_COLS];
    int larg[MAX_COLS];
} Layout;

static void montar_borda(const Layout *L, char c, char *dest)
{
    int pos = 0;
    dest[pos++] = '+';
    for (int i = 0; i < L->n; i++)
    {
        for (int k = 0; k < L->larg[i] + 2; k++)
            dest[pos++] = c;
        dest[pos++] = '+';
    }
    dest[pos] = '\0';
}

static int escrever_celula(char *dest, const char *t, int w)
{
    int pos = 0, lv = larg_visual(t);
    if (lv <= w)
    {
        int nb = (int)strlen(t);
        memcpy(dest, t, nb);
        pos = nb;
        for (int i = lv; i < w; i++)
            dest[pos++] = ' ';
    }
    else if (w >= 4)
    {
        int nb = bytes_ate(t, w - 3);
        memcpy(dest, t, nb);
        pos = nb;
        memcpy(dest + pos, "...", 3);
        pos += 3;
    }
    else
    {
        int nb = bytes_ate(t, w);
        memcpy(dest, t, nb);
        pos = nb;
    }
    return pos;
}

static void montar_linha_dados(const Layout *L, char cel[][TAM_CEL], char *dest)
{
    int pos = 0;
    dest[pos++] = '|';
    for (int i = 0; i < L->n; i++)
    {
        dest[pos++] = ' ';
        pos += escrever_celula(dest + pos, cel[L->idx[i]], L->larg[i]);
        dest[pos++] = ' ';
        dest[pos++] = '|';
    }
    dest[pos] = '\0';
}

static int ler_arquivo(const char *caminho, Linha **out)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return -1;
    int cap = 64, n = 0;
    Linha *dados = malloc(cap * sizeof(Linha));
    char linha[1024];
    while (dados && fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0' || linha[0] == '#')
            continue;
        if (n == cap)
        {
            Linha *maior = realloc(dados, cap * 2 * sizeof(Linha));
            if (!maior)
                break;
            dados = maior;
            cap *= 2;
        }
        char *resto = linha;
        for (int c = 0; c < MAX_COLS; c++)
            snprintf(dados[n][c], TAM_CEL, "%s", extrair_campo(&resto));
        n++;
    }
    fclose(f);
    if (!dados)
        return -1;
    *out = dados;
    return n;
}

static Layout calcular_layout(const BlocoDocumento *b, Linha *dados, int n)
{
    const Fonte *F = &FONTES[b->fonte_tabela - 1];
    Layout L = {0};
    for (int c = 0; c < F->n_cols; c++)
        if (b->colunas[c])
        {
            L.idx[L.n] = c;
            int w = larg_visual(F->nomes[c]);
            for (int i = 0; i < n; i++)
            {
                int l = larg_visual(dados[i][c]);
                if (l > w)
                    w = l;
            }
            L.larg[L.n++] = w;
        }

    for (;;)
    {
        int total = 1;
        for (int i = 0; i < L.n; i++)
            total += L.larg[i] + 3;
        if (total <= LARGURA_LINHA)
            break;
        int maior = 0;
        for (int i = 1; i < L.n; i++)
            if (L.larg[i] > L.larg[maior])
                maior = i;
        if (L.larg[maior] <= LARG_MIN_COL)
            break;
        L.larg[maior]--;
    }
    return L;
}

static void emitir_topo_tabela(Render *r, const BlocoDocumento *b, const char *borda,
                               const char *cab, const char *sep, int continuacao)
{
    emitir_texto_curto(r, continuacao ? "Tabela: %s (continuação)" : "Tabela: %s", FONTES[b->fonte_tabela - 1].titulo);
    emitir_linha(r, borda);
    emitir_linha(r, cab);
    emitir_linha(r, sep);
}

static void emitir_tabela(Render *r, const BlocoDocumento *b)
{
    const Fonte *F = &FONTES[b->fonte_tabela - 1];
    Linha *dados = NULL;
    int n = ler_arquivo(F->arquivo, &dados);
    if (n < 0)
    {
        emitir_texto_curto(r, "[ERRO] não abriu '%s'", F->arquivo);
        return;
    }

    Layout L = calcular_layout(b, dados, n);

    char borda[LINHA_BUF], sep[LINHA_BUF], cab[LINHA_BUF], lin[LINHA_BUF];
    montar_borda(&L, '-', borda);
    montar_borda(&L, '=', sep);
    char nomes[MAX_COLS][TAM_CEL];
    for (int c = 0; c < F->n_cols; c++)
        snprintf(nomes[c], TAM_CEL, "%s", F->nomes[c]);
    montar_linha_dados(&L, nomes, cab);

    if (r->linhas + LINHAS_MIN_TABELA > LINHAS_POR_PAGINA)
        quebrar_pagina(r, "automática");
    emitir_topo_tabela(r, b, borda, cab, sep, 0);

    for (int i = 0; i < n; i++)
    {

        if (r->linhas + 2 > LINHAS_POR_PAGINA)
        {
            linha_crua(r, borda);
            quebrar_pagina(r, "automática");
            emitir_topo_tabela(r, b, borda, cab, sep, 1);
        }
        montar_linha_dados(&L, dados[i], lin);
        emitir_linha(r, lin);
    }
    emitir_linha(r, borda);
    emitir_linha(r, "");
    free(dados);
}

static void renderizar(FILE *out, const BlocoDocumento *head, int marcas, const Cabecalho *cab)
{
    Render r = {out, 1, 0};
    emitir_cabecalho(&r, cab);
    int pos = 1;
    for (const BlocoDocumento *b = head; b; b = b->prox, pos++)
    {
        if (b->tipo == BLOCO_QUEBRA)
        {
            if (r.linhas > 0)
                quebrar_pagina(&r, "manual");
            continue;
        }
        if (marcas)
            fprintf(out, "[B%d]\n", pos);
        if (b->tipo == BLOCO_TEXTO)
            emitir_paragrafo(&r, b->conteudo_texto, b->alinhamento);
        else
            emitir_tabela(&r, b);
    }
    emitir_rodape(&r);
}

Vereador *carregar_vereadores(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    Vereador *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Vereador *novo = (Vereador *)malloc(sizeof(Vereador));
        if (!novo) continue;
        memset(novo, 0, sizeof(Vereador));

        char *resto = linha;

        char *token = extrair_campo(&resto);
        novo->id_vereador = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->nome, token, sizeof(novo->nome));

        token = extrair_campo(&resto);
        copiar_string(novo->partido, token, sizeof(novo->partido));

        token = extrair_campo(&resto);
        copiar_string(novo->email, token, sizeof(novo->email));

        if (!head) {
            head = novo;
            tail = novo;
        } else {
            tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}
Reuniao *carregar_reunioes(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    Reuniao *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Reuniao *novo = (Reuniao *)malloc(sizeof(Reuniao));
        if (!novo) continue;
        memset(novo, 0, sizeof(Reuniao));

        char *resto = linha;

        char *token = extrair_campo(&resto);
        novo->id_reuniao = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->data_reuniao, token, sizeof(novo->data_reuniao));

        token = extrair_campo(&resto);
        copiar_string(novo->hora, token, sizeof(novo->hora));

        token = extrair_campo(&resto);
        copiar_string(novo->pauta, token, sizeof(novo->pauta));

        if (!head) {
            head = novo;
            tail = novo;
        } else {
            tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}

Projeto *carregar_projetos(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    Projeto *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Projeto *novo = (Projeto *)malloc(sizeof(Projeto));
        if (!novo) continue;
        memset(novo, 0, sizeof(Projeto));

        char *resto = linha;

        char *token = extrair_campo(&resto);
        novo->id_proj = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->titulo, token, sizeof(novo->titulo));

        token = extrair_campo(&resto);
        novo->id_autor = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->data_mostra, token, sizeof(novo->data_mostra));

        token = extrair_campo(&resto);
        copiar_string(novo->categoria, token, sizeof(novo->categoria));

        token = extrair_campo(&resto);
        copiar_string(novo->status, token, sizeof(novo->status));

        if (!head) {
            head = novo;
            tail = novo;
        } else {
            tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}

Presenca *carregar_presencas(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    Presenca *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Presenca *novo = (Presenca *)malloc(sizeof(Presenca));
        if (!novo) continue;
        memset(novo, 0, sizeof(Presenca));

        char *resto = linha;

        char *token = extrair_campo(&resto);
        novo->id_presenca = atoi(token);

        token = extrair_campo(&resto);
        novo->id_reuniao = atoi(token);

        token = extrair_campo(&resto);
        novo->id_vereador = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->status_presenca, token, sizeof(novo->status_presenca));

        if (!head) {
            head = novo;
            tail = novo;
        } else {
            tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}
Votacao *carregar_votacoes(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    Votacao *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;

        Votacao *novo = (Votacao *)malloc(sizeof(Votacao));
        if (!novo) continue;
        memset(novo, 0, sizeof(Votacao));

        char *resto = linha;

        char *token = extrair_campo(&resto);
        novo->id_votacao = atoi(token);

        token = extrair_campo(&resto);
        novo->id_projeto = atoi(token);

        token = extrair_campo(&resto);
        novo->id_reuniao = atoi(token);

        token = extrair_campo(&resto);
        novo->id_vereador = atoi(token);

        token = extrair_campo(&resto);
        copiar_string(novo->voto, token, sizeof(novo->voto));

        if (!head) {
            head = novo;
            tail = novo;
        } else {
            tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}

void liberar_vereadores(Vereador *head)
{
    while (head)
    {
        Vereador *t = head;
        head = head->prox;
        free(t);
    }
}
void liberar_reunioes(Reuniao *head)
{
    while (head)
    {
        Reuniao *t = head;
        head = head->prox;
        free(t);
    }
}
void liberar_projetos(Projeto *head)
{
    while (head)
    {
        Projeto *t = head;
        head = head->prox;
        free(t);
    }
}
void liberar_presencas(Presenca *head)
{
    while (head)
    {
        Presenca *t = head;
        head = head->prox;
        free(t);
    }
}
void liberar_votacoes(Votacao *head)
{
    while (head)
    {
        Votacao *t = head;
        head = head->prox;
        free(t);
    }
}
void liberar_blocos(BlocoDocumento *head)
{
    while (head)
    {
        BlocoDocumento *t = head;
        head = head->prox;
        free(t);
    }
}

static char g_aviso[800];

static void aviso(const char *fmt, ...)
{
    char tmp[300];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (g_aviso[0])
        strncat(g_aviso, "\n", sizeof(g_aviso) - strlen(g_aviso) - 1);
    strncat(g_aviso, tmp, sizeof(g_aviso) - strlen(g_aviso) - 1);
}

static void mostrar_aviso(void)
{
    if (g_aviso[0])
    {
        printf("%s\n", g_aviso);
        g_aviso[0] = '\0';
    }
}

static void limpar_tela(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void descartar_resto_da_linha(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void pausar(const char *msg)
{
    char t[64];
    printf("%s", msg);
    if (fgets(t, sizeof(t), stdin) && !strchr(t, '\n'))
        descartar_resto_da_linha();
}

static int g_eof;

static int ler_int(void)
{
    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin))
    {
        g_eof = 1;
        return 0;
    }
    if (!strchr(buf, '\n'))
        descartar_resto_da_linha();
    char *fim;
    long v = strtol(buf, &fim, 10);
    return (fim == buf) ? -1 : (int)v;
}

static void ler_linha(const char *prompt, char *dest, size_t tam)
{
    char tmp[256];
    printf("%s", prompt);
    if (!fgets(tmp, sizeof(tmp), stdin))
        return;
    size_t len = strcspn(tmp, "\r\n");
    if (tmp[len] == '\0')
        descartar_resto_da_linha();
    tmp[len] = '\0';
    if (len == 0)
        return;
    for (size_t i = 0; i < len; i++)
        if (tmp[i] == '|')
            tmp[i] = '/';
    snprintf(dest, tam, "%s", tmp);
}

static void configurar_cabecalho(Cabecalho *c)
{
    limpar_tela();
    printf("=== CONFIGURAÇÃO DO CABEÇALHO (vazio mantém o atual) ===\n\n");
    char prompt[120];
    snprintf(prompt, sizeof(prompt), "Município [%s]: ", c->municipio);
    ler_linha(prompt, c->municipio, sizeof(c->municipio));
    snprintf(prompt, sizeof(prompt), "Data da sessão DD/MM/AAAA [%s]: ", c->data);
    ler_linha(prompt, c->data, sizeof(c->data));
    snprintf(prompt, sizeof(prompt), "Tipo da sessão [%s]: ", c->tipo);
    ler_linha(prompt, c->tipo, sizeof(c->tipo));
    printf("Número da sessão [%d]: ", c->sessao);
    int s = ler_int();
    if (s > 0)
        c->sessao = s;
    c->configurado = 1;
}

static char letra_sem_acento(unsigned char b2)
{
    static const char T[] =
        "AAAAAA?CEEEEIIII"
        "?NOOOOO??UUUUY??"
        "aaaaaa?ceeeeiiii"
        "?nooooo??uuuuy?y";
    return T[b2 & 0x3F];
}

static int sanear_nome(char *nome)
{
    char out[NOME_MAX];
    int j = 0;
    for (int i = 0; nome[i] && j < NOME_MAX - 1; i++)
    {
        unsigned char c = (unsigned char)nome[i];
        char r = 0;
        if (c == 0xC3 && nome[i + 1])
        {
            r = letra_sem_acento((unsigned char)nome[++i]);
            if (r == '?')
                r = 0;
        }
        else if (c == ' ')
            r = '_';
        else if (c == '_' || c == '-' || (c >= '0' && c <= '9') ||
                 (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            r = (char)c;
        if (r)
            out[j++] = r;
    }
    out[j] = '\0';
    memcpy(nome, out, j + 1);
    return j;
}

static void caminho_de(const char *nome, const char *ext, char *dest, size_t tam)
{
    snprintf(dest, tam, "%s.%s", nome[0] ? nome : RASCUNHO, ext);
}

static int arquivo_existe(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (f)
        fclose(f);
    return f != NULL;
}

static int obter_proximo_id_bloco(const BlocoDocumento *head)
{
    int max_id = 0;
    for (; head; head = head->prox)
        if (head->id > max_id)
            max_id = head->id;
    return max_id + 1;
}

static BlocoDocumento *bloco_na_pos(BlocoDocumento *head, int pos)
{
    for (int i = 1; head && i < pos; i++)
        head = head->prox;
    return head;
}

static BlocoDocumento *criar_bloco(int tipo)
{
    BlocoDocumento *b = (BlocoDocumento *)calloc(1, sizeof(BlocoDocumento));
    if (b)
        b->tipo = tipo;
    return b;
}

static void listar_blocos(const Doc *d)
{
    printf("==================== ESTRUTURA DO DOCUMENTO ====================\n");
    printf(" Documento: %s\n\n", d->nome[0] ? d->nome : "(sem nome - guardado como rascunho)");
    if (!d->head)
        printf(" [Nenhum bloco cadastrado]\n");
    int pos = 1;
    for (const BlocoDocumento *b = d->head; b; b = b->prox, pos++)
    {
        if (b->tipo == BLOCO_TEXTO)
        {
            char prev[LINHA_BUF];
            snprintf(prev, sizeof(prev), "%s", b->conteudo_texto);
            int longo = larg_visual(prev) > PREVIA_TEXTO;
            cortar_visual(prev, PREVIA_TEXTO);
            printf(" %d. [TEXTO, alinha %d] \"%s%s\"\n", pos, b->alinhamento, prev, longo ? "..." : "");
        }
        else if (b->tipo == BLOCO_TABELA)
        {
            const Fonte *F = &FONTES[b->fonte_tabela - 1];
            printf(" %d. [TABELA] %s | colunas:", pos, F->titulo);
            for (int c = 0; c < F->n_cols; c++)
                if (b->colunas[c])
                    printf(" %s", F->nomes[c]);
            printf("\n");
        }
        else
            printf(" %d. [QUEBRA DE PÁGINA]\n", pos);
    }
}

static void inserir(Doc *d, int pos, BlocoDocumento *novo)
{
    novo->id = obter_proximo_id_bloco(d->head);
    if (pos <= 1 || !d->head)
    {
        novo->prox = d->head;
        d->head = novo;
    }
    else
    {
        BlocoDocumento *ant = bloco_na_pos(d->head, pos - 1);
        novo->prox = ant->prox;
        ant->prox = novo;
    }
    d->nb++;
}

static int contar_linhas_texto(const char *texto, int alinha)
{
    Render r = {NULL, 1, 0};
    emitir_paragrafo(&r, texto, alinha);
    return r.linhas;
}

static int ler_texto_bloco(BlocoDocumento *b)
{
    char tmp[TAM_BUF_TEXTO * 2];

    for (;;)
    {
        printf("\nDigite o texto livre (máx. %d linhas / %d caracteres; vazio cancela): ",
               LINHAS_POR_PAGINA - 1, TAM_TEXTO);
        if (!fgets(tmp, sizeof(tmp), stdin))
            return 0;

        for (int i = 0; tmp[i] != '\0'; i++)
            if (tmp[i] == '\t')
                tmp[i] = ' ';
        size_t len = strcspn(tmp, "\r\n");
        int sobrou = (tmp[len] == '\0');
        tmp[len] = '\0';
        if (sobrou)
            descartar_resto_da_linha();
        if (len == 0)
            return 0;

        if (sobrou || len >= sizeof(b->conteudo_texto) || larg_visual(tmp) > TAM_TEXTO)
        {
            printf("[!] Texto com mais de %d caracteres. Divida em dois blocos.\n", TAM_TEXTO);
            continue;
        }
        int linhas = contar_linhas_texto(tmp, b->alinhamento);
        if (linhas > LINHAS_POR_PAGINA)
        {
            printf("[!] Ocupa %d linhas, mas a página só tem %d. Reduza ou divida em dois blocos.\n",
                   linhas, LINHAS_POR_PAGINA);
            continue;
        }
        snprintf(b->conteudo_texto, sizeof(b->conteudo_texto), "%s", tmp);
        aviso("Texto ocupa %d de %d linhas da página (%d caracteres).",
              linhas, LINHAS_POR_PAGINA, larg_visual(tmp));
        return 1;
    }
}

static int montar_texto(BlocoDocumento *b)
{
    printf("Alinhamento: 1-Esquerda 2-Centro 3-Direita: ");
    b->alinhamento = ler_int();
    if (b->alinhamento < ALINHA_ESQUERDA || b->alinhamento > ALINHA_DIREITA)
        b->alinhamento = ALINHA_ESQUERDA;
    return ler_texto_bloco(b);
}

static int montar_tabela(BlocoDocumento *b)
{
    printf("=== SELEÇÃO DE PLANILHA ===\n");
    for (int i = 0; i < N_FONTES; i++)
        printf("%d - %s (%s)\n", i + 1, FONTES[i].titulo, FONTES[i].arquivo);
    printf("Escolha a fonte: ");
    int fonte = ler_int();
    if (fonte < 1 || fonte > N_FONTES)
        fonte = 1;
    b->fonte_tabela = fonte;

    const Fonte *F = &FONTES[fonte - 1];
    printf("\n--- Escolha as colunas (1-Sim / 0-Não) ---\n");
    int algum = 0;
    for (int c = 0; c < F->n_cols; c++)
    {
        printf("%s? ", F->nomes[c]);
        b->colunas[c] = (ler_int() == 1);
        algum |= b->colunas[c];
    }
    if (!algum)
        aviso("[!] Nenhuma coluna escolhida: tabela cancelada.");
    return algum;
}

static int novo_texto(Doc *d)
{
    BlocoDocumento *b = criar_bloco(BLOCO_TEXTO);
    if (!b)
    {
        aviso("[ERRO] sem memória para o bloco.");
        return 0;
    }
    if (!montar_texto(b))
    {
        free(b);
        aviso("[!] Bloco de texto cancelado.");
        return 0;
    }
    inserir(d, d->nb + 1, b);
    aviso("[OK] Bloco de texto adicionado.");
    return 1;
}

static int nova_tabela(Doc *d)
{
    BlocoDocumento *b = criar_bloco(BLOCO_TABELA);
    if (!b)
    {
        aviso("[ERRO] sem memória para o bloco.");
        return 0;
    }
    if (!montar_tabela(b))
    {
        free(b);
        return 0;
    }
    inserir(d, d->nb + 1, b);
    aviso("[OK] Bloco de tabela adicionado.");
    return 1;
}

static int nova_quebra(Doc *d)
{
    printf("Inserir quebra ANTES de qual bloco (1-%d, ou %d para o fim)? ", d->nb, d->nb + 1);
    int pos = ler_int();
    if (pos < 1 || pos > d->nb + 1)
    {
        aviso("[!] Posição inválida.");
        return 0;
    }
    BlocoDocumento *b = criar_bloco(BLOCO_QUEBRA);
    if (!b)
    {
        aviso("[ERRO] sem memória para o bloco.");
        return 0;
    }
    inserir(d, pos, b);
    aviso("[OK] Quebra de página inserida.");
    return 1;
}

static int remover(Doc *d)
{
    printf("Remover qual bloco (1-%d)? ", d->nb);
    int pos = ler_int();
    if (pos < 1 || pos > d->nb)
    {
        aviso("[!] Bloco inválido.");
        return 0;
    }
    BlocoDocumento *alvo;
    if (pos == 1)
    {
        alvo = d->head;
        d->head = alvo->prox;
    }
    else
    {
        BlocoDocumento *ant = bloco_na_pos(d->head, pos - 1);
        alvo = ant->prox;
        ant->prox = alvo->prox;
    }
    free(alvo);
    d->nb--;
    aviso("[OK] Bloco removido.");
    return 1;
}

static const char *NOMES_ALINHA[] = {"", "esquerda", "centro", "direita"};

static int editar_alinhamento(BlocoDocumento *b, int pos)
{
    printf("Novo alinhamento: 1-Esquerda 2-Centro 3-Direita: ");
    int novo = ler_int();
    if (novo < ALINHA_ESQUERDA || novo > ALINHA_DIREITA)
    {
        aviso("[!] Alinhamento inválido, bloco mantido.");
        return 0;
    }
    int linhas = contar_linhas_texto(b->conteudo_texto, novo);
    if (linhas > LINHAS_POR_PAGINA)
    {
        aviso("[!] Com esse alinhamento o texto ocupa %d linhas (máx. %d). Mantido.",
              linhas, LINHAS_POR_PAGINA);
        return 0;
    }
    if (novo == b->alinhamento)
    {
        aviso("Alinhamento já era esse, nada mudou.");
        return 0;
    }
    b->alinhamento = novo;
    aviso("[OK] Bloco %d agora alinhado à %s.", pos, NOMES_ALINHA[novo]);
    return 1;
}

static int editar(Doc *d)
{
    printf("Editar qual bloco (1-%d)? ", d->nb);
    int pos = ler_int();
    if (pos < 1 || pos > d->nb)
    {
        aviso("[!] Bloco inválido.");
        return 0;
    }
    BlocoDocumento *atual = bloco_na_pos(d->head, pos);
    BlocoDocumento *novo = criar_bloco(atual->tipo);
    if (!novo)
    {
        aviso("[ERRO] sem memória para o bloco.");
        return 0;
    }
    int ok = 0;

    if (atual->tipo == BLOCO_TEXTO)
    {
        printf("Texto atual: %s\nAlinhamento atual: %s\n", atual->conteudo_texto, NOMES_ALINHA[atual->alinhamento]);
        printf("O que mudar? 1-Só o alinhamento / 2-Texto e alinhamento / 0-Cancelar: ");
        int o = ler_int();
        if (o == 1)
        {
            free(novo);
            return editar_alinhamento(atual, pos);
        }
        if (o == 2)
            ok = montar_texto(novo);
    }
    else if (atual->tipo == BLOCO_TABELA)
        ok = montar_tabela(novo);
    else
    {
        free(novo);
        aviso("[!] Quebra de página não tem o que editar (remova ou insira outra).");
        return 0;
    }

    if (!ok)
    {
        free(novo);
        aviso("[!] Edição cancelada, bloco mantido.");
        return 0;
    }
    novo->id = atual->id;
    novo->prox = atual->prox;
    *atual = *novo;
    free(novo);
    aviso("[OK] Bloco %d atualizado.", pos);
    return 1;
}

static int salvar_meta(const char *caminho, const Doc *d)
{
    FILE *f = fopen(caminho, "w");
    if (!f)
        return 0;

    if (d->cab.configurado)
        fprintf(f, "# CABECALHO|%s|%s|%d|%s\n\n", d->cab.municipio, d->cab.data, d->cab.sessao, d->cab.tipo);
    for (const BlocoDocumento *b = d->head; b; b = b->prox)
    {
        if (b->tipo == BLOCO_TEXTO)
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:1|ALINHA:%d\n%s\n# BLOCO_FIM\n\n", b->id, b->alinhamento, b->conteudo_texto);
        else if (b->tipo == BLOCO_TABELA)
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:2|FONTE:%d|COLS:%d,%d,%d,%d,%d,%d\n# BLOCO_FIM\n\n",
                    b->id, b->fonte_tabela, b->colunas[0], b->colunas[1], b->colunas[2],
                    b->colunas[3], b->colunas[4], b->colunas[5]);
        else
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:3\n# BLOCO_FIM\n\n", b->id);
    }
    return fclose(f) == 0;
}

static void autosalvar(const Doc *d)
{
    char meta[NOME_MAX + 8];
    caminho_de(d->nome, "meta", meta, sizeof(meta));
    if (salvar_meta(meta, d))
        aviso("(salvo automaticamente em %s)", meta);
    else
        aviso("[ERRO] autosave falhou: não gravou %s", meta);
}

static int carregar_meta(const char *caminho, Doc *d)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return 0;
    BlocoDocumento *tail = NULL;
    char linha[700];
    while (fgets(linha, sizeof(linha), f))
    {
        if (strncmp(linha, "# CABECALHO|", 12) == 0)
        {
            Cabecalho c = d->cab;
            if (sscanf(linha, "# CABECALHO|%59[^|]|%11[^|]|%d|%29[^\r\n]", c.municipio, c.data, &c.sessao, c.tipo) == 4)
            {
                c.configurado = 1;
                d->cab = c;
            }
            continue;
        }
        if (strncmp(linha, "# BLOCO_INICIO", 14) != 0)
            continue;
        BlocoDocumento *b = criar_bloco(0);
        if (!b)
            break;
        sscanf(linha, "# BLOCO_INICIO|ID:%d|TIPO:%d", &b->id, &b->tipo);

        if (b->tipo == BLOCO_TEXTO)
        {
            b->alinhamento = ALINHA_ESQUERDA;
            sscanf(linha, "# BLOCO_INICIO|ID:%*d|TIPO:%*d|ALINHA:%d", &b->alinhamento);
            if (b->alinhamento < ALINHA_ESQUERDA || b->alinhamento > ALINHA_DIREITA)
                b->alinhamento = ALINHA_ESQUERDA;
        }
        else if (b->tipo == BLOCO_TABELA)
        {
            b->fonte_tabela = 1;
            sscanf(linha, "# BLOCO_INICIO|ID:%*d|TIPO:%*d|FONTE:%d|COLS:%d,%d,%d,%d,%d,%d", &b->fonte_tabela,
                   &b->colunas[0], &b->colunas[1], &b->colunas[2], &b->colunas[3], &b->colunas[4], &b->colunas[5]);
            if (b->fonte_tabela < 1 || b->fonte_tabela > N_FONTES)
                b->fonte_tabela = 1;
        }

        char sub[700];
        size_t usado = 0;
        int cortado = 0;
        while (fgets(sub, sizeof(sub), f) && strncmp(sub, "# BLOCO_FIM", 11) != 0)
        {
            size_t l = strlen(sub);
            if (b->tipo != BLOCO_TEXTO)
                continue;
            if (usado + l < sizeof(b->conteudo_texto))
            {
                memcpy(b->conteudo_texto + usado, sub, l);
                usado += l;
                b->conteudo_texto[usado] = '\0';
            }
            else
                cortado = 1;
        }
        if (b->tipo < BLOCO_TEXTO || b->tipo > BLOCO_QUEBRA)
        {
            free(b);
            continue;
        }
        b->conteudo_texto[strcspn(b->conteudo_texto, "\r\n")] = '\0';
        if (b->tipo == BLOCO_TEXTO && larg_visual(b->conteudo_texto) > TAM_TEXTO)
        {
            cortar_visual(b->conteudo_texto, TAM_TEXTO);
            cortado = 1;
        }
        if (cortado)
            aviso("[AVISO] Bloco %d tinha mais de %d caracteres e foi cortado.", d->nb + 1, TAM_TEXTO);

        if (!tail)
            d->head = b;
        else
            tail->prox = b;
        tail = b;
        d->nb++;
    }
    fclose(f);
    return 1;
}

static int nome_de_base(const char *nome)
{
    for (int i = 0; i < N_FONTES; i++)
    {
        const char *arq = FONTES[i].arquivo;
        size_t l = strlen(arq);
        if (l > 4 && strcmp(arq + l - 4, ".txt") == 0)
            l -= 4;
        if (strlen(nome) != l)
            continue;
        size_t k = 0;
        while (k < l && tolower((unsigned char)nome[k]) == tolower((unsigned char)arq[k]))
            k++;
        if (k == l)
            return 1;
    }
    return 0;
}

static int pedir_nome(const Doc *d, char novo[NOME_MAX])
{
    snprintf(novo, NOME_MAX, "%s", d->nome);
    for (;;)
    {
        char prompt[120], tmp[128] = "";
        if (d->nome[0])
            snprintf(prompt, sizeof(prompt), "\nNome do documento [%s]: ", d->nome);
        else
            snprintf(prompt, sizeof(prompt), "\nNome do documento (vazio cancela): ");
        ler_linha(prompt, tmp, sizeof(tmp));

        if (tmp[0] == '\0')
            return d->nome[0] != '\0';
        if (sanear_nome(tmp) == 0 || strcmp(tmp, RASCUNHO) == 0)
        {
            printf("[!] Nome inválido ou reservado. Use letras, números, _ ou -.\n");
            continue;
        }
        if (nome_de_base(tmp))
        {
            printf("[!] '%s' é o nome de uma base de dados (%s.txt). Escolha outro nome.\n", tmp, tmp);
            continue;
        }
        snprintf(novo, NOME_MAX, "%s", tmp);
        return 1;
    }
}

static void salvar_documento(Doc *d)
{
    if (!d->cab.configurado)
        configurar_cabecalho(&d->cab);

    limpar_tela();
    renderizar(stdout, d->head, 0, &d->cab);
    pausar("\nPressione ENTER para salvar...");

    char novo[NOME_MAX];
    if (!pedir_nome(d, novo))
    {
        aviso("[!] Salvamento cancelado.");
        return;
    }

    char txt[NOME_MAX + 8], meta[NOME_MAX + 8];
    caminho_de(novo, "txt", txt, sizeof(txt));
    caminho_de(novo, "meta", meta, sizeof(meta));

    if (strcmp(novo, d->nome) != 0 && (arquivo_existe(txt) || arquivo_existe(meta)))
    {
        printf("'%s' já existe. Sobrescrever? (1-Sim / 0-Não): ", novo);
        if (ler_int() != 1)
        {
            aviso("[!] Salvamento cancelado (nome já existia).");
            return;
        }
    }

    FILE *f = fopen(txt, "w");
    if (!f || (renderizar(f, d->head, 0, &d->cab), fclose(f)) != 0 || !salvar_meta(meta, d))
    {
        aviso("[ERRO] não foi possível gravar '%s' / '%s'.", txt, meta);
        return;
    }
    if (d->nome[0] == '\0')
    {
        char rasc[NOME_MAX + 8];
        caminho_de("", "meta", rasc, sizeof(rasc));
        remove(rasc);
    }
    snprintf(d->nome, sizeof(d->nome), "%s", novo);
    aviso("[SUCESSO] Documento salvo em '%s' e estrutura em '%s'.", txt, meta);
}

#define MAX_DOCS 64

static int listar_metas(char nomes[][NOME_MAX])
{
    DIR *dir = opendir(".");
    if (!dir)
        return 0;
    int n = 0;
    struct dirent *e;
    while (n < MAX_DOCS && (e = readdir(dir)) != NULL)
    {
        size_t l = strlen(e->d_name);
        if (l > 5 && l - 5 < NOME_MAX && strcmp(e->d_name + l - 5, ".meta") == 0)
        {
            memcpy(nomes[n], e->d_name, l - 5);
            nomes[n][l - 5] = '\0';
            n++;
        }
    }
    closedir(dir);
    return n;
}

static void documento_vazio(Doc *d)
{
    liberar_blocos(d->head);
    memset(d, 0, sizeof(*d));
    snprintf(d->cab.municipio, sizeof(d->cab.municipio), "MUNICÍPIO");
    snprintf(d->cab.data, sizeof(d->cab.data), "01/01/2026");
    snprintf(d->cab.tipo, sizeof(d->cab.tipo), "Ordinária");
    d->cab.sessao = 1;
}

static int novo_documento(Doc *d)
{
    char rasc[NOME_MAX + 8];
    caminho_de("", "meta", rasc, sizeof(rasc));
    if (arquivo_existe(rasc))
    {
        printf("Existe um rascunho anterior (%s) que será SUBSTITUÍDO pelo novo documento.\n", rasc);
        printf("(Para recuperá-lo, volte e use \"Abrir documento existente\".)\n");
        printf("Continuar? (1-Sim / 0-Não): ");
        if (ler_int() != 1)
            return 0;
    }
    documento_vazio(d);
    return 1;
}

typedef struct
{
    int tipo;
    int alinhamento;
    int fonte;
    int colunas[MAX_COLS];
    char texto[TAM_BUF_TEXTO];
    char chave[TAM_BUF_TEXTO];
} ItemTxt;

static void montar_chave_texto(char *dest, size_t tam, const char *texto)
{
    size_t j = 0;
    if (tam < 2)
        return;
    dest[j++] = 'T';
    for (const char *p = texto; *p && j < tam - 1; p++)
        if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
            dest[j++] = *p;
    dest[j] = '\0';
}

static void montar_chave_tabela(char *dest, size_t tam, int fonte, const int *colunas)
{
    snprintf(dest, tam, "B%d:%d%d%d%d%d%d", fonte, colunas[0] != 0, colunas[1] != 0, colunas[2] != 0,
             colunas[3] != 0, colunas[4] != 0, colunas[5] != 0);
}

static void chave_do_bloco(const BlocoDocumento *b, char *dest, size_t tam)
{
    if (b->tipo == BLOCO_TEXTO)
        montar_chave_texto(dest, tam, b->conteudo_texto);
    else if (b->tipo == BLOCO_TABELA)
        montar_chave_tabela(dest, tam, b->fonte_tabela, b->colunas);
    else
        snprintf(dest, tam, "Q");
}

static ItemTxt *novo_item(ItemTxt **v, int *n, int *cap, int tipo)
{
    if (*n == *cap)
    {
        int nova = *cap ? *cap * 2 : 16;
        ItemTxt *m = (ItemTxt *)realloc(*v, (size_t)nova * sizeof(ItemTxt));
        if (!m)
            return NULL;
        *v = m;
        *cap = nova;
    }
    ItemTxt *it = &(*v)[(*n)++];
    memset(it, 0, sizeof(*it));
    it->tipo = tipo;
    return it;
}

static void marcar_colunas_do_cabecalho(const char *linha, ItemTxt *it)
{
    const Fonte *F = &FONTES[it->fonte - 1];
    char copia[LINHA_BUF];
    snprintf(copia, sizeof(copia), "%s", linha);
    for (char *tok = strtok(copia, "|"); tok; tok = strtok(NULL, "|"))
    {
        while (*tok == ' ')
            tok++;
        size_t l = strlen(tok);
        while (l > 0 && (tok[l - 1] == ' ' || tok[l - 1] == '\r' || tok[l - 1] == '\n'))
            tok[--l] = '\0';
        if (l == 0)
            continue;
        int cortado = l > 3 && strcmp(tok + l - 3, "...") == 0;
        for (int c = 0; c < F->n_cols; c++)
            if (strcmp(tok, F->nomes[c]) == 0 || (cortado && strncmp(tok, F->nomes[c], l - 3) == 0))
            {
                it->colunas[c] = 1;
                break;
            }
    }
}

static void fechar_texto(ItemTxt **v, int *n, int *cap, char *acum, int alinha)
{
    if (acum[0] == '\0')
        return;
    ItemTxt *it = novo_item(v, n, cap, BLOCO_TEXTO);
    if (it)
    {
        snprintf(it->texto, sizeof(it->texto), "%s", acum);
        it->alinhamento = alinha;
        montar_chave_texto(it->chave, sizeof(it->chave), it->texto);
    }
    acum[0] = '\0';
}

static int ler_itens_txt(const char *caminho, ItemTxt **out)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return -1;
    ItemTxt *v = NULL;
    int n = 0, cap = 0, barras = 0, apos_quebra = 0, espera_cab = 0;
    char linha[LINHA_BUF * 2], acum[TAM_BUF_TEXTO] = "";
    int alinha = ALINHA_ESQUERDA;

    while (fgets(linha, sizeof(linha), f))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strncmp(linha, "======", 6) == 0)
        {
            if (++barras >= 3)
                break;
            continue;
        }
        if (barras < 2)
            continue;
        if (linha[0] == '\0')
        {
            if (!apos_quebra)
                fechar_texto(&v, &n, &cap, acum, alinha);
            continue;
        }
        if (linha[0] == '-' && strstr(linha, "fim da página"))
        {
            apos_quebra = 1;
            if (strstr(linha, "[manual]"))
            {
                fechar_texto(&v, &n, &cap, acum, alinha);
                ItemTxt *it = novo_item(&v, &n, &cap, BLOCO_QUEBRA);
                if (it)
                    snprintf(it->chave, sizeof(it->chave), "Q");
            }
            continue;
        }
        apos_quebra = 0;
        if (linha[0] == '+')
            continue;
        if (linha[0] == '|')
        {
            if (espera_cab && n > 0)
            {
                marcar_colunas_do_cabecalho(linha, &v[n - 1]);
                montar_chave_tabela(v[n - 1].chave, sizeof(v[n - 1].chave), v[n - 1].fonte, v[n - 1].colunas);
                espera_cab = 0;
            }
            continue;
        }
        if (strncmp(linha, "Tabela: ", 8) == 0)
        {
            fechar_texto(&v, &n, &cap, acum, alinha);
            char titulo[LINHA_BUF * 2];
            snprintf(titulo, sizeof(titulo), "%s", linha + 8);
            char *cont = strstr(titulo, " (continuação)");
            if (cont)
                *cont = '\0';
            int fonte = 0;
            for (int i = 0; i < N_FONTES; i++)
                if (strcmp(titulo, FONTES[i].titulo) == 0)
                    fonte = i + 1;
            if (fonte && !(cont && n > 0 && v[n - 1].tipo == BLOCO_TABELA && v[n - 1].fonte == fonte))
            {
                ItemTxt *it = novo_item(&v, &n, &cap, BLOCO_TABELA);
                if (it)
                {
                    it->fonte = fonte;
                    espera_cab = 1;
                }
            }
            continue;
        }

        int esp = 0;
        while (linha[esp] == ' ')
            esp++;
        if (acum[0] == '\0')
        {
            int len = larg_visual(linha + esp);
            alinha = esp == 0 ? ALINHA_ESQUERDA : (esp + len >= LARGURA_LINHA ? ALINHA_DIREITA : ALINHA_CENTRO);
        }
        size_t usado = strlen(acum);
        if (usado + strlen(linha + esp) + 2 < sizeof(acum))
            snprintf(acum + usado, sizeof(acum) - usado, "%s%s", usado ? " " : "", linha + esp);
    }
    fechar_texto(&v, &n, &cap, acum, alinha);
    fclose(f);
    *out = v;
    return n;
}

static int sincronizar_com_txt(Doc *d, const char *txt)
{
    ItemTxt *itens = NULL;
    int m = ler_itens_txt(txt, &itens);
    if (m < 0)
        return 0;

    int n = d->nb;
    BlocoDocumento **blocos = (BlocoDocumento **)malloc((size_t)(n + 1) * sizeof(*blocos));
    char(*chaves)[TAM_BUF_TEXTO] = (char(*)[TAM_BUF_TEXTO])malloc((size_t)(n + 1) * TAM_BUF_TEXTO);
    int *lcs = (int *)calloc((size_t)(n + 1) * (size_t)(m + 1), sizeof(int));
    if (!blocos || !chaves || !lcs)
    {
        free(blocos);
        free(chaves);
        free(lcs);
        free(itens);
        return 0;
    }
    int i = 0;
    for (BlocoDocumento *b = d->head; b; b = b->prox, i++)
    {
        blocos[i] = b;
        chave_do_bloco(b, chaves[i], TAM_BUF_TEXTO);
    }
#define LCS(x, y) lcs[(x) * (m + 1) + (y)]
    for (int x = n - 1; x >= 0; x--)
        for (int y = m - 1; y >= 0; y--)
            LCS(x, y) = strcmp(chaves[x], itens[y].chave) == 0
                            ? LCS(x + 1, y + 1) + 1
                            : (LCS(x + 1, y) > LCS(x, y + 1) ? LCS(x + 1, y) : LCS(x, y + 1));

    BlocoDocumento *novo_head = NULL, *tail = NULL;
    int removidos = 0, adicionados = 0, a = 0, b = 0;
    while (a < n || b < m)
    {
        BlocoDocumento *saida;
        if (a < n && b < m && strcmp(chaves[a], itens[b].chave) == 0)
        {
            saida = blocos[a++];
            b++;
        }
        else if (a < n && (b >= m || LCS(a + 1, b) >= LCS(a, b + 1)))
        {
            free(blocos[a++]);
            removidos++;
            continue;
        }
        else
        {
            ItemTxt *it = &itens[b++];
            saida = criar_bloco(it->tipo);
            if (!saida)
                continue;
            saida->alinhamento = it->alinhamento ? it->alinhamento : ALINHA_ESQUERDA;
            saida->fonte_tabela = it->fonte;
            memcpy(saida->colunas, it->colunas, sizeof(saida->colunas));
            snprintf(saida->conteudo_texto, sizeof(saida->conteudo_texto), "%s", it->texto);
            adicionados++;
        }
        saida->prox = NULL;
        if (tail)
            tail->prox = saida;
        else
            novo_head = saida;
        tail = saida;
    }
#undef LCS
    int total = 0, maior = 0;
    for (BlocoDocumento *x = novo_head; x; x = x->prox, total++)
        if (x->id > maior)
            maior = x->id;
    for (BlocoDocumento *x = novo_head; x; x = x->prox)
        if (x->id == 0)
            x->id = ++maior;
    d->head = novo_head;
    d->nb = total;
    free(blocos);
    free(chaves);
    free(lcs);
    free(itens);
    return removidos + adicionados;
}

static int abrir_documento(Doc *d)
{
    char nomes[MAX_DOCS][NOME_MAX];
    int n = listar_metas(nomes);
    if (n == 0)
    {
        aviso("[!] Nenhum documento salvo (.meta) encontrado nesta pasta.");
        return 0;
    }
    printf("=== DOCUMENTOS EXISTENTES ===\n");
    for (int i = 0; i < n; i++)
        printf("%d - %s%s\n", i + 1, nomes[i], strcmp(nomes[i], RASCUNHO) == 0 ? "  (rascunho automático)" : "");
    printf("0 - Voltar\nEscolha: ");
    int op = ler_int();
    if (op < 1 || op > n)
        return 0;

    char meta[NOME_MAX + 8];
    caminho_de(nomes[op - 1], "meta", meta, sizeof(meta));
    documento_vazio(d);
    if (!carregar_meta(meta, d))
    {
        aviso("[ERRO] não abriu '%s'.", meta);
        return 0;
    }
    char txt[NOME_MAX + 8];
    caminho_de(nomes[op - 1], "txt", txt, sizeof(txt));
    int ajustes = arquivo_existe(txt) ? sincronizar_com_txt(d, txt) : 0;
    if (ajustes)
        salvar_meta(meta, d);
    if (strcmp(nomes[op - 1], RASCUNHO) != 0)
        snprintf(d->nome, sizeof(d->nome), "%s", nomes[op - 1]);
    aviso("[OK] '%s' aberto (%d blocos).%s", meta, d->nb, ajustes ? " Ajustado ao .txt." : "");
    return 1;
}

static void editar_documento(Doc *d)
{
    for (;;)
    {
        limpar_tela();
        listar_blocos(d);
        printf("\n");
        mostrar_aviso();
        printf("\n--- MENU ---\n"
               "1 - Adicionar bloco de texto\n"
               "2 - Adicionar bloco de tabela\n"
               "3 - Inserir quebra de página (antes de um bloco)\n"
               "4 - Remover bloco\n"
               "5 - Visualizar documento (largura %d, %d linhas/página)\n"
               "6 - Salvar (.txt + .meta)\n"
               "7 - Configurar cabeçalho\n"
               "8 - Editar bloco\n"
               "0 - Sair (voltar ao início)\nOpção: ",
               LARGURA_LINHA, LINHAS_POR_PAGINA);

        int op = ler_int();
        int mudou = 0;
        switch (op)
        {
        case 0:
            if (g_eof)
                return;
            if (d->nome[0] == '\0' && d->nb > 0)
            {
                printf("\nDocumento ainda sem nome: fica guardado só como '%s.meta'.\nSair mesmo assim? (1-Sim / 0-Não): ", RASCUNHO);
                if (ler_int() != 1)
                    break;
            }
            return;
        case 1: limpar_tela(); mudou = novo_texto(d); break;
        case 2: limpar_tela(); mudou = nova_tabela(d); break;
        case 3: mudou = nova_quebra(d); break;
        case 4: mudou = remover(d); break;
        case 5:
            limpar_tela();
            renderizar(stdout, d->head, 1, &d->cab);
            pausar("\nPressione ENTER para voltar ao menu...");
            break;
        case 6: salvar_documento(d); break;
        case 7: configurar_cabecalho(&d->cab); mudou = 1; break;
        case 8: limpar_tela(); mudou = editar(d); break;
        default: aviso("Opção inválida!");
        }
        if (mudou)
            autosalvar(d);
    }
}

#define N_ARR(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define MAX_VER 200
#define MAX_REU 100
#define MAX_PROJ 200
#define ID_LIMITE 1000000
#define MAX_MATRIZ (50L * 1000 * 1000)

static const int ORDEM_DADOS[5] = {IDX_VER, IDX_REU, IDX_PROJ, IDX_PRES, IDX_VOT};

static long aleat(long n)
{
    return (((long)rand() << 15) ^ rand()) % n;
}

static unsigned char g_usado[ID_LIMITE];

static long id_unico(long lo, long hi)
{
    long v;
    do
        v = lo + aleat(hi - lo + 1);
    while (g_usado[v]);
    g_usado[v] = 1;
    return v;
}

static void data_aleatoria(char *dest, size_t tam)
{
    snprintf(dest, tam, "%02d/%02d/2026", (int)aleat(28) + 1, (int)aleat(12) + 1);
}

static int contar_registros(int idx)
{
    FILE *f = fopen(FONTES[idx].arquivo, "r");
    if (!f)
        return 0;
    int n = 0, inicio = 1;
    char buf[1024];
    while (fgets(buf, sizeof(buf), f))
    {
        if (inicio && buf[0] != '\n' && buf[0] != '\r' && buf[0] != '#')
            n++;
        inicio = (strchr(buf, '\n') != NULL);
    }
    fclose(f);
    return n;
}

static int pedir_qtd(const char *rotulo, int max)
{
    printf("%s (1-%d, 0 cancela): ", rotulo, max);
    int v = ler_int();
    return (v < 1 || v > max) ? 0 : v;
}

static int confirmar_substituir(int idx)
{
    int n = contar_registros(idx);
    if (n == 0)
        return 1;
    printf("'%s' já tem %d registros e será SUBSTITUÍDO. Continuar? (1-Sim / 0-Não): ", FONTES[idx].arquivo, n);
    return ler_int() == 1;
}

static int gerar_vereadores(int n)
{
    FILE *f = fopen(FONTES[IDX_VER].arquivo, "w");
    if (!f)
        return -1;
    memset(g_usado, 0, sizeof(g_usado));
    for (int i = 0; i < n; i++)
    {
        const char *p[3];
        for (int k = 0; k < 3; k++)
            p[k] = NOMES_BASE[aleat(TOTAL_NOMES)];
        fprintf(f, "%ld;%s %s %s;%s;%c%c%c%02d@%s\n", id_unico(1000, 9999), p[0], p[1], p[2],
                PARTIDOS[aleat(TOTAL_PARTIDOS)],
                tolower((unsigned char)p[0][0]), tolower((unsigned char)p[1][0]),
                tolower((unsigned char)p[2][0]), (int)aleat(100), DOMINIOS[aleat(TOTAL_DOMINIOS)]);
    }
    fclose(f);
    return n;
}

static int gerar_reunioes(int n)
{
    FILE *f = fopen(FONTES[IDX_REU].arquivo, "w");
    if (!f)
        return -1;
    memset(g_usado, 0, sizeof(g_usado));
    for (int i = 0; i < n; i++)
    {
        char data[16], pauta[100];
        data_aleatoria(data, sizeof(data));
        sortear_tema_complexo(pauta, sizeof(pauta), "Debate sobre");
        fprintf(f, "%ld;%s;%02d:00;%s\n", id_unico(1, 999), data, 8 + (int)aleat(11), pauta);
    }
    fclose(f);
    return n;
}

static int gerar_projetos(int n)
{
    Vereador *lv = carregar_vereadores(FONTES[IDX_VER].arquivo);
    int nv = 0;
    for (Vereador *v = lv; v; v = v->prox)
        nv++;
    if (nv == 0)
        return -2;
    FILE *f = fopen(FONTES[IDX_PROJ].arquivo, "w");
    if (!f)
    {
        liberar_vereadores(lv);
        return -1;
    }
    memset(g_usado, 0, sizeof(g_usado));
    for (int i = 0; i < n; i++)
    {
        Vereador *autor = lv;
        for (long k = aleat(nv); k > 0; k--)
            autor = autor->prox;
        char data[16], titulo[100];
        data_aleatoria(data, sizeof(data));
        sortear_tema_complexo(titulo, sizeof(titulo), "PL");
        fprintf(f, "%ld;%s;%d;%s;%s;%s\n", id_unico(100, 999), titulo, autor->id_vereador, data,
                CATEGORIAS[aleat(TOTAL_CATEGORIAS)], STATUS_PROJ[aleat(TOTAL_STATUS_PROJ)]);
    }
    fclose(f);
    liberar_vereadores(lv);
    return n;
}

static int gerar_presencas(void)
{
    Vereador *lv = carregar_vereadores(FONTES[IDX_VER].arquivo);
    Reuniao *lr = carregar_reunioes(FONTES[IDX_REU].arquivo);
    int nv = 0, nr = 0, n = 0, ret = 0;
    for (Vereador *v = lv; v; v = v->prox)
        nv++;
    for (Reuniao *r = lr; r; r = r->prox)
        nr++;
    static const char *ST[] = {"presente", "presente", "presente", "presente", "presente",
                               "presente", "presente", "ausente", "ausente", "justificado"};
    if (nv == 0 || nr == 0)
        ret = -2;
    else if ((long)nv * nr > ID_LIMITE / 4)
        ret = -3;
    else
    {
        FILE *f = fopen(FONTES[IDX_PRES].arquivo, "w");
        if (!f)
            ret = -1;
        else
        {
            memset(g_usado, 0, sizeof(g_usado));
            for (Reuniao *r = lr; r; r = r->prox)
                for (Vereador *v = lv; v; v = v->prox, n++)
                    fprintf(f, "%ld;%d;%d;%s\n", id_unico(10000, ID_LIMITE - 1), r->id_reuniao,
                            v->id_vereador, ST[aleat(N_ARR(ST))]);
            fclose(f);
            ret = n;
        }
    }
    liberar_vereadores(lv);
    liberar_reunioes(lr);
    return ret;
}

static int gerar_votacoes_completas_otimizada(const char *caminho_saida, Reuniao *lista_reunioes,
                                              Projeto *lista_projetos, Vereador *lista_vereadores,
                                              Presenca *lista_presencas)
{
    int nr = 0, np = 0, nv = 0, max_reuniao = 0, max_vereador = 0;
    for (Reuniao *r = lista_reunioes; r; r = r->prox)
        nr++;
    for (Projeto *p = lista_projetos; p; p = p->prox)
        np++;
    for (Vereador *v = lista_vereadores; v; v = v->prox)
        nv++;
    if (nr == 0 || np == 0 || nv == 0 || !lista_presencas)
        return -2;
    if ((long)np * nv > ID_LIMITE / 4)
        return -3;

    for (Presenca *a = lista_presencas; a; a = a->prox)
    {
        if (a->id_reuniao > max_reuniao)
            max_reuniao = a->id_reuniao;
        if (a->id_vereador > max_vereador)
            max_vereador = a->id_vereador;
    }
    long colunas = (long)max_vereador + 1;
    if ((long)(max_reuniao + 1) * colunas > MAX_MATRIZ)
        return -3;
    unsigned char *presentes = (unsigned char *)calloc((size_t)(max_reuniao + 1) * colunas, 1);
    if (!presentes)
        return -1;
    for (Presenca *a = lista_presencas; a; a = a->prox)
        if (strcmp(a->status_presenca, "presente") == 0)
            presentes[a->id_reuniao * colunas + a->id_vereador] = 1;

    FILE *f = fopen(caminho_saida, "w");
    if (!f)
    {
        free(presentes);
        return -1;
    }
    memset(g_usado, 0, sizeof(g_usado));
    int n = 0;
    for (Projeto *p = lista_projetos; p; p = p->prox)
    {
        Reuniao *r = lista_reunioes;
        for (long k = aleat(nr); k > 0; k--)
            r = r->prox;
        for (Vereador *v = lista_vereadores; v; v = v->prox)
            if (r->id_reuniao <= max_reuniao && v->id_vereador <= max_vereador &&
                presentes[r->id_reuniao * colunas + v->id_vereador])
            {
                fprintf(f, "%ld;%d;%d;%d;%s\n", id_unico(10000, ID_LIMITE - 1), p->id_proj,
                        r->id_reuniao, v->id_vereador, VOTOS[aleat(TOTAL_VOTOS)]);
                n++;
            }
    }
    fclose(f);
    free(presentes);
    return n;
}

static int gerar_votacoes(void)
{
    Reuniao *lr = carregar_reunioes(FONTES[IDX_REU].arquivo);
    Projeto *lp = carregar_projetos(FONTES[IDX_PROJ].arquivo);
    Vereador *lv = carregar_vereadores(FONTES[IDX_VER].arquivo);
    Presenca *pres = carregar_presencas(FONTES[IDX_PRES].arquivo);
    int r = gerar_votacoes_completas_otimizada(FONTES[IDX_VOT].arquivo, lr, lp, lv, pres);
    liberar_reunioes(lr);
    liberar_projetos(lp);
    liberar_vereadores(lv);
    liberar_presencas(pres);
    return r;
}

static void relatar_geracao(const char *titulo, int r)
{
    if (r >= 0)
        aviso("[SUCESSO] %d registros gerados em %s.", r, titulo);
    else if (r == -2)
        aviso("[!] Faltam dados-base para gerar %s (gere vereadores, reuniões e projetos antes).", titulo);
    else if (r == -3)
        aviso("[!] Combinação grande demais para %s.", titulo);
    else
        aviso("[ERRO] não foi possível gravar o arquivo de %s.", titulo);
}

static void menu_gerar_dados(void)
{
    for (;;)
    {
        limpar_tela();
        printf("=== GERAR DADOS ALEATÓRIOS ===\n");
        mostrar_aviso();
        for (int i = 0; i < 5; i++)
            printf("%d - %s (%s: %d registros hoje)\n", i + 1, FONTES[ORDEM_DADOS[i]].titulo,
                   FONTES[ORDEM_DADOS[i]].arquivo, contar_registros(ORDEM_DADOS[i]));
        printf("6 - TUDO (você informa vereadores, reuniões e projetos; presenças e votações são derivadas)\n"
               "0 - Voltar\n"
               "(gerar substitui o conteúdo do arquivo escolhido)\nOpção: ");
        int op = ler_int();
        if (op == 0 || g_eof)
            return;
        if (op < 1 || op > 6)
        {
            aviso("Opção inválida!");
            continue;
        }
        if (op == 6)
        {
            int nv = pedir_qtd("Quantos vereadores", MAX_VER);
            int nr = nv ? pedir_qtd("Quantas reuniões", MAX_REU) : 0;
            int np = nr ? pedir_qtd("Quantos projetos", MAX_PROJ) : 0;
            if (!np)
            {
                aviso("Geração cancelada.");
                continue;
            }
            printf("Isto SUBSTITUI os 5 arquivos de dados. Continuar? (1-Sim / 0-Não): ");
            if (ler_int() != 1)
            {
                aviso("Geração cancelada.");
                continue;
            }
            int a = gerar_vereadores(nv), b = gerar_reunioes(nr), c = gerar_projetos(np);
            int d = gerar_presencas(), e = gerar_votacoes();
            if (a < 0 || b < 0 || c < 0 || d < 0 || e < 0)
                aviso("[ERRO] falha ao gerar a base (arquivo sem permissão?).");
            else
                aviso("[SUCESSO] %d vereadores, %d reuniões, %d projetos, %d presenças, %d votações.",
                      a, b, c, d, e);
            continue;
        }
        int idx = ORDEM_DADOS[op - 1];
        int qtd = 0;
        if (idx == IDX_VER)
            qtd = pedir_qtd("Quantos vereadores", MAX_VER);
        else if (idx == IDX_REU)
            qtd = pedir_qtd("Quantas reuniões", MAX_REU);
        else if (idx == IDX_PROJ)
            qtd = pedir_qtd("Quantos projetos", MAX_PROJ);
        else
            qtd = 1;
        if (!qtd || !confirmar_substituir(idx))
        {
            aviso("Geração cancelada.");
            continue;
        }
        int r = (idx == IDX_VER)    ? gerar_vereadores(qtd)
                : (idx == IDX_REU)  ? gerar_reunioes(qtd)
                : (idx == IDX_PROJ) ? gerar_projetos(qtd)
                : (idx == IDX_PRES) ? gerar_presencas()
                                    : gerar_votacoes();
        relatar_geracao(FONTES[idx].titulo, r);
    }
}

static void menu_apagar_dados(void)
{
    for (;;)
    {
        limpar_tela();
        printf("=== APAGAR DADOS DA BASE ===\n");
        mostrar_aviso();
        for (int i = 0; i < 5; i++)
            printf("%d - %s (%s: %d registros)\n", i + 1, FONTES[ORDEM_DADOS[i]].titulo,
                   FONTES[ORDEM_DADOS[i]].arquivo, contar_registros(ORDEM_DADOS[i]));
        printf("6 - TUDO\n0 - Voltar\nO que apagar? ");
        int op = ler_int();
        if (op == 0 || g_eof)
            return;
        if (op < 1 || op > 6)
        {
            aviso("Opção inválida!");
            continue;
        }
        int de = (op == 6) ? 0 : op - 1, ate = (op == 6) ? 4 : op - 1;
        int total = 0;
        for (int i = de; i <= ate; i++)
            total += contar_registros(ORDEM_DADOS[i]);
        if (total == 0)
        {
            aviso("Nada a apagar: já está vazio.");
            continue;
        }
        printf("Apagar %d registros de %s? Isso não pode ser desfeito. (1-Sim / 0-Não): ", total,
               op == 6 ? "TODA a base" : FONTES[ORDEM_DADOS[op - 1]].titulo);
        if (ler_int() != 1)
        {
            aviso("Nada foi apagado.");
            continue;
        }
        int falhas = 0;
        for (int i = de; i <= ate; i++)
        {
            FILE *f = fopen(FONTES[ORDEM_DADOS[i]].arquivo, "w");
            if (f)
                fclose(f);
            else
                falhas++;
        }
        if (falhas)
            aviso("[ERRO] %d arquivo(s) não puderam ser esvaziados.", falhas);
        else
            aviso("[SUCESSO] %d registros apagados.", total);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    static Doc doc;

    for (;;)
    {
        limpar_tela();
        printf("====================================================\n"
               "     SISTEMA DE EDIÇÃO DE RELATÓRIO POR BLOCOS\n"
               "====================================================\n");
        mostrar_aviso();
        printf("1 - Novo documento\n"
               "2 - Abrir documento existente\n"
               "3 - Gerar dados aleatórios\n"
               "4 - Apagar dados da base\n"
               "0 - Encerrar o programa\nOpção: ");

        int op = ler_int();
        if (op == 0 || g_eof)
            break;
        if (op == 1)
        {
            limpar_tela();
            if (novo_documento(&doc))
                editar_documento(&doc);
        }
        else if (op == 2)
        {
            limpar_tela();
            if (abrir_documento(&doc))
                editar_documento(&doc);
        }
        else if (op == 3)
            menu_gerar_dados();
        else if (op == 4)
            menu_apagar_dados();
        else
            aviso("Opção inválida!");
    }
    liberar_blocos(doc.head);
    return 0;
}
