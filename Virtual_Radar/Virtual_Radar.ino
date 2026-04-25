#include <Servo.h>
Servo myServo;

const int trigPin = 9;
const int echoPin = 8;
long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  myServo.attach(3);
}

void loop() {
  for(int i=15; i<=165; i++){  
    myServo.write(i);
    delay(30);
    distance = calculateDistance();
    
    Serial.print(i); Serial.print(",");
    Serial.print(distance); Serial.print(".");
  }

  for(int i=165; i>15; i--){  
    myServo.write(i);
    delay(30);
    distance = calculateDistance();
    
    Serial.print(i); Serial.print(",");
    Serial.print(distance); Serial.print(".");
  }
}

int calculateDistance(){ 
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}