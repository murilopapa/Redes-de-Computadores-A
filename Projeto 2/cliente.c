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
#define qtdTelefone 10
#define qtdNome 51
#define tamanhoBuffer 101

// Structs
struct contato
{
    char nome[qtdNome];
    char telefone[qtdTelefone];
    struct contato *prox;
};

struct grupo
{
    char nome[qtdNome];
    struct contato *raiz;
    struct grupo *prox;
};

struct mensagem
{
    char mensagem[200];
    char telefone[qtdTelefone];
    char nome[qtdNome];
    int leitura;
    struct mensagem *prox;
};

//variaveis globais
pthread_t thread_id;
int porta;
pthread_mutex_t mutex;
struct contato *raiz_contato = NULL;
struct grupo *raiz_grupo = NULL;
struct mensagem *raiz_mensagem = NULL;

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
    char telefone[qtdTelefone];
    unsigned short port;
    char sendbuf[tamanhoBuffer];
    char recvbuf[tamanhoBuffer];
    char ipAtual[16];
    char portaAtual[5];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    char mensagem[80];
    char nome[20];
    char envio[tamanhoBuffer]; // + o "#"

    // Variáveis para a opção 3
    char salvarTelefone[qtdTelefone];
    char salvarNome[qtdNome];
    // Variaveis para a opcao 1
    char lerNome[qtdNome];
    char lerTelefone[qtdTelefone];

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
    if (send(s, telefone, qtdTelefone, 0) < 0)
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

    char nome_arquivo_contatos[50];
    strcpy(nome_arquivo_contatos, "contatos_");
    strcat(nome_arquivo_contatos, telefone);
    strcat(nome_arquivo_contatos, ".dat");

    arquivoContatos = fopen(nome_arquivo_contatos, "ab");
    if (!arquivoContatos)
    {
        printf("\nErro na abertura do arquivo.\n");
    }
    else
    {
        fclose(arquivoContatos);
    }

    arquivoContatos = fopen(nome_arquivo_contatos, "rb");
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
            if (raiz_contato == NULL)
            {
                raiz_contato = novo;
            }
            else
            {
                aux = raiz_contato;
                while (aux->prox != NULL)
                {
                    aux = aux->prox;
                }
                aux->prox = novo;
            }
        }
        fclose(arquivoContatos);
    }

    int op;
    do
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(mensagem, 0, sizeof(mensagem));
        memset(nome, 0, sizeof(nome));
        char op_rec[qtdTelefone];
        char num_pesquisar[qtdTelefone];
        printf("\nSelecione a opção\n");
        printf("\n1) Usuários");
        printf("\n2) Grupos");
        printf("\n3) Cadastrar usuário");
        printf("\n4) Criar grupo");
        printf("\n5) Ler mensagens");
        printf("\n6) Sair\n");
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

            //aqui o cliente esta apenas verificando se o numero
            printf("\nDeseja enviar msg para qual numero?\n");
            struct contato *aux1;
            int count = 0;
            aux1 = raiz_contato;
            if (aux1 == NULL)
            {
                printf("\nSem contatos salvos!\n");
            }
            else
            {
                while (aux1->prox != NULL)
                {
                    printf("%d) NOME: %s - TELEFONE: %s\n", count, aux1->nome, aux1->telefone);
                    aux1 = aux1->prox;
                    count++;
                }
                printf("%d) NOME: %s - TELEFONE: %s\n", count, aux1->nome, aux1->telefone);

                char opcao[10];
                int opcao_int = 0;

                do
                {
                    printf("\nOpcao: ");
                    fflush(stdin);
                    gets(opcao);
                    fflush(stdin);

                    opcao_int = atoi(opcao);
                } while (opcao_int > count);

                aux1 = raiz_contato;

                for (int i = 0; i < opcao_int; i++)
                {
                    aux1 = aux1->prox;
                }

                strcpy(num_pesquisar, aux1->telefone);
                //printf("Mandando msg para: %s\n", num_pesquisar);

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

                    //printf("Ip atual: %s\n", recvbuf);
                    //printf("Porta atual: %s", portaAtual);

                    unsigned short port2;
                    char sendbuf2[tamanhoBuffer];
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
                    char mensagem[100];// nao eh 101? Se sim, substituir por tamanhoBuffer
                    printf("\nMensagem: ");

                    fflush(stdin);
                    gets(mensagem);
                    fflush(stdin);

                    //printf("Msg enviada: %s", mensagem);
                    strcat(mensagem, "&");
                    strcat(mensagem, telefone);
                    strcat(mensagem, "$");
                    if (send(s2, mensagem, strlen(mensagem) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(5);
                    } //informa ao servidor o numero de telefone
                    close(s2);
                }
            }
            break;

        case 2:
            // envia pra grupo

            break;

        case 3: // Registrar usuário

            printf("Nome do contato: ");
            fflush(stdin);
            gets(salvarNome);
            fflush(stdin);
            printf("Telefone: ");
            fflush(stdin);
            gets(salvarTelefone);
            fflush(stdin);

            struct contato *aux_contato;
            struct contato *novo_contato = (struct contato *)malloc(sizeof(struct contato));

            strcpy(novo_contato->nome, salvarNome);
            novo_contato->prox = NULL;
            strcpy(novo_contato->telefone, salvarTelefone);
            if (raiz_contato == NULL)
            {
                raiz_contato = novo_contato;
            }
            else
            {
                aux_contato = raiz_contato;
                while (aux_contato->prox)
                {
                    aux_contato = aux_contato->prox;
                }
                aux_contato->prox = novo_contato;
            }
            arquivoContatos = fopen(nome_arquivo_contatos, "ab");
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
        case 5: // Ler msg
            printf("\n TODAS AS MENSAGENS\n");
            struct mensagem *aux = raiz_mensagem;
            if (aux == NULL)
            {
                printf("NENHUMA MENSAGEM\n");
            }
            else
            {
                while (aux->prox != NULL)
                {
                    if (aux->leitura == 0)
                    {
                        if (strcmp(aux->nome, "") == 0)
                        {
                            printf("*NOVA*[%s] - %s\n", aux->nome, aux->mensagem);
                        }
                        else
                        {
                            printf("*NOVA*[%s] - %s\n", aux->nome, aux->mensagem);
                        }
                        aux->leitura = 1;
                        aux = aux->prox;
                    }
                    else
                    {
                        if (strcmp(aux->nome, "") == 0)
                        {
                            printf("[%s] - %s\n", aux->nome, aux->mensagem);
                        }
                        else
                        {
                            printf("[%s] - %s\n", aux->nome, aux->mensagem);
                        }
                        aux = aux->prox;
                    }
                }
                if (aux->leitura == 0)
                {
                    printf("*NOVA*[%s] - %s\n", aux->nome, aux->mensagem);
                    aux->leitura = 1;
                }
                else
                {
                    printf("[%s] - %s\n", aux->nome, aux->mensagem);
                }
            }
            break;
        case 6: // Sair
            break;
        default:
            printf("Opcao Inválida!\n");
            break;
        }

    } while (op != 6);

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
    char recvbuf[tamanhoBuffer];
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
        struct mensagem *novo = (struct mensagem *)malloc(sizeof(struct mensagem));
        struct mensagem *aux;
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

        strcpy(novo->mensagem, strtok(recvbuf, "&"));
        strcpy(novo->telefone, strtok(NULL, "$"));
        novo->leitura = 0;
        novo->prox = NULL;
        strcpy(novo->nome, "");
        struct contato *aux_contato;
        aux_contato = raiz_contato;
        if (aux_contato != NULL)
        {
            while (aux_contato->prox != NULL)
            {
                if (strcmp(aux_contato->telefone, novo->telefone) == 0)
                {
                    strcpy(novo->nome, aux_contato->nome);
                }
                aux_contato = aux_contato->prox;
            }
            if (strcmp(aux_contato->telefone, novo->telefone) == 0)
            {
                strcpy(novo->nome, aux_contato->nome);
            }
        }

        if (strcmp(novo->nome, "") == 0)
        {
            printf("\n[%s] - %s\n", novo->nome, novo->mensagem);
            printf("Opção: ");
        }
        else
        {
            printf("\n[%s] - %s\n", novo->nome, novo->mensagem);
            printf("Opção: ");
        }

        if (raiz_mensagem == NULL)
        {
            raiz_mensagem = novo;
        }
        else
        {
            aux = raiz_mensagem;
            while (aux->prox != NULL)
            {
                aux = aux->prox;
            }
            aux->prox = novo;
        }
        //fecha o socket
        close(ns);
    }
}