#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


typedef enum {
    NENHUM,
    DISCO,
    FITA,
    IMPRESSORA,
} TipoIO;

typedef enum {
    NOVO,
    PRONTO,
    EXECUTANDO,
    BLOQUEADO,
    FINALIZADO,
} Status;

typedef struct {
    int pid;
    int tempoRestante;
    int tempoIO;
    int tempoAteIO;
    TipoIO proximoIO;
    int tempoInicioIO;
    // int prioridade;
    Status status;

    // itens de estatistica abaixo
    int tempoChegada;
    int tempoFinalizacao;
    int tempoTotalEspera;
    int tempoUltimaEntradaPronto;
} Processo;

typedef struct {
    int tempoTotal;
    int quantidadePreempcoes;

    int eventosIO[4];

    int tempoTotalEspera;
    int tempoTotalRetorno;

    int quantidadeProcessos;

    int tempoCPUOciosa;
} Estatisticas;

typedef struct No {
    Processo processo;
    struct No *proximo;
} No;

No* criarNo(Processo processo) {
    No* enderecoDoNovoNo = malloc(sizeof(No));
    if (enderecoDoNovoNo == NULL) {
        printf("#! Erro ao alocar memória");
        return NULL;
    }

    enderecoDoNovoNo->processo = processo;
    enderecoDoNovoNo->proximo = NULL;
    return enderecoDoNovoNo;
}

void imprimirProcesso(Processo *p) {
    printf("PID: %d\n", p->pid);
}

void inserirProcesso(No* *inicio, Processo processo) {
    // inicio já é um ponteiro. e aqui, pegamos o ponteiro do ponteiro
    // para alterar a variável ao qual o ponteiro ta apontando.
    // to anotando isso porque demorei muito tempo pra entender essa explicação.
    No *novo = criarNo(processo);

    if (novo == NULL) {
        return;
    }

    if (*inicio == NULL) {
        *inicio = novo;
        return;
    }

    No *atual = *inicio;

    while(atual->proximo != NULL) {
        // varre a lista, indo até o último elemento (onde ele não aponta para nenhum outro)
        atual = atual->proximo;
    }

    atual->proximo = novo;
}

void imprimirLista(No *inicioDaLista) {
    No *atual = inicioDaLista;

    while (atual != NULL) {
        imprimirProcesso(&atual->processo);
        atual = atual->proximo;
    }
}

void removerProcessoDaLista(No* *inicio, int pid) {
    No *atual = *inicio;
    No *anterior = NULL;

    while (atual != NULL) {
        // varrer a lista completa para encontrar o processo que queremos apagar

        if (atual->processo.pid == pid) {

            if (anterior == NULL) {
                // se não tiver nenhum processo anterior, é porque estamos no primeiro elemento da lista
                // então a lista terá um novo início
                *inicio = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
                // se existir um anterior,
                // substitua o elemento para o qual ele aponta pelo elemento que é apontado pelo ponteiro que será apagado
            }

            free(atual);
            return;
        }

        anterior = atual;
        atual = atual->proximo;
        // lembrando que, se for o fim da lista, o próximo será NULL, então atual se tornará NULL
    }
}

void imprimirEstatisticas(Estatisticas *estatisticas) {
    printf("\n========== ESTATISTICAS FINAIS ==========\n");
    printf("Tempo total da simulacao: %d unidades\n", estatisticas->tempoTotal);
    printf("Processos finalizados: %d\n", estatisticas->quantidadeProcessos);
    printf("Quantidade de preempcoes: %d\n", estatisticas->quantidadePreempcoes);

    printf("\nEventos de I/O:\n");
    printf("  Disco: %d\n", estatisticas->eventosIO[DISCO]);
    printf("  Fita: %d\n", estatisticas->eventosIO[FITA]);
    printf("  Impressora: %d\n", estatisticas->eventosIO[IMPRESSORA]);

    if (estatisticas->quantidadeProcessos > 0) {
        printf("\nTempo medio de espera: %.2f unidades\n",
            (double) estatisticas->tempoTotalEspera / estatisticas->quantidadeProcessos);
        printf("Tempo medio de turnaround: %.2f unidades\n",
            (double) estatisticas->tempoTotalRetorno / estatisticas->quantidadeProcessos);
    } else {
        printf("\nTempo medio de espera: 0.00 unidades\n");
        printf("Tempo medio de turnaround: 0.00 unidades\n");
    }

    if (estatisticas->tempoTotal > 0) {
        double percentualOcioso =
            100.0 * estatisticas->tempoCPUOciosa / estatisticas->tempoTotal;
        printf("Tempo de CPU ociosa: %d unidades (%.2f%%)\n",
            estatisticas->tempoCPUOciosa, percentualOcioso);
    } else {
        printf("Tempo de CPU ociosa: 0 unidades (0.00%%)\n");
    }
    printf("=========================================\n");
}

void executarProcesso(Processo* processo, No* *lista, No* *listaIO, int quantum, int *timer, const char *tipoIONomes[], Estatisticas *estatisticas) {
    // Ao ser escolhido para executar, o processo deixa de esperar na fila de prontos.
    processo->tempoTotalEspera += *timer - processo->tempoUltimaEntradaPronto;
    processo->status = EXECUTANDO;

    int diferencaDeTempoAteIO = (*processo).tempoAteIO - quantum;
    // printf(
    //     "DEBUG PID(%d): TipoIO=%d | tempoAteIO=%d | diferenca=%d\n",
    //     processo->pid,
    //     processo->proximoIO,
    //     processo->tempoAteIO,
    //     diferencaDeTempoAteIO
    // );

    if (diferencaDeTempoAteIO <= 0 && (*processo).proximoIO != NENHUM) {

        (*processo).status = BLOQUEADO;
        int tempoAdicionado = quantum + diferencaDeTempoAteIO;
        (*timer) += tempoAdicionado;
        (*processo).tempoRestante -= tempoAdicionado;

        printf("[%dms] - PID(%d) pediu acesso no I/O (%s) depois de %d unidades\n",
            (*timer),
            (*processo).pid,
            // (*processo).proximoIO,
            tipoIONomes[(*processo).proximoIO],
            tempoAdicionado);

        processo->tempoInicioIO = *timer;
        estatisticas->eventosIO[processo->proximoIO]++;
        inserirProcesso(listaIO, *processo);

        removerProcessoDaLista(lista, (*processo).pid);
        return;
    } else {
        (*processo).tempoAteIO -= quantum;
    }

    (*processo).tempoRestante -= quantum;

    int tempoAdicionado = 0;
    if ((*processo).tempoRestante <= 0) {
        tempoAdicionado += quantum + (*processo).tempoRestante;
    } else {
        tempoAdicionado += quantum;
    }
    printf("[%dms] - exec PID(%d) por %d unidades\n", (*timer), (*processo).pid, tempoAdicionado);
    (*timer) += tempoAdicionado;

    if ((*processo).tempoRestante <= 0) {
        (*processo).status = FINALIZADO;
        processo->tempoFinalizacao = *timer;
        estatisticas->quantidadeProcessos++;
        estatisticas->tempoTotalEspera += processo->tempoTotalEspera;
        estatisticas->tempoTotalRetorno += processo->tempoFinalizacao - processo->tempoChegada;
        removerProcessoDaLista(lista, (*processo).pid);
    } else {
        estatisticas->quantidadePreempcoes++;
        // o processo continua pronto e vai voltar a esperar a partir deste instante.
        processo->tempoUltimaEntradaPronto = *timer;
    }
}


// nas duas funções abaixo, tem elementos gerados aleatoriamente
void inserirProximoIOeTempoDeIO(Processo* processo) {
    srand(time(NULL));
    int tempoRestante = processo->tempoRestante;
    int tempoAteIO = 0;
    int tempoDeIO = 0;
    TipoIO proximoIO;

    if (tempoRestante <= 3) {
        proximoIO = NENHUM;
    } else {
        int sorteio = rand() % 4;
        switch (sorteio) {
            case 0:
                proximoIO = NENHUM;
                break;
            case 1:
                proximoIO = DISCO;
                break;
            case 2:
                proximoIO = FITA;
                break;
            case 3:
                proximoIO = IMPRESSORA;
                break;
        }
        tempoAteIO = 2 + rand() % (tempoRestante - 1);
        tempoDeIO = 1 + rand() % 8;
    }


    processo->proximoIO = proximoIO;
    processo->tempoAteIO = tempoAteIO;
    processo->tempoIO = tempoDeIO;
}
void inicializarProcessos(int quantidadeDeProcessos, No* *filaAAdicionar, int *pidCount) {
    srand(time(NULL));
    for (int i = 0; i < quantidadeDeProcessos; i++) {
        (*pidCount)++;

        int sorteioTempo = 2 + rand() % 14;

        Processo novoProcesso = {0};
        novoProcesso.pid = *pidCount;
        novoProcesso.tempoRestante = sorteioTempo;
        novoProcesso.status = PRONTO;
        novoProcesso.tempoChegada = 0;
        novoProcesso.tempoUltimaEntradaPronto = 0;
        inserirProximoIOeTempoDeIO(&novoProcesso);

        inserirProcesso(filaAAdicionar, novoProcesso);
    }
}

int main () {
    printf("\n-- RoundRobin iniciado --\n");
    const char* TipoIONomes[] = {
        "NENHUM",
        "DISCO",
        "FITA",
        "IMPRESSORA"
    };
    const char* StatusEmString[] = {
        "NOVO",
        "PRONTO",
        "EXECUTANDO",
        "BLOQUEADO",
        "FINALIZADO"
    };
    Estatisticas estatisticasDoAlgoritmo = {0};

    int pidCounter = -1;
    int const quantum = 3;
    int timer = 0;

    // inicializar as listas/filas
    No *listaAlta = NULL;
    No *listaBaixa = NULL;
    No *listaIO = NULL;

    inicializarProcessos(5, &listaAlta, &pidCounter);

    // printf("--= PROCESSOS =--\n");
    // imprimirLista(listaAlta);

    // removerProcessoDaLista(&listaAlta, 2);
    // imprimirLista(listaAlta);

    bool existemProcessosNaoFinalizados = true;
    while(existemProcessosNaoFinalizados) {
        // Se nao existe processo pronto, a CPU fica ociosa enquanto os I/Os avancam.
        if (listaAlta == NULL && listaBaixa == NULL && listaIO != NULL) {
            timer += quantum;
            estatisticasDoAlgoritmo.tempoCPUOciosa += quantum;
        }

        No* *tempIO = &listaIO;

        while(*tempIO != NULL) {
            No *noProcessoAtual = *tempIO;
            Processo *processoIO = &noProcessoAtual->processo;
            int instanteFinalizacaoIO = processoIO->tempoInicioIO + processoIO->tempoIO;

            // O tempoIO guarda a duracao total. O I/O termina quando o timer alcanca seu prazo final.
            if (timer >= instanteFinalizacaoIO) {
                int tempoDecorridoIO = processoIO->tempoIO;

                printf("[%dms] !- I/O FINALIZADO PID(%d) {%s} depois de %d unidades\n",
                    instanteFinalizacaoIO,
                    processoIO->pid,
                    TipoIONomes[processoIO->proximoIO],
                    tempoDecorridoIO
                );

                processoIO->status = PRONTO;
                processoIO->tempoUltimaEntradaPronto = instanteFinalizacaoIO;

                if (processoIO->proximoIO == DISCO) {
                    inserirProximoIOeTempoDeIO(processoIO);
                    inserirProcesso(&listaBaixa, *processoIO);
                } else {
                    // isso serve para FITA e IMPRESSORA
                    inserirProximoIOeTempoDeIO(processoIO);
                    inserirProcesso(&listaAlta, *processoIO);
                }

                // aqui, eu to mudando o valor do elemento ao qual tempIO representa na lista, atualizando para o próximo valor
                *tempIO = noProcessoAtual->proximo;
                free(noProcessoAtual);
                // aqui, o tempIO ainda aponta pro mesmo index da lista que apontava antes, mas ele altera quem esse index representa

            } else {
                printf("[%dms] - I/O PID(%d) {%s} (em andamento; termina em ~%dms)\n",
                    timer,
                    processoIO->pid,
                    TipoIONomes[processoIO->proximoIO],
                    instanteFinalizacaoIO
                );

                // aqui, eu to mudando pra quem o tempIO aponta
                tempIO = &(noProcessoAtual->proximo);
            }
        }


        No *tempAlta = listaAlta;
        while(tempAlta != NULL) {
            No *proximoItemAlto = tempAlta->proximo;
            executarProcesso(&(tempAlta->processo), &listaAlta, &listaIO, quantum, &timer, TipoIONomes, &estatisticasDoAlgoritmo);
            tempAlta = proximoItemAlto;
        }

        if (listaAlta == NULL) {
            No *tempBaixa = listaBaixa;
            while(tempBaixa != NULL) {
                No* proximoItemBaixo = tempBaixa->proximo;
                executarProcesso(&(tempBaixa->processo), &listaBaixa, &listaIO, quantum, &timer, TipoIONomes, &estatisticasDoAlgoritmo);
                tempBaixa = proximoItemBaixo;
            }
        }


        if (listaAlta == NULL &&
            listaBaixa == NULL &&
            listaIO == NULL) {
            existemProcessosNaoFinalizados = false;
        }

        printf("#- FIM DO LOOP -> %d (debug)\n\n", timer);
    }

    estatisticasDoAlgoritmo.tempoTotal = timer;
    imprimirEstatisticas(&estatisticasDoAlgoritmo);
}
