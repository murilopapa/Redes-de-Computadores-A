#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h> /* for msgget(), msgctl() */ //faltando sys
#include <sys/msg.h> /* for msgget(), msgctl() */ //faltando sys
#include <sys/shm.h>
#include <sys/sem.h>

#define TRUE 1
#define SHM_KEY 9025
#define MESSAGE_MTYPE 1
#define SEM_PERMS 0666
#define SEM_KEY_A 9035

void createIntSharedMemory(int *shared_mem_id, int shared_mem_key, int **shared_mem_address);
void createSemaphore(int *semaphore_id, int semaphore_key);
void removeSharedMemory(int shared_mem_id);
void removeSemaphore(int semaphore_id);
void lockSemaphore(int semaphore_id);
void unlockSemaphore(int semaphore_id);

//********* estruturas necessarias para o buffer

typedef struct
{
    char usuarios[10][20];  // 0 a 9 usuarios e o décimo é /0
    char mensagens[10][80]; // 0 a 9 mensagens e a décima é /0
    int indice;             // variavel para contar a quantidade de mensagens
} dados_usuarios;

//memoria compartilhada
int semaphore_id_A;
int g_sem_id;
int g_shm_id;
int *g_shm_addr;
sem_t mutex;

dados_usuarios shared_mem_id;
dados_usuarios *shared_mem_address;
struct sembuf semaphore_lock_op[1];   // Struct containing a lock operation
struct sembuf semaphore_unlock_op[1]; // Struct containing an unlock operation

/*
 * Servidor TCP
 */
main(argc, argv) int argc;
char **argv;
{
    unsigned short port;
    char sendbuf[101];
    char recvbuf[101];
    struct sockaddr_in client;
    struct sockaddr_in server;
    int s;  /* Socket para aceitar conex�es       */
    int ns; /* Socket conectado ao cliente        */
    int namelen;

    char mensagembuf[80]; // mensagem que recebe na função
    char usuariobuf[20];  // usuario que recebe na função

    char mensagem_inteira[101];
    pid_t pid, fid;

    semaphore_lock_op[0].sem_num = 0;
    semaphore_lock_op[0].sem_op = -1;
    semaphore_lock_op[0].sem_flg = 0;
    semaphore_unlock_op[0].sem_num = 0;
    semaphore_unlock_op[0].sem_op = 1;
    semaphore_unlock_op[0].sem_flg = 0;

    createSemaphore(&semaphore_id_A, SEM_KEY_A);
    createIntSharedMemory(&shared_mem_id, SHM_KEY, &shared_mem_address);

    /*
     * O primeiro argumento (argv[1]) � a porta
     * onde o servidor aguardar� por conex�es
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    int queue_id;         // declarando o ID da fila
    int rtn = 1;          // declarando rtn
    int cont_barb = 0;    // contador de barbeiros
    int cont_cliente = 0; // contador de clientes
    int controle = 0;
    // Initializing the semaphore operators

    port = (unsigned short)atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conex�es
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    /*
    * Define a qual endere�o IP e porta o servidor estar� ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endere�os IP
    */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor � porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }

    /*
     * Prepara o socket para aguardar por conex�es e
     * cria uma fila de conex�es pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    printf("\n...SERVIDOR INICIADO...\n");
    while (1)
    {
        /*
	  * Aceita uma conex�o e cria um novo socket atrav�s do qual
	  * ocorrer� a comunica��o com o cliente.
	  */
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        if ((pid = fork()) == 0)
        {
            /*
		 * Processo filho 
		 */

            /* Fecha o socket aguardando por conex�es */
            close(s);

            /* Processo filho obtem seu pr�prio pid */
            fid = getpid();

            while (1)
            {
                /* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
                memset(mensagembuf, 0, sizeof(mensagembuf));
                memset(usuariobuf, 0, sizeof(usuariobuf));
                memset(recvbuf, 0, sizeof(recvbuf));
                memset(sendbuf, 0, sizeof(sendbuf));

                if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
                {
                    perror("Recvbuf()");
                    exit(6);
                }
                printf("\n[%d] Mensagem recebida do cliente: %s\n", fid, recvbuf);

                // Recebe a primeira mensagem para selecionar a operação
                if (strcmp(recvbuf, "cad") == 0)
                {
                    // Cadastrar mensagem
                    if (shared_mem_address->indice == 10)
                    {
                        strcpy(sendbuf, "Numero maximo de mensagens atingido!");
                        if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                        {
                            perror("Send()");
                            exit(7);
                        }
                    }
                    else
                    {
                        if (recv(ns, mensagem_inteira, sizeof(mensagem_inteira), 0) == -1)
                        {
                            perror("Usuariobuf()");
                            exit(6);
                        }
                        else
                        {
                            printf("\n[%d] Mensagem inteira: %s\n", fid, mensagem_inteira);
                            lockSemaphore(semaphore_id_A);
                            strcpy(shared_mem_address->usuarios[shared_mem_address->indice], strtok(mensagem_inteira, "#"));
                            strcpy(shared_mem_address->mensagens[shared_mem_address->indice], strtok('\0', "$$"));

                            printf("[%d] Usuario cadastrado: %s\n", fid, shared_mem_address->usuarios[shared_mem_address->indice]);
                            //strcpy(mensagens[indice], mensagembuf);
                            printf("[%d] Mensagem cadastrada: %s\n", fid, shared_mem_address->mensagens[shared_mem_address->indice]);
                            printf("[%d] Indice: %d\n", fid, shared_mem_address->indice);
                            shared_mem_address->indice++;
                            unlockSemaphore(semaphore_id_A);

                            /* Envia uma mensagem ao cliente através do socket conectado */
                            strcpy(sendbuf, "Mensagem cadastrada com sucesso!");
                            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                            {
                                perror("Send()");
                                exit(7);
                            }
                        }
                    }
                    printf("[%d] Mensagem enviada ao cliente: %s\n", fid, sendbuf);
                }
                if (strcmp(recvbuf, "ler") == 0)
                {
                    // Ler mensagem
                    char qtd_msg[2];

                    sprintf(qtd_msg, "%d", shared_mem_address->indice);

                    strcpy(sendbuf, qtd_msg);
                    printf("[%d] SENDBUF: %s\n", fid, sendbuf);
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }
                    lockSemaphore(semaphore_id_A);

                    for (int i = 0; i < shared_mem_address->indice; i++)
                    {
                        sleep(1);
                        memset(sendbuf, 0, sizeof(sendbuf));
                        strcpy(mensagem_inteira, shared_mem_address->usuarios[i]);
                        strcat(mensagem_inteira, "#");
                        strcat(mensagem_inteira, shared_mem_address->mensagens[i]);
                        strcat(mensagem_inteira, "$$");
                        strcpy(sendbuf, mensagem_inteira);

                        if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                        {
                            perror("Send()");
                            exit(7);
                        }

                        //receber msg confirmação do cliente
                    }
                    unlockSemaphore(semaphore_id_A);
                }
                if (strcmp(recvbuf, "apa") == 0)
                {
                    // Apaga mensagem
                    char nome[20];
                    if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
                    {
                        perror("Recvbuf()");
                        exit(6);
                    }
                    strcpy(nome, recvbuf);
                    strcpy(sendbuf, "Usuario nao encontrado!\n");

                    // MOSTRAR MENSAGENS APAGADAS

                    int msg_apagadas = 0;
                    char k[2];

                    lockSemaphore(semaphore_id_A);

                    for (int i = 0; i < shared_mem_address->indice; i++)
                    {
                        printf("[%d] Nome: %d\n", fid, i);
                        if (strcmp(nome, shared_mem_address->usuarios[i]) == 0)
                        {
                            msg_apagadas++;
                        }
                    }
                    unlockSemaphore(semaphore_id_A);
                    sprintf(k, "%d", msg_apagadas);
                    strcpy(sendbuf, k);

                    printf("[%d] SENDBUF: %s\n", fid, sendbuf);
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }
                    lockSemaphore(semaphore_id_A);

                    for (int i = 0; i < shared_mem_address->indice; i++)
                    {
                        if (strcmp(nome, shared_mem_address->usuarios[i]) == 0)
                        {
                            sleep(1);
                            memset(sendbuf, 0, sizeof(sendbuf));
                            strcpy(mensagem_inteira, shared_mem_address->usuarios[i]);
                            strcat(mensagem_inteira, "#");
                            strcat(mensagem_inteira, shared_mem_address->mensagens[i]);
                            strcat(mensagem_inteira, "$$");
                            strcpy(sendbuf, mensagem_inteira);

                            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                            {
                                perror("Send()");
                                exit(7);
                            }
                        }
                        //receber msg confirmação do cliente
                    }
                    unlockSemaphore(semaphore_id_A);

                    // FIM MOSTRAR MENSAGENS APAGADAS

                    lockSemaphore(semaphore_id_A);
                    for (int i = 0; i < shared_mem_address->indice; i++)
                    {
                        printf("[%d] Nome: %d\n", fid, i);
                        if (strcmp(nome, shared_mem_address->usuarios[i]) == 0)
                        {
                            printf("[%d] Nome %d localizado\n", fid, i);
                            for (int j = i; j < shared_mem_address->indice; j++)
                            {
                                printf("[%d] Usuario %d recebe usuario %d\n", fid, j, j + 1);
                                strcpy(shared_mem_address->usuarios[j], shared_mem_address->usuarios[j + 1]);
                                strcpy(shared_mem_address->mensagens[j], shared_mem_address->mensagens[j + 1]);
                            }
                            shared_mem_address->indice--;
                            printf("[%d] Indice: %d\nI: %d\n", fid, shared_mem_address->indice, i);
                        }
                    }
                    unlockSemaphore(semaphore_id_A);/*
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }*/
                }
                if (strcmp(recvbuf, "out") == 0)
                {
                    close(ns);

                    /* Processo filho termina sua execu��o */
                    printf("[%d] Processo filho terminado com sucesso.\n", fid);
                    exit(0);
                }
                if (strcmp(recvbuf, "stp") == 0)
                {

                    printf("Processo servidor encerrado por:[%d]\n", fid);
                    removeSharedMemory(&shared_mem_id);
                    removeSemaphore(semaphore_id_A);
                    exit(0);
                }
            }
        }
        else
        {
            /*
		 * Processo pai 
		 */

            if (pid > 0)
            {
                printf("Processo filho criado: %d\n", pid);

                /* Fecha o socket conectado ao cliente */
                close(ns);
            }
            else
            {
                perror("Fork()");
                exit(7);
            }
        }
    }
}

// ***************** FUNÇOES EXTRAS
void createSemaphore(int *semaphore_id, int semaphore_key)
{
    // Creating the semaphore
    if ((*semaphore_id = semget(semaphore_key, 1, IPC_CREAT | SEM_PERMS)) == -1)
    {
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }
    // Initializing it as unlocked
    if (semop(*semaphore_id, semaphore_unlock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }
}
void createIntSharedMemory(int *shared_mem_id, int shared_mem_key, int **shared_mem_address)
{
    // Creating the shared memory segment
    if ((*shared_mem_id = shmget(shared_mem_key, sizeof(int), IPC_CREAT | SEM_PERMS)) == -1)
    {
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada. Erro: %s\n", strerror(errno));
        exit(1);
    }
    if ((*shared_mem_address = (int *)shmat(*shared_mem_id, NULL, 0)) == (int *)-1)
    {
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada! Erro: %s\n", strerror(errno));
        exit(1);
    }
    **shared_mem_address = 0;
}
void removeSharedMemory(int shared_mem_id)
{
    if (shmctl(shared_mem_id, IPC_RMID, NULL) != 0)
    {
        fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
        exit(1);
    }
    else
    {
        fprintf(stdout, "\nSegmento de memoria compartilhada removido com sucesso!");
    }
}
void removeSemaphore(int semaphore_id)
{
    if (semctl(semaphore_id, 0, IPC_RMID, 0) != 0)
    {
        fprintf(stderr, "Impossivel remover o conjunto de semaforos! Error: %s\n semaphore id: %d", strerror(errno), semaphore_id);
        exit(1);
    }
    else
    {
        fprintf(stdout, "\nConjunto de semaforos removido com sucesso!");
    }
}
void lockSemaphore(int semaphore_id)
{
#ifdef PROTECT
    if (semop(semaphore_id, semaphore_lock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso! Erro: %s", strerror(errno));
        exit(1);
    }
    else
    {
        //fprintf(stdout, "\n<Locked semaphore with id: %d>\n", semaphore_id);
        //fflush(stdout);
    }
#endif
}
void unlockSemaphore(int semaphore_id)
{
#ifdef PROTECT
    if (semop(semaphore_id, semaphore_unlock_op, 1) == -1)
    {
        fprintf(stdout, "chamada semop() falhou, impossivel liberar o recurso!");
        exit(1);
    }
    else
    {
        //fprintf(stdout, "\n<Unlocked semaphore with id: %d>\n", semaphore_id);
        //fflush(stdout);
    }
#endif
}