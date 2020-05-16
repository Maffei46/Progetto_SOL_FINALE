#include <supermercato.h>

int *count;
bool *isOpen;
int numeroAperte;

//Processo BOSS2 - Aspetta la conclusione del "Gestore Casse"
void* attesaChiusuraThread(){ //Public
    printf("GestProc)   - Avviato\n");

    //Assegno un nome al Thread
    pthread_detach(pthread_self());
    pthread_setname_np(pthread_self(),"BOSS2");

    //Attesa terminazione processo "Gestore Casse"
    pthread_join(gestoreCasseTID,NULL);

    printf("GestProc)   - Tutti Processi Terminati\n");
    //Invio messaggio al BOSS di chiusura
    messaggeToBoss(NULL,NULL,0,0);
    printf("GestProc)   - Chiudo\n");

    //Chiudo il Thread
    pthread_exit(0);
}

//Inizializza lo stato interno generando la memoria dinamica
void inizializzaStatoInternoBoss(){ //Private
    count = malloc(sizeof(int)*vars->K);
    isOpen = malloc(sizeof(int)*vars->K);
    int i;
    for(i=0;i<vars->K;i++){
        count[i]= 0;
        isOpen[i]= 0;
    }
    numeroAperte = 0;
}

//Pulisco lo stato interno
void pulisciStatoInternoBoss(){ //Public
    free(count);
    free(isOpen);
}

//Funzione di aggiornamento dello stato interno e di decisione
void updateStatus(Cassa *cassa, int NinCoda){ //Private
    int id = cassa->id;
    isOpen[id] = 1;
    count[id] = NinCoda;

    if(NinCoda >= vars->S2){
        if(numeroAperte >= vars->K){
            return;
        }
        int idAperta = apriCoda();
        if(idAperta!=-1){
            isOpen[idAperta] = 1;
            numeroAperte++;
        }
        
        return;
    }

    int casseConUnCliente = 0;
    int i;
    for(i=0;i<vars->K;i++){
        if(isOpen[i]==1){
            if(count[i]<=1){
                casseConUnCliente++;
            }
        }
    }

    if(casseConUnCliente>= vars->S1){
        if(numeroAperte<=1){
            return;
        }
        chiudiCoda(cassa->id);
        isOpen[cassa->id] = 0;
        numeroAperte--;
        return;
    }

    return;
}

//Thread BOSS - Aspetta dei messaggi e agisce di conseguenza ad essi
void* bossProcess(){ //Public
    printf("Boss)       - Avviato\n");
    pthread_setname_np(pthread_self(),"BOSS");
    inizializzaStatoInternoBoss();
    Messaggio *msg;
    while(true){
        //Attesa Messaggio
        pthread_mutex_lock(&MUTEX_MESSAGGIBOSS);
        while (messaggiBoss == NULL){
            pthread_cond_wait(&COND_BOSS,&MUTEX_MESSAGGIBOSS);
        }
        msg = getMessageBoss();
        pthread_mutex_unlock(&MUTEX_MESSAGGIBOSS);
        
        if(msg!=NULL){
            int codice = msg->codice;
            Cliente *cln = msg->cliente;
            Cassa *cassa = msg->cassa;
            int numeroClientiInCoda = msg->info;
            free(msg);
            if(codice == 0){
                //Messaggio di chiusura da Boss2
                printf("Boss)       - Terminato\n");
                pthread_exit(0);
            }else if(codice == 1){
                //Un Cliente vuole uscire senza prodotti
                messaggeToCliente(cln,1,0);            
            }else if(codice == 2){
                //Aggiornamento stato interno
                updateStatus(cassa,numeroClientiInCoda);
            }
        }
    }
    //Uscita Thread
    pthread_exit(0);
}