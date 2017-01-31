

#include <LOccam.h>

#define NBRM 10                 //  nbre of messages emited by each  process

ALT altrn                      ;



PROC  kbd(void *pvParameters){ // ecrit canal 2
    static MSG msg              ;
    static char cardispo        ; 
    static ALT *alt             ;

    alt = (ALT *)pvParameters   ;
    msg.canal = 2               ;
    msg.data  = -1              ; // no sens
    WAIT(5)                  ;
    
    writeALT(alt, msg)          ;
    EXIT()                      ;
}



PROC Control (void *pvParameters ) {
    static MSG msg                ;
    static int count, NBRMC       ;
    ALT *alt                      ;

    alt = (ALT *)pvParameters     ;
    NBRMC = 2* NBRM               ; 
    count = 0                     ;
    setFALSE(alt,0)               ; 
    while(count <  NBRMC){
        //WAIT(500)                 ; // pour voir kbd 
        msg = readALT(alt)        ;
        switch(msg.canal){
          case 0:{
            Serial.print("canal 0 :");
            Serial.println(msg.data)    ;
              count++                   ;
              break ;
          }
          case 1:{
              Serial.print("canal 1 :");
              Serial.println(msg.data)    ;
              count++                   ;
              break ;
          }
        
          case 2:{ // keyboard
              setTRUE(alt,0)      ;
              break               ;
          }
          default: break          ;
        }
    }
    WAIT(100)                     ; // for klean printing at the end
    Serial.println("\nControl BY ....")     ;
    EXIT()                                  ;
} // Control



PROC Sender0 (void *pvParameters ){
    static MSG data0              ;
    static int i                  ;
    static ALT *alt               ;

    alt = (ALT *)pvParameters     ;
    i = 0 ;
    data0.data = 10                ;
    data0.canal = 0               ;
    while(i< NBRM){
      writeALT(alt, data0)        ;
      i++                         ;
    }
    WAIT(100);
    Serial.println("\nS0 BY ...") ;
    EXIT()                        ;
}


PROC Sender1 (void *pvParameters ){
    static MSG data1              ;
    static int i                  ;
    static ALT *alt               ;
    
    alt = (ALT *)pvParameters     ;
    i = 0                         ;
    data1.data = 20                ;
    data1.canal = 1               ;
    while(i< NBRM){
      writeALT(alt, data1)        ;
      i++                         ;
    }
    WAIT(100);
    Serial.println("\nS1 BY ...") ;
    EXIT()                        ;
}






void setup() {
  
    Serial.begin(9600)    ;
    Serial.flush()        ;

    ////////////////////////////////////
    
    NEWALT(&altrn,3)       ;
  
    PAR(Sender0 ,&altrn)   ;
    PAR(Sender1 ,&altrn)   ;
    PAR(kbd,&altrn)        ;
    
    PAR(Control,&altrn)    ;
    START()                ;
}

void loop() {

}
