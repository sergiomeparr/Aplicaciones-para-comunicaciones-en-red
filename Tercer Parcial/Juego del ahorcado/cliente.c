#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAXBUF 1024
struct usuario{
	char nombre[10];
	char edad[3];
};


int main(int argc, char **argv){
        	int sockfd, len, n;
	int cliente = 0;
	char caracter;
        	struct sockaddr_in dest;
	struct usuario *u;
        	char buf[MAXBUF + 1];
        	char buf2[MAXBUF + 1];
	char nombre[10];
	char edad[3];
	char opcion[2];
        	fd_set rfds;
	int bandera = 0;
        	struct timeval tv;
    	int retval, maxfd = -1;
    	if(argc != 3){
            	printf("Usage: %s IP Port",argv[0]);
            	exit(0);
        	}

    	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            	perror("Socket");
            	exit(errno);
	}
	cliente = 0;
    	int val=1;
   	int op = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
   	if(op<0){
   		perror("No se modifico opcion de socket\n");
   	}
    	//int flags = fcntl(sockfd, F_GETFL, 0);
    	//fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    	bzero(&dest, sizeof(dest));
        	dest.sin_family = AF_INET;
        	dest.sin_port = htons(atoi(argv[2]));
        	if (inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
            	perror(argv[1]);
            	exit(errno);
        	}
    	if(connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
            	perror("Connect ");
            	exit(errno);
        	}



    	printf("connect to server...\n");
			printf("Ingresa el nombre de usuario: \n" );

	while(1){

		FD_ZERO(&rfds);
                  FD_SET(0, &rfds);
                  maxfd = 0;
                  FD_SET(sockfd, &rfds);
                  if (sockfd > maxfd)
                        maxfd = sockfd;

           	tv.tv_sec = 1;
           	tv.tv_usec = 0;

           	retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);

            	if (retval == -1){
               		printf("select error! %s", strerror(errno));
                		break;
                	} else if (retval == 0) {
                        	//printf("no msg,no key, and continue to wait\n");
                        	continue;
		} else {
			if (FD_ISSET(0, &rfds)){
					//Modificado  Aqui se está modificando
				if(cliente == 0){
					bzero(buf, MAXBUF + 1);
					memset(nombre, '\0', 10);
					memset(edad, '\0', 3);
					scanf("%s", nombre);
					fflush(stdin);
					printf("Ingresa edad: \n");
					fflush(stdin);
					scanf("%s", edad);
					system("clear");
					printf("\t\t\n\nJuego del ahorcado");
					printf("\t\t\t\n 1.- Facil");
					printf("\t\t\t\n 2.- Dificil");
					printf("\t\t\n\nIngresa opcion: ");
					fflush(stdin);
					scanf("%s", opcion);
					fflush(stdin);
					strcat(buf, nombre);
					strcat(buf, ",");
					strcat(buf, edad);
					strcat(buf, ",");
					strcat(buf, opcion);
					//printf("%d", strlen(buf));

					len = send(sockfd, buf, 30, 0);
		                       	if (len > 0)
		                           	printf("msg:%s send successful,totalbytes: %d!\n", buf, len);
		                       	else {
		                           	printf("msg:'%s  failed!\n", buf);
		                           	break;
		                       	}
					cliente = 1;
				}
				bzero(buf, MAXBUF + 1);
				printf("Ingresa una letra: \n" );
				fflush(stdin);
				scanf("%s",buf);
				fflush(stdin);
				if (!strncasecmp(buf, "quit", 4)){
		                  	printf("request terminal chat!\n");
		                  	break;
		    }
		              	len = send(sockfd, buf, 30, 0);
				if (len > 0){
		                  	//printf("msg:%s envio exitoso, total de bytes: %d!\n", buf, len);
					//printf("letra : %s ", buf);
				}
				else {
		                  	printf("msg:'%s  fallo!\n", buf);
		                  	break;
		              	}
				//Se reciben los datos desde el servidor
				printf("Presiona enter para continuar\n" );
				if((n = recv(sockfd,buf,MAXBUF + 5,0)) > 0){
					printf("Palabra : %s ",buf);
      				}
				//Si hay un error en el envio de datos se desconecta el cliente
      				else{
          				printf("error al enviar datos!\n");
					system("exit");
					//close(sockfd);
          				//FD_CLR(sockfd,&allset);
          				//client[i].fd = -1;
      				}
							if ((!strncasecmp(buf, "Palabra adivinada!!", 19))||(!strncasecmp(buf, "Haz fallado!!", 13))){
															printf("Fin del juego\n");
															printf("Desconectado y saliendo del juego\n" );
															close(sockfd);
															exit(1);
													}

					//Aqui termina la modificacion
                        	}
                      /*  	else if (FD_ISSET(sockfd, &rfds)){
                                	bzero(buf, MAXBUF + 1);
                                	len = recv(sockfd, buf, MAXBUF, 0);
                                	if (len > 0)
                                    	printf("recv:'%s, total: %d \n", buf, len);
                                	else{
                        			if (len < 0)
                                            		printf("recv failed!errno:%d,error msg: '%s'\n", errno, strerror(errno));
                                    	else
                                            		printf("other exit,terminal chat\n");
                           		break;
                    		}
                   	}*/
            	}
    	}
    close(sockfd);
    return 0;
}
