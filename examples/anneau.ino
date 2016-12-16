
// anneau.ino

#include <LOccam.h>

#define NBP 20          // nbre de processus dans l'anneau

typedef struct cc{
  CHAN *first           ;
  CHAN *second          ;
} TWOCHAN               ;

CHAN canal[NBP]         ;
int procid[NBP]         ;




PROC  process(void *pvParameters){
    int *id  , pid , recu               ; //déroge à la regle static 

    id = (int  *) pvParameters          ;
    pid = *id                           ;
    
    if(pid  > 0){
        recu = readCHAN(((CHAN *)&canal[pid-1]))   ; // lit c pid-1 
        recu++                          ;
        writeCHAN(((CHAN *)&canal[pid]),recu)      ; // ecrit c pid
    }
    else{
        writeCHAN(((CHAN *)&canal[0]), 0)          ; // ecrit c0  
        recu = readCHAN(((CHAN *)&canal[NBP-1]))   ; // lit c nbp-1 
        //
        Serial.print("process 0 a recu : ");
        Serial.println(recu)              ;
    }
    EXIT()                                ;
}


void setup() {
  int i    ;
  
  Serial.begin(9600)       ;
  
  for(i=0;i< NBP; i++) {
    NEWCHAN(&canal[i])     ;
    procid[i] = i          ;
  }
 
  for(i=0;i< NBP; i++) PAR(process, procid+i);
 
  Serial.println("START\n") ;
  START()                   ;

}

void loop() {}

