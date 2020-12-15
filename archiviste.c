#include "types.h"

int semap;    /* ID du semaphore    */
int * taille;
int * algo; /*nombre de lecteurs et d'ecrivains*/
id_t * themes;
char ** theme_courant;

void terminaison(int s){
    /*On detache les shm*/
    shmdt(taille);
    shmdt(themes);
    shmdt(algo);
    exit(0);
}

int P(int sem){ 
    struct sembuf op={sem, -1, SEM_UNDO};

    return semop(semap,&op,1);
}

int V(int sem){ 
    struct sembuf op={sem, 1, SEM_UNDO};

    return semop(semap,&op,1);
}


int main(int argc, char * argv[]){
    struct stat st;
    key_t cles[3];
    int smpthemes, smptaille, smpalgo, file_msg, ret_envoi;
    size_t taille_requete;
    requete_t requete;
    reponse_t reponse;
    int mutex_nb_ecrivains , ecriture , avant , lecture , mutex_nb_lecteurs;
    int nb_lecteurs, nb_ecrivains;
    int nb_themes;
    int i;
    

    /*On recupère les paramètres                            */
    nb_themes = atoi(argv[2]);

    fprintf(stderr,"archiviste %d lancé \n", getpid());
    /* Creation de la cle :                                 */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    for(i=0;i<nb_themes+3;i++){
        cles[i] = ftok(FICHIER_CLE,i);
        if (cles[i] == -1){
	        printf("Pb creation cle\n");
	        exit(-1);
        }
    }

    /* On recupere les smp :        */
    smpthemes = shmget(cles[0],sizeof(id_t) * nb_themes,0);
    if (smpthemes==-1){
	    printf("probleme de recuperation du SMPTHEME dans l'archiviste (%d)\n", getpid());
	    exit(-1);
    }
    smptaille = shmget(cles[1],sizeof(int) * nb_themes,0);
    if (smptaille==-1){
	    printf("probleme de recuperation du SMPTAILLE dans l'archiviste (%d)\n", getpid());
	    exit(-1);;
    }
    smpalgo = shmget(cles[2],sizeof(int)*2*nb_themes,0);
    if (smpalgo==-1){
	    printf("probleme de recuperation du SMPALGO dans l'archiviste (%d)\n", getpid());
	    exit(-1);;
    }

    /* Attachement de la memoire partagee :                 */
    themes = shmat(smpthemes, NULL, 0);
    if(themes == (void *)-1){
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }
    taille = shmat(smptaille, NULL, 0);
    if(taille == (void *)-1){
        shmdt(themes);
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }
    algo = shmat(smpalgo, NULL, 0);
    if(algo == (void *)-1){
        shmdt(themes);
        shmdt(taille);
        printf("Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
        exit(-1);
    }

    /* On recupere l'ensemble de semaphore :                */
    semap = semget(cles[0],0,0);
    if (semap==-1){
	    printf("(%d) Pb recuperation semaphore\n",getpid());
	    exit(-1);
    }

    /* Creation file de message :                           */
    file_msg = msgget(cles[0], 0);
    if (file_msg==-1){
	    printf("Pb de recuperation de la file de message dans l'archiviste (%d)\n", getpid());
	    exit(-1);
    }

    mon_sigaction(SIGUSR1, terminaison);

    /* Boucle de traitement des requetes */
    while(0<1){
        /* Archiviste attend des requetes */
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
        
        /*On gère les variables qui dépendent de la position du thème*/
        mutex_nb_ecrivains = requete.theme *5; /*Car chaque theme a 5 semaphores*/
        ecriture = mutex_nb_ecrivains + 1;
        avant = mutex_nb_ecrivains + 2;
        lecture = mutex_nb_ecrivains + 3;
        mutex_nb_lecteurs = mutex_nb_ecrivains + 4;
        nb_ecrivains = requete.theme * 2; /*Car pour chaque theme on dois avoir nb_ecrivains et nb_lecteurs*/
        nb_lecteurs = nb_ecrivains + 1;
        
        /*On attache le smp du theme*/
        theme_courant = shmat(themes[requete.theme],NULL,0);
        if(theme_courant == (void *)-1){
            shmdt(themes);
            shmdt(taille);
            shmdt(algo);
            fprintf(stderr,"Pb attachement du SMP dans l'archiviste (%d)\n", getpid());
            exit(-1);
        }

        /*Traitement de la requete*/
        switch(requete.nature){

            case 'C': /*Consultation donc c'est un lecteur*/
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[requete.theme]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                        
                        P(avant);
                            P(lecture);
                                P(mutex_nb_lecteurs); 
                                    algo[nb_lecteurs]++;
                                    if(algo[nb_lecteurs] == 1) P(ecriture);
                                V(mutex_nb_lecteurs);
                            V(lecture);
                        V(avant);
                        
                        /*lire*/
                        reponse.erreur = 0;
                        strcpy(reponse.contenu, theme_courant[requete.numero]);
                        /*finlire*/
                        
                        P(mutex_nb_lecteurs);  
                            algo[nb_lecteurs]--;
                            if(algo[nb_lecteurs]==0) V(ecriture);
                        V(mutex_nb_lecteurs);
                }
                break;

            case 'P': /*Publication ecrivain*/
                if(requete.numero < TAILLE_MAX_SMP){
                    if(requete.contenu != NULL){ /*On verifie que la requete contient bien le contenu*/
                        
                        P(mutex_nb_ecrivains);
                            algo[nb_ecrivains]++;
                            if(algo[nb_ecrivains] == 1) P(lecture);
                        V(mutex_nb_ecrivains);
                        
                        P(ecriture); 
                        /*ecrire*/
                        theme_courant[taille[requete.theme]] = (char*) malloc(sizeof(char) * 5);
                        strcpy(theme_courant[taille[requete.theme]],requete.contenu);
                        taille[requete.theme] = taille[requete.theme] + 1;
                        reponse.erreur = 0;
                        /*finecrire*/
                        V(ecriture);
                        
                        P(mutex_nb_ecrivains);
                            algo[nb_ecrivains]--;
                            if(algo[nb_ecrivains] == 0) V(lecture);
                        V(mutex_nb_ecrivains);
                    }
                }
                break;

            case 'E': /*Effacement ecrivain*/
                if(requete.numero < TAILLE_MAX_SMP && requete.numero < taille[requete.theme]){ /*On verifie que le numero demandé est bien inferieur au nombre d'articles max*/
                    if(theme_courant[requete.numero] != NULL){ /*On verifie que l'article existe bien*/
                        
                        P(mutex_nb_ecrivains);
                            algo[nb_ecrivains]++;
                            if(algo[nb_ecrivains] == 1) P(lecture);
                        V(mutex_nb_ecrivains);
                        
                        P(ecriture);
                        /*effacement*/
                        theme_courant[requete.numero] = NULL;
                        /*On decale le tableau*/
                        for(int i=requete.numero;i<taille[requete.theme]-1;i++){
                            theme_courant[i] = theme_courant[i+1];
                        }
                        taille[requete.theme]--;
                        reponse.erreur = 0;
                        /*fineffacement*/
                        V(ecriture);
                        
                        P(mutex_nb_ecrivains);
                            algo[nb_ecrivains]--;
                            if(algo[nb_ecrivains] == 0) V(lecture);
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