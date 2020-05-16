#define _GNU_SOURCE
#include <stdio.h>  //printf scanf
#include <time.h>   //time
#include <signal.h> //signals
#include <stdlib.h> //malloc
#include <unistd.h> //sleep
#include <pthread.h> //pthreads
#include <stdbool.h> //booleans
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>


typedef struct{
    int K;int C;int E;
    int T;int P;int S;
    int TPP; int UBT;
    int S1; int S2;
    int LO; int CAI;
}variables;

typedef struct cliente Cliente;
typedef struct tidCliente{
    int id;
    pthread_t tidCliente;
    bool completed;
    Cliente *cliente;
    struct tidCliente *next;
}TIDCLIENTE;

typedef struct cassa Cassa;
typedef struct tidCassa{
    int id;
    pthread_t tidCassa;
    bool completed;
    Cassa *cassa;
    struct tidCassa *next;
}TIDCASSA;

typedef struct messaggio Messaggio;
typedef struct cliente{
    int id;
    
    struct timespec *tempoEntrata;
    int tempoRicerca;
    int numeroProdotti;
    struct timespec *tempoEntrataCoda;
    struct timespec *tempoUscitaCoda;
    TIDCLIENTE *TID;
    pthread_cond_t *RESPONSE_COND;
    pthread_mutex_t CLIENTE_MUTEX;
    Messaggio *listaMessaggi;
}Cliente;

typedef struct cassa{
    int id;
    int tempoLavoro;
    bool chiudiAllaFine;
    bool chiudiSubito;
    int numeroProdottiElaborati;
    int numeroDiClienti;
    time_t tempoServiziSecondi;
    long tempoServiziMillisecondi;
    struct timespec *tempoEntrata;
    TIDCASSA *TID;
    pthread_cond_t *RESPONSE_COND;
}Cassa;

typedef struct elementoCoda{
    Cliente* cliente;
    int msg;
    struct elementoCoda* next;
}ElementoCoda;

typedef struct coda{
    pthread_mutex_t CODA_MUTEX;
    bool aperta;
    ElementoCoda *first; 
}Coda;

typedef struct messaggio{
    Cliente *cliente;
    Cassa *cassa;
    int codice;
    int info;
    struct messaggio *next;
}Messaggio;

extern Messaggio *messaggiBoss;
extern Messaggio *messaggiClienti;
extern pthread_mutex_t MUTEX_MESSAGGIBOSS;
extern pthread_cond_t COND_MESSAGGIBOSS;
extern pthread_cond_t COND_BOSS;
extern pthread_mutex_t MUTEX_MESSAGGICLIENTI; //Mutex add/get messaggi


extern variables *vars;
extern bool evacuazione;
extern pthread_mutex_t MUTEX_EVACUAZIONE;
extern bool chiusura;
extern pthread_mutex_t MUTEX_CHIUSURA;

extern int numeroClienti;
extern pthread_mutex_t MUTEX_ID_CLIENTI;

extern int dilazioneTempo;
extern Cliente **clienti;
extern pthread_mutex_t MUTEX_CLIENTI;

extern pthread_t gestoreClientiTID; //Supermercato.c
extern pthread_t gestoreCasseTID;   //Supermercato.c


//Casse
extern pthread_mutex_t MUTEX_CASSE;

//TEST
extern pthread_mutex_t MUTEX_CODE;
extern pthread_mutex_t STATO_CODE;
extern Coda **code;
extern int codeAperte;
extern pthread_mutex_t NUMERO_CODE_APERTE;

//boss
void* bossProcess();
bool getChiusura();
bool getEvaquazione();
void* attesaChiusuraThread();
void pulisciStatoInternoBoss();


//Cliente
void cleanClienti();
void* clienteProcess();
ElementoCoda* getCliente(int i);
void *gestoreClienti();
void differceTime(char* ret,struct timespec t1, struct timespec t2);

//Code
void inizializzaCode();
void cleanCode();
int apriCoda();
int insertRandomCoda(Cliente *cln);
void chiudiCoda(int i);

//Casse
void *gestoreCasse();
void nuovaCassa(int i);


//Messaggi
void cleanMessaggi();
void messaggeToBoss(Cliente *cln,Cassa *cassa, int codice,int info);
Messaggio* getMessageBoss();
void messaggeToCliente(Cliente *cln, int codice,int info);
Messaggio* getMessageCliente(Cliente* cln);

//Log
void LOGIT2(char *msg); //Clienti
void LOGIT3(char *msg); //Casse

//Signal
void handle_SIGQUIT(int sig);
void handle_SIGHUP(int sig);