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
    printf("=== BIENVENU SUR LE CHAT ===\n");
    printf("vous pouvez envoyer des messages désormais %s: ",args -> name);
    while(1) {
        printf("\n->");
        fgets(message, LENGTH, stdin);
        strcpy(buffer, message);
        // sprintf(buffer, "%s: %s", args -> name, message);
        if(strlen(buffer) > 0 && strcmp(buffer, "\n") !=0){
            send(args -> sockfd, buffer, strlen(buffer), 0);
        }
        if(strcmp(buffer, "exit\n") == 0){
            printf("vous avez bien été déconnecté du serveur\n");
            //SIGNAL
            kill(getpid(), SIGTERM);
        }
        }
        //clean messages and buffer
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    // }
}



void getRequest(struct arg_struct *args){
    char choice[2] = "";
    char username[16];
    char password[16];
    char buffer[LENGTH + 32] = {};
    char entry[4092];
    printf("Vous etes sur le point de rentrer dans le chat:\n-si vous voulez créer un compte, tapez 1\n-si vous voulez vous connecter, tapez 2\n-si vous voulez supprimer un compte, tapez 3\n");

    while(1){
        scanf("%s", entry);
        if(strlen(entry) < 2){
            strcpy(choice, entry);
            bzero(entry, 4092);
            break;
        }else{
            printf("\nveuillez entrer une valeur correcte parmis les choix proposés\n");
            bzero(entry, 4092);
        }
    }
//TODO: faire un switch   
    if(strcmp(choice, "1") == 0){
        printf("veuillez définir vos credentials, 16 caractères chacuns au maximum\n");
    }
    if(strcmp(choice, "2") == 0){
        printf("veuillez entrer votre pseudo et votre mot de passe\n");
    }
    if(strcmp(choice, "3") == 0){
        printf("veuillez entrer les credentials du compte à supprimer\n");
    }
	printf("Username:  ");
    while(1){
        scanf("%s", entry);
        if(strlen(entry) < 16){
            strcpy(username, entry);
            bzero(entry, 4092);
            break;
        }else{
            printf("\nveuillez entrer un pseudo faisait 16 caractères ou moins\n");
            bzero(entry, 4092);
        }
    }
    printf("\nPassword: ");
    while(1){
        scanf("%s", entry);
        if(strlen(entry) < 16){
            strcpy(password, entry);
            bzero(entry, 4092);
            break;
        }else{
            printf("\nveuillez entrer un mot de passe faisait 16 caractères ou moins\n");
            bzero(entry, 4092);
        }
    }
    strcpy(args->name, username);
    send(args->sockfd, choice, 1, 0);
    send(args->sockfd, username, 16, 0);
    send(args->sockfd, password, 16, 0);
    bzero(username, 16);
    bzero(password, 16);
    bzero(buffer, LENGTH + 32);
    
}


void recv_msg_handler(void *arguments) {
	char message[LENGTH] = {};
    struct arg_struct *args = arguments;
    // int *sockvalue = (int *) arg;
    // int sockfd = *sockvalue;

    while (1) {
        int receive = recv(args -> sockfd, message, LENGTH, 0);
        if (receive > 0) {
            if(strcmp(message, "3") == 0){
                printf("le compte à bien été supprimé\nVeuillez relancer le client si vous souhaitez requeter le serveur de nouveau");
                kill(getpid(), SIGTERM);
            }
            else if(strcmp(message, "1") == 0){
                printf("le compte existe déjà, veuillez relancer le client, et créer un compte avec un pseudo qui n'a pas déjà été pris");
                kill(getpid(), SIGTERM);
            }
        } else if (receive == 0) {
                break;
        } else {
                kill(getpid(), SIGKILL);
            }
            memset(message, 0, sizeof(message));
    }
    bzero(message, LENGTH);
}


int main(int argc, char **argv){

    struct arg_struct args;
    args.sockfd = 0;
    
    struct sockaddr_in server_addr;
    
	if(argc != 2){
		printf("Veuillez rentrer le port sur lequel le serveur écoute: %s <port>\n", argv[0]);
		kill(getpid(), SIGKILL);
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	/* Socket settings */
	args.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


  // Connect to Server
    int err = connect(args.sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("Erreur de connection\n");
        kill(getpid(), SIGTERM);
    }
    
    getRequest(&args);

	

    pthread_t recv_msg_thread;
    pthread_t send_msg_thread;

    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, (void*)&args) != 0){
		printf("ERROR while using thread for receive messages\n");
		kill(getpid(), SIGTERM);
	}
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, (void*)&args) != 0){
		printf("ERROR while using thread for sending message\n");
        kill(getpid(), SIGTERM);
	}

    while(1){
    
    }
	return EXIT_SUCCESS;
}