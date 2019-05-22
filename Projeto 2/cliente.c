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

/*
 * Cliente TCP
 */
main(argc, argv) int argc;
char **argv;
{

    FILE *agenda;
    fopen("agenda.txt", "w+");  //abrindo o arquivo agenda.txt para leitura e escrita

    unsigned short port;
    char sendbuf[101];
    char recvbuf[101];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    char telefone[9];

    char mensagem[80];
    char nome[20];
    char envio[101]; // + o "#"

    char operacao[4]; //cad, ler, apa

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

    do
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(mensagem, 0, sizeof(mensagem));
        memset(nome, 0, sizeof(nome));
        
        printf("Informe seu numero de telefone:  ");
        gets(telefone);

        if (send(s, telefone, strlen(telefone) + 1, 0) < 0)
            {
                perror("Send()");
                exit(5);
            } //informa ao servidor o numero de telefone

        //provavelmente um menu com as opcoes (mandar mensagem, criar grupo, etc)
        switch (op)
        {

        case 1:
            // enviar mensagem
            break;

        case 2:
            // criar grupo
            
            break;

        case 3:
            // 
            break;

        case 4: //sair
            printf("Obrigado por utilizar a aplicacao\n");

            break;

        default:
            printf("Opcao invalida!\n");
            break;
        }

    } while (op != 4);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
