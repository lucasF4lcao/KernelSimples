// Alterações: 
// 1. Alteramos o escalanomento de Primeiro a chegar, primeiro a ser servido para Round-Robin
// 2. Implementamos o Terminate_process
// 3. Implementamos uma chance de um processo entrar em estado Bloqueado

// obs - Dupla: Edinei Xavier e Lucas Oliveira

// Import de bibliotecas 
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define QUANTUM 3  // Quantum de tempo em segundos, o número pode ser modificado para qualquer outro

// Definição dos estados do processo
typedef enum {
    PRONTO,
    EM_EXECUCAO,
    BLOQUEADO,
    CONCLUIDO
} ProcessState;

// Estrutura para representar um processo (PCB)
typedef struct Process {
    int id;
    int time_remaining;
    ProcessState state;
    struct Process* next;
} Process;

// Cabeça da lista de processos
Process* process_list = NULL;
int next_process_id = 1;  // Identificador único para processos

// Funções auxiliares para manipulação de tela
void terminal_initialize(void);
void terminal_writestring(const char* str);


// Funções do gerenciamento de processos
void create_process(void);
void list_processes(void);
void terminate_process(int id);
void execute_processes_fcfs(void); // não esta sendo mais refenciado no codigo
void execute_processes_round_robin(void);

void terminal_initialize(void) {
    terminal_writestring("Inicializando terminal...\n");
}

// Função principal do kernel
int main(void) {
    int opcao;
    terminal_initialize();

    // Loop principal do kernel
    while (1) {
        // Verifica processos pendentes antes de continuar
        terminal_writestring("\nKernel Simples\n");
        terminal_writestring("1. Executar Processos (Round-Robin)\n");
        terminal_writestring("2. Criar Processo\n");
        terminal_writestring("3. Listar Processos\n");
        terminal_writestring("4. Terminar Processo\n");
        terminal_writestring("5. Sair\n");
        terminal_writestring("Escolha uma opção: ");

        scanf("%d", &opcao);
        
        //Menu né
        switch (opcao) {
            case 1:
                execute_processes_round_robin();
                break;
            case 2:
                create_process();
                break;
            case 3:
                list_processes();
                break;
            case 4:
                terminal_writestring("Digite o ID do processo a ser terminado: ");
                int id;
                if (scanf("%d", &id) == 1) {
                    terminate_process(id);
                }
                break;
            case 5:
                 terminal_writestring("Sistema finalizado.\n");
                exit(0);
                break;
            default:
                terminal_writestring("Opção inválida!\n");
        }

        sleep(1);  // Simula tempo de processamento
    }

    return 0;
}



// Função para criar um novo processo
void create_process(void) {
    Process* new_process = (Process*)malloc(sizeof(Process));
    
    if(!new_process){
        terminal_writestring("Erro ao criar processo\n");
        return;
    }
    new_process->id = next_process_id++; 
    new_process->time_remaining = rand() % 10 + 1; //tempo aleatorio entre 1 e 10
    new_process->state = PRONTO;
    new_process->next= NULL;
    
    //adicionar o processo ao final da lista
    if(!process_list){
        process_list=new_process;
    } else{
        Process* temp = process_list;
        while(temp->next){
            temp = temp->next;
        }
        temp->next = new_process;
    }
     terminal_writestring("Processo criado com sucesso\n");
}

// Função para listar os processos ativos
void list_processes(void) {
    Process* current = process_list;
    if(!current){
         terminal_writestring("Nenhum processo foi criado\n");
         return;
    }
    
     terminal_writestring("Lista de processo criados:\n");
     while(current){
        printf("ID: %d | ESTADO: %s| TEMPO RESTANTE: %d\n",
        current->id,
        (current->state == PRONTO) ? "PRONTO":
        (current->state == EM_EXECUCAO) ? "EM_EXECUCAO":
        (current->state == BLOQUEADO) ? "BLOQUEADO":
        "CONCLUIDO",
        current->time_remaining);
        
        current=current->next;
    }
}

// execução com Round-Robin
void execute_processes_round_robin(void) {
    if (!process_list) {
        terminal_writestring("Nenhum processo existente\n");
        return;
    }

    terminal_writestring("Iniciando o escalonamento Round Robin...\n");

    Process* queue = process_list;
    Process* tail = NULL;
    int current_time = 0;

    // Encontra o último processo da lista para formar uma fila circular
    while (queue->next) {
        queue = queue->next;
    }
    tail = queue;
    queue = process_list;
    tail->next = process_list;  // Liga o último processo ao primeiro (lista circular)

    Process* current = queue;   // Começa no início da fila
    Process* prev = tail;       // O anterior começa como o último da fila

    // Executa enquanto ainda houver processos na lista
    while (process_list) {

        // Se o processo estiver pronto ou já em execução
        if (current->state == PRONTO || current->state == EM_EXECUCAO) {
            current->state = EM_EXECUCAO;

            //  Determina quanto tempo vai rodar (QUANTUM)
            int exec_time = (current->time_remaining > QUANTUM) ? QUANTUM : current->time_remaining;

            // Simula execução segundo a segundo
            for (int i = 0; i < exec_time; i++) {
                sleep(1); // simula tempo passando
                current_time++;
                current->time_remaining--;

                //️ Mostra o progresso da execução
                printf("Tempo: %d | Processo: %d | Tempo restante: %d\n",
                    current_time, current->id, current->time_remaining);

                // Só bloqueia se ainda tiver tempo restante
                if (current->time_remaining > 0 && rand() % 10 == 0) {
                    current->state = BLOQUEADO;
                    printf("Tempo: %d | Processo: %d BLOQUEADO!\n", current_time, current->id);
                    break; // sai do loop de execução antes do fim do quantum
                }
            }

            // Se o processo terminou todo o tempo de execução
            if (current->time_remaining == 0) {
                current->state = CONCLUIDO;
                printf("Tempo: %d | Processo: %d CONCLUÍDO!\n", current_time, current->id);

                // Se só tinha 1 processo na fila circular
                if (current == current->next) {
                    free(current);
                    process_list = NULL;
                    break;
                } else {
                    // Remove o processo da lista circular
                    if (current == process_list) {
                        process_list = current->next; // atualiza cabeça
                    }
                    prev->next = current->next; // pula o atual
                    Process* to_free = current;
                    current = current->next;
                    free(to_free);
                }
                continue; // volta pro começo do while principal
            }

            // Se não foi bloqueado, volta pra estado PRONTO
            if (current->state != BLOQUEADO) {
                current->state = PRONTO;
            }

            prev = current;
            current = current->next;
        }

        // Se o processo está BLOQUEADO
        else if (current->state == BLOQUEADO) {
            // Tenta DESBLOQUEAR com 5% de chance
            if (rand() % 20 == 0) {
                current->state = PRONTO;
                printf("Tempo: %d | Processo: %d DESBLOQUEADO!\n", current_time, current->id);
            }
            prev = current;
            current = current->next;
        }

        // Qualquer outro estado, apenas avança
        else {
            prev = current;
            current = current->next;
        }
    }

    terminal_writestring("Todos os processos foram executados.\n");
}



// Função para terminar um processo com um determinado ID
void terminate_process(int id) {
    if (!process_list) {
        terminal_writestring("Nenhum processo criado ainda.\n");
        return;
    }

    Process* current = process_list;
    Process* prev = NULL;

    // Verifica se é uma lista circular (comparando com a referência inicial)
    int circular = 0;
    Process* temp = process_list;
    while (temp->next != NULL) {
        if (temp->next == process_list) {
            circular = 1;  // Se algum nó aponta de volta para o início, é circular
            break;
        }
        temp = temp->next;
    }

    // Caso a lista seja circular
    if (circular) {
        Process* head = process_list;

        // Percorre a lista circular
        do {
            if (current->id == id) {  // Encontrou o processo a remover
                if (current == current->next) {
                    // Único processo na lista circular
                    free(current);
                    process_list = NULL;
                } else if (current == process_list) {
                    // Se o processo a ser removido é o primeiro da lista
                    temp = process_list;

                    // Encontrar o último processo (o que aponta para a cabeça)
                    while (temp->next != process_list) {
                        temp = temp->next;
                    }

                    // Atualiza o último para apontar para o novo início
                    temp->next = current->next;

                    // Atualiza a cabeça
                    process_list = current->next;

                    // Libera o processo atual
                    free(current);
                } else {
                    // Processo no meio da lista
                    prev->next = current->next;
                    free(current);
                }

                printf("Processo %d terminado manualmente.\n", id);
                return;
            }

            // Avança os ponteiros
            prev = current;
            current = current->next;

        } while (current != process_list);  // Enquanto não dá a volta na lista

        // Se saiu do laço sem encontrar
        terminal_writestring("Processo com o ID especificado não encontrado.\n");
        return;
    }

    // Caso seja uma lista linear (não circular)
    current = process_list;
    prev = NULL;
    while (current) {
        if (current->id == id) {
            // Encontrou o processo

            if (prev) {
                // Processo no meio ou final
                prev->next = current->next;
            } else {
                // Processo no início da lista
                process_list = current->next;
            }

            free(current);
            printf("Processo %d terminado manualmente.\n", id);
            return;
        }

        // Avança os ponteiros
        prev = current;
        current = current->next;
    }

    // Processo não encontrado
    terminal_writestring("Processo com o ID especificado não encontrado.\n");
}


// Função para escrever no terminal
void terminal_writestring(const char* str) {
    printf("%s", str);
}
