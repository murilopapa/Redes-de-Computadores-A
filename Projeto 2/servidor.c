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
int inserir_no_fim(struct clientes **raiz, char *telefone,  unsigned short porta, long unsigned int ip);
void remover_no_fim(struct clientes **raiz);

//variaveis globais, compartilhadas pelas threads
pthread_t thread_id;
int count_servers = 0;
pthread_mutex_t mutex;


//necessario salvar de alguma maneira os ips e portas dos clientes.
struct clientes{
    unsigned short porta;
    long unsigned int ip;
    char telefone [9];
    struct clientes_conectados *prox;
};

/*
 * Servidor TCP
 */
main(argc, argv) int argc;
char **argv;
{   
    struct clientes *raiz = null;  //cria o primeiro elemento da lista dos clientes conectados
    raiz = (struct clientes *) malloc(sizeof(struct clientes));
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

        //salvar na lista ligada.
        if (inserir_no_fim(*raiz, telefone, client.sin_port, client.sin_addr) == -1)
        {
            printf("ERRO: não foi possivel incluir o elemento na lista ligada.\n");
        }

        if (pthread_create(&thread_id, NULL, servidor, (void *)ns))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
        count_servers++;
        //pthread_detach(thread_id);
    }

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}

void *servidor(int ns)
{   
    struct clientes *atual;
    int retorno;
    char telefone_recebido[9];
    char sendbuf[101];
    char recvbuf[101];
    int id_this_thread = count_servers; //no momento que a função é chamada, recebe o count pra saber qual o "id" dela

    /*
    O servidor deve aguardar por requisições de conexão enviadas pelos clientes, os clientes informarao o numero para o qual eles desejam saber o ip e porta.
    O servidor responderá com o Ip e porta.
    */

    printf("\n[%d] Thread criada com sucesso\n", id_this_thread);
    while (1)
    {
        retorno = recv(ns, recvbuf, sizeof(recvbuf), 0); //recebe a mensagem do cliente e verifica o valor de retorno
        
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

        //percorre a lista para verificar se o cliente dono do telefone recebido se encontra online.
        *atual = *raiz;
        while(atual-> prox =! null)
        {
            if(strcmp(atual->telefone, telefone_recebido))  //encontrado o telefone procurado
            {   
                
            }
            else
            {
                atual = atual->prox;
            }
        }

        //enviar ao cliente a mensagem contendo o ip e porta que se encontram na struct clientes atual
        sprintf(sendbuf, "%lu+%u", atual->ip, atual->porta);    //resultado: ip+porta ex: "192.168.0.1+450"
        if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
        {
            perror("Send()");
                exit(5);
        }//informa ao servidor o ip e porta

    }//talvez seja necessario garantir a sincronia aqui, caso dois clientes tentem conectar com o servidor ao mesmo tempo, ou caso algum cliente se disconecte
}

int inserir_no_fim(struct clientes **raiz, char *telefone,  unsigned short porta, long unsigned int ip)
{
    struct clientes *novo; 
    struct clientes *atual;

    novo = (struct clientes *) malloc(sizeof(struct No));
    if(novo == NULL){
        printf("Falta Memoria\n");
        return -1;
    }
    novo->telefone = telefone;
    novo->ip = ip;
    novo->porta = porta;
    novo->p_prox = NULL;
    
    if(*raiz == NULL)  //Lista vazia
    {
        *raiz = novo;
    }
    else
    {
        atual = *raiz;   /*@ Primeiro elemento*/ 
        while(atual->prox != NULL){
            atual = atual->p_prox;
        }
        atual->prox = novo;
    }
}

void remover_no_fim(struct clientes **raiz){
    if(*raiz == NULL)
    {
        printf("A lista ja esta vazia\n");
    }
    else
    {
        struct clientes atual, anterior;
        atual = *raiz;
        while(p_atual->p_prox != NULL)
        {
            p_anterior = p_atual ;
            p_atual    = p_atual->p_prox;
        }
        anterior->prox = NULL;
        free(atual);
    }
}


