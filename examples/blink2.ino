
// blink2.ino

// OK Arduino-1.8.5 le 30/10/17

#include <LOccam.h>

const int redled = 3    ;
const int greenled = 2  ;

int procid[2] ;


PROC  process(void *pvParameters){
     int i, *id, pid, led   ;
  
   id = (int *)pvParameters       ;
   pid = *id                      ;

    if(!pid) led = redled        ;
    else led = greenled          ;

      for(i=0;i<5;i++){ 
        digitalWrite(led,LOW)     ;
        WAIT(1000)                ;
        digitalWrite(led,HIGH)    ;
        WAIT(1000)                ;
      }  
    
    Serial.print( pid);  
    Serial.print("  BY ....\n")   ;
    EXIT()                        ;
} 


void setup() { 
  int i         ;

  Serial.begin(9600)        ;
  pinMode(redled,OUTPUT)    ;
  pinMode(greenled,OUTPUT)  ;
  
  for(i=0;i<2;i++) procid[i]=i            ;

  // Replicated parallel
  for(i=0;i<2;i++) PAR(process,procid+i)  ;
  START(NULL)   ;
}


void loop() {
  
}
