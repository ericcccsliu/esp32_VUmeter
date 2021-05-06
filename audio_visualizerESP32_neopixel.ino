#include <arduinoFFT.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy // i don't even have an arduino due // but ok fine 
#endif


#define samples                1024
#define samplingFrequency      40000  //experimentally determined 
#define audioPin               35
#define ledPin                 5

//should be tweaked
#define noise                  500
#define amplitude              1000


//number of fucking bands lmfao can't you even read
#define bandNumber             12
#define height                 12

//fft stuff
unsigned int samplingPerioduS; 
double vReal[samples];
double vImag[samples]; 
int bandValues[bandNumber]; 
byte peak[bandNumber]; 
int oldBarHeights[bandNumber]; 

unsigned long newTime; //use for testing
unsigned long newTime2; //use for testing frequency to match

arduinoFFT FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); 
//end fft stuff

unsigned long decayTimeCounter; 

//neomatrix stuff
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(bandNumber, height, ledPin,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);
//end neomatrix constructor

//color setup
uint16_t rainbow[] = {matrix.Color(255,56,56),
                      matrix.Color(255,145,55),
                      matrix.Color(255,238,53),
                      matrix.Color(181,255,52),
                      matrix.Color(88,255,51),
                      matrix.Color(49,255,104),
                      matrix.Color(48,255,200),
                      matrix.Color(46,217,255),
                      matrix.Color(45,122,255),
                      matrix.Color(62,44,255),
                      matrix.Color(159,42,255),
                      matrix.Color(255,41,255)}; 
uint16_t peakColor = matrix.Color(255,255,255); //me_irl

void setup () {
  Serial.begin (115200);

  //initialize bandvalues and peak and oldbarheights to zero so display isn't wildin out 
  for(int i = 0; i < bandNumber; i++){
    bandValues[i] = 0; 
    oldBarHeights[i] = 0; 
    peak[i] = 0; 
  }

  //start neomatrix
  matrix.begin(); 
  matrix.setBrightness(125); 

  decayTimeCounter = millis(); //set initial value for decay time 
  samplingPerioduS = round(1000000 * (1.0/samplingFrequency));
  
}  // end of setup


void loop (){
  for(int i = 0; i < bandNumber; i++){
    bandValues[i] = 0; 
  }
  
  //sampling
  //newTime2 = micros(); 
  for (int i = 0; i < samples; i++){
    newTime = micros(); 
    vReal[i] = analogRead(audioPin);
    vImag[i] = 0; 
    while (((unsigned long)(micros()-newTime)) < samplingPerioduS){ /* chill */} //not necessary to achieve this sampling rate? idfk
  }
 /*
  float conversionTime = ((float)(micros()-newTime2))/samples; 

  
  Serial.print("conversion time: "); 
  Serial.print(conversionTime);
  Serial.println(" uS");

  Serial.print("max sampling frequency: "); 
  Serial.print((1.0/conversionTime)*1000000); 
  Serial.println(" Hz");
   */

  //compute fft 
  FFT.DCRemoval(); 
  FFT.Windowing(FFT_WIN_TYP_HAMMING,FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude(); 

  //analyze
  for (int i = 1; i < (samples/2); i++){
    if (vReal[i] > noise){
      if (i> 1 && i <= 3 )  bandValues[0]  += ((int)vReal[i]);
      if (i>3   && i<=4  ) bandValues[1]  += ((int)vReal[i]);
      if (i>4   && i<=7  ) bandValues[2]  += ((int)vReal[i]);
      if (i>7   && i<=11  ) bandValues[3]  += (int)vReal[i];
      if (i>11   && i<=18  ) bandValues[4]  += (int)vReal[i];
      if (i>18   && i<=29 ) bandValues[5]  += (int)vReal[i];
      if (i>29  && i<=46 ) bandValues[6]  += (int)vReal[i];
      if (i>46  && i<=75 ) bandValues[7]  += (int)vReal[i];
      if (i>75  && i<=120 ) bandValues[8]  += (int)vReal[i];
      if (i>120  && i<=193 ) bandValues[9]  += (int)vReal[i];
      if (i>193  && i<=311 ) bandValues[10] += (int)vReal[i];
      if (i>311  && i<=501 ) bandValues[11] += (int)vReal[i];
    }
  }
  matrix.clear(); 
  //processing into bar heights and draw
  for(int band = 0; band < bandNumber; band++){
    int barHeight = bandValues[band]/amplitude; //scale
    
    if (barHeight > height){
      barHeight = height + 1; 
    } // self explanatory 

    barHeight = (oldBarHeights[band]+barHeight)/2; //average out

    if (barHeight >= peak[band]){
      peak[band] = min(height, barHeight); 
    }
     
    //draw bands
    for(int y = height; y > height - barHeight - 1; y--){
      matrix.drawPixel(band, y, rainbow[band]);          
    }
    
    //draw peaks
   
    //matrix.drawPixel(band, height-peak[band]-1, peakColor); 
    

      
    //save barheight into oldbarheights for averaging later
    oldBarHeights[band] = barHeight;

     
  }
  matrix.show(); 
  //decay peaks

  if(millis()-decayTimeCounter >= 60){
    for(int band = 0; band < bandNumber; band++){
      if(peak[band] >= 0){
        peak[band] -= 1; 
      }
    }
    decayTimeCounter = millis(); 
  }
  
  
}  // end of loop
