#include <supermercato.h>

pthread_mutex_t STATO_CODE = PTHREAD_MUTEX_INITIALIZER;
Coda **code = NULL;

int codeAperte = 0;
pthread_mutex_t NUMERO_CODE_APERTE = PTHREAD_MUTEX_INITIALIZER;

//Inizializzo le code
void inizializzaCode(){ //public
    int i;
    code = malloc(sizeof(Coda*)*vars->K);
    for(i=0;i<vars->K;i++){
        code[i] = malloc(sizeof(Coda));
        pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
        code[i]->aperta = false;
        code[i]->CODA_MUTEX = MUTEX;
        code[i]->first = NULL;
    }
}

//Inserisce un cliente in una coda aperta random
int insertRandomCoda(Cliente *cln){ //Public
    pthread_mutex_lock(&MUTEX_CHIUSURA);
    if(evacuazione || chiusura){
        pthread_mutex_unlock(&MUTEX_CHIUSURA);
        return -1;
    }else{pthread_mutex_unlock(&MUTEX_CHIUSURA);}
    
    ElementoCoda *ele = malloc(sizeof(ElementoCoda));
    ele->cliente = cln;
    ele->next = NULL;
    ele->msg = 0;

    bool isOpen[vars->K];
    int i;
    //Salvo lo stato delle code
    pthread_mutex_lock(&STATO_CODE);
    for(i=0;i<vars->K;i++){
        if(code[i]->aperta == true){
            isOpen[i] = true;
        }else{
            isOpen[i] = false;
        }
    }
    pthread_mutex_unlock(&STATO_CODE);

    //Trovo la coda in maniera random in cui andare
    int aperte = 0;
    for(i=0;i<vars->K;i++){
        if(isOpen[i] == true){
            aperte++;
        }
    }
    srand(pthread_self()+time(NULL));
    int choosen = 1+rand()%aperte;
    i=0;
    while(choosen > 0){
        if(isOpen[i]==true){
            choosen--;
        }
        i++;
    }
    int codaScelta = i-1;

    //Blocco l'accesso alla coda scelta e aggiungo il nuovo elemento
    pthread_mutex_lock(&code[codaScelta]->CODA_MUTEX);
        if(code[codaScelta]->aperta==false){
            pthread_mutex_unlock(&code[codaScelta]->CODA_MUTEX);
            return -1;
        }
        ElementoCoda *p = code[codaScelta]->first;
        if(p==NULL){
            code[codaScelta]->first = ele;
        }else{
            while(p->next!=NULL){
                p=p->next;
            }
            p->next = ele;
        }
    pthread_mutex_unlock(&code[codaScelta]->CODA_MUTEX);

    return 0;
}

//Pulisco la memoria dinamica per le code
void cleanCode(){ //public
    int i;
    for(i=0;i<vars->K;i++){
        ElementoCoda *p = code[i]->first;
        while(p!=NULL){
            ElementoCoda *curr = p;
            p = p->next;
            free(curr);
        }
        free(code[i]);
        free(p);
    }
    free(code);
}

//Ritorno l'elementoCoda da una coda
ElementoCoda* getCliente(int i){ //Public
    ElementoCoda *toRet;
    pthread_mutex_lock(&MUTEX_CODE);
    toRet = code[i]->first;
    if(toRet!=NULL){
        code[i]->first = toRet->next;    
    }
    pthread_mutex_unlock(&MUTEX_CODE);
    return toRet;
}

//Chiudo una coda
void chiudiCoda(int i){ //Public
    pthread_mutex_lock(&NUMERO_CODE_APERTE);
        if(codeAperte<=1){
            pthread_mutex_unlock(&NUMERO_CODE_APERTE);
            return;
        }
    pthread_mutex_unlock(&NUMERO_CODE_APERTE);
    
    pthread_mutex_lock(&code[i]->CODA_MUTEX);
        code[i]->aperta = false;
    pthread_mutex_unlock(&code[i]->CODA_MUTEX);

    pthread_mutex_lock(&NUMERO_CODE_APERTE);
        codeAperte--;
    pthread_mutex_unlock(&NUMERO_CODE_APERTE);


    //Aggiungo messaggio di chiusura, quando la cassa vedrà questo messaggio chiuderà
    ElementoCoda *ele = malloc(sizeof(ElementoCoda));
    ele->cliente = NULL;
    ele->next = NULL;
    ele->msg = 1;
    pthread_mutex_lock(&code[i]->CODA_MUTEX);
        ElementoCoda *p = code[i]->first;
        code[i]->first = ele;
        ele->next = p;
        printf("\033[0;31m- CODA.%d)   - AGGIUNTO SEGNALE DI CHIUSURA\033[0;90m\n",i);
    pthread_mutex_unlock(&code[i]->CODA_MUTEX);
    

}

//Apro una coda a caso
int apriCoda(){ //public
    pthread_mutex_lock(&NUMERO_CODE_APERTE);
    if(codeAperte>=vars->K){
        pthread_mutex_unlock(&NUMERO_CODE_APERTE);
        return -1;
    }else{
        pthread_mutex_unlock(&NUMERO_CODE_APERTE);
    }

    //apre una coda a caso
    bool isOpen[vars->K];
    int i;

    //Salvo lo stato attuale
    pthread_mutex_lock(&STATO_CODE);
    for(i=0;i<vars->K;i++){
        if(code[i]->aperta == true){
            isOpen[i] = true;
        }else{
            isOpen[i] = false;
        }
    }
    pthread_mutex_unlock(&STATO_CODE);

    int aperte = 0;
    for(i=0;i<vars->K;i++){
        if(isOpen[i] == true){
            aperte++;
        }
    }
    int chiuse = vars->K - aperte;
    if(chiuse==0){
        printf("Tutte le code sono aperte\n");
        return -1;
    }
    srand(pthread_self()+time(NULL));
    int choosen = 1+rand()%chiuse;
    i=0;
    while(choosen > 0){
        if(isOpen[i] == false){
            choosen--;
        }
        i++;
    }
    int daAprire = i-1;

    //Aggiorno la coda
    pthread_mutex_lock(&STATO_CODE);
    code[daAprire]->aperta = true;
    pthread_mutex_unlock(&STATO_CODE);

    pthread_mutex_lock(&NUMERO_CODE_APERTE);
        codeAperte++;
    pthread_mutex_unlock(&NUMERO_CODE_APERTE);

    //Apro la Cassa
    nuovaCassa(daAprire);
    return daAprire;
}
