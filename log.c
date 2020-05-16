#include <supermercato.h>

//Processo di scrittura su ./data/clientiOut.txt
void LOGIT2(char *msg){ //Public
    FILE *logFile;
    int dimBuffer = strlen(msg);
    //Se non esiste lo crea
    if((logFile = fopen("data/clientiOut.txt","a"))==NULL){
        printf("Errore apertura file\n");
        exit(1);
    }
    fwrite(msg,1,dimBuffer,logFile);
    fclose(logFile);
}

//Processo di scrittura su ./data/casseOut.txt
void LOGIT3(char *msg){ //Public
    FILE *logFile;
    int dimBuffer = strlen(msg);
    //Se non esiste lo crea
    if((logFile = fopen("data/casseOut.txt","a"))==NULL){
        printf("Errore apertura file\n");
        exit(1);
    }
    fwrite(msg,1,dimBuffer,logFile);
    fclose(logFile);
}