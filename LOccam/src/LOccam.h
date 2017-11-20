
//
//	LOccam.h   Version 1.0
//

#include <FreeRTOS_AVR.h>       // processeurs AVR
// #include <FreeRTOS_ARM.h>    // processeurs ARM

//********************************************************
//********************************************************

#define boolean unsigned char
#define u_long unsigned long
#define TRUE	1
#define FALSE 	0

#define EXIT()          vTaskDelete(NULL)
#define PROC            void
#define MAX_ALT_CHAN    11      // 11 = 10 + 1 (SKIP , TIMER) 
#define TMP_SIZE        11

#define sem_t           SemaphoreHandle_t
#define semCreateBinary xSemaphoreCreateBinary
#define semGive(s)      xSemaphoreGive(s)
#define semTake(s) 	    xSemaphoreTake(s,portMAX_DELAY )


#define writeCHAN(x,v)  x->write(x,v)
#define readCHAN(x)     x->read(x)

//..............................................

#define setTRUE(x,v)    x->setTrue(x,v)
#define setFALSE(x,v)   x->setFalse(x,v)

#define writeALT(x,y)   x->altwrite(x,y)
#define readALT(x)      x->altread(x)

//..............................................

#define WAIT(t)       	vTaskDelay(t/portTICK_PERIOD_MS);
#define PAR(p,v)      	xTaskCreate(p, NULL,240, v, 2, NULL);

//................................................

#define TIMER_NOW(x)    x->tnow(x)
#define TIMER_RUN(x)    x->trun(x)
#define TIMER_PLUS(x,d) x->tplus(x,d)

#define cTIMER          10
#define cSKIP           10

//..............................................

#define writePALT(x,y)   x->paltwrite(x,y)
#define readPALT(x)      x->paltread(x)


#define psetTRUE(x,v)    x->psetTrue(x,v)
#define psetFALSE(x,v)   x->psetFalse(x,v)


///////////////////////////////////////////////////////////
//                 Les structures
////////////////////////////////////////////////////////////

typedef struct stm{
    int canal           ;
    int data            ;
}   MSG                 ;




typedef struct altrn{
    unsigned nbrProc                    	;   // nbr process input
    MSG     buffer[MAX_ALT_CHAN]  	    	;   // buffer circulaire
    MSG     notReady[MAX_ALT_CHAN]          ;
    boolean	guard[MAX_ALT_CHAN]	            ;
    unsigned  head, queue                   ;  // associes au buffer
        
    void (* altwrite)(struct altrn *,MSG)   ;
    MSG  (* altread)(struct altrn *)        ;
    
    void (* setTrue)(struct altrn *, unsigned)	          ;   
    void (* setFalse)(struct altrn *, unsigned )          ;   
    
    sem_t       binCritic              	    ;   //  ecrit en region critique
    sem_t       emptySem            	    ;   // gele lecteur si vide
    sem_t       fullSem                     ;   // gele ecrivain si plein
    sem_t   	semAck [MAX_ALT_CHAN]       ;   // acquittement ecrivain
        
}   ALT ;


typedef struct cn{
    sem_t  sembinw, sembinr             ;
    int     valeur                      ;
    
    void (* write) (struct cn *, int)   ;
    int  (* read)   (struct cn *)       ;
}   CHAN;


typedef struct stim {
  u_long  ticksCount, ticksPlus     ;
  byte  PlusOn                      ;
  unsigned tiks                     ; // nbre milli ,secondes
  sem_t tsem                        ;

  u_long  (* tnow)(struct stim *) ;
  void (* trun) (struct stim *)     ;
  void (* tplus)(struct stim *, u_long)     ;
} TIMER  ;


typedef struct pralt{
    unsigned    nbrProc                     ;   // nbr process input
    int         buffer[MAX_ALT_CHAN]        ;
    boolean     pguard[MAX_ALT_CHAN]         ;
    boolean     pready[MAX_ALT_CHAN]         ;
        
    void (* paltwrite)(struct pralt *,MSG)   ;
    MSG  (* paltread)(struct pralt *)        ;
    
    void (* psetTrue)(struct pralt *, unsigned)           ;   
    void (* psetFalse)(struct pralt *, unsigned )          ;   
    
    sem_t       binCritic                   ;
    sem_t       emptySem                    ;   // gele lecteur si vide
    sem_t       fullSem                     ;   // gele ecrivain si plein
    
    sem_t       semAck [MAX_ALT_CHAN]       ; // acquittement ecrivain
        
}   PRI_ALT ;



typedef struct str{
  TIMER       *stime    ;
  PRI_ALT     *salt     ;
} ALTIME                ;



/////////////////////////////////////////////////////
//              ALT
/////////////////////////////////////////////////////


void _setTrue(ALT *alt, unsigned nc){
  static boolean ready;
  
    // critique
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)    ;
    
    if( alt->guard[nc] == FALSE){  
        alt->guard[nc] = TRUE		                ;
        ready = FALSE                                ;
        // s'il y a eu  ecriture dans le canal nc
        // tester  si  notReady[nc].canal vaut -1 
        if(alt->notReady[nc].canal == -1){
            alt->notReady[nc].canal = nc                ;
            alt->buffer[alt->head] = alt->notReady[nc]  ;
            alt->head++                                 ;
            if(alt->head == TMP_SIZE) alt->head = 0     ;
            xSemaphoreGive(alt->emptySem)               ;
            ready = TRUE                               ;
        }
    }
    xSemaphoreGive(alt->binCritic)                      ; 
    // critique
        
    //if(ready)
        //xSemaphoreTake(alt->semAck[nc],portMAX_DELAY )  ; // attente 
}

void _setFalse(ALT *alt, unsigned nc){
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)    ; // debut critique
      //Serial.print("SF garde non prete : ")            ;
      //Serial.println(nc)                            ;
    alt->guard[nc] = FALSE                      ;
    xSemaphoreGive(alt->binCritic)              ; 
}
  

void _altwrite(ALT *alt, MSG msg){  
    static int nc                               ;
    static MSG message                          ;
    
    xSemaphoreTake(alt->fullSem,portMAX_DELAY)  ;
    
    // critique 
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)  	; // debut critique
    
    message = msg                               ;
    nc = message .canal                         ;
    if(alt->guard[nc] == TRUE){
      alt->buffer[alt->head] = msg              ;
      alt->head++                               ;
      if(alt->head == TMP_SIZE) alt->head = 0   ;
      xSemaphoreGive(alt->emptySem)             ;
    }
    else{   // guarde non prete
      message.canal = -1                        ;
      alt->notReady[nc] = message               ;
    }
    xSemaphoreGive(alt->binCritic)     		    ; 
    // critique
    
    xSemaphoreTake(alt->semAck[nc],portMAX_DELAY )  ; 
}


MSG _altread(ALT *alt){
    static MSG recu                     ;   // retourne le canal pret choisi
    static int nc                       ;
  
    
    xSemaphoreTake(alt->emptySem,portMAX_DELAY)     ;
    
    // critique
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)    ;
    
    //Serial.println("read ...........;");
    recu = alt->buffer[alt->queue]                  ;
    nc = recu.canal                                 ;
    xSemaphoreGive(alt->semAck[nc])                 ;
    xSemaphoreGive(alt->fullSem)                    ;
    alt->queue++                                    ;
    if(alt->queue == TMP_SIZE) alt->queue = 0       ;
    
    xSemaphoreGive(alt->binCritic)                  ; 
    //  critique
    
    return recu             	                    ;
}


void NEWALT(ALT *alt, unsigned nProc){
  unsigned i                	                    ;
  
  alt->nbrProc = nProc                              ;
  for(i=0;i< nProc;i++) alt->guard[i] = TRUE	    ; 
  
  alt->head  = 0    ;
  alt->queue = 0    ;
    
  alt->altwrite = _altwrite   		                ;
  alt->altread  = _altread    		                ;
  alt->setTrue   = _setTrue                         ;
  alt->setFalse  = _setFalse                        ;
  
  alt->binCritic   = xSemaphoreCreateBinary()       ;
  xSemaphoreGive(alt->binCritic)                    ;
  
  alt->emptySem = xSemaphoreCreateCounting(MAX_ALT_CHAN,0)      ;
  alt->fullSem  = xSemaphoreCreateCounting(MAX_ALT_CHAN, nProc) ;
  //
  for(i=0;i<nProc;i++)
      alt->semAck[i]   = xSemaphoreCreateBinary()    ;
}

///////////////////////////////////////////////////////////////
//          PRI_ALT
//////////////////////////////////////////////////////////////


void _psetTrue(PRI_ALT *alt, unsigned chan){
  alt->pguard[chan] = TRUE                       ;
}

void _psetFalse(PRI_ALT *alt, unsigned chan){
  alt->pguard[chan] = FALSE                      ;
}
  

void _paltwrite(PRI_ALT *alt, MSG msg){
    int nc                                      ;
    
    xSemaphoreTake(alt->fullSem,portMAX_DELAY) ;
    
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)    ; // debut critique
  
    nc = msg.canal                              ;
    alt->buffer[nc] = msg.data                  ;
    alt->pready[nc] = TRUE                      ;
    
    if(alt->pguard[nc]== TRUE){
      xSemaphoreGive(alt->emptySem)             ;
    }
    xSemaphoreGive(alt->binCritic)              ; // fin critique
    xSemaphoreTake(alt->semAck[nc],portMAX_DELAY )  ; // attente 
}


MSG _paltread(PRI_ALT *alt){                 
    static MSG recu                  ;   // retourne le canal pret choisi
    int       nc                     ;   // le numero  >= 0 du canal pret choisi
    unsigned choisi                  ;
  
    //  Si le semaphore emptySem laisse passer alors au moins un canal est pret
    //  si les gardes ne sont pas gérées
    
    xSemaphoreTake(alt->emptySem,portMAX_DELAY)     ;
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)    ; 

    choisi = FALSE                  ;   // associé au choix d'un canal pret
    nc = 0                          ;
    do{
      if((alt->pguard[nc] == TRUE) && (alt->pready[nc] == TRUE))
        choisi = TRUE               ;
      else nc++                     ;
    } while((choisi == FALSE) && (nc < alt->nbrProc ))   ; 
    if(choisi == TRUE){
      alt->pready[nc] = FALSE            ;
      recu.canal = nc                   ;
      recu.data  = alt->buffer[nc]      ;
      xSemaphoreGive(alt->fullSem)      ;
      xSemaphoreGive(alt->semAck[nc])   ;   // 
    }
    else if(alt->pready[10] == TRUE){   // TIMER ou SKIP
      alt->pready[10] = FALSE           ;
      recu.canal = 10                   ;
      recu.data = 0                     ;
      xSemaphoreGive(alt->fullSem)      ;
      xSemaphoreGive(alt->semAck[10])   ;   //
    }
    xSemaphoreGive(alt->binCritic)      ;   // fin critique 
      
    //
    return recu                          ;
}


void NEWPRI_ALT(PRI_ALT *alt, unsigned nProc){
  unsigned i    , nbrProc               ;
  
  alt->nbrProc = nProc                                  ;
  for(i=0;i< nProc;i++) alt->pguard[i] = TRUE            ; 
  for(i=0;i< nProc;i++) alt->pready[i] = FALSE           ; 
  
  alt->pready[10] = FALSE           ; // SKIP ou Timer
  alt->pguard[10] = TRUE            ; // SKIP ou Timer 
  
  alt->paltwrite  = _paltwrite                        ;
  alt->paltread   = _paltread                         ;
  alt->psetTrue   = _psetTrue                         ;
  alt->psetFalse  = _psetFalse                        ;
  
  alt->binCritic   = xSemaphoreCreateBinary()           ;
  xSemaphoreGive(alt->binCritic)                        ;
  
  alt->emptySem = xSemaphoreCreateCounting(MAX_ALT_CHAN,0)  ;
  alt->fullSem = xSemaphoreCreateCounting(MAX_ALT_CHAN, nProc)      ;
  //
  for(i=0;i<nProc;i++)
      alt->semAck[i]   = xSemaphoreCreateBinary()       ;
  alt->semAck[10]   = xSemaphoreCreateBinary()          ;
}

/////////////////////////////////////////////////////////////
//                Les canaux
////////////////////////////////////////////////////////////



void _write(CHAN *can, int v){
    can->valeur = v                     ;
    xSemaphoreGive(can->sembinw)        ;
    xSemaphoreTake(can->sembinr,portMAX_DELAY ) ;
}

int  _read(CHAN *can)   {
    static int recoit                  ;
    
    xSemaphoreGive(can->sembinr)        ;
    xSemaphoreTake(can->sembinw,portMAX_DELAY ) ;
    recoit = can->valeur                ;
    return recoit                       ;
}

void NEWCHAN (CHAN *can){
    can->sembinr =  xSemaphoreCreateBinary();
    can->sembinw =  xSemaphoreCreateBinary();
    //
    can->write = _write ;
    can->read  = _read  ;
}

/////////////////////////////////////////////////
//              Le timer 
///////////////////////////////////////////////////


void _run(TIMER *tmr) {
  while (1) {
    vTaskDelay(tmr->tiks / portTICK_PERIOD_MS)   ;
    tmr->ticksCount++                     ;
    if (tmr->PlusOn == 1) {
      if (tmr->ticksPlus >  0) tmr->ticksPlus--;
      else{
        xSemaphoreGive(tmr->tsem)       ;
        tmr->PlusOn = 0L                ;
      }
    }
  }
} // _run


u_long  _now(TIMER *tmr) {
  return tmr->ticksCount    ;
}

void _plus(TIMER *tmr, u_long delai) {
  tmr->ticksPlus = delai    ;
  tmr->PlusOn = 1           ;
  xSemaphoreTake(tmr->tsem, portMAX_DELAY )  ;
}

void NEWTIMER(TIMER *tmr, unsigned tks) {
  tmr->tnow = _now        ;
  tmr->trun = _run        ;
  tmr->tplus = _plus      ;
  
  tmr->ticksCount = 0L    ;
  tmr->PlusOn = 0L        ;
  tmr->tiks = tks         ;

  tmr->tsem = xSemaphoreCreateBinary()       ;
}

////////////////////////////////////////////////
//         START() et la gestion du timer
////////////////////////////////////////////////

PROC ptime(void *pvParameters){
  static PRI_ALT *palt            ;
  static MSG msg                  ;

  palt = (PRI_ALT *) pvParameters ;
  msg.canal = cTIMER              ;  // canal dedié au timer
  msg.data = 0                    ;  // bidon
  while(1) writePALT(palt,msg) ;  
}


PROC pTIMER(void *pvParameters) {
  static TIMER *tmr              ;

  tmr = (TIMER *)pvParameters    ;
  TIMER_RUN(tmr)                 ;
}


void START(void *pt){
  static ALTIME *paltim     ;
  static TIMER * ptim       ;
  static PRI_ALT * palt     ;
  
  if(pt != NULL){
    paltim = (ALTIME *) pt  ;
    ptim = paltim->stime    ;
    palt = paltim->salt     ;
      xTaskCreate(pTIMER, NULL,240, ptim, 2, NULL);
      xTaskCreate(ptime , NULL,240, palt, 2, NULL);
  }
  vTaskStartScheduler()  ;
  
}





