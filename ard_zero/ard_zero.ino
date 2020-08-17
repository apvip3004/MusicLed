#include <Wire.h>
#include <I2S.h>

#define RMS_SAMPLE 1000

int rms_lvl;
int n = 0;
char rms_tab[2] = {};
char data_transmit_rms[2] = {};
char data_transmit_command[15] = {};

void new_rms_lvl() {
  // read a sample
  int sample = I2S.read();
  static int32_t rms_flt = 0;
  static int32_t avg_flt = 0;
  uint32_t x;
  uint64_t rms_sq;
  uint32_t v;
  
  
  
  if ((sample == 0) || (sample == -1) ) {
    return;
  }
  // convert to 18 bit signed
  sample = (int16_t)(sample >> 16);
  
  x = sample - ((avg_flt + 128) >> 8);
  avg_flt += x;

  v = sample - ((avg_flt + 128) >> 8);
  x = (v * v >> 2) - ((rms_flt + 128) >> 8);
  rms_flt += x;


  n++;

  // if it's non-zero print value to serial
  if (n > RMS_SAMPLE) {

//    SerialUSB.println(rms_flt);
    
    rms_sq = rms_flt;
    rms_sq *= rms_sq;

    rms_lvl = 0;
    
    while (rms_sq) {
      rms_sq >>= 1;
      rms_lvl++;
    }

    rms_lvl -= 22;
    
    if (rms_lvl <= 0)
      rms_lvl = 1;
    if (rms_lvl > 50)
      rms_lvl = 50;

    n = 0;
  }

}

void rms_encode(void) {
    memset(data_transmit_rms, 0, sizeof(data_transmit_rms));
    
    itoa(rms_lvl, rms_tab, 10);
    
    for(int8_t i = 0; i < sizeof(rms_tab); i++ ) {
      data_transmit_rms[i] = rms_tab[i];
    }
}

void command_encode(void) {
  memset(data_transmit_command, 0, sizeof(data_transmit_command));
  data_transmit_command[0] = 'C';
  data_transmit_command[1] = 'H';
  data_transmit_command[2] = '1';
  data_transmit_command[3] = '_';
  data_transmit_command[4] = 'B';
  data_transmit_command[5] = 'R';
  data_transmit_command[6] = 'I';
  data_transmit_command[7] = 'G';
  data_transmit_command[8] = 'H';
  data_transmit_command[9] = 'T';
  data_transmit_command[10] = '_';
  data_transmit_command[11] = '1';
  data_transmit_command[12] = '5';
  data_transmit_command[13] = '0';
}


void setup() {
  // Start the I2C Bus as Master
  Wire.begin(); 
    // start I2S at 26 kHz with 32-bits per sample
  if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 32)) {
    while (1); // do nothing
  }

  command_encode();
  Wire.beginTransmission(9); 
  for(int8_t i = 0; i < sizeof(data_transmit_command); i++ ) {
    Wire.write(data_transmit_command[i]);
  }
  Wire.endTransmission();

  delay(1000);
}

void loop() {

  
  
  new_rms_lvl();
  
//  if (n > RMS_SAMPLE) {
        Wire.beginTransmission(9); 
        for(int8_t i = 0; i < sizeof(data_transmit_rms); i++ ) {
          Wire.write(data_transmit_rms[i]); 
        }
        Wire.endTransmission(); 
//    n = 0;
//  }
  delay(100);
}
