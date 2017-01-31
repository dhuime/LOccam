
  // canal.ino

#include <LOccam.h>

CHAN mcanal                           ;

PROC  proc0(void *pvParameters){      // ecrit dans mcanal
    static CHAN *pchan                ;

    pchan = (CHAN *)pvParameters      ;
    WAIT(2)                           ; //  suspendu 2 millisecondes  
    Serial.println("\nP0 emet 123")     ;
    writeCHAN(pchan, 123)             ; 
    Serial.print("\nP0 : BY ...")     ;
    EXIT()                            ;
}

PROC  proc1(void *pvParameters){        // lit dans mcanal
    static int recu                   ;
    static CHAN *pchan                ;

    pchan = (CHAN *)pvParameters      ;
    Serial.print("P1 first : ")       ;
    recu = readCHAN(pchan)            ;
    
    Serial.print("\nP1 a recu : ")    ;
    Serial.print(recu)                ;
    Serial.print("\nP1 : BY ...")     ;
    EXIT()                            ;
}


void setup() {

  Serial.begin(9600)        ;
  Serial.flush()            ;
  
  NEWCHAN(&mcanal)          ;
  
  PAR(proc0, &mcanal)       ;
  PAR(proc1, &mcanal)       ;
  START()                   ;
  
}

void loop() {
}

