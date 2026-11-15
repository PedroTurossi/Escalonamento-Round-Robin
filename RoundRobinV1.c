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

typedef struct {
    int pid;
    int tempoRestante;
    int tempoIO;
    int tempoAteIO;
    TipoIO proximoIO;
    // int prioridade;
    int status;
} Processo;

void imprimirProcesso(Processo *p) {
    printf("PID: %d\n", p->pid);
}


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

void executarProcesso(Processo* processo, No* *lista, No* *listaIO, int quantum, int *timer) {
    int diferencaDeTempoAteIO =  (*processo).tempoAteIO - quantum;
    if ((*processo).tempoAteIO <= 0) {
        (*processo).status = 4; // aqui seria o Status BLOQUEADO
        int tempoAdicionado = quantum + diferencaDeTempoAteIO;
        *timer += tempoAdicionado;

        printf("[%dms] - PID(%d) pediu acesso no I/O (IO?) depois de %d unidades\n", *timer, (*processo).pid, tempoAdicionado);
        inserirProcesso(listaIO, *processo);
        removerProcessoDaLista(lista, (*processo).pid);
        return;
    } else {
        (*processo).tempoAteIO -= quantum;
    }

    (*processo).tempoRestante -= quantum;

    int tempoAdicionado = 0;
    if ((*processo).tempoRestante < 0) {
        tempoAdicionado += quantum + (*processo).tempoRestante;
    }  else {
        tempoAdicionado += quantum;
    }
    printf("[%dms] - exec PID(%d) por %d unidades\n", *timer, (*processo).pid, tempoAdicionado);
    *timer += tempoAdicionado;

    if ((*processo).tempoRestante <= 0) {
        // status finalizado
        removerProcessoDaLista(&(*lista), (*processo).pid);
    } else {
        // registrar preemptação
    }
}


void inicializarProcessos(int quantidadeDeProcessos, No* *filaAAdicionar, int *pidCount) {
    for (int i = 0; i < quantidadeDeProcessos; i++) {
        (*pidCount)++;
        TipoIO proximoIO;
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
        Processo novoProcesso = {*pidCount, 5, 0, 0, 1};

        inserirProcesso(filaAAdicionar, novoProcesso);
    }
}

int main () {
    // coloquei esse print só pra separar os comandos do meu console
    printf("\n-- RoundRobin iniciado --\n");

    int pidCounter = -1;
    int quantum = 3;
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
        No* *tempIO = &listaIO;
        while(*tempIO != NULL) {
            (*tempIO)->processo.tempoIO -= quantum;
            printf("[%dms] - I/O PID(%d) {processo.ProximoIO}\n", timer, (*tempIO)->processo.pid);

            if ((*tempIO)->processo.tempoIO <= 0) {
                // não tem nenhum prin especifico que mostra que o IO foi finalizado durante meio quantum
                // só pra avisar mesmo
                switch((*tempIO)->processo.proximoIO) {
                    case DISCO:
                        inserirProcesso(&listaBaixa, (*tempIO)->processo);
                        removerProcessoDaLista(&listaIO, (*tempIO)->processo.pid);
                        break;
                    case FITA:
                        inserirProcesso(&listaAlta, (*tempIO)->processo);
                        removerProcessoDaLista(&listaIO, (*tempIO)->processo.pid);
                        break;
                    case IMPRESSORA:
                        inserirProcesso(&listaAlta, (*tempIO)->processo);
                        removerProcessoDaLista(&listaIO, (*tempIO)->processo.pid);
                        break;
                }

                printf("[%dms] !- I/O FINALIZADO PID(%d) {%d}", timer, (*tempIO)->processo.pid, (*tempIO)->processo.proximoIO);
                // depois, tem que definir o que os números dos status significam
                // ou fazem um enum deles também, talvez seria melhor pra ler o código
                (*tempIO)->processo.status = 0;
            }
            tempIO = &((*tempIO)->proximo);
        }


        No *tempAlta = listaAlta;
        while(tempAlta != NULL) {
            executarProcesso(&(tempAlta->processo), &listaAlta, &listaIO, quantum, &timer);
            tempAlta = tempAlta->proximo;
        }

        if (listaAlta == NULL) {
            No *tempBaixa = listaBaixa;
            while(tempBaixa != NULL) {
                executarProcesso(&(tempBaixa->processo), &listaBaixa, &listaIO, quantum, &timer);
                tempBaixa = tempBaixa->proximo;
            }
        }


        if (listaAlta == NULL &&
            listaBaixa == NULL &&
            listaIO == NULL) {
            existemProcessosNaoFinalizados = false;
        }
    }
}
