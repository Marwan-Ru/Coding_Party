#include "types.h"

int main(int argc, char* argv[]){
    struct stat st;
    key_t cle;
    int file_msg, ret_envoi;
    size_t taille_requete;
    requete_t requete;
    reponse_t reponse;

    fprintf(stderr,"journaliste %d lancé avec pour action %s\n", getpid(), argv[2]);

    /* Creation de la cle :                                 */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    cle = ftok(FICHIER_CLE,'a');
    if (cle==-1){
	    printf("Pb creation cle1\n");
	    exit(-1);
    }

    /* Creation file de message :                           */
    file_msg = msgget(cle, 0);
    if (file_msg==-1){
	    printf("Pb de recuperation de la file de message dans l'archiviste (%d)\n", getpid());
	    exit(-1);
    }

    /*On cree la requête selon les arguments*/

    /*On envoie la requête dans la file de message*/

    /*On attend une réponse*/

    /*On termine propement*/

    exit(0);
}