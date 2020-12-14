#include <unistd.h>

int main(){
    fprintf(stderr,"archiviste %d lancé \n", getpid());
    pause();
}