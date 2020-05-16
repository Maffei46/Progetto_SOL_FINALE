#include <supermercato.h>

TIDCASSA *firstTIDcassa;
pthread_mutex_t MUTEX_TIDCASSE = PTHREAD_MUTEX_INITIALIZER; //TID PRIVATE
pthread_mutex_t MUTEX_NUMERO_CASSE_ATTIVE = PTHREAD_MUTEX_INITIALIZER; //private

typedef struct casseLogs{ //Private Struct
    int numeroProdottiElaborati;
    int numeroClientiElaborati;
    time_t tempoTotaleAperturaSecondi;
    long tempoTotaleAperturaMillisecondi;
    time_t tempoTotaleServiziSecondi;
    long tempoTotaleServiziMillisecondi;
    int numeroChiusure;
}CasseLogs;

CasseLogs **LogsCasse = NULL; //Private List

typedef struct args{
    TIDCASSA *tid;
    int id;
}ARGS;

int casseAttive;

//Ritorna il numero di casse attive
int getCasseAttive(){ //Private
    int ret;
    pthread_mutex_lock(&MUTEX_NUMERO_CASSE_ATTIVE);
    ret = casseAttive;
    pthread_mutex_unlock(&MUTEX_NUMERO_CASSE_ATTIVE);
    return ret;
}

//Incrementa il numero di casse attive
void incrCasseAttive(){ //Private
    pthread_mutex_lock(&MUTEX_NUMERO_CASSE_ATTIVE);
    casseAttive++;
    pthread_mutex_unlock(&MUTEX_NUMERO_CASSE_ATTIVE);
}

//Decrementa il numero di casse attive
void decrCasseAttive(){ //Private
    pthread_mutex_lock(&MUTEX_NUMERO_CASSE_ATTIVE);
    casseAttive--;
    pthread_mutex_unlock(&MUTEX_NUMERO_CASSE_ATTIVE);
}

//Aggiunge un TID
TIDCASSA* addTID(int id){ //Private
    TIDCASSA *newTID = malloc(sizeof(TIDCASSA));
    newTID->next = NULL;
    newTID->id = id;
    newTID->completed = false;
    pthread_mutex_lock(&MUTEX_TIDCASSE);
    if(firstTIDcassa==NULL){
        firstTIDcassa = newTID;
    }else{
        TIDCASSA *p = firstTIDcassa;
        while(p->next!=NULL){
            p = p->next;
        }
        p->next = newTID;
    }
    pthread_mutex_unlock(&MUTEX_TIDCASSE);
    return newTID;
}

//Rimuove un TID
void removeTID(int id){ //Private
    pthread_mutex_lock(&MUTEX_TIDCASSE);
    TIDCASSA *p= firstTIDcassa;
    TIDCASSA *prec = NULL;
    if(firstTIDcassa==NULL){
        printf("No TID Avaible\n");
        return;
    }

    while(p!=NULL){
        if(p->id == id){
            break;
        }
        prec = p;
        p = p->next;
    }

    if(p==firstTIDcassa){
        firstTIDcassa = p->next;
        free(p);
    }else if(p!=NULL){
        prec->next = p->next;
        free(p);
    }
    pthread_mutex_unlock(&MUTEX_TIDCASSE);
}

//Ritorna il firstTIDcassa
TIDCASSA* getTID(){ //Private
    TIDCASSA *p;
    pthread_mutex_lock(&MUTEX_TIDCASSE);
    p= firstTIDcassa;
    pthread_mutex_unlock(&MUTEX_TIDCASSE);
    return p;
}

//Funzione che genera la media del tempo di servizio
float trovaMedia(int numClienti, time_t Secondi, long Millisecondi){ //Private
    if(numClienti<=0){
        return 0;
    }
    float f = Millisecondi;
    while(f>=1){
        f /= 10;
    }
    f += Secondi;
    f = f/numClienti;
    return f;
}

//Pulisce ed esegue il log
void cleanCasse(){ //Private
    pthread_mutex_lock(&MUTEX_TIDCASSE);
    TIDCASSA *p = firstTIDcassa;
    TIDCASSA *curr;
    while(p!=NULL){
        if(p->cassa!=NULL){
            free(p->cassa->tempoEntrata);
            free(p->cassa);
        }
        curr = p;
        p = p->next;
        free(curr);
    }
    pthread_mutex_unlock(&MUTEX_TIDCASSE);
    if(vars->LO == 0){return;}
    int i;
    char l[50];
    
    for(i=0; i<vars->K ;i++){
        float media = trovaMedia(LogsCasse[i]->numeroClientiElaborati,LogsCasse[i]->tempoTotaleServiziSecondi,LogsCasse[i]->tempoTotaleServiziMillisecondi);
        sprintf(l,"| %d | %d | %d | %ld.%ld | %.3f | %d |\n",i,LogsCasse[i]->numeroProdottiElaborati,LogsCasse[i]->numeroClientiElaborati,LogsCasse[i]->tempoTotaleAperturaSecondi,LogsCasse[i]->tempoTotaleAperturaMillisecondi,media,LogsCasse[i]->numeroChiusure);
        LOGIT3(l);    
        free(LogsCasse[i]);
    }
    free(LogsCasse);
}

//Crea la memoria dinamica per la cassa
Cassa* creaCassa(int id,pthread_cond_t *RES, TIDCASSA* tidP){ //Private
    Cassa *cassa = malloc(sizeof(Cassa));
    cassa->id = id;
    cassa->RESPONSE_COND = RES;
    cassa->chiudiAllaFine = false;
    cassa->chiudiSubito = false;
    cassa->TID = tidP;
    cassa->tempoServiziSecondi = 0;
    cassa->tempoServiziMillisecondi = 0;
    cassa->numeroProdottiElaborati = 0;
    cassa->numeroDiClienti = 0;
    cassa->tempoEntrata = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME,cassa->tempoEntrata);

    srand(pthread_self()+time(0));
    cassa->tempoLavoro = 20+rand()%61;
    return cassa;
}

//Processo per l'incremento del tempo totale di servizio
void incrTempoTotaleServizi(Cassa *cassa){ //Private
    LogsCasse[cassa->id]->tempoTotaleServiziSecondi += cassa->tempoServiziSecondi;
    long tot = LogsCasse[cassa->id]->tempoTotaleServiziMillisecondi + cassa->tempoServiziMillisecondi;
    if(tot>999){
        LogsCasse[cassa->id]->tempoTotaleAperturaSecondi++;
        tot = tot - 1000;
    }
    LogsCasse[cassa->id]->tempoTotaleServiziMillisecondi = tot; 
}

//Processo per l'incremento del tempo totale di apertura
void incrTempoTotaleApertura(Cassa *cassa){ //Private
    struct timespec tempoAttuale;
    clock_gettime(CLOCK_REALTIME,&tempoAttuale);

    time_t sT = tempoAttuale.tv_sec;
    long msT = round(tempoAttuale.tv_nsec / 1.0e6);
    time_t scT = cassa->tempoEntrata->tv_sec;
    long mscT = round(cassa->tempoEntrata->tv_nsec / 1.0e6);

    time_t secTotal = sT-scT;
    long msTotal;
    if(mscT >= msT){
        msTotal = mscT - msT;
    }else{
        long diff = msT - mscT;
        msTotal = 1000-diff;
        secTotal--;
    }

    LogsCasse[cassa->id]->tempoTotaleAperturaSecondi += secTotal;
    long tot = LogsCasse[cassa->id]->tempoTotaleAperturaMillisecondi + msTotal;
    if(tot>999){
        LogsCasse[cassa->id]->tempoTotaleAperturaSecondi++;
        tot = tot - 1000;
    }
    LogsCasse[cassa->id]->tempoTotaleAperturaMillisecondi = tot;
}

//Processo di chiusura di un processo Cassa
void chiudiProcessoCassa(Cassa *cassa){ //Private
    //Aggiorno il logCasse
    if(vars->LO==1){
        LogsCasse[cassa->id]->numeroChiusure++;
        LogsCasse[cassa->id]->numeroClientiElaborati += cassa->numeroDiClienti;
        LogsCasse[cassa->id]->numeroProdottiElaborati += cassa->numeroProdottiElaborati;
        incrTempoTotaleApertura(cassa);
        incrTempoTotaleServizi(cassa);
    }


    printf("\033[0;31m- Cassa.%d)  - Chiusa\033[0;90m\n",cassa->id);

    pthread_mutex_lock(&MUTEX_TIDCASSE);
    cassa->TID->completed = true;
    pthread_mutex_unlock(&MUTEX_TIDCASSE);

    decrCasseAttive();
    pthread_exit(0);
}

//Processo di richiesta chiusura di una cassa
void chiudiCassa(Cassa *cassa){ //Private
    cassa->chiudiAllaFine = true;
}

//Processo incremento del tempo di servizio
void incrTempoServizio(Cassa *cassa,struct timespec tempoPre){ //Private
    time_t preSec = tempoPre.tv_sec;
    long preMs = round(tempoPre.tv_nsec / 1.0e6);

    struct timespec tempoAttuale;
    clock_gettime(CLOCK_REALTIME,&tempoAttuale);
    
    time_t attSec = tempoAttuale.tv_sec; 
    long attMs = round(tempoAttuale.tv_nsec / 1.0e6);
    
    time_t totSec = attSec - preSec;
    long totMs;
    if(attMs < preMs){
        totSec--;
        long diff = preMs-attMs;
        totMs = 1000-diff;
    }else{
        totMs = attMs - preMs;
    }

    cassa->tempoServiziSecondi = cassa->tempoServiziSecondi + totSec;
    long tot = cassa->tempoServiziMillisecondi + totMs;
    if(tot>999){
        cassa->tempoServiziSecondi++;
        tot = tot-1000;
    }
    cassa->tempoServiziMillisecondi = tot;
}

//Thread Cassa
void* processoCassa(void* arg){ //Private
    pthread_detach(pthread_self());
    //Prendo l'id
    ARGS *arguments = (ARGS*)arg;
    int id = arguments->id;
    TIDCASSA *tid = arguments->tid;
    free(arguments);

    //Aggiungo nome al processo
    char name[10];
    sprintf(name,"Cassa.%d",id);
    pthread_setname_np(pthread_self(),name);


    //Genero la cassa
    pthread_cond_t RESPONSE_COND = PTHREAD_COND_INITIALIZER;
    Cassa* cassa = creaCassa(id,&RESPONSE_COND,tid);
    tid->cassa = cassa;
    
    //Stampo su console
    printf("\033[0;32m+ Cassa.%d)  - Aperta\033[0;90m\n",id);

    //Processo effettivo
    time_t t_pre = time(NULL);
    time_t now;

    bool chiudendoCassa = false;

    while(true){
        struct timespec tempoPre;
        clock_gettime(CLOCK_REALTIME,&tempoPre);
        ElementoCoda *elementoCoda = getCliente(cassa->id);
        
        if(elementoCoda != NULL){
            int msg = elementoCoda->msg;
            Cliente *cln = elementoCoda->cliente;
            free(elementoCoda);
            if(msg == 1){
                //Direttore richiede la chiusura della cassa
                chiudendoCassa = true;                
            }else if(cln!=NULL){
                if(chiudendoCassa==true){
                    //Cassa è stata chiusa avverto i Clienti in coda
                    messaggeToCliente(cln,4,0);
                }else{    
                    pthread_mutex_lock(&MUTEX_CHIUSURA);
                    if(evacuazione == true){
                        pthread_mutex_unlock(&MUTEX_CHIUSURA);
                        //Stiamo evacuando
                        messaggeToCliente(cln,3,0);
                    }else{
                        //Pagamento
                        pthread_mutex_unlock(&MUTEX_CHIUSURA);
                        int attesa = (cassa->tempoLavoro + (vars->TPP * cln->numeroProdotti)) * 1000 * dilazioneTempo;
                        usleep(attesa);
                        if(vars->LO == 1){
                            cassa->numeroDiClienti++;
                            cassa->numeroProdottiElaborati += cln->numeroProdotti;
                            incrTempoServizio(cassa,tempoPre);
                        }
                        messaggeToCliente(cln,2,0); //2=Pagato
                    }
                }

                

            }
        }else{
            //Non ci sono elementi in coda
            if(chiudendoCassa==true){
                //Chiusura del processo
                chiudiProcessoCassa(cassa);
            }
            pthread_mutex_lock(&MUTEX_CHIUSURA);
            if(chiusura==true || evacuazione==true){

                pthread_mutex_lock(&code[cassa->id]->CODA_MUTEX);
                if(code[cassa->id]->first == NULL){
                    code[cassa->id]->aperta = false;
                    pthread_mutex_unlock(&code[cassa->id]->CODA_MUTEX);
                    pthread_mutex_unlock(&MUTEX_CHIUSURA);
                    chiudiProcessoCassa(cassa);
                }else{
                    pthread_mutex_unlock(&code[cassa->id]->CODA_MUTEX);
                    pthread_mutex_unlock(&MUTEX_CHIUSURA);
                }
                
            }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}
            sleep(1);
        }
        //Aggiorno il boss
        now = time(NULL);
        int diff = (now - t_pre)*100;
        if(diff >= vars->UBT){
            pthread_mutex_lock(&MUTEX_CHIUSURA);
            if(chiusura==false && evacuazione==false){
                pthread_mutex_unlock(&MUTEX_CHIUSURA);
                int count = 0;
                pthread_mutex_lock(&code[cassa->id]->CODA_MUTEX);
                ElementoCoda *p = code[cassa->id]->first;
                while(p!=NULL){
                    count++;
                    p = p->next;
                }
                pthread_mutex_unlock(&code[cassa->id]->CODA_MUTEX);
                messaggeToBoss(NULL,cassa,2,count);
                
            }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}

            t_pre = time(NULL);
        }
        
    }
    return (0);
}

//Processo che crea una cassa
void nuovaCassa(int i){ //Pubblica
    incrCasseAttive();
    int x = i;
    TIDCASSA *tid = addTID(x);
    ARGS *args = malloc(sizeof(ARGS));
    args->tid = tid;
    args->id = x;
    pthread_create(&tid->tidCassa,NULL,&processoCassa,(void *)args);
}

//Processo di inizializzazione di una cassa
void initialCasse(){ //Private
    firstTIDcassa = NULL;
    casseAttive = 0;
    if(vars->LO == 0){return;}
    char l[100];
    sprintf(l,"| id.Cassa | n.Prodotti | n.Clienti | Tempo TOT Apertura | Tempo Medio di Servizio | n.Chiusure |\n");
    LOGIT3(l);
    LogsCasse = malloc(sizeof(CasseLogs*)*vars->K);
    int i;
    for(i=0;i<vars->K;i++){
        LogsCasse[i] = malloc(sizeof(CasseLogs));
        LogsCasse[i]->numeroChiusure = 0;
        LogsCasse[i]->numeroProdottiElaborati = 0;
        LogsCasse[i]->numeroClientiElaborati = 0;
        LogsCasse[i]->tempoTotaleAperturaMillisecondi = 0;
        LogsCasse[i]->tempoTotaleAperturaSecondi = 0;
        LogsCasse[i]->tempoTotaleServiziMillisecondi = 0;
        LogsCasse[i]->tempoTotaleServiziSecondi = 0;
    }
}

//Thread Gestore Casse
void *gestoreCasse(){ //Public
    //Inizializzo i clienti
    pthread_setname_np(pthread_self(),"GestoreCasse");
    initialCasse();
    printf("GestCasse)  - Avviato\n");
    
    //Apro di default una coda/cassa;
    int i;
    for(i=0;i<vars->CAI;i++){
        apriCoda();
    }
 
    //Processo in corso
    while(true){
        pthread_mutex_lock(&MUTEX_CHIUSURA);
        if((chiusura==true) || (evacuazione==true)){
            pthread_mutex_unlock(&MUTEX_CHIUSURA);

            int count = getCasseAttive();
            if(count == 0){
                pthread_join(gestoreClientiTID,NULL);
                cleanCasse();
                printf("GestCasse)  - Chiudo\n");
                pthread_exit(0);
            }

        }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}
        sleep(1);
    }
    
}