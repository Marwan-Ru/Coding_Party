#include "types.h"

int main(int argc, char* argv[]){
    fprintf(stderr,"journaliste %d lancé avec pour action %s\n", getpid(), argv[2]);
    exit(0);
}