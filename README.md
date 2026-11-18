# Trabalho Final de Sistemas Operacionais

## Grupo
- Andy
- Bruna Rafaela dos Santos Hoch
- Caio Bernardo
- Eduardo Augusto Kirst
- Pedro Henrique Turossi


## Linguagem utilizada
- C <- código principal
- Python <- planejamento e estrutura

## Premissas do escalonador

- **Algoritmo:** Round Robin com duas filas de prioridade;
- **Quantum:** 3 unidades de tempo;
- **Número máximo de processos:** 5 processos;
- **Tempos de CPU:** Gerados aleatoriamente, variando entre 2 e 15 unidades de tempo;
- **Tempos de I/O:** Gerados aleatoriamente, variando entre 1 e 8 unidades de tempo;
- **Critério de geração dos processos:** Os processos são inicializados com tempos de execução e operações de I/O definidos aleatoriamente;
- **Semente aleatória:** Utiliza a biblioteca `stdlib.h` para geração de números pseudoaleatórios. -> `srand(time(NULL))`.

### Funcionamento das filas

O escalonador utiliza duas filas de processos prontos:

- **Fila de alta prioridade:** Recebe processos novos e processos que retornam de operações de I/O de fita ou impressora;
- **Fila de baixa prioridade:** Recebe processos que retornam de operações de I/O de disco.

A fila de alta prioridade é atendida primeiro. A fila de baixa prioridade somente é executada quando não existem processos na fila de alta prioridade.

- **Fila de I/O:** Armazena os processos bloqueados que estão aguardando a conclusão de uma operação de entrada/saída (disco, fita ou impressora), retornando à fila de prontos adequada após o término da operação.

---

# Como executar o projeto

É necessário possuir um compilador C, como o GCC, instalado no sistema.

Na raiz do projeto, execute:

```bash
gcc src/*.c -o escalonador
./escalonador
```

Caso o arquivo `.c` esteja diretamente na raiz do projeto:

```bash
gcc RoundRobinV1.c -o escalonador
./escalonador
```

## Como compilar com CMake

O projeto utiliza o CMake para automatizar o processo de compilação.

### Pré-requisitos
- CMake instalado.
- Compilador C compatível com o padrão utilizado no projeto (GCC, por exemplo).

### Compilação

Na pasta raiz do projeto, execute:

```bash
cmake -S . -B build
cmake --build build
```

### Execução

Após a compilação, execute o programa gerado dentro da pasta `build`:

```bash
./build/escalonador
```

*Observação: o caminho do executável pode variar conforme a configuração do `CMakeLists.txt`.*

## Como executar com Docker

É necessário ter o Docker instalado (no Windows, o Docker Desktop com WSL 2).

Na raiz do projeto, execute:

```bash
docker build -t so-escalonador .
docker run --rm so-escalonador
```

---

## O que aparece na saída

Durante a execução, o simulador apresenta os principais eventos do escalonamento, incluindo:

- Início da execução de um processo e seu tempo de utilização da CPU.
- Solicitações de operações de I/O, identificando o processo e o dispositivo solicitado.
- Processos executando operações de I/O.
- Finalização das operações de I/O e retorno dos processos às filas de prontos.
- Finalização dos processos.

Ao término da simulação, é apresentado um relatório com as estatísticas coletadas, incluindo tempo total da simulação, quantidade de preempções, eventos de I/O por dispositivo, tempo médio de espera, tempo médio de turnaround e utilização da CPU.

## Limitações conhecidas
- Os processos e suas operações de I/O são gerados aleatoriamente;
- A simulação utiliza um modelo simplificado de gerenciamento de operações de I/O;
- Não são simulados dispositivos físicos ou operações reais de entrada e saída;
- O escalonador possui regras de prioridade simplificadas, com preferência pela fila de alta prioridade.
