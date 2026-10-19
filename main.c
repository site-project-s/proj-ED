#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

void formatar_colunas_ativas(int fonte_tabela, const int colunas[6], char *destino, size_t tam_max);
void formatar_cabecalho(char *destino, size_t tamanho, const char *municipio, const char *data, int sessao, const char *tipo_sessao);
void formatar_rodape(char *destino, size_t tamanho);
int imprimir_formulario_txt(const char *caminho, const char *cabecalho, const char *conteudo, const char *rodape);
int editar_formulario_txt(const char *caminho);

/* structs*/

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
    char conteudo_texto[1024];
    int fonte_tabela;
    int colunas[6];
    struct BlocoDocumento *prox;
} BlocoDocumento;

/* listas */

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

// ============================================================================
//   FUNÇÕES BASICAS
// ============================================================================

int ler_inteiro_seguro() 
{
    char linha [128];
    char *fim;
    long valor;

    while (1)
    {
        if (!fgets(linha, sizeof (linha), stdin)) return -1; 
        linha[strcspn(linha, "\n")] = 0;
        
        errno = 0; 
        valor = strtol(linha, &fim, 10);

        if (fim == linha || *fim != '\0' || errno == ERANGE)
        {
            printf("[!] Entrada inválida. Digite apenas números válidos: ");
        }
        else 
        {
            return (int)valor; 
        }
    }
}


void formatar_texto_alinhado(char *destino, size_t tamanho_destino, const char *texto, int alinhamento)
{
    size_t tam_texto = strlen(texto);
    int largura_maxima = 70;
    
    if (tam_texto >= largura_maxima || alinhamento == ALINHA_ESQUERDA)
    {
        snprintf ( destino, tamanho_destino, "%s\n\n", texto); 
        return; 
    }

    int espacos = 0;
    if(alinhamento == ALINHA_DIREITA)
    {
        espacos = largura_maxima - tam_texto;
    }
    else if ( alinhamento == ALINHA_CENTRO)
    {
        espacos = ( largura_maxima - tam_texto) / 2; 
    }

    snprintf(destino, tamanho_destino, "%*s%s\n\n", espacos, "", texto);

}


void limpar_tela()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar_tela()
{
    printf("\nPressione ENTER para continuar...");
    getchar();
}

void copiar_string(char *pra_ca, const char *daqui, size_t tamanho_max)
{
    if (!pra_ca || !daqui || tamanho_max == 0)
        return;
    strncpy(pra_ca, daqui, tamanho_max - 1);
    pra_ca[tamanho_max - 1] = '\0';
}

void escrever(const char *caminho, const char *texto)
{
    if (!caminho || !texto)
        return;
    FILE *f = fopen(caminho, "a");
    if (!f)
        return;
    fputs(texto, f);
    fclose(f);
}

int obter_maior_id(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return 1000;

    int maior = 0;
    char linha[256];
    while (fgets(linha, sizeof(linha), f))
    {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r')
            continue;
        int id = atoi(linha);
        if (id > maior)
            maior = id;
    }
    fclose(f);
    return (maior > 0) ? maior : 1000;
}

int carregar_todos_ids(const char *caminho, int ids[], int max_ids)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return 0;

    int qtd = 0;
    char linha[256];
    while (fgets(linha, sizeof(linha), f) && qtd < max_ids)
    {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r')
            continue;
        int id = atoi(linha);
        if (id > 0)
            ids[qtd++] = id;
    }
    fclose(f);
    return qtd;
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

/* carregamento de dados*/

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

/* liberação de memória*/

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

/* geradores */

int estava_presente_memoria(Presenca *head_presenca, int id_reuniao, int id_vereador)
{
    Presenca *atual = head_presenca;
    while (atual)
    {
        if (atual->id_reuniao == id_reuniao && atual->id_vereador == id_vereador)
        {
            return (strcmp(atual->status_presenca, "presente") == 0);
        }
        atual = atual->prox;
    }
    return 0;
}

void gerar_votacoes_completas_otimizada(
    const char *caminho_saida,
    Reuniao *lista_reunioes,
    Projeto *lista_projetos,
    Vereador *lista_vereadores,
    Presenca *lista_presencas)
{
    FILE *f = fopen(caminho_saida, "w");
    if (!f)
    {
        printf("[ERRO] Nao foi possivel criar o arquivo '%s'.\n", caminho_saida);
        return;
    }

    // 1 Descobre os limites reais dos IDs (elimina o limite rigido de 100)
    int max_reuniao = 0;
    int max_vereador = 0;
    Presenca *aux = lista_presencas;

    while (aux)
    {
        if (aux->id_reuniao > max_reuniao)
            max_reuniao = aux->id_reuniao;
        if (aux->id_vereador > max_vereador)
            max_vereador = aux->id_vereador;
        aux = aux->prox;
    }

    // 2 Aloca matriz plana 1D mapeada dinamicamente (Proteção contra Memory Leak)
    int colunas = max_vereador + 1;
    int *presentes = (int *)calloc((max_reuniao + 1) * colunas, sizeof(int));
    if (!presentes)
    {
        printf("[ERRO] Falha de alocacao de memoria para tabela de presencas.\n");
        fclose(f);
        return;
    }

    // 3 Preenche a matriz dinâmica contígua usando (linha * colunas + coluna)
    aux = lista_presencas;
    while (aux)
    {
        if (strcmp(aux->status_presenca, "presente") == 0)
        {
            presentes[aux->id_reuniao * colunas + aux->id_vereador] = 1;
        }
        aux = aux->prox;
    }

    // 4 Processamento sequencial e gravacao no arquivo
    int id_votacao = 1;
    Reuniao *r = lista_reunioes;

    while (r)
    {
        Projeto *p = lista_projetos;
        while (p)
        {
            Vereador *v = lista_vereadores;
            while (v)
            {
                // Checa a presenca com a nova matriz dinâmica
                if (r->id_reuniao <= max_reuniao && 
                    v->id_vereador <= max_vereador &&
                    presentes[r->id_reuniao * colunas + v->id_vereador])
                {
                    fprintf(f, "%d;%d;%d;%d;SIM\n",
                            id_votacao++,
                            p->id_proj, // Usando id_proj como ajustamos anteriormente
                            r->id_reuniao,
                            v->id_vereador);
                }
                v = v->prox;
            }
            p = p->prox;
        }
        r = r->prox;
    }

    // 5. Fechamento do arquivo e liberacao OBRIGATÓRIA da memoria
    fclose(f);
    free(presentes);
    printf("[OK] Processamento de votacoes finalizado e memoria liberada com sucesso!\n");
}


/* blocos do documento*/

int obter_proximo_id_bloco(BlocoDocumento *head)
{
    int max_id = 0;
    while (head)
    {
        if (head->id > max_id)
            max_id = head->id;
        head = head->prox;
    }
    return max_id + 1;
}

const char *obter_nome_fonte(int fonte)
{
    switch (fonte)
    {
    case 1:
        return "OsCapaPreta.txt";
    case 2:
        return "reunioes.txt";
    case 3:
        return "votacao.txt";
    case 4:
        return "projetos.txt";
    case 5:
        return "presenca.txt";
    default:
        return "OsCapaPreta.txt";
    }
}

const char *obter_titulo_fonte(int fonte)
{
    switch (fonte)
    {
    case 1:
        return "TABELA DE VEREADORES";
    case 2:
        return "TABELA DE REUNIÕES";
    case 3:
        return "TABELA DE VOTAÇÕES";
    case 4:
        return "TABELA DE PROJETOS DE LEI";
    case 5:
        return "TABELA DE PRESENÇAS";
    default:
        return "TABELA";
    }
}

void renderizar_conteudo_do_relatorio(BlocoDocumento *head, char *destino, size_t tamanho_max)
{
    destino[0] = '\0';
    size_t usado = 0;
    int pagina_atual = 1;

    usado += snprintf(destino + usado, tamanho_max - usado, "--- PÁGINA %d ---\n\n", pagina_atual);

    BlocoDocumento *atual = head;
    while (atual)
    {
        if (atual->tipo == BLOCO_TEXTO)
        {
            char texto_formatado[1100]; 

            formatar_texto_alinhado(texto_formatado, sizeof(texto_formatado), atual->conteudo_texto, atual -> alinhamento);

            usado += snprintf(destino + usado, tamanho_max - usado, "%s", texto_formatado);
        }
        else if (atual->tipo == 2)
        {
            usado += snprintf(destino + usado, tamanho_max - usado, "--- %s ---\n", obter_titulo_fonte(atual->fonte_tabela));

            const char *arq_fonte = obter_nome_fonte(atual->fonte_tabela);
            FILE *f_in = fopen(arq_fonte, "r");
            if (f_in)
            {
                char linha[256];
                while (fgets(linha, sizeof(linha), f_in))
                {
                    if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r')
                        continue;

                    linha[strcspn(linha, "\r\n")] = '\0';

                    char *cols[6] = {NULL};
                    cols[0] = strtok(linha, ";");
                    for (int i = 1; i < 6; i++)
                        cols[i] = strtok(NULL, ";");

                    int pos = 0;
                    char linha_montada[512] = "";

                    for (int i = 0; i < 6; i++)
                    {
                        if (atual->colunas[i] && cols[i])
                        {
                            pos += snprintf(linha_montada + pos, sizeof(linha_montada) - pos,
                                            "%s%s", (pos == 0 ? "" : " | "), cols[i]);
                        }
                    }
                    if (pos > 0)
                    {
                        usado += snprintf(destino + usado, tamanho_max - usado, "%s\n", linha_montada);
                    }
                }
                fclose(f_in);
            }
            usado += snprintf(destino + usado, tamanho_max - usado, "\n");
        }
        else if (atual->tipo == 3)
        {
            pagina_atual++;
            usado += snprintf(destino + usado, tamanho_max - usado, "\n--- PÁGINA %d ---\n\n", pagina_atual);
        }
        if (usado >= tamanho_max - 1)
            break;
        atual = atual->prox;
    }
}

BlocoDocumento *carregar_relatorio(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f)
        return NULL;

    BlocoDocumento *head = NULL, *tail = NULL;
    char linha[512];

    while (fgets(linha, sizeof(linha), f))
    {
        if (strncmp(linha, "# BLOCO_INICIO", 14) == 0)
        {
            BlocoDocumento *novo = (BlocoDocumento *)malloc(sizeof(BlocoDocumento));
            if (!novo)
                continue;
            memset(novo, 0, sizeof(BlocoDocumento));

            sscanf(linha, "# BLOCO_INICIO|ID:%d|TIPO:%d", &novo->id, &novo->tipo);

            if (novo->tipo == 2)
            {
                sscanf(linha, "# BLOCO_INICIO|ID:%*d|TIPO:%*d|FONTE:%d|COLS:%d,%d,%d,%d,%d,%d",
                       &novo->fonte_tabela,
                       &novo->colunas[0], &novo->colunas[1], &novo->colunas[2],
                       &novo->colunas[3], &novo->colunas[4], &novo->colunas[5]);
            }

            char sub_linha[512];
            size_t usado = 0; // Nosso "odômetro" de memória

            while (fgets(sub_linha, sizeof(sub_linha), f))
            {
                if (strncmp(sub_linha, "# BLOCO_FIM", 11) == 0)
                    break;
                
                if (novo->tipo == BLOCO_TEXTO) 
                {
                    size_t len_novo = strlen(sub_linha);
                    
                    
                    if (usado + len_novo < sizeof(novo->conteudo_texto) - 1) 
                    {
                        memcpy(novo->conteudo_texto + usado, sub_linha, len_novo);
                        usado += len_novo;
                        novo->conteudo_texto[usado] = '\0'; 
                    } 
                    else 
                    {
                        printf("[AVISO] Texto do Bloco ID %d era muito grande e foi truncado para evitar falha de memória.\n", novo->id);
                    }
                }
            }


            size_t len = strlen(novo->conteudo_texto);
            if (len > 0 && novo->conteudo_texto[len - 1] == '\n')
            {
                novo->conteudo_texto[len - 1] = '\0';
            }

            if (!head)
                head = novo;
            else
                tail->prox = novo;
            tail = novo;
        }
    }
    fclose(f);
    return head;
}

void exibir_blocos_menu(BlocoDocumento *head)
{
    printf("\n====================================================\n");
    printf("         ESTRUTURA ATUAL DO DOCUMENTO               \n");
    printf("====================================================\n");

    if (!head)
    {
        printf(" [Nenhum bloco cadastrado]\n");
        return;
    }

    BlocoDocumento *atual = head;
    int pos = 1;
    while (atual)
    {
        if (atual->tipo == 1)
        {
            char preview[40] = "";
            strncpy(preview, atual->conteudo_texto, 35);
            preview[35] = '\0';
            printf(" %d. [TEXTO (ID %d)] \"%s...\"\n", pos, atual->id, preview);
        }
        else if (atual->tipo == 2)
        {
            // Formata os nomes das colunas selecionadas
            char str_colunas[256];
            formatar_colunas_ativas(atual->fonte_tabela, atual->colunas, str_colunas, sizeof(str_colunas));

            printf(" %d. [TABELA (ID %d)] Fonte: %s | Colunas: [%s]\n",
                   pos, atual->id, obter_titulo_fonte(atual->fonte_tabela), str_colunas);
        }
        else if (atual->tipo == 3)
        {
            printf(" %d. [QUEBRA DE PÁGINA (ID %d)]\n", pos, atual->id);
        }
        atual = atual->prox;
        pos++;
    }
}

void adicionar_novo_bloco_texto(BlocoDocumento **head)
{
    BlocoDocumento *novo = (BlocoDocumento *)malloc(sizeof(BlocoDocumento));
    if (!novo)
        return;
    memset(novo, 0, sizeof(BlocoDocumento));

    novo->id = obter_proximo_id_bloco(*head);
    novo->tipo = 1;

    printf("\nDigite o texto livre a ser inserido no bloco: ");
    if (fgets(novo->conteudo_texto, sizeof(novo->conteudo_texto), stdin))
    {
        novo->conteudo_texto[strcspn(novo->conteudo_texto, "\r\n")] = '\0';
    }

    printf("\nComo deseja alinhar este texto?\n");
    printf("1 - Esquerda\n2 - Centro\n3 - Direita\nOpcao: ");
    novo->alinhamento = ler_inteiro_seguro();
    
    if (novo->alinhamento < 1 || novo->alinhamento > 3) novo -> alinhamento = 1;

    if (!*head)
    {
        *head = novo;
    }
    else
    {
        BlocoDocumento *t = *head;
        while (t->prox)
            t = t->prox;
        t->prox = novo;
    }
    printf("[OK] Bloco de Texto (ID %d) adicionado!\n", novo->id);
}

void adicionar_novo_bloco_tabela(BlocoDocumento **head)
{
    BlocoDocumento *novo = (BlocoDocumento *)malloc(sizeof(BlocoDocumento));

    if (!novo)
        return;
    memset(novo, 0, sizeof(BlocoDocumento));

    novo->id = obter_proximo_id_bloco(*head);
    novo->tipo = 2;

    printf("\n=== SELEÇÃO DE PLANILHA PARA O BLOCO ===\n");
    printf("1 - Vereadores (OsCapaPreta.txt)\n");
    printf("2 - Reuniões (reunioes.txt)\n");
    printf("3 - Votações (votacao.txt)\n");
    printf("4 - Projetos de Lei (projetos.txt)\n");
    printf("5 - Presenças (presenca.txt)\n");
    printf("Escolha a fonte: ");

    int opcao = 1;
    if (scanf("%d", &opcao) != 1)
        opcao = 1;
    getchar();

    novo->fonte_tabela = opcao;

    // Garante que todas as posições começam em 0
    for (int i = 0; i < 6; i++)
        novo->colunas[i] = 0;

    printf("\n--- Escolha as colunas a incluir (1-Sim / 0-Não) ---\n");

    if (opcao == 1)
    {
        printf("ID_VEREADOR? ");
        scanf("%d", &novo->colunas[0]);
        printf("NOME? ");
        scanf("%d", &novo->colunas[1]);
        printf("PARTIDO? ");
        scanf("%d", &novo->colunas[2]);
        printf("EMAIL? ");
        scanf("%d", &novo->colunas[3]);
    }
    else if (opcao == 2)
    {
        printf("ID_REUNIAO? ");
        scanf("%d", &novo->colunas[0]);
        printf("DATA_REUNIAO? ");
        scanf("%d", &novo->colunas[1]);
        printf("HORA? ");
        scanf("%d", &novo->colunas[2]);
        printf("PAUTA? ");
        scanf("%d", &novo->colunas[3]);
    }
    else if (opcao == 3)
    {
        printf("ID_VOTACAO? ");
        scanf("%d", &novo->colunas[0]);
        printf("ID_PROJETO? ");
        scanf("%d", &novo->colunas[1]);
        printf("ID_REUNIAO? ");
        scanf("%d", &novo->colunas[2]);
        printf("ID_VEREADOR? ");
        scanf("%d", &novo->colunas[3]);
        printf("VOTO? ");
        scanf("%d", &novo->colunas[4]);
    }
    else if (opcao == 4)
    {
        printf("ID_PROJ? ");
        scanf("%d", &novo->colunas[0]);
        printf("TITULO? ");
        scanf("%d", &novo->colunas[1]);
        printf("ID_AUTOR? ");
        scanf("%d", &novo->colunas[2]);
        printf("DATA_MOSTRA? ");
        scanf("%d", &novo->colunas[3]);
        printf("CATEGORIA? ");
        scanf("%d", &novo->colunas[4]);
        printf("STATUS? ");
        scanf("%d", &novo->colunas[5]);
    }
    else
    {
        printf("ID_PRESENCA? ");
        scanf("%d", &novo->colunas[0]);
        printf("ID_REUNIAO? ");
        scanf("%d", &novo->colunas[1]);
        printf("ID_VEREADOR? ");
        scanf("%d", &novo->colunas[2]);
        printf("STATUS_PRESENCA? ");
        scanf("%d", &novo->colunas[3]);
    }
    getchar();

    if (!*head)
        *head = novo;
    else
    {
        BlocoDocumento *t = *head;
        while (t->prox)
            t = t->prox;
        t->prox = novo;
    }
    printf("\n[OK] Bloco de Tabela (ID %d) adicionado!\n", novo->id);
}

void adicionar_novo_bloco_quebra(BlocoDocumento **head)
{
    BlocoDocumento *novo = (BlocoDocumento *)malloc(sizeof(BlocoDocumento));
    if (!novo)
        return;
    memset(novo, 0, sizeof(BlocoDocumento));

    novo->id = obter_proximo_id_bloco(*head);
    novo->tipo = 3;

    if (!*head)
        *head = novo;
    else
    {
        BlocoDocumento *t = *head;
        while (t->prox)
            t = t->prox;
        t->prox = novo;
    }
    printf("[OK] Bloco de Quebra de Página (ID %d) adicionado!\n", novo->id);
}

void salvar_estrutura_relatorio(const char *caminho_meta, BlocoDocumento *head)
{
    FILE *f = fopen(caminho_meta, "w");
    if (!f)
        return;

    BlocoDocumento *atual = head;
    while (atual)
    {
        if (atual->tipo == 1)
        { // Texto
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:1\n%s\n# BLOCO_FIM\n\n",
                    atual->id, atual->conteudo_texto);
        }
        else if (atual->tipo == 2)
        { // Tabela
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:2|FONTE:%d|COLS:%d,%d,%d,%d,%d,%d\n# BLOCO_FIM\n\n",
                    atual->id, atual->fonte_tabela,
                    atual->colunas[0], atual->colunas[1], atual->colunas[2],
                    atual->colunas[3], atual->colunas[4], atual->colunas[5]);
        }
        else if (atual->tipo == 3)
        { // Quebra de Página
            fprintf(f, "# BLOCO_INICIO|ID:%d|TIPO:3\n# BLOCO_FIM\n\n", atual->id);
        }
        atual = atual->prox;
    }
    fclose(f);
}

int obter_nomes_colunas_tabela(int fonte_tabela, const char *nomes_out[])
{
    switch (fonte_tabela)
    {
    case 1:
    { // Vereadores
        static const char *cols[] = {"ID_VEREADOR", "NOME", "PARTIDO", "EMAIL"};
        for (int i = 0; i < 4; i++)
            nomes_out[i] = cols[i];
        return 4;
    }
    case 2:
    { // Reuniões
        static const char *cols[] = {"ID_REUNIAO", "DATA_REUNIAO", "HORA", "PAUTA"};
        for (int i = 0; i < 4; i++)
            nomes_out[i] = cols[i];
        return 4;
    }
    case 3:
    { // Votações
        static const char *cols[] = {"ID_VOTACAO", "ID_PROJETO", "ID_REUNIAO", "ID_VEREADOR", "VOTO"};
        for (int i = 0; i < 5; i++)
            nomes_out[i] = cols[i];
        return 5;
    }
    case 4:
    { // Projetos de Lei
        static const char *cols[] = {"ID_PROJ", "TITULO", "ID_AUTOR", "DATA_MOSTRA", "CATEGORIA", "STATUS"};
        for (int i = 0; i < 6; i++)
            nomes_out[i] = cols[i];
        return 6;
    }
    case 5:
    { // Presenças
        static const char *cols[] = {"ID_PRESENCA", "ID_REUNIAO", "ID_VEREADOR", "STATUS_PRESENCA"};
        for (int i = 0; i < 4; i++)
            nomes_out[i] = cols[i];
        return 4;
    }
    default:
        return 0;
    }
}

void formatar_colunas_ativas(int fonte_tabela, const int colunas[6], char *destino, size_t tam_max)
{
    const char *nomes[6];
    int total = obter_nomes_colunas_tabela(fonte_tabela, nomes);

    destino[0] = '\0';
    size_t pos = 0;

    for (int i = 0; i < total; i++)
    {
        if (colunas[i])
        {
            pos += snprintf(destino + pos, tam_max - pos, "%s%s",
                            (pos == 0 ? "" : ", "), nomes[i]);
        }
    }
    if (pos == 0)
    {
        snprintf(destino, tam_max, "Nenhuma coluna");
    }
}

/* cabeçalho */

void formatar_cabecalho(
    char *destino,
    size_t tamanho,
    const char *municipio,
    const char *data,
    int sessao,
    const char *tipo_sessao)
{
    snprintf(destino, tamanho,
             "======================================================================\n"
             " CÂMARA MUNICIPAL DE %s\n"
             " RELATÓRIO DO FORMULÁRIO LEGISLATIVO\n"
             " Sessão Nº: %02d (%s) | Data: %s\n"
             "======================================================================\n\n",
             municipio, sessao, tipo_sessao, data);
}
void coletar_dados_cabecalho(
    char *municipio, size_t tam_mun,
    char *data, size_t tam_data,
    char *tipo_sessao, size_t tam_tipo,
    int *sessao)
{
    limpar_tela();
    printf("=== CONFIGURAÇÃO DO CABEÇALHO DO RELATÓRIO ===\n");

    printf("Município da Câmara: ");
    fgets(municipio, tam_mun, stdin);
    municipio[strcspn(municipio, "\r\n")] = '\0';

    printf("Data da Sessão (DD/MM/AAAA): ");
    fgets(data, tam_data, stdin);
    data[strcspn(data, "\r\n")] = '\0';

    printf("Tipo da Sessão (Ordinária / Extraordinária): ");
    fgets(tipo_sessao, tam_tipo, stdin);
    tipo_sessao[strcspn(tipo_sessao, "\r\n")] = '\0';

    printf("Número da Sessão: ");
    if (scanf("%d", sessao) != 1)
        *sessao = 1;
    getchar();
}

/* rodape */

void formatar_rodape(char *destino, size_t tamanho)
{
    snprintf(destino, tamanho,
             "\n==================================================\n"
             " Documento Gerado pelo Sistema Legislativo em C   \n"
             " Assinatura do Responsável: _______________________\n"
             " Data de Emissão: 2026-09-26                      \n"
             "==================================================\n");
}

/* impressao do formulario */

int imprimir_formulario_txt(const char *arquivo, const char *cabecalho, const char *conteudo, const char *rodape)
{
    FILE *f = fopen(arquivo, "w"); // Limpa o arquivo para regravar do zero
    if (!f)
    {
        printf("[ERRO] Não foi possível criar/abrir o arquivo '%s'.\n", arquivo);
        return 0;
    }
    fclose(f);

    if (cabecalho)
        escrever(arquivo, cabecalho);
    if (conteudo)
        escrever(arquivo, conteudo);
    if (rodape)
        escrever(arquivo, rodape);

    return 1;
}

/* leitura e alteração*/

int editar_formulario_txt(const char *arquivo)
{
    FILE *f = fopen(arquivo, "r");
    if (!f)
    {
        printf("[ERRO] Arquivo '%s' não encontrado para edição.\n", arquivo);
        return 0;
    }

    printf("\n--- CONTEÚDO ATUAL DO ARQUIVO '%s' ---\n\n", arquivo);
    char linha[256];
    while (fgets(linha, sizeof(linha), f))
    {
        printf("%s", linha);
    }
    fclose(f);

    printf("\n--------------------------------------------------\n");
    printf("Deseja anexar uma observação ao formulário? (1 - Sim / 0 - Não): ");
    int opcao = 0;
    if (scanf("%d", &opcao) != 1)
        opcao = 0;
    getchar();

    if (opcao == 1)
    {
        char nota[256];
        printf("Digite a observação a ser anexada: ");
        if (fgets(nota, sizeof(nota), stdin))
        {
            nota[strcspn(nota, "\r\n")] = '\0';
            char nota_formatada[300];
            snprintf(nota_formatada, sizeof(nota_formatada), "\n[OBSERVAÇÃO REGISTRADA]: %s\n", nota);
            escrever(arquivo, nota_formatada);
        }
        printf("[OK] Alterações salvas com sucesso em '%s'!\n", arquivo);
    }

    return 1;
}

/* menu */

void editar_documento_por_blocos(const char *caminho_relatorio_txt)
{
    char caminho_meta[256];
    snprintf(caminho_meta, sizeof(caminho_meta), "relatorio_estrutura.meta");

    BlocoDocumento *head = NULL;
    char cabecalho[512] = "";
    char conteudo[16384] = "";
    char rodape[512] = "";
    char mun[60] = "MUNICÍPIO";
    char dt[12] = "01/01/2026";
    char tipo_sessao[30] = "Ordinária";
    int num_sessao = 1;
    int cabecalho_configurado = 0;

    limpar_tela();
    printf("====================================================\n");
    printf("       SISTEMA DE EDIÇÃO DE RELATÓRIO POR BLOCOS    \n");
    printf("====================================================\n");
    printf("1 - Criar Novo Relatório (Do Zero)\n");
    printf("2 - Carregar e Editar Relatório Existente\n");
    printf("Escolha uma opção: ");

    int opcao_inicial = ler_inteiro_seguro();

    if (opcao_inicial == 2)
    {
        head = carregar_relatorio(caminho_meta);
    }

    int opcao = -1;
    while (opcao != 0)
    {
        limpar_tela();
        exibir_blocos_menu(head);

        printf("\n--- MENU DE EDIÇÃO ---\n");
        printf("1 - Adicionar Bloco de Texto Livre\n");
        printf("2 - Adicionar Bloco de Tabela\n");
        printf("3 - Adicionar Quebra de Página\n");
        printf("4 - Guardar Estrutura e Gerar Ficheiro TXT Limpo\n");
        printf("0 - Concluir Edição de Blocos\n");
        printf("Opção: ");

        opcao = ler_inteiro_seguro();

        switch (opcao)
        {
        case 1:
            limpar_tela();
            adicionar_novo_bloco_texto(&head);
            pausar_tela();
            break;
        case 2:
            limpar_tela();
            adicionar_novo_bloco_tabela(&head);
            pausar_tela();
            break;
        case 3:
            limpar_tela();
            adicionar_novo_bloco_quebra(&head);
            pausar_tela();
            break;
        case 4:
            limpar_tela();
            salvar_estrutura_relatorio(caminho_meta, head);

            // Pergunta as informações apenas uma vez ao gerar
            if (!cabecalho_configurado)
            {
                coletar_dados_cabecalho(mun, sizeof(mun), dt, sizeof(dt),
                                        tipo_sessao, sizeof(tipo_sessao), &num_sessao);
                cabecalho_configurado = 1;
            }

            // Monta o cabeçalho com os dados reais informados
            formatar_cabecalho(cabecalho, sizeof(cabecalho), mun, dt, num_sessao, tipo_sessao);
            renderizar_conteudo_do_relatorio(head, conteudo, sizeof(conteudo));
            formatar_rodape(rodape, sizeof(rodape));

            if (imprimir_formulario_txt(caminho_relatorio_txt, cabecalho, conteudo, rodape))
            {
                printf("\n[SUCESSO] Ficheiro limpo gerado em '%s'!\n", caminho_relatorio_txt);
                printf("[INFO] Estrutura guardada em '%s'.\n", caminho_meta);
            }
            pausar_tela();
            break;
        case 0:
            salvar_estrutura_relatorio(caminho_meta, head);
            break;
        default:
            printf("\nOpção inválida!\n");
            pausar_tela();
            break;
        }
    }

    liberar_blocos(head);
}

int main()
{
    srand((unsigned int)time(NULL));

    // Criação das listas dinamicas
    Vereador *v = carregar_vereadores("OsCapaPreta.txt");
    Reuniao *r = carregar_reunioes("reunioes.txt");
    Projeto *p = carregar_projetos("projetos.txt");
    Presenca *pres = carregar_presencas("presenca.txt");
    Votacao *vot = carregar_votacoes("votacao.txt");

    // Endereço do relatorio
    const char *arquivo = "relatorio.txt";

    // Usuario vai poder adicionar "bloco a bloco",
    editar_documento_por_blocos(arquivo);

    // aq ele poe no formulario e ve se precisa editar algo
    editar_formulario_txt(arquivo);

    // NÃO ESQUEÇAM DE LIBERAR MEMORIA, IMPORTANTE MANTER (É O FREE GERAL)
    liberar_vereadores(v);
    liberar_reunioes(r);
    liberar_projetos(p);
    liberar_presencas(pres);
    liberar_votacoes(vot);

    return 0;
}
