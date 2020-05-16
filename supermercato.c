#include <supermercato.h>
//Struct Vars
variables *vars = NULL;

//Struct Messaggio
Messaggio *messaggiBoss = NULL;
Messaggio *messaggiClienti = NULL;
pthread_mutex_t MUTEX_MESSAGGIBOSS = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t COND_MESSAGGIBOSS = PTHREAD_COND_INITIALIZER;
pthread_cond_t COND_BOSS = PTHREAD_COND_INITIALIZER;
pthread_mutex_t MUTEX_MESSAGGICLIENTI = PTHREAD_MUTEX_INITIALIZER;

bool evacuazione = false;
pthread_mutex_t MUTEX_EVACUAZIONE = PTHREAD_MUTEX_INITIALIZER;
bool chiusura = false;
pthread_mutex_t MUTEX_CHIUSURA = PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t MUTEX_ID_CLIENTI = PTHREAD_MUTEX_INITIALIZER;

//Casse
pthread_mutex_t MUTEX_CASSE = PTHREAD_MUTEX_INITIALIZER;

//pthread_t *clientiTID = NULL;

int dilazioneTempo = 1; //set to 1 for default, 10for sec
pthread_t gestoreClientiTID;
pthread_t gestoreCasseTID;


//Apre il file ./tests/text1.txt, legge le variabili e le salva in una struct condivisa
void getVars(char* path){ //Private
    int MAXCHAR = 1000;
    vars = malloc(sizeof(variables)); 
    FILE *fp;
    char str[MAXCHAR];
    fp = fopen(path, "r");
    if (fp == NULL){
        printf("Could not open file %s\n",path);
    }
    fgets(str,MAXCHAR,fp);
    sscanf(str,"K=%d, C=%d, E=%d, T=%d, P=%d, S=%d, TPP=%d, UBT=%d, S1=%d, S2=%d, LO=%d, CAI=%d",&vars->K,&vars->C,&vars->E,&vars->T,&vars->P,&vars->S,&vars->TPP,&vars->UBT,&vars->S1,&vars->S2,&vars->LO,&vars->CAI);
    printf("--------------------- Variabili ---------------------\n");
    printf("| Numero Casse:                               (K)=%d |\n",vars->K);
    printf("| Clienti Massimi Insieme:                   (C)=%d |\n",vars->C);
    printf("| Clienti Minimi per Reinserimento:           (E)=%d |\n",vars->E);
    printf("| Tempo Massimo Acquisti Cliente:           (T)=%d |\n",vars->T);
    printf("| Numero Prodotti Massimi per Cliente:      (P)=%d |\n",vars->P);
    printf("| Tempo Decisione Spostamento Cliente:       (S)=%d |\n",vars->S);
    printf("| Tempo Cassa Per Prodotto:                (TPP)=%d |\n",vars->TPP);
    printf("| Tempo Cassa Per Aggiornare Boss:        (UBT)=%d |\n",vars->UBT);
    printf("| Soglia Minima Coda per Aprire Cassa:       (S1)=%d |\n",vars->S1);
    printf("| Soglia Massima Per Chiudere Cassa:        (S2)=%d |\n",vars->S2);
    printf("| Eseguo i Log su file?:                     (LO)=%d |\n",vars->LO);
    printf("| Numero di casse aperte iniziali:          (CAI)=%d |\n",vars->CAI);
    printf("-----------------------------------------------------\n");
    fclose(fp);
}

//Esegue la pulizia della memoria dinamica (IN PARTE)
void clean(){
    printf("CLEAN)      - Avvio Pulizia\n");
    cleanMessaggi();
    cleanCode();
    cleanClienti();
    pulisciStatoInternoBoss();
    free(vars);
    printf("\033[0;32mCLEAN)      - Pulizia fatta\033[0;90m\n");
    return;
}

pthread_mutex_t MUTEX_CODE = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
    //Inizializzo le variabili
    getVars(argv[1]);

    //Gestione Segnali
    signal(SIGHUP,handle_SIGHUP);
    signal(SIGQUIT,handle_SIGQUIT);

    //Inizializzo le Code
    inizializzaCode();

    //Avvio il processo BOSS
    pthread_t bossTID;
    pthread_create(&bossTID,NULL,&bossProcess,NULL);

    //Avvio il processo Gestore Casse
    pthread_create(&gestoreCasseTID,NULL,&gestoreCasse,NULL);

    //Aspetto un secondo per far creare la prima cassa
    sleep(1); 

    //Avvio il processo Gestore Clienti
    pthread_create(&gestoreClientiTID,NULL,&gestoreClienti,NULL);

    //Avvio il processo BOSS2
    pthread_t boss2;
    pthread_create(&boss2,NULL,&attesaChiusuraThread,NULL);

    //Aspetto la terminazione del processo BOSS
    pthread_join(bossTID,NULL);

    //Eseguo funzione di pulizia   
    clean();

    //Esco dal programma
    return 0;
}