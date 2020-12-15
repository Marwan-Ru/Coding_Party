#include "types.h"

int semap;    /* ID du semaphore    */
int * taille;
int * algo; /*nombre de lecteurs et d'ecrivains*/
char ** theme;

void terminaison(int s){
    /*On detache les shm*/
    shmdt(taille);
    shmdt(theme);
    shmdt(algo);
    exit(0);
}

int P(int sem){ /*Permet de réserver le theme numero sem*/
    struct sembuf op={sem, -1, SEM_UNDO};

    return semop(semap,&op,1);
}

int V(int sem){ /*Permet de liberer le theme numero sem*/
    struct sembuf op={sem, 1, SEM_UNDO};

    return semop(semap,&op,1);
}


int main(int argc, char * argv[]){
    struct stat st;
    key_t cle1, cle2, cle3;
    int smptheme, smptaille, smpalgo, file_msg, ret_envoi;
    size_t taille_requete;
    requete_t requete;
    reponse_t reponse;
    int mutex_nb_ecrivains = 0, ecriture = 1, avant = 2, lecture = 3, mutex_nb_lecteurs = 4;
    

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
    cle3 = ftok(FICHIER_CLE,'c');
    if (cle3==-1){
	    printf("Pb creation cle\n");
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
    smpalgo = shmget(cle3,sizeof(int)*2,0);
    if (smpalgo==-1){
	    printf("probleme de recuperation du SMPALGO dans l'archiviste (%d)\n", getpid());
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
    algo = shmat(smpalgo, NULL, 0);
    if(algo == (void *)-1){
        shmdt(theme);
        shmdt(taille);
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }

    /* On recupere l'ensemble de semaphore :                */
    semap = semget(cle1,0,0);
    if (semap==-1){
	    printf("(%d) Pb recuperation semaphore\n",getpid());
	    exit(-1);
    }

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

            case 'C': /*Consultation donc c'est un lecteur*/
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[0]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                    if(theme[requete.numero] != NULL){ /*On verifie que l'article existe bien*/
                        
                        P(avant);
                            P(lecture);
                                P(mutex_nb_lecteurs); 
                                    algo[1]++;
                                    if(algo[1] == 1) P(ecriture);
                                V(mutex_nb_lecteurs);
                            V(lecture);
                        V(avant);
                        
                        /*lire*/
                        reponse.erreur = 0;
                        /*strcpy(reponse.contenu, theme[requete.numero]); Provoque un bloquage de facons apparement aléatoire*/
                        /*finlire*/
                        
                        P(mutex_nb_lecteurs);  
                            algo[1]--;
                            if(algo[1]==0) V(ecriture);
                        V(mutex_nb_lecteurs);
                    }
                }
                break;

            case 'P': /*Publication ecrivain*/
                if(requete.numero < TAILLE_MAX_SMP){
                    if(requete.contenu != NULL){ /*On verifie que la requete contient bien le contenu*/
                        
                        P(mutex_nb_ecrivains);
                            algo[0]++;
                            if(algo[0] == 1) P(lecture);
                        V(mutex_nb_ecrivains);
                        
                        P(ecriture);
                        /*ecrire*/
                        theme[taille[0]] = (char*) malloc(sizeof(char) * 5);
                        strcpy(theme[taille[0]],requete.contenu);
                        taille[0] = taille[0] + 1;
                        reponse.erreur = 0;
                        /*finecrire*/
                        V(ecriture);
                        
                        P(mutex_nb_ecrivains);
                            algo[0]--;
                            if(algo[0] == 0) V(lecture);
                        V(mutex_nb_ecrivains);
                    }
                }
                break;

            case 'E': /*Effacement ecrivain*/
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[0]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                    if(theme[requete.numero] != NULL){ /*On verifie que l'article existe bien*/
                        
                        P(mutex_nb_ecrivains);
                            algo[0]++;
                            if(algo[0] == 1) P(lecture);
                        V(mutex_nb_ecrivains);
                        
                        P(ecriture);
                        /*effacement*/
                        theme[requete.numero] = NULL;
                        memcpy(theme[requete.numero], theme[requete.numero + 1], sizeof(char*)*4*(taille[0] - requete.numero - 1)); /*On decale a droite les articles rangés a droite de l'article supprimé */
                        taille[0]--;
                        reponse.erreur = 0;
                        /*fineffacement*/
                        V(ecriture);
                        
                        P(mutex_nb_ecrivains);
                            algo[0]--;
                            if(algo[0] == 0) V(lecture);
                        V(mutex_nb_ecrivains);
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