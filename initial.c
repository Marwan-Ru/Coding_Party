/*
 * Processus père de la coding party 2020
 * Auteur : Marwan Ait Addi
 */
#include "types.h"

int  nb_archivistes, nb_themes,smpthemes, smptaille, file_msg, semap, smpalgo;
int * taille;
int * algo;
pid_t * themes;
pid_t l_archiviste[MAX_ARCHIVISTE];

void usage(char* s){
    printf("usage : %s <nb_archiviste> <nb_themes>\n", s);
    exit(-1);
}

void terminaison(int s){
    int i;
    /*On envoie un signal a tout les archivistes pour qu'ils se terminent*/
    for(i=0;i<nb_archivistes;i++){
        kill(l_archiviste[i], SIGUSR1);
    }
    for(i=0;i<nb_themes;i++){
        shmctl(themes[i],IPC_RMID,NULL);
    }
    /*On detruit les ipc*/
    shmdt(taille);
    shmdt(themes);
    shmdt(algo);
    shmctl(smpthemes,IPC_RMID,NULL);
    shmctl(smptaille,IPC_RMID,NULL);
    shmctl(smpalgo,IPC_RMID,NULL);
    semctl(semap,0,IPC_RMID,NULL);
    msgctl(file_msg,IPC_RMID,NULL);
	exit(-1);
}

int main(int argc, char* argv[]){
    int i;
    struct stat st;
    key_t * cles;
    pid_t pid;
    char ordre[10], rtheme[10], numero[10], snb_themes[10], snb_archivistes[10];
    int action, res_init;

    /*Gestion des paramètres                                 */
    if(argc != 3){
        usage(argv[0]);
    }
    nb_archivistes = atoi(argv[1]);
    if(nb_archivistes > MAX_ARCHIVISTE) nb_archivistes = MAX_ARCHIVISTE;
    nb_themes = atoi(argv[2]);
    sprintf(snb_themes,"%d",nb_themes); /*On transforme nb_themes en char* pour le passer en paramètre*/
    sprintf(snb_archivistes,"%d",nb_archivistes); /*Idem mais pour les archivistes*/

    /*On initialise le tableau de cles*/
    cles = (key_t *) malloc(sizeof(key_t) * (nb_themes+3));

    /* Creation de la cle :                                  */
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

    /* On cree les SMP et on teste si ils existe deja :        */
    /*Contient les id des themes*/
    smpthemes = shmget(cles[0],sizeof(id_t) * nb_themes,IPC_CREAT | IPC_EXCL | 0660);
    if (smpthemes == -1){
	    printf("Pb creation SMPTHEMES ou il existe deja\n");
	    exit(-1);
    }
    /*Contient la taille des themes*/
    smptaille = shmget(cles[1],sizeof(int)*nb_themes,IPC_CREAT | IPC_EXCL | 0660);
    if (smptaille == -1){
	    printf("Pb creation SMPTAILLE ou il existe deja\n");
        /*On detruit les ipc déjà présents*/
        terminaison(0);
    }
    /*contient le nombre de lecteurs et d'ecrivains*/
    smpalgo = shmget(cles[2] ,sizeof(int)*2*nb_themes,IPC_CREAT | IPC_EXCL | 0660);
    if (smpalgo == -1){
	    printf("Pb creation SMPALGO ou il existe deja\n");
        /*On detruit les ipc déjà présents*/
        terminaison(0);
    }

    /* Attachement de la memoire partagee :                 */
    themes = shmat(smpthemes, NULL, 0);
    if(themes == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire les SMP */
	    terminaison(0);
    }

    taille = shmat(smptaille, NULL, 0);
    if(taille == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire les SMP */
	    terminaison(0);
    }

    algo = shmat(smpalgo, NULL, 0);
    if(algo == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire les SMP */
	    terminaison(0);
    }


    /*On initialise les thèmes*/
    for(i=0;i<nb_themes;i++){
        themes[i] = shmget(cles[i+3],sizeof(char *)*TAILLE_MAX_SMP,IPC_CREAT | IPC_EXCL | 0660);
        if (themes[i] == -1){
            printf("Pb creation SMP ou il existe deja\n");
            terminaison(0);
        }
    }

    /*On initialise la taille*/
    for(i=0;i<nb_themes;i++){
        taille[i] = 0;
    }

    /*On initialise algo*/
    for(i=0;i<nb_themes*2;i++){
        algo[i]=0;
    }

    /* On cree le semaphore :                               */
    semap = semget(cles[0],5*nb_themes,IPC_CREAT | IPC_EXCL | 0660);
    if (semap==-1){
	    printf("Pb creation semaphore ou il existe deja\n");
	    /* Il faut detruire les SMP */
	    terminaison(0);
    }

    for(i=0;i<5*nb_themes;i++){
        res_init = semctl(semap,i,SETVAL,1);
        if (res_init==-1){
            printf("Pb d'init du semaphore\n");
            /* Il faut detruire les SMP */
            terminaison(0);
        }
    }

    /* Creation file de message :                           */
    file_msg = msgget(cles[0], IPC_CREAT | IPC_EXCL | 0660);
    if (file_msg==-1){
	    printf("Pb creation de la file de message ou elle existe deja\n");
        /*On detruit les ipc déjà créés*/
	    terminaison(0);
    }

    /*On capture tout les signaux sauf SIGKILL et SIGCHLD   */
    for(i=1; i<=NSIG; i++){
        if(i != SIGKILL && i != SIGCHLD) /*On affecte l'action terminaison a tout les signaux sauf SIGKILL et SIGCHILD*/
            mon_sigaction(i, terminaison);
    }

    /* On lance nb_archivistes archivistes :                */
    for(i=0;i<nb_archivistes;i++){
	    pid = fork();   
        if (pid==-1)  
            break;
        if (pid==0){
            l_archiviste[i] = pid;
            sprintf(ordre,"%d",i);
            execl("archiviste","archiviste",ordre,snb_themes,NULL);
            /* en principe jamais atteint */
            exit(-1);
	    }
    }

    /* On lance indéfiniment des journalistes :             */
    srand(time(NULL));
    while(0<1){
        action = rand() % 10;
        pid = fork();
        if (pid==-1)
            break;
        if (pid==0){
            /*On cree les parametres des journalistes*/
            sprintf(rtheme,"%d",rand()%nb_themes);
            sprintf(numero,"%d",rand()%TAILLE_MAX_SMP);
            
            if(action < 7){/*7/10 font des demandes de consultation*/
                execl("journaliste","journaliste",snb_archivistes,"C",rtheme, numero,NULL);
            }else if(action < 9){/*2/10 font des demandes de publication d'articles*/
                execl("journaliste","journaliste",snb_archivistes,"P",rtheme,"abcd",NULL);
            }else{/*les autres (1/10) font des demandes d'effacement d'article*/
                execl("journaliste","journaliste",snb_archivistes,"E",rtheme, numero,NULL);
            }
            /* en principe jamais atteint */
            exit(-1);
	    }
    }
    /*Idem, on ne l'atteint jamais mais juste au cas ou*/
    terminaison(0);
    exit(0);
}