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
    char *mensagem;
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
    FILE *foto;            //arquivo para enviar foto
    FILE *foto_convertida; //arquivo para receber foto
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
    //Variaveis para a opcao 4
    char salvarNomeGrupo[qtdNome];
    char lerNomeGrupo[qtdNome];

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
    char nome_arquivo_grupos[50];
    strcpy(nome_arquivo_contatos, "contatos_");
    strcat(nome_arquivo_contatos, telefone);
    strcat(nome_arquivo_contatos, ".dat");

    strcpy(nome_arquivo_grupos, "grupos_");
    strcat(nome_arquivo_grupos, telefone);
    strcat(nome_arquivo_grupos, ".dat");

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

    // ABRE ARQUIVO GRUPO
    arquivoGrupos = fopen(nome_arquivo_grupos, "ab");
    if (!arquivoGrupos)
    {
        printf("\nErro na abertura do arquivo.\n");
    }
    else
    {
        fclose(arquivoGrupos);
    }

    arquivoGrupos = fopen(nome_arquivo_grupos, "rb");
    if (!arquivoGrupos)
    {
        printf("\nErro na abertura do arquivo.\n");
    }
    else
    {
        printf("\n");
        struct grupo *aux_grupo5;
        struct grupo abre_arquivo;
        struct contato abre_arquivo1;
        struct contato aux_arquivo;
        int auxQtd2 = 0;
        while (fread(&salvarNomeGrupo, sizeof(salvarNomeGrupo), 1, arquivoGrupos) != 0)
        {
            struct grupo *novo_grupo_arquivo = (struct grupo *)malloc(sizeof(struct grupo));
            novo_grupo_arquivo->prox = NULL;
            novo_grupo_arquivo->raiz = NULL;
            strcpy(novo_grupo_arquivo->nome, salvarNomeGrupo);
            if (raiz_grupo == NULL)
            {
                raiz_grupo = novo_grupo_arquivo;
            }
            else
            {
                aux_grupo5 = raiz_grupo;
                while (aux_grupo5->prox != NULL)
                {
                    aux_grupo5 = aux_grupo5->prox;
                }
                aux_grupo5->prox = novo_grupo_arquivo;
                novo_grupo_arquivo->prox = NULL;
            }
            fread(&auxQtd2, sizeof(auxQtd2), 1, arquivoGrupos);
            for (int i = 0; i < auxQtd2; i++)
            {
                fread(&aux_arquivo, sizeof(aux_arquivo), 1, arquivoGrupos);
                struct contato *copia_contato1 = (struct contato *)malloc(sizeof(struct contato));
                struct grupo *aux_grupo3;
                struct contato *aux_loc3;
                strcpy(salvarNome, aux_arquivo.nome);
                strcpy(copia_contato1->nome, salvarNome);
                strcpy(salvarTelefone, aux_arquivo.telefone);
                strcpy(copia_contato1->telefone, salvarTelefone);
                copia_contato1->prox = NULL;

                if (novo_grupo_arquivo->raiz == NULL)
                {
                    novo_grupo_arquivo->raiz = copia_contato1;
                    aux_loc3 = copia_contato1;
                }
                else
                {
                    aux_grupo3 = novo_grupo_arquivo;
                    while (aux_grupo3->raiz->prox != NULL)
                    {
                        aux_grupo3->raiz = aux_grupo3->raiz->prox;
                    }
                    aux_grupo3->raiz->prox = copia_contato1;

                    novo_grupo_arquivo->raiz = aux_loc3;
                }
            }
        }
        fclose(arquivoGrupos);
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
                int tam;
                char cabecalho[115];
                char cabecalho_foto[15];
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
                //printf("num recebido\n");
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
                    //printf("recebeu ok o ip e porta\n");
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
                    char mensagem[200]; // nao eh 101? Se sim, substituir por tamanhoBuffer
                    int op_msg = 0;
                    //printf("conexao ok\n");
                    do
                    {
                        printf("Deseja enviar: \n 1 - Texto\n 2 - Imagem\n");
                        scanf("%d", &op_msg);
                    } while (op_msg < 1 || op_msg > 2);

                    if (op_msg == 1) //enviando texto
                    {

                        strcpy(cabecalho, "&txt&");
                        //printf("Msg enviada: %s", mensagem);
                        strcpy(mensagem, cabecalho); //insere o cabecalho na msg NAO ta funcionando (talvez seja o nome mensagem o prob)
                        printf("\nMensagem: ");

                        fflush(stdin);
                        gets(mensagem); //passando reto nao sei pq
                        gets(mensagem);
                        fflush(stdin);

                        tam = strlen(mensagem);
                        char tamChar[20];
                        sprintf(tamChar, "%d", tam);

                        //printf("conteudo msg: %s\n", mensagem);
                        //strcat(cabecalho, tamChar);
                        //strcat(cabecalho, "&");
                        strcat(cabecalho, mensagem);
                        strcat(cabecalho, "&");
                        strcat(cabecalho, telefone);
                        strcat(cabecalho, "$"); //modelo: "txt&178&ola, bom dia&9999999999$"
                        int auxLen, auxLenTrue;
                        char charLen[10], *cabecalhoTrue;
                        auxLen = strlen(cabecalho);
                        sprintf(charLen, "%d", auxLen);
                        auxLenTrue = strlen(charLen) + auxLen;
                        sprintf(cabecalhoTrue, "%d", auxLenTrue);
                        strcat(cabecalhoTrue, cabecalho);
                        printf("\nTrue: %d\n", auxLenTrue);
                        printf("\ncabecalho: %s\n", cabecalhoTrue);
                        if (send(s2, cabecalhoTrue, strlen(cabecalhoTrue) + 1, 0) < 0)
                        {
                            perror("Send()");
                            exit(5);
                        } //informa ao servidor o numero de telefone
                        close(s2);
                    }
                    else if (op_msg == 2) //foto
                    {
                        char caminho[] = "gurilo.jpg";
                        int tamanho_foto;
                        char tamanho_fotoChar[10];
                        char *buffer_foto, *buffer_msg;
                        char extensao_foto[4];
                        printf("digite o nome do arquivo que deseja abrir: ");
                        //fflush(stdin);
                        //gets(caminho);
                        //fflush(stdin);

                        foto = fopen(caminho, "rb");
                        fseek(foto, 0L, SEEK_END);                       //acha o fim do arquivo
                        tamanho_foto = ftell(foto);                      //acha a posicao do ultimo byte
                        printf("tamanho da imagem: %d\n", tamanho_foto); //tamanho do arquivo em bytes
                        rewind(foto);                                    //volta ao inicio do arquivo

                        buffer_foto = (char *)malloc(75638);
                        buffer_msg = (char *)malloc(75608); //30 e suficiente para o cabecalho
                        printf("\npassou buffer\n");
                        strtok(caminho, ".");
                        strcpy(extensao_foto, strtok(NULL, "\0"));
                        printf("\nantes copy\n");
                        sprintf(tamanho_fotoChar, "%d", tamanho_foto);
                        strcpy(buffer_foto, tamanho_fotoChar);
                        strcat(buffer_foto, "&");
                        strcat(buffer_foto, extensao_foto);
                        strcat(buffer_foto, "&");
                        printf("\ncabecalho foto: %s\n", buffer_foto);
                        fread(buffer_msg, tamanho_foto, 1, foto); //salva o conteudo da foto na variavel buffer
                        printf("buffer msg: %s\n", buffer_msg);
                        strcat(buffer_foto, buffer_msg);
                        strcat(buffer_foto, "&");
                        strcat(buffer_foto, telefone); //modelo: 20000&png&conteudo&telefone$
                        strcat(buffer_foto, "$");
                        printf("\ncabecalho true:\n %s", buffer_foto);
                        printf("\nbuffer foto: %d\n", strlen(buffer_foto));
                        int tamanho_enviado, num_pac = 0, resto = 0;
                        num_pac = tamanho_foto / 1024;
                        resto = tamanho_foto % 1024;
                        int i = 0;
                        for (i = 0; i < num_pac; i++)
                        {
                            if (send(s2, buffer_foto + (i * 1024), 1024 + 1, 0) < 0)
                            {
                                perror("Send()");
                                exit(5);
                            } //informa ao servidor o numero de telefone
                        }
                        if (resto > 0)
                        {
                            if (send(s2, buffer_foto + (i * 1024), resto + 1, 0) < 0)
                            {
                                perror("Send()");
                                exit(5);
                            } //informa ao servidor o numero de telefone
                        }

                        fclose(foto);
                    }
                }
            }
            break;

        case 2:
            // envia pra grupo
            printf("GRUPOS:\n");
            struct grupo *aux_print;
            struct contato *aux_contato1;
            int count_grupo = 1;
            char opcao_grupo[2], cabecalho_grupo[215];
            int opcao_grupo_int;
            aux_print = raiz_grupo;
            while (aux_print)
            {
                printf("%d) Nome Grupo: %s\n", count_grupo, aux_print->nome);
                count_grupo++;
                printf("INTEGRANTES:\n");
                aux_contato1 = aux_print->raiz;
                while (aux_print->raiz)
                {
                    printf("- Nome: %s\n", aux_print->raiz->nome);
                    aux_print->raiz = aux_print->raiz->prox;
                }
                printf("\n");
                aux_print->raiz = aux_contato1;
                aux_print = aux_print->prox;
            }
            fflush(stdin);
            gets(opcao_grupo);
            fflush(stdin);
            opcao_grupo_int = atoi(opcao_grupo);
            aux_print = raiz_grupo;
            for (int i = 1; i < opcao_grupo_int; i++)
            {
                aux_print = aux_print->prox;
            }
            char mensagem[100]; // nao eh 101? Se sim, substituir por tamanhoBuffer
            printf("\nMensagem: ");

            fflush(stdin);
            gets(mensagem);
            fflush(stdin);
            //printf("Msg enviada: %s", mensagem);
            strcat(mensagem, "&");
            strcat(mensagem, telefone);
            strcat(mensagem, "$");
            int auxCabecalho, auxTam;
            char auxCabecalhoChar[10];
            auxCabecalho = strlen(mensagem) + 4; 
            printf("\n aux cabecalho: %d\n", auxCabecalho);   
            sprintf(auxCabecalhoChar, "%d", auxCabecalho);
            auxTam = strlen(auxCabecalhoChar);
            auxCabecalho = auxCabecalho + auxTam;
            printf("\naux cabecalho + tam num: %d", auxCabecalho);
            sprintf(cabecalho_grupo, "%d", auxCabecalho);
            strcat(cabecalho_grupo, "&txt&");
            strcat(cabecalho_grupo, mensagem);
            strcat(cabecalho_grupo, "&");
            strcat(cabecalho_grupo, telefone);
            strcat(cabecalho_grupo, "$");
            printf("\ncabecalho grupo: %s\n", cabecalho_grupo);
            aux_contato1 = aux_print->raiz;
            while (aux_print->raiz)
            {

                strcpy(num_pesquisar, aux_print->raiz->telefone);
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
                    printf("\nCliente online\n");
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

                    if (send(s2, cabecalho_grupo, strlen(cabecalho_grupo) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(5);
                    } //informa ao servidor o numero de telefone
                    close(s2);
                }
                aux_print->raiz = aux_print->raiz->prox;
                memset(recvbuf, 0, sizeof(recvbuf));
            }
            aux_print->raiz = aux_contato1;
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
            printf("Digite o nome do grupo: ");
            fflush(stdin);
            gets(salvarNomeGrupo);
            fflush(stdin);
            //Cria o grupo na Lista
            struct grupo *aux_grupo;
            struct grupo *novo_grupo = (struct grupo *)malloc(sizeof(struct grupo));
            int auxQtd = 0;
            strcpy(novo_grupo->nome, salvarNomeGrupo);
            novo_grupo->prox = NULL;
            novo_grupo->raiz = NULL;

            if (raiz_grupo == NULL)
            {
                raiz_grupo = novo_grupo;
            }
            else
            {
                aux_grupo = raiz_grupo;
                while (aux_grupo->prox)
                {
                    aux_grupo = aux_grupo->prox;
                }
                aux_grupo->prox = novo_grupo;
            }
            printf("\nCRIOU O GRUPO\n\n");
            char insereUsuarioGrupo[10];
            int insereUsuarioGrupoInt;
            do
            {
                struct contato *aux1;
                int count = 0;
                aux1 = raiz_contato;
                if (aux1 == NULL)
                {
                    printf("\nSem contatos salvos!\n");
                    break;
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
                }
                printf("Deseja inserir mais algum usuario? '-1' para sair\n");
                fflush(stdin);
                gets(insereUsuarioGrupo);
                fflush(stdin);
                insereUsuarioGrupoInt = atoi(insereUsuarioGrupo);
                //se ele quer inserir mais um usuario
                if (insereUsuarioGrupoInt != -1)
                {
                    auxQtd++;
                    aux1 = raiz_contato;
                    for (int i = 0; i < insereUsuarioGrupoInt; i++)
                    {
                        aux1 = aux1->prox;
                    }
                    struct contato *copia_contato = (struct contato *)malloc(sizeof(struct contato));
                    struct grupo *aux_grupo2;
                    struct contato *aux_loc;
                    strcpy(salvarNome, aux1->nome);
                    strcpy(copia_contato->nome, salvarNome);
                    strcpy(salvarTelefone, aux1->telefone);
                    strcpy(copia_contato->telefone, salvarTelefone);
                    copia_contato->prox = NULL;

                    if (novo_grupo->raiz == NULL)
                    {
                        novo_grupo->raiz = copia_contato;
                        aux_loc = copia_contato;
                    }
                    else
                    {
                        aux_grupo2 = novo_grupo;
                        while (aux_grupo2->raiz->prox != NULL)
                        {
                            aux_grupo2->raiz = aux_grupo2->raiz->prox;
                        }
                        aux_grupo2->raiz->prox = copia_contato;

                        novo_grupo->raiz = aux_loc;
                    }
                }
            } while (insereUsuarioGrupoInt != -1);

            //arquivo grupo
            arquivoGrupos = fopen(nome_arquivo_grupos, "ab");
            if (!arquivoGrupos)
            {
                printf("\nErro na abertura do arquivo.\n");
            }
            else
            {
                struct grupo *aux_grupo3;
                struct contato *aux_loc2;
                struct contato salvaContato;
                strcpy(salvarNomeGrupo, novo_grupo->nome);
                fwrite(&salvarNomeGrupo, sizeof(salvarNomeGrupo), 1, arquivoGrupos);
                fwrite(&auxQtd, sizeof(auxQtd), 1, arquivoGrupos);
                aux_grupo3 = novo_grupo;
                aux_loc2 = novo_grupo->raiz;
                while (aux_grupo3->raiz->prox != NULL)
                {
                    strcpy(salvaContato.nome, novo_grupo->raiz->nome);
                    strcpy(salvaContato.telefone, novo_grupo->raiz->telefone);
                    fwrite(&salvaContato, sizeof(salvaContato), 1, arquivoGrupos);
                    aux_grupo3->raiz = aux_grupo3->raiz->prox;
                }
                strcpy(salvaContato.nome, novo_grupo->raiz->nome);
                strcpy(salvaContato.telefone, novo_grupo->raiz->telefone);
                fwrite(&salvaContato, sizeof(salvaContato), 1, arquivoGrupos);
                novo_grupo->raiz = aux_loc2;
                fclose(arquivoGrupos);
                printf("\nGrupo salvo com sucesso!\n");
            }
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
                            printf("*NOVA*[%s] - %s\n", aux->telefone, aux->mensagem);
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
                            printf("[%s] - %s\n", aux->telefone, aux->mensagem);
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
    FILE *foto_convertida;
    unsigned short port;
    char *buffer_recebimento;
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
        char tipo_msg[4];
        char tamanho_msg[50];
        struct mensagem *aux;
        char *nome_arquivo_foto = "output.";
        int namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }
        long count_tamanho = 0;
        //receber msg

        /*int retorno = recv(ns, recvbuf, sizeof(recvbuf), 0); //recebe a mensagem do cliente e verifica o valor de retorno
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
        }*/

        long tamanho_msg_int = 0;
        do
        {
            int retorno = recv(ns, recvbuf, sizeof(recvbuf), 0);
            count_tamanho = count_tamanho + retorno;
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
            if (tamanho_msg_int == 0)
            {
                //do while para garantir o recebimento da mensagem completa
                //possivel solucao: a cada iteracao copiar recvbuf pra uma string de tamanho = tamanho_msg
                strcpy(tamanho_msg, strtok(recvbuf, "&")); //salva em tipo_msg tudo que ha antes do &
                strcpy(tipo_msg, strtok(NULL, "&"));       //salva em tamanho_msg tudo que ha antes do &
                printf("\nTipo msg: %s\n", tipo_msg);
                buffer_recebimento = (char *)malloc(atoi(tamanho_msg));
                strcpy(buffer_recebimento, strtok(NULL, "$"));
                tamanho_msg_int = atoi(tamanho_msg);
                printf("tamanho msg = %s\n", tamanho_msg);
            }
            else
            {
                strcat(buffer_recebimento, recvbuf); //continua escrevendo no vetor que sera convertido na foto
            }
            printf("\nRecebido: %s\n\n", buffer_recebimento);
            printf("\ntamanho recebido: %d\ntamanho real: %d\n", count_tamanho, tamanho_msg_int);
        } while (count_tamanho < tamanho_msg_int + 1);

        char auxMsg[tamanho_msg_int + 1];
        strcpy(auxMsg, strtok(buffer_recebimento, "&"));
        struct mensagem *novo = (struct mensagem *)malloc(sizeof(struct mensagem));
        strcpy(novo->telefone, strtok(NULL, "$")); //salva o telefone
        //printa a mensagem
        if (strcmp(tipo_msg, "txt") == 0)
        {
            novo->mensagem = (char *)malloc(20000);
            strcpy(novo->mensagem, buffer_recebimento);
            //novo->mensagem = buffer_recebimento;
            printf("\n\nMengasem novo->mensagem: %s\n\n", novo->mensagem);
            //strcpy(novo->telefone, strtok(NULL, "$"));
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
                printf("\n[%s] - %s\n", novo->telefone, novo->mensagem);
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
        }
        else //qualquer extensao que nao seja "txt" e uma imagem
        {

            foto_convertida = fopen(strcat("recebido.", tipo_msg), "wb");
            strcpy(novo->mensagem, "imagem");

            fwrite(buffer_recebimento, atoi(buffer_recebimento), 1, foto_convertida); //salva o conteudo da foto no arquivo aberto
            fclose(foto_convertida);

            //salvar algo sobre a imagem e contato na lista ligada?
        }

        //fecha o socket
        close(ns);
    }
}