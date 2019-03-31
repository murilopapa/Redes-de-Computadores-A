// Ettore Biazon Baccan         16000465
// Mateus Henrique Zorzi        16100661
// Matheus Martins Pupo         16145559
// Murilo Martos Mendonçca      16063497
// Victor Hugo do Nascimento    16100588

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

//prototipos
void *servidor(int ns);

//variaveis globais, compartilhadas pelas threads
pthread_t servidores[50];
char usuarios[10][20];  // 0 a 9 usuarios e o décimo é /0
char mensagens[10][80]; // 0 a 9 mensagens e a décima é /0
int indice = 0;         // variavel para contar a quantidade de mensagens
int count_servers = 0;
/*
 * Servidor TCP
 */
main(argc, argv) int argc;
char **argv;
{
    unsigned short port;

    struct sockaddr_in client;
    struct sockaddr_in server;
    int s;  /* Socket para aceitar conex�es       */
    int ns; /* Socket conectado ao cliente        */
    int namelen;

    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

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
    * Define a qual endereço IP e porta o servidor estará ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereços IP
    */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    printf("*** Servidor Iniciado! ***\n");

    /*
     * Aceita uma conex�o e cria um novo socket atrav�s do qual
     * ocorrer� a comunica��o com o cliente.
     */

    while (1)
    {
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        if (pthread_create(&servidores[count_servers], NULL, servidor, (void *)ns))
        {
            printf("ERRO: impossivel criar um thread consumidor\n");
            exit(-1);
        }
        count_servers++;
    }
    /* Fecha o socket conectado ao cliente */
    close(ns);
    /* Fecha o socket aguardando por conex�es */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}

void *servidor(int ns)
{
    char sendbuf[101];
    char recvbuf[101];
    char mensagembuf[80]; // mensagem que recebe na função
    char usuariobuf[20];  // usuario que recebe na função
    char mensagem_inteira[101];
    int id_this_thread = count_servers; //no momento que a função é chamada, recebe o count pra saber qual o "id" dela

    printf("\n[%d] Thread criada com sucesso\n", id_this_thread);
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
        printf("\n[%d] Mensagem recebida do cliente: %s\n", id_this_thread, recvbuf);

        // Recebe a primeira mensagem para selecionar a operação
        if (strcmp(recvbuf, "cad") == 0)
        {
            // Cadastrar mensagem
            if (indice == 10)
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
                    printf("\n[%d] Mensagem inteira: %s\n", id_this_thread, mensagem_inteira);

                    strcpy(usuarios[indice], strtok(mensagem_inteira, "#"));
                    strcpy(mensagens[indice], strtok('\0', "$$"));

                    printf("[%d] Usuario cadastrado: %s\n", id_this_thread, usuarios[indice]);
                    //strcpy(mensagens[indice], mensagembuf);
                    printf("[%d] Mensagem cadastrada: %s\n", id_this_thread, mensagens[indice]);
                    printf("[%d] Indice: %d\n", id_this_thread, indice);
                    indice++;

                    /* Envia uma mensagem ao cliente através do socket conectado */
                    strcpy(sendbuf, "Mensagem cadastrada com sucesso!");
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }
                }
            }
            printf("[%d] Mensagem enviada ao cliente: %s\n", id_this_thread, sendbuf);
        }
        if (strcmp(recvbuf, "ler") == 0)
        {
            // Ler mensagem
            char qtd_msg[2];
            sprintf(qtd_msg, "%d", indice);
            strcpy(sendbuf, qtd_msg);
            printf("[%d] SENDBUF: %s\n", id_this_thread, sendbuf);
            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
            {
                perror("Send()");
                exit(7);
            }
            for (int i = 0; i < indice; i++)
            {
                memset(sendbuf, 0, sizeof(sendbuf));
                strcpy(mensagem_inteira, usuarios[i]);
                strcat(mensagem_inteira, "#");
                strcat(mensagem_inteira, mensagens[i]);
                strcat(mensagem_inteira, "$$");
                strcpy(sendbuf, mensagem_inteira);

                if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                {
                    perror("Send()");
                    exit(7);
                }
                if (recv(ns, mensagem_inteira, sizeof(mensagem_inteira), 0) == -1)
                {
                    perror("Usuariobuf()");
                    exit(6);
                }

                //receber msg confirmação do cliente
            }
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
            for (int i = 0; i < indice; i++)
            {
                printf("[%d] Nome: %d\n", id_this_thread, i);
                if (strcmp(nome, usuarios[i]) == 0)
                {
                    printf("[%d] Nome %d localizado\n", id_this_thread, i);
                    for (int j = i; j < indice; j++)
                    {
                        printf("[%d] Usuario %d recebe usuario %d\n", id_this_thread, j, j + 1);
                        strcpy(usuarios[j], usuarios[j + 1]);
                        strcpy(mensagens[j], mensagens[j + 1]);
                    }
                    indice--;
                    printf("[%d] Indice: %d\nI: %d\n", id_this_thread, indice, i);
                    strcpy(sendbuf, "Usuario e mensagem apagado com sucesso!\n");
                }
            }
            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
            {
                perror("Send()");
                exit(7);
            }
        }
        if (strcmp(recvbuf, "out") == 0)
        {
            pthread_exit(NULL);
        }
    }
}