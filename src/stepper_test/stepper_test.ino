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
#define RPM         160 //120//60
#define HOMING_RPM  160 //120
// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 16
// invertion of moving direction if defined
//#define INVERT_DIRECTION 1
// direction aliases
#define DIR_RIGHT   1
#define DIR_LEFT    -1
#define DIR_NONE    0
// geometry
#define STEP_DISTANCE           80 // 1 / 0.0125F  mm per motor step
#define MAX_DISTANCE_MM         340 // (mm) actual distance between endstops is 340 mm
#define HOMING_DISTANCE_MM      (MAX_DISTANCE_MM + 10) // (mm) 
#define LEFT_OFFSET_MM          55 // (mm) offset from left endstop to left edge of canvas 
#define RIGHT_OFFSET_MM         51 // (mm) offset from right endstop to right edge of canvas
#define COL_SPACING_MM          4.6 // (mm) 
#define COL_COUNT               50 // count of columns
// data
#define COL_LEN_BYTES           2 // length of column in bytes
#define SYMBOL_WIDHT            10 // width of each symbol place in rows
#define SYMBOL_COUNT            11 // 10 digits 0 - 9, : (colon)
#define SYMBOL_DATA_LEN         5 // COL_COUNT / SYMBOL_WIDHT
#define SYM_COLON               10 // index of colon symbol in SIMBOLS
// timings
#define FLARE_DELAY             15 // ms
// precalculation (all in steps)
#define MAX_DISTANCE            (MAX_DISTANCE_MM * STEP_DISTANCE)
#define HOMING_DISTANCE         (HOMING_DISTANCE_MM * STEP_DISTANCE)
#define LEFT_OFFSET             (LEFT_OFFSET_MM * STEP_DISTANCE) 
#define RIGHT_OFFSET            (RIGHT_OFFSET_MM * STEP_DISTANCE)
#define COL_SPACING             (COL_SPACING_MM * STEP_DISTANCE) 
// macro
#define LED_ON()  digitalWrite(MBI_OE, LOW);
#define LED_OFF() digitalWrite(MBI_OE, HIGH);

const uint8_t SYMBOLS[SYMBOL_COUNT][SYMBOL_WIDHT][COL_LEN_BYTES] PROGMEM = {
{
{0b00000000,0b00000000},
{0b01111110,0b11111110},
{0b10111100,0b01111101},
{0b11000000,0b00000011},
{0b11000000,0b00000011},
{0b11000000,0b00000011},
{0b11000000,0b00000011},
{0b10111100,0b01111101},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00111100,0b01111100},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b00000000,0b11111110},
{0b10000001,0b01111101},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10111101,0b00000001},
{0b01111110,0b00000000},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b10000001,0b00000001},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10111101,0b01111101},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b01111110,0b00000000},
{0b00111101,0b00000000},
{0b00000011,0b10000000},
{0b00000011,0b10000000},
{0b00000011,0b10000000},
{0b00000011,0b10000000},
{0b00111101,0b01111100},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b01111110,0b00000000},
{0b10111101,0b00000001},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10000001,0b01111101},
{0b00000000,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b01111110,0b11111110},
{0b10111101,0b01111101},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10000001,0b01111101},
{0b00000000,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b10000000,0b00000000},
{0b11000000,0b00000000},
{0b11000000,0b00000000},
{0b11000000,0b00000000},
{0b11000000,0b00000000},
{0b10111100,0b01111100},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b01111110,0b11111110},
{0b10111101,0b01111101},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10111101,0b01111101},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b01111110,0b00000000},
{0b10111101,0b00000001},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b11000011,0b10000011},
{0b10111101,0b01111101},
{0b01111110,0b11111110},
{0b00000000,0b00000000}},
{
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00001000,0b00010000},
{0b00011100,0b00111000},
{0b00011100,0b00111000},
{0b00001000,0b00010000},
{0b00000000,0b00000000},
{0b00000000,0b00000000},
{0b00000000,0b00000000}}};


A4988 _stepper(MOTOR_STEPS, DIR, STEP, ENABLE); 
DS3231M_Class DS3231M; 

#define INPUT_BUFFER_SIZE 32
char _inputBuffer[INPUT_BUFFER_SIZE];
uint8_t _inputBytes = 0;

int8_t _state = 0;
int8_t _moving_direction = DIR_NONE;
int8_t _print_direction = DIR_RIGHT;
uint16_t _steps_moved = 0;

uint8_t _last_minute = 99;
int8_t _buffer_index = 0; // index of buffer
uint8_t _buffer[COL_COUNT][COL_LEN_BYTES]; // display buffer
uint8_t _next_col_dist[COL_COUNT]; // distances in steps to next not zero filled column
uint8_t _data[5] = {0, 0, 0, 0, 0}; // symbol numbers to fill in buffer

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
    LED_OFF();

    _stepper.begin(HOMING_RPM, MICROSTEPS);
    _stepper.setSpeedProfile(A4988::LINEAR_SPEED, 700, 700);
    //_stepper.setSpeedProfile(A4988::CONSTANT_SPEED, 500, 500);
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    _stepper.setEnableActiveState(LOW);
    //_stepper.enable();

    //Serial.println("START");
    
    // init RTC
    while (!DS3231M.begin()) {                                                  
        delay(3000);                                                              
    } 
    //DS3231M.adjust();
    // begin homing cycle
    begin_homing_cycle();
    
}

void light_column(uint8_t data[], uint8_t lenght) {
    digitalWrite(MBI_LE, LOW);
    for (uint8_t i = lenght; i > 0; i--) {
        //shiftOut(MBI_SDI, MBI_CLK, MSBFIRST, data[i]);
        shiftOut(MBI_SDI, MBI_CLK, LSBFIRST , data[i-1]);
    }
    // store data
    digitalWrite(MBI_LE, HIGH);
    delayMicroseconds(1);
    digitalWrite(MBI_LE, LOW);
    //delayMicroseconds(1);
    // turn leds on
    //digitalWrite(MBI_OE, LOW);
    //delay(FLARE_DELAY);
    //digitalWrite(MBI_OE, HIGH);
}

void fill_buffer() {
    uint8_t _buffer_len = 0;
    for (uint8_t dt = 0; dt < SYMBOL_DATA_LEN; dt++) {
        for (uint8_t col = 0; col < SYMBOL_WIDHT; col++) {
            for (uint8_t bt = 0; bt < COL_LEN_BYTES; bt++) {
                uint8_t dbyte;
                if (_print_direction == DIR_RIGHT) {
                    dbyte = pgm_read_byte(&SYMBOLS[_data[dt]][col][bt]);
                }
                else {
                    dbyte = pgm_read_byte(&SYMBOLS[_data[SYMBOL_DATA_LEN - 1 - dt]][SYMBOL_WIDHT - 1 - col][bt]);
                }
                _buffer[_buffer_len][bt] = dbyte;
            }
            _buffer_len++;
        }
    }
}

void start_move(long steps, int8_t dir) {
    _moving_direction = dir;
    _steps_moved = 0;
    _stepper.enable();
#ifdef INVERT_DIRECTION
    _stepper.startMove(-steps * dir);
#else
    _stepper.startMove(steps * dir);
#endif
}

void stop_move() {
    _stepper.stop();
    _stepper.disable();
    _moving_direction = DIR_NONE;
}

void begin_homing_cycle() {
    // move right to reach endstop
    _state = 1; // homing
    start_move(HOMING_DISTANCE, DIR_RIGHT);
}

int8_t set_print_direction() {
    if (digitalRead(STOPPER_RIGHT_PIN) == LOW) _print_direction = DIR_LEFT;
    else if (digitalRead(STOPPER_LEFT_PIN) == LOW) _print_direction = DIR_RIGHT;
    else _print_direction = DIR_NONE;
}

void begin_print() {
    // check for error state
    if (_state < 0) return;

    set_print_direction();

    if (_print_direction == DIR_NONE){
        // need to go home
        _state = 3;
        start_move(HOMING_DISTANCE, DIR_LEFT);
    }
    else {
        // already at home position - can print
        fill_buffer();            
        _state = 4;
        _buffer_index = 0;
        _stepper.begin(RPM, MICROSTEPS);
        //start_move((_print_direction == DIR_RIGHT ? LEFT_OFFSET : RIGHT_OFFSET) + _next_col_dist[_buffer_index] * COL_SPACING, _print_direction);
        start_move(HOMING_DISTANCE, _print_direction);
    }
}

void ajust_time() {
    unsigned int tokens,year,month,day,hour,minute,second;
    tokens = sscanf(_inputBuffer,"%u-%u-%u %u:%u:%u;",              
                &year,&month,&day,&hour,&minute,&second);           
    if (tokens == 6) {
        DS3231M.adjust(DateTime(year,month,day,hour,minute,second));
        Serial.print(F("Date has been set.\n"));
    } else {
        Serial.print(F("Enter date in format: Y-m-d h:m:s\n"));
    }

}

void loop() {
    // checking endstops
    if (_moving_direction == DIR_RIGHT && digitalRead(STOPPER_RIGHT_PIN) == LOW) {
        stop_move(); // reset _moving_direction 
        LED_OFF();
        //Serial.println("right triger");
        switch (_state) {
            case 1:
                // homing stage #1
                _state = 2;
                //Serial.println("home1");
                start_move(HOMING_DISTANCE, DIR_LEFT); // go homing stage #2
                break;
            case 3:
                // homing before print
                //Serial.println("print3<-");
                begin_print();
                break;
        }
    }
    else if (_moving_direction == DIR_LEFT && digitalRead(STOPPER_LEFT_PIN) == LOW) {
        stop_move(); // reset _moving_direction 
        LED_OFF();
        //Serial.println("left triger");
        
        switch (_state) {
            case 2:
                // homing stage #2
                _state = 0; // homing cycle done
                //Serial.println("home2");
                break;
            case 3:
                // homing before print
                //Serial.println("print3->");
                begin_print();
                break;
        }
        
    }
    else if (_moving_direction != DIR_NONE) {  
        // motor control loop - send pulse and return how long to wait until next pulse
        unsigned wait_time_micros = _stepper.nextAction();
        _steps_moved++;
        if (_state == 4 && (_steps_moved == (_moving_direction == DIR_RIGHT ? LEFT_OFFSET : RIGHT_OFFSET - COL_SPACING))) {
            LED_ON();
            _state = 5;
            _steps_moved = 0;
            
        } else if (_state == 5 && _steps_moved == COL_SPACING) {
          if (_buffer_index < COL_COUNT) {
            light_column(_buffer[_buffer_index++], COL_LEN_BYTES);
            _steps_moved = 0;
          }
        }
    }
    
    //else if (_moving_direction == DIR_NONE) {
    else {
        // not moving - can make things
        DateTime now = DS3231M.now();
        uint8_t minute = now.minute();
        //if (_last_minute != minute && (0 == now.second() || 30 == now.second())) {
        if (now.second() % 10 == 0) {
            //Serial.println("print");
            //sprintf(inputBuffer,"%04d-%02d-%02d %02d:%02d:%02d", now.year(),          // Use sprintf() to pretty print    //
            //now.month(), now.day(), now.hour(), now.minute(), now.second());  // date/time with leading zeros     //
            //Serial.println(inputBuffer);                                              // Display the current date/time    //

            _last_minute = minute;
            uint8_t hour = now.hour();
            //fill buffer
            // hour
            uint8_t pos = 0;
            uint8_t digit = hour / 10;
            //load_symbol(digit, pos);
            _data[0] = digit;
            
            pos += SYMBOL_WIDHT;
            digit = hour % 10;
            //load_symbol(digit, pos);
            _data[1] = digit;
            
            // colon
            pos += SYMBOL_WIDHT;
            //load_symbol(SYM_COLON, pos);
            _data[2] = SYM_COLON;
            
            // minute
            pos += SYMBOL_WIDHT;
            digit = minute / 10;
            //load_symbol(digit, pos);
            _data[3] = digit;
            

            pos += SYMBOL_WIDHT;
            digit = minute % 10;
            //load_symbol(digit, pos);
            _data[4] = digit;

            begin_print();
        }
    }
    if (Serial.available()) {
        _inputBuffer[_inputBytes] = Serial.read();
        if (_inputBuffer[_inputBytes]!='\n' && _inputBytes < INPUT_BUFFER_SIZE)      
            _inputBytes++;                                                         
        else { 
            _inputBuffer[_inputBytes] = 0;
            ajust_time(); 
            _inputBytes = 0;
        }
    }
}
