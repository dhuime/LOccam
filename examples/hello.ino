#include <LOccam.h>

PROC hello(void *pvParameters){
  
  Serial.println("Hello World !!")  ;
  EXIT()          ; // terminate hello
}

void setup() {
  Serial.begin(9600)  ;
  
  PAR(hello, NULL)      ; // declare hello without parameters
  START()               ; // start hello
}

void loop(){}
