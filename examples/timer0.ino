
//  timer0 

#include <LOccam.h>


TIMER timer ;

PROC process(void *pvParameters) {
  static TIMER *tmr           ;
  static u_long tim1, tim2    ;

  Serial.println("\nprocess START ");
  tmr = (TIMER *)pvParameters ;

  tim1 = readTIMER(tmr)     ;
  do{
    waitTIMER(tmr,100L)     ; // attente de 1 secondes
    tim2 = readTIMER(tmr)    ;
    Serial.print(" *")       ;
  } while((tim2 -tim1) < 2000L );


  
  Serial.println("\nprocess BY ...");
  EXIT()                   ;

}


void setup() { 
  
  Serial.begin(9600)        ;

  NEWTIMER(&timer, 10)      ; // timer ticks tous les 10 millis 

  PAR(runTIMER, &timer)     ; // lance le timer
  PAR(process, &timer)      ;
  START()                   ;

}

void loop() {}
