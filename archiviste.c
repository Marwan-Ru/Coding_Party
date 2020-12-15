#include "types.h"

int * taille;
char ** theme;

void terminaison(int s){
    /*On detache les shm*/
    shmdt(taille);
    shmdt(theme);
    exit(0);
}

int main(int argc, char * argv[]){
    struct stat st;
    key_t cle1, cle2;
    int smptheme, smptaille, file_msg, ret_envoi;
    size_t taille_requete;
    requete_t requete;
    reponse_t reponse;
    

    /*On recupère les paramètres                            */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    fprintf(stderr,"archiviste %d lancé \n", getpid());
    /* Creation de la cle :                                 */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    cle1 = ftok(FICHIER_CLE,'a');
    if (cle1==-1){
	    printf("Pb creation cle1\n");
	    exit(-1);
    }

    cle2 = ftok(FICHIER_CLE,'b');
    if (cle2==-1){
	    printf("Pb creation cle2\n");
	    exit(-1);
    }

    /* On recupere les smp :        */
    smptheme = shmget(cle1,sizeof(char*) * TAILLE_MAX_SMP,0);
    if (smptheme==-1){
	    printf("probleme de recuperation du SMPTHEME dans l'archiviste (%d)\n", getpid());
	    exit(-1);
    }
    smptaille = shmget(cle2,sizeof(int),0);
    if (smptaille==-1){
	    printf("probleme de recuperation du SMPTAILLE dans l'archiviste (%d)\n", getpid());
	    exit(-1);;
    }

    /* Attachement de la memoire partagee :                 */
    theme = shmat(smptheme, NULL, 0);
    if(theme == (void *)-1){
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }
    taille = shmat(smptaille, NULL, 0);
    if(taille == (void *)-1){
        shmdt(theme);
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }

    /* On recupere le semaphore :                           */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Creation file de message :                           */
    file_msg = msgget(cle1, 0);
    if (file_msg==-1){
	    printf("Pb de recuperation de la file de message dans l'archiviste (%d)\n", getpid());
	    exit(-1);
    }

    mon_sigaction(SIGUSR1, terminaison);

    /* Boucle de traitement des requetes */
    while(0<1){
        /* Qrchiviste attend des requetes */
        taille_requete = msgrcv(file_msg, &requete, sizeof(requete_t) - sizeof(long), 1, 0);
        if(taille_requete == -1){
            fprintf(stderr, "Erreur de reception de la requete \n");
            exit(-1);
        }
        /*On initialise la réponse comme une erreur par défaut*/
        reponse.mtype = requete.expediteur;
        reponse.erreur = -1;

        /*On simule un temps de travail*/
        sleep(rand() % 3);

        /*Traitement de la requete*/
        switch(requete.nature){
            case 'C': /*Consultation*/
                fprintf(stderr,"Ctaille = %d\n", taille[0]);
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[0]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                    if(theme[requete.numero] != NULL){ /*On verifie que l'article existe bien*/
                        reponse.erreur = 0;
                        strcpy(reponse.contenu, theme[requete.numero]);
                    }
                }
                break;
            case 'P': /*Publication*/
                if(requete.numero < TAILLE_MAX_SMP){
                    if(requete.contenu != NULL){ /*On verifie que la requete contient bien le contenu*/
                        theme[taille[0]] = (char*) malloc(sizeof(char) * 4);
                        strncpy(theme[taille[0]],requete.contenu,4); /*On utilise strncpy pour eviter les segmentation fault*/
                        fprintf(stderr,"Ptaille = %d\n", taille[0]);
                        taille[0] = taille[0] + 1;
                        fprintf(stderr,"Ptaille = %d\n", taille[0]);
                        reponse.erreur = 0;
                    }
                }
                break;
            case 'E': /*Effacement*/
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[0]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                    if(theme[requete.numero] != NULL){ /*On verifie que l'article existe bien*/
                        theme[requete.numero] = NULL;
                        memcpy(theme[requete.numero], theme[requete.numero + 1], sizeof(char*)*4*(taille[0] - requete.numero - 1)); /*On decale a droite les articles rangés a droite de l'article supprimé */
                        taille[0]--;
                        fprintf(stderr,"Etaille = %d\n", taille[0]);
                        reponse.erreur = 0;
                    }
                }
                break;
            default:
                fprintf(stderr,"Nature de la requete recue incorrecte (%d)", getpid());
                break;
        }

        ret_envoi = msgsnd(file_msg, &reponse, sizeof(reponse_t) - sizeof(long), 0);
        if(ret_envoi == -1){
            fprintf(stderr, "Erreur d'envoi de la reponse coté serveur \n");
        }
    }

    exit(-1); /*On ne devrais jamais atteindre cette instruction*/
}