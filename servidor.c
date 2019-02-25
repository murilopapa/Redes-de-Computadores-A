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
    char resposta1[2000], resposta2[2000], respostafinal[2000], reserro[2000];
    int sockint, s, namelen, client_address_size;
    struct sockaddr_in client, server;
    char buf[MSG_SIZE];

    unsigned short port;

    strcpy(comando1, "clima");
    strcpy(comando2, "tempo");
    strcpy(comandoexit, "exit");
    strcpy(resposta1, "O clima no Brasil no mes de fevereiro eh quente e chuvoso\n");
    strcpy(resposta2, "O tempo hoje esta nublado e com ventos fortes\n");
    strcpy(respostafinal, "Obrigado por utilizar o servidor\n");
    strcpy(reserro, "Comando nao reconhecido");

    if (argc != 2)
    {
        printf("Use: %s porta\n", argv[0]);
        exit(1);
    }

    //host byte to network byte
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
    * Define a qual endereço IP e porta o servidor estará ligado.
    * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereços IP
    */
    server.sin_family = AF_INET;                                          /* Tipo do endereço             */
    server.sin_port = port; /* Escolhe uma porta disponível */            //para que a mensagem chegue ao servidor, a porta do cliente deve ser a mesma daqui
    server.sin_addr.s_addr = INADDR_ANY; /* Endereço IP do servidor    */ //INADDR_ANY liga o socket a todos as interfaces (portas) disponiveis INADDR_ANY pode assumir qualquer endereco IP do host

    /*
    * Liga o servidor á porta definida anteriormente.
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
    * O endere�o do cliente ser� armazenado em "client".
    */
    do
    {

        client_address_size = sizeof(client);
        if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_address_size) < 0)
        {
            perror("recvfrom()");
            exit(1);
        }

        /*
    * Imprime a mensagem recebida, o endere�o IP do cliente
    * e a porta do cliente 
    */
        printf("Recebida a mensagem '%s' do endereco IP %s da porta %d\n", buf, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        if (strcmp(buf, comando1) == 0)
        { //comando1 recebido do cliente
            //printf("comando clima recebido\n"); //OK
            if (sendto(s, resposta1, strlen(resposta1) + 1, 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(1);
            }
        }
        else if (strcmp(buf, comando2) == 0)
        { //comando2 recebido do cliente
            //printf("comando tempo recebido\n"); //OK
            if (sendto(s, resposta2, strlen(resposta2) + 1, 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(1);
            }
        }
        else if (strcmp(buf, comandoexit) == 0)
        { //exit recebido do cliente
            //printf("exit recebido\n"); //OK
            if (sendto(s, respostafinal, strlen(respostafinal) + 1, 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(1);
            }
        }
        else
        { //caso o comando nao seja valido
            //printf("comando desconhecido recebido\n"); //OK
            if (sendto(s, reserro, strlen(reserro) + 1, 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(1);
            }
        }

    } while (strcmp(buf, "exit") != 0);

    /*
    * Fecha o socket.
    */
    close(s);
}
