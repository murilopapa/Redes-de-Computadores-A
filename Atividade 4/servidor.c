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

/*
 * Servidor TCP
 */
//variaveis globais
pthread_t servidores[50];
int count_servers = 0;
unsigned short port;
char sendbuf[101];
char recvbuf[101];
struct sockaddr_in client;
struct sockaddr_in server;
int s;  /* Socket para aceitar conex�es       */
int ns; /* Socket conectado ao cliente        */
int namelen;

int indice = 0;       // variavel para contar a quantidade de mensagens
char mensagembuf[80]; // mensagem que recebe na função
char usuariobuf[20];  // usuario que recebe na função

char usuarios[10][20];  // 0 a 9 usuarios e o décimo é /0
char mensagens[10][80]; // 0 a 9 mensagens e a décima é /0

char mensagem_inteira[101];

main(argc, argv) int argc;
char **argv;
{

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
        if (pthread_create(&servidores[count_servers], NULL, servidor, (void *)count_servers + 1))
        {
            printf("ERRO: impossivel criar um thread consumidor\n");
            exit(-1);
        }
        //criar a thread aqui
    }
    /* Fecha o socket conectado ao cliente */
    close(ns);

    /* Fecha o socket aguardando por conex�es */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}

void *servidor()
{

    close(s);
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
        printf("\nMensagem recebida do cliente: %s\n", recvbuf);

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
                    printf("\nMensagem inteira: %s\n", mensagem_inteira);

                    strcpy(usuarios[indice], strtok(mensagem_inteira, "#"));
                    strcpy(mensagens[indice], strtok('\0', "$$"));

                    printf("Usuario cadastrado: %s\n", usuarios[indice]);
                    //strcpy(mensagens[indice], mensagembuf);
                    printf("Mensagem cadastrada: %s\n", mensagens[indice]);
                    printf("Indice: %d\n", indice);
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
            printf("Mensagem enviada ao cliente: %s\n", sendbuf);
        }
        if (strcmp(recvbuf, "ler") == 0)
        {
            // Ler mensagem
            char qtd_msg[2];
            sprintf(qtd_msg, "%d", indice);
            strcpy(sendbuf, qtd_msg);
            printf("SENDBUF: %s\n", sendbuf);
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
                printf("Nome: %d\n", i);
                if (strcmp(nome, usuarios[i]) == 0)
                {
                    printf("Nome %d localizado\n", i);
                    for (int j = i; j < indice; j++)
                    {
                        printf("Usuario %d recebe usuario %d\n", j, j + 1);
                        strcpy(usuarios[j], usuarios[j + 1]);
                        strcpy(mensagens[j], mensagens[j + 1]);
                    }
                    indice--;
                    printf("Indice: %d\nI: %d\n", indice, i);
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
            close(ns);
            namelen = sizeof(client);
            if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
            {
                perror("Accept()");
                exit(5);
            }
        }
    }
}