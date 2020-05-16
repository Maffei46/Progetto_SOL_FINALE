#include <supermercato.h>

Cliente **clienti = NULL; //public
pthread_mutex_t MUTEX_CLIENTI = PTHREAD_MUTEX_INITIALIZER; //public
pthread_mutex_t MUTEX_NUMERO_CLIENTI_ATTIVI = PTHREAD_MUTEX_INITIALIZER; //private
pthread_mutex_t MUTEX_TID_CLIENTE = PTHREAD_MUTEX_INITIALIZER; //TID
pthread_mutex_t MUTEX_NUOVO_CLIENTE = PTHREAD_MUTEX_INITIALIZER;

TIDCLIENTE *firstTIDCliente = NULL;
typedef struct args{
    TIDCLIENTE *tid;
    int id;
}ARGS;

int clientiAttivi;
int numeroClienti;
pthread_mutex_t MUTEX_NUMERO_CLIENTI_TOTALI = PTHREAD_MUTEX_INITIALIZER;

//Ritorna il TID del Cliente
TIDCLIENTE* addTIDCliente(int id){ //Private
    TIDCLIENTE *newTID = malloc(sizeof(TIDCLIENTE));
    newTID->next = NULL;
    newTID->completed = false;
    newTID->id = id;
    pthread_mutex_lock(&MUTEX_TID_CLIENTE);
    if(firstTIDCliente==NULL){
        firstTIDCliente = newTID;
    }else{
        TIDCLIENTE *p = firstTIDCliente;
        while(p->next!=NULL){
            p = p->next;
        }
        p->next = newTID;
    }
    pthread_mutex_unlock(&MUTEX_TID_CLIENTE);
    return newTID;
}

//Rimuove un TID
void removeTIDCliente(int id){ //Private
    pthread_mutex_lock(&MUTEX_TID_CLIENTE);
    TIDCLIENTE *p= firstTIDCliente;
    TIDCLIENTE *prec = NULL;
    if(firstTIDCliente==NULL){
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

    if(p==firstTIDCliente){
        firstTIDCliente = p->next;
        free(p);
    }else if(p!=NULL){
        prec->next = p->next;
        free(p);
    }
    pthread_mutex_unlock(&MUTEX_TID_CLIENTE);
}

//Ritorna un TID
TIDCLIENTE* getTIDCliente(){ //Private
    TIDCLIENTE *p;
    pthread_mutex_lock(&MUTEX_TID_CLIENTE);
    p= firstTIDCliente;
    pthread_mutex_unlock(&MUTEX_TID_CLIENTE);
    return p;
}

//Funzione che ritorna il numero di clienti attivi
int getClientiAttivi(){ //Private
    int ret;
    pthread_mutex_lock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
    ret = clientiAttivi;
    pthread_mutex_unlock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
    return ret;
}

//Processo incremento numero clienti attivi
void incrClientiAttivi(){ //Private
    pthread_mutex_lock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
    clientiAttivi++;
    pthread_mutex_unlock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
}

//Processo decremento numero clienti attivi
void decrClientiAttivi(){ //Private
    pthread_mutex_lock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
    clientiAttivi--;
    pthread_mutex_unlock(&MUTEX_NUMERO_CLIENTI_ATTIVI);
}

//Funzione incremento e ritorno clienti attivi
int getIncrNumeroClienti(){ //Private
    int toRet;
    pthread_mutex_lock(&MUTEX_NUMERO_CLIENTI_TOTALI);
    toRet = numeroClienti;
    numeroClienti++;
    pthread_mutex_unlock(&MUTEX_NUMERO_CLIENTI_TOTALI);
    return toRet;
}

//Processo inizializzazione memoria dinamica per i clienti
void initialClienti(){ //Private
    //Non serve mutex perchè avverrà solo all'inizio quando non ci sono threads
    clientiAttivi = 0;
    numeroClienti = 0;
    if(vars->LO==1){
        char l[100];
        sprintf(l,"| id.Cliente | n.Prodotti | Tempo TOT Super | Tempo TOT in Coda | n.Code Visitate |\n");
        LOGIT2(l);
    }
}

//Processo pulizia della memoria dinamica dei Clienti
void cleanClienti(){ //public
    pthread_mutex_lock(&MUTEX_TID_CLIENTE);
    TIDCLIENTE *p = firstTIDCliente;
    TIDCLIENTE *curr;
    while(p!=NULL){
        if(p->cliente!=NULL){
            free(p->cliente->tempoEntrata);
            if(p->cliente->tempoEntrataCoda!=NULL){
                free(p->cliente->tempoEntrataCoda);
            }
            if(p->cliente->tempoUscitaCoda!=NULL){
                free(p->cliente->tempoUscitaCoda);
            }
            free(p->cliente);
        }
        curr = p;
        p = p->next;
        free(curr);
    }
    pthread_mutex_unlock(&MUTEX_TID_CLIENTE);
}

//Funzione che genera la memoria dinamica per il Cliente
Cliente* creaCliente(int id,pthread_cond_t *RES, TIDCLIENTE* tidP){ //Private
    pthread_mutex_t MUTEX_CLIENTE = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&MUTEX_NUOVO_CLIENTE);
    Cliente *nuovoCliente = malloc(sizeof(Cliente));
    srand(pthread_self()+time(0));
    int Tacquisti = 1+rand()%vars->T;
    int Nprodotti = rand()%vars->P;
    nuovoCliente->id = id;
    nuovoCliente->numeroProdotti = Nprodotti;
    nuovoCliente->tempoEntrata = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME,nuovoCliente->tempoEntrata);

    nuovoCliente->tempoEntrataCoda = NULL;
    nuovoCliente->tempoUscitaCoda = NULL;

    nuovoCliente->tempoRicerca = Tacquisti;
    nuovoCliente->RESPONSE_COND = RES;
    nuovoCliente->TID = tidP;
    nuovoCliente->CLIENTE_MUTEX = MUTEX_CLIENTE;
    nuovoCliente->listaMessaggi = NULL;
    pthread_mutex_unlock(&MUTEX_NUOVO_CLIENTE);
    return nuovoCliente;
}

//Processo che calcola la differenza in tempo
void differceTime(char* ret,struct timespec t1, struct timespec t2){
    long msT1,msT2,msDiff;
    time_t sT1,sT2,sDiff;
    sT1 = t1.tv_sec;
    sT2 = t2.tv_sec;
    msT1 = round(t1.tv_nsec / 1.0e6);
    msT2 = round(t2.tv_nsec / 1.0e6);
    if(msT1>999){sT1++; msT1=msT1-1000;}
    if(msT2>999){sT2++; msT2=msT2-1000;}
    sDiff = sT1 - sT2;
    if(msT1 >= msT2){
        msDiff = msT1 - msT2;
    }else{
        sDiff--;
        long d = msT2-msT1;
        msDiff = 1000-d;
    }
    sprintf(ret,"%ld.%ld",sDiff,msDiff);
}

//Processo di uscita del cliente + log
void uscitaCliente(Cliente* cln,int codUscita){
    char l[100];
    codUscita++;

    if(vars->LO == 1){
        struct timespec tempoAttuale;
        clock_gettime(CLOCK_REALTIME,&tempoAttuale);

        char permanenza[10]; 
        differceTime(permanenza,tempoAttuale,*(cln->tempoEntrata));

        char tempoInCoda[10];
        if(cln->tempoEntrataCoda !=NULL && cln->tempoUscitaCoda != NULL){
            differceTime(tempoInCoda,*(cln->tempoUscitaCoda),*(cln->tempoEntrataCoda));
        }else{
            strcpy(tempoInCoda,"0.000");
        }

                /*| id Cliente | #acquisti | tempo nel super | tempo in coda | code visitate*/
        switch(codUscita){
            case 1: //Pagato
                sprintf(l,"| %d | %d | %s | %s | 1 |\n",cln->id,cln->numeroProdotti,permanenza,tempoInCoda);
                break;
            case 2: //Nessun Prodotto
                sprintf(l,"| %d | 0 | %s | 0.000 | 0 |\n",cln->id,permanenza);
                break;
            case 3: //Chiusura/Evacuazione mentre era in coda
                sprintf(l,"| %d | 0 | %s | %s | 1 |\n",cln->id,permanenza,tempoInCoda);
                break;
            case 4: //Chiusura/Evacuazione mentre non era in coda
                sprintf(l,"| %d | 0 | %s | 0.000 | 0 |\n",cln->id,permanenza);
                break;
            default: 
                sprintf(l,"| %d | err | err | err | err |\n",cln->id);
                break;
        }
        LOGIT2(l);
    }

    pthread_mutex_lock(&MUTEX_TID_CLIENTE);
    cln->TID->completed = true;
    pthread_mutex_unlock(&MUTEX_TID_CLIENTE);

    decrClientiAttivi();
    
    pthread_exit(0); 
}

//Processo per stamapare un messaggio
void printMSG(Messaggio *msg){
    printf("\033[0;31mCliente.%d) GOTMSG: ",msg->cliente->id);
    if(msg->cassa!=NULL){
        printf("Cassa.%d - ",msg->cassa->id);
    }
    printf("Codice(%d) - ",msg->codice);
    printf("Info(%d)\033[0;90m\n",msg->info);
}

//Processo del cliente (Senza Prodotti)
void noProduct(Cliente *cliente){
    Messaggio *msg;

    pthread_mutex_lock(&MUTEX_CHIUSURA);
    if(evacuazione || chiusura){
        pthread_mutex_unlock(&MUTEX_CHIUSURA);
        uscitaCliente(cliente,2);
    }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}

    pthread_mutex_lock(&cliente->CLIENTE_MUTEX);
    messaggeToBoss(cliente,NULL,1,0); 
    while (cliente->listaMessaggi == NULL){
        pthread_cond_wait(cliente->RESPONSE_COND,&cliente->CLIENTE_MUTEX);
    }
    msg = getMessageCliente(cliente);
    pthread_mutex_unlock(&cliente->CLIENTE_MUTEX);
    if(msg == NULL){
        uscitaCliente(cliente,6);
    }
    if(msg->codice != 1){
        printMSG(msg);
    }
    int codice = msg->codice;
    free(msg);

    if(codice == 1){
        uscitaCliente(cliente,1);
    }else{
        uscitaCliente(cliente,6);
    } 
}

//Processo del cliente (Con Prodotti)
void attesaRisposta(Cliente *cliente, bool firstTime){//Private

    Messaggio *msg;
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    if(evacuazione || chiusura){
        pthread_mutex_unlock(&MUTEX_CHIUSURA);
        uscitaCliente(cliente,3);
        return;
    }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}

    int res = insertRandomCoda(cliente);
    if(res != 0){
        //Casse stanno chiudendo
        uscitaCliente(cliente,3);
        return;
    }

    if(firstTime == true){
        cliente->tempoEntrataCoda = malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME,cliente->tempoEntrataCoda); 
    }

    pthread_mutex_lock(&cliente->CLIENTE_MUTEX);
    while (cliente->listaMessaggi == NULL){
        pthread_cond_wait(cliente->RESPONSE_COND,&cliente->CLIENTE_MUTEX);
    }
    msg = getMessageCliente(cliente);
    pthread_mutex_unlock(&cliente->CLIENTE_MUTEX);

    int codice = msg->codice;
    free(msg);

    if(codice == 2){
        cliente->tempoUscitaCoda = malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME,cliente->tempoUscitaCoda);
        uscitaCliente(cliente,0);

    }else if(codice == 3){
        cliente->tempoUscitaCoda = malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME,cliente->tempoUscitaCoda);
        uscitaCliente(cliente,2);
    }else if(codice==4){
        attesaRisposta(cliente,false);
        return;
    }else{
        cliente->tempoUscitaCoda = malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME,cliente->tempoUscitaCoda);
        uscitaCliente(cliente,4);
    }
    
}

//Thread Cliente
void* clienteProcess(void* arg){ //Public
    pthread_detach(pthread_self());
    ARGS *arguments = (ARGS*)arg;
    int id = arguments->id;
    TIDCLIENTE *tid = arguments->tid;
    free(arguments);
    pthread_cond_t RESPONSE_COND = PTHREAD_COND_INITIALIZER;
    Cliente *cliente = creaCliente(id,&RESPONSE_COND,tid);
    tid->cliente = cliente;

    //Aggiungo nome al thread
    char name[12];
    sprintf(name,"Cliente.%d",cliente->id);
    pthread_setname_np(pthread_self(),name);


    //Se stiamo evaquando esco dal supermercato
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    if(evacuazione == true){
        pthread_mutex_unlock(&MUTEX_CHIUSURA);
        uscitaCliente(cliente,3);
    }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}

    //Aspetto il tempo di acquisto: 1millisecond = 1000microseconds
    int attesa = cliente->tempoRicerca * 1000 * dilazioneTempo;
    usleep(attesa);

    //Se stiamo evaquando esco dal supermercato
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    if(evacuazione == true){
        pthread_mutex_unlock(&MUTEX_CHIUSURA);
        uscitaCliente(cliente,3);
    }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}


    //Se numeroProdotti == 0 chiedo di uscire
    if(cliente->numeroProdotti == 0){
        noProduct(cliente);
    }else{
        attesaRisposta(cliente,true);
    }
    return(0);
}

//Processo avvio nuovo cliente
void nuovoCliente(){ //Private
    incrClientiAttivi();
    int x = getIncrNumeroClienti();
    TIDCLIENTE *tid = addTIDCliente(x);
    ARGS *args = malloc(sizeof(ARGS));
    args->tid = tid;
    args->id = x;
    pthread_create(&tid->tidCliente,NULL,&clienteProcess,(void *)args);
}

//Thread Gestore Clienti
void *gestoreClienti(){ //Public
    //Inizializzo i clienti
    pthread_setname_np(pthread_self(),"GestoreClienti");
    initialClienti();
    int i;
    printf("GestClient) - Avviato\n");

    //Creo i primi K clienti(Threads)
    printf("\033[0;32mGestClient) - %d Nuovi Clienti\033[0;90m\n",vars->C);
    for(i=0;i<vars->C;i++){
        nuovoCliente();
    }

    while(true){
        int clnAtt = getClientiAttivi();
        pthread_mutex_lock(&MUTEX_CHIUSURA);
        if( (clnAtt<vars->E) && (chiusura==false) && (evacuazione==false)){
            pthread_mutex_unlock(&MUTEX_CHIUSURA);
            int missing = vars->C-clnAtt;
            printf("\033[0;32mGestClient) - %d Nuovi Clienti\033[0;90m\n",missing);
            for(i=0;i<missing;i++){
                nuovoCliente();
            }
        }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}

        pthread_mutex_lock(&MUTEX_CHIUSURA);
        if((chiusura==true) || (evacuazione==true)){
            pthread_mutex_unlock(&MUTEX_CHIUSURA);

            int count = getClientiAttivi();
            if(count == 0){
                printf("GestClient) - Chiudo\n");
                pthread_exit(0);
            }
        }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}
        //Il Controllo lo esegue ogni secondo altrimenti occupa le mutex in continuo
        sleep(1);
        
    }

    return(0);
}