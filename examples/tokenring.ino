

// tokenring.ino


#include <LOccam.h>

#define u_int unsigned int


// codage du  jeton

/*
bits 2..0   destinataire
bits 5..3   emetteur
bits 6..7   ........
bits 8..15  information
*/

/////////////////////////////////////////////////

CHAN canal[5]               ;
int procid[5]               ;


int  setJeton(int dest, int emet,  int data){
  int mj      ;
  
  mj = dest                 ;
  mj = mj | (emet << 3)     ;
  mj = mj | (data << 8 )    ;
  return mj                 ;
}

int read_dest(u_int jeton){
  return jeton & 0x7        ;
}

int read_emet(u_int jeton){
  return (jeton & 0x38) >> 3    ; 
}


int read_data(u_int jeton){
  return (jeton & 0xFF00) >> 8  ;
}

///////////////////////////////////////////

PROC init(void *pvParameters){
  int jeton , dest , emet, data         ;
  
  jeton = setJeton(2,0,10)              ; // 
  writeCHAN(((CHAN *)&canal[0]), jeton) ;
  
  ///////
  
  jeton = readCHAN(((CHAN *)&canal[4])) ;
  dest = read_dest(jeton)               ;
  
  writeCHAN(((CHAN *)&canal[0]), jeton) ;
  jeton = readCHAN(((CHAN *)&canal[4])) ;
  
  dest = read_dest(jeton)               ;
  data = read_data(jeton);
  emet = read_emet(jeton);
  Serial.print("process : ")         ;
  Serial.print(0)                    ;
  Serial.print(" a recu : ")         ;
  Serial.print(data)                 ;
  Serial.print(" de : ")             ;
  Serial.println(emet)               ;
  WAIT(100)                             ;
  Serial.println("\n STOP tokenring running .  BY  ")  ;
  EXIT()                             ;
}



PROC  process(void *pvParameters){
  int *id, pid , dest , emet      ;
  int data ,  tour                ;
  int jeton                       ;

   id = (int *)pvParameters       ;
   pid = *id                      ;
   tour = 0                       ;
   do{
      jeton = readCHAN(((CHAN *)&canal[pid-1])) ;
      tour++                            ;
      dest = read_dest(jeton)           ;
      if(dest == pid){
          data = read_data(jeton)       ;
          emet = read_emet(jeton)       ;
          Serial.print("process : ")    ;
          Serial.print(pid)             ;
          Serial.print(" a recu : ")    ;
          Serial.print(data)            ;
          Serial.print(" de : ")        ;
          Serial.println(emet)          ;
          data = pid + 10               ;
          dest = (pid + 2) %5           ;
          jeton = setJeton(dest,pid,data) ;
      }
      writeCHAN(((CHAN *)&canal[pid]), jeton)  ;
   }  while(tour < 2)              ;
   EXIT()                         ;
} // process




void setup() {
  int i ;
  
  Serial.begin(9600)        ;
  for(i=0;i<5;i++){ 
    NEWCHAN(&canal[i])      ;
    procid[i] = i           ; 
  }

  PAR(init, NULL)           ;
  for(i=1 ;i<5;i++) PAR(process, procid+i)   ;
  START()                   ;
}

void loop() {}

