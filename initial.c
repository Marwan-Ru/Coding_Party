/*
 * Processus père de la coding party 2020
 * Auteur : Marwan Ait Addi
 */
#include "types.h"

int idsmp, file_msg;

void usage(char* s){
    printf("usage : %s <nb_archiviste> <nb_themes>", s);
    exit(-1);
}

void terminaison(int s){
    shmctl(idsmp,IPC_RMID,NULL);
    /*COMPLETER AVEC LENSMEBLE DE SEMAPHORE*/
    msgctl(file_msg,IPC_RMID,NULL);
	exit(-1);
}

void mon_sigaction(int signal, void (*f)(int)){
    struct sigaction action;

    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal, &action, NULL);
}

int main(int argc, char* argv[]){
    int nb_archivistes, nb_themes, i;
    struct stat st;
    key_t cle;
    char ** theme;
    pid_t pid;
    char ordre[10], snb_themes[10];

    /*Gestion des paramètres                                 */
    if(argc != 3){
        usage(argv[0]);
    }
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);
    sprintf(snb_themes,"%d",nb_themes); /*On transforme nb_themes en char* pour le passer en paramètre*/

    /* Creation de la cle :                                  */
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

    /* On cree le SMP et on teste s'il existe deja :        */
    idsmp = shmget(cle,sizeof(char*) * TAILLE_MAX_SMP,IPC_CREAT | IPC_EXCL | 0660);
    if (idsmp==-1){
	    printf("Pb creation SMP ou il existe deja\n");
	    exit(-1);
    }

    /* Attachement de la memoire partagee :                 */
    theme = shmat(idsmp, NULL, 0);
    if(theme == (void *)-1){
        printf("Pb attachement\n");
	    /* Il faut detruire le SMP */
	    shmctl(idsmp,IPC_RMID,NULL);
    }

    /* On cree le semaphore :                               */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* On l'initialise :                                    */
    /*------------------------------------------------------*
     *                                                      *
     *                 A COMPLETER !!!                      *
     *                                                      *
     *------------------------------------------------------*/

    /* Creation file de message :                           */
    file_msg = msgget(cle, IPC_CREAT | IPC_EXCL | 0660);
    if (file_msg==-1){
	    printf("Pb creation de la file de message ou elle existe deja\n");
        /*On detruit les ipc déjà créés*/
        shmctl(idsmp,IPC_RMID,NULL);
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
            sprintf(ordre,"%d",i);
            execl("archiviste",ordre,snb_themes,NULL);
            /* en principe jamais atteint */
            exit(-1);
	    }
    }

    /* On lance indéfiniment des journalistes :             */
    while(0<1){
    }

    terminaison(0);
    exit(0);
}