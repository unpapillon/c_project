#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

struct arg_struct {
    int sockfd;
    char name[62];
};

struct auth{
    int sockfd;
    int choice;
    char username[30];
    char passwd[30];
};

void send_msg_handler(void *arguments) {
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};
    struct arg_struct *args = arguments;
    // int *sockvalue = (int *) arguments;
    // int sockfd = *sockvalue;

    while(1) {
        fgets(message, LENGTH, stdin);

        if (strcmp(message, "exit\n") == 0) {
                break;
        } else {
        sprintf(buffer, "%s: %s\n", args -> name, message);
        send(args -> sockfd, buffer, strlen(buffer), 0);
        }
        //clean messages and buffer
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
}

void recv_msg_handler(void *arguments) {
	char message[LENGTH] = {};
    struct arg_struct *args = arguments;
    // int *sockvalue = (int *) arg;
    // int sockfd = *sockvalue;

    while (1) {
        int receive = recv(args -> sockfd, message, LENGTH, 0);
        if (receive > 0) {
        printf("%s", message);
        } else if (receive == 0) {
                break;
        } else {
                // -1
            }
            memset(message, 0, sizeof(message));
    }
}

void sendAuthInfo(void *arguments){
    char username[30];
    char password[30];
    char buffer[LENGTH + 32] = {};

    struct auth *authent = arguments;
    printf("Username:  ");
    fgets(username, 30, stdin);
    strcpy(authent->username, username);
    printf("\nPassword: ");
    fgets(password, 20, stdin);
    strcpy(authent->passwd, password);
    
    sprintf(buffer, "%s %s", username, password);
    send(authent->sockfd, buffer, strlen(buffer), 0);
    bzero(username, 30);
    bzero(password, 30);
    bzero(buffer, LENGTH + 32);
}


int main(int argc, char **argv){

    struct arg_struct args;
    

    args.sockfd = 0;
    char choice[2] = "";
    struct sockaddr_in server_addr;
    char username[16];
    char password[16];
    char buffer[LENGTH + 32] = {};

	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

    printf("Vous etes sur le point de rentrer dans le chat:\n-si vous voulez créer un compte, tapez 1\n-si vous voulez vous connecter, tapez 2\n-si vous voulez supprimer un compte, tapez 3\n");
    scanf("%s", choice);

    if(strcmp(choice, "1")){
        printf("veuillez définir vos credentials\n");
    }

    if(strcmp(choice, "2")){
        printf("veuillez entrer votre pseudo et votre mot de passe\n");
    }

	printf("Username:  ");
    scanf("%s", username);
    printf("\nPassword: ");
    scanf("%s", password);

    strcpy(args.name, username);
	// if (strlen(args.name) > 32 || strlen(args.name) < 2){
	// 	printf("Name must be less than 30 and more than 2 characters.\n");
	// 	return EXIT_FAILURE;
	// }


	/* Socket settings */
	args.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(args.sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR while connecting\n");
		return EXIT_FAILURE;
	}
    send(args.sockfd, choice, 1, 0);
    //send authentifications informations
    // strcat(username, "\n");
    // strcat(password, "\n");
    
    // sprintf(args.name, "%s\n%s\n", username, password);
    send(args.sockfd, username, 16, 0);
    send(args.sockfd, password, 16, 0);
    bzero(username, 16);
    bzero(password, 16);
    bzero(buffer, LENGTH + 32);
    

	// Send name
	// send(args.sockfd, args.name, 32, 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");

  
    pthread_t recv_msg_thread;
    pthread_t send_msg_thread;

    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, (void*)&args) != 0){
		printf("ERROR while using thread for receive messages\n");
		return EXIT_FAILURE;
	}
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, (void*)&args) != 0){
		printf("ERROR while using thread for sending message\n");
    return EXIT_FAILURE;
	}


    while(1){
    
  
    }

	return EXIT_SUCCESS;
}