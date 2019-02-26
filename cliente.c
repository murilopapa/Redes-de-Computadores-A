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
void main(argc, argv)
int argc;
char **argv;
{


   int s, server_address_size;	//descritor do socket
   unsigned short port;
   struct sockaddr_in server;	//aqui sera armazenado o endereco do servidor
   char buf[200], res[2000];	//buf eh o conteudo a ser enviado ao servidor, em res sera armazenada sua resposta


   /* 
    * O primeiro argumento (argv[1]) é o endereço IP do servidor.
    * O segundo argumento (argv[2]) é a porta do servidor.
    */

   if(argc != 3) //argc = quantidade de parametros (./nome IP porta)
   {
      printf("Use: %s enderecoIP porta\n",argv[0]);
      exit(1);
   }
  	port = atoi(argv[2]);	//recupera o valor recebido pelo argv

   /*
    * Cria um socket UDP (dgram).
    */

   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //zero = erro, caso sucesso, socket recebe o endereco IP
   {
       perror("socket()");
       exit(1);
   }
   //printf("Valor da porta do servidor: %i\n\n", port); OK

   /* Define o endereço IP e a porta do servidor */
   server.sin_family      = AF_INET;            /* Tipo do endereço         */
   server.sin_port        = port;               /* Porta do servidor        */
   server.sin_addr.s_addr = inet_addr(argv[1]); /* Endereço IP do servidor  */

   
   do{ //enquanto o usuario nao desejar sair

   printf("> ");
   fgets(buf, sizeof(buf), stdin);
   strtok(buf, "\n");	//tira o \n inserido pelo fgets
   
   /* Envia a mensagem no buffer para o servidor */
   if (sendto(s, buf, (strlen(buf)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)	//envia o buffer ao servidor
   {
       perror("sendto()");
       exit(2);
   }

   //recebe a resposta do servidor
   server_address_size = sizeof(server);
   if (recvfrom(s, res, sizeof(res), 0, (struct sockaddr *)&server, &server_address_size) < 0)	//recebe res do servidor
   {
       perror("recvfrom()");
       exit(2);
   }

   printf("resposta do servidor ao comando %s: \n\n %s\n\n", buf, res);

   }while(strcmp( buf, "exit") != 0);	//fecha o cliente caso receba o comando "exit"
 
   /* Fecha o socket apenas quando o cliente digitar "exit" */
   close(s);
}
