/*Teensy HiHat 808 audio model
 * A no frills teensy 3.2 HiHat eurorack module. Using Teensy audio DSP
 * 
 * Based on 808 model of
 * six square waves with three filters and an envelope.
 * white noise added to fill
 * three filters for hat frequency narrowing, plus one for colour
 * use only one envelope for both open and closed, so choking is automatic
 * 
 * two trigger inputs, open and closed.
 * closed chokes open
 * three poteniometers
 * - length of closed
 * - length of open
 * - resonance or filter effect
 * onboard DAC gives a single mixed audio output on pin14
 * two trigger indicators leds
 * 
 * 
 * six square waves to make an 808 hihat: http://joesul.li/van/synthesizing-hi-hats/
Fundamental:40
Osc1 ratio:2
Osc2 ratio:3 
Osc3 ratio:4.16 
Osc4 ratio:5.43
Osc5 ratio:6.79
Osc6 ratio:8.21

https://www.muffwiggler.com/forum/viewtopic.php?t=120280

example of lovely sounding eurorack hats with sweep
 https://www.youtube.com/watch?time_continue=344&v=8wF7hxk022k&feature=emb_logo
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioSynthWaveform       waveform1;      //sq wave1
AudioSynthWaveform       waveform2;      //sq wave2
AudioSynthWaveform       waveform3;      //sq wave3
AudioSynthWaveform       waveform4;      //sq wave4
AudioSynthWaveform       waveform5;      //sq wave5
AudioSynthWaveform       waveform6;      //sq wave6
AudioSynthNoiseWhite     noise1;         //white noise

AudioMixer4              mixer1;         //four squares in
AudioMixer4              mixer2;         //two squares and white noise in 
AudioMixer4              mixer3;         //two mixes in
AudioFilterBiquad        biquad1;        //Low pass filter //not sure if needed
AudioFilterBiquad        biquad2;        //first high pass filter 
AudioFilterBiquad        biquad3;        //second high pass
AudioEffectEnvelope      envelope1;      //shape the hihat sound
AudioFilterStateVariable HPF_reso;       //set up a 'resonant' hipass filter to colour output
AudioAmplifier           amp1;
AudioOutputAnalog        dac1;           //final ouput

AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform2, 0, mixer1, 1);
AudioConnection          patchCord3(waveform3, 0, mixer1, 2);
AudioConnection          patchCord4(waveform4, 0, mixer1, 3);
AudioConnection          patchCord5(waveform5, 0, mixer2, 0);
AudioConnection          patchCord6(waveform6, 0, mixer2, 1);
AudioConnection          patchCord7(noise1,    0, mixer2, 2);

AudioConnection          patchCord8(mixer1, 0, mixer3, 0);
AudioConnection          patchCord9(mixer2, 0, mixer3, 1);

AudioConnection          patchCord10(mixer3, biquad1);//lowpass filter
AudioConnection          patchCord11(biquad1, envelope1); //signal out
AudioConnection          patchCord13(envelope1, biquad2);
AudioConnection          patchCord14( biquad2, biquad3);
AudioConnection          patchCord15(biquad3, 0, HPF_reso, 0); //resonating filter
AudioConnection          PatchCord16(HPF_reso,2, amp1,0);
AudioConnection          patchCord12(amp1,0, dac1,0); //signal out
//AudioConnection          patchCord12(HPF_reso,2, dac1,0); //signal out


int HPF = 10000; //give the HPF a starting point

//---hat freqs ratios
int Fundamental=40;
int Osc1= 2*Fundamental; //(2 = ratio)
int Osc2 =3*Fundamental; 
int Osc3 =4.16*Fundamental; 
int Osc4 =5.43*Fundamental;
int Osc5 =6.79*Fundamental;
int Osc6 =8.21*Fundamental;

//-----------------------inputs outputs
int closedLeng = A9;//rotary to control lenth of close hihat
int openLeng = A8;//rotary to control length of open hat
int reson = A7;//resonance / filter sweep
int trigOpenHat=A5;
int trigClosedHat=A4;
int LedOpen = 17; //closed trig indicator pin for led
int LedClosed =16;

//----for receiving triggers--------
unsigned long currentClTime = 0;
unsigned long oldClTime = 0;
unsigned long clockGap = 50; //gap for counting only one trigger per pin high
unsigned long currentClTime2 = 0;
unsigned long oldClTime2 = 0;
unsigned long clockGap2 = 50; //change to 50 after debug

int closedRead=0;
int closedHold =0;
int openRead=0;
int openHold =0;

float sweepRead=0;
float sweepVal;

//-------indication leds
unsigned long newLedTime = millis();
unsigned long ledTime = 0;
unsigned long oldLedTime = 0;
//-------indication leds
unsigned long newLedTime2 = millis();
unsigned long ledTime2= 0;
unsigned long oldLedTime2 = 0;

//---------------------------setup---------------//
void setup() {
//------provide CPU space
  AudioMemory(50);
//dac1.analogReference(EXTERNAL);// may need to remove this re noise...
  
// ------ configure the six waveforms ----
  waveform1.frequency(Osc1);
  waveform2.frequency(Osc2);
  waveform3.frequency(Osc3);
  waveform4.frequency(Osc4);
  waveform5.frequency(Osc5);
  waveform6.frequency(Osc6);
  
  waveform1.amplitude(1);
  waveform2.amplitude(1);
  waveform3.amplitude(1);
  waveform4.amplitude(1);
  waveform5.amplitude(1);
  waveform6.amplitude(1);
//---config the noise-----
  noise1.amplitude(1.0);//zero to 1
//--------config the filters--------
HPF_reso.frequency(HPF);
HPF_reso.resonance(2.0);
//Set the filter's resonance. Q ranges from 0.7 to 5.0. 
//Resonance greater than 0.707 will amplify the signal near the corner frequency. 

biquad1.setLowpass(0, 12000, 0.707);
biquad2.setHighpass(0, 7000, 0.707);
biquad3.setHighpass(0, 7000, 0.707);

//------------config inital evelope settings-----
envelope1.delay(0);//0
envelope1.attack(20);//20
envelope1.hold(1);//1
envelope1.decay(10);//10
envelope1.sustain(0.0);//0.0
envelope1.release(0.0);//0.2

//----------------config the mixers
  mixer1.gain(0, 1.0);
  mixer1.gain(1, 1.0);
  mixer2.gain(0, 1.0); 
  mixer2.gain(1, 1.0);
  mixer3.gain(0, 1.0); 
  mixer3.gain(1, 1.0);

  amp1.gain(1.0);

//---------------set the six waveforms all to Square
    waveform1.begin(WAVEFORM_SQUARE);
    waveform2.begin(WAVEFORM_SQUARE);
    waveform3.begin(WAVEFORM_SQUARE);
    waveform4.begin(WAVEFORM_SQUARE);
    waveform2.begin(WAVEFORM_SQUARE);
    waveform3.begin(WAVEFORM_SQUARE);
    waveform4.begin(WAVEFORM_SQUARE);
    waveform5.begin(WAVEFORM_SQUARE);
    waveform6.begin(WAVEFORM_SQUARE);

//-------dont forget to set the pinmodes
 pinMode(closedLeng, INPUT); //rotary forclock division selection
 pinMode(openLeng, INPUT); 
 pinMode(reson, INPUT);
 pinMode(trigOpenHat, INPUT_PULLDOWN);
 pinMode(trigClosedHat, INPUT_PULLDOWN);
 pinMode(LedOpen, OUTPUT);
 pinMode(LedClosed, OUTPUT);
 
}//----------end of setup----------------//

  
void loop() {
  //-------switch of indicator Leds if on for long enough
  newLedTime = millis();
  if (newLedTime > (ledTime + 20)) {
    digitalWrite(17, LOW);
  }
  newLedTime2 = millis();
  if (newLedTime2 > (ledTime2 + 20)) {
    digitalWrite(16, LOW);
  }

  
  sweep();//continuously read sweep value
  
//-------check for triggers
 if (analogRead(trigClosedHat) > 250) { //250mV reading means trigger voltage
     closedHat();  
 }
 if (analogRead(trigOpenHat) > 250) {
     openHat();  
 } 

}//--------------end of loop-------------//


void closedHat() {
  //if a closedHat input is registered, is it the same one?,
  currentClTime = millis();
 if (currentClTime > (oldClTime + clockGap)){
  
  oldClTime = currentClTime;
  digitalWrite(16, HIGH);
  ledTime2 = millis();
  closedRead = analogRead(closedLeng) + 1; // value is 0-1023
  closedHold = map(closedRead, 0, 1023, 0, 100); // map to range of 1-250
  envelope1.decay(closedHold);
    AudioNoInterrupts();
    envelope1.noteOn();
    AudioInterrupts();
  }
  else{ 
  }
}

void openHat() {
  currentClTime2 = millis();
 if (currentClTime2 > (oldClTime2 + clockGap2)){
  
  oldClTime2 = currentClTime2;
  digitalWrite(17, HIGH);
  ledTime = millis();
  openRead = analogRead(openLeng) + 1; // value is 0-1023
  openHold = map(openRead, 0, 1023, 90, 2000);
  envelope1.decay(openHold);
    AudioNoInterrupts();
    envelope1.noteOn();
    AudioInterrupts();
 }
 else{
 }
}

void sweep() {
  sweepRead = analogRead(reson) + 1; // value is 0-1023
  sweepVal = map(sweepRead, 0, 1023, 1500, 12000);//top and bottom end of filter >3000 to 10000
  HPF_reso.frequency(sweepVal);
}
