#include "types.h"

int main(int argc, char* argv[]){
    struct stat st;
    key_t cle;
    int file_msg, ret_envoi;
    size_t taille_reponse;
    requete_t requete;
    reponse_t reponse;
    char action = argv[2][0];

    fprintf(stderr,"journaliste %d lancé avec pour action %s\n", getpid(), argv[2]);

    /* Creation de la cle :                                 */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    cle = ftok(FICHIER_CLE,0);
    if (cle==-1){
	    fprintf(stderr,"Pb creation cle1\n");
	    exit(-1);
    }

    /* Creation file de message :                           */
    file_msg = msgget(cle, 0);
    if (file_msg==-1){
	    fprintf(stderr,"Pb de recuperation de la file de message dans le journaliste (%d)\n", getpid());
	    exit(-1);
    }

    /*On cree la requête selon les arguments*/
    requete.mtype = 1;
    requete.expediteur = getpid();
    requete.nature = action;
    requete.theme = atoi(argv[3]);
    switch(action){
        case 'C': /*Consultation*/
            requete.numero = atoi(argv[4]);
            break;
        case 'P': /*Publication*/
            strcpy(requete.contenu, argv[4]);
            break;
        case 'E': /*Effacement*/
            requete.numero = atoi(argv[4]);
            break;
        default:
            fprintf(stderr,"Erreur dans l'action (%c) du Journaliste (%d)", action, getpid());
            exit(-1);
    }

    /*On envoie la requête dans la file de message*/
    ret_envoi = msgsnd(file_msg, &requete, sizeof(requete_t) - sizeof(long), 0);
    if(ret_envoi == -1){
        fprintf(stderr, "Erreur d'envoi de la requete pour le journaliste (%d)\n", getpid());
    }
    /*On attend une réponse*/
    taille_reponse = msgrcv(file_msg, &reponse, sizeof(reponse_t) - sizeof(long), getpid(), 0);
    if(taille_reponse == -1){
        fprintf(stderr, "Erreur de reception de la reponse pour le journaliste (%d)\n", getpid());
        exit(-1);
    }

    /*On termine*/
    if(reponse.erreur == -1){
        fprintf(stderr,"Le journaliste (%d) termine en recevant une erreur\n", getpid());
    }else{
        printf("Le journaliste (%d) termine en recevant correctement", getpid());
        if(action == 'C') printf(" contenu = %s", reponse.contenu);
        printf("\n");
    }
    exit(0);
}