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
	unsigned short port;
	char sendbuf[101];
	char recvbuf[101];
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;
	int op;
	int quantidade = 0;

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
		printf("\n\t*** Menu ***\n");
		printf("\n1 - Cadastrar mensagem\n");
		printf("2 - Ler mensagens\n");
		printf("3 - Apagar mensagens\n");
		printf("4 - Sair da aplicacao\n");
		printf("\nOpcao: ");
		scanf("%d", &op);

		switch (op)
		{

		case 1:
			// Cadastrar mensagem
			strcpy(operacao, "cad");
			if (send(s, operacao, strlen(operacao) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //informa ao servidor qual operacao sera feita

			__fpurge(stdin);
			printf("\nDigite o nome: ");
			fgets(nome, sizeof(nome), stdin);
			strtok(nome, "\n"); //tira o \n inserido pelo fgets

			__fpurge(stdin);
			printf("Digite a mensagem: ");
			fgets(mensagem, sizeof(mensagem), stdin);
			strtok(mensagem, "\n"); //tira o \n inserido pelo fgets
			printf("\nNome recebido do cliente: %s\n", nome);
			printf("\nMensagem recebida do cliente: %s\n", mensagem);

			strcpy(envio, nome);
			strcat(envio, "#");
			strcat(envio, mensagem);
			strcat(envio, "$$");
			printf("\nEnvio: %s\n", envio);

			if (send(s, envio, strlen(envio) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //envia o nome ao servidor

			if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
			{
				perror("Recv()");
				exit(6);
			} //recebe a resposta do servidor
			printf("Servidor: %s\n", recvbuf);

			break;

		case 2:
			// Ler mensagens
			strcpy(operacao, "ler");

			if (send(s, operacao, strlen(operacao) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //informa ao servidor qual operacao sera feita

			//salva as mensagens e ja printa
			if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
			{
				perror("Recv()");
				exit(6);
			} //recebe o numero de mensagens cadastradas
			printf("\n%s\n", recvbuf);
			//talvez tenhamos que converter quantidade para int (char)

			char indice_recebido[2];
			strcpy(indice_recebido, recvbuf);
			quantidade = atoi(indice_recebido);
			printf("QUANTIDADE: %d\n", quantidade);

			for (int i = 0; i < quantidade; i++)
			{
				//recebe o nome e imprime
				if (recv(s, envio, sizeof(envio), 0) < 0)
				{
					perror("Nome()");
					exit(6);
				}

				strcpy(nome, strtok(envio, "#"));
				strcpy(mensagem, strtok('\0', "$$"));

				printf("Usuario: %s", nome);
				printf("\t\tMensagem: %s\n", mensagem);
			}
			break;

		case 3:
			// Apagar mensagem
			strcpy(operacao, "apa");

			if (send(s, operacao, strlen(operacao) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //informa ao servidor qual operacao sera feita

			printf("Digite o nome do usuario que tera a mensagem apagada: ");

			__fpurge(stdin);
			fgets(nome, sizeof(nome), stdin);
			strtok(nome, "\n"); //tira o \n inserido pelo fgets

			if (send(s, nome, strlen(nome) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //envia o nome associado a mensagem que sera apagada

			if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
			{
				perror("Recv()");
				exit(6);
			} //recebe a resposta do servidor
			char msg_apagadas[2];

			strcpy(msg_apagadas, recvbuf);

			printf("\nMensagens apagadas: %s\n", recvbuf);

			for (int i = 0; i < msg_apagadas; i++)
			{
				//recebe o nome e imprime
				if (recv(s, envio, sizeof(envio), 0) < 0)
				{
					perror("Nome()");
					exit(6);
				}

				strcpy(nome, strtok(envio, "#"));
				strcpy(mensagem, strtok('\0', "$$"));

				printf("Usuario: %s", nome);
				printf("\t\tMensagem: %s\n", mensagem);
			}
			break;

		case 4: //sair
			printf("Obrigado por utilizar a aplicacao\n");

			strcpy(operacao, "out");

			if (send(s, operacao, strlen(operacao) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //informa ao servidor qual operacao sera feita

			break;

		case 9:
			printf("Comando secreto para encerrar servidor\n");

			strcpy(operacao, "stp");

			if (send(s, operacao, strlen(operacao) + 1, 0) < 0)
			{
				perror("Send()");
				exit(5);
			} //informa ao servidor qual operacao sera feita
			break;
		default:
			printf("Opcao invalida!\n");
			break;
		}

	} while ((op != 4) || (op != 9));

	/* Fecha o socket */
	close(s);

	printf("Cliente terminou com sucesso.\n");
	exit(0);
}
