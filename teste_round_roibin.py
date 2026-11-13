import random
# depois, botar uma seed no random

def definir_tempo_ate_IO(textoIO, tempoRestante):
    if textoIO == "NENHUM":
        return 0
    if tempoRestante > 4:
        return random.randint(2, tempoRestante-1)
        # aqui eu to retornando 9 para não dar um erro de código lá na frente, mas ele só vai receber esse "9"
        # quando o tempo restante for menor que 5. logo, ele não executará mais o I/O se o tempo restante for menor que 5.
    return 9

class Processo:
    def __init__(self, pid = None, ppid = None,
                 status=None, prioridade = None,
                 tempo_res = None, ProximoIO = None,
                 tempoIO = 1):

        tempoAteIO = 0

        if (not pid):
            # fazer lógica para o PID não repetir
            pid = random.randint(1, 50)
        if (not ppid):
            ppid = 0
        if (not status):
            status = "NOVO"
        if (not prioridade):
            # processos ao serem iniciados estão em prioridade alta. se não for feito desse jeito,
            # da pra fazer com duas filas e tirar essa propriedade da classe
            prioridade = "ALTA"
        if (not tempo_res):
            tempo_res = random.randint(5, 15)

        if (not ProximoIO):
            ProximoIO =  random.choice(["DISCO", "FITA", "IMPRESSORA", "NENHUM"])
            tempoAteIO = definir_tempo_ate_IO(ProximoIO, tempo_res)

            match (ProximoIO):
                case "DISCO":
                    tempoIO = 4
                case "FITA":
                    tempoIO = 6
                case "IMPRESSORA":
                    tempoIO = 5
                case "NENHUM":
                    tempoIO = 0

        self.PID = pid
        self.PPID = ppid
        self.Status = status
        self.Prioridade = prioridade
        self.TempoRestante = tempo_res
        self.ProximoIO = ProximoIO
        self.TempoIO = tempoIO
        self.TempoAteIO = tempoAteIO

    def __str__(self):
        return f'### {self.PID} ({self.PPID}) --> {self.Status} - {self.Prioridade}; tempo: {self.TempoRestante}s'

    def processo_bloqueado(self):
        return self.Status == "BLOQUEADO"


def inicializar_processos(quantidade_de_processos, lista_processos = []):
    if quantidade_de_processos == 0:
        return lista_processos
    else:
        lista_processos.append(Processo())
        return inicializar_processos(quantidade_de_processos-1, lista_processos)


def verificar_tempo_restante_e_atualiza_estado_do_processo(processo, tempo, quantum):
    # essa função pode ser renomeada depois, porque ela representa o ciclo de execução da CPU
    # aqui, quando for em C, tu pode usar direto o ponteiro para a variável tempo externa invés de retornar ela
    diferencaDaTempoAteIO = processo.TempoAteIO - quantum
    if (diferencaDaTempoAteIO <= 0 and processo.ProximoIO != "NENHUM"):
        # essa gambiarra de código e a variável "diferendaDaTempoAteIO" serve para medir se o tempo de ação seria um quantum inteiro ou não
        processo.Status = "BLOQUEADO"
        tempoAdicionado = quantum + diferencaDaTempoAteIO
        tempo += tempoAdicionado

        print(f'[{tempo}ms] - PID({processo.PID}) pediu acesso no I/O ({processo.ProximoIO}) depois de {tempoAdicionado} unidades')
        return tempo
    else:
        processo.TempoAteIO -= quantum

    processo.TempoRestante -= quantum

    tempoAdicionado = 0
    if processo.TempoRestante < 0:
        tempoAdicionado += quantum + processo.TempoRestante
    else:
        tempoAdicionado += quantum

    print(f'[{tempo}ms] - exec PID({processo.PID}) por {tempoAdicionado} unidades')
    tempo += tempoAdicionado

    if (processo.TempoRestante <= 0):
        processo.Status = "FINALIZADO"
        # PROCESSOS são REMOVIDOS na linha ---114---
        # porque essa função executa 2 vezes em locais diferentes
    else:
        # REGISTRAR PREEMPTAÇÃO
        # debug:
        # print("!# - PREEMPTADO")
        pass


    return tempo

fila_processos_io = []

processos = inicializar_processos(5)
existem_processos_nao_finalizados = len(processos) > 0
tempo = 0
quantum = 3

while existem_processos_nao_finalizados:
    # aqui eu não preciso mover os novos processos pra uma nova prioridade porque eu to
    # tratando tudo isso como uma lista só, o que eu sei que pode gerar complicações em C
    # então, em C, eu acho melhor ter uma pilha de alta prioridade e outra de baixa

    # atualizar filas de IO
    for processo in processos:
        if processo.Status == "BLOQUEADO":
            # não sei o quanto que tem que mexer aqui
            processo.TempoIO -= quantum
            print(f'[{tempo}ms] - I/O PID({processo.PID}) {processo.ProximoIO}')

            if (processo.TempoIO <= 0):
                match (processo.ProximoIO):
                    case "DISCO":
                        processo.Prioridade = "BAIXA"
                    case "FITA":
                        processo.Prioridade = "ALTA"
                    case "IMPRESSORA":
                        processo.Prioridade = "ALTA"
                print(f'[{tempo}ms] !- I/O FINALIZADO PID({processo.PID}) {processo.ProximoIO}')
                processo.Status = "PRONTO"

                processo.ProximoIO = random.choice(["DISCO", "FITA", "IMPRESSORA", "NENHUM"])

    # primeiro passar pelos processos de alta prioridade
    print('-- Tentando executar fila de alta prioridade --')
    tem_algum_processo_de_alta_prioridade = False
    for processo in processos:
        if processo.Prioridade == "ALTA" and not processo.processo_bloqueado():
            tem_algum_processo_de_alta_prioridade = True
            tempo = verificar_tempo_restante_e_atualiza_estado_do_processo(processo, tempo, quantum)
            # remover processo da lista de processos
            # LEMBRAR DE COLOCAR UM OUTPUT DEPOIS

            # mover o processo para baixa prioridade quanto o quantum acaba
            processo.Prioridade = "BAIXA"

    tem_algum_processo_de_baixa_prioridade = False
    # se não tiver processos de alta prioridade, fazer os de baixa prioridade
    if not tem_algum_processo_de_alta_prioridade:
        print('-- Tentando executar fila de baixa prioridade --')

        for processo in processos:
            if processo.Prioridade == "BAIXA" and not processo.processo_bloqueado():
                tempo = verificar_tempo_restante_e_atualiza_estado_do_processo(processo, tempo, quantum)
                tem_algum_processo_de_alta_prioridade = True



    if not tem_algum_processo_de_alta_prioridade and not tem_algum_processo_de_baixa_prioridade:
        tempo += quantum
        print(f'#! - CPU OCIOSA +[{tempo}]')

    # linha pythonica que tira os processos finalizados da lista
    processos = [processo for processo in processos if processo.Status != "FINALIZADO"]


    print(f'# fim do ciclo ({tempo}ms) - processos restantes: {len(processos)}\n')
    # for processo in processos:
    #     print(processo)
    existem_processos_nao_finalizados = (len(processos) > 0)
# printar tempo médio de espera;
# o turnaround;
# n° de preempções;
# I/O por tipo;
#  % de CPU ociosa
