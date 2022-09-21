#include <Servo.h>

int d = 10;
Servo Servo1;
Servo Servo2;
Servo Servo3;
Servo Servo4;
Servo Servo5;
Servo Servo6;

void setup() {
  Servo1.attach(3);
  Servo2.attach(5);
  Servo3.attach(6);
  Servo4.attach(9);
  Servo5.attach(10);
  Servo6.attach(11);
}

void loop() {
  d = 10;
  Servo1.write(d);
  Servo2.write(d);
  Servo3.write(d);
  Servo4.write(d);
  Servo5.write(d);
  Servo6.write(d);
  delay(1000);

  d = 170;
  Servo1.write(d);
  Servo2.write(d);
  Servo3.write(d);
  Servo4.write(d);
  Servo5.write(d);
  Servo6.write(d);
  delay(1000);
}