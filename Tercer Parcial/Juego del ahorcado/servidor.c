#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAXBUF 1024

struct usuario{
	char nombre[10];
	char apellido[20];
	int edad;
};

typedef struct CLIENT {
    	int fd;
    	struct sockaddr_in addr;
}CLIENT;

/***************************
**server for multi-client
**PF_SETSIZE=1024
****************************/
int main(int argc, char** argv){
	int palabras_faciles[] = {0, 14, 37, 51, 66, 82, 97, 114, 129, 143, 159,
			        172, 187, 203, 218, 232, 249, 262, 268, 281};
	int palabras_dificiles[] = {6, 18, 28, 42, 56, 72, 88, 103, 119, 134, 149,
				 164, 177, 192, 209, 224, 238, 253, 273, 285};
	//int palabras_medias[] = {};
	//int palabras_dificiles = {};
	struct sockaddr_in  my_addr,addr;
    	CLIENT client[FD_SETSIZE];
	char adivinar_palabra[15];
	int palabra_adivinada = 0;
	int ganaste_perdiste = 0;
	int oportunidades = 0;
	int posiciones = 0;
	char buf[MAXBUF + 1];
	unsigned int myport;
	unsigned int lisnum;
    	fd_set rset,allset;
	char palabras[300];
	char auxiliar[15];
	struct timeval tv;
	int contador = 0;
	int maxfd = -1;
	int maxi = -1;
	int connectfd;
	socklen_t len;
	int letra = 1;
	int aleatorio;
	int respuesta;
	char caracter;
	int longitud;
	int slisten;
	int bandera = 0;
	int mensaje;
	int archivo;
	int indice;
	int nready;
	int sockfd;
	char c;
	int i;
	int n;
	//Puerto  1234
    	if(argv[1])
        		myport = atoi(argv[1]);
    	else
        		myport = 1234;
	//direccion ip
    	if(argv[2])
        		lisnum = atoi(argv[2]);
    	else
        		lisnum = FD_SETSIZE;

	//Genera el socket
    	if((slisten = socket(AF_INET,SOCK_STREAM,0)) == -1){
        		perror("socket");
        		exit(1);
    	}
    	int flags = fcntl(slisten, F_GETFL, 0);
	//habilita la bandera para que no se bloquee el socket al leer datos
    	fcntl(slisten, F_SETFL, flags | O_NONBLOCK);

    	bzero(&my_addr,sizeof(my_addr));    //coloca todo en cero
    	my_addr.sin_family = AF_INET;
    	my_addr.sin_port = htons(myport);
    	my_addr.sin_addr.s_addr = INADDR_ANY;

	//Asigna el puerto al socket
    	if(bind(slisten, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
            	perror("bind");
            	exit(1);
        	}

	//Comienza a escuchar en el socket
    	if (listen(slisten, lisnum) == -1) {
            	perror("listen");
            	exit(1);
        	}
    	for(i=0;i<FD_SETSIZE;i++){
        		client[i].fd = -1;
    	}

    	FD_ZERO(&allset);
        	FD_SET(slisten, &allset);
    	maxfd = slisten;
    	printf("Servidor listo para recibir un maximo de %d conexiones...\n",FD_SETSIZE);

	//Se abre el archivo con palabras faciles
	archivo = open("palabras.txt", O_RDONLY);
	if(archivo!=-1){
	//Leyendo el archivo e ingresandolo en un arreglo
		while(read(archivo,&c,sizeof(c)!= 0)){
			palabras[contador] = c;
			contador++;
		}
		close(archivo);
	}
	else{
		printf("No existe el archivo");
	}



    	while (1){
        		rset = allset;
        		tv.tv_sec = 1;
   		tv.tv_usec = 0;

            	nready = select(maxfd + 1, &rset, NULL, NULL, &tv);   //revisa si hay algo para leer datos

        		if(nready == 0)
            		continue;   // si no entonces regresa al bucle
		else if(nready < 0){
            		printf("select failed!\n");   //si habia datos pero no se leyeron correctamente
            		break;
        		}
        		else{
            		if(FD_ISSET(slisten,&rset)){ // new connection

                			len = sizeof(struct sockaddr);
				//Acepta las conexiones que se soliciten
       				if((connectfd = accept(slisten,(struct sockaddr*)&addr,&len)) == -1){
                    			perror("accept() error\n");
                    			continue;
                			}
                			for(i=0;i<FD_SETSIZE;i++){
                    			if(client[i].fd < 0){
                        				client[i].fd = connectfd;
                        				client[i].addr = addr;
               					printf("Cliente conectado desde %s.\n",
							inet_ntoa(client[i].addr.sin_addr));

						break;
                    			}
                			}
                			if(i == FD_SETSIZE)     //se sobrepaso el limite de clientes para escuchar
                    			printf("Demasiadas conexiones");
                			FD_SET(connectfd,&allset);
                			if(connectfd > maxfd)
           				maxfd = connectfd;
                			if(i > maxi)
                    			maxi = i;
            		}
            		else{
                			for(i=0;i<=maxi;i++){
                    			if((sockfd = client[i].fd)<0)
                        				continue;
                    			if(FD_ISSET(sockfd,&rset)){
							//Aqui empieza la modificacion del servidor
						bzero(buf,MAXBUF + 5);
                        				//Se reciben los datos desde el cliente
						if((n = recv(sockfd,buf,MAXBUF + 5,0)) > 0){
							printf("Dato del cliente: %s", buf);

							//Aqui se elige la dificultad, dependiendo el cliente
							srand(time(NULL));
							//Nivel Facil o Nivel Dificil
							if(buf[strlen(buf) - 1] == '1'||buf[strlen(buf) - 1] == '2'){
								bzero(auxiliar, 10);
								//Indice de cada palabra aleatoriamente
								aleatorio = rand() % 21;
								//Palabras nivel fáciles
								if(buf[strlen(buf) - 1] == '1')
									indice = palabras_faciles[aleatorio];
								//Palabras nivel dificil
								if(buf[strlen(buf) - 1] == '2')
									indice = palabras_dificiles[aleatorio];
								//Localizamos la palabra que se escogió aleatoriamente
								for(int i = indice; i < contador; i++){
									if(palabras[i] != ','){
										auxiliar[posiciones] = palabras[i];
										posiciones++;
									}
									else
										break;
								}
								printf("\nPalabra a adivinar : %s\n\n", auxiliar);
								bzero(adivinar_palabra, 15);
								for(int i = 0; i < posiciones; i++)
									adivinar_palabra[i] = '_';

							}
							//Aqui es para cada letra que envia el cliente al servidor
							else{
								//Se destapan las letras adivinadas
								for(int j = 0; j < posiciones; j++){
									//Si la letra esta en la cadena, mostrarla
									if(auxiliar[j] == buf[0]){
										adivinar_palabra[j] = auxiliar[j];
										letra++;
									}
								}
								//Con este sabemos si adivinó la palabra
								palabra_adivinada = palabra_adivinada + letra;
								//Aqui son las oportunidades que tiene el cliente
								if(letra == 0){
									oportunidades++;
								}
								letra = 0;
								//Si se adivina la palabra, se termina el juego
								if(palabra_adivinada == posiciones + 1){
									ganaste_perdiste = 1;
								}
								//Se verifica si el cliente superó las oportunidades
								if(oportunidades == 4)
									ganaste_perdiste = 2;
							}
                        				}
						//Si hay un error en el envio de datos se desconecta el cliente
                        				else{
                            				printf("Cliente Desconectado!\n");
                   					close(sockfd);
                            				FD_CLR(sockfd,&allset);
                            				client[i].fd = -1;
                        				}
						bzero(buf, MAXBUF + 1);
						if(ganaste_perdiste == 1){
							strcat(buf, "Palabra adivinada!!");
								ganaste_perdiste = 0;
						}
						if(ganaste_perdiste == 2){
							strcat(buf, "Haz fallado!!");
							ganaste_perdiste = 0;
						}
						else
							strcat(buf, adivinar_palabra);
                                                     	//Envio de datos al cliente
						longitud = send(sockfd, buf, 15, 0);
						if ((!strncasecmp(buf, "Palabra adivinada!!", 19))||(!strncasecmp(buf, "Haz fallado!!", 13))){
														printf("Fin del juego\n");
														close(slisten);
														exit(1);
												}
						//Verificamos si el cliente mando datos de los contrario hay fallo
						if (longitud > 0)
	                           			printf("Palabra: %s \n", adivinar_palabra);
                       				else {
	                           			printf("Re-envio:'%s  fallado!\n", buf);
	                           			//break;
	                       			}
							//Aqui termina la modificacion del servidor

           				}
                			}
            		}
        		}
    	}
	//Se cierra el socket
    	close(slisten);
}
