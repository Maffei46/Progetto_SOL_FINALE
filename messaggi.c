#include <supermercato.h>

//Inserisce un messaggio alla lista messaggi del boss
void messaggeToBoss(Cliente *cln, Cassa *cassa, int codice,int info){ //public
    Messaggio *newMsg = malloc(sizeof(Messaggio));
    newMsg->cliente = cln;
    newMsg->cassa = cassa;
    newMsg->codice = codice;
    newMsg->info = info;
    newMsg->next = NULL;
    pthread_mutex_lock(&MUTEX_MESSAGGIBOSS);
    if(messaggiBoss == NULL){
        messaggiBoss = newMsg;
    }else{
        Messaggio *p = messaggiBoss;
        while(p->next!=NULL){
            p = p->next;
        }
        p->next = newMsg;
        p = NULL;
    }
    pthread_cond_signal(&COND_BOSS);
    pthread_mutex_unlock(&MUTEX_MESSAGGIBOSS);
}   

//Ritorna un messaggio dalla lista dei messaggi per il Boss
Messaggio* getMessageBoss(){ //Public
    if(messaggiBoss == NULL){
        return NULL;
    }
    Messaggio *toRet = messaggiBoss;
    messaggiBoss = toRet->next;
    return toRet;
}

//Inserisce un messaggio al cliente
void messaggeToCliente(Cliente *cln, int codice,int info){ //public
    if(cln == NULL){
        printf("MessageToCliente: cln==NULL\n");
        return;
    }
    
    Messaggio *newMsg = malloc(sizeof(Messaggio));
    newMsg->cliente = cln;
    newMsg->codice = codice;
    newMsg->info = info;
    newMsg->next = NULL;

    pthread_mutex_lock(&cln->CLIENTE_MUTEX);
    if(cln->listaMessaggi == NULL){
        cln->listaMessaggi = newMsg;
    }else{
        Messaggio *p = cln->listaMessaggi;
        while(p->next!=NULL){
            p = p->next;
        }
        p->next = newMsg;
    }
    pthread_cond_signal(cln->RESPONSE_COND);
    pthread_mutex_unlock(&cln->CLIENTE_MUTEX);
}

//Prende il primo messaggio dalla lista messaggi di un cliente
Messaggio* getMessageCliente(Cliente* cln){ //Public
    if(cln->listaMessaggi == NULL){
        return NULL;
    }
    Messaggio *toRet = cln->listaMessaggi;
    cln->listaMessaggi = toRet->next;
    return toRet;
}

//Pulisce la memoria dinamica dei messaggi
void cleanMessaggi(){ //public
    Messaggio *p;
    Messaggio *curr;

    //Pulisco MessaggiAlBoss
    if(messaggiBoss !=NULL){
        p = messaggiBoss;
        while(p!=NULL){
            curr = p;
            p = p->next;
            free(curr);
        }
        messaggiBoss = NULL;
    }

    //Pulisco MessaggiClienti
    if(messaggiClienti !=NULL){
        p = messaggiClienti;
        while(p!=NULL){
            curr = p;
            p = p->next;
            free(curr);
        }
        messaggiClienti = NULL;
    }
    
}