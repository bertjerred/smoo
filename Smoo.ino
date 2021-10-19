#include <Adafruit_MPR121.h>
#include <Tiny4kOLED.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

AudioSynthWaveform       sineModulator;      //xy=106,146
AudioSynthWaveform       filterFreqControl;      //xy=109,346
AudioSynthWaveformSineModulated fmSine;       //xy=268,211
AudioEffectFreeverbStereo freeverbs1;     //xy=381,434
AudioFilterStateVariable filter;        //xy=453.33331298828125,270.6666717529297
AudioAmplifier           amp1;           //xy=593,431
AudioMixer4              firstMixer;         //xy=636.3333740234375,194.111083984375
AudioEffectEnvelope      envelope1;      //xy=673,346
AudioOutputI2S           i2s1;           //xy=745.2222900390625,448
AudioConnection          patchCord1(sineModulator, fmSine);
AudioConnection          patchCord2(filterFreqControl, 0, filter, 1);
AudioConnection          patchCord3(fmSine, 0, filter, 0);
AudioConnection          patchCord4(fmSine, freeverbs1);
AudioConnection          patchCord5(freeverbs1, 0, firstMixer, 3);
AudioConnection          patchCord6(filter, 0, firstMixer, 0);
AudioConnection          patchCord7(filter, 1, firstMixer, 1);
AudioConnection          patchCord8(filter, 2, firstMixer, 2);
AudioConnection          patchCord9(amp1, 0, i2s1, 0);
AudioConnection          patchCord10(firstMixer, envelope1);
AudioConnection          patchCord11(envelope1, amp1);

int freq;
int freq01 = 262; //C4  261.63
int freq02 = 277; //C#4 277.18
int freq03 = 294; //D4  293.66
int freq04 = 311; //D#4 311.13
int freq05 = 330; //E4  329.63
int freq06 = 349; //F4  349.23
int freq07 = 370; //F#4 369.99
int freq08 = 392; //G4  392.00
int freq09 = 415; //G#4 415.30
int freq10 = 440; //A4  440.00
int freq11 = 466; //A#4 466.16
int freq12 = 494; //B4  493.88

int lpPot = A13;
float lp;
int bpPot = A14;
float bp;
int hpPot = A3;
float hp;
int ffcPot = A0;
float ffc;
int mfPot = A12;
float modFactor;

int attackPot = A1;
float attackVal;
//int sustainPot = A17;
float sustainVal;
//int verbPot = A2;
//float verb;
int masterPot = A16;
float master;

void setup() {
  AudioMemory(1000);
  cap.begin(0x5A);
  oled.begin();
  oled.setFont(FONT6X8);
  oled.clear();
  oled.on();
}

void loop() {
  oled.setCursor(0, 0);
  oled.print(modFactor);

  lp = analogRead(lpPot);
  lp = map(lp, 0.0, 1023.0, 0.0, 0.33);
  bp = analogRead(bpPot);
  bp = map(bp, 0.0, 1023.0, 0.0, 0.33);
  hp = analogRead(hpPot);
  hp = map(hp, 0.0, 1023.0, 0.0, 0.33);
  firstMixer.gain(0, lp);
  firstMixer.gain(1, bp);
  firstMixer.gain(2, hp);
  firstMixer.gain(3, 0.01);
  ffc = analogRead(ffcPot);
  ffc = map(ffc, 0.0, 1023.0, 620.0, 0.0); // what frequencies make sense here?

  modFactor = analogRead(mfPot);
  modFactor = map(modFactor, 0.0, 1023.0, 0.0, (freq / PI));

  attackVal = analogRead(attackPot);
  attackVal = map(attackVal, 0.0, 1023.0, 11.0, 10000.0);
//  sustainVal = analogRead(sustainPot);
//  sustainVal = map(sustainVal, 0.0, 1023.0, 0.0, 1.0);
  envelope1.attack(attackVal);
  envelope1.sustain(1.0);
  envelope1.release(11880);

  //verb = analogRead(verbPot);
  //verb = map(verb, 0.0, 1023.0, 0.000, 1.000);
  freeverbs1.roomsize(0.5);
  freeverbs1.damping(0.8);

  master = analogRead(masterPot);
  master = map(master, 0.0, 1023.0, 0.0, 5.0);
  amp1.gain(master);

  currtouched = cap.touched();
  for (uint8_t i = 0; i < 12; i++) {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      envelope1.noteOn();
      if (i == 0)       {
        freq = freq01;
      }
      else if (i == 1)  {
        freq = freq02;
      }
      else if (i == 2)  {
        freq = freq03;
      }
      else if (i == 3)  {
        freq = freq04;
      }
      else if (i == 4)  {
        freq = freq05;
      }
      else if (i == 5)  {
        freq = freq06;
      }
      else if (i == 6)  {
        freq = freq07;
      }
      else if (i == 7)  {
        freq = freq08;
      }
      else if (i == 8)  {
        freq = freq09;
      }
      else if (i == 9)  {
        freq = freq10;
      }
      else if (i == 10) {
        freq = freq11;
      }
      else if (i == 11) {
        freq = freq12;
      }
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      freq = 0;
      envelope1.noteOff();
    }
  }
  lasttouched = currtouched;
  delay(100);
  sineModulator.begin(1.0, modFactor, WAVEFORM_SAWTOOTH);
  filterFreqControl.begin(1.0, ffc, WAVEFORM_TRIANGLE);
  fmSine.frequency(freq);
  AudioInterrupts();

}
