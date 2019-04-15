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
pthread_t thread_id;
float maior_temp = 0; //para armazenar a maior temp
int count_servers = 0;
pthread_mutex_t mutex;
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
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }
    port = (unsigned short)atoi(argv[1]);
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    printf("*** Servidor Iniciado! ***\n");
    while (1)
    {
        int namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }
        printf("Novo cliente!\n");
        printf("IP: %lu\nPORTA: %u", client.sin_addr.s_addr, client.sin_port);

        if (pthread_create(&thread_id, NULL, servidor, (void *)ns))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
        count_servers++;
        pthread_detach(thread_id);
    }

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}

void *servidor(int ns)
{
    char sendbuf[101];
    char recvbuf[101];
    int id_this_thread = count_servers; //no momento que a função é chamada, recebe o count pra saber qual o "id" dela
    float temp_recebida = 0;
    int retorno = 0;
    /*
    O servidor deve aguardar por requisições de conexão enviadas pelos clientes
    e quando um novo cliente seconectar deve exibir uma mensagem informando o 
    endereço IP e a porta do cliente que solicitou a conexão.

    tem que exibir o ip do cliente e a porta que ele conectou
    */

    printf("\n[%d] Thread criada com sucesso\n", id_this_thread);
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf)); //zera as variaveis
        memset(sendbuf, 0, sizeof(sendbuf));

        retorno = recv(ns, recvbuf, sizeof(recvbuf), 0); //recece a mensagem do cliente e verifica o valor de retorno
        if (retorno == -1)
        {
            perror("Recvbuf()");
            close(ns);
            pthread_exit(NULL);
        }
        else if (retorno == 0)
        {
            printf("thread encerrada pois o cliente foi fechado num momento inesperado\n");
            close(ns);
            pthread_exit(NULL);
        }
        retorno = 0;
        printf("\n[%d] Temperatura recebida do cliente: %s\n", id_this_thread, recvbuf);

        temp_recebida = atof(recvbuf); //atof é pra float, atoi é pra int, vamos usar float

        if (temp_recebida >= maior_temp) //se ele for a maior temp, salva como maior temp e manda 1
        {
            pthread_mutex_lock(&mutex);
            maior_temp = temp_recebida;
            //envia 1 para ligar led
            strcpy(sendbuf, "1");
            retorno = send(ns, sendbuf, strlen(sendbuf) + 1, 0);
            if (retorno == -1)
            {
                perror("Send()");
                close(ns);
                pthread_exit(NULL);
            }
            if (retorno == 0)
            {
                perror("Send()");
                close(ns);
                pthread_exit(NULL);
            }
            retorno = 0;
            pthread_mutex_unlock(&mutex);
            printf("\n[%d] Ligando led\n", id_this_thread);
        }
        else //se ele n for a maior temp, so apaga o led
        {
            //se ele n for a maior temp, apaga led enviando 0
            strcpy(sendbuf, "0");
            retorno = send(ns, sendbuf, strlen(sendbuf) + 1, 0);
            if (retorno == -1)
            {
                perror("Send()");
                close(ns);
                pthread_exit(NULL);
            }
            if (retorno == 0)
            {
                perror("Send()");
                close(ns);
                pthread_exit(NULL);
            }
            retorno = 0;
            
            printf("\n[%d] Apagando led\n", id_this_thread);
        }
    }
}
