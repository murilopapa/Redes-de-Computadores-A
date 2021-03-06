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
#include <arpa/inet.h>
#include <string.h>

#define MSG_SIZE 200

/*
 * Servidor UDP
 */
void main(argc, argv) int argc;
char **argv;
{
    printf("**SERVIDOR INICIADO**\n");
    char comando1[MSG_SIZE], comando2[MSG_SIZE], comandoexit[MSG_SIZE];
    char resposta[2000], respostafinal[2000];
    int sockint, s, namelen, client_address_size;
    struct sockaddr_in client, server;
    char buf[MSG_SIZE];
    FILE *fp;

    memset(resposta, 0, sizeof(resposta));
    int return_fread;
    int size_sendto;

    unsigned short port;

    strcpy(respostafinal, "Obrigado por utilizar o servidor\n");

    if (argc != 2)
    {
        printf("Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]); //recebe o numero da porta atraves do parametro dado no terminal

    printf("%u\n", port);

    /*
    * Cria um socket UDP (dgram). 
    */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }

    /*
    * Define a qual endereÃ§o IP e porta o servidor estarÃ¡ ligado.
    * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereÃ§os IP
    */
    server.sin_family = AF_INET;                                           /* Tipo do endereÃ§o             */
    server.sin_port = port; /* Escolhe uma porta disponÃ­vel */            //para que a mensagem chegue ao servidor, a porta do cliente deve ser a mesma daqui
    server.sin_addr.s_addr = INADDR_ANY; /* EndereÃ§o IP do servidor    */ //INADDR_ANY liga o socket a todos as interfaces (portas) disponiveis INADDR_ANY pode assumir qualquer endereco IP do host

    /*
    * Liga o servidor Ã¡ porta definida anteriormente.
    */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) //bind designa um nome ao socket cujo IP esta associado ao addr
    {
        perror("bind()");
        exit(1);
    }

    /* Consulta qual porta foi utilizada. */
    namelen = sizeof(server);
    if (getsockname(s, (struct sockaddr *)&server, &namelen) < 0) //getsockname retorna o IP ao qual o socket esta ligado (ao &server -> server.sin_addr.s_addr)
    {
        perror("getsockname()");
        exit(1);
    }

    //imprime o endereco do servidor
    printf("o endereco e: %d\n", server.sin_addr.s_addr);

    /* Imprime qual porta foi utilizada. */
    printf("Porta utilizada eh: %d\n", server.sin_port);

    namelen = sizeof(client);
    if (getsockname(s, (struct sockaddr *)&client, &namelen) < 0) //enderece ip do cliente
    {
        perror("getsockname()");
        exit(1);
    }

    /*
    * Recebe uma mensagem do cliente.
    * O endereï¿½o do cliente serï¿½ armazenado em "client".
    */
    do
    {
        memset(resposta, 0, sizeof(resposta));
        client_address_size = sizeof(client);
        if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_address_size) < 0) //recebe a mensagem do cliente e armazena em buf
        {
            perror("recvfrom()");
            exit(1);
        }

        /*
    * Imprime a mensagem recebida, o endereï¿½o IP do cliente
    * e a porta do cliente 
    */
        printf("Recebida a mensagem '%s' do endereco IP %s da porta %d\n", buf, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        /* Open the command for reading. */
        fp = popen(buf, "r");
        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        fread(resposta, (sizeof(resposta) + 1), 1, fp); //le o conteudo apontado por fp e o armazena em resposta
        
        // Adiciona o \0 para enviar exatos 2000 caractéres
        resposta[1999] = '\0';
        
        /* close */
        pclose(fp);

        //usar fread, ver qnt que ele leu
        //printf("\n PRINT PATH ");
        //printf("%s", path);

        if (strcmp(buf, "exit") == 0)
        {                              //exit recebido do cliente
            printf("exit recebido\n"); //OK
            if (sendto(s, respostafinal, strlen(respostafinal) + 1, 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(1);
            }
        }
        else
        { //outro comando recebido do cliente
            if ((size_sendto = sendto(s, resposta, strlen(resposta) + 1, 0, (struct sockaddr *)&client, sizeof(client))) < 0)
            {
                perror("sendto()");
                exit(1);
            }
            printf("\n SIZE SENDTO: %d\n", size_sendto);
            printf("\n STRLEN RESPOSTA + 1: %ld\n", strlen(resposta) + 1);
        }
    } while (1); //servidor continua sendo executado mesmo apos o cliente ser encerrado.

    /*
    * Fecha o socket.
    */
    close(s);
}
