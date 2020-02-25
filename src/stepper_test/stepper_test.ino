/*
 * Example using non-blocking mode to move until a switch is triggered.
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>
#include "A4988.h"
#include <DS3231M.h>

// endstops pins
#define STOPPER_LEFT_PIN    8
#define STOPPER_RIGHT_PIN   7

// stepper driver pins
#define DIR 3
#define STEP 4
#define ENABLE 2

// mbi pins
#define MBI_OE  9
#define MBI_LE  10
#define MBI_CLK 11
#define MBI_SDI 12

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120
// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 16
// invertion of moving direction if defined
#define INVERT_DIRECTION 1
// direction aliases
#define DIR_RIGHT   1
#define DIR_LEFT    -1
#define DIR_NONE    0
// geometry
#define STEP_DISTANCE           80 // 1 / 0.0125F  mm per motor step
#define MAX_DISTANCE_MM         340 // (mm) actual distance between endstops is 340 mm
#define HOMING_DISTANCE_MM      (MAX_DISTANCE_MM + 5) // (mm) 
#define LEFT_OFFSET_MM          0 // (mm) offset from left endstop to left edge of canvas 
#define RIGHT_OFFSET_MM         0 // (mm) offset from right endstop to right edge of canvas
#define COL_SPACING_MM          4.6 // (mm) 
#define COL_COUNT               50 // count of columns
#define ROW_COUNT               16 // count of rows

// precalculation (all in steps)
#define MAX_DISTANCE            (MAX_DISTANCE_MM * STEP_DISTANCE)
#define HOMING_DISTANCE         (HOMING_DISTANCE_MM * STEP_DISTANCE)
#define LEFT_OFFSET             (LEFT_OFFSET_MM * STEP_DISTANCE) 
#define RIGHT_OFFSET            (RIGHT_OFFSET_MM * STEP_DISTANCE)
#define COL_SPACING             (COL_SPACING_MM * STEP_DISTANCE) 

A4988 _stepper(MOTOR_STEPS, DIR, STEP, ENABLE); 
DS3231M_Class DS3231M; 

int8_t _state = 0;
uint32_t _steps_from_begin = 0; // distace in steps moved by carret
uint16_t _col_steps_left = 0; // steps left before netxt column
int8_t _moving_direction = DIR_NONE;

uint8_t _last_minute = 99;
uint8_t _test_data[] = {0b11111111, 0b11111111};

// #include "BasicStepperDriver.h" // generic
// BasicStepperDriver stepper(DIR, STEP);

void setup() {
    Serial.begin(115200);

    // Configure stopper pin to read HIGH unless grounded
    pinMode(STOPPER_LEFT_PIN, INPUT_PULLUP);
    pinMode(STOPPER_RIGHT_PIN, INPUT_PULLUP);
    // mbi pins configuration
    pinMode(MBI_OE, OUTPUT);
    pinMode(MBI_LE, OUTPUT);
    pinMode(MBI_CLK, OUTPUT);
    pinMode(MBI_SDI, OUTPUT);
    // disable output
    digitalWrite(MBI_OE, LOW); 

    _stepper.begin(RPM, MICROSTEPS);
    _stepper.setSpeedProfile(A4988::LINEAR_SPEED, 500, 500);
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    _stepper.setEnableActiveState(LOW);
    //_stepper.enable();

    Serial.println("START");

    // begin homing cycle
    begin_homing_cycle();
    
    // init RTC
    while (!DS3231M.begin()) {                                                  
        delay(3000);                                                              
    } 
}

void mbi_send_data(byte data[]) {
  digitalWrite(MBI_LE, LOW);
  for (int i = 0; i < 1; i++) {
    shiftOut(MBI_SDI, MBI_CLK, MSBFIRST, data[i]);
  }
  digitalWrite(MBI_LE, HIGH);
  delayMicroseconds(1);
  digitalWrite(MBI_LE, LOW);
}

void start_move(long steps, int8_t dir) {
    _steps_from_begin = 0;
    _col_steps_left = LEFT_OFFSET;
    _moving_direction = dir;
    _stepper.enable();
#ifdef INVERT_DIRECTION
    _stepper.startMove(-steps * dir);
#else
    _stepper.startMove(steps * dir);
#endif
}

void stop_move() {
    _stepper.stop();
    _moving_direction = DIR_NONE;
}

void begin_homing_cycle() {
    // move right to reach endstop
    _state = 1; // homing
    start_move(HOMING_DISTANCE, DIR_RIGHT);
}

void begin_print() {
    // check for error state
    if (_state < 0) return;
    
    if (digitalRead(STOPPER_LEFT_PIN) != LOW){
        // need to go left homing
        _state = 3;
        start_move(HOMING_DISTANCE, DIR_LEFT);
    }
    else {
        // already at left home position - can print
        _state = 4;
        start_move(HOMING_DISTANCE, DIR_RIGHT);
    }
}

void loop() {
    // checking endstops
    if (_moving_direction == DIR_RIGHT && digitalRead(STOPPER_RIGHT_PIN) == LOW) {
        stop_move();  
        //     
        switch (_state) {
            case 1:
                // next step of homing cycle
                _state = 2;
                start_move(HOMING_DISTANCE, DIR_LEFT);
                break;
                
            default:
                // by default going to home position
                start_move(HOMING_DISTANCE, DIR_LEFT);
                break;
        }
    } 
    else if (_moving_direction == DIR_LEFT && digitalRead(STOPPER_LEFT_PIN) == LOW) {
        stop_move(); 
        switch (_state) {
            case 2:
                // end of homing - all ok
                _state = 0;
                break;

            case 3:
                // homing left before printing
                begin_print();
                break;
            default:
                _state = 0;
                break;
        }
         
    }
    else if (_moving_direction == DIR_NONE) {
        // not moving - can make things
        DateTime now = DS3231M.now();
        if (_last_minute != now.minute() && 0 == now.second()) {
            begin_print();
            _last_minute = now.minute();
        }
    }
      
    // motor control loop - send pulse and return how long to wait until next pulse
    unsigned wait_time_micros = _stepper.nextAction();
    // 0 wait time indicates the motor has stopped
    if (wait_time_micros <= 0) {
        _stepper.disable();       // comment out to keep motor powered
        if (_state == 1 || _state == 2) {
            // error state - homing cycle failed
            _state = -1;
        }
    }
    
    // increase the number of steps taken
    ++_steps_from_begin;
    
    if (++_col_steps_left == 0) {
        // print next column
        mbi_send_data(_test_data);
        // set new offset
        _col_steps_left = COL_SPACING;
    }

    
    // (optional) execute other code if we have enough time
    //if (wait_time_micros > 100){
        // other code here
    //}
}
