#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

//#define CFG_SLEEP_FIXEDBTN
//#define CFG_NO_SLEEP_SWITCHBTN
#define CFG_SLEEP_SWITCHBTN

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define WDTO_INFINITE 255
#define SLEEP_PERIOD WDTO_8S
#define SKIP_WDT_WAKEUPS 14 // x SLEEP_PERIOD + SLEEP_PERIOD (14 ~= 120 sec)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
#ifdef CFG_SLEEP_FIXEDBTN
#define PIN 1
#define PIN_SOLAR 2
#define FIXED_BUTTON 4
#define PIN_BUTTON       -1
#define INCREASE_EFFECT_EVERY_SWITCH
#define FIXED_MODE 0
#elif defined(CFG_NO_SLEEP_SWITCHBTN)
#define PIN 1
#define PIN_SOLAR -1
#define FIXED_BUTTON -1
#define PIN_BUTTON       4

#elif  defined(CFG_SLEEP_SWITCHBTN)
#define PIN 1
#define PIN_SOLAR 2
#define FIXED_BUTTON -1
#define PIN_BUTTON       4
#endif

#ifndef NUM_PIXzs
#define NUM_PIX 12
#endif

#define EEPROM_ADDR_EFFECT_NUM 0x10


  
#define MODES_COUNT      11
#define SWITCH_MODE_DELAY_MS 5000
#define SOLAR_CHECK_DELAY_MS 1000
#define SWITCH_ON_BARIER 20




Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIX, PIN, NEO_GRB + NEO_KHZ800);

bool isOn=true;
bool isButtonPress=false;
int current_mode=0;
long next_switch_ms=0;
long next_checksolar_ms=SOLAR_CHECK_DELAY_MS;
bool isAutoModesSwitch=false;
#ifdef FIXED_MODE 
bool isFixedMode=false;
#endif
void showStrip() 
{

   strip.show();

 

}
//-----------------------------------------------------------------------------------------
void setAll(byte red, byte green, byte blue) 
{
  for(int i = 0; i < NUM_PIX; i++) 
  {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}
//-----------------------------------------------------------------------------------------
void setPixel(int Pixel, byte red, byte green, byte blue) 
{
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));



}
void setup() 
{

  if(PIN_SOLAR>=0)
    pinMode(PIN_SOLAR,INPUT);
  if(PIN_BUTTON>=0)
    pinMode(PIN_BUTTON,INPUT);
#ifdef FIXED_MODE 
  if(FIXED_BUTTON>=0){
    pinMode(FIXED_BUTTON, INPUT_PULLUP);
    delay(20);
    isFixedMode=digitalRead(FIXED_BUTTON)==LOW;
    current_mode=FIXED_MODE;
  }
#endif
#ifdef FIXED_MODE 
if(!isFixedMode){
#endif
  byte state=eeprom_read_byte((uint8_t*)EEPROM_ADDR_EFFECT_NUM);
  if(state>0 && state<=MODES_COUNT){
   
    current_mode=state-1;
  }
  else{ 
    eeprom_write_byte((uint8_t*)EEPROM_ADDR_EFFECT_NUM, (uint8_t)(current_mode+1));
  }
#ifdef INCREASE_EFFECT_EVERY_SWITCH
   eeprom_write_byte((uint8_t*)EEPROM_ADDR_EFFECT_NUM, (uint8_t)(((current_mode-1)<MODES_COUNT?current_mode+1:0)+1));
#ifdef FIXED_MODE
}
#endif
#endif

  strip.begin();
  strip.setBrightness(255);
  strip.show(); 

 attachInterrupts();
 
}
ISR(PCINT0_vect)
{
  if(PIN_SOLAR>=0)
    isOn= (digitalRead(PIN_SOLAR)==LOW);
 if(PIN_BUTTON>=0 && !isButtonPress)  //give chance to process
   isButtonPress= (digitalRead(PIN_BUTTON)==HIGH);

//isButtonPress=!isOn;
}
ISR(WDT_vect)
{
  
}
void loop() 
{
 
  if(!isOn){
    setAll(0,0,0);
    delay(100);
 //   digitalWrite(PIN,LOW);
    sleep();
    return;
  }


  switch(current_mode){
    case 0:  Fire(40, 120, 200);break;
    case 1:  SnowSparkle(0x10, 0x10, 0x10, 20, random(100, 1000)); break;
    case 2:  RGBLoop(); break;
    case 3:   setAll(255,255,255); showStrip();delay(100); break;
    case 4:  Strobe(0x25, 0x20, 0x25, 10, 50, 1000); break; 
    case 5:  CylonBounce(0xff, 0, 0, 4, 10, 50); break;
    case 6:  Twinkle(0xff, 0, 0, 10, 100, false); break;
    case 7:  TwinkleRandom(20, 100, false); break;
    case 8:  RunningLights(0,0xFF, 0xFF, 50); break;
    case 9: rainbowCycle(20); break;
    case 10: theaterChase(0xff,0,0,50); break;
    default:
    break;
  }
  if(isButtonPress){
   // isAutoModesSwitch=!isAutoModesSwitch;
   current_mode++;
   if (current_mode >=MODES_COUNT) 
    current_mode = 0;
    eeprom_write_byte((uint8_t*)EEPROM_ADDR_EFFECT_NUM, (uint8_t)(current_mode+1));
    isButtonPress=false;
  }
  if(isAutoModesSwitch && next_switch_ms<millis()){
    current_mode++;
    if (current_mode > MODES_COUNT) current_mode = 0; 
    eeprom_write_byte((uint8_t*)EEPROM_ADDR_EFFECT_NUM, (uint8_t)(current_mode+1));
    next_switch_ms=millis()+SWITCH_MODE_DELAY_MS;
  }
 
  
}
void wakeUp(){

}
void attachInterrupts(){

    cli();
  // Disable interrupts during setup

  if(PIN_BUTTON>=0)
    PCMSK |= _BV( PIN_BUTTON);    // Enable interrupt handler (ISR) FOR BUTTON
  if(PIN_SOLAR>=0)
    PCMSK |= _BV(PIN_SOLAR);    // Enable interrupt handler (ISR) FOR SOLAR
  GIMSK |= _BV(PCIE);             // Enable PCINT interrupt in the general interrupt mask
 
  sei(); 
}
void detachInterupts(){
  if(PIN_BUTTON>=0)
    PCMSK &= ~_BV( PIN_BUTTON);    // Disable interrupt handler (ISR) FOR BUTTON
  if(PIN_SOLAR>=0)
    PCMSK &= ~_BV(PIN_SOLAR);    // Disable interrupt handler (ISR) FOR SOLAR
 
}
void sleep()
{
//  GIMSK = _BV(PCIE); // Включить Pin Change прерывания
//  if (SLEEP_PERIOD == WDTO_INFINITE )
//    PCMSK |= _BV(PIN_SOLAR); 
  
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    ACSR |= _BV(ACD); // Disable analog comparator
    /*
  if (SLEEP_PERIOD != WDTO_INFINITE) {
    wdt_enable(SLEEP_PERIOD); // установить таймер
    WDTCR |= _BV(WDIE); //switch on timer interrupts
  }
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //  Power-down
  

  sleep_enable(); // 
 
  sleep_cpu();   
   cli();
    sleep_disable(); 
     ADCSRA &= _BV(ADEN); 
    ACSR |= _BV(ACD);
    attachInterrupts();
   sei();
 

  wakeUp();
 // ADCSRA |= _BV(ADEN); // on ADC
  
}

//-----------------------------------------------------------------------------------------

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) 
{
  setAll(red,green,blue);
 
  int Pixel = random(NUM_PIX);
  setPixel(Pixel,0xff,0xff,0xff);
  showStrip();
  delay(SparkleDelay);
  setPixel(Pixel,red,green,blue);
  showStrip();
  delay(SpeedDelay);
}
void Fire(int Cooling, int Sparking, int SpeedDelay) 
{
  static byte heat[NUM_PIX];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_PIX; i++) 
  {
    cooldown = random(0, ((Cooling * 10) / NUM_PIX) + 2);
   
    if(cooldown>heat[i]) 
    {
      heat[i]=0;
    } 
    else 
    {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_PIX - 1; k >= 2; k--) 
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) 
  {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_PIX; j++) 
  {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}
void RunningLights(byte red, byte green, byte blue, int WaveDelay) 
{
  int Position=0;
 
  for(int i=0; i<NUM_PIX*2; i++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_PIX; i++) 
      {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
     
      showStrip();
      delay(WaveDelay);
  }
}
void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  for(int i = 0; i < NUM_PIX-EyeSize-2; i++) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_PIX-EyeSize-2; i > 0; i--) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
 
  delay(ReturnDelay);
}
void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause)
{
  for(int j = 0; j < StrobeCount; j++) 
  {
    setAll(red,green,blue);
    showStrip();
    delay(FlashDelay);
    setAll(0,0,0);
    showStrip();
    delay(FlashDelay);
  }
 delay(EndPause);
}
void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne) 
{
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) 
  {
     setPixel(random(NUM_PIX),red,green,blue);
     showStrip();
     delay(SpeedDelay);
     if(OnlyOne) 
     {
       setAll(0,0,0);
     }
   }
 
  delay(SpeedDelay);
}
byte * Wheel(byte WheelPos) 
{
  static byte c[3];
  if(WheelPos < 85) 
  {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) 
  {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } 
  else 
  {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}
void rainbowCycle(int SpeedDelay) 
{
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) 
  { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_PIX; i++) 
    {
      c=Wheel(((i * 256 / NUM_PIX) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}
void theaterChase(byte red, byte green, byte blue, int SpeedDelay) 
{
  for (int j=0; j<10; j++) 
  {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) 
    {
      for (int i=0; i < NUM_PIX; i=i+3) 
      {
        setPixel(i+q, red, green, blue);    //turn every third pixel on
      }
      showStrip();
     
      delay(SpeedDelay);
     
      for (int i=0; i < NUM_PIX; i=i+3) 
      {
        setPixel(i+q, 0,0,0);        //turn every third pixel off
      }
    }
  }
}
//-----------------------------------------------------------------------------------------
void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) 
{
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) 
  {
     setPixel(random(NUM_PIX),random(0,255),random(0,255),random(0,255));
     showStrip();
     delay(SpeedDelay);
     if(OnlyOne) 
     {
       setAll(0,0,0);
     }
   }

  delay(SpeedDelay);
}
void RGBLoop()
{
  for(int j = 0; j < 3; j++ ) 
  {
    // Fade IN
    for(int k = 0; k < 256; k++) 
    {
      switch(j) 
      {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3);
    }
    // Fade OUT
    for(int k = 255; k >= 0; k--) 
    {
      switch(j) 
      {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3);
    }
  }
}
//-----------------------------------------------------------------------------------------
void setPixelHeatColor (int Pixel, byte temperature) 
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
 // heatramp <<= 2; // scale up to 0..252
 setPixel(Pixel, 0, 0, 0);
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) 
  {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) 
  {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } 
  else 
  {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}
