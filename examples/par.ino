
// par.ino

#include <LOccam.h>

char *str1 = "str1 est passe en parametre  a  proc1" ;


PROC  proc0(void *pvParameters){
    static int i  ;
    
    for(i=0;i<20;i++){ 
      Serial.print("0")             ;
      WAIT(2)                       ;  //suspendu 2 millisec
    }            
    Serial.print("\nP0 BY ....\n")  ;
    EXIT()                            ;
}

PROC  proc1(void *pvParameters){
    static int i  ;
    static char *pstr ;

    pstr = (char *)pvParameters     ;
    Serial.println(pstr)            ;  // affiche str1
    
    for(i=0;i<10;i++){ 
      Serial.print("1")             ;
      WAIT(2)                       ;  // suspendu 2 millisec
    }
    Serial.print("\nP1 BY ...\n")   ;
    EXIT()                            ;
}

void setup() {

  Serial.begin(9600)        ;
  Serial.flush()            ;
      
  PAR(proc0, NULL)          ;  // aucun parametre passé
  PAR(proc1, str1)          ;  // passage de str1 en parametre
  START()                   ;
}


void loop() {
  
}
