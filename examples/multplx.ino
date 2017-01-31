
//  multplx.ino
//  implement a multiplexer

#include <LOccam.h>

ALT altrn               ;
CHAN canal              ;

PROC receive(void *pvParameters ) {
  CHAN *canal                   ;
  int i , data                  ;
  
  canal = (CHAN *) pvParameters ;
  i=0                           ;
  do{
    data = readCHAN(canal)      ;
    i++                         ;
    Serial.print(data)          ;
    Serial.print(" ")           ;
  } while(i < 20)               ;
  EXIT()                        ;
}   
  
  

PROC Control (void *pvParameters ) { // alternation process
  static MSG msg                ;
  static unsigned compteur      ;
  static ALT *palt              ;

  palt = (ALT *)pvParameters    ; 
  compteur = 0                  ;
  do{
        msg = readALT(palt)     ; // suspended if no channel ready
        switch(msg.canal){
              case 0:{            // Executed if channel 0 is ready
                  writeCHAN(((CHAN *)&canal),msg.data); 
                  compteur++    ;
                  break         ;
              }
              case 1:{            // Executed if channel 1 is ready
                  writeCHAN(((CHAN*)&canal),msg.data) ;
                  compteur++    ;
                  break         ;
              }
              default:  break   ;
        }
  } while(compteur < 20)         ;
  Serial.println("\nControl BY ...")  ;
  EXIT()                        ;
}


PROC Sender0 (void *pvParameters ){
    static MSG msg              ;
    static ALT *palt            ;
    static int count            ;

    palt = (ALT *)pvParameters  ; // palt = pointer on altrn
    msg.data = 0                ;
    msg.canal = 0               ;
    count = 0                  ;
    
    while(count < 10){
      writeALT(palt, msg)         ;
      count++                     ;
    }
    Serial.print("\nSender0 BY ...");
    EXIT()                      ;
}


PROC Sender1 (void *pvParameters ){
    static MSG msg              ;
    static ALT *palt            ;
    static int count            ;

    palt = (ALT *)pvParameters  ; // palt = pointer on altrn
    count = 0                   ;
    msg.data = 1                ;
    msg.canal = 1               ;
    
    while(count < 10){
    writeALT(palt, msg)         ;
    count++                     ;
    }
    Serial.print("\nSender1 BY ...");
    EXIT()                      ;
}


void setup( ){
  
    Serial.begin(9600)  ;
    Serial.flush()      ;

    NEWALT(&altrn,2)    ;
    NEWCHAN(&canal)     ;

    PAR(receive, &canal); // receive data from sender0 and sender1
    PAR(Sender0,&altrn) ; 
    PAR(Sender1,&altrn) ; 
    
    PAR(Control,&altrn) ; // implement the multiplexer
    START()             ;
}

void loop(){}
