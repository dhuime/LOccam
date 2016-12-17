
//
//	Version du  25/11/2016


#include <FreeRTOS_AVR.h>       // processeurs AVR
// #include <FreeRTOS_ARM.h>    // processeurs ARM

#define boolean unsigned char
#define TRUE	1
#define FALSE 	0

#define EXIT()          vTaskDelete(NULL)
#define START()         vTaskStartScheduler()
#define PROC            void
#define MAX_ALT_CHAN    10

#define sem_t           SemaphoreHandle_t
#define semCreateBinary xSemaphoreCreateBinary
#define semGive(s)	    xSemaphoreGive(s)
#define semTake(s)	    xSemaphoreTake(s,portMAX_DELAY )


#define writeCHAN(x,v)  x->write(x,v)
#define readCHAN(x)     x->read(x)

#define setTRUE(x,v)    x->setTrue(x,v)
#define setFALSE(x,v)   x->setFalse(x,v)

#define writeALT(x,y)   x->altwrite(x,y)
#define readALT(x)      x->altread(x)

#define WAIT(t)       	vTaskDelay(t/portTICK_PERIOD_MS);
#define PAR(p,v)      	xTaskCreate(p, NULL,240, v, 2, NULL);



///////////////////////////////////////////////////////////

typedef struct stm{
    int canal           ;
    int data            ;
}   MSG                 ;


typedef struct altrn{
    unsigned nbrProc                    ;   // nbr process input
    int     buffer[MAX_ALT_CHAN]  	    ;
    boolean	guard[MAX_ALT_CHAN]	        ;
    boolean	ready[MAX_ALT_CHAN]	        ;
        
    void (* altwrite)(struct altrn *,MSG)   ;
    MSG  (* altread)(struct altrn *)         ;
    
    void (* setTrue)(struct altrn *, unsigned)	          ;   
    void (* setFalse)(struct altrn *, unsigned )          ;   
    
    
    sem_t       binCritic              	    ;
    sem_t       emptySem            	    ;   // gele lecteur si vide
    sem_t       fullSem                     ;   // gele ecrivain si plein
    
    sem_t   	semAck [MAX_ALT_CHAN]       ; // acquittement ecrivain
        
}   ALT ;


typedef struct cn{
    sem_t  sembinw, sembinr             ;
    int     valeur                      ;
    
    void (* write) (struct cn *, int)   ;
    int  (* read)   (struct cn *)       ;
}   CHAN;


void _setTrue(ALT *alt, unsigned chan){
  alt->guard[chan] = TRUE		                ;
}

void _setFalse(ALT *alt, unsigned chan){
  alt->guard[chan] = FALSE                      ;
}
  

void _altwrite(ALT *alt, MSG msg){
    int nc                                      ;
    
    xSemaphoreTake(alt->fullSem,portMAX_DELAY) ;
    
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)  	; // debut critique
  
    nc = msg.canal                              ;
    alt->buffer[nc] = msg.data                  ;
    alt->ready[nc] = TRUE                       ;
    
    if(alt->guard[nc]== TRUE){
      xSemaphoreGive(alt->emptySem)             ;
    }
    xSemaphoreGive(alt->binCritic)     		    ; // fin critique
    xSemaphoreTake(alt->semAck[nc],portMAX_DELAY )  ; // attente 
}


MSG _altread(ALT *alt){
    static MSG recu                  ;   // retourne le canal pret choisi
    int       nc                     ;   // le numero  >= 0 du canal pret choisi
    unsigned choisi                  ;
  
    //  Si le semaphore emptySem laisse passer alors au moins un canal est pret
    //  si les gardes ne sont pas gérées
    
    xSemaphoreTake(alt->emptySem,portMAX_DELAY) ;
    //
    xSemaphoreTake(alt->binCritic,portMAX_DELAY)   ; 
    choisi = FALSE                  ;   // associé au choix d'un canal pret
    nc = 0                          ;
    do{
      if((alt->guard[nc] == TRUE) && (alt->ready[nc] == TRUE))
        choisi = TRUE               ;
      else nc++                     ;
    } while((choisi == FALSE) && (nc < alt->nbrProc ))   ; 
    if(choisi == TRUE){
      alt->ready[nc] = FALSE            ;
      recu.canal = nc                   ;
      recu.data  = alt->buffer[nc]      ;
      xSemaphoreGive(alt->fullSem)      ;
      xSemaphoreGive(alt->semAck[nc])   ;   // 
    }
    else{
      recu.canal = -1                   ;
      recu.data  = 0                    ; // bidon
    }
    xSemaphoreGive(alt->binCritic)      ;   // fin critique 
      
    //
    return recu             	         ;
}


void NEWALT(ALT *alt, unsigned nProc){
  unsigned i	, nbrProc			;
  
  alt->nbrProc = nProc                              ;
  for(i=0;i< nProc;i++) alt->guard[i] = TRUE	    ; 
  for(i=0;i< nProc;i++) alt->ready[i] = FALSE       ; 
  
  alt->altwrite = _altwrite   		                ;
  alt->altread  = _altread    		                ;
  alt->setTrue   = _setTrue                         ;
  alt->setFalse  = _setFalse                        ;
  
  alt->binCritic   = xSemaphoreCreateBinary()       ;
  xSemaphoreGive(alt->binCritic)                    ;
  
  alt->emptySem = xSemaphoreCreateCounting(MAX_ALT_CHAN,0)  ;
  alt->fullSem = xSemaphoreCreateCounting(MAX_ALT_CHAN, nProc)      ;
  //
  for(i=0;i<nProc;i++)
      alt->semAck[i]   = xSemaphoreCreateBinary()    ;
  
   
  //alt->sguard   = xSemaphoreCreateBinary()       ;
  //xSemaphoreGive(alt->sguard)                    ;
  
}

///////////////////////////////////////////////////////////////

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

