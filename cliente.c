#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Cliente UDP
 */
void main(argc, argv) int argc;
char **argv;
{
    printf("**CLIENTE INICIADO**\n");
    int s, server_address_size; //descritor do socket
    unsigned short port;
    struct sockaddr_in server, client; //aqui sera armazenado o endereco do servidor
    char buf[200], res[2000];          //buf eh o conteudo a ser enviado ao servidor, em res sera armazenada sua resposta

    /* 
    * O primeiro argumento (argv[1]) � o endere�o IP do servidor.
    * O segundo argumento (argv[2]) � a porta do servidor.
    */

    if (argc != 3) //argc = quantidade de parametros (./nome IP porta)
    {
        printf("Use: %s enderecoIP porta\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[2]); //converte o valor recebido para adequar-se ao numero da porta do servidor (1024 = 4)
    printf("%u\n", port);

    /*
    * Cria um socket UDP (dgram).
    */

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //zero = erro, caso sucesso, socket recebe o endereco IP
    {
        perror("socket()");
        exit(1);
    }
    //printf("Valor da porta do servidor: %i\n\n", port); OK

    /* Define o endere�o IP e a porta do servidor */
    server.sin_family = AF_INET;                 /* Tipo do endere�o         */
    server.sin_port = port;                      /* Porta do servidor        */
    server.sin_addr.s_addr = inet_addr(argv[1]); /* Endere�o IP do servidor  */

    do
    { //enquanto o usuario nao desejar sair

        printf("> ");
        /*fgets(buf, sizeof(buf), stdin);
   strtok(buf, "\n");	//tira o \n inserido pelo fgets
   */

        scanf("%s", buf);

        /* Envia a mensagem no buffer para o servidor */
        if (sendto(s, buf, (strlen(buf) + 1), 0, (struct sockaddr *)&server, sizeof(server)) < 0) //envia o buffer ao servidor
        {
            perror("sendto()");
            exit(2);
        }

        //recebe a resposta do servidor
        server_address_size = sizeof(server);
        if (recvfrom(s, res, sizeof(res), 0, (struct sockaddr *)&server, &server_address_size) < 0) //recebe res do servidor
        {
            perror("recvfrom()");
            exit(2);
        }

        printf("resposta do servidor ao comando %s: \n\n %s\n\n", buf, res);
        int namelen = sizeof(client);

        if (getsockname(s, (struct sockaddr *)&client, &namelen) < 0) //getsockname retorna o IP ao qual o socket esta ligado (ao &server -> server.sin_addr.s_addr)
        {
            perror("getsockname()");
            exit(1);
        }

        //imprime o endereco do client
        printf("O endereco e: %d\n", client.sin_addr.s_addr);
        /* Imprime qual porta foi utilizada. */
        printf("Porta utilizada eh: %d\n", htons(client.sin_port));
    } while (strcmp(buf, "exit") != 0);

    /* Fecha o socket apenas quando o cliente digitar "exit" */
    close(s);
}
