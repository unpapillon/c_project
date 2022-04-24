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

struct client {
    int sockfd;
    char name[16];
};

void sendingMsg(void *arguments) {
    char message[LENGTH] = {};
    struct client *args = arguments;
    printf("=== BIENVENU SUR LE CHAT ===\n");
    printf("vous pouvez envoyer des messages désormais %s: \nsi vous souhaitez vous déconnecter, faites ctrl+c, ou bien tapez exit",args -> name);
    while(1) {
        printf("\n->");
        //l'utilisateur rentre le message qu'il souhaite envoyer
        fgets(message, LENGTH, stdin);
        //on vérifie que le message n'est pas vide, pour ne pas envoyer de données inutiles
        if(strlen(message) > 0 && strcmp(message, "\n") !=0){
            send(args -> sockfd, message, strlen(message), 0);
        }else if(strlen(message) < 0 || strlen(message)> LENGTH){
            //si les conditions ne sont pas vérifiées, alors on interromp le programme
            kill(getpid(), SIGINT);
        }
        if(strcmp(message, "exit\n") == 0){
            //si le mot exit est tapé, alors on déconnecte, tant bien du coté serveur (CF serv.c) que du coté client.
            printf("vous avez bien été déconnecté du serveur\n");
            kill(getpid(), SIGTERM);
        }

        }
        //on vide l'espace mémoire du message pour le prochain
        bzero(message, LENGTH);
}



void getRequest(struct client *args){
    char choice[2] = "";
    char username[16];
    char password[16];
    char buffer[LENGTH + 32] = {};
    char entry[4092];
    printf("Vous etes sur le point de rentrer dans le chat:\n-si vous voulez créer un compte, tapez 1\n-si vous voulez vous connecter, tapez 2\n-si vous voulez supprimer un compte, tapez 3\n");
    //vérification des input pour éviter l'overflow de mémoire
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
    if(strcmp(choice, "1") == 0){
        printf("veuillez définir vos credentials, 16 caractères chacuns au maximum\n");
    }
    if(strcmp(choice, "2") == 0){
        printf("veuillez entrer votre pseudo et votre mot de passe\n");
    }
    if(strcmp(choice, "3") == 0){
        printf("veuillez entrer les credentials du compte à supprimer\n");
    }
    //vérification des input pour éviter l'overflow de mémoire
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
    //on envoie le type de requete, ainsi que les credentials rentrées, puis on efface l'espace mémoire utilisé
    strcpy(args->name, username);
    send(args->sockfd, choice, 1, 0);
    send(args->sockfd, username, 16, 0);
    send(args->sockfd, password, 16, 0);
    bzero(username, 16);
    bzero(password, 16);
    bzero(buffer, LENGTH + 32); 
}


void receptionMsg(void *arguments) {
	char message[LENGTH] = {};
    struct client *args = arguments;
    //traitement des réponses provenant du serveur suite aux requetes envoyées par le client
    while (1) {
        int receive = recv(args -> sockfd, message, LENGTH, 0);
        if (receive > 0) {
            if(strcmp(message, "3") == 0){
                printf("le compte à bien été supprimé\nVeuillez relancer le client si vous souhaitez requeter le serveur de nouveau\n");
                kill(getpid(), SIGTERM);
            }
            else if(strcmp(message, "1") == 0){
                printf("le compte existe déjà, veuillez relancer le client, et créer un compte avec un pseudo qui n'a pas déjà été pris\n");
                kill(getpid(), SIGTERM);
            }
            else if (strcmp(message, "2") == 0)
            {
                printf("mauvais pseudo ou mot de passe, veuillez relancer le client et tenter de vous connecter avec les bons identifiants\n");
                kill(getpid(), SIGTERM);
            }
            
        } else if (receive == 0) {
                break;
        } else {
            kill(getpid(), SIGKILL);
        }
    }
    bzero(message, LENGTH);
}


int main(int argc, char **argv){

    struct client args;
    args.sockfd = 0;
    pthread_t receptionThread;
    pthread_t sendingThread;
    struct sockaddr_in serveur;
    
	if(argc != 2){
		printf("Veuillez rentrer le port sur lequel le serveur écoute: %s <port>\n", argv[0]);
		kill(getpid(), SIGKILL);
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	//paramètre basiques pour la connection de la socket
	args.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveur.sin_family = AF_INET;
    serveur.sin_addr.s_addr = inet_addr(ip);
    serveur.sin_port = htons(port);


  // connection au serveur sur le port donné, à l'addresse données
    int err = connect(args.sockfd, (struct sockaddr *)&serveur, sizeof(serveur));
    if (err == -1) {
        printf("Erreur de connection\n");
        kill(getpid(), SIGTERM);
    }
    
    getRequest(&args);

    
    //mise en place d'un thread pour la fonction de reception des messages de la part du serveur
    if(pthread_create(&receptionThread, NULL, (void *) receptionMsg, (void*)&args) != 0){
		printf("ERROR while using thread for receive messages\n");
		kill(getpid(), SIGTERM);
	}
    //mise en place d'un autre thread pour l'envoi des messages
    if(pthread_create(&sendingThread, NULL, (void *) sendingMsg, (void*)&args) != 0){
		printf("ERROR while using thread for sending message\n");
        kill(getpid(), SIGTERM);
	}

    while(1){
    
    }
	return EXIT_SUCCESS;
}