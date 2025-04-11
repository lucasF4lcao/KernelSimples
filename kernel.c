#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

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
void execute_processes_fcfs(void);
void execute_processes_rr(void);


// Função principal do kernel
int main(void) {
    int opcao;
    terminal_initialize();

    // Loop principal do kernel
    while (1) {
        // Verifica processos pendentes antes de continuar
        terminal_writestring("\nKernel Simples\n");
        terminal_writestring("1. Executar Processos (FCFS)\n");
        terminal_writestring("2. Executar Processos (RR)\n");
        terminal_writestring("3. Criar Processo\n");
        terminal_writestring("4. Listar Processos\n");
        terminal_writestring("5. Terminar Processo\n");
        terminal_writestring("6. Sair\n");
        terminal_writestring("Escolha uma opção: ");

        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                execute_processes_fcfs();
                break;
            case 2:
                execute_processes_rr();
                break;
            case 3:
                create_process();
                break;
            case 4:
                list_processes();
                break;
            case 5:
                terminal_writestring("Digite o ID do processo a ser terminado: ");
                int id;
                if (scanf("%d", &id) == 1) {
                    terminate_process(id);
                }
                break;
            case 6:
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
        terminal_writestring("Erro ao criar processo");
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
     terminal_writestring("Processo criado com sucesso");
}

// Função para listar os processos ativos
void list_processes(void) {
    Process* current = process_list;
    if(!current){
         terminal_writestring("Nenhum processo foi criado");
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
// Função para terminar um processo
void terminate_process(int id) {
}

// Função de escalonamento FCFS (First-Come, First-Served)
void execute_processes_fcfs(void) {
    
    if(!process_list){
        terminal_writestring("Nenhum processo existente\n");
        return;
    }
    
    terminal_writestring("Iniciar o Escalonamento...\n");
    Process* current = process_list;
    Process* prev = NULL;
    int current_time = 0;
    
    while(current){
        //COMEÇA A EXECUCAO
        current->state = EM_EXECUCAO;
        printf("Tempo: %d Processo %d em execução\n", current_time, current->id);
        
        while(current->time_remaining>0){
            sleep(1);
            current_time++;
            current->time_remaining--;
            printf("Tempo: %d Processo: %d Tempo restate: %d\n", current_time, current->id, current->time_remaining);
        }
        
        current->state = CONCLUIDO;
        printf("Tempo: %d Processo: %d concluido \n", current_time, current->id);
        
        //remover o processo concluido
        if(prev){
            prev->next = current->next;
        }else{
            process_list=current->next;
        }
        free(current);
        current = (prev) ? prev->next : process_list;
        
    }
    terminal_writestring("Todos os processos foram executados...\n");
}

// Função de escalonamento Round Robin
void execute_processes_rr(void) {
    const int quantum = 2;
    int current_time = 0;

    if (!process_list) {
        terminal_writestring("Nenhum processo existente\n");
        return;
    }

    terminal_writestring("Iniciando Escalonamento Round Robin...\n");

    Process *current = process_list;
    Process *prev = NULL;

    while (process_list) {
        current = process_list;
        prev = NULL;

        while (current) {
            if (current->state == CONCLUIDO) {
                prev = current;
                current = current->next;
                continue;
            }

            current->state = EM_EXECUCAO;
            printf("Tempo: %d | Executando Processo %d\n", current_time, current->id);

            int time_slice = (current->time_remaining > quantum) ? quantum : current->time_remaining;

            for (int i = 0; i < time_slice; i++) {
                sleep(1);
                current_time++;
                current->time_remaining--;
                printf("Tempo: %d | Processo %d | Tempo restante: %d\n", current_time, current->id, current->time_remaining);
            }

            if (current->time_remaining <= 0) {
                current->state = CONCLUIDO;
                printf("Processo %d concluído!\n", current->id);
            } else {
                current->state = PRONTO;
            }

            // mover para o final da fila se não for concluído
            Process* next = current->next;

            if (current->state != CONCLUIDO) {
                // mover para o final
                if (current == process_list) {
                    process_list = current->next;
                } else if (prev) {
                    prev->next = current->next;
                }

                // achar o final da lista
                Process* tail = process_list;
                if (!tail) {
                    process_list = current;
                    current->next = NULL;
                } else {
                    while (tail->next) {
                        tail = tail->next;
                    }
                    tail->next = current;
                    current->next = NULL;
                }
                current = next;
            } else {
                // remover da lista
                if (current == process_list) {
                    process_list = current->next;
                    free(current);
                    current = process_list;
                } else if (prev) {
                    prev->next = current->next;
                    free(current);
                    current = prev->next;
                }
            }
        }
    }

    terminal_writestring("Todos os processos foram executados (RR).\n");
}


// Função para inicializar o terminal
void terminal_initialize(void) {
    terminal_writestring("Inicializando terminal...\n");
}

// Função para escrever no terminal
void terminal_writestring(const char* str) {
    printf("%s", str);
}
