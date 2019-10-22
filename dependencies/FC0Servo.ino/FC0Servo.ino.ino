const int maxServoAngle = 180;
const String angleCommandString = "ANGLE";

#include <Servo.h>

Servo angleServo;
int angle = 0;

void setup() {
  // put your setup code here, to run once:
  angleServo.attach(9, 750, 2250);
  angleServo.write(angle);
  
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > angleCommandString.length()){
    String commandRead = Serial.readStringUntil("\r");

    int indexSplit = commandRead.indexOf(" ");
    if(indexSplit > 0 && commandRead.length() >= indexSplit+1){
      String commandName = commandRead.substring(0,indexSplit);
      String angleString = commandRead.substring(indexSplit+1);
      if(commandName == angleCommandString){
        int angleToSet = angleString.toInt();
        if(angleToSet<=maxServoAngle && angleToSet>=0){
          angle = angleToSet;
          angleServo.write(angle);
        }
        Serial.println(commandName+':'+angle);
      }else{
        Serial.println("ERROR");
      }
    }else{
      indexSplit = commandRead.indexOf("?");
      if(indexSplit > 0){
        String commandName = commandRead.substring(0,indexSplit);
        if(commandName == angleCommandString){
          Serial.println(commandName+':'+angle);
        }else{
          Serial.println("ERROR");
        }
      }else{
        Serial.println("ERROR");
      }
    }
  }
}
