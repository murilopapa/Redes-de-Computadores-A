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
/*
 * Servidor TCP
 */
main(argc, argv) int argc;
char **argv;
{
    unsigned short port;
    char sendbuf[101];
    char recvbuf[101];
    struct sockaddr_in client;
    struct sockaddr_in server;
    int s;  /* Socket para aceitar conex�es       */
    int ns; /* Socket conectado ao cliente        */
    int namelen;

    int indice = 0;       // variavel para contar a quantidade de mensagens
    char mensagembuf[80]; // mensagem que recebe na função
    char usuariobuf[20];  // usuario que recebe na função

    char usuarios[10][20];  // 0 a 9 usuarios e o décimo é /0
    char mensagens[10][80]; // 0 a 9 mensagens e a décima é /0

    char mensagem_inteira[101];

    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short)atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conex�es
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    /*
    * Define a qual endereço IP e porta o servidor estará ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereços IP
    */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    printf("*** Servidor Iniciado! ***\n");

    /*
     * Aceita uma conex�o e cria um novo socket atrav�s do qual
     * ocorrer� a comunica��o com o cliente.
     */
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }

printf("\nCliente Conectou\n");
    while (1)
    {
int var;
        /* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
        memset(mensagembuf, 0, sizeof(mensagembuf));
        memset(usuariobuf, 0, sizeof(usuariobuf));
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
var = recv(ns, recvbuf, sizeof(recvbuf), 0);


        if (var == -1)
        {
            perror("Recvbuf()");
            exit(6);
        }

	if (var == 0)
        {
            perror("Recvbuf()");
		close(ns);
		close(s);
            exit(6);
        }
	
        printf("\nMensagem recebida do cliente: %s\n", recvbuf);

			strcpy(sendbuf, "Enviando mensagem para o Arduino!");
                            if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                            {
                                perror("Send()");
                                exit(7);
                            }
			printf("\nEnviando: %s\n",sendbuf);

    }
    /* Fecha o socket conectado ao cliente */
    close(ns);

    /* Fecha o socket aguardando por conex�es */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}
