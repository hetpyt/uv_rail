
#define MBI_OE  7
#define MBI_LE  8
#define MBI_CLK 9
#define MBI_SDI 10

uint8_t _test_data[] = {0b11111111, 0b11111111};
uint16_t _test_data2 = 1;
uint8_t _test_data3 = 1;

inline uint16_t ROL(uint16_t x)
{
    //return ((x & 0x8000) >> 15)|(x << 1);
    return x << 1;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(MBI_OE, OUTPUT);
  pinMode(MBI_LE, OUTPUT);
  pinMode(MBI_CLK, OUTPUT);
  pinMode(MBI_SDI, OUTPUT);
  digitalWrite(MBI_OE, LOW); // disable output
  send_data(_test_data, 2);
}

void send_data(byte data[], int count) {
  digitalWrite(MBI_LE, LOW);
  for (int i = 0; i < count; i++) {
    shiftOut(MBI_SDI, MBI_CLK, MSBFIRST, data[i]);
  }
  digitalWrite(MBI_LE, HIGH);
  delayMicroseconds(1);
  digitalWrite(MBI_LE, LOW);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  _test_data[1] = lowByte(_test_data2);
  _test_data[0] = highByte(_test_data2);
  send_data(_test_data, 2);
  _test_data2 <<= 1;
  if (_test_data2 == 0) _test_data2 = 1;
  delay(5);
//  send_data(_test_data2, 2);
//  _test_data2 =  ROL(_test_data2);
//  delay(500);
//  
//  if (_test_data2 = 1) _test_data2 = 0;
//  else _test_data2 = 1;
}
