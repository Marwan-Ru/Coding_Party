/*
 * Processus père de la coding party 2020
 * Auteur : Marwan Ait Addi
 */
#include "types.h"

void usage(char* s){
    printf("usage : %s <nb_archiviste> <nb_themes>", s);
    exit(-1);
}

int main(int argc, char* argv[]){
    int nb_archivistes, nb_themes;
    struct stat st;
    key_t cle;

    /*Gestion des paramètres*/
    if(argc != 3){
        usage(argv[0]);
    }
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);

    /* Creation de la cle :          */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    cle = ftok(FICHIER_CLE,LETTRE_CODE);
    if (cle==-1){
	    printf("Pb creation cle\n");
	    exit(-1);
    }

    /* On cree le SMP et on teste s'il existe deja :    */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Attachement de la memoire partagee :          */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* On cree le semaphore (meme cle) :                     */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* On l'initialise :                                     */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Creation file de message :                          */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Tout est OK, on initialise :                          */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* On lance nb_archivistes archivistes :                 */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* On lance indéfiniment des journalistes :              */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/
    
    exit(0);
}