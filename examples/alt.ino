
// alt.ino

#include <LOccam.h>

ALT altrn          ;

PROC Control (void *pvParameters ) {
  static MSG msg                ;
  static unsigned compteur      ;
  static ALT *palt              ;

  palt = (ALT *)pvParameters    ; // palt = pointeur sur  altrn 
  compteur = 0                  ;
  do{
        msg = readALT(palt)    ; // suspendu a la lecture d'un canal pret
        switch(msg.canal){
              case 0:{
                  Serial.println(msg.data);
                  compteur++    ;
                  break         ;
              }
              case 1:{
                  Serial.println(msg.data);
                  compteur++    ;
                  break         ;
              }
              default:  break   ;
        }
  } while(compteur < 2)         ;
  Serial.println("Control BY ...")  ;
  EXIT()                        ;
}


PROC Sender0 (void *pvParameters ){
    static MSG msg              ;
    static ALT *palt            ;

    palt = (ALT *)pvParameters  ; // palt = pointeur sur altrn
    msg.data = 345              ;
    msg.canal = 0               ;

    //WAIT(2);
    writeALT(palt, msg)        ;
    Serial.print("Sender0 BY...\n");
    EXIT()                      ;
}


PROC Sender1 (void *pvParameters ){
    static MSG msg              ;
    static ALT *palt            ;

    palt = (ALT *)pvParameters  ; // palt = pointeur sur altrn
    msg.data = 123              ;
    msg.canal = 1               ;

    //WAIT(2) ;
    writeALT(palt, msg)        ;
    Serial.print("Sender1 BY ...\n");
    EXIT()                      ;
}


void setup( ){
  
    Serial.begin(9600)  ;
    Serial.flush()      ;

    NEWALT(&altrn,2)    ;

    PAR(Sender0,&altrn) ; // parametre = pointeur sur altrn
    PAR(Sender1,&altrn) ; // idem
    PAR(Control,&altrn) ; // idem
    
    START()         ;
}

void loop(){}

