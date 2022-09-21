#include <Servo.h>

const float pi = 3.14159;
const float theta_r = radians(48.0);
const float theta_p = radians(23.2);
const float theta_s[] = { -pi/3, 2*pi/3, pi, 0, pi/3, -2*pi/3 };
const float RD = 2.395;
const float PD = 3.3;
const float L1 = 1.0;
const float L2 = 4.72;
const float z_home = 4.25;
const float servo_min = 10;
const float servo_max = 170;
const float servo_mult = 1;
const float p[2][6] = {{PD*cos(pi/6 + theta_p), PD*cos(pi/6 - theta_p), PD*cos(-(pi/2 - theta_p)), -PD*cos(-(pi/2 - theta_p)), -PD*cos(pi/6 - theta_p), -PD*cos(pi/6 + theta_p)}, {PD*sin(pi/6 + theta_p), PD*sin(pi/6 - theta_p), PD*sin(-(pi/2 - theta_p)), PD*sin(-(pi/2 - theta_p)), PD*sin(pi/6 - theta_p), PD*sin(pi/6 + theta_p)}};
const float re[2][6] = {{RD*cos(pi/6 + theta_r), RD*cos(pi/6 - theta_r), RD*cos(-(pi/2 - theta_r)), -RD*cos(-(pi/2 - theta_r)), -RD*cos(pi/6 - theta_r), -RD*cos(pi/6 + theta_r)}, {RD*sin(pi/6 + theta_r), RD*sin(pi/6 - theta_r), RD*sin(-(pi/2 - theta_r)), RD*sin(-(pi/2 - theta_r)), RD*sin(pi/6 - theta_r), RD*sin(pi/6 + theta_r)}};

const int servo_pin[] = { 9, 3, 5, 11, 6, 10 };
const int servo_zero[6] = { 1710, 1280, 1700, 1300, 1680, 1300 };

Servo servo[6];

/*
    theta_r    = angle between attachment points
    theta_p    = angle between rotation points
    theta_s    = orientation of the servos
    RD         = distance to end effector attachment points
    PD         = distance to servo rotation points
    L1         = servo arm length
    L2         = connecting arm length
    z_home     = default z height with servo arms horizontal
    servo_min  = lower limit for servo arm angle
    servo_max  = upper limit for servo arm angle
    servo_mult = multiplier to convert to milliseconds
    p          = location of servo rotation points in base frame [x/y][1-6]
    re         = location of attachment points in end effector frame [x/y][1-6]

    servo_pin  = servo pin assignments,
    servo_zero = zero angles for each servo (horizontal)

    Servos 0, 2, 4: reversed (+ = down, - = up)
    Servos 1, 3, 5: normal (+ = up, - = down)
*/



void movePlatform(float sway, float surge, float heave, float pitch, float roll, float yaw) {
    static float pe[6] = { sway, surge, heave, radians(pitch), radians(roll), radians(yaw) };
    static float theta_a[6];
    static float servo_pos[6];
    static float q[3][6];
    static float dl[3][6];
    static float dl2[6];
    static float r[3][6];

    /*
        pe        = location and orientation of end effector frame relative to the base frame [sway, surge, heave, pitch, roll, yaw)
        theta_a   = angle of the servo arm
        servo_pos = value written to each servo
        q         = position of lower mounting point of connecting link [x,y,x][1-6] r = position of upper mounting point of connecting link
        dl        = difference between x,y,z coordinates of q and r
        dl2       = distance between q and r
    */

    for(int i = 0; i < 6; i++) {
        q[0][i] = L1*cos(-theta_a[i])*cos(theta_s[i]) + p[0][i]; q[1][i] = L1*cos(-theta_a[i])*sin(theta_s[i]) + p[1][i]; q[2][i] = -L1*sin(-theta_a[i]);
        r[0][i] = re[0][i]*cos(pe[4])*cos(pe[5]) + re[1][i]*(sin(pe[3])*sin(pe[4])*cos(pe[5]) - cos(pe[3])*sin(pe[5])) + pe[0];
        r[1][i] = re[0][i]*cos(pe[4])*sin(pe[5]) + re[1][i]*(cos(pe[3])*cos(pe[5]) + sin(pe[3])*sin(pe[4])*sin(pe[5])) + pe[1];
        r[2][i] = -re[0][i]*sin(pe[4]) + re[1][i]*sin(pe[3])*cos(pe[4]) + z_home + pe[2];
        dl[0][i] = q[0][i] - r[0][i]; dl[1][i] = q[1][i] - r[1][i]; dl[2][i] = q[2][i] - r[2][i];
        dl2[i] = sqrt(dl[0][i]*dl[0][i] + dl[1][i]*dl[1][i] + dl[2][i]*dl[2][i]) - L2; theta_a[i] += dl2[i];
        theta_a[i] = constrain(theta_a[i], servo_min, servo_max);

        if(i%2 == 1) {
            servo_pos[i] = servo_zero[i] + theta_a[i]*servo_mult;
        } else {
            servo_pos[i] = servo_zero[i] - theta_a[i]*servo_mult;
        }
    }

    for(int i = 0; i < 6; i++) {
        servo[i].writeMicroseconds(servo_pos[i]);
    }
}



void setup() {
    for(int i = 0; i < 6; i++) {
        servo[i].attach(servo_pin[i]);
        servo[i].writeMicroseconds(servo_zero[i]);
    }

    delay(1000);
}



void loop() {
    movePlatform(0, 0, 0, 0, 0, 0);
    delay(1000);

    movePlatform(100, 0, 0, 0, 0, 0);
    delay(1000);
}