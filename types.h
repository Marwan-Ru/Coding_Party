
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

typedef struct
{
    long type;
    pid_t expediteur;
    int nature;
    int theme; /*De 1 a nb_theme */
    int numero; /*Numero de l'article concerné (-1 si inutilisé)*/
    char contenu[4]; /*Contenu de l'article lors de la creation (NULL dans les autre cas)*/
} 
requete_t;


#define FICHIER_CLE "cle.serv"

#define LETTRE_CODE 'a'

#define TAILLE_MAX_SMP 100 /*Un thème peut contenir 100 articles au maximum*/

typedef char article[4];
