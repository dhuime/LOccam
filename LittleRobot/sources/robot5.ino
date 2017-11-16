
// OK le 15/11/2017

/*  robot5.ino
 *
 * 
 * A simple robot with two motors an a ultra sound sensor
 * 
 * The motors are driven by Ardumoto sparkfun shield
 * 
 *  The ultrasonic detector is SRF05.   
 *  3 pins of the latter are used: 5V, GND, TRIGGER (7) 
 *
 * 
 * Coding of messages issued by the controller for the control of motors
 * 
 * bit 0  motor         0 : MOTOR_A  ; 1:MOTOR_B
 * bit 1  direction     0 : backward : 1 forward 
 * bits 8..15           0 <= speed <= 0xFF
 */


  #include <LOccam.h>
  #include "tsl2561.h"
  
#define MOTOR_A 0
#define MOTOR_B 1

// commandes de direction des moteurs

#define CW 1  // en avant
#define CCW 0 // en arriere 

// commandes de stop des moteurs

#define stop_A  0
#define stop_B  1

#define forward_A 0x8002
#define forward_B 0x8003

#define backward_A 0x8000
#define backward_B 0x8001

 
// La structure "altchan"  permet le passage de 2 parametres au processus control

typedef struct st {
    ALT   *_control     ;
    CHAN  *_motors      ;
}   altchan             ;


// Les parametres pour Ardumoto

const char PWMA = 3;  // PWM control (speed) for motor A
const char PWMB = 11; // PWM control (speed) for motor B
const char DIRA = 12; // Direction control for motor A
const char DIRB = 13; // Direction control for motor B

CHAN 	chanMotors                	;
ALT  	altControl                	;
altchan parameters			          ;

////////////////////   utilitaires ///////////////////////////////


  void driveMotor(unsigned command){
    byte motor , dir,sped  		          ;
     
    motor =  command & 1                ;
    dir   = (command & 2) >> 1          ;
    sped  = (command & 0xFF00) >> 8     ;
    
    if (motor == MOTOR_A){
      digitalWrite(DIRA, dir);
      analogWrite(PWMA, sped);
    }
    else if (motor == MOTOR_B){
      digitalWrite(DIRB, dir);
      analogWrite(PWMB, sped);
    }  
}


///////////////////////////////////////////////////////////////////////


  PROC tsl2561 (void *pvParameters) { // Managing the light received by the TSL2561
     
  ALT *palt                   	;
  MSG msg                     	;
  
  unsigned char  gain           ;  // Gain setting, 0 = X1, 1 = X16 definit en partie m
  unsigned char ltime           ;  // definit , en partie,la valeur de ms
  unsigned  ms                  ;  // ms : Integration ("shutter") time in milliseconds
    
  unsigned  data0, data1	      ;  // determinent la valeur de lux 
  double lux            	      ;  // Resulting lux value
  
  boolean goodLux          	    ;  // True if neither sensor is saturated
  byte error            	      ;
  
  //..........................................
   
  palt =(ALT *) pvParameters  ;
  msg.canal = 1               ;
    
  ////// fin des initialisations
     
  begin()         	                     ; // initialise I2C
  gain = 0                               ;
  ltime = 2                              ;
   
  setTiming(gain,ltime,&ms)             ; // set ms
  setPowerUp()                          ; // start
  
  while(true){
    WAIT(ms);
    // Retrieve data0, data1 and determine lux
  
    if (getData(&data0,&data1)) { 
      goodLux = getLux(gain,ms,data0,data1,&lux);
      msg.data = (unsigned)lux    	    ;
      writeALT(palt,msg)          	    ;
    }
    else {
      Serial.println("ERREUR TSL2561")	;
      EXIT()                            ;
    }
  }  // while()
    
} //TSL2561()



PROC srf05(void *pvParameters){
  //
  //  pilote le srf05  detecteur a ultra sons
  //  communique avec control sur le canal 0
  
  ALT *palt			;
  MSG	msg			;
  unsigned  duration, distance  ;   
  const int TRIGGER = 7       	;
  
  palt =(ALT *) pvParameters	;
  msg.canal = 0			;
  
  while(true){
    pinMode(TRIGGER, OUTPUT)    ;
    digitalWrite(TRIGGER, LOW)  ;   // Make sure pin is low before sending a short high to TRIGGER 
    delayMicroseconds(2)        ;
  
    digitalWrite(TRIGGER, HIGH) ;   // Send a short 10 microsecond high burst on pin to start ranging
    delayMicroseconds(10)       ;
  
    digitalWrite(TRIGGER, LOW)  ;   // Send pin low again before waiting for pulse back in
    pinMode(TRIGGER, INPUT)     ;
    duration = pulseIn(TRIGGER, HIGH);     // Reads echo pulse in from SRF05 in micro seconds
  
    distance = duration/58      ;  // distance in cm
    msg.data = distance		;
    writeALT(palt,msg)	      	;
    WAIT(100)			;
  
 } // while()
} // srf05

//............................................................
 

PROC motors(void *pvParameters){
  // pilote les moteurs
  // recoit des commandes de control sur le canal chanMotors
  
    CHAN   *canal                 ;
    unsigned command              ;
  
    canal = (CHAN *) pvParameters ;
    while(true){  
      command = readCHAN(canal)   ;
      driveMotor(command)         ;
   }
  
} // motors


//...........................................................


PROC control(void *pvParameters){
//
// Processus alternatif
// Recoit des codes du srf05 sur le canal 0 
// recoit des codes du tsl2561 sur le canal 1

  altchan  *paltchan            ;
  ALT  *palt                    ;
  CHAN *moteurs                 ; 
  MSG msg                       ;
  int encore                    ;
  unsigned lux  , distance      ;

  paltchan = (altchan *)pvParameters  ;
  palt = paltchan->_control     ;
  moteurs= paltchan->_motors    ;
  //
  writeCHAN(moteurs,forward_A)  ;
  writeCHAN(moteurs,forward_B)  ;
  encore = 1                    ;
  do{
    msg = readALT(palt)         ;
    switch(msg.canal){
        case 0:{                  //	 from srf05  : msg.data = distance en CM
            distance = msg.data ;
            if(distance < 50){
              writeCHAN(moteurs, backward_A) ;
              writeCHAN(moteurs, backward_B) ;
            }
            break                 ;
        }
        case 1:{                  //  	from tsl2561 : msg.data = lux
            lux = msg.data        ;
            if(lux < 20){
              Serial.println("STOP MOTEURS .....");
              writeCHAN(moteurs,stop_A) ;
              writeCHAN(moteurs,stop_B) ;
              EXIT()            ;
            }
            break               ;
        }
        default: break          ;
    }
  } while(encore)               ;
    
} // control



void setup() {
  
  Serial.begin(9600)      ;
    
  // initialisations pour Ardumoto 
  
  pinMode(PWMA, OUTPUT)   ;
  pinMode(PWMB, OUTPUT)   ;
  pinMode(DIRA, OUTPUT)   ;
  pinMode(DIRB, OUTPUT)   ;
  
  digitalWrite(PWMA, LOW) ;
  digitalWrite(PWMB, LOW) ;
  digitalWrite(DIRA, LOW) ;
  digitalWrite(DIRB, LOW) ;
  
   ///////////////////////////////


  NEWCHAN(&chanMotors)      ;
  NEWALT(&altControl, 2)    ;

  parameters._control = &altControl   ;
  parameters._motors = &chanMotors    ;
  
  PAR(srf05 , &altControl)  ;	// communique avec control via le canal 0
  PAR(control,&parameters)  ;
  PAR( motors, &chanMotors) ;
  PAR(tsl2561,&altControl)  ;  // communique avec control via le canal 1
  
  START(NULL)               ;
  Serial.println("BY ...")  ;

}

void loop(){}
