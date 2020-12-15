/*
 * Processus père de la coding party 2020
 * Auteur : Marwan Ait Addi
 */
#include "types.h"

int  nb_archivistes, smptheme, smptaille, file_msg;
int *taille;
char ** theme;
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
    /*On detruit les ipc*/
    shmdt(taille);
    shmdt(theme);
    shmctl(smptheme,IPC_RMID,NULL);
    shmctl(smptaille,IPC_RMID,NULL);
    /*COMPLETER AVEC LENSMEBLE DE SEMAPHORE*/
    msgctl(file_msg,IPC_RMID,NULL);
	exit(-1);
}

int main(int argc, char* argv[]){
    int nb_themes, i;
    struct stat st;
    key_t cle1, cle2;
    pid_t pid;
    char ordre[10], rtheme[10], numero[10], snb_themes[10], snb_archivistes[10];
    int action;

    /*Gestion des paramètres                                 */
    if(argc != 3){
        usage(argv[0]);
    }
    nb_archivistes = atoi(argv[1]);
    if(nb_archivistes > MAX_ARCHIVISTE) nb_archivistes = MAX_ARCHIVISTE;
    nb_themes = atoi(argv[2]);
    sprintf(snb_themes,"%d",nb_themes); /*On transforme nb_themes en char* pour le passer en paramètre*/
    sprintf(snb_archivistes,"%d",nb_archivistes); /*Idem mais pour les archivistes*/

    /* Creation de la cle :                                  */
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    if ((stat(FICHIER_CLE,&st) == -1) &&
	(open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1)){
	    fprintf(stderr,"Pb creation fichier cle\n");
	    exit(-1);
    }

    cle1 = ftok(FICHIER_CLE,'a');
    if (cle1==-1){
	    printf("Pb creation cle\n");
	    exit(-1);
    }
    cle2 = ftok(FICHIER_CLE,'b');
    if (cle2==-1){
	    printf("Pb creation cle\n");
	    exit(-1);
    }

    /* On cree les SMP et on teste si ils existe deja :        */
    smptheme = shmget(cle1,sizeof(char*) * TAILLE_MAX_SMP,IPC_CREAT | IPC_EXCL | 0660);
    if (smptheme == -1){
	    printf("Pb creation SMP ou il existe deja\n");
	    exit(-1);
    }
    smptaille = shmget(cle2,sizeof(int),IPC_CREAT | IPC_EXCL | 0660);
    if (smptaille == -1){
	    printf("Pb creation SMP ou il existe deja\n");
        /*On detruit les ipc déjà présents*/
        shmctl(smptheme,IPC_RMID,NULL);
	    exit(-1);
    }

    /* Attachement de la memoire partagee :                 */
    theme = shmat(smptheme, NULL, 0);
    if(theme == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire les SMP */
	    shmctl(smptheme,IPC_RMID,NULL);
        shmctl(smptaille,IPC_RMID,NULL);
    }

    taille = shmat(smptaille, NULL, 0);
    if(taille == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire les SMP */
        shmdt(theme);
	    shmctl(smptheme,IPC_RMID,NULL);
        shmctl(smptaille,IPC_RMID,NULL);
    }

    /*On initialise la taille*/
    taille[0] = 0;
    /* On cree le semaphore :                               */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Creation file de message :                           */
    file_msg = msgget(cle1, IPC_CREAT | IPC_EXCL | 0660);
    if (file_msg==-1){
	    printf("Pb creation de la file de message ou elle existe deja\n");
        /*On detruit les ipc déjà créés*/
        shmdt(taille);
        shmdt(theme);
        shmctl(smptheme,IPC_RMID,NULL);
        shmctl(smptaille,IPC_RMID,NULL);
        /*COMPLETER AVEC LENSEMBLE DE SEMAPHORE*/
        msgctl(file_msg,IPC_RMID,NULL);
	    exit(-1);
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
    while(0<1){
        sleep((rand() % 2) +1);
        action = rand() % 10;
        pid = fork();
        if (pid==-1)
            break;
        if (pid==0){
            /*On cree les parametres des journalistes*/
            sprintf(rtheme,"%d",rand()%nb_themes);
            sprintf(numero,"%d",rand()%2);
            
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

    terminaison(0);
    exit(0);
}