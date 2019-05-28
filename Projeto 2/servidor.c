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
#include <arpa/inet.h>
#define qtdTelefone 10

//necessario salvar de alguma maneira os ips e portas dos cliente.
struct cliente
{
    unsigned short porta;
    char *ip;
    char telefone[qtdTelefone];
    int id;
    struct cliente *prox;
};

//prototipos
//obs, os que manipulam a lista, nao recebem ela por parametro pq ela é var global
void *servidor(int ns);
int inserir_cliente(char *telefone, unsigned short porta, char *ip);
void remover_cliente(char *telefone);

//variaveis globais, compartilhadas pelas threads
pthread_t thread_id;
int count_servers = 0;
pthread_mutex_t mutex;
struct cliente *raiz = NULL; //cria o primeiro elemento da lista dos cliente conectados
/*
 * Servidor TCP
 */
main(argc, argv) int argc;
char **argv;
{
    unsigned short port;
    struct sockaddr_in client;
    struct sockaddr_in server;
    char recvbuf[101];
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

        //receber telefone
        int retorno = recv(ns, recvbuf, qtdTelefone, 0); //recebe a mensagem do cliente e verifica o valor de retorno
        if (retorno == -1)
        {
            perror("Recvbuf()");
            close(ns);
            exit(5);
        }
        else if (retorno == 0)
        {
            printf("Thread encerrada pois o cliente foi fechado num momento inesperado\n");
            close(ns);
            exit(5);
        }
        char telefone[10];
        strcpy(telefone, recvbuf);

        retorno = recv(ns, recvbuf, 5, 0); //recebe a mensagem do cliente e verifica o valor de retorno
        if (retorno == -1)
        {
            perror("Recvbuf()");
            close(ns);
            exit(5);
        }
        else if (retorno == 0)
        {
            printf("Thread encerrada pois o cliente foi fechado num momento inesperado\n");
            close(ns);
            exit(5);
        }

        int porta = atoi(recvbuf);
        unsigned short porta_cliente = (unsigned short)porta;

        char *ip_consultado;
        ip_consultado = inet_ntoa(client.sin_addr);

        pthread_mutex_lock(&mutex);
        if (inserir_cliente(telefone, porta_cliente, ip_consultado) == -1)
        {
            printf("ERRO: não foi possivel incluir o elemento na lista ligada.\n");
            //tratar melhor esse erro
        }
        pthread_mutex_unlock(&mutex);

        if (pthread_create(&thread_id, NULL, servidor, (void *)ns))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
    }

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}

void *servidor(int ns)
{
    struct cliente *atual, *localizado, *this_client;
    int retorno;
    char telefone_recebido[qtdTelefone];
    char sendbuf[101];
    char recvbuf[101];
    int id_this_thread = count_servers; //no momento que a função é chamada, recebe o count pra saber qual o "id" dela
    count_servers++;

    atual = raiz;
    while (atual->prox != NULL)
    {
        if (atual->id == id_this_thread) //encontrado o telefone procurado
        {
            this_client = atual;
        }
        atual = atual->prox;
    }
    if (atual->id == id_this_thread) //encontrado o telefone procurado
    {
        this_client = atual;
    }
    printf("\n[%s] Thread criada com sucesso\n", this_client->telefone);
    while (1)
    {
        //a msg que recebe, é o telefone que quer verificar
        retorno = recv(ns, recvbuf, sizeof(recvbuf), 0); //recebe a mensagem do cliente e verifica o valor de retorno
        if (retorno == -1)
        {
            perror("Recvbuf()");
            remover_cliente(this_client->telefone);
            close(ns);
            pthread_exit(NULL);
        }
        else if (retorno == 0)
        {
            printf("[%s] Thread encerrada pois o cliente foi fechado num momento inesperado\n", this_client->telefone);
            remover_cliente(this_client->telefone);
            close(ns);
            pthread_exit(NULL);
        }
        //percorrer o vetor todo, e ver se o cliente solicitado esta online
        int online = 0;
        atual = raiz;

        while (atual->prox != NULL)
        {
            if (strcmp(atual->telefone, recvbuf) == 0) //encontrado o telefone procurado
            {
                localizado = atual;
                online = 1;
            }

            atual = atual->prox;
        }
        if (strcmp(atual->telefone, recvbuf) == 0) //encontrado o telefone procurado
        {
            localizado = atual;
            online = 1;
        }

        printf("\n[%s] Verificar telefone: %s\n", this_client->telefone, recvbuf);
        printf("\n[%s] Online = %d\n", this_client->telefone, online);
        if (online == 1)
        {
            //enviar ao cliente a mensagem contendo o ip e porta que se encontram na struct cliente atual
            sprintf(sendbuf, "%s+%u$", localizado->ip, localizado->porta); //resultado: ip+porta ex: "192.168.0.1+450"
            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
            {
                perror("Send()");
                remover_cliente(this_client->telefone);
                close(ns);
                pthread_exit(NULL);
            } //informa ao servidor o ip e porta
        }
        else
        {
            if (send(ns, "offline", 7, 0) < 0)
            {
                perror("Send()");
                remover_cliente(this_client->telefone);
                close(ns);
                pthread_exit(NULL);
            }
        }
    }
}

int inserir_cliente(char *telefone, unsigned short porta, char *ip)
{

    struct cliente *novo;
    struct cliente *atual;
    //instancia o novo cliente
    novo = (struct cliente *)malloc(sizeof(struct cliente));
    if (novo == NULL)
    {
        printf("Falta Memoria\n");
        return -1;
    }
    //preenche o novo cliente
    strcpy(novo->telefone, telefone);
    novo->ip = ip;
    novo->porta = porta;
    novo->prox = NULL;
    novo->id = count_servers;

    if (raiz == NULL) //Lista vazia
    {
        printf("\nprimeiro cadastro\n");
        raiz = novo;
    }
    else
    {
        printf("\nn eh primeiro cadastro\n");
        atual = raiz; /*@ Primeiro elemento*/
        while (atual->prox != NULL)
        {
            atual = atual->prox;
        }
        atual->prox = novo;
    }
    printf("\nINSERIDO CLIENTE:\n");
    printf("NUMERO: %s\n", novo->telefone);
    printf("IP: %s\n", novo->ip);
    printf("PORTA: %u\n", novo->porta);
    printf("ID: %u\n\n", novo->id);

    //teste pra printar a lista toda
    novo = raiz;

    if (novo->prox == NULL) //1 cliente so
    {
        printf("\nCLIENTES NA LISTA:\n");
        printf("NUMERO: %s\n", novo->telefone);
        printf("IP: %s\n", novo->ip);
        printf("PORTA: %u\n", novo->porta);
        printf("ID: %u\n\n", novo->id);
    }
    else
    {
        printf("\nCLIENTES NA LISTA:\n");
        while (novo->prox != NULL)
        {
            printf("\n----------------------------\n");
            printf("NUMERO: %s\n", novo->telefone);
            printf("IP: %s\n", novo->ip);
            printf("PORTA: %u\n", novo->porta);
            printf("ID: %u\n\n", novo->id);
            novo = novo->prox;
        }
        printf("\n----------------------------\n");
        printf("NUMERO: %s\n", novo->telefone);
        printf("IP: %s\n", novo->ip);
        printf("PORTA: %u\n", novo->porta);
        printf("ID: %u\n\n", novo->id);
    }
    //fim do teste

    return 1;
}

void remover_cliente(char *telefone)
{
    if (raiz == NULL)
    {
        printf("A lista esta vazia\n");
    }
    else
    {
        struct cliente *atual, *anterior;
        anterior = NULL;
        atual = raiz;
        while (strcmp(atual->telefone, telefone) != 0) //talvez usar strcmp?
        {
            anterior = atual;
            atual = atual->prox;
        }
        if (anterior == NULL) //pq só tem 1 elemento
        {
            raiz = NULL;
        }
        else
        {
            anterior->prox = atual->prox;
        }
        printf("REMOVIDO:\n");
        printf("NUMERO: %s\n", atual->telefone);
        printf("IP: %s\n", atual->ip);
        printf("PORTA: %u\n", atual->porta);
        printf("ID: %u\n\n", atual->id);
        free(atual);
    }
}
