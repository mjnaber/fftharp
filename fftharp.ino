/*
fftharp
 
 Heavily uses guest openmusiclabs.com 8.18.12
 example sketch.
 
 Uses a pre-amplifier and an electret microphone on ADC0 to capture
 incoming sound data. Then performs FFT and determines largest peak
 and drives one of the 8 lasers connected to pins 6-13. Checks that
 there is more than just a background level of noise in order to make 
 sure that all lasers turn off when it's quiet. Onlt lights one laser
 at a time, could do more, but probably less than 4 at a time as they 
 draw about 15mA each, so getting close to the limit for the 328.
 
 */

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

int baseline[8];
int flag = 0;
int t;
int currlaser = 6;

int first = 0;


void align_mode() // run through lasers continuously to allow laser alignment on harp
{
 while (1)
      {
        if (currlaser > 13) currlaser = 6;
        digitalWrite(currlaser++, HIGH);   // actually turn the laser on
        delay(20);
        for (int pos = 6; pos <= 13; pos++) // turn all the lasers off
        {
          digitalWrite(pos, LOW);
        }

      } 
}

void setup() {
  Serial.begin(9600); // use the serial port
  pinMode(6, OUTPUT); // set up digital outputs for laser modules
  pinMode(7, OUTPUT);     
  pinMode(8, OUTPUT);     
  pinMode(9, OUTPUT);     
  pinMode(10, OUTPUT);     
  pinMode(11, OUTPUT);     
  pinMode(12, OUTPUT);     
  pinMode(13, OUTPUT);     

  sei();
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  for (int pos = 6; pos <= 13; pos++) // make sure all lasers are turned off
  {
    digitalWrite(pos, LOW);
  }
}

void loop() {
  int max = 0;
  int bin = 0;
  int last = 0;

  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }


    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();

    max = 0;

    for (int i = 2; i< 128; i++)// would be nice to find the top two peaks and light two lasers
    {                           // or scale to make lower frequencies need a higher threshold to
      t = fft_log_out[i];//-baseline[i]; // turn on as the microphone seems to respond better at
      if (t > max)              // lower frequencies
      {
        max = t;
        bin = i;
      }
    }

    if(last != bin) // only print out when bin changes
    {
      Serial.println(bin);
      last = bin;
    }

    for (int pos = 6; pos <= 13; pos++) // turn all the lasers off
    {
      digitalWrite(pos, LOW);
    }

    if (first == 0) // if on the first time through check if there is a loud noise and enter align mode (all lasers on)
    {
      first = 1;
      if (max > 45)
        align_mode();
    }   

    if (max > 43) // only light a laser if the sound is above a (very low) threshold
    {
      switch (bin)
      {
      case 3:
        currlaser = 6;
        break;
      case 4:
        currlaser = 6;
        break;
      case 5:
        currlaser = 7;
        break;
      case 6:
        currlaser = 7;
        break;
      case 7:
        currlaser = 8;
        break;
      case 8:
        currlaser = 8;
        break;
      case 9:
        currlaser = 9;
        break;
      case 10:
        currlaser = 10;
        break;
      case 11:
        currlaser = 11;
        break;
      case 12:
        currlaser = 12;
        break;
      case 13:
        currlaser = 13;
      }

      digitalWrite(currlaser, HIGH);   // actually turn the laser on
    }

  }
}

