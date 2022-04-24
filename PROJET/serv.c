#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define SIZE_BUFF 2048
#define MAX 4092

//structure type définissant un client
typedef struct{
	struct sockaddr_in address;
	int clientSocket;
	char name[16];
} clientStruct;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void req_send(char *s, int clientSocket){
	pthread_mutex_lock(&mutex);
	//envoie, via la meme socket du client, d'un message de la part du serveur
		if(write(clientSocket, s, strlen(s)) < 0){
			printf("erreur pendant l'envoie du message");
			kill(getpid(), SIGABRT);
		}

	pthread_mutex_unlock(&mutex);
}

int addAccount(char name[16], char passwd[16], clientStruct *clientConnected){
	FILE *login;
	char readUsername[16];
	char readPassword[16];
	int leave_flag = 0;
	int connected = 0;
		//on l'ouvre ici pour ne pas l'ouvrir alors qu'on ne souhaite pas ajouter de compte
		login = fopen("./login.txt","a+");
		//verification que le compte n'existe pas déjà
		while (!feof(login))
		{
			//ce fscanf permet de vérifier que l'on lie bien une ligne sur deux, pour ne voir que le username, et voir si ce username existe déjà
			fscanf(login, "%s;%s\n", readUsername, readPassword);
			if(strcmp(readUsername, name)==0){
				printf("Ce nom d'utilisateur existe déjà, veuillez relancer le client et entrer un username qui n'existe pas\n");
				req_send("1", clientConnected -> clientSocket);
				leave_flag = 1;
				break;
			}
		}
		if (leave_flag!=1)
		{
		fprintf(login, "%s ; %s\n", name, passwd);
		printf("les credentials sont bien enregistrées\n");
		connected =1;
		}
		fclose(login);
	return connected;
}

void writeInTemp(char sauv[MAX]){
	FILE *temp;
	temp = fopen("./temp.txt", "a+");
	fprintf(temp, "%s", sauv);
	fclose(temp);
}


int login(char name[16], char passwd[16]){
	FILE *login;
	char readUsername[16];
	char readPassword[16];
	int connected = 0;
	login = fopen("./login.txt", "r");
	while (!feof(login))
	{
		fscanf(login, "%s ; %s\n", readUsername, readPassword);
		if(strcmp(readUsername, name)==0 && strcmp(readPassword, passwd)==0){
			printf("\nbien connecté\n");
			connected = 1;
			break;
		}
	}
	return connected;
}

void deleteAccount(char name[16], char passwd[16]){
	FILE *login;
	char buffer[40];
	char tempString[MAX];
	char readUsername[16];
	char readPassword[16];
	login = fopen("./login.txt", "r");
	while (!feof(login))
	{
		fscanf(login, "%s ; %s\n", readUsername, readPassword);
		//on ne prend que les comptes qui ne sont pas celui que l'on veut supprimer
		if(strcmp(readUsername, name)!=0 && strcmp(readPassword, passwd)!=0){
			sprintf(buffer, "%s ; %s\n", readUsername, readPassword);
			//on les concatène dans une string temporaire..
			strcat(tempString, buffer);
		}
	}
	//que l'on écrit dans un fichier temporaire
	writeInTemp(tempString);
	bzero(tempString, MAX);
	fclose(login);
	//que l'on renomme login.txt après avoir supprimé l'ancien fichier
	remove("./login.txt");
	rename("./temp.txt", "./login.txt");
}

int authent(clientStruct *clientConnected){
	char buff_out[SIZE_BUFF];
	char name[16];
	char passwd[16];
	char choice[2] = "";
	int connected = 0;

	if(recv(clientConnected->clientSocket, choice, 1, 0)<= 0){
		printf("didn't received the choice\n");
	}else{
		printf("choice: %s|\n", choice);
		// Lecture du nom pour le client
		//si l'entrée est nulle, alors elle est rejetée
		if(recv(clientConnected->clientSocket, name, 16, 0) <= 0 || strlen(name) > 16){
			printf("Probleme avec le pseudo\n");
		} else{
			if (recv(clientConnected -> clientSocket, passwd, 16, 0 || strlen(passwd) > 16) <=0)
			{
				printf("Probleme avec le MDP\n");
			}else{
				//si la requete est d'ajouter un compte
				if(strcmp(choice, "1") == 0){
					int verif = addAccount(name, passwd, clientConnected);
					if(verif){connected = verif;}
				}
				//si la requete est de se connecter
				else if(strcmp(choice, "2") == 0){
					int verif = login(name, passwd);
					if(verif){connected = verif;}
					else{
						printf("mauvais identifiants");
						req_send(choice, clientConnected ->clientSocket);
					}
				}
				//si la requete est de supprimer un compte
				else if(strcmp(choice, "3") == 0){
					deleteAccount(name, passwd);
					printf("account deleted\n");
					req_send(choice, clientConnected->clientSocket);
				}
			if (connected == 1)
			{
				//sinon on copie dans l'attribut name du client, la variable name recue par le serveur
				//on la met ensuite dans un buffer, que l'on affiche et que l'on envoie aux autre clients (à fixer avec la fonction d'envoie de messages)
				strcpy(clientConnected->name, name);
				sprintf(buff_out, "%s has joined\n", clientConnected->name);
				printf("%s", buff_out);
			}else{
				printf("veuillez relancer le client pour ressayer de vous connecter\n");
			}
			}
		}
	}
	bzero(buff_out, SIZE_BUFF);
	return connected;
}

/* Handle all communication with the client */
void *clientCommunications(void *arg){
	char buff_out[SIZE_BUFF];
	int leave_flag = 0;
	
	clientStruct *clientConnected = (clientStruct *)arg;

	if(authent(clientConnected) == 1){
		while(1){
		if (leave_flag) {
			break;
		}
		//si on reçoit un message
		int receive = recv(clientConnected->clientSocket, buff_out, SIZE_BUFF, 0);
		if (receive > 0){
			//et que la taille du message est supérieure à 0
			if(strlen(buff_out) > 0 && strcmp(buff_out, "\n") !=0){
				//soit on écrit le message, soit, si la demande est de quitter le chat, alors
				//le client est déconnecté
				if(strcmp(buff_out, "exit\n") == 0){
					sprintf(buff_out, "%s has left\n", clientConnected->name);
					printf("%s", buff_out);
					leave_flag = 1;
				}else{
					printf("\n%s: %s\n", clientConnected->name, buff_out);
				}
			}
		} else if (receive == 0){
			sprintf(buff_out, "%s has left\n", clientConnected->name);
			printf("%s", buff_out);
			leave_flag = 1;
			//sinon erreur
		} else {
			printf("Error while connection\n");
			leave_flag = 1;
		}
		bzero(buff_out, SIZE_BUFF);
	}
	}else{
		printf("Vous n'etes pas connectés\n");

	}
	
	close(clientConnected->clientSocket);

  	pthread_detach(pthread_self());

	return NULL;
}


int main(int argc, char **argv){

	int port = atoi(argv[1]);
	int option = 1;
	int listenforclients = 0, connectionEstab = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t clientThread;

	if(argc != 2){
		printf("Veuillez rentrer le port sur lequel le serveur va écouter: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	

   //paramètres basiques pour la connection de la socket
	listenforclients = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//d'après le man pour htons 
	//  The htons() function converts the unsigned short integer hostshort from
    //    host byte order to network byte order.
	serv_addr.sin_port = htons(port);


	//on met en place la socket, avec les meme port, et meme adresse pour chaque nouvel arrivant
	if(setsockopt(listenforclients, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option))<0){
		printf("la socket n'a pas pu être mise en place\n");
		kill(getpid(), SIGABRT);
	}

	//liaison avec la socket

	if(bind(listenforclients, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		printf("erreur lors du bind\n");
		kill(getpid(), SIGABRT);
	}

	//on écoute sur le port, avec la socket établie plus tot, pour recevoir les messages
	//on autorise seulement 5 clients ici, mais cette valeur est modifiable
	if (listen(listenforclients, 5)< 0){
		printf("erreur lors du listen\n");
		kill(getpid(), SIGABRT);
	}

	printf("VOICI LES MESSAGES\n");

	while(1){

		socklen_t clilen = sizeof(cli_addr);
		connectionEstab = accept(listenforclients, (struct sockaddr*)&cli_addr, &clilen);

		clientStruct *clientConnected = (clientStruct *)malloc(sizeof(clientStruct));
		clientConnected->address = cli_addr;
		clientConnected->clientSocket = connectionEstab;

		pthread_create(&clientThread, NULL, &clientCommunications, (void*)clientConnected);

	};
	close(connectionEstab);
}