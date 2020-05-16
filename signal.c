#include <supermercato.h>

//Processo di gestoione segnale di SIGQUIT
void handle_SIGQUIT(int sig){
    printf("\033[0;31m! SIGNAL)   - SIGQUIT -> Evacuazione\033[0;90m\n");
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    evacuazione = true;
    pthread_mutex_unlock(&MUTEX_CHIUSURA);
 
}

//Processo di gestoione segnale di SIGHUP
void handle_SIGHUP(int sig){
    printf("\033[0;31m! SIGNAL)   - SIGHUP -> Chiusura\033[0;90m\n");
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    chiusura = true;
    pthread_mutex_unlock(&MUTEX_CHIUSURA);

}