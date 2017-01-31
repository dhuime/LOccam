
//  watchdog


#include <LOccam.h>

#define MAXTIME 0xFFFFFFFF


TIMER  timer          ;
PRI_ALT     paltrn     ;


PROC  keyboard(void *pvParameters){ // ecrit canal 0
    static MSG msg              ;
    static int dispo            ; 
    static PRI_ALT *alt         ;
    unsigned lu                 ;

    alt = (PRI_ALT *)pvParameters   ;
    msg.canal = 0               ;
    while(1){
      dispo = Serial.available() ;
      if(dispo != 0){
        lu = Serial.read()       ;
        msg.data = (int)lu       ;
        writePALT(alt, msg)      ;
      }
    }
}


PROC watchdog(void *pvParameters){
  static PRI_ALT *palt        ;
  static MSG msg              ;

  static u_long now1 , now2, delta    ;
  static byte carlu , encore          ;

  palt =(PRI_ALT *)pvParameters       ;
  
  setPTRUE(palt,cTIMER)               ; // enable TIMER channel
  now1 = readTIMER(&timer)            ;
  encore = TRUE                       ;
  do{
      WAIT(2000)                      ;
      msg = readPALT(palt)            ;
      switch(msg.canal){
        case 0:{
            carlu = (byte) msg.data   ;
            Serial.println(carlu)     ; 
            now1 = readTIMER(&timer)  ;
            break                     ;
        }
        case cTIMER :{                    // TIMER canal 10
            now2 =  readTIMER(&timer)  ;
            delta = SUB(now2 ,now1)  ;
            if(delta > 200L) encore = FALSE  ; // attente max de 200 ticks
            break                     ;
        }
        default: break;
      }
      Serial.println("Y AM WORKING");
  }   while(encore)                   ;
  Serial.println("TIME OUT ....")     ;
  EXIT()                              ;
  
}


void setup() {
  Serial.begin(9600)        ;

  NEWPRI_ALT(&paltrn,2)     ; // timer + keyboard
  NEWTIMER(&timer, 10)      ; // ticks every 10 milli seconds
  
  PAR(runTIMER, &timer)     ; // lance le timer
  PAR(watchdog, &paltrn)    ;
  PAR(keyboard, &paltrn)    ;
  START()                   ;
}

void loop() {

}
