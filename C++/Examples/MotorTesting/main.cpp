#include <iostream>
#include "Navio/PCA9685.h"

#define NAVIO_RCOUTPUT_1 3
#define NAVIO_RCOUTPUT_2 4


using namespace std;

int main()
{
    PCA9685 pwm;
    pwm.initialize():
    pwm.setFrequency(50);

    while(true){

        pwm.setPWM(NAVIO_RCOUTPUT_1, 500, 1012);

    }
    return 0;
}
