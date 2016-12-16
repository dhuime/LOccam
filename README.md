# LOccam
LOccam a light implemantation of OCCAM langage for Arduino


LOccam based upon OCCAM langage is  a wrapper , writen in C,  on the top of the 
FreeRTOS whose adaptation for Arduino boards by Bill Greiman (https://github.com/greiman),
 we choose for our project.
 
LOccam is very simple to learn and implements very few constructs such : PAR, 
CHAN and  ALT   and is idealy suited for concurrent programming in the 
hobbyist world of Arduino.
  
Installation of LOccam in  Arduino's IDE requires you  copy in the libraries 
folder the following files :

  AVR platforms : the files LOccam and FreeRTOS_AVR ( Bill Greiman ) which is 
the default .

  ARM platforms : the files LOccam and FreeRTOS_ARM (Bill Greiman) . 
  In this later case you  must edit the file LOccam.h .
  You comment  #include < FreeRTOS_AVR > and  uncomment   #include < FreeRTOS_ARM >
  that's all .
  

The manual fully explains the main constructs of LOccam .
These are illustrated by many examples who where all tested on the ATMega2560  board .
    
    
Content of different folders :

LOccam 

greiman  : include  FreeRTOS_AVR and FreeRTOS_ARM of Bill Greiman

examples : The examples

manual   : LOccam manual
