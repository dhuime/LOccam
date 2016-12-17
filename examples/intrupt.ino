
//  bouton3.ino
//  gestion des IT



// Le bouton est connecté à la broche 2 (code 0 de l'IT)
// ATTENTION : Mettre la tension sous 3Volts avec l'ATMEGA2560

#include <LOccam.h>

sem_t semBin    ;
ALT  altrn    	;

void handler(){
    semGive(semBin)        	    ;
}


PROC proc0(void *pvParameters){
  static ALT *palt              ;
  static MSG msg                ;

  palt = (ALT *)pvParameters    ;
  msg.canal = 0                 ;
  msg.data = 100                ;
  
  while(1){
      semTake(semBin)           ;
      writeALT(palt, msg)       ;
      msg.data++                ;
  }
}

PROC proc1(void *pvParameters){
  static ALT *palt             ;
  static MSG  msg              ;

  palt = (ALT *)pvParameters  ;
  msg.canal = 1               ;
  msg.data  = 0               ;
  
  while(TRUE){
    WAIT(1000)                ;
    writeALT(palt, msg)       ;
    msg.data++                ;
  }
}

PROC control(void *pvParameters){
  static ALT *palt  ;
  static MSG msg    ;

  palt = (ALT *)pvParameters  ;
  while(TRUE){
      msg = readALT(palt)     ;
      switch(msg.canal){
              case 0:{
                  Serial.print("IT CANAL0 : ");
                  Serial.println(msg.data)    ;
                  break         ;
              }
              case 1:{
                  Serial.println(msg.data);
                  break         ;
              }
              default:  break   ;
        } // switch
  }
} //control


void setup(){
    Serial.begin(9600)                    ;
    attachInterrupt(0,handler,FALLING)    ;

    semBin = semCreateBinary()            ;
    
    NEWALT(&altrn, 2)                     ;
        
    PAR(proc0,  &altrn)                   ;
    PAR(proc1,  &altrn)                   ;
    PAR(control,&altrn)                   ;
    
    START()                               ;
}

void loop(){

}
