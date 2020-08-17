#include <Wire.h>
#include "FastLED.h"
#include "defines.h"
#include <avr/wdt.h>

#define LED_PIN_1     5
#define LED_PIN_2     6
#define LED_PIN_3     7

#define NUM_LEDS_1    156
#define NUM_LEDS_2    141
#define NUM_LEDS_3    150

#define LED_TYPE    WS2812


/*  MUSIC ANIMATIONSS
 * 1-   ma_level_strip(ch)
 * 2-   ma_level_strip(ch)
 * 3-   ma_level_strip(ch)
 * 4-   ma_level_strip(ch)
 * 5-   ma_level_strip_split(ch, 2)
 * 6-   ma_level_strip_split(ch, 3)
 * 7-   ma_level_strip_split(ch, 4)
 * 8-   ma_level_strip_split(ch, 5)
 * 9-   ma_level_strip_split(ch, 6)
 * 10-  ma_level_strip_split(ch, 7)
 * 11-  ma_strobe(ch, 7)
 * 12-  ma_strobe_split(ch, 7)
 * 13-  ma_gun(ch)
 * 
 * IDLE ANIMATIONS
 * 1- mi_rainbow_flow(ch)
 * 2- 
 * 3- 
 * 4- 
 * 5- 
 * 6-
 * 7-
 * 8-
 * 
 */
struct StripLed {
  uint8_t leds_nr;
  uint8_t hue;
  uint8_t saturation;
  uint8_t brightness;
  uint8_t mode;
  uint8_t music;
  uint8_t idle;
  bool    off;
};

StripLed ledStrip1 {NUM_LEDS_1, 0, 0, 255,   0, 1, 1, true};
StripLed ledStrip2 {NUM_LEDS_2, 0, 0, 255,   0, 1, 1, true};
StripLed ledStrip3 {NUM_LEDS_3, 0, 0, 255,   0, 1, 1, true};
bool ledStripALL = false;

CRGB led_strip_1[NUM_LEDS_1];
CRGB led_strip_2[NUM_LEDS_2];
CRGB led_strip_3[NUM_LEDS_3];

uint8_t rms_lvl = 0;
uint8_t rms_lvl_rec = 0;
char data_rec[20];

void sm_loader(uint8_t ch);


double threeway_min(double a, double b, double c) {
    return min(a, min(b, c));
}

double threeway_max(double a, double b, double c) {
    return max(a, max(b, c));
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t* rgbToHsv(byte r, byte g, byte b) {
    double rd = (double) r/255;
    double gd = (double) g/255;
    double bd = (double) b/255;
    double max = threeway_max(rd, gd, bd), min = threeway_min(rd, gd, bd);
    double h, s, v = (max + min) / 2;
    static uint8_t hs[2] = {0,0};
    
    double d = max - min;
    s = max == 0 ? 0 : d / max;

    if (max == min) { 
        h = 0; // achromatic
    } else {
        if (max == rd) {
            h = (gd - bd) / d + (gd < bd ? 6 : 0);
        } else if (max == gd) {
            h = (bd - rd) / d + 2;
        } else if (max == bd) {
            h = (rd - gd) / d + 4;
        }
        h /= 6;
    }

    h = mapfloat(h, 0, 1, 0, 255);
    s = mapfloat(s, 0, 1, 0, 255);
    hs[0] = h;
    hs[1] = s;

    return hs;
}

void set_data_color(uint8_t ch, uint8_t* col) {
  
  switch (ch) {
    case 1:
      ledStripALL = false;
      ledStrip1.hue = *col;
      ledStrip1.saturation = *(col + 1);
    break;
    case 2:
      ledStripALL = false;
      ledStrip2.hue = *col;
      ledStrip2.saturation = *(col + 1);
    break;
    case 3:
      ledStripALL = false;
      ledStrip3.hue = *col;
      ledStrip3.saturation = *(col + 1);
    break;
    case 4:
      ledStripALL = true;
      ledStrip1.hue = *col;
      ledStrip1.saturation = *(col + 1);
      ledStrip2.hue = *col;
      ledStrip2.saturation = *(col + 1);
      ledStrip3.hue = *col;
      ledStrip3.saturation = *(col + 1);
    break;
  }
}

void set_data(uint8_t ch, Commands comm, uint8_t val) {
  
  switch(ch) {
    case 1:
      ledStripALL = false;
      switch (comm) {
        case VALUE:
          ledStrip1.brightness = map(val, 0, 100, 0, 255);
        break;
        case MODE:
          ledStrip1.mode = val;
        break;
        case MUSIC:
          ledStrip1.music = val;
        break;
        case IDLEA:
          ledStrip1.idle = val;
        break;
      }
    break;
    case 2:
      ledStripALL = false;
      switch (comm) {
        case VALUE:
          ledStrip2.brightness = map(val, 0, 100, 0, 255);
        break;
        case MODE:
          ledStrip2.mode = val;
        break;
        case MUSIC:
          ledStrip2.music = val;
        break;
        case IDLEA:
          ledStrip2.idle = val;
        break;
      }
    break;
    case 3:
      ledStripALL = false;
      switch (comm) {
        case VALUE:
          ledStrip3.brightness = map(val, 0, 100, 0, 255);
        break;
        case MODE:
          ledStrip3.mode = val;
        break;
        case MUSIC:
          ledStrip3.music = val;
        break;
        case IDLEA:
          ledStrip3.idle = val;
        break;
      }
    break;
    case 4: // ALL
      ledStripALL = true;
      switch (comm) {
        case VALUE:
          ledStrip1.brightness = map(val, 0, 100, 0, 255);
          ledStrip2.brightness = map(val, 0, 100, 0, 255);
          ledStrip3.brightness = map(val, 0, 100, 0, 255);
        break;
        case MODE:
          ledStrip1.mode = val;
          ledStrip2.mode = val;
          ledStrip3.mode = val;
        break;
        case MUSIC:
          ledStrip1.music = val;
          ledStrip2.music = val;
          ledStrip3.music = val;
        break;
        case IDLEA:
          ledStrip1.idle = val;
          ledStrip2.idle = val;
          ledStrip3.idle = val;
        break;
      }
    break;
  }
}

uint8_t decode_channel() {
  uint8_t channel;
  
  if (data_rec[2] == '1')
    channel = 1;
  else if (data_rec[2] == '2') 
    channel = 2;
  else if (data_rec[2] == '3') 
    channel = 3;
  else if (data_rec[2] == '4') 
    channel = 4;
  else
    channel = 0;
    
  return channel;
}

Commands decode_command() {
  Commands command;
  
  if (data_rec[4] == 'M' && data_rec[5] == 'O') 
    command = MODE;
  else if (data_rec[4] == 'M' && data_rec[5] == 'U') 
    command = MUSIC;
  else if (data_rec[4] == 'I' && data_rec[5] == 'D') 
    command = IDLEA;
  else if (data_rec[4] == 'V' && data_rec[5] == 'A') 
    command = VALUE;
  else if (data_rec[4] == 'C' && data_rec[5] == 'O') 
    command = COLOR;

  return command;
}

int decode_value() {
  uint8_t value = 0;
  char val[3];

  for (uint8_t i = 0; i < 3; i++) {
    val[i] = data_rec[8 + i];
  }
  value = atoi(val);
  return value;
} 

uint8_t* decode_color() {
  uint8_t hue = 0;
  int rgb[3];
  char val[3];
  char c;
  uint8_t ind_r = 8;
  uint8_t ind_g, ind_b = 0;
  
  // Red
  for (uint8_t j = 0; j < 3; j++) {
      c = data_rec[ind_r + j];
      if (c == '_') 
        break;
      val[j] = c;
      ind_g = ind_r + j + 2;
  }
  rgb[0] = atoi(val);
  memset(val, 0, sizeof(val));
  
  // Green
  for (uint8_t j = 0; j < 3; j++) {
      c = data_rec[ind_g + j];
      if (c == '_') 
        break;
      val[j] = c;
      ind_b = ind_g + j + 2;
  }
  rgb[1] = atoi(val);
  memset(val, 0, sizeof(val));
  
  // Blue
  for (uint8_t j = 0; j < 3; j++) {
      c = data_rec[ind_b + j];
      if (c == '_') 
        break;
      val[j] = c;
  }
  rgb[2] = atoi(val);
  memset(val, 0, sizeof(val));

//  hue = rgbToHsv(rgb[0],rgb[1],rgb[2]);
  uint8_t* hs;
  hs = rgbToHsv(rgb[0],rgb[1],rgb[2]);

  return hs;
} 

// CH1_MOD_0-3
// CH1_MUS_1-10
// CH1_IDL_1-10
// CH1_VAL_0-100
// CH1_COL_0-100
void receive_command(void) {

  uint8_t channel = decode_channel();
  Commands command = decode_command();

  if (command == COLOR) {
    uint8_t* color = decode_color();
    
    set_data_color(channel, color);
  }
  else {
    uint8_t value = decode_value();
    
    Serial.println("CH:");
    Serial.println(channel);
    Serial.println("Command:");
    Serial.println(command);
    Serial.println("value");
    Serial.println(value);
    Serial.println(""); 
    
    set_data(channel, command, value);
  }
}

bool is_music_on () {
  if (ledStrip1.mode == 2 || ledStrip2.mode == 2 || ledStrip3.mode == 2)
    return true;
  return false;
}

void init_led(void) {

  fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(0,0,0));
  fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(0,0,0));
  fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(0,0,0));

  FastLED.show();
}

void setup() {
  wdt_enable(WDTO_8S);
  
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);

  FastLED.addLeds<NEOPIXEL, LED_PIN_1>(led_strip_1, ledStrip1.leds_nr);
  FastLED.addLeds<NEOPIXEL, LED_PIN_2>(led_strip_2, ledStrip2.leds_nr);
  FastLED.addLeds<NEOPIXEL, LED_PIN_3>(led_strip_3, ledStrip3.leds_nr);

  init_led();

  Serial.begin(9600);
}


void receiveEvent(int bytes) {

  uint8_t cnt = 0;
  bool cmnd = false;
  char c;
  while (Wire.available() > 0) { // loop through all but the last
    c = Wire.read();        // receive byte as a character
    
    if (cnt == 0 ) {
      if (c == 'C') {
        cmnd = true;
        memset(data_rec, 0, sizeof(data_rec));
      }
//      Serial.print("Data: ");
    } 
//    Serial.print(c);
    
    if(!cmnd) 
    {
      uint8_t rms = int(c)- 48;
      if(cnt == 0) 
      {
        rms_lvl_rec = rms*10;
      } 
      else if(cnt == 1) 
      {
        rms_lvl_rec = rms_lvl_rec + rms;
      }
    } 
    else 
    {
      data_rec[cnt] = c;
    }
    cnt ++;
  }
//  Serial.println("");
  if(cmnd)
    receive_command();
}


void loop() {
  wdt_reset();
  
  if (is_music_on() && (rms_lvl_rec != rms_lvl)) {
    rms_lvl = rms_lvl_rec;
  }

  if (!ledStripALL) {
    sm_loader(1);
    sm_loader(2);
    sm_loader(3);
  } else { // ALL channels
    sm_loader(4);
  }
  
  

}

//  ---------------------- FINISH_LOOP -------------------------

void clear_strip(uint8_t ch) {
  switch (ch) {
    case 1:
      fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(0, 0, 0));
    break;
    case 2:
      fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(0, 0, 0));
    break;
    case 3:
      fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(0, 0, 0));
    break;
    case 4:
      fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(0, 0, 0));
      fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(0, 0, 0));
      fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(0, 0, 0));
    break;
  }
}

void light_strip(uint8_t ch) {
  switch (ch) {
    case 1:
      fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(ledStrip1.hue, ledStrip1.saturation, ledStrip1.brightness));
    break;
    case 2:
      fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(ledStrip2.hue, ledStrip2.saturation, ledStrip2.brightness));
    break;
    case 3:
      fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(ledStrip3.hue, ledStrip3.saturation, ledStrip3.brightness));
    break;
    case 4:
      fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(ledStrip1.hue, ledStrip1.saturation, ledStrip1.brightness));
      fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(ledStrip2.hue, ledStrip2.saturation, ledStrip2.brightness));
      fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(ledStrip3.hue, ledStrip3.saturation, ledStrip3.brightness));
    break;
  }
  FastLED.show();
}

void sm_off_mode(uint8_t ch) {

        
        
  switch (ch) {
    case 1:
      if (ledStrip1.off) {
        return;
      }
      ledStrip1.off = true;
    break;
    case 2:
      if (ledStrip2.off)
        return;
      ledStrip2.off = true;
    break;
    case 3:
      if (ledStrip3.off) {
        return;
      }
      ledStrip3.off = true;
    break;
    case 4:
      if (ledStrip3.off && ledStrip2.off && ledStrip1.off)
        return;
      ledStrip1.off = true;
      ledStrip2.off = true;
      ledStrip3.off = true;
    break;
  }
  
  Serial.println("");
  Serial.println("clear_strip");
  Serial.println(ch);
  clear_strip(ch);
  FastLED.show();
}

void sm_manual_mode(uint8_t ch) {

  static uint8_t h1 = 0;
  static uint8_t h2 = 0;
  static uint8_t h3 = 0;
  static uint8_t s1 = 0;
  static uint8_t s2 = 0;
  static uint8_t s3 = 0;
  static uint8_t v1 = 0;
  static uint8_t v2 = 0;
  static uint8_t v3 = 0;
  static uint8_t mode1 = 0;
  static uint8_t mode2 = 0;
  static uint8_t mode3 = 0;
  
  delay(1);
  switch (ch) {
    case 1:
      if(mode1 != ledStrip1.mode || h1 != ledStrip1.hue || s1 != ledStrip1.saturation || v1 != ledStrip1.brightness) {
        h1 = ledStrip1.hue;
        s1 = ledStrip1.saturation;
        v1 = ledStrip1.brightness;
        mode1 = ledStrip1.mode;
        light_strip(ch);
        
        if (ledStrip1.off)
          ledStrip1.off = false;
//        Serial.println("ZAPAL 1");
      }
    break;
    case 2:
      if(mode2 != ledStrip2.mode || h2 != ledStrip2.hue || s2 != ledStrip2.saturation || v2 != ledStrip2.brightness) {
        h2 = ledStrip2.hue;
        s2 = ledStrip2.saturation;
        v2 = ledStrip2.brightness;
        mode2 = ledStrip2.mode;
        light_strip(ch);
        
        if (ledStrip2.off)
          ledStrip2.off = false;
//        Serial.println("ZAPAL 2");
      }
    break;
    case 3:
      if(mode3 != ledStrip3.mode || h3 != ledStrip3.hue || s3 != ledStrip3.saturation || v3 != ledStrip3.brightness) {
        h3 = ledStrip3.hue;
        s3 = ledStrip3.saturation;
        v3 = ledStrip3.brightness;
        mode3 = ledStrip3.mode;
        light_strip(ch);
        if (ledStrip3.off)
          ledStrip3.off = false;
//        Serial.println("ZAPAL 3");
      }
    break;
    case 4:
      if((mode1 != ledStrip1.mode || h1 != ledStrip1.hue || s1 != ledStrip1.saturation || v1 != ledStrip1.brightness)
        && (mode2 != ledStrip2.mode || h2 != ledStrip2.hue || s2 != ledStrip2.saturation || v2 != ledStrip2.brightness)
        && (mode3 != ledStrip3.mode || h3 != ledStrip3.hue || s3 != ledStrip3.saturation || v3 != ledStrip3.brightness)) {
        
        h1 = ledStrip1.hue;
        s1 = ledStrip1.saturation;
        v1 = ledStrip1.brightness;
        mode1 = ledStrip1.mode;

        h2 = ledStrip2.hue;
        s2 = ledStrip2.saturation;
        v2 = ledStrip2.brightness;
        mode2 = ledStrip2.mode;
        
        h3 = ledStrip3.hue;
        s3 = ledStrip3.saturation;
        v3 = ledStrip3.brightness;
        mode3 = ledStrip3.mode;
        
        light_strip(ch);
        if (ledStrip3.off && ledStrip2.off && ledStrip1.off) {
          ledStrip1.off = false;
          ledStrip2.off = false;
          ledStrip3.off = false;
        }
      }
    break;
  }
}

void sm_music_mode(uint8_t ch, uint8_t anim) {

  static uint8_t rms1 = 0;
  static uint8_t rms2 = 0;
  static uint8_t rms3 = 0;
  
  switch (ch) {
    case 1:
      if (rms1 == rms_lvl) 
      {
        return;
      }
      else 
      {
        rms1 = rms_lvl;
        if (ledStrip1.off)
          ledStrip1.off = false;
      }

    break;
    case 2:
      if (rms2 == rms_lvl) 
      {
        return;
      }
      else 
      {
        rms2 = rms_lvl;
        if (ledStrip2.off)
          ledStrip2.off = false;
      }
    break;
    case 3:
      if (rms3 == rms_lvl) 
      {
        return;
      }
      else 
      {
        rms3 = rms_lvl;
        if (ledStrip3.off)
          ledStrip3.off = false;
      }
    break;
  }
  
  switch (anim) {
    case 1: 
      ma_level_strip(ch, false);
    break;
    case 2: 
      ma_level_strip(ch, true);
    break;
    case 3: 
      ma_level_strip_double(ch, false);
    break;
    case 4: 
      ma_level_strip_double(ch, true);
    break;
    case 5: 
      ma_level_strip_col(ch, false);
    break;
    case 6: 
      ma_level_strip_col(ch, true);
    break;
    case 7: 
      ma_strobe(ch, 0);
    break;
    case 8: 
      ma_strobe(ch, 3);
    break;
    case 9: 
      ma_strobe(ch, 5);
    break;
    case 10: 
      ma_strobe(ch, 10);
    break;
    case 11: 
      ma_level_strip_split(ch,3);
    break;
    case 12: 
      ma_level_strip_split(ch,5);
    break;
    case 13: 
      ma_level_strip_split(ch,6);
    break;
    case 14: 
      ma_level_strip_split(ch,7);
    break;
    case 15: 
      ma_level_strip_split(ch,10);
    break;
  }
}

void sm_idle_mode(uint8_t ch, uint8_t anim) {

  switch (ch) {
    case 1:
      if (ledStrip1.off) 
        ledStrip1.off = false;
    break;
    case 2:
      if (ledStrip2.off) 
        ledStrip2.off = false;
    break;
    case 3:
      if (ledStrip3.off) 
        ledStrip3.off = false;
    break;
  }
  
  switch (anim) {
    case 1: 
      mi_rainbow_flow(ch);
    break;
    case 2: 
      mi_rainbow_flow_rev(ch);
    break;
    case 3: 
      mi_rainbow_flow_bounce(ch);
    break;
    case 4: 
 
    break;
    case 5: 

    break;
    case 6: 

    break;
    case 7: 

    break;
    case 8: 

    break;
  }
}

void sm_loader(uint8_t ch) {
  static uint8_t mode;
  static uint8_t music;
  static uint8_t idle;
  
  switch(ch) {
    case 1:
      if (mode != ledStrip1.mode)
        mode = ledStrip1.mode;
      if (music != ledStrip1.music)
        music = ledStrip1.music;
      if (idle != ledStrip1.idle)
        idle = ledStrip1.idle;
    break;
    case 2:
      if (mode != ledStrip2.mode)
        mode = ledStrip2.mode;
      if (music != ledStrip2.music)
        music = ledStrip2.music;
      if (idle != ledStrip2.idle)
        idle = ledStrip2.idle;
    break;
    case 3:
      if (mode != ledStrip3.mode)
        mode = ledStrip3.mode;
      if (music != ledStrip3.music)
        music = ledStrip3.music;
      if (idle != ledStrip3.idle)
        idle = ledStrip3.idle;
    break;
    case 4:
      sm_manual_mode(1);
      sm_manual_mode(2);
      sm_manual_mode(3);

      // sm_manual_mode(4);
    break;
  }

  if (ch != 4) {
    switch(mode) {
      case 0:
        sm_off_mode(ch);
      break;
      case 1:
        sm_manual_mode(ch);
      break;
      case 2: 
        sm_music_mode(ch, music);
      break;
      case 3: 
        sm_idle_mode(ch, idle);
      break;
      default:
      break;
    }
  }
}

// ANIMATIONS
// -----------------------------------------------------------------------------------------

void fadeall(uint8_t ch, uint8_t scale) {
  
  switch (ch) {
    case 1:
      for(int i = 0; i < ledStrip1.leds_nr; i++) { led_strip_1[i].nscale8(scale); } 
    break;
    case 2:
      for(int i = 0; i < ledStrip2.leds_nr; i++) { led_strip_2[i].nscale8(scale); } 
    break;
    case 3:
      for(int i = 0; i < ledStrip3.leds_nr; i++) { led_strip_3[i].nscale8(scale); } 
    break;
  }
}

// MUSIC ANIMATIONS

void ma_level_strip(uint8_t ch, bool reverse) {
    int rms;

    switch (ch) {
    case 1:
      rms = map(rms_lvl, 1, 30, 0, ledStrip1.leds_nr);
      clear_strip(1);

      if (reverse) 
      {
        for(int i = ledStrip1.leds_nr - 1; i > (ledStrip1.leds_nr - 1 - rms); i--) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
      }
    break;
    case 2:;
      rms = map(rms_lvl, 1, 30, 0, ledStrip2.leds_nr);
      clear_strip(2);
      
      if (reverse)
      {
        for(int i = ledStrip2.leds_nr - 1; i > (ledStrip2.leds_nr - 1 - rms); i--) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
      }
    break;
    case 3:
      rms = map(rms_lvl, 1, 30, 0, ledStrip3.leds_nr);
      clear_strip(3);
      if (reverse)
      {
        for(int i = ledStrip3.leds_nr - 1; i > (ledStrip3.leds_nr - 1 - rms); i--) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
      }
    break;
    }
    
    FastLED.show();
}

void ma_level_strip_double(uint8_t ch, bool reverse) {
    int rms;
//    Serial.println("reverse");
    switch (ch) {
    case 1:
      rms = map(rms_lvl, 1, 30, 0, ledStrip1.leds_nr/2);
      clear_strip(1);
      if (reverse) 
      {
        int middle = (ledStrip1.leds_nr - 1)/2;
        for(int i = middle; i < middle + rms; i++) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
        for(int i = middle; i > middle - rms; i--) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
        for(int i = ledStrip1.leds_nr - 1; i > (ledStrip1.leds_nr - 1 - rms); i--) { led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,ledStrip1.brightness); }
      }
    break;
    case 2:
      rms = map(rms_lvl, 1, 30, 0, ledStrip2.leds_nr/2);
      clear_strip(2);
      if (reverse) 
      {
        int middle = (ledStrip2.leds_nr - 1)/2;
        for(int i = middle; i < middle + rms; i++) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
        for(int i = middle; i > middle - rms; i--) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
        for(int i = ledStrip2.leds_nr - 1; i > (ledStrip2.leds_nr - 1 - rms); i--) { led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,ledStrip2.brightness); }
      }
    break;
    case 3:
      rms = map(rms_lvl, 1, 30, 0, ledStrip3.leds_nr/2);
      clear_strip(3);
      if (reverse) 
      {
        int middle = (ledStrip3.leds_nr - 1)/2;
        for(int i = middle; i < middle + rms; i++) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
        for(int i = middle; i > middle - rms; i--) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
      }
      else
      {
        for(int i = 0; i < rms; i++) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
        for(int i = ledStrip3.leds_nr - 1; i > (ledStrip3.leds_nr - 1 - rms); i--) { led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,ledStrip3.brightness); }
      }
    break;
    }
    
    FastLED.show();
}

void ma_level_strip_col(uint8_t ch, bool reverse) {
    static int rms_lvl_saved = 0;
    int rms;
    uint8_t hue;
    
    switch (ch) {
    case 1:
      rms = map(rms_lvl, 1, 30, 0, ledStrip1.leds_nr);
      clear_strip(1);
      hue = ledStrip1.hue;
      if (reverse) 
      {
        for(int i = ledStrip1.leds_nr - 1; i > (ledStrip1.leds_nr - 1 - rms); i--) { 
          led_strip_1[i] = CHSV(hue,ledStrip1.saturation,ledStrip1.brightness);
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
      else
      {
        for(int i = 0; i < rms; i++) {
          led_strip_1[i] = CHSV(hue,ledStrip1.saturation,ledStrip1.brightness); 
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
    break;
    case 2:
      rms = map(rms_lvl, 1, 30, 0, ledStrip2.leds_nr);
      clear_strip(2);
      hue = ledStrip2.hue;
      if (reverse) 
      {
        for(int i = ledStrip2.leds_nr - 1; i > (ledStrip2.leds_nr - 1 - rms); i--) { 
          led_strip_2[i] = CHSV(hue,ledStrip2.saturation,ledStrip2.brightness);
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
      else
      {
        for(int i = 0; i < rms; i++) {
          led_strip_2[i] = CHSV(hue,ledStrip2.saturation,ledStrip2.brightness); 
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
    break;
    case 3:
      rms = map(rms_lvl, 1, 30, 0, ledStrip3.leds_nr);
      clear_strip(3);
      hue = ledStrip3.hue;
      if (reverse) 
      {
        for(int i = ledStrip1.leds_nr - 1; i > (ledStrip1.leds_nr - 1 - rms); i--) { 
          led_strip_3[i] = CHSV(hue,ledStrip3.saturation,ledStrip3.brightness);
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
      else
      {
        for(int i = 0; i < rms; i++) {
          led_strip_3[i] = CHSV(hue,ledStrip3.saturation,ledStrip3.brightness); 
          if (hue < 255)
            hue++;
          else
            hue = 0;
        }
      }
    break;
    }
    
    FastLED.show();
}

void ma_level_strip_split(uint8_t ch, uint8_t maxlv) {

  uint8_t led2lightZero = 0;
  uint8_t lv_current = 0;
  uint8_t led_range_nr;
  uint8_t nr_ranges;
  led_range_nr = (2*maxlv + 1);
  static int rms_lvl_saved = 0;

  // Map lv of leds to light in specific range 
  lv_current = map(rms_lvl, 0, 30, 0, maxlv) + 1;
  
  switch (ch) {
    case 1:
      nr_ranges = (ledStrip1.leds_nr)/led_range_nr;
      clear_strip(1);
      
      for(uint8_t j = 0; j < nr_ranges; j++) {
        led2lightZero = maxlv + (led_range_nr)*j;
    
        for(uint8_t i = 1; i <= lv_current; i++) {
          if(i == 1) {
            led_strip_1[led2lightZero] = CHSV(((ledStrip1.hue - 50) < 0) ? (255 - (ledStrip1.hue - 50)) : ledStrip1.hue - 50,
                                                                                                          ledStrip1.saturation,
                                                                                                          ledStrip1.brightness);
          }
          if(i > 1) {
            led_strip_1[led2lightZero - i + 1] = CHSV(ledStrip1.hue, ledStrip1.saturation, ledStrip1.brightness);
            led_strip_1[led2lightZero + i - 1] = CHSV(ledStrip1.hue, ledStrip1.saturation, ledStrip1.brightness);
          }
        }
      }
    break;
    case 2:
      nr_ranges = (ledStrip2.leds_nr)/led_range_nr;
      clear_strip(2);
      
      for(uint8_t j = 0; j < nr_ranges; j++) {
        led2lightZero = maxlv + (led_range_nr)*j;
    
        for(uint8_t i = 1; i <= lv_current; i++) {
          if(i == 1) {
            led_strip_2[led2lightZero] = CHSV(((ledStrip2.hue - 50) < 0) ? (255 - (ledStrip2.hue - 50)) : ledStrip2.hue - 50,
                                                                                                          ledStrip2.saturation,
                                                                                                          ledStrip2.brightness);
          }
          if(i > 1) {
            led_strip_2[led2lightZero - i + 1] =  CHSV(ledStrip2.hue, ledStrip2.saturation, ledStrip2.brightness);
            led_strip_2[led2lightZero + i - 1] =  CHSV(ledStrip2.hue, ledStrip2.saturation, ledStrip2.brightness);
          }
        }
      }
    break;
    case 3:
      nr_ranges = (ledStrip3.leds_nr)/led_range_nr;
      clear_strip(3);
      
      for(uint8_t j = 0; j < nr_ranges; j++) {
        led2lightZero = maxlv + (led_range_nr)*j;
    
        for(uint8_t i = 1; i <= lv_current; i++) {
          if(i == 1) {
            led_strip_3[led2lightZero] = CHSV(((ledStrip3.hue - 50) < 0) ? (255 - (ledStrip3.hue - 50)) : (ledStrip3.hue - 50),
                                                                                                          ledStrip3.saturation,
                                                                                                          ledStrip3.brightness);
          }
          if(i > 1) {
            led_strip_3[led2lightZero - i + 1] = CHSV(ledStrip3.hue, ledStrip3.saturation, ledStrip3.brightness);
            led_strip_3[led2lightZero + i - 1] = CHSV(ledStrip3.hue, ledStrip3.saturation, ledStrip3.brightness);
          }
        }
      }
    break;
  }
  FastLED.show();
}

void ma_strobe(uint8_t ch, uint8_t len) {
  
  uint8_t lv;
  uint8_t segment_nr = 0;
    
  switch (ch) {
    case 1:
      lv = map(rms_lvl, 0, 30, 0, ledStrip1.brightness);
      if (len == 0) {
        fill_solid(led_strip_1, ledStrip1.leds_nr, CHSV(ledStrip1.hue, ledStrip1.saturation, lv));
      } else {
        clear_strip(1);
        
        for(int i = 0; i < ledStrip1.leds_nr; i++) {
          if (segment_nr%3 == 0) 
            led_strip_1[i] = CHSV(ledStrip1.hue,ledStrip1.saturation,lv); 
          if(i%len == 0 && i != 0)
            segment_nr++;
        }
      }
    break;
    case 2:
      lv = map(rms_lvl, 0, 30, 0, ledStrip2.brightness);
      if (len == 0) {
        fill_solid(led_strip_2, ledStrip2.leds_nr, CHSV(ledStrip2.hue, ledStrip2.saturation, lv));
      } else {
        clear_strip(1);
        
        for(int i = 0; i < ledStrip2.leds_nr; i++) {
          if (segment_nr%3 == 0) 
            led_strip_2[i] = CHSV(ledStrip2.hue,ledStrip2.saturation,lv); 
          if(i%len == 0 && i != 0)
            segment_nr++;
        }
      }
    break;
    case 3:
      lv = map(rms_lvl, 0, 30, 0, ledStrip3.brightness);
      if (len == 0) {
        fill_solid(led_strip_3, ledStrip3.leds_nr, CHSV(ledStrip3.hue, ledStrip3.saturation, lv));
      } else {
        clear_strip(1);
        
        for(int i = 0; i < ledStrip3.leds_nr; i++) {
          if (segment_nr%3 == 0) 
            led_strip_3[i] = CHSV(ledStrip3.hue,ledStrip3.saturation,lv); 
          if(i%len == 0 && i != 0)
            segment_nr++;
        }
      }
    break;
  }
  FastLED.show();
}



// IDLE ANIMATIONS

void mi_rainbow_flow(uint8_t ch) {
  static uint8_t hue1 = 0;
  static uint8_t hue2 = 0;
  static uint8_t hue3 = 0;
  static uint8_t i1 = 0;
  static uint8_t i2 = 0;
  static uint8_t i3 = 0;

  switch (ch) {
    case 1:
      if(i1 < ledStrip1.leds_nr) {
        led_strip_1[i1] = CHSV(hue1++, 255, ledStrip1.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i1 ++;
      } else {
        i1 = 0;
      }
    break;
    case 2:
      if(i2 < ledStrip2.leds_nr) {
        led_strip_2[i2] = CHSV(hue2++, 255, ledStrip2.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i2++;
      } else {
        i2 = 0;
      }
    break;
    case 3:
      if(i3 < ledStrip3.leds_nr) {
        led_strip_3[i3] = CHSV(hue3++, 255, ledStrip3.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i3++;
      } else {
        i3 = 0;
      }
    break;
  }
}

void mi_rainbow_flow_rev(uint8_t ch) {
  static uint8_t hue1 = 0;
  static uint8_t hue2 = 0;
  static uint8_t hue3 = 0;
  static uint8_t i1 = ledStrip1.leds_nr;
  static uint8_t i2 = ledStrip2.leds_nr;
  static uint8_t i3 = ledStrip3.leds_nr;

  switch (ch) {
    case 1:
      if(i1 > 0) {
        led_strip_1[i1] = CHSV(hue1++, 255, ledStrip1.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i1 --;
      } else {
        i1 = ledStrip1.leds_nr;
      }
    break;
    case 2:
      if(i2 > 0) {
        led_strip_2[i2] = CHSV(hue2++, 255, ledStrip2.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i2 --;
      } else {
        i2 = ledStrip2.leds_nr;
      }
    break;
    case 3:
      if(i3 > 0) {
        led_strip_3[i3] = CHSV(hue3++, 255, ledStrip3.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i3 --;
      } else {
        i3 = ledStrip3.leds_nr;
      }
    break;
  }
}

void mi_rainbow_flow_bounce(uint8_t ch) {
  static uint8_t hue1 = 0;
  static uint8_t hue2 = 0;
  static uint8_t hue3 = 0;
  static uint8_t i1 = ledStrip1.leds_nr;
  static uint8_t i2 = ledStrip2.leds_nr;
  static uint8_t i3 = ledStrip3.leds_nr;
  static bool dir1 = true;
  static bool dir2 = true;
  static bool dir3 = true;
  
  switch (ch) {
    case 1:
    if(dir1) {
      if(i1 > 0) {
        led_strip_1[i1] = CHSV(hue1++, 255, ledStrip1.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i1 --;
      } else {
        i1 = ledStrip1.leds_nr;
        dir1 = false;
      }
    } else {
      if(i1 < ledStrip1.leds_nr) {
        led_strip_1[i1] = CHSV(hue1++, 255, ledStrip1.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i1 ++;
      } else {
        i1 = 0;
        dir1 = true;
      }
    }
    break;
    case 2:
      if(i2 > 0) {
        led_strip_2[i2] = CHSV(hue2++, 255, ledStrip2.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i2 --;
      } else {
        i2 = ledStrip2.leds_nr;
      }
    break;
    case 3:
      if(i3 > 0) {
        led_strip_3[i3] = CHSV(hue3++, 255, ledStrip3.brightness);
        FastLED.show(); 
        fadeall(ch, 250);
        i3 --;
      } else {
        i3 = ledStrip3.leds_nr;
      }
    break;
  }
}
