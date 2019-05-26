// Ettore Biazon Baccan         16000465
// Mateus Henrique Zorzi        16100661
// Matheus Martins Pupo         16145559
// Murilo Martos Mendonçca      16063497
// Victor Hugo do Nascimento    16100588

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

// Structs
struct contato
{
    char nome[51];
    char telefone[9];
    struct contato *prox;
};

struct grupo
{
    char nome[51];
    struct contato *raiz;
    struct grupo *prox;
};

//variaveis globais
pthread_t thread_id;
int porta;
struct contato *raiz = NULL;

//prototipos
void *servidor();

/*
 * Cliente TCP
 */

main(argc, argv) int argc;
char **argv;
{
    FILE *arquivoContatos;
    FILE *arquivoGrupos;
    srand(time(NULL));
    char telefone[9];
    unsigned short port;
    char sendbuf[101];
    char recvbuf[101];
    char ipAtual[16];
    char portaAtual[5];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    char mensagem[80];
    char nome[20];
    char envio[101]; // + o "#"

    // Variáveis para a opção 3
    char salvarTelefone[9];
    char salvarNome[51];
    // Variaveis para a opcao 1
    char lerNome[51];
    char lerTelefone[9];

    /*
     * O primeiro argumento (argv[1]) � o hostname do servidor.
     * O segundo argumento (argv[2]) � a porta do servidor.
     */
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    /*
     * Obtendo o endere�o IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short)atoi(argv[2]);

    /*
     * Define o endere�o IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conex�o com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
    printf("\nInforme seu zap, rs: ");
    fflush(stdin);
    gets(telefone);
    fflush(stdin);
    if (send(s, telefone, 9, 0) < 0)
    {
        perror("Send()");
        exit(5);
    } //informa ao servidor o numero de telefone

    porta = 5000 + (rand() % 1000);

    //cria uma thread que seria o servidor, que aguarda por conexões
    if (pthread_create(&thread_id, NULL, servidor, (void *)NULL))
    {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
    }
    sleep(1);
    char porta_this_server[5];
    sprintf(porta_this_server, "%d", porta);

    if (send(s, porta_this_server, 5, 0) < 0)
    {
        perror("Send()");
        exit(5);
    } //informa ao servidor o numero de telefone

    int op;
    do
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(mensagem, 0, sizeof(mensagem));
        memset(nome, 0, sizeof(nome));
        char op_rec[10];
        char num_pesquisar[9];
        printf("\nSelecione a opção\n");
        printf("\n1) Usuários");
        printf("\n2) Grupos");
        printf("\n3) Cadastrar usuário");
        printf("\n4) Criar grupo");
        printf("\n5) Sair\n");
        printf("\nOpção: ");

        fflush(stdin);
        gets(op_rec);
        fflush(stdin);

        op = atoi(op_rec);
        //provavelmente um menu com as opcoes (mandar mensagem, criar grupo, etc)
        switch (op)
        {

        case 1:
            // enviar mensagem

            arquivoContatos = fopen("contatos.dat", "rb");
            if (!arquivoContatos)
            {
                printf("\nErro na abertura do arquivo.\n");
            }
            else
            {
                printf("\n");
                int idTemporario = 1;
                while (fread(&lerNome, sizeof(lerNome), 1, arquivoContatos) != NULL)
                {
                    struct contato *novo = (struct contato *)malloc(sizeof(struct contato));
                    struct contato *aux;
                    fread(&lerTelefone, sizeof(lerTelefone), 1, arquivoContatos);
                    strcpy(novo->nome, lerNome);
                    strcpy(novo->telefone, lerTelefone);
                    novo->prox = NULL;
                    printf("[%d] %s\t\t%s\n", idTemporario, novo->nome, novo->telefone);
                    idTemporario++;
                    if (raiz == NULL)
                    {
                        raiz = novo;
                    }
                    else
                    {
                        aux = raiz;
                        while (aux->prox != NULL)
                        {
                            aux = aux->prox;
                        }
                        aux->prox = novo;
                    }
                }
                fclose(arquivoContatos);
            }

            //aqui o cliente esta apenas verificando se o numero
            printf("\nDeseja enviar msg para qual numero: ");
            fflush(stdin);
            gets(num_pesquisar);
            fflush(stdin);

            if (send(s, num_pesquisar, sizeof(num_pesquisar), 0) < 0)
            {
                perror("Send()");
                exit(5);
            }
            int retorno = recv(s, recvbuf, sizeof(recvbuf), 0); //recebe a mensagem do cliente e verifica o valor de retorno

            if (retorno == -1)
            {
                perror("Recvbuf()");
                close(s);
                exit(1);
            }
            else if (retorno == 0)
            {
                printf("thread encerrada pois o cliente foi fechado num momento inesperado\n");
                close(s);
                exit(1);
            }
            if (strcmp("offline", recvbuf) == 0)
            {
                printf("\nCliente offline\n");
            }
            else
            {
                strcpy(ipAtual, strtok(recvbuf, "+"));
                strcpy(portaAtual, strtok(NULL, "$"));

                printf("Ip atual: %s\n", recvbuf);
                printf("Porta atual: %s", portaAtual);

                unsigned short port2;
                char sendbuf2[101];
                struct hostent *hostnm2;
                struct sockaddr_in server2;
                int s2;

                hostnm2 = gethostbyname(ipAtual);
                if (hostnm2 == (struct hostent *)0)
                {
                    fprintf(stderr, "Gethostbyname failed\n");
                    exit(2);
                }
                port2 = (unsigned short)atoi(portaAtual);

                /*
                * Define o endere�o IP e a porta do servidor
                */
                server2.sin_family = AF_INET;
                server2.sin_port = htons(port2);
                server2.sin_addr.s_addr = *((unsigned long *)hostnm2->h_addr);

                /*
                * Cria um socket TCP (stream)
                */
                if ((s2 = socket(PF_INET, SOCK_STREAM, 0)) < 0)
                {
                    perror("Socket()");
                    exit(3);
                }

                /* Estabelece conex�o com o servidor */
                if (connect(s2, (struct sockaddr *)&server2, sizeof(server2)) < 0)
                {
                    perror("Connect()");
                    exit(4);
                }
                char mensagem[100];
                printf("\nMensagem: ");

                fflush(stdin);
                gets(mensagem);
                fflush(stdin);

                printf("Msg enviada: %s", mensagem);
                if (send(s2, mensagem, strlen(mensagem) + 1, 0) < 0)
                {
                    perror("Send()");
                    exit(5);
                } //informa ao servidor o numero de telefone
                close(s2);
            }
            break;

        case 2:
            // envia pra grupo

            break;

        case 3: // Registrar usuário
            printf("Nome do contato: ");
            scanf("%s", &salvarNome);
            printf("Telefone: ");
            scanf("%s", &salvarTelefone);
            arquivoContatos = fopen("contatos.dat", "ab");
            if (!arquivoContatos)
            {
                printf("\nErro na abertura do arquivo.\n");
            }
            else
            {
                fwrite(&salvarNome, sizeof(salvarNome), 1, arquivoContatos);
                fwrite(&salvarTelefone, sizeof(salvarTelefone), 1, arquivoContatos);
                fclose(arquivoContatos);
                printf("\nContato salvo com sucesso!\n");
            }
            break;
        case 4: // Cria grupo
            break;
        case 5: // Sair
            break;
        default:
            printf("Opcao Inválida!\n");
            break;
        }

    } while (op != 5);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
void *servidor()
{
    unsigned short port;

    struct sockaddr_in client;
    struct sockaddr_in server;
    char recvbuf[101];
    int s;  /* Socket para aceitar conex�es       */
    int ns; /* Socket conectado ao cliente        */

    port = (unsigned short)porta;
    //printf("\nPORTA USADA: %d\n", porta);
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
    while (1)
    {
        int namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }
        //receber msg
        int retorno = recv(ns, recvbuf, sizeof(recvbuf), 0); //recebe a mensagem do cliente e verifica o valor de retorno
        if (retorno == -1)
        {
            perror("Recvbuf()");
            close(ns);
            pthread_exit(NULL);
        }
        else if (retorno == 0)
        {
            printf("Thread encerrada pois o cliente foi fechado num momento inesperado\n");
            close(ns);
            pthread_exit(NULL);
        }
        //printa a msg
        printf("RECEBIDO: %s", recvbuf);
        //fecha o socket
        close(ns);
    }
}