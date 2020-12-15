
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

/*Structures utilisées dans la file de message*/
typedef struct
{
    long mtype;
    pid_t expediteur;
    int nature; /*C : Consultation, P : Publication, E : Effacement*/
    int theme; /*De 1 a nb_theme */
    int numero; /*Numero de l'article concerné (-1 si inutilisé)*/
    char contenu[4]; /*Contenu de l'article lors de la creation (NULL dans les autre cas)*/
} 
requete_t;

typedef struct{
    long mtype;
    int erreur; /*-1 si il y a eu une erreur*/
    char contenu[4]; /*Contenu renvoyé lors d'une consultation, NULL dans les autres cas*/
}
reponse_t;

/*Constantes*/
#define FICHIER_CLE "cle.serv"

#define TAILLE_MAX_SMP 100 /*Un thème peut contenir 100 articles au maximum*/

#define MAX_ARCHIVISTE 1000 /*Nombre maximum d'archiviste*/

typedef char article[4];


/*Fonctions utilisées par les trois programmes*/
void mon_sigaction(int signal, void (*f)(int)){ /*Fonction du cours pour la capture de signaux*/
    struct sigaction action;

    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal, &action, NULL);
}