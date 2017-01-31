
// cSKIP channel


#include <LOccam.h>


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


PROC process(void *pvParameters){
  static PRI_ALT *palt        ;
  static MSG msg              ;

  static u_long now1 , now2, delta    ;
  static byte carlu , encore          ;

  palt =(PRI_ALT *)pvParameters       ;
  
  setPTRUE(palt,cSKIP)               ; // enable SKIP channel
  encore = TRUE                       ;
  do{
      WAIT(2000)                      ;
      msg = readPALT(palt)            ;
      switch(msg.canal){
        case 0:{
            carlu = (byte) msg.data   ;
            Serial.print("il y a du courrier ");
            Serial.println(carlu)     ;
            break                     ;
        }
        case cSKIP :{                // SKIP canal 11
            Serial.println("pas de courrier aujourdh'ui");
            Serial.println("Y AM WORKING");
            break                     ;
        }
        default: break;
      }
  }   while(encore)                   ;
  EXIT()                              ;
  
}


void setup() {
  Serial.begin(9600)        ;

  NEWPRI_ALT(&paltrn,1)     ; 

  PAR(process, &paltrn)    ;
  PAR(keyboard, &paltrn)    ;
  START()                   ;
}

void loop() {

}
