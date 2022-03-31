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

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

// static  unsigned int cli_count = 0;
// static int uid = 10;

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


//utiliser la fonction send message du server que l'on peut trouver sur https://www.binarytides.com/server-client-example-c-sockets-linux/
void send_message(char *s, int uid, int sockfd){
	pthread_mutex_lock(&clients_mutex);

				if(write(sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
				}

	pthread_mutex_unlock(&clients_mutex);
}

/* Handle all communication with the client */
void *handle_client(void *arg){
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	//cli_count++;
	client_t *cli = (client_t *)arg;

	// Lecture du nom pour le client
	//si l'entrée est nulle, alors elle est rejetée
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		//sinon on copie dans l'attribut name du client, la variable name recue par le serveur
		//on la met ensuite dans un buffer, que l'on affiche et que l'on envoie aux autre clients (à fixer avec la fonction d'envoie de messages)
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->uid, cli->sockfd);
	}

	bzero(buff_out, BUFFER_SZ);

	while(1){
		if (leave_flag) {
			break;
		}
		//si on reçoit un message
		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
		if (receive > 0){
			//et que la taille du message est supérieure à 0
			if(strlen(buff_out) > 0){
				//on l'envoie à tout le monde, et on l'affiche sur le serveur avec le nom de l'envoyeur
				send_message(buff_out, cli->uid, cli->sockfd);

				printf("%s -> %s\n", buff_out, cli->name);
			}
			//sinon, si le buffer est nul, ou bien qu'il a tapé le mot exit, alors on clos la connection
		} else if (receive == 0 || strcmp(buff_out, "exit") == 0){
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid, cli->sockfd);
			leave_flag = 1;
			//sinon erreur
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SZ);
	}

  /* Delete client from queue and yield thread */
	close(cli->sockfd);
//   queue_remove(cli->uid);
//   free(cli);
//   cli_count--;
  pthread_detach(pthread_self());

	return NULL;
}


int main(int argc, char **argv){
	static unsigned int cli_count = 0;
    static int uid = 10;

	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	//String to short
	int port = atoi(argv[1]);

	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

    //Socket settings
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	//d'après le man pour htons 
	//  The htons() function converts the unsigned short integer hostshort from
    //    host byte order to network byte order.
	serv_addr.sin_port = htons(port);

	//Signals
	signal(SIGPIPE, SIG_IGN);

	//A creuser 
	if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option))<0){
		printf("socket setting up failed\n");
		return EXIT_FAILURE;
	}

	//Bind

	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR while binding\n");
		return EXIT_FAILURE;
	}

	//Listen 
 
	if (listen(listenfd, 10)< 0){
		printf("ERROR while listening\n");
		return EXIT_FAILURE;
	}

	printf("HERE IS THE CHAT AND MESSAGES\n");

	while(1){

		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
	
	//testing max client

	if((cli_count + 1) == MAX_CLIENTS){
		printf("Nombre Max de personne sur le chat atteint\n");
		close(connfd);
		continue;
	}
	
	/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		/* Add client to the queue and fork thread */
		// queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);


	};

}