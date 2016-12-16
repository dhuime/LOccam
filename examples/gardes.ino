
//  gardes.ino

#include <LOccam.h>

#define NBRM 20                 //  nbre of messages emited by each  process

CHAN can0,can1                 ;
ALT altrn                      ;

PROC litCanal0 (void *pvParameters){
    static int lu ,i            ;
    static CHAN *canal          ;

    canal = (CHAN *)pvParameters    ;
    i = 0                       ;
    while(i < NBRM){
      lu = readCHAN(canal)      ;
      Serial.print(lu)          ;
    }
    EXIT()                      ;
}

PROC litCanal1 (void *pvParameters){
    static int lu,i             ;
    static CHAN *canal          ;

    canal = (CHAN *)pvParameters    ;
    i = 0                       ;
    while(i < NBRM){
      lu = readCHAN(canal)      ;
      Serial.print(lu)          ;
    }
    EXIT()                      ;
}


PROC  keyboard(void *pvParameters){
    static MSG msg              ;
    static int cardispo         ; 
    static ALT *alt             ;

    alt = (ALT *)pvParameters   ;
    msg.canal = 2               ;
    msg.data = -1               ;
    while(1){
      cardispo = Serial.available()   ;
      if(cardispo != 0){
        writeALT(alt, msg)      ;
      }
    }
    EXIT()                      ;
}


PROC Control (void *pvParameters ) {
    static MSG msg                ;
    static int count, NBRMC       ;
    ALT *alt                      ;

    alt = (ALT *)pvParameters     ;
    NBRMC = 2* NBRM               ; // Control reads NBRMC times his inputs
    count = 0                     ;
    setFALSE(alt,0)               ; 
    while(count <  NBRMC){
        msg = readALT(alt)        ;
        count++                   ;
        switch(msg.canal){
          case 0:{
              writeCHAN(((CHAN *)&can0),msg.data);
              break ;
          }
          case 1:{
              writeCHAN(((CHAN *)&can1),msg.data);
              break ;
          }
        
          case 2:{
              setTRUE(alt,0)      ;
              break ;
          }
          default: break          ;
        }
    }
    WAIT(100)                     ; // for klean printing at the end
    Serial.println("\n\n   Control BY ....")     ;
    EXIT()                                       ;
} // Control



PROC Sender0 (void *pvParameters ){
    static MSG data0              ;
    static int i                  ;
    static ALT *alt               ;

    alt = (ALT *)pvParameters     ;
    i = 0 ;
    data0.data = 0                ;
    data0.canal = 0               ;
    while(i< NBRM){
      WAIT(100)                   ; // for interleaving
      writeALT(alt, data0)        ;
      i++                         ;
    }
    EXIT()                        ;
}


PROC Sender1 (void *pvParameters ){
    static MSG data1              ;
    static int i                  ;
    static ALT *alt               ;
    
    alt = (ALT *)pvParameters     ;
    i = 0                         ;
    data1.data = 1                ;
    data1.canal = 1               ;
    while(i< NBRM){
      WAIT(200)                   ; // for interleaving
      writeALT(alt, data1)        ;
      i++                         ;
    }
    EXIT()                        ;
}


void setup( ){
  
    Serial.begin(9600)  ;
    Serial.flush()      ;

    ////////////////////////////////////
    
    NEWALT(&altrn,3)    ;
    
    NEWCHAN(&can0)      ;
    NEWCHAN(&can1)      ;

    /////////////////////////////////////
  
    PAR(Sender0,&altrn)    ;
    PAR(Sender1,&altrn)    ;

    PAR(keyboard,&altrn)   ;
    
    PAR(Control,&altrn)   ;

    PAR(litCanal0,&can0)  ;
    PAR(litCanal1,&can1)  ;
    
    START()               ;
}

void loop(){}
