

// tokenring2.ino


// See la doc in the manual . 

#include <LOccam.h>


typedef struct sp{
	CHAN 	*pred		;
	CHAN 	*succ		;
	int 	procId	;
} PARAM					;


CHAN 	canal[5]	;
PARAM	param[5]	;


PROC init(void *pvParameters){						// cas particulier du process 0
  int jeton ,pid,sender	              ;
  PARAM *param												;
  CHAN *precede, *succede							;
  
  param = (PARAM *)pvParameters       ;
  precede = param->pred  						  ;
  succede = param->succ					      ;

  pid = param->procId                 ;
  sender = (pid+4) %5                ;
  
  jeton = 0              							;  
  writeCHAN(succede, jeton) 					;
  jeton = readCHAN(precede) 				  ;
  
  jeton++                             ;
  writeCHAN(succede, jeton)          ;
  jeton = readCHAN(precede)           ;
  Serial.print("process : ")      ;
  Serial.print(pid)               ;
  Serial.print(" a recu : ")      ;
  Serial.print(jeton)             ;
  Serial.print(" de : ")          ;
  Serial.println(sender)          ;
  //
  Serial.println("BY ...")            ;
  
  EXIT()                             	;
}


PROC  process(void *pvParameters){
  int  pid , sender            				;
  int jeton , tour            				;
  
  PARAM *param                        ;
  CHAN *precede, *succede             ;
  
  param = (PARAM *)pvParameters       ;
  pid = param->procId                 ;
  precede = param->pred               ;
  succede = param->succ               ;
  sender = (pid + 4) %5               ;
  tour = 0                              ;
  do{
        jeton = readCHAN(precede)       ; 
        Serial.print("process : ")      ;
        Serial.print(pid)               ;
        Serial.print(" a recu : ")      ;
        Serial.print(jeton)             ;
        Serial.print(" de : ")          ;
        Serial.println(sender)          ;
        jeton++                         ;
        tour++                          ;
        writeCHAN(succede, jeton)       ;
  } while(tour <2);
   EXIT()                             	;
} // process




void setup() {
  int i ;
  
  Serial.begin(9600)        					;
  
  for(i=0;i<5;i++)NEWCHAN(&canal[i]) 	;
  
 for(i=0; i<5; i++){
	
    param[i].succ = &canal[i]  				;
    param[i].pred = &canal[(i+4)%5] 	;
    param[i].procId = i       				;
 
 }
    

  PAR(init,&param[0])                 ;
  for(i=1; i<5; i++ ) PAR(process,&param[i])             ;
  
  START(NULL)               					;
}

void loop() {}







