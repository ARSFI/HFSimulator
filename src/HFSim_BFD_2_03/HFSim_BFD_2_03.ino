 /*  IONOS SIM HF/VHF Channel Simulator  Rick Muething, KN6KB  rmuething@cfl.rr.com
       Copyright (C) 2020  Amateur Radio Safety Foundation Inc. (ARSFI)
       Licensed under the MIT Open Source Initiative.
       
       Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
       to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
       and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
       The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
       THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
       FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
       WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Change  Log:
    Date:     Rev:      Description:
    11/17/2020   2.03   Modify multipath delay settings for 4 path mode as suggested by Peter Helfert.

    11/7/2020    2.02   Make mods to 4 path implementation and re calibration fltSTD values for correct long term average level.
    
    11/1/2020    2.01   Begin of experimental version to support 2 or 4 multipath Using 1.01 as a starting point
    11/1/2020           basic connection and duplication of Gauss filter in and compiles...not tested yet
                        Selection and save/recall from EEPROM for Multipat =2 or = 4 in and tested.
                        Modify Path processing to use two 4 input muxes for a total of 8 paths if Multipaths = 4
                        Do complete evaluation of modes and gains to allow recalibration
   
                        
    
    10/17/202    1.01   Preliminary ready for production release. No technical changes from Rev .9.82
                        Remove revision notes prior to .9.82 (Still available in old backups)
                        Edit line # used in debug printing. Add TFT Mode display update after serial command. 
                        Known issues: 1) Still ~4 dB calibration offset. 2) Busy Detector not rechecked/verified  
    
    10/4/2020    .9.82  Added rms Measure rmsDelayI0 to Delay I1 Tap to facilitate rms average computation
                        Insured nominal Input 1 of MixIQ set to .8498 on all WGN modes to insure no out of limits 
                        Re adjust calibration factor to 4.28 dB to match response of version .9.81
                        Used a separate p-p input measure (faster response than sliding Window) to display internal Lvl (mvp-p) 
// Unused area for future comments (remove as comments added) 

























     This is line #50
    Summary of Simulation Modes and ranges:

    HF/VHF  Channel Modes
    Channel     Spread     Delay                      Range                         Notes:
    Type(mode)  Hz 2Sigma    ms
    WGN (0)        0          0                                                   Also used with Flat fading or FM Deviation on VHF/UHF modeling
    MPG (1)        .1         .5                                                  CCIR Multipath Good (CCIR/ITU)
    MPM (2)        .5         1                                                   CCIR Multipath Moderate (CCIR/ITU)
    MPP (3)        1          2                                                   CCIR Multipath Poor (CCIR/ITU)
    MPD (4)        2          4                                                   Multipath Disturbed
    FADE DEPTH(6) 0                            0 to 40dB, 1dB steps               Flat Fading (depth) Can be used with all ch types
    FADE FREQ(7)                               0,.1,.2,.5,1,2,5,10,20 Hz          Flat Fading (rate) Can be used with all ch types
    OFFSET(8)                                  -200 to + 200 Hz steps 10Hz        Can be used with all ch types
    FM Deviation(9)                            0,1,2,5,10,20,50,100,200 Hz        Can be used with all ch types
    FM Rate(10)                                 0,.1,.2,.5,1,2,5,10,20 Hz         Can be used with all ch types
    CH1 In GAIN (11)                            0,1,2,5,10,20,50,100,200
    CH2 In GAIN (12)                            0,1,2,5,10,20,50,100,200    
    CH1 Out GAIN (13)                           0,.01,.02,.05,.1..2,.5,1,2
    CH2 Out GAIN (14)                           0,.01,.02,.05,.1..2,.5,1,2
    BANDWIDTH(15)                               3000,6000 Hz
    SERIAL iinterface rate (16)                 4800,9600,14400,19200,38400,57600,115200 baud
    TEST3K                                      Test/Calibration 3000Hz BW        Test Freq 300 - 3300 Hz, 100 Hz steps
    TEST6K                                      Test/Calibration 6000Hz BW        Test Freq 300 = 6300 Hz, 200 Hz steps               
    
    OFFSET and FM modeling can be used with any channel type and adds a fixed offset or VLF sine wave (FM Deviation) to the Down mixer LO
*/

//Libraries
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Bounce2.h>
#include <Encoder2.h>
//#include "SPI.h"
#include "ILI9341_t3.h"
#include <EEPROM.h>

Encoder2 ENC1(1, 0); // (right) modes
Encoder2 ENC2(4, 3); // (left) parameters

// For the Adafruit shield  ILI9341, these are the default. (Pin 6 below was Pin 9. RM 3/2/2020)
#define TFT_DC  6
#define TFT_CS 10
#define ENC1Button 2//Right (parameter) 
#define ENC2Button 5//Left  (mode)

//Code for Serial Numbers (These two 32 bit numbers are used to generate MAC address (not used) and 4 digit Serial Number. 
#define OCOTP_CFG0 (*(uint32_t *)0x401F4410)// lower 13 bits define the "Serial Num" ( & with 8191)
#define OCOTP_CFG1 (*(uint32_t *)0x401F4420)// currently not used.

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

// Teensy Audio components (currently 35 components used) 
AudioControlSGTL5000            sgtl5000_1;
AudioInputI2S                   i2s0;
AudioOutputI2S                  i2s1;

AudioFilterFIR                  filLPIn_0_FIR;  // Input LP Filter
AudioFilterFIR                  filLPWhiteFIR; // Low pass filter for White noise
AudioFilterFIR                  filHP7500FIR; // Highpass for up mixing 7500 Hz Injection Freq
AudioFilterFIR                  filLPDownMix; // LowPass filter for Down Mixing 7500 nominal (no shift) injection freq.
AudioFilterFIR                  filHilQ_1234; // Hilbert FIR filter for Q1, Q2, Q3, Q4 channels

AudioSynthWaveformSine          InputTestWaveform; // Used to provide single tone test signal to make measurements at various points.
AudioSynthWaveformSine          sine_Upmix;  // 7.7 KHz Upmix for Low Freq Offset/drift  testing
AudioSynthWaveformSineModulated sine_Dnmix;  // ~7.7 KHz Down mix injection for Low Freq Offset/drift testing
AudioSynthNoiseWhite            whiteOut;   // White noise to add to Channels for S:N
AudioSynthWaveformSine          sine_VLF_Dnmix_Mod;// Slow VLF modulation feeds sine_Dnmix modulation input.

AudioEffectDelay                delayI; // Delay for audio I paths 1, 2,(multi tap)
AudioEffectDelay                delayQ; // Delay for audio Q paths 1, 2,(multi tap)
AudioEffectMultiply             multUpmix;// Multiplier for Upmixing 7500 Hz
AudioEffectMultiply             multDnmix;// Multiplier for Downmixing ~ 7500 Hz
AudioEffectMultiply             multI2;// Multiplier for true rms measure of I input
AudioEffectMultiply             multITap7; //Multiplier for squaring I delay tap # 7 

AudioMixer4                     mixIQ12;// Mixer to combine  Delays for paths 1 and 2
AudioMixer4                     mixIQ34;// Mixer to combine  Delays for paths 3 and 4
AudioMixer4                     mixIQ1234;// Mixer to combine mixIQ12 and mixIQ34
AudioMixer4                     mixChannels;// Mixer to combine ch 1,2,3,4  and White Noise
AudioMixer4                     mixInpSel;// Mixer to select InputTestWaveform, or external inputs 1(L) or 2 (R) 
AudioMixer4                     mixPathSel;// Mixer to select direct path or Up/Down mixer path
AudioAmplifier                  ampLeftOut;// Amp for gain control Left Output
AudioAmplifier                  ampRightOut;//Amp for gain control Right Output

AudioAnalyzePeak                ppLPInput; //p-p value of the filLPIn_0_FIR output
AudioAnalyzePeak                ppi2s0In;//  Probe to measure input i2s0 p-p value (Used in auto test and Auto level)
AudioAnalyzePeak                ppi2s1In;//  Probe to measure input i2s1 p-p value (Used in auto test and Auto level)
AudioAnalyzePeak                ppAmpRightOut;// Probe to measure AmpRightOut p-p value (Used in auto test)
AudioAnalyzePeak                ppAmpLeftOut;// Probe to measure AmpRightOut p-p value (Used in auto test)
AudioAnalyzeRMS                 rmsLPInput;//  rms probe to compute rms value of filLPIn_0_FIR output
AudioAnalyzeFFT1024             fft1024; //  FFT analysis and Spectrum plotting
AudioAnalyzeRMS                 rmsMixIQ1234Out; // RMS measure of mixIQOut34 for Average computation
AudioAnalyzeRMS                 rmsNoise; // Probe to compute rms of BW filtered WGN 
AudioAnalyzeRMS                 rmsDelayI0;  //Probe to compute rmsValue of output of Delay I0 tap.

// Audio Connections  (for a total of 2 audio paths through the simulator each path containing I and Q paths)
AudioConnection           pcInp_1(InputTestWaveform, 0, mixInpSel, 0);//Test sine wave to input 0 of input mixer
AudioConnection           pcInp_2(i2s0, 0, mixInpSel, 2);// Input pin 0 of  i2s0 to input 2 of input mixer (Left)
AudioConnection           pcInp_3 (i2s0, 1, mixInpSel, 3);// Input pin 1 of  i2s0 to input 3 of input mixer (Right)
AudioConnection           pcInp_4(i2s0, 0, ppi2s0In, 0); // p-p measurement of input i2s0 pin 0
AudioConnection           pcInp_5(i2s0, 1, ppi2s1In, 0); // p-p measurement of input i2s0 pin 1

// Audio Connections mixInpSel through to inputs (normal and offset/modulated) t0 mixAudioSelect inputs 0(normal) and 1(offset/modulated)
AudioConnection           pc0_1(mixInpSel, filLPIn_0_FIR); //Input mixer to filLPIn_FIR;  
AudioConnection           pc0_2(filLPIn_0_FIR, 0, multUpmix, 0);// filLPIn to Upmixer
AudioConnection           pc0_3(filLPIn_0_FIR, 0, mixPathSel, 0);//filLPIn to mixPathSel
AudioConnection           pc0_4(sine_Upmix, 0, multUpmix, 1);// Upmix L.O. injection 7.7 KHz
AudioConnection           pc0_5(multUpmix, filHP7500FIR);// Upmix to HPFilter.
AudioConnection           pc0_6(sine_VLF_Dnmix_Mod, 0, sine_Dnmix, 0); // Modulation input to sineDnmix
AudioConnection           pc0_7(sine_Dnmix, 0, multDnmix, 1);// Dnmix L.). injection ~ 7.7 KHz
AudioConnection           pc0_8(filHP7500FIR, 0, multDnmix, 0);// HP7500FIR to DownMix mult
AudioConnection           pc0_9 (multDnmix, filLPDownMix);// Down Mix to filLPDnMix
AudioConnection           pc0_10 (filLPDownMix,0, mixPathSel,1);//filLPDownMix to mixPathSel

// filLPDownMix output through summed I & Q  Outputs of Channels 1 & 2 (4 rays total)
AudioConnection           pc1_1(mixPathSel, delayI);// mixPathSel   to delay I (no boxcar filter ) 
AudioConnection           pc1_2(mixPathSel, filHilQ_1234);//filLPDownMix to filHilQ_1234
AudioConnection           pc1_3(filHilQ_1234,delayQ);// filHilQ_1234 to delay Q
AudioConnection           pc1_4(delayI, 0, mixIQ12, 1);// delayI tap #0 to mixIQ12 input 1
AudioConnection           pc1_5(delayI, 1, mixIQ12, 3);// delay1 tap #1 to mixIQ12 input 3
AudioConnection           pc1_6(delayQ, 0, mixIQ12, 0);// delayQ tap #0 to mixIQ12 input 0
AudioConnection           pc1_7(delayQ, 1, mixIQ12, 2);// delayQ tap #1 to mixIQ12 input 2
AudioConnection           pc1_8 (mixIQ12, 0, mixIQ1234,0); 

// filLPDownMix output through summed I & Q  Outputs of Channels 3 & 4 (4 rays total)
AudioConnection           pc3_1(delayI, 2, mixIQ34, 1);// delayI tap #2 to mixIQ input 1
AudioConnection           pc3_2(delayI, 3, mixIQ34, 3);// delayI tap #3 to mixIQ34 input 3
AudioConnection           pc3_3(delayQ, 2, mixIQ34, 0);// delayQ tap #2 to mixIQ34 input 0
AudioConnection           pc3_4(delayQ, 3, mixIQ34, 2);// delayQ tap #3 to mixIQ34 input 2
AudioConnection           pc3_5 (delayI,0, rmsDelayI0, 0); // Multiplier to compute I^2 for rms calculation
AudioConnection           pc3_6 (mixIQ34, 0, mixIQ1234, 1); // MixIQ34 output to mixIQ1234 Input 1 
AudioConnection           pc3_7 (mixIQ1234, rmsMixIQ1234Out); //rms measurement of mixIQ1234

// Channel outputs and Noise to mixerChannels and Amplifiers Left and Right outputs
AudioConnection           pc2_1(mixIQ1234, 0, mixChannels, 1); //mixIQ1234 output to mixChannels Input 1
AudioConnection           pc2_2(whiteOut, filLPWhiteFIR); //whiteOut to to filLPWhiteFIR
AudioConnection           pc2_3(filLPWhiteFIR, 0, mixChannels, 0); //Low pass filtered whiteOut to to mixChannels input 0 (mixChannels inputs  2,3 not used)
AudioConnection           pc2_4(filLPIn_0_FIR, ppLPInput); // filLPIn_0_FIR out t ppLPInput (note: p-p measurement)
AudioConnection           pc2_5(mixChannels, ampLeftOut); // mixChannels to LeftOutAmp for gain adjust
AudioConnection           pc2_6(mixChannels, ampRightOut); // mixChannels to RightOutAmp for gain adjust
AudioConnection           pc2_7(ampLeftOut, 0, i2s1, 0);// AmpLeftOut to headphones left ch
AudioConnection           pc2_8(ampRightOut, 0, i2s1, 1);// AmpRightOut to headphones right ch
AudioConnection           pc2_9(filLPIn_0_FIR, 0 , mixChannels, 3); //filLPIn_0_FIR to mixChannels for Busy detect
AudioConnection           pc2_10(mixChannels, fft1024);// mixChannels to FFT analysis
AudioConnection           pc2_11(ampRightOut, ppAmpRightOut); // p-p monitor for ampRightOut
AudioConnection           pc2_12(ampLeftOut, ppAmpLeftOut);// p-p monitor for ampLeftOut
AudioConnection           pc2_13(filLPIn_0_FIR, rmsLPInput);// LPDownmix output to rms measurement to allow PAPR computation.
AudioConnection           pc2_14 (filLPWhiteFIR,rmsNoise); //rms measurement of BW filtered WGN

//End of audio connections (Currently 38 connections) 

#define SCL1  16 // pin 16 for Teensy 4.0 SCL1
#define SDA1  17 // pin 17 for Teensy 4.0 SDA1
#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCDp
 typedef volatile struct fir_arg_float_t
{
  float x;        // input sample x
  uint16_t nh;      // length of vector h  
  float *h;       // pointer to coefficient vector h
  float *delayx;     // pointer to delay buffer
  float r;        // output sample r
} fir_arg_float;


// Static Arrays:
static float fltLogs[] = { 0.0, .01, .02, .05, .1, .2, .5, 1.0, 2.0}; // Log values (Used [with scaling] for all parameters with log steps)
static char  *chrModes[] = {"WGN:            ", "MPG:            ", "MPM:            ", "MPP:            ", "MPD:            ","MULTIPATHS      ",
                            "FADE DEPTH:     ", "FADE FREQ:      ", "OFFSET:         ", "FM DEVIATION:   ", "FM RATE:        ",
                            "CH1 IN:         ", "CH2 IN:         ", "CH1 OUT:        ", "CH2 OUT:        ", "BANDWIDTH:      ", "SERIAL:         ", 
                            "TEST3K Sine:    ", "TEST6K Sine:    ", "BUSY:           "};
static char  *chrBusyModes[] = {"ENB BUSY:       ", "DIS BUSY:       ", "LOW:            ", "HIGH:           ",
                                "THRESH:         ", "TONE ON:        ", "TONE OFF:       ", "CH1 IN          ", "BANDWIDTH       ", "SPECTRUM:       ", "SIM:            "
                               };

//Strings
String strMode = ""; String strParameter = ""; String strRevision = "    Rev 2.03";
String strLastCF = ""; String strLastLevel = "";

//Boolean
boolean blnSim = true;//Default for Simulator operation, false = busy detector mode.
boolean blnInitialized = false; boolean blnEnableTestTone = false;
boolean blnPlotSpectrum = false; boolean blnInitSpectrum = false; 
boolean blnModes = true; boolean blnInitModes = true; boolean blnColon = false; boolean blnChanBusySent = false;
boolean blnChanClearSent = false; boolean blnPlotBusyRed = false; boolean blnEnbBusyDetect = false;
boolean blnTestMode3K = false; boolean blnTestMode6K = false; boolean blnInitializedFromEEPROM = false; boolean blnResetrmsMixIQCount = false; 
boolean blnDisplayLvl = true;

// Unsigned long
unsigned long ulngLastSpectrumUpdateMs = millis(); unsigned long ulngLastLevelDisplayMs = millis();
unsigned long ulngLastDelayUpdateUs; unsigned long ulngCurrentElapsedTimeUs ; unsigned long ulngLastPPAvgTimeUs; unsigned long ulngLastSNUpdateUs;

//Long
long lngENC1Old = 0; long lngENC2Old = 0; long lngENC1New = 0; long lngENC2New = 0; // For using Encoder2 library

//Int
int intA3history = 0;  int intTargetSN = 40; int intBandwidth = 3000; int intLastBandwidth = 6000;
int intMode = 0;// The main index 0 - 18. Always points to one of the 19 cases of chrModes
int intFadeRatePtr = 1; int intFMRatePtr = 1; int intFMDevPtr = 1;
int intFadeDepth_dB = 0;// Fade depth in dB (0 to 40 dB) bounded to Minimum S:N of -40dB
int intTuneOffset = 0;// Small (+/- 200 Hz max) frequency offset to allow check for tuning
int intGainLevel[] = {4, 4, 7, 7}; //Default gains:[100mv, 100mv,  1000mv  p-p, 1000mv p-p;
int intDelayToUpdate = 0; int intFFTCnt = 0; int intStartFFTus; int intBusyMode = 0; int intBinsAveraged = 0; int intNumofBinsToPlot = 0;
int intBusyFreqStep = 100; int intBusyBWLoHz = 0; int intBusyBWHiHz = 6500;  int intBusyGain = 10;
int intLastIQPlaneMode = -1; int intDetectSN = 0;
int intSerialCmdMode = -1; int intSerialCmdParam = -1; int intAvg = 10; int intSpecLow = 0; int intSpecHigh = 6500; int intThresh = 10;
int intTestFreqHz = 1500; int intDelayCount = 0; int intMultipaths = 2;  int intPathPtr =1; int intBaudPtr = 1; int intCountppMixIQOut =0;
int intCountrmsMixIQ1234Out = 0;
static int intMaxSN = 40; static int intMinSN = -40; static int intMaxFade = 40;
static int intMinOffset = -200; static int intMaxOffset = 200; static int intOffsetStep = 10; static int intNumModes = 19;
static int intNumBusyModes = 11; static int intBaudRate[] = {4800, 9600, 14400, 19200, 38400, 57600, 115200};

//Float
float fltppLPInputMeasAvg =0; float fltppLPInputPk; float fltrmsLPInputMeasAvg;// Measurement of filLPIn_0_FIR output
float fltVLF_UpFreq = 100; float fltVLF_Amp = 0; float fltFadeRate = 0;
float fltCurrentDelayI0Ms = 1.3378684807 ; float fltCurrentDelayI1Ms = 1.3378684807  ; //Minimum delays for I Channels (offsets 120 tap Hilbert Filter) 
float fltUpdatedDelayMs; float fltMaxDelayMs;
float fltppInpSelectMeas; float fltppInpSelectMeasAvg; float fltPeakSum = 0; float fltNoiseSum = 0;
float fltFFT[151]; float fltMax = 0; float fltAvg = 0; float fltSum; float fltSortedBins[151];
float fltAvgBinsOfInt = 0.0;// the rolling average of all bins within the bins of interest (experimental for busy detector)
float fltppi2s0InAvg = 0.0; float fltppi2s1InAvg = 0.0; float fltppAmpRightOutAvg = 0.0; float fltppAmpLeftOutAvg = 0.0; //Testing and Cal
float fltMixI1Gain; float fltMixQ1Gain; float fltMixI2Gain; float fltMixQ2Gain; float fltppMixIQOutAvg = 0; float fltppMixIQOut = 0; 
float fltrmsMixIQ1234Out =0.0; float fltrmsMixIQ1234OutAvg = 0.0; float fltrmsMixIQ1234OutPeak = 0.0;
float fltrmsNoiseAvg = 0.0; float fltInput_pp_to_rmsRatio=0.0; float fltComputedrmsMixIQOutput =0.0; 
float fltppInputForDisplay = 0.0; float fltSlidingWindowI2Sum = 0.0; 
static float fltCalTestLevel = .11416 ;// Value for calibration to set max Peak measured level to ~1800 
static float fltNomInOutRatio = .41055;// Average calibration value for ratio Input to Output
static float fltMinimumIDelayMs = 1.3378684807; //59 taps. Compensates for 59 tap delay for 119 tap Hilbert filter on  Q paths  
static float fltNomCalLevel = 1800.0;

//Filters
/*
  High pass FIR filter designed with
  http://t-filter.appspot.com
  sampling frequency: 44100 Hz
  fixed point precision: 16 bits
  0 Hz - 7100 Hz
  gain = 0
  desired attenuation = -60 dB
  actual attenuation = -61.1
  7900 Hz - 22000 Hz
  gain = 1
  desired ripple = 1 dB
  actual ripple = .66 dB
*/
static short sht7500HzHPCoef[120] = {
  171, -270, -54, 326, -105, -5, -180, 84, 38, 128, -66, -69, -104, 63, 98, 91, -73, -126, -81, 92, 152, 71, -119, -178, -57, 155, 204,
  38, -198, -229, -11, 250, 254, -25, -313, -277, 73, 388, 299, -138, -481, -319, 228, 598, 337, -354, -759, -352, 547, 997, 364, -874,
  -1415, -372, 1573, 2412, 378, -4308, -9215, 21466, -9215, -4308, 378, 2412, 1573, -372, -1415, -874, 364, 997, 547, -352, -759, -354,
  337, 598, 228, -319, -481, -138, 299, 388, 73, -277, -313, -25, 254, 250, -11, -229, -198, 38, 204, 155, -57, -178, -119, 71, 152, 92,
  -81, -126, -73, 91, 98, 63, -104, -69, -66, 128, 38, 84, -180, -5, -105, 326, -54, -270, 171, 0
};// 0 tap added to keep #even.

/* Rev 2 6 KHz FIR FIlter (less ripple) 120 taps
  FIR filter designed with
  http://t-filter.appspot.com
  sampling frequency: 44100 Hz
  fixed point precision: 16 bits
  0 Hz - 6300 Hz
  gain = 1
  desired ripple = 0.5 dB
  actual ripple = .12 dB

  7300 Hz - 22000 Hz
  gain = 0
  desired attenuation = -50 dB
  actual attenuation = -59.9 dB
*/
static short sht6KHzLPFIRCoeffRev2[120] = {
  -18, -52, -45, -33, 6, 35, 36, 2, -38, -50, -18, 36, 66, 40, -27, -80, -69, 8, 89, 101, 23, -88, -135, -66, 74, 166,
  120, -42, -188, -182, -10, 195, 250, 85, -180, -318, -185, 137, 379, 309, -55, -423, -459, -77, 439, 637, 276, -412,
  -851, -579, 311, 1129, 1078, -62, -1573, -2124, -648, 2805, 6888, 9637, 9637, 6888, 2805, -648, -2124, -1573, -62, 1078,
  1129, 311, -579, -851, -412, 276, 637, 439, -77, -459, -423, -55, 309, 379, 137, -185, -318, -180, 85, 250, 195, -10, -182,
  -188, -42, 120, 166, 74, -66, -135, -88, 23, 101, 89, 8, -69, -80, -27, 40, 66, 36, -18, -50, -38, 2, 36, 35, 6, -33, -45,
  -52, -18
};

/* Rev 2 FIR filter designed with
  http://t-filter.appspot.com
  sampling frequency: 44100 Hz
  fixed point precision: 16 bits
  0 Hz - 3300 Hz
  gain = 1
  desired ripple = 0.5 dB
  actual ripple = .05 dB
  4500 Hz - 22000 Hz
  gain = 0
  desired attenuation = -50 dB
  actual attenuation = -66.8
*/
static short sht3KHzLPFIRCoeffRev2[120] = {
  5, -6, -11, -17, -22, -24, -19, -8, 7, 24, 36, 39, 30, 10, -19, -47, -66, -67, -47, -8, 40, 85, 111, 105, 66, -2, -80, -146,
  -176, -154, -81, 30, 149, 239, 267, 215, 87, -90, -265, -384, -400, -291, -71, 210, 473, 631, 616, 399, 5, -478, -921, -1173,
  -1102, -629, 247, 1440, 2785, 4071, 5080, 5634, 5634, 5080, 4071, 2785, 1440, 247, -629, -1102, -1173, -921, -478, 5, 399, 616,
  631, 473, 210, -71, -291, -400, -384, -265, -90, 87, 215, 267, 239, 149, 30, -81, -154, -176, -146, -80, -2, 66, 105, 111, 85,
  40, -8, -47, -67, -66, -47, -19, 10, 30, 39, 36, 24, 7, -8, -19, -24, -22, -17, -11, -6, 5
};

// Alternate Hilbert filter 
//Iowa Hills 119 Tap Phase Added BPF.ih_hilbert
// Samp Freq=44100Hz, Fc 3.09KHz, BW:6.88KHz,Num Taps = 119, Phase Add:90 Kaiser Beta 2.8, Raised Cosine .930 Auto Gain -.1411
// Add one 0 tap at end to satisfy Even # of taps requirement for Teensy FIR loader. 
static double dblHilbertFIR_IowaHills_119TapsCoeff[120] {
 0.001075300944082752, 0.001557842094045493, 0.002067956459847213, 0.002232229700062531,
 0.001915327302396347, 0.001413511286627190, 0.001279608958818636, 0.001857667356399288,
 0.002905904225855494, 0.003690116277597962, 0.003565457635367228, 0.002602206425023009,
 0.001673230882225981, 0.001807494527589340, 0.003265489369521406, 0.005145285359644720,
 0.005990378025073845, 0.005026943287679242, 0.003002935960215830, 0.001780861353642867,
 
 0.002834237448682267, 0.005834597002163801, 0.008639548675492503, 0.008932544947514989,
 0.006296747899610226, 0.002907384204440259, 0.001988005015538047, 0.005069693282134854,
 0.010342669613640223, 0.013761059873684783, 0.012333472204065523, 0.006974666580179149,
 0.002288401336281471, 0.003009816731856978, 0.009789686192278572, 0.017992973621028828,
 0.021038195815007790, 0.015981078878614750, 0.006706466072115251, 0.001528268353695360,
 
 0.006480454546744812, 0.019590157925452621, 0.031356416122142114, 0.031984623673091885,
 0.020070756638178998, 0.005326713375133442, 0.001966066428253671, 0.016951977819063026,
 0.042458353075356919, 0.059855241846675102, 0.053956031386950570, 0.027374824579469624,
 0.002955748656588099, 0.009890051106628113, 0.061467904275008395, 0.140243163017217620,
 0.203070364439508322, 0.205332573839930616, 0.129674892922227059,

 -89.01547616414120E-15,
 
-0.129674892922367141,-0.205332573839978189,-0.203070364439459611,-0.140243163017117312,
-0.061467904274918730,-0.009890051106591132,-0.002955748656605178,-0.027374824579509106,
-0.053956031386974829,-0.059855241846666421,-0.042458353075325299,-0.016951977819033567,
-0.001966066428245613,-0.005326713375147162,-0.020070756638198857,-0.031984623673100447,
-0.031356416122133482,-0.019590157925435111,-0.006480454546732502,-0.001528268353696470,

-0.006706466072126626,-0.015981078878625786,-0.021038195815009545,-0.017992973621020644,
-0.009789686192267468,-0.003009816731851239,-0.002288401336284157,-0.006974666580186507,
-0.012333472204070857,-0.013761059873684051,-0.010342669613634762,-0.005069693282129567,
-0.001988005015537044,-0.002907384204443737,-0.006296747899614824,-0.008932544947516852,
-0.008639548675490322,-0.005834597002159460,-0.002834237448678956,-0.001780861353642518,

-0.003002935960217908,-0.005026943287681609,-0.005990378025074670,-0.005145285359643786,
-0.003265489369519931,-0.001807494527588732,-0.001673230882226682,-0.002602206425024312,
-0.003565457635368013,-0.003690116277597603,-0.002905904225854225,-0.001857667356397879,
-0.001279608958817785,-0.001413511286627150,-0.001915327302396961,-0.002232229700063489,
-0.002067956459848226,-0.001557842094046308,-0.001075300944083103, 0.0};/// Last 0 coefficient = 0 to satisfy Teensy loader 


short shtHilbertFIR_120TapCoeff[120]; //This array will be populated from the above
void(* resetFunc)(void) = 0;//declare reset function at address 0

/*****************Subroutine to adjust mixChannels gains to obtain desired S:N Simulation mode ***********/
void AdjustS_N (int intDesiredSN_dB, float fltppLPInputMeasAvg)
{
  //Sets the appropriate gains of mixChannels inputs 0,1 and 2 to obtain the desired S:N
  /*
   * Revised 10/3/2020 to accomodate Multipath changes and to manage p-p output of mixChannels
   * Sets the S:N ratio based on the average p-p value of fltppLPInputMeasAvg. This is the sliding window Filtered p-p Input before any scaling or multipath processing 
   * This accomodates the wide (>4:1 swings) in output from Mix IQ when in Multipath modes and keeps the long term average S:N as requested (-40 to +40 db) 
   * The scaling of S:N is based on sliding window p-p readings of the input to not give any advantage of High CF signals (which have lower Energy /bit) 
   * Reference for computing Complex power: https://www.tek.com/blog/calculating-rf-power-iq-samples
   * White noise is already scaled for Bandwidth 
   */
  static int intLastDesiredSN_dB =41;// Used to allow debug Serial.print below only on changes to requested S:N (for debugging). 
  static float fltrmsDelayI0; static float fltrmsDelayI0Avg = 0.0;
  float fltSignalBWCorrectiondB = 10 * log10(float(intBandwidth)/float(intBandwidth +300));// Provides approx .4 or .2 dB correction for 3.3 or 6.3 KHz Filters:3KHz or 6KHz BW
  float fltS_NCorrectiondB = 4.28; //Trial Calibration factor  
  // Note Calibration factor 4.28 above verified to produce same S:N Spectrum Analyzer measured values (+/- ~.2dB) as revision .9.81 
    
  if (rmsDelayI0.available())// Compute and average the rms Output of Delay tap I0
    {
      fltrmsDelayI0 = 849.8 *  rmsDelayI0.read();// This is the audio input from mixPathSel output time alligned to the HilbertQ12FIR output
      //Note scale factor 849.8 used instead of 1000 to scale output of MixIQ down to avoid limiting on MultiPath Modes. 
      fltrmsDelayI0Avg = (.95 * fltrmsDelayI0Avg) + .05 * fltrmsDelayI0; //moderate - slow speed rolling average
    }
  double fltDesiredSNRatio = pow(10.000, ((float(intDesiredSN_dB) + fltS_NCorrectiondB + fltSignalBWCorrectiondB)/ 20.0));//Compute the S:N ratio based on Desired S:N dB  and bandwidth correction ( .01 to 100 for -/+ 40 dB)
  float fltSignalGain  =  fltDesiredSNRatio/(1.0 + fltDesiredSNRatio);
  float fltNoiseGain =  (1- fltSignalGain);
  // Set gains for mixChannels
  mixChannels.gain(0, fltNoiseGain);// Ch 0 (noise) gain factor 
  mixChannels.gain(1, fltSignalGain); // Ch 1,2,3,4 (signal) gain factor// F
  mixChannels.gain(2, 0.0);
  mixChannels.gain(3, 0.0);//Insure other mixChannels input disabled  
   
    //  Debug printout for calibration 
  if (intDesiredSN_dB != intLastDesiredSN_dB)
    {
      Serial.println(" ");
      Serial.println("AdjustS_N:");
      Serial.print(" DesiredSN_db= ");Serial.println(intDesiredSN_dB); 
      Serial.print(" Bandwidth Correction Factor (dB)= ");Serial.println(fltSignalBWCorrectiondB);
      Serial.print(" ppLPInputMeasAvg= ");Serial.println(fltppLPInputMeasAvg);
      Serial.print(" fltrmsDelayI0Avg= ");Serial.print(fltrmsDelayI0Avg); Serial.print("   Computed rmsMixIQOutput= ");Serial.println(fltComputedrmsMixIQOutput);
      Serial.print(" rmsNoiseAvg= ");Serial.println(fltrmsNoiseAvg);
      Serial.print(" Desired SN Ratio= ");Serial.println(fltDesiredSNRatio);
      Serial.print("    SigGain=");Serial.print(fltSignalGain); Serial.print("    NoiseGain=");Serial.println(fltNoiseGain);
      Serial.print(" CF= ");Serial.println( fltppLPInputMeasAvg/(2.828 * fltrmsLPInputMeasAvg));
      Serial.print(" intMultipaths = ");Serial.println(intMultipaths);
      Serial.print(" rmsMixIQ1234Out="); Serial.println(fltrmsMixIQ1234Out);
      intLastDesiredSN_dB = intDesiredSN_dB;
    }
 }//**End AdjustS_N******************************************************************************************

//*****************Subroutine to adjust mixChannels gains to obtain desired S:N for Busy Detector Testing ***
void AdjustBusyS_N (int intDesiredSN_dB, float fltppInpSelectMeasAvg , float fltppWhiteMeasAvg)
{
  //Sets the appropriate gains of mixChannels inputs 0 and 2 to obtain the desired S:N
  double fltDesiredSNRatio = pow(10.000, (intDesiredSN_dB / 20.0));
  if (fltDesiredSNRatio >= 1)
  {
    //Signal >= Noise. (S:N db >= 0) fltDesiredSNRatio >= 1 Compute effective needed gain values for mixChannels Noise input
    mixChannels.gain(0, (fltppInpSelectMeasAvg / fltppWhiteMeasAvg) / fltDesiredSNRatio);
    mixChannels.gain(2, 1); // may need to calibrate to insure no clipping/saturation
  }
  else
  {
    //Signal < Noise. (S:N db < 0) fltDesiredSNRatio < 1 Compute effective needed gain values for mixChannels Signal input
    mixChannels.gain(0, (fltppInpSelectMeasAvg / fltppWhiteMeasAvg));
    mixChannels.gain(2, fltDesiredSNRatio);
  }
  mixChannels.gain(1, 0); // Insures no bleed through of mixChannels input 1
  return;
}//**End AdjustBusyS_N*************************************************************************************

//*********Subroutine For Fading **************************************************************************
// Can be applied to all modes WGN-MPD. Adjusts mixChannels gains to obtain desired S:N and depth of fade
void Fade (int intMaxSN_dB, int intFadeDepth_dB, float fltFadeRateHz)
{
  //Sets the appropriate gains of mixerChannels inputs 0 and 1 to obtain the desired S:N
  int intCurrentFadeSN_dB; 
  float fltRunTimeSec = millis()/1000.0; //rollover only will happen every 50 days!
  intCurrentFadeSN_dB = intMaxSN_dB -  int((intFadeDepth_dB * .5 * (1 - cos(fltFadeRateHz * 2 * 3.14159 * fltRunTimeSec))));
  if (intCurrentFadeSN_dB < -40) { intCurrentFadeSN_dB = -40;} // bound minimum fade to intMinSN 
  AdjustS_N (intCurrentFadeSN_dB , fltppLPInputMeasAvg);
  return;
}//**End Fade****************************************************************************************************

//*************** From Iowa Hills FIR Filter Designer Version 7.0
/* File: 128 Tap Adj Gauss LPF Rev2.ih_fir
 * Sample Freq 64 Hz, Fc=.5856 Hz, Window Off, Num Taps 128, ~Gauss -.900, N Poles=9
 * Freq Response: 1Hz: -9.1dB, 2Hz: -37.1 dB, 3Hz: -66.8 dB; Group delay ~ 840 ms
 */
static float gaus_fir_coeffs[128]
{11.75559267133204600E-12, 201.8800495613742780E-12, 1.723633362394617620E-9, 9.815423109243150530E-9,
 42.19820040519088170E-9, 146.9342948623463490E-9, 433.8503956649552150E-9, 1.122118806000393040E-6,
 2.604091536729610910E-6, 5.522713327023963000E-6, 10.85739046564233410E-6, 20.01141264924749310E-6,
 34.89306870616279350E-6, 57.98247725939228300E-6, 92.37678934582388020E-6, 141.8076728400771460E-6,
 
 210.6266900063565690E-6, 303.7561533907517630E-6, 426.6051229102419030E-6, 584.9522379382009380E-6,
 784.7989367901134300E-6, 0.001032198204838678, 0.001333065243166745, 0.001692977321877409,
 0.002116970561258896, 0.002609341477645015, 0.003173460865247882, 0.003811607001168640,
 0.004524824309342139, 0.005312812558055807, 0.006173850455688009, 0.007104756211217515,
 
 0.008100886298035966, 0.009156172355120720, 0.010263194925878348, 0.011413291611735991,
 0.012596696236577472, 0.013802704802828978, 0.015019863385625786, 0.016236172665450827,
 0.017439303542147160, 0.018616818198095349, 0.019756391073966928, 0.020846024470695095,
 0.021874253876569307, 0.022830338616651881, 0.023704434009553566, 0.024487741869950012,
 
 0.025172636890277961, 0.025752767148979578, 0.026223127704178725, 0.026580106921515002,
 0.026821505836160390, 0.026946531447567135, 0.026955765379786157, 0.026851109801616951,
 0.026635712883536795, 0.026313876369100774, 0.025890948056565766, 0.025373202123371387,
 0.024767710285281262, 0.024082206768594926, 0.023324949994399220, 0.022504583735910824,
 
 0.021630000321890532, 0.020710208229666707, 0.019754206149457228, 0.018770865316331563,
 0.017768821605931590, 0.016756378583127243, 0.015741422386651591, 0.014731349034225070,
 0.013733004447687327, 0.012752637231260112, 0.011795863992392318, 0.010867646776884902,
 0.009972282000446704, 0.009113400098909145, 0.008293974989634013, 0.007516342337046732,
 
 0.006782225544921226, 0.006092768355660706, 0.005448572920512081, 0.004849742212182516,
 0.004295925680163602, 0.003786367096475506, 0.003319953602663025, 0.002895265044807468,
 0.002510622769190125, 0.002164137144270543, 0.001853753172183700, 0.001577293652557479,
 0.001332499460868328, 0.001117066600796574, 928.6797833839573290E-6, 765.0423737761393570E-6,
 
 623.9026277671727030E-6, 503.0762143350814310E-6, 400.4650862100223210E-6, 314.0728178353742240E-6,
 242.0165786791055780E-6, 182.5359497431699650E-6, 133.9988224980702400E-6, 94.90464268873809320E-6,
 63.88527699708468790E-6, 39.70378899074398050E-6, 21.25141280107635570E-6, 7.543009276742865590E-6,
-2.288719293693219470E-6,-8.999992637884092870E-6,-13.24344714850232660E-6,-15.57613368708467760E-6,

-16.46781142685044590E-6,-16.30935284928609970E-6,-15.42109793945524210E-6,-14.06101870492278390E-6,
-12.43257788819570920E-6,-10.69218773541830810E-6,-8.956195575428226970E-6,-7.307342471035109810E-6,
-5.800659111239630410E-6,-4.468779263172822700E-6,-3.326665397016021600E-6,-2.375753489237729440E-6,
-1.607534498745384790E-6,-1.006598637105850620E-6,-553.1753923550552370E-9,-225.2074190014706120E-9};

//fir_float routine From Peter (processed at 64 x the Dopplar Spread ...not using Teensy Audio)  *********************
float delay_bufferI1[128];
float delay_bufferQ1[128];
float delay_bufferI2[128];
float delay_bufferQ2[128];
float delay_bufferI3[128];
float delay_bufferQ3[128];
float delay_bufferI4[128];
float delay_bufferQ4[128];

fir_arg_float firI1;
fir_arg_float firI2;
fir_arg_float firQ1;
fir_arg_float firQ2;
fir_arg_float firI3;
fir_arg_float firI4;
fir_arg_float firQ3;
fir_arg_float firQ4;


void fir_float(fir_arg_float *f)
{
  uint16_t i;
  uint16_t u, v;
  uint16_t nh;
  float *delayx, *h;

  float sum;

  delayx = f->delayx;
  nh = f->nh;
  h = f->h;

  u = nh-2;
  v = nh-1;

  for(i=0; i<nh-1; i++)
  {
    delayx[v] = delayx[u];
    u--;
    v--;
  }

  delayx[0] = f->x;

  sum = 0;

  for(i=0; i<nh; i++)
  {
    sum += delayx[i] * h[i];
  }

  f->r = sum;
}
// *****End of fir_float *********************************************************************************************


void QuadGauss12FIR128( int Mode)
{
  // Implements generation of Gaussian filtered noise  for the 2 Paths (I1,Q1, I2,Q2) case (Rev 2.x in process of expanding to 3 path and/or 4 path) 
  // Test 8/12/2020 was 12-14 microsec for 2Path (I1,Q1, I2,Q2) execution
  
  float fltMagI[2]= {0.0, 0.0}; float fltMagQ[2]= {0.0, 0.0};  float fltComplexMagSq = 0.0 ;
  float fltComplexMag; float fltRoot; float fltSTD = 4.0 / 1.41421;// default fltSTD for 4 Paths comfirmed 11/6//2020. 
  float fltI1; float fltI2; float fltQ1; float fltQ2;
  //unsigned long ulngTstart = micros();
  // Generate the 4 Gaussian random I and Q values for all 4 I, Q, Paths 
  //if (intMultipaths == 2){fltSTD = 3.93895;}//Corect fltSTD for 2 paths to keep constant avg output as WGN and 4 Path 
  if (intMultipaths == 2){fltSTD = 4.0;}//Corect fltSTD for 2 paths to keep constant avg output as WGN and 4 Pat
  for (int k=0; k<2; k++)
      {
      fltComplexMagSq = 2.0;// Init to value OUTSIDE unit circle
      while ((fltComplexMagSq >=1.0) || (fltComplexMagSq <.1))// This mechanism insures the ComplexMag random value falls inside the unit circle  ()
        {
          fltMagI[k] = .00001 * random(-100000,100001);
          fltMagQ[k] = .00001 * random(-100000,100001);
          fltComplexMagSq = (fltMagI[k] * fltMagI[k]) + (fltMagQ[k] * fltMagQ[k]);
        } 
     
      fltComplexMag = sqrt(fltComplexMagSq);
      fltRoot = sqrt(-2.0 * log(fltComplexMagSq));
      fltMagI[k] = (fltMagI[k] * fltRoot * fltSTD)/fltComplexMag;
      fltMagQ[k] = (fltMagQ[k] * fltRoot * fltSTD)/fltComplexMag;
      //if (k == 0){Serial.print("fltMagI[0]= ");Serial.print(fltMagI[k]); Serial.print("  fltMagQ[0]= ");Serial.println(fltMagQ[k]); }
      //else {Serial.print("fltMagI[1]= ");Serial.print(fltMagI[k]); Serial.print("  fltMagQ[1]= ");Serial.println(fltMagQ[k]);}  
    }
  // From Hans Peter (modified for four filter instances) 
  firI1.delayx = delay_bufferI1;
  firI1.nh = 128;
  firI1.h = gaus_fir_coeffs;
  firI1.x = fltMagI[0];
  fir_float(&firI1);
  fltI1 = firI1.r;

  firQ1.delayx = delay_bufferQ1;
  firQ1.nh = 128;
  firQ1.h = gaus_fir_coeffs;
  firQ1.x = fltMagQ[0];
  fir_float(&firQ1);
  fltQ1 = firQ1.r;

  firI2.delayx = delay_bufferI2;
  firI2.nh = 128;
  firI2.h = gaus_fir_coeffs;
  firI2.x = fltMagI[1];
  fir_float(&firI2);
  fltI2 = firI2.r;

  firQ2.delayx = delay_bufferQ2;
  firQ2.nh = 128;
  firQ2.h = gaus_fir_coeffs;
  firQ2.x = fltMagQ[1];
  fir_float(&firQ2);
  fltQ2 = firQ2.r;
  //Serial.print("Time for QuadGaussFIR128:(us):");Serial.println(micros() - ulngTstart);
  /* Explanation of how mixIQ combines and computes Mixed output for 2 Paths:
   *  Assume the fltMagI and fltMagQ values create two complex modulation vectors #1 is fltMagI[0] + j fltMagQ[0] 
   *  #2 is fltMagI[1] +  j fltMagQ[1] one for each path (I1,Q1 and I2,Q2)
   *  for the I1 Path gain the product of I1 and the Modulation vector is:
   *     I1 * fltMagI[0]  + j * I1 * fltMagQ[0]  The second component is the Quadrature component. 
   *  for the Q1 Path gain the product of Q1 and the Modulation vector is:
   *     j * Q1 * fltMagR[0] + j *j * fltMagQ[0] or  j * Q1 * fltMagR[0]  - 1 * fltMagQ[0] {j * j is replaced by-1}
   *     Adding only the real components yields I1 * fltMagR[0] - Q1 * fltMagQ[0]
   *     Imaginary components are discarded (only real audio is sent to the modem). 
   *  A similar process is used for the I2 and Q2 components for path I2,Q2   
   */
  mixIQ12.gain(1, fltI1); mixIQ12.gain(0,-fltQ1); mixIQ12.gain(3, fltI2); mixIQ12.gain(2,-fltQ2);//Update the mixIQgains
  mixPathSel.gain(0, .5);mixPathSel.gain(1, 0.0);;mixPathSel.gain(2, 0.0);mixPathSel.gain(3, 0.0);
}//  End *************************************** QuadGauss12FIR128 ************************************

void QuadGauss34FIR128( int Mode)
{
  // Implements generation of Gaussian filtered noise  for the 2 Paths (I3,Q3, I4,Q4) case (expansion to 4 paths) 
 
  
  float fltMagI[2]= {0.0, 0.0}; float fltMagQ[2]= {0.0, 0.0};  float fltComplexMagSq = 0.0 ;
  float fltComplexMag; float fltRoot; float fltSTD = 4.0 / 1.41421;// fltSTD for 4 paths comfirmed 11/6/2020. 
  float fltI3; float fltI4; float fltQ3; float fltQ4;
  //unsigned long ulngTstart = micros();
  
  // Generate the 4 Gaussian random I and Q values for all 4 I, Q, Paths 
  for (int k=0; k<2; k++)
    {
      fltComplexMagSq = 2.0;// Init to value OUTSIDE unit circle
      while ((fltComplexMagSq >=1.0) || (fltComplexMagSq <.1))// This mechanism insures the ComplexMag random value falls inside the unit circle  ()
        {
          fltMagI[k] = .00001 * random(-100000,100001);
          fltMagQ[k] = .00001 * random(-100000,100001);
          fltComplexMagSq = (fltMagI[k] * fltMagI[k]) + (fltMagQ[k] * fltMagQ[k]);
        } 
     
      fltComplexMag = sqrt(fltComplexMagSq);
      fltRoot = sqrt(-2.0 * log(fltComplexMagSq));
      fltMagI[k] = (fltMagI[k] * fltRoot * fltSTD)/fltComplexMag;
      fltMagQ[k] = (fltMagQ[k] * fltRoot * fltSTD)/fltComplexMag;
      //if (k == 0){Serial.print("fltMagI[0]= ");Serial.print(fltMagI[k]); Serial.print("  fltMagQ[0]= ");Serial.println(fltMagQ[k]); }
      //else {Serial.print("fltMagI[1]= ");Serial.print(fltMagI[k]); Serial.print("  fltMagQ[1]= ");Serial.println(fltMagQ[k]);}  
    }
  // From Hans Peter (modified for four filter instances) 
  firI3.delayx = delay_bufferI3;
  firI3.nh = 128;
  firI3.h = gaus_fir_coeffs;
  firI3.x = fltMagI[0];
  fir_float(&firI3);
  fltI3 = firI3.r;

  firQ3.delayx = delay_bufferQ3;
  firQ3.nh = 128;
  firQ3.h = gaus_fir_coeffs;
  firQ3.x = fltMagQ[0];
  fir_float(&firQ3);
  fltQ3 = firQ3.r;

  firI4.delayx = delay_bufferI4;
  firI4.nh = 128;
  firI4.h = gaus_fir_coeffs;
  firI4.x = fltMagI[1];
  fir_float(&firI4);
  fltI4 = firI4.r;

  firQ4.delayx = delay_bufferQ4;
  firQ4.nh = 128;
  firQ4.h = gaus_fir_coeffs;
  firQ4.x = fltMagQ[1];
  fir_float(&firQ4);
  fltQ4 = firQ4.r;
  //Serial.print("Time for QuadGaussFIR128:(us):");Serial.println(micros() - ulngTstart);
  /* Explanation of how mixIQ combines and computes Mixed output for 2 Paths:
   *  Assume the fltMagI and fltMagQ values create two complex modulation vectors #1 is fltMagI[0] + j fltMagQ[0] 
   *  #2 is fltMagI[1] +  j fltMagQ[1] one for each path (I1,Q1 and I2,Q2)
   *  for the I1 Path gain the product of I1 and the Modulation vector is:
   *     I1 * fltMagI[0]  + j * I1 * fltMagQ[0]  The second component is the Quadrature component. 
   *  for the Q1 Path gain the product of Q1 and the Modulation vector is:
   *     j * Q1 * fltMagR[0] + j *j * fltMagQ[0] or  j * Q1 * fltMagR[0]  - 1 * fltMagQ[0] {j * j is replaced by-1}
   *     Adding only the real components yields I1 * fltMagR[0] - Q1 * fltMagQ[0]
   *     Imaginary components are discarded (only real audio is sent to the modem). 
   *  A similar process is used for the I2 and Q2 components for path I2,Q2   
   */
  mixIQ34.gain(1, fltI3); mixIQ34.gain(0,-fltQ3); mixIQ34.gain(3, fltI4); mixIQ34.gain(2,-fltQ4);//Update the mixIQgains
  mixPathSel.gain(0, .5);mixPathSel.gain(1, 0.0);;mixPathSel.gain(2, 0.0);mixPathSel.gain(3, 0.0);
}//  End *************************************** QuadGauss34FIR128************************************



// Subroutine to plot 3.5 or 6.5 KHz spectrum on the Graphics display)***********************************
void PlotSpectrum(float fltFFTBins[], int intLowHz, int intHighHz, boolean blnInitSpectrum, int intBW )
{
  // fltFFTBins[] is an array of bins each representing 43.0664Hz (44100/1024) representing 3.5 or 6.5 KHz of spectrum
  // all elememts of fltFFTBins range 0 to 4.30 4.30 = 2000 mv, 1000 mv =4.0
  // intLowHz is the lowest frequency bin of interest, below this spectrum plots will be yellow
  // intHighHz is the highest frequency bin of interest above this spectrum plots will be yellow
  // between (inclusive) intLow and int High spectrum plots will be green
  // plots can be linear or log and are scaled outside this PlotSpectrum routine. All plot values must be scaled 0 to 144
  int intLowBin = round(intLowHz / 43.0664); int intHighBin = round(intHighHz / 43.0664);

  // For intBW  = 3000 all rectangles will be <= 144 pixels high x 2 pixels wide with 1 pixel spacing (total of 240 pixels)
  // For intBW = 6000  all rectangles will be <= 144 pixels high x 1 pixel wide with 1 pixel spacing (total of 302 pixels)
  if (blnInitSpectrum)//initialize tft, set rotation, clear screen, initialize text (will cause flicker)
    {
      blnInitSpectrum = false;
      tft.begin();
      tft.setCursor(0,0);
      tft.setRotation(1);
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println("    SPECTRUM");
      tft.setTextColor(ILI9341_WHITE);
      if (intBW == 3000){tft.setTextSize(2);tft.println(" 0 Hz               3.5KHz");}
      else {tft.setTextSize(2); tft.println(" 0 Hz               6.5KHz");}
    }
  if (intBW == 3000)
    {
      for (int i = 3; i < 244; i += 3)
        {
        if ((( i / 3) >= intLowBin) & ((i / 3) <= intHighBin))
          {
            tft.drawLine (40 + i, 238, 40 + i, 48, ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
            tft.drawLine (41 + i, 238, 41 + i, 48, ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
            if (blnPlotBusyRed)// Plot region of interest in Red if channel determined busy by SearchRatioDetect
              {
                tft.drawLine (40 + i, 238, 40 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_RED); //
                tft.drawLine (41 + i, 238, 41 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_RED);
              }
            else
              {
                tft.drawLine (40 + i, 238, 40 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_GREEN); //
                tft.drawLine (41 + i, 238, 41 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_GREEN);
              }
          }
        else
          {
            tft.drawLine (40 + i, 238, 40 + i, 48, ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
            tft.drawLine (41 + i, 238, 41 + i, 48 , ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
            tft.drawLine (40 + i, 238, 40 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_YELLOW); //
            tft.drawLine (41 + i, 238, 41 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 3])))), ILI9341_YELLOW);
          }
        }
      return;
    }
  else if (intBW == 6000)
    {
      for (int i = 2; i < 301; i += 2)
        {
          if (( i / 2 >= intLowBin) && (i / 2 <= intHighBin))
            {
              tft.drawLine (10 + i, 238, 10 + i, 48, ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
              tft.drawLine (10 + i, 238, 10 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 2])))), ILI9341_GREEN); //value 25 sets display vertical scaling
            }
          else
            {
              tft.drawLine (10 + i, 238, 10 + i, 48, ILI9341_BLACK); //paint "BLACK" over old plot to eliminate flicker
              tft.drawLine (10 + i, 238, 10 + i, (238 - (50 * max(0, log10(fltFFTBins[i / 2])))), ILI9341_YELLOW); //value 25 sets display vertical scaling
            }
        }
      return;
    } 
 }// End PlotSpectrum *****************************************************************************************

//*******Function to Parse and Set Parameter received via Serial Port **********************
boolean ParseSetParameter(String strParameter, int intMode)
// Determins if strParameter is compatible with intMode. if not returns false
// If compatible sets the parameter and Mode, Updates display showing mode and parameter and returns true
{
  int intParam = strParameter.toInt(); String str1; String str2; String str3; float fltParam = strParameter.toFloat();
  if (intMode < 5)//WGN thru MPD
  {
    intParam = strParameter.toInt();
    if ((-40 <= intParam) && (intParam <= 40))
    {
      //Serial.print(" ENC1 intTargetSN= ");Serial.println(intTargetSN);;
      intTargetSN = intParam;
      str1 = chrModes[intMode];  str2 = "    S:N= " + String(intTargetSN); str3 = " dB";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
  }
  if (intMode == 5)//MULTIPATHS
  {
    if ((intParam == 2) || (intParam == 4)) 
      {
        intMultipaths = intParam; return true;
      }
  
   
  }
  if (intMode == 6)//FADE DEPTH
  {
    if ((intParam >= 0) && (intParam <= 40))
    {
      intFadeDepth_dB = intParam;
      str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 =  " dB";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
  }
  if (intMode == 7)//FADE FREQ  fltFadeRates
  {
    for (int j = 0; j < 9; j += 1)
      if  ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMDevPtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(10 * fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, ""); 
        return true;
      }
  }
  if (intMode == 8)//OFFSET
    {
      if ((intParam >= -200) && (intParam <= 200))
        {
          intTuneOffset = intParam;
          str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 =  " Hz";
          UpdateTFTModeParameter(str1, str2, str3);
          return true;
        }
    }
  if (intMode == 9)// FM DEVIATION  
    {
      for (int j = 0; j < 9; j ++)
        if  ((abs(fltLogs[j] - fltParam)) < .001)
          {
            intFMDevPtr = j;
            str1 = chrModes[intMode]; str2 = "      " + String(100 * fltLogs[j]) ; str3 =  " Hz";
            UpdateTFTModeParameter(str1, str2, "");
            return true;
          }
    }
  if (intMode == 10)//FM RATE 
  {

    for (int j = 0; j < 9; j += 1)
      if ((abs(fltLogs[j] - fltParam)) < .001)
      {
        intFMRatePtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(100 * fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, "");
        return true;
      }
  }
  if ((intMode >= 11) && (intMode <= 14)) //IN and OUT levels
  {
    intParam = strParameter.toInt();
    if ((intParam >= 0) && (intParam <= 20))
    {
      intGainLevel[intMode - 11] = intParam;//Serial.print("Line 795: intParam= ");Serial.println(intParam);
      str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 =  "";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
  }
  if (intMode == 15)//Bandwidth
  {
    intParam = strParameter.toInt();
    if ((intParam == 3000) || (intParam == 6000))
    {
      intBandwidth = intParam;
      str1 = chrModes[intMode]; str2 = "      " + String(intBandwidth) ; str3 =  " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }

}// End ParseSetParameter **************************************************************

// Subroutine for Setting Filter Bandwith**********************************************************
void SetFilterBandwidth(int intBW)
{
  if (!((intBW == 3000) || (intBW == 6000))) {return;} // intBW must be 3000 or 6000
  if (intBW == 3000)
  {
    //Stop all LP filters
    filLPIn_0_FIR.end();
    filLPWhiteFIR.end();
    filLPDownMix.end();
    filLPIn_0_FIR.begin(sht3KHzLPFIRCoeffRev2, 120); // Start Input LP Filter in 3000 Hz BW mode
    filLPWhiteFIR.begin(sht3KHzLPFIRCoeffRev2, 120); // White Noise LP Filter
    filLPDownMix.begin(sht3KHzLPFIRCoeffRev2, 120); // Down Mix LP Filter
  }
  else
  {
    //Stop all LP Filters
    filLPIn_0_FIR.end();
    filLPWhiteFIR.end();
    filLPDownMix.end();
    filLPIn_0_FIR.begin(sht6KHzLPFIRCoeffRev2, 120); // Start Input LP Filter in 6000 Hz BW mode
    filLPWhiteFIR.begin(sht6KHzLPFIRCoeffRev2, 120); // White Noise LP Filter
    filLPDownMix.begin(sht6KHzLPFIRCoeffRev2, 120); // Down Mix LP Filter
  }
  return;
}//END SetFilterBandwidth ************************************************************************

// Subroutine to set IQ Tap delays based on intMode ********************************************
void SetIQTapDelays(int intMode)
{
      //These delay guidelines for MPG-MPP Described on Rec 520-2 pages 1, 2. 
      //For 2 paths use minimum delay for for Path 1, max delay for path for Path 2
      //For 4 paths use same delay increment (.5 x delay of Path type) between paths (Peter's recommendation except for 4 path MPG) 
      
     if (intMultipaths < 4)
      {
        switch (intMode)
          {
            case 0:   delayI.delay(0, fltMinimumIDelayMs);  //Minimum I0 ...no other paths.(WGN)
                      delayQ.delay(0, 0.0); break;
                
            case 1:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPG Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + .5);delayQ.delay(1, .5); break; //MPG Path #2
                                    
            case 2:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPM Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + 1.0);delayQ.delay(1, 1.0); break; //MPM Path #2
                
            case 3:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPP Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + 2.0);delayQ.delay(1, 2.0); break; //MPP Path #2
                     
            case 4:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPD Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + 4.0);delayQ.delay(1, 4.0); break; //MPD Path #2
         
          }
      }
    if (intMultipaths == 4)
      {
        switch (intMode)
          {
            case 0:   delayI.delay(0, fltMinimumIDelayMs);  //Minimum I0 ...no other paths.(WGN)
                      delayQ.delay(0, 0.0); break;
                
            case 1:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPG Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + .25);delayQ.delay(1, .25); //MPG Path #2
                      delayI.delay(2, fltMinimumIDelayMs + .5);delayQ.delay(2, .5);   //MPG Path #3 
                      delayI.delay(3, fltMinimumIDelayMs + .75);delayQ.delay(3, .75); break; //MPG Path #4
                                            
            case 2:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPM Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + .5);delayQ.delay(1, .5); //MPM Path #2
                      delayI.delay(2, fltMinimumIDelayMs + 1.0);delayQ.delay(2, 1.0);   //MPM Path #3 
                      delayI.delay(3, fltMinimumIDelayMs + 1.5);delayQ.delay(3, 1.5); break; //MPM Path #4
                
            case 3:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0,0.0);   //MPP Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + 1.0);delayQ.delay(1, 1.0); //MPP Path #2
                      delayI.delay(2, fltMinimumIDelayMs + 2.0);delayQ.delay(2, 2.0);   //MPP Path #3 
                      delayI.delay(3, fltMinimumIDelayMs + 3.0);delayQ.delay(3, 3.0); break; //MPP Path #4
      
            case 4:   delayI.delay(0, fltMinimumIDelayMs);delayQ.delay(0, 0.0);   //MPD Path #1 
                      delayI.delay(1, fltMinimumIDelayMs + 2.0);delayQ.delay(1, 2.0); //MPD Path #2
                      delayI.delay(2, fltMinimumIDelayMs + 4.0);delayQ.delay(2, 4.0);   //MPD Path #3 
                      delayI.delay(3, fltMinimumIDelayMs + 6.0);delayQ.delay(3, 6.0); break; //MPD Path #4
        }
      }
    Serial.print("Line 1014 SetIQTapDelays: intMode = ");Serial.print(intMode); Serial.print("  Paths:"); Serial.println(intMultipaths);
} // End SetIQTapDelays **************************************************************************

//******Function to Parse Simulator Mode received via Serial Port *********************************
int ParseSimMode (String strMode)
{
  // Returns mode index (0-15) if strMode is legitimate, -1 other wise (case insensitive)
  for (int i=0; i < (intNumModes-3); i++) //Search through the available Modes and Parameters TEST3K, TEST6K not used. 
  {
    String strTemp = chrModes[i];
    if  (strTemp.startsWith(strMode.toUpperCase()))
      {
        if (strTemp.startsWith("BUSY:"))
          {
            blnSim = false; //blnInitialized = false;  blnEnableTestTone = false;
          }
      return i;
    }
  }
  return -1;
}// End of ParseSimMode ************************************************************************

//******Function to Parse Busy Mode received via Serial Port **************************************
int ParseBusyMode (String strMode)
{
  // Returns mode index (0-10) if strMode is legitimate, -1 other wise (case insensitive)
  for (int i = 0; i < intNumBusyModes; i++) //Search through the available Busy Modes and Parameters
  {
    String strTemp = chrBusyModes[i];
    if  (strTemp.startsWith(strMode.toUpperCase()))
    {
      if (strTemp.startsWith("SIM:"))
      {
        Serial.println("OK");
        blnSim = true;  blnInitialized = false;  blnEnableTestTone = false; blnInitializedFromEEPROM = false;
        blnPlotSpectrum = false; blnInitSpectrum = false; 
        setup();
      }
      return i;
    }
  }
  return -1;
}// End of ParseBusyMode ****************************************************************************

//*******Function to Parse and Set Simulation Parameter received via Serial Port **********************
boolean ParseSetSimParameter(String strParameter, int intMode)
// Determins if strParameter is compatible with intMode. if not returns false
// If compatible sets the parameter and Mode, Updates display showing mode and parameter and returns true
{
  int intParam = strParameter.toInt(); String str1; String str2; String str3; float fltParam = strParameter.toFloat();
  //Serial.print("Line# 915: ParseSetSimParameter: intParam = ");Serial.print(intParam);Serial.print("  fltParam= ");Serial.println(fltParam);
  if (intMode < 5)//WGN thru MPD
    {
      intParam = strParameter.toInt();
      if ((-40 <= intParam) && (intParam <= 40))
        {
          intTargetSN = intParam;
          
          return true;
        }
     }
  if (intMode == 5)//INTMULTIPATHS
    {
      intParam = strParameter.toInt();
      if ((intParam == 2) || (intParam == 4))
        { intMultipaths = intParam; return true; }
      else {return false;}
    }
  if (intMode == 6)//FADE DEPTH
  {
    if ((intParam >= 0) && (intParam <= 40))
    {
      intFadeDepth_dB = intParam;
      return true;
    }
  }
  if (intMode == 7)//FADE FREQ  fltFadeRates
    { 
      for (int j = 0; j < 9; j ++)
        if  ((abs(10 *fltLogs[j] - fltParam)) < .001)
          {
            intFadeRatePtr  = j;
            fltFadeRate = 10 * fltLogs[intFadeRatePtr];
            return true;
          }
    }
  if (intMode == 8)//OFFSET
    {
      if ((intParam >= -200) && (intParam <= 200))
        {
          intTuneOffset = intParam;
          return true;
        }
    }
  if (intMode == 9)// FM DEVIATION  
    {
      for (int j = 0; j < 9; j ++)
        if  ((abs(100 * fltLogs[j] - fltParam)) < .001)
          {
            intFMDevPtr = j;
            if (fltLogs[intFMDevPtr] <.01){sine_VLF_Dnmix_Mod.amplitude(0.0);}  
            else {sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .0129870129870 );} //sets max deviation in Hz e.g. .000129870129870 * 7700  yields +/- 1 Hz peak deviation
            return true;
          }
    }
  if (intMode == 10)//FM RATE 
    {
      for (int j = 0; j < 9; j ++)
        if ((abs(10 * fltLogs[j] - fltParam)) < .001)
          {
            intFMRatePtr = j;
            if (intFMRatePtr == 0){sine_VLF_Dnmix_Mod.amplitude(0);}
            else
              {
                sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .0129870129870 ); //sets max deviation in Hz e.g. .0129870129870 * 7700  yields +/- 1 Hz peak deviation
                sine_VLF_Dnmix_Mod.frequency(10 * fltLogs[intFMRatePtr]);// Rate is .1 to 20 Hz
              }
            return true;
          }
      }
  if ((intMode >= 11) && (intMode <= 12)) //IN GAINS
    {
      for (int j = 0; j < 9; j ++)
        {
          if  ((abs(100 * fltLogs[j] - fltParam)) < .001)
            {
              intGainLevel[intMode - 11] = j;
              mixInpSel.gain(intMode -9, 100 * fltLogs[j]);
              mixInpSel.gain((intMode - 9), (100 * fltLogs[intGainLevel[intMode - 11]])); //sets input mixer gain (0-100 (log))
              return true;
            } 
        }  
     } 
          
  if ((intMode >= 13) && (intMode <= 14)) //OUT levels
    {
      for (int j = 0; j < 9; j ++)
        {
          if  (abs(fltLogs[j] - fltParam) < .001)
            {
              intGainLevel[intMode - 11] = j;
              if (intMode == 13){ampLeftOut.gain(fltLogs[ intGainLevel[intMode - 11]]); }//sets ampLeftOut gain (0-2) (log))
              if (intMode == 14){ampRightOut.gain(fltLogs[ intGainLevel[intMode - 11]]); }//sets ampRightOut gain (0-2) (log))
              return true;
            } 
        }  
     }   
      
  if (intMode == 15)//Bandwidth
    {
      intParam = strParameter.toInt();
      if ((intParam == 3000) || (intParam == 6000))
        {
          SetFilterBandwidth(intParam);
          intBandwidth = intParam;
          return true;
        }
      else {return false;}
    }  // Serial baud rate selection not implemented via serial port.
  return false;
}// End ParseSetSimParameter *********************************************************************

//*******Function to Parse and Set Busy Parameter received via Serial Port **********************
boolean ParseSetBusyParameter(String strParameter, int intMode)
// Determins if strParameter is compatible with intMode. if not returns false
// If compatible sets the parameter and Mode, Updates display showing mode and parameter and returns true
{
  int intParam = strParameter.toInt(); String str1; String str2; String str3; 
  if (intMode == 0)//ENB BUSY
  {
    intParam = strParameter.toInt();
    if ((1 <= intParam) && (intParam <= 50))
    {
      intAvg = intParam;
      str1 = chrBusyModes[intMode];  str2 = "    AVG= " + String(intAvg); str3 = "";
      UpdateTFTModeParameter(str1, str2, str3);
      blnEnbBusyDetect = true;
      return true;
    }
  }
  if (intMode == 1)//DIS BUSY
  {
    intParam = strParameter.toInt();
    if ( intParam == 0)
    {
      str1 = chrBusyModes[intMode];  str2 = ""; str3 = "";
      UpdateTFTModeParameter(str1, str2, str3);
      blnEnbBusyDetect = false;
      return true;
    }
  }
  if (intMode == 2)//LOW FREQ
  {
    intParam = strParameter.toInt();
    if (( intParam >= 43) && (intParam <= 5000) && (intParam < (intSpecHigh - 259)))
    {
      str1 = chrBusyModes[intMode];  str2 = "    " + String(intParam); str3 = " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      intBusyBWLoHz = intParam; ; return true;
    }
  }
  if (intMode == 3)//HIGH FREQ
  {
    if (( intParam >= 500) && (intParam <= 6000) && (intParam > (intSpecLow + 259)))
    {
      str1 = chrBusyModes[intMode];  str2 = "    " + String(intParam); str3 = " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      intBusyBWHiHz = intParam; ; return true;
    }
  }
  if (intMode == 4)//THRES (dB)
  {
    intParam = strParameter.toInt();
    if (( intParam >= 3) && (intParam <= 40))
    {
      str1 = chrBusyModes[intMode];  str2 = "    " + String(intParam); str3 = " dB";
      UpdateTFTModeParameter(str1, str2, str3);
      intThresh = intParam;
      return true;
    }
  }
  if (intMode == 5)//TONE ON (Hz)
  {
    intParam = strParameter.toInt();
    if (( intParam >= 43) && (intParam <= 6300))
    {
      str1 = chrBusyModes[intMode];  str2 = "    " + String(intParam); str3 = " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      InputTestWaveform.frequency (intParam); InputTestWaveform.amplitude(.251880522);
      mixInpSel.gain(0, 2); mixInpSel.gain(1, 0); mixInpSel.gain(2, 0); mixInpSel.gain(3, 0);
      return true;
    }
  }
  if (intMode == 6)//TONE OFF (Hz)
  {
    intParam = strParameter.toInt();
    if ( intParam == 0)
    {
      str1 = chrBusyModes[intMode];  str2 = ""; str3 = "";
      UpdateTFTModeParameter(str1, str2, str3);
      mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intGainLevel[0]]); mixInpSel.gain(3, 0);
      return true;
    }
  }
  if (intMode == 7)//CH1 IN
  {
    intParam = strParameter.toInt();
    if (( intParam >= 0) && (intParam <= 10))
    {
      str1 = chrBusyModes[intMode];  str2 = "     " + String(fltLogs[intParam]); str3 = "";
      UpdateTFTModeParameter(str1, str2, str3);
      intGainLevel[0] = intParam;
      //Note following sets gain level ch 1 and forces ch 2 gain = 0.
      mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intParam]); mixInpSel.gain(3, 0);
      return true;
    }
  }
  if (intMode == 8)//BANDWIDTH: 3000/6000
  {
    intParam = strParameter.toInt();
    if ( (intParam == 3000) || (intParam == 6000) )
    {
      str1 = chrBusyModes[intMode];  str2 = "    " + String(intParam); str3 = " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      SetFilterBandwidth(intParam);
      intBandwidth = intParam;
      blnInitSpectrum = true;
      return true;
    }
  }
  if (intMode == 9)//SPECTRUM:1/0
  {
    intParam = strParameter.toInt();
    if (!((intParam == 0) || (intParam == 1))) {
      return false;
    }
    if (intParam == 0) {
      str3 = "    OFF";
    };
    if (intParam == 1) {
      str3 = "    ON";
    };
    str1 = chrBusyModes[intMode];  str2 = "";
    UpdateTFTModeParameter(str1, str2, str3);
    if (intParam == 1) {
      blnPlotSpectrum = true;
      blnInitSpectrum = true;
      return true;
    }
    if (intParam == 0) {
      blnPlotSpectrum = false;
      blnInitSpectrum = false;
      return true;
    }
    return false;
  }
  return false;
}// End ParseSetBusyParameter *******************************************************************

//**** Subroutine UpdateTFTModeParameter**********************************************************
void UpdateTFTModeParameter(String s1, String s2, String s3)
{
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(s1);
  tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(s2 + s3);
}// End UpdateTFTModeParameter *******************************************************************

// **** Subroutine to  AvgerageFFTBins *********************************************************
void AvgFFTBins(boolean blnInit, float fltKavg, int intLoFHz, int intHiFHz)
// 3/25/2020 Good results for fltKavg = .1, averaging every 20 bins (~220ms)
// 3/26/2020 Improved with using 2x fltKavag on attack or .5x fltKavg on release
{
  float fltTemp = 0; int intHiBin = intHiFHz / 43.0664; int intLoBin = intLoFHz / 43.0664;
  float fltAvgBinSum = 0.0;

  if (blnInit)//Initialize the average bins
  {

    for (int i = 1; i < 151; i++) //Skip bin 0 (DC component)
    {
      fltFFT[i] = 4000.0 * fft1024.read(i);
      if ((i >= intLoBin) && (i <= intHiBin)) {
        fltAvgBinSum += fltFFT[i];
      }
    }
    fltAvgBinsOfInt = fltAvgBinSum / (1 + intHiBin - intLoBin); //Initialize fltAvgBinsOfInt on first call with blnInit = true
    return;
  }
  else//Average in new values with fast attack, slow release Rolling Avg Filter
  {
    for (int i = 1; i < 151; i++)// Modified 3/26/2020 to use fast attack slow release  Rolling Avg Filter
      // This definitely helps by "holding" the busy condition for a while following an indication.
    {
      fltTemp = 4000.0 * fft1024.read(i);
      if ((i > intHiBin) || (i < intLoBin))
      { //Normal = attack and release for bins outside the band of interest
        fltFFT[i] = ((1.0 - ( fltKavg)) * fltFFT[i]) + ( fltKavg *  fltTemp);
      }
      else if (fltTemp > fltFFT[i])//Faster attack using 2x fltKavg
      {
        fltFFT[i] = ((1.0 - (2 * fltKavg)) * fltFFT[i]) + (2 * fltKavg * fltTemp);
      }
      else // Slower release using .5x fltKavavg
      {
        fltFFT[i] = ((1.0 - (.2 * fltKavg)) * fltFFT[i]) + (.2 * fltKavg * fltTemp);
      }
    }
    return;
  }
}/// End Sub AvgFFTBins ******************************************************************************

//******** Sub SearchRatioDetect*************Used in Busy detector ************************************
//Status: Works well for both narrow and wide  band signal detection...Still optimizing********
int SearchRatioDetect(int intLowFHz, int intHiFHz, int intSearchWidthHz, int intThreshdB)
{
  int intLowBin = round(intLowFHz / 43.0664); int intHiBin = round(intHiFHz / 43.0664);int intIatMax;int intIatMin;
  int intSearchWidthBins = round(intSearchWidthHz / 43.0664); int intMaxS_NdB; int intMinS_NdB;
  float fltSearchSum = 0.0; float fltSearchSumMax = 0.0; float fltSearchSumMin = 1000000;
  float fltHi_LowSum = 0.0; float fltDenom = 0.0; float fltNum = 0.0; 

  // Search for largest AND smallest  "intSearchWidthHz" signal (between intLowBin and intHighBin inclusive)
  for (int i = intLowBin; i <= (intHiBin); i++)
  {
    fltHi_LowSum = fltHi_LowSum + fltFFT[i];//Sums all the bins between intLowBin and intHighBin inclusive
    if (i <= (intHiBin - intSearchWidthBins)) //This bounds the search to the region of interest
    {
      fltSearchSum = 0;// Search for the max and min contiguous bins of width intSearchWidthBins
      for (int j = 0; j < intSearchWidthBins; j++)
      {
        fltSearchSum += fltFFT[i + j];// Sum all the bins within intSearchWidthBins
      }
      if (fltSearchSum > fltSearchSumMax)
        {
          fltSearchSumMax = fltSearchSum;//The max sum found in a range of intSearchWidth
          intIatMax = i;
        }
      if (fltSearchSum < fltSearchSumMin)
        {
          fltSearchSumMin = fltSearchSum; //The minimum sum found in a range of intSearchWidth
          intIatMin = i;
        }
    }
  }
  // Generate Numerator and Denominator normalized to per bin for the Highest S:N
  fltDenom = (fltHi_LowSum - fltSearchSumMax) / ((intHiBin - intLowBin) - intSearchWidthBins); //Avg energy/bin  outside region of max
  fltNum = fltSearchSumMax / intSearchWidthBins; //Maximum Energy per bin over intSearchWidthHz between intLowBin and intHighBin
  intMaxS_NdB = round(20 * log10(fltNum / fltDenom));
  if (intMaxS_NdB >= intThreshdB)//This trips detector when a narrow band peak is found
  {
    blnPlotBusyRed = true;
    return intMaxS_NdB;
  }

  // Generate the Numerator and Denominator normalized to per bin for the Lowest S:N
  fltDenom = (fltHi_LowSum - fltSearchSumMin) / ((intHiBin - intLowBin) - intSearchWidthBins); //Avg energy/bin  outside region of max
  fltNum = fltSearchSumMin / intSearchWidthBins; //Minimum Energy per bin over intSearchWidthHz between intLowBin and intHighBin
  intMinS_NdB = round(20 * log10(fltNum / fltDenom)); //Typically -3 to -10
  if ((intMaxS_NdB - intMinS_NdB) >= (2 * intThreshdB))//This trips the detector when a narrow band minimum is found
  {
    blnPlotBusyRed = true;
    return (intMaxS_NdB - intMinS_NdB);
  }

  else if (intMinS_NdB <= (intThreshdB - 2)) //Only clear the busy condition when intMinS_NdB is 2 dB or more BELOW threshold
  {
    blnPlotBusyRed = false;
    return 0;
  }
  else return intMaxS_NdB;
}//End************* SearchRatioDetect *********************************************************

// Subroutine to Save ParametersToEEProm *******************************************************************
void SaveParametersToEEPROM()
{ // Only called from manual Right Encoder Push While displaying modes 5-16
  int intAdd = intMode - 4;
  //Serial.print("EEPROM SAVE: mode =");Serial.print(intMode);Serial.print(" val= ");
  //Modes 0-4 (WGN-MPD) not saved as Right Encoder Push toggles 1500 Hz tone on/off.  So S:N values not saved/restored.
  // These confirmed based on Serial.print (remove print statements after debug).
  if (intMode == 5) {
    EEPROM.write(intAdd, byte(intMultipaths));  //  intMultipaths
    //Serial.println(byte(intMultipaths));
  }
  if (intMode == 6) {
    EEPROM.write(intAdd, byte(intFadeDepth_dB));  //Fade Depth 0 to 40 dB
    //Serial.println(intFadeDepth_dB);
  }
  if (intMode == 7) {
    EEPROM.write(intAdd, byte(intFadeRatePtr));  //Fade Rate
    //Serial.println(intFadeRatePtr);
  }
  if (intMode == 8) {
    EEPROM.write(intAdd, byte(20 + intTuneOffset / 10));  //intTuneOffset
    //Serial.println(20 + intTuneOffset / 10);
  }

  if (intMode == 9) {
    EEPROM.write(intAdd, byte(intFMDevPtr));  //FMDeviation Pointer
   //Serial.println(intFMDevPtr);
  }
  if (intMode == 10) {
    EEPROM.write(intAdd, byte(intFMRatePtr));  //FM Rate Pointer
    //Serial.println(intFMRatePtr);
  }
  if ((intMode > 10) && (intMode < 15)) {
    EEPROM.write(intAdd, byte(intGainLevel[intMode - 11]));  // intGainIndex
    //Serial.println(intGainLevel[intMode - 11]);
  }
  if (intMode == 15) {
    EEPROM.write(intAdd, byte(intBandwidth / 100));  //  Bandwidth 3000 or 6000 / 100
    //Serial.println(byte(intBandwidth / 100));
  }
  if (intMode == 16) {
    EEPROM.write(intAdd, byte(intBaudPtr));  //intBaudPtr
    //Serial.println(intBaudRate[intBaudPtr]);
  }  
  blnInitializedFromEEPROM = true; return;
}  //  End SaveParametersToEEPROM ***********************************************************************

// ******* Subroutine to InitializeParametersFromEEProm **************************************************
void InitializeParametersFromEEPROM()
{ // Only called from Setup upon initial power on or Reset
  byte bytParameter;
  if (!blnSim) {
    return;
  }
  if (blnInitializedFromEEPROM) { return;} //Exit if already initialized
  for (int i = 1; i <= 12; i++) //Read 12 initialization values and verify in range
  {
    bytParameter = EEPROM.read(i);
    //Serial.print("Init From EEPROM(");Serial.print(i);Serial.print(") = ");Serial.println(bytParameter);//For debug
    if ((i == 1) && ((bytParameter ==2) || (bytParameter ==4))) {intMultipaths = int(bytParameter) ;}
    else if ((i == 2) && (bytParameter < 41)) { intFadeDepth_dB = int(bytParameter) ;}
    else if ((i == 3) && (bytParameter < 9)) {intFadeRatePtr = int(bytParameter);}
    else if ((i == 4) && (bytParameter < 41)) {intTuneOffset = 10 * (int(bytParameter) - 20);}
    else if ((i == 5) && (bytParameter < 9)) {intFMDevPtr = int(bytParameter);}
    else if ((i == 6) && (bytParameter < 9)) {intFMRatePtr = int(bytParameter);}
    else if (((i > 6) && (i < 11)) && (bytParameter < 9)) {intGainLevel[i - 7] = int(bytParameter);}
    else if ((i == 11) && ((bytParameter == 30) || (bytParameter == 60))) {intBandwidth = 100 * int(bytParameter);} 
    else if ((i == 12) && (bytParameter <7))  {intBaudPtr =  int(bytParameter);}
   }
  //Serial.println("Init from EEPROM Complete");
  blnInitializedFromEEPROM = true; return;
}  //  End InitializeParametersFromEEPROM ****************************************************************

//********Subroutine Setup *********************************************************************************
void setup()
{
  pinMode(ENC1Button, INPUT_PULLUP);   //#define ENC1Button 1
  debouncer1.attach(ENC1Button);//Right Encoder button
  debouncer1.interval(10);
  pinMode(ENC2Button, INPUT_PULLUP);   //#define ENC1Button 2
  debouncer2.attach(ENC2Button);// Left Encoder Button1
  debouncer2.interval(10);
  Serial.end(); Serial.begin(intBaudRate[intBaudPtr]);

// Pins for Rotary Encoders RE1(modes) and RE2(parameters)
  pinMode(0, INPUT_PULLUP);//RENC1a
  pinMode(1, INPUT_PULLUP);//RENC1b
  pinMode(3, INPUT_PULLUP);//RENC2a
  pinMode(4, INPUT_PULLUP);//RENC2b
  sgtl5000_1.enable(); sgtl5000_1.volume(.2); // keep volume low for headphones
  AudioMemory(50);
  delayI.delay(7,11.3605); //set unused tap #7 1 ms longer than needed 1.3605 + 10)  to keep from dynamically re allocating memory
  delayQ.delay(7,9.0);//set unused tap #7 1 ms longer than needed 1 + 8)  to keep from dynamically allocating memory
  whiteOut.amplitude(.6452); //Set white noise level to yield ~235mv rms (3KHz bandwidth) or ~313mv rms (6KHz bandwidth)
  fft1024.windowFunction(AudioWindowHanning1024); // FFT Testing Default Window is Hanning
 
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(4);  tft.println("IONOS SIM      ");
  tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(strRevision);
  //this prints out a 8 digit hex SN unique to the Specific teensy  
  tft.setTextColor(ILI9341_MAGENTA);tft.setCursor(0,180);tft.print(" Serial: "); tft.printf("%08X",OCOTP_CFG0);
  delay(4000);
  Serial.print("IONOS SIM ");Serial.print(strRevision); Serial.print(" Serial: ");Serial.printf("%08X",OCOTP_CFG0);Serial.println("");
  //This sets up all non static variables as the would be upon power on to eanble clean startup from SIM: command
  strMode = ""; strParameter= "";
  
  blnSim = true;  blnInitialized = false; blnEnableTestTone = false; blnPlotSpectrum = false; blnInitSpectrum = false; 
  blnModes = true; blnInitModes = true; blnColon = false;  blnChanBusySent = false;  blnChanClearSent = false; blnPlotBusyRed = false;  blnEnbBusyDetect = false;
  blnTestMode3K = false; blnTestMode6K = false; blnInitializedFromEEPROM = false;

  ulngLastSpectrumUpdateMs = millis(); 

  lngENC1Old = 0; lngENC2Old = 0; lngENC1New = 0; lngENC2New = 0; 

  intA3history = 0;  intTargetSN = 40; intBandwidth = 3000; intMode = 0; intFadeRatePtr = 0; intFMRatePtr = 0; intFMDevPtr = 0;intFadeDepth_dB = 0;
  intTuneOffset = 0; intGainLevel[0] = 1; intGainLevel[1] = 1; intGainLevel[2]= 7; intGainLevel[3]= 7;  
  intDelayToUpdate = 0; intFFTCnt = 0; intBusyMode = 0; intBinsAveraged = 0; intNumofBinsToPlot = 0;
  intBusyFreqStep = 100; intBusyBWLoHz = 0; intBusyBWHiHz = 6500;  intBusyGain = 10; intLastIQPlaneMode = -1; intDetectSN = 0; intSerialCmdMode = -1; 
  intSerialCmdParam = -1; intAvg = 10; intSpecLow = 0; intSpecHigh = 6500; intThresh = 10; intTestFreqHz = 1500; intDelayCount = 0; intMultipaths = 2;
  fltVLF_UpFreq = 100; fltVLF_Amp = 0; fltFadeRate = 0;  
    fltPeakSum = 0; fltNoiseSum = 0; fltMax = 0; fltAvg = 0; fltAvgBinsOfInt = 0.0;
  fltppi2s0InAvg = 0.0; fltppi2s1InAvg = 0.0; fltppAmpRightOutAvg = 0.0; fltppAmpLeftOutAvg = 0.0;
// End of basic initialization.
 InitializeParametersFromEEPROM();delay(1000);
  SetFilterBandwidth(intBandwidth);
 whiteOut.amplitude(.6452); //Set white noise level to yield 150 mv rms
// Initialize the tap coefficients for the 119 tap Hilbert Filter 
  for (int i = 0; i < 120; i ++) // for 120 taps including forced 0 end tap to make number of taps even 
    {  // Reverse order of coefficients for Teensy FIR (put negative values first) FIR will reverse order
       shtHilbertFIR_120TapCoeff[119-i] = dblHilbertFIR_IowaHills_119TapsCoeff[i] * 33702; //scale by 33702 To set same level into delayQ as delayI
       //Serial.print("120TapCoeff[");Serial.print(119-i);Serial.print("] = ");Serial.println(shtHilbertFIR_120TapCoeff[119-i]); 
     }
  
  // Initialize the Hilbert FIR. A single 6.5 KHz Hilbert 120 tap filter for both 3KHz and 6KHz bandwidths.
  filHilQ_1234.end();
  filHilQ_1234.begin(shtHilbertFIR_120TapCoeff, 120); // Hilbert FIR filter for Ch1,2,3,4 Q paths
  SetFilterBandwidth(intBandwidth);
  filHP7500FIR.end(); sine_Dnmix.frequency(7700); sine_Dnmix.amplitude(1.0); sine_Upmix.frequency(7700); sine_Upmix.amplitude(1.0);
  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
  sine_VLF_Dnmix_Mod.amplitude(0);sine_Upmix.frequency(7700); sine_Upmix.amplitude(1.0);
  sine_Dnmix.frequency(7700.0- intTuneOffset);sine_Dnmix.amplitude(1.0);
  ampRightOut.gain(fltLogs[intGainLevel[3]]); ampLeftOut.gain(fltLogs[intGainLevel[2]]);// default gain (half scale)[range = 0 to 2]
  SetIQTapDelays(intMode);
  mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, 100 * fltLogs[intGainLevel[0]]); mixInpSel.gain(3, 100 * fltLogs[intGainLevel[1]]);
  ulngCurrentElapsedTimeUs = micros(); ulngLastDelayUpdateUs = ulngCurrentElapsedTimeUs; ulngLastPPAvgTimeUs = ulngCurrentElapsedTimeUs;
 // mixI2Q2.gain(0, .5); mixI2Q2.gain(1, .5); mixI2Q2.gain(2, 0.0); mixI2Q2.gain(3, 0.0);// Set gains for IQ power mixer
  
  } // End Subroutine Setup ***********************************************************************************

// InitializeBusy *******************************************************************************************
void InitializeBusy()
{
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(4);  tft.println("BUSY DETECT  ");
  tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(strRevision);
  ampRightOut.gain(fltLogs[intGainLevel[3]]); 
  delay(1000);
  fltCurrentDelayI1Ms = fltMinimumIDelayMs; 
  mixIQ12.gain(0, 0); mixIQ12.gain(1, .8498); mixIQ12.gain(2, 0); mixIQ12.gain(3,0);
  mixPathSel.gain(0, .5);mixPathSel.gain(1, 0.0);;mixPathSel.gain(2, 0.0);;mixPathSel.gain(3, 0.0);
  
  filLPWhiteFIR.begin(sht3KHzLPFIRCoeffRev2,120); // White Noise LP Filter
  intFMRatePtr = 1;
  fltFadeRate = 1;
  InputTestWaveform.frequency (1500); InputTestWaveform.amplitude(.5);
  SetFilterBandwidth(intBandwidth);

} //End InitializeBusy ****************************************************************************************

//   Main Loop***********************************************************************************************
void loop()
{
   String str1, str2, str3;
  //Code to update Rotary Encoder 2 (modes)
  lngENC2New = ENC2.read();//Encoder 2 controls Modes (Left hand knob)
  
  if ((lngENC2Old != lngENC2New) || (! blnInitialized))
    {
      if (blnSim)
        {
          intMode = intMode + (lngENC2New - lngENC2Old);
          if (intMode > 18) {intMode = 18;} //Hold at limit
          if (intMode < 0 ){intMode = 0;}
          if (intMode < 17)
            {
              blnTestMode3K = false; blnTestMode6K = false;  blnEnableTestTone = false;
              InputTestWaveform.amplitude(0);
              mixInpSel.gain(0, 0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 100 * fltLogs[intGainLevel[0]]); mixInpSel.gain(3, 100 * fltLogs[intGainLevel[1]]);
              ampLeftOut.gain(fltLogs[intGainLevel[2]]);//sets ampLeftOut gain (0-2 (log))
              ampRightOut.gain(fltLogs[intGainLevel[3]]);//sets ampRightOut gain (0-2 (log))
              if ( ((intMode >= 1)&& (intMode <=4))  || ((intMode == 0) && (intTuneOffset == 0) && (intFMDevPtr ==0)) )
                {// Use Direct path (no Up/Down mixing
                  if (intMode == 0){mixIQ12.gain(0,0.0);mixIQ12.gain(1,.8498);mixIQ12.gain(2,0.0);mixIQ12.gain(3,0.0);};
                  // If mode != 0 above, mixIQ.gains will be set by calls to QuadGaussFIR128 @ 64 x the doppler rate
                  SetIQTapDelays(intMode);
                  mixPathSel.gain(0, .5);mixPathSel.gain(1, 0.0);;mixPathSel.gain(2, 0.0);;mixPathSel.gain(3, 0.0);
                  SetIQTapDelays(intMode);
                  blnModes = true;
                }
              else if ((intMode == 0 ) && (( intTuneOffset != 0) || (intFMDevPtr !=0)))
                {// Use Up/Down mixing
                  SetIQTapDelays(intMode);
                  mixIQ12.gain(0,0.0);mixIQ12.gain(1,.8498);mixIQ12.gain(2,0.0);mixIQ12.gain(3,0.0);SetIQTapDelays(intMode);
                  mixPathSel.gain(0, 0.0);mixPathSel.gain(1, 2.0819);;mixPathSel.gain(2, 0.0);;mixPathSel.gain(3, 0.0);
                  // Value 2.0819 calibrates mixUpDn Path gain = to direct path gain.
                  blnModes = true;
                }
              }
          if ((intMode >= 1) && (intMode <= 4))
            {
             SetIQTapDelays(intMode);
              blnModes = true; 
            }
        }
      tft.begin();
      tft.setRotation(1);
      tft.fillScreen(ILI9341_BLACK);
      if ((intMode == 0) || (! blnInitialized))//WGN
        {
          intCountrmsMixIQ1234Out = 0;
          SetIQTapDelays(intMode);
          mixIQ12.gain(0,0); mixIQ12.gain(1,.8498); mixIQ12.gain(2,0.0);mixIQ12.gain(3,0.0);
          mixIQ1234.gain(0, 1.0); mixIQ1234.gain(1, 0.0); mixIQ1234.gain(2, 0.0);mixIQ1234.gain(3, 0.0);
          //Serial.print("Line 1504: intMode = ");Serial.println(intMode);
          str1 = chrModes[intMode];  str2 = "    S:N= "; str3 = " dB";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTargetSN + str3);
        }
      if ((intMode > 0) && (intMode <= 4))//MPG,MPM,MPP,MPD
        {
          intCountrmsMixIQ1234Out =0.0; 
          SetIQTapDelays(intMode);
          if (intMultipaths == 2)
            { 
              mixIQ1234.gain(0,1.0); mixIQ1234.gain(1,0.0);mixIQ1234.gain(2,0.0);mixIQ1234.gain(3,0.0);
            }
           if (intMultipaths == 4)
            { // Ch 0 and 1 gains need calibration for 4 path
              mixIQ1234.gain(0,1.0); mixIQ1234.gain(1,1.0);mixIQ1234.gain(2,0.0);mixIQ1234.gain(3,0.0);
            }  
          Serial.print("Line 1651: intMode = ");Serial.println(intMode);
          str1 = chrModes[intMode];  str2 = "    S:N= "; str3 = " dB";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTargetSN + str3);
          if (( intTuneOffset != 0) || (intFMDevPtr !=0))
            {
              tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);  tft.println("    (no FM or Offset!)");
            }
        }
      if (intMode == 5)//MULTIPATHS
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = "";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intMultipaths);
        }
      if (intMode == 6) //FADE Depth
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " dB";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intFadeDepth_dB + str3);
        }
      if (intMode == 7) // FADE Freq
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(10 * fltLogs[intFadeRatePtr]) + str3);
        }
      if (intMode == 8) //Offset
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTuneOffset + str3);
        }
      if (intMode == 9) //FM Deviation
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(100 * fltLogs[intFMDevPtr]) + str3);
        }
      if (intMode == 10) //FM Rate
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(10 * fltLogs[intFMRatePtr]) + str3);
        }

      if ((intMode >= 11) && (intMode < 15)) //Ch1, Ch2, IN;Out
        {
          str1 = chrModes[intMode];  str2 = "    ";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3); 
          if (intMode < 13) {tft.println(str2 +String(100 * fltLogs[intGainLevel[ intMode - 11]]));}// Scale up input fltLogs by 100}
          else {tft.println(str2 +String( fltLogs[intGainLevel[ intMode - 11]]));}// Scale output fltLogs by 1
        }

      if (intMode == 15)//BANDWIDTH
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intBandwidth + str3);
        }

      if (intMode == 16)//Baud Rate
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Baud";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intBaudRate[intBaudPtr] + str3);  
          Serial.end(); Serial.begin(intBaudRate[intBaudPtr]);
        }  
      if (intMode == 17)//TEST3K
        {
          blnTestMode3K = true; blnTestMode6K = false; intTestFreqHz = 1500; intTargetSN = 40;
          // Set gains to over ride other mode settings:
          ampLeftOut.gain(2.0); ampRightOut.gain(1.0);
          mixInpSel.gain(0, 2.0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
          mixIQ12.gain(0, 0.0); mixIQ12.gain(1, 4.0); mixIQ12.gain(2, 0.0); mixIQ12.gain(3, 0.0);// 
          
          if (!(intBandwidth == 3000)); intBandwidth = 3000; SetFilterBandwidth(intBandwidth);
          sine_VLF_Dnmix_Mod.amplitude(0);
          InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
        }
      if (intMode == 18)//TEST6K
        {
          blnTestMode6K = true; blnTestMode3K = false; intTestFreqHz = 3000; intTargetSN = 40;
          // Set gains to over ride other mode settings:
          ampLeftOut.gain(1.0); ampRightOut.gain(2.0);
          mixInpSel.gain(0, 2.00); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
          mixIQ12.gain(0, 0.0); mixIQ12.gain(1, 4.0); mixIQ12.gain(2, 0.0); mixIQ12.gain(3, 0.0);//
          sine_VLF_Dnmix_Mod.amplitude(0);
          if (!(intBandwidth == 6000)); intBandwidth = 6000; SetFilterBandwidth(intBandwidth);
          InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
        }
       
      blnInitialized = true;
      blnInitSpectrum = true;
      lngENC2Old = lngENC2New;
    }
  

  // Code to update Rotary encoder 1 (parameters,Right hand knob)
  lngENC1New = ENC1.read();
  if ((lngENC1Old != lngENC1New) || blnInitModes)
    {
      blnInitModes = false; blnInitSpectrum  = true;
      if (blnSim)// code for SIM
        {
          tft.begin();
          tft.setRotation(1);
          tft.fillScreen(ILI9341_BLACK);
          if ((intMode >= 0) && (intMode <= 4))
            {
              if (intMode == 0) //(WGN)
                {
                  mixIQ12.gain(0,0.0);mixIQ12.gain(1,.8498);mixIQ12.gain(2,0.0);mixIQ12.gain(3,0.0);SetIQTapDelays(intMode);
                }
                
              else 
                {
                  SetIQTapDelays(intMode);
                }
                
              intTargetSN = intTargetSN + (lngENC1New - lngENC1Old);
              if (intTargetSN > intMaxSN) 
                {
                  intTargetSN = intMaxSN; //Hold at stops
                }
              if (intTargetSN < intMinSN) 
                {
                  intTargetSN = intMinSN;
                }
              str1 = chrModes[intMode];  str2 = "    S:N= "; str3 = " dB";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTargetSN + str3);
            }
          if (intMode == 5)//MULTIPATHS
            {
              
              intMultipaths = intMultipaths + 2 * (lngENC1New - lngENC1Old);
              if (intMultipaths <= 2) 
                {
                  intMultipaths =2;
                  mixIQ1234.gain(0,1.0); mixIQ1234.gain(1,0.0);mixIQ1234.gain(2,0.0);mixIQ1234.gain(3,0.0);
                }
              else if (intMultipaths >= 3)
                {
                  intMultipaths = 4;
                  mixIQ1234.gain(0,1.0); mixIQ1234.gain(1,1.0);mixIQ1234.gain(2,0.0);mixIQ1234.gain(3,0.0);
                 }
              Serial.print("Line 1799: intMultipaths ="); Serial.println(intMultipaths);
              str1 = chrModes[intMode]; str2 = "    " ; 
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3); tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intMultipaths);
             
            }
          if (intMode == 6)//FADE DEPTH
            {
              intFadeDepth_dB = intFadeDepth_dB +  (lngENC1New - lngENC1Old);
              if (intFadeDepth_dB > intMaxFade) 
                {
                  intFadeDepth_dB = intMaxFade; //Hold at stops
                }
              if (intFadeDepth_dB < 0) {intFadeDepth_dB = 0;}
              str1 = chrModes[intMode];   str2 = "    ";  str3 = " dB";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intFadeDepth_dB + str3);
            }
          if (intMode == 7)//FADE FREQ
            {
              intFadeRatePtr = intFadeRatePtr + (lngENC1New - lngENC1Old);
              if (intFadeRatePtr > 8) {intFadeRatePtr = 8;} //Hold at stops
              if  (intFadeRatePtr < 0) {intFadeRatePtr = 0;}
              fltFadeRate = 10 * fltLogs[intFadeRatePtr];// Range 0, .1 to 20 [Log]
              str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltFadeRate)  + str3);
            }
          if (intMode == 8)//OFFSET
            {
              intTuneOffset = intTuneOffset + (intOffsetStep * (lngENC1New - lngENC1Old));
              if (intTuneOffset > intMaxOffset) {intTuneOffset = intMaxOffset;} //Hold at stops
              if (intTuneOffset < intMinOffset) {intTuneOffset = intMinOffset;}
              sine_VLF_Dnmix_Mod.amplitude(0);sine_Dnmix.frequency(7700 - intTuneOffset); sine_Dnmix.amplitude(1.0); sine_Upmix.frequency(7700); sine_Upmix.amplitude(1.0);
              str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTuneOffset + str3);
            }
          if (intMode == 9) //FM DEVIATION
            {
              intFMDevPtr = intFMDevPtr + (lngENC1New - lngENC1Old);
              if (intFMDevPtr > 8) {intFMDevPtr = 8;} //Hold at stops
              if (intFMDevPtr < 0) {intFMDevPtr = 0;}
              if (fltLogs[intFMDevPtr] <.01){sine_VLF_Dnmix_Mod.amplitude(0.0);}  
              else {sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .0129870129870 );} //sets max deviation in Hz e.g. .000129870129870 * 7700  yields +/- 1 Hz peak deviation
              str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(100 * fltLogs[intFMDevPtr]) + str3);
            }
        if (intMode == 10)//FM RATE
          {
            intFMRatePtr = intFMRatePtr + (lngENC1New - lngENC1Old);
            if (intFMRatePtr > 8) {intFMRatePtr = 8;} //Hold at stops
            if (intFMRatePtr < 0) {intFMRatePtr = 0;}
            if (intFMRatePtr == 0){sine_VLF_Dnmix_Mod.amplitude(0);}
            else
              {
                sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .0129870129870 ); //sets max deviation in Hz e.g. .0129870129870 * 7700  yields +/- 1 Hz peak deviation
                sine_VLF_Dnmix_Mod.frequency(10 * fltLogs[intFMRatePtr]);// Rate is .1 to 20 Hz
              }
            str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(10 * fltLogs[intFMRatePtr]) + str3);
          }
        if ((intMode > 10) && (intMode < 13)) //Input GAINS: (0,1,2,5,10,20,50,100,200)[Nominal input range for 1000 mv p-p internal: 5 mv p-p to 1000 mv p-p]
          {
            intGainLevel[intMode - 11] = intGainLevel[intMode - 11] + (lngENC1New - lngENC1Old);
            if (intGainLevel[intMode - 11] > 8) { intGainLevel[intMode - 11] = 8;} //Hold at stops
            if (intGainLevel[intMode - 11] < 0 ) {intGainLevel[intMode - 11] = 0;}
            str1 = chrModes[intMode]; str2 = "    "; tft.setTextColor(ILI9341_CYAN);tft.setTextSize(3); tft.println(str1);
            tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3);
            tft.println(str2 +String(100 * fltLogs[intGainLevel[ intMode - 11]]));// Scale up fltLogs by 100 
            mixInpSel.gain((intMode - 9), (100 * fltLogs[intGainLevel[intMode - 11]])); //sets input mixer gain (0-100 (log))
            
          }
        else if  ((intMode > 12) && (intMode < 15)) //Output GAINS (0,.01,.02,.05,.1,.2,.5,1,2)Out mv p-p @ 1000 mv p-p internal: 10 mv p-p to 2000 mv p-p
          { 
            intGainLevel[intMode - 11] = intGainLevel[intMode - 11] + (lngENC1New - lngENC1Old);
            if (intGainLevel[intMode - 11] > 8) { intGainLevel[intMode - 11] = 8;} //Hold at stops
            if (intGainLevel[intMode - 11] < 0 ) {intGainLevel[intMode - 11] = 0;}
            if (intMode == 13){ampLeftOut.gain(fltLogs[ intGainLevel[intMode - 11]]); }//sets ampLeftOut gain (0-2) (log))
            if (intMode == 14){ampRightOut.gain(fltLogs[ intGainLevel[intMode - 11]]); }//sets ampRightOut gain (0-2) (log)) 
            str1 = chrModes[intMode]; str2 = "    " ;
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3); tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltLogs[intGainLevel[intMode - 11]]));
          }   
             
        if (intMode == 15)//BANDWIDTH
          {
            if ((lngENC1New - lngENC1Old) >0){intBandwidth = 6000;}
            if ((lngENC1New - lngENC1Old) <0){intBandwidth = 3000;}
            str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3); tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intBandwidth + str3);
            SetFilterBandwidth(intBandwidth);// Sets filters and parameters for correct bandwidth 3000 or 6000 Hz
          }
        if (intMode == 16)//SERIAL
          {
            intBaudPtr = intBaudPtr + (lngENC1New - lngENC1Old);
            if (intBaudPtr > 6) {intBaudPtr = 6;} //Hold at stops
            if (intBaudPtr < 0) {intBaudPtr = 0;}
            
            str1 = chrModes[intMode]; str2 = "    " ; str3 = " Baud";
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(intBaudRate[intBaudPtr]) + str3);
            Serial.end(); Serial.begin(intBaudRate[intBaudPtr]); 
          } 
          
        if (intMode == 17)//TEST3K (Sine)
          {
            SetFilterBandwidth(3000);
            intTestFreqHz = intTestFreqHz + 100 * (lngENC1New - lngENC1Old); //Inc/Dec by 100 Hz per click
            if (intTestFreqHz > 3300) {intTestFreqHz = 3300; }//Hold at stops
            if (intTestFreqHz < 300) { intTestFreqHz = 300;} //Hold at 86 Hz. Lower limit is ~40 Hz at -.5dB (-1.5 dB @10 Hz)  with 10 uf Caps.
            intBandwidth = 3000; intTuneOffset = 0;
            InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
            mixInpSel.gain(0, 2); mixInpSel.gain(1, 0); mixInpSel.gain(2, 0); mixInpSel.gain(3, 0);
          }
        if (intMode == 18)//TEST6K (Sine)
          {
            SetFilterBandwidth(6000);
            intTestFreqHz = intTestFreqHz + 200 * (lngENC1New - lngENC1Old); //Inc/Dec by 200 Hz per click
            if (intTestFreqHz > 6300) {intTestFreqHz = 6300;} //Hold at stops
            if (intTestFreqHz < 300) {intTestFreqHz = 300;}
            intBandwidth = 6000; intTuneOffset = 0;
            InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
            mixInpSel.gain(0, 2); mixInpSel.gain(1, 0); mixInpSel.gain(2, 0); mixInpSel.gain(3, 0);
          }
         
          
        lngENC1Old = lngENC1New;
      }
    else
      { //Handles Busy Detector ENC1 parameter functions
        if (intBusyMode == 0)
          {
            intTargetSN = intTargetSN + (lngENC1New - lngENC1Old);
            if (intTargetSN > intMaxSN){intTargetSN = intMinSN;} //Circular wrap around
            if (intTargetSN < intMinSN) {intTargetSN = intMaxSN;}
          }
        if (intBusyMode == 1)
          {
            intBusyGain = intBusyGain + (lngENC1New - lngENC1Old);
            if (intBusyGain > 8) {intBusyGain = 8;} //Stop at limits
            if (intBusyGain < 0) {intBusyGain = 0;} //Stop at limits
          }
        mixInpSel.gain(2, intBusyGain / 10.0); //sets input mixer 0 gain (0 to 2) for Audio In #1
      }
    if (intBusyMode == 2)
      {
        intBusyBWLoHz = intBusyBWLoHz + intBusyFreqStep * (lngENC1New - lngENC1Old);
        if (intBusyBWLoHz > (intBusyBWHiHz - 300)) {intBusyBWLoHz = 300;} //Circular wrap around
        if (intBusyBWLoHz < 300) {intBusyBWLoHz = (intBusyBWHiHz - 300);}
      }
    if (intBusyMode == 3)
      {
        intBusyBWHiHz = intBusyBWHiHz + intBusyFreqStep * (lngENC1New - lngENC1Old);
        if (intBusyBWHiHz > 3300) {intBusyBWHiHz = (intBusyBWLoHz + 300);} //Circular wrap around
        if (intBusyBWHiHz < (intBusyBWLoHz + 300)) {intBusyBWHiHz = 3300;}
      }
    if (intBusyMode == 4)
      {
        intAvg = intAvg + (lngENC1New - lngENC1Old);
        if (intAvg > 50) { intAvg = 1;} //Circular wrap around
      }
    if (intAvg < 1) 
        {
          intAvg = 50;
        }
      lngENC2Old = lngENC2New;
    }
 

  // Code to handle computation of p-p measurements  ppLPInput
  ulngCurrentElapsedTimeUs = micros();
  if (ulngLastPPAvgTimeUs > ulngCurrentElapsedTimeUs){ulngLastPPAvgTimeUs = ulngCurrentElapsedTimeUs;}// Handles micros() rollover
  if ((ulngCurrentElapsedTimeUs - ulngLastPPAvgTimeUs) > 2000)//now using p-p measurement every 2 ms  
    {
      if (ppLPInput.available() == true)//Update fltppLPInputMeasAvg  
        // This implements a fast attack slow release computation for fltppMixIQOutMeasAvg to follow signals with high crest factors
        // This also follows the peak of a multipath signal caused by the induced multipath peaks and fades
        {
          fltppLPInputPk =  1000 * ppLPInput.readPeakToPeak();//Serial.println(fltppMixIQOutMeasAvg);
          if (fltppInputForDisplay < fltppLPInputPk) // calculate rolling averages (only used for display of Lvl) fltppInputForDisplay displayed every 200 ms below. 
            { 
              fltppInputForDisplay = (.5 * fltppInputForDisplay) + .5 * fltppLPInputPk; // Fast attack to capture peak: 
            }
          else
            {
              fltppInputForDisplay = (.95 * fltppInputForDisplay) + .05 * fltppLPInputPk; // Slower decay :   
            }
                     
            //####################### TEST: sliding window peak detection by Peter DL6MAA ###########################
            #define NUMPEAKS 1500  //Need to confirm on why 1500 chosen!
            
            static int peakcount = 0;
            static float peaklevel = 0;
            static float old_peaklevel = 0;
            static float peaks[NUMPEAKS] = {0};
           
            
            peaks[peakcount] = fltppLPInputPk;
            
            peakcount++;
            peakcount %= NUMPEAKS;

            if(peaks[peakcount] == peaklevel)
            {
                peaklevel = 0;
                 int r, k;

                for(r=0, k = peakcount+1; r<NUMPEAKS-1; r++)
                {
                  if(peaks[k % NUMPEAKS] > peaklevel)
                  {
                    peaklevel = peaks[k % NUMPEAKS]; 
                  }
                   k++;
                }
                fltppLPInputMeasAvg = peaklevel;
            }
            else
            {
              if(fltppLPInputPk > peaklevel)
              {
                peaklevel = fltppLPInputPk;
                fltppLPInputMeasAvg = peaklevel;
              }
            }
            //Serial.print("Line #1878 fltppLPInputPk=");Serial.print(fltppLPInputPk);Serial.print("   fltppLPInputMeasAvg=");Serial.println(fltppLPInputMeasAvg);
            //############################### END OF TEST SECTION ########################################
                     
          // Update Lvl display every 200 ms: GREEN Text: 200mv < fltppInputForDislplay < 1200mv AND fltppMixIWOut <1800. Otherwise RED 
          // Does not use or require sliding window peak detection above.
          blnDisplayLvl= (millis() - ulngLastLevelDisplayMs )  > 200; 
          if ((intMode <5)&&(!blnPlotSpectrum) && blnDisplayLvl)
            {
              tft.setCursor(0,140); tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);  tft.println(strLastLevel);
              tft.setCursor(0,180); tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);  tft.println(strLastCF);
              str1 = "   Lvl=";  str2 =  String(int(fltppInputForDisplay));str3 = " mvp-p"; strLastLevel = str1 + str2 + str3;
              if ((199 < fltppInputForDisplay)&&(fltppInputForDisplay < 1200) &&(fltppMixIQOut <1800) )//"Green" range 
                {tft.setCursor(0,140); tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(strLastLevel);}
              else{tft.setCursor(0,140); tft.setTextColor(ILI9341_RED);  tft.setTextSize(3);  tft.println(strLastLevel);}
            }             
        }
      
      //Noise measurement (rms) 
      if (rmsNoise.available())
        { 
          float fltrmsNoiseRead = 1000 * rmsNoise.read(); 
          
          if (intBandwidth != intLastBandwidth)//This mechanism speeds up acquistion on Bandwidth/filters change then smooths with slow averaging. 
            {
              if (intBandwidth == 3000){fltrmsNoiseAvg = 235;}// 3KHz bandwidth 
              else {fltrmsNoiseAvg = 313;}//   6KHz bandwidth
              intLastBandwidth = intBandwidth;
            }
          else
            {
               fltrmsNoiseAvg = .999 * fltrmsNoiseAvg + .001 * fltrmsNoiseRead; //slow averaging 
            }
            //Serial.print(" rmsNoiseAvg= ");Serial.println(fltrmsNoiseAvg);
         }
      if (rmsLPInput.available() == true)//Update fltrmsLPInputAvg  average every 2 ms if rmsLPInput available
        {
          
          if (fltrmsLPInputMeasAvg < 50){fltrmsLPInputMeasAvg =  1000 * rmsLPInput.read();}
          else { fltrmsLPInputMeasAvg =  (.9 * fltrmsLPInputMeasAvg )+ 100* rmsLPInput.read();}//Rolling avg
          //Serial.print("LPInRmsAvg= "); Serial.println(fltrmsLPInputMeasAvg);
          if ((intMode <5)&&(!blnPlotSpectrum) && blnDisplayLvl)
            {
              if ((fltrmsLPInputMeasAvg > 50))// Only display CF if fltrmsLPInputMeas > 50 mv rms
                { 
                  fltInput_pp_to_rmsRatio = (fltppLPInputMeasAvg)/fltrmsLPInputMeasAvg;// Pure sine wave would have a ratio of 2.828 : 1
                  if (fltInput_pp_to_rmsRatio >=2.82)// Only display CF if > 1.0 
                    {
                      str1 = "CF=";  str2 =  String(abs(fltInput_pp_to_rmsRatio/2.828));str2 = str2 + " [" + (abs(20* log10(fltInput_pp_to_rmsRatio/2.828))) +"dB]";
                      str2 = str2 + "(PEP/Pavg)"; // RF Power metrology for SSB
                      tft.setCursor(0,180); tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);  tft.println(str1 + str2); strLastCF = str1 + str2;
                    }  
                }
            }
        } 
       if (blnDisplayLvl) {blnDisplayLvl = false; ulngLastLevelDisplayMs = millis();}
       if (rmsMixIQ1234Out.available() == true)// Update rmsMixIQ1234OutAvg
        {
          fltrmsMixIQ1234Out = 1000 * rmsMixIQ1234Out.read();
            
          // Below code used to measure long term level averages to establish fltSTD calibration factor used in IIR filters
          //Normally All modes WGN, MPG, MPM, MPP, MPD should have very similaro long term (> 300,000 counts) average rmsMixIQOutAvg values
          //With Revision .9.79 this is normally 150 mv +/- 4 mv (About +/- .22 dB) with the 1000 mv sine wave test tone input so individual calibration factors 
          // (fltSTD) are not needed for each path type. 
          // 
           
          if (blnResetrmsMixIQCount == true)
            {
              fltrmsMixIQ1234OutAvg = 0.0; intCountrmsMixIQ1234Out = 0; fltrmsMixIQ1234OutPeak = 0.0; blnResetrmsMixIQCount = false;
            }
          
          fltrmsMixIQ1234OutAvg = ((fltrmsMixIQ1234OutAvg * intCountrmsMixIQ1234Out) + (fltrmsMixIQ1234Out))/(intCountrmsMixIQ1234Out +1); intCountrmsMixIQ1234Out +=1;
          fltrmsMixIQ1234OutPeak = max(fltrmsMixIQ1234OutPeak, fltrmsMixIQ1234Out);
          //This computes the long term >20 minutes) average rms to allow calibration of Scale factors (fltSTD) for Multipath
          
          if (intCountrmsMixIQ1234Out == 20000 *  (intCountrmsMixIQ1234Out/20000))// only print every 20000 updates 
            {
              Serial.print("Line 2089: fltrmsLPInputMeasAvg= "); Serial.print(fltrmsLPInputMeasAvg);
              Serial.print("  rmsMixIQ1234Out = " );Serial.print(fltrmsMixIQ1234Out); 
              Serial.print("  rmsMixIQ1234OutAvg = ");Serial.print(fltrmsMixIQ1234OutAvg);
              Serial.print("  Cnt="); Serial.print(intCountrmsMixIQ1234Out); 
              Serial.print("  rmsMixIQ1234OutPk= ");Serial.println(fltrmsMixIQ1234OutPeak);
              Serial.print("  ppLPInputMeasAvg= ");Serial.println(fltppLPInputMeasAvg);
            } 
        } 
         
      if ((blnTestMode3K || blnTestMode6K) && (!blnPlotSpectrum))
        {
          if (ppi2s0In.available() == true)//Update ppi2s0In average if available
            {
              if (fltppi2s0InAvg < 50){fltppi2s0InAvg = 1000 * ppi2s0In.readPeakToPeak();}
              else {fltppi2s0InAvg = (.9 * fltppi2s0InAvg) +  100 * ppi2s0In.readPeakToPeak(); }//moderate time constant for ppi2s1In  average
            }
     
          if (ppi2s1In.available() == true)//Update In average if available
        {
          if (fltppi2s1InAvg < 50){fltppi2s1InAvg = 1000 * ppi2s1In.readPeakToPeak();}
          else {fltppi2s1InAvg = (.9 * fltppi2s1InAvg) +  100 * ppi2s1In.readPeakToPeak();} //moderate time constant for ppi2s1In  average
        
        }
      if (ppAmpRightOut.available() == true)//Update ppAmpRightOut average if available
        {
          if (fltppAmpRightOutAvg < 50) {fltppAmpRightOutAvg = 1000 * ppAmpRightOut.readPeakToPeak();}
          else {fltppAmpRightOutAvg = (.9 * fltppAmpRightOutAvg) +  100 * ppAmpRightOut.readPeakToPeak();} //moderate time constant for ppAmpRight  average
        }
      if (ppAmpLeftOut.available() == true)//Update ppAmpLeftOut average if available
        {
          if (fltppAmpLeftOutAvg < 50) {fltppAmpLeftOutAvg = 1000 * ppAmpLeftOut.readPeakToPeak();}
          else {fltppAmpLeftOutAvg = (.9 * fltppAmpLeftOutAvg) +  100 * ppAmpLeftOut.readPeakToPeak();} //moderate time constant for ppAmpLeft  average
        }
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(0, 0);
      str1 = chrModes[intMode];  str2 =  " Ch1 Out = "; str3 = " Ch1 In  = ";
      tft.setTextColor(ILI9341_MAGENTA);  tft.setTextSize(3);  tft.println(str1);
      tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3); tft.println(" Freq (Hz) =" + String(intTestFreqHz));
      tft.println(" ");
      //Threshold test based on Rev 2 120 tap LPFIR
      if (blnTestMode3K)
        {
          if ((fltppAmpLeftOutAvg > (fltNomCalLevel * .89)) && (fltppAmpLeftOutAvg < (fltNomCalLevel * 1.12))) //Nominal +/- 1 dB
            {
              tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
            }
          else 
            {
              tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
            }
          if ((fltppi2s0InAvg > (fltNomCalLevel * fltNomInOutRatio * .89)) && (fltppi2s0InAvg < (fltNomCalLevel * fltNomInOutRatio * 1.12))) //Nominal +/- 1 dB
            {
              tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s0InAvg)));
            }
          else 
            {
              tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s0InAvg)));
            }
          str2 =  " Ch2 Out = "; str3 = " Ch2 In  = ";

          if ((fltppAmpRightOutAvg > (fltNomCalLevel * .5 * .89)) && (fltppAmpRightOutAvg < (fltNomCalLevel * .5 * 1.12)))// //Nominal +/- 1 dB
            {
              tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
            }
          else 
            {
              tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
            }
          if ((fltppi2s1InAvg > (fltNomCalLevel * .5 * .89 * fltNomInOutRatio )) && (fltppi2s1InAvg < fltNomCalLevel * .5 * 1.12 * fltNomInOutRatio ))// Nominal +/- 1 dB
            {
              tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s1InAvg))); 
            }
          else 
            {
              tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s1InAvg)));
            }
        }
      if (blnTestMode6K)
        {
          if ((fltppAmpLeftOutAvg > (fltNomCalLevel * .5 * .89 )) && (fltppAmpLeftOutAvg < (fltNomCalLevel * .5 * 1.12  )))//Nominal +/- 1 dB
            {
              tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
            }
            else 
              {
                tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
              }
            if ((fltppi2s0InAvg > (fltNomCalLevel * .5 * .89 * fltNomInOutRatio )) && (fltppi2s0InAvg < fltNomCalLevel * .5 * 1.12 * fltNomInOutRatio ))//Nominal +/- 1 dB
              {
                tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s0InAvg)));
              }
            else 
              {
                tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s0InAvg)));
              }

            str2 =  " Ch2 Out = "; str3 = " Ch2 In  = ";
            if ((fltppAmpRightOutAvg > (fltNomCalLevel * .89)) && (fltppAmpRightOutAvg < (fltNomCalLevel * 1.12))) //Nominal +/- 1 dB
              {
                tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
              }
            else 
              {
                tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
              }
            if ((fltppi2s1InAvg > (fltNomCalLevel * fltNomInOutRatio * .89)) && (fltppi2s1InAvg < (fltNomCalLevel * fltNomInOutRatio * 1.12))) //Nominal +/- 1 dB
              {
                tft.setTextColor(ILI9341_GREEN); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s1InAvg)));
              }
            else 
              {
                tft.setTextColor(ILI9341_RED); tft.setTextSize(3); tft.println(str3 +  String(int(fltppi2s1InAvg)));
              }
        }
      delay(50);
    }
    if (!(blnTestMode3K || blnTestMode6K))//Code to measure input levels (not during TEST3K or TEST6K)
    {
      if (ppi2s0In.available() == true)//Update ppi2s0In average if available
      {
        if (fltppi2s0InAvg < 100) { fltppi2s0InAvg = 1000 * ppi2s0In.readPeakToPeak();}
        else {fltppi2s0InAvg = (.9 * fltppi2s0InAvg) +  100 * ppi2s0In.readPeakToPeak();} //Fast time constant for ppi2s1In  average
        }
      }
      if (ppi2s1In.available() == true)//Update In average if available
      {
        if  (fltppi2s1InAvg < 100) {fltppi2s1InAvg = 1000 * ppi2s1In.readPeakToPeak();}
        else {fltppi2s1InAvg = (.9 * fltppi2s1InAvg) +  100.0 * ppi2s1In.readPeakToPeak();} //Fast time constant for ppi2s1In  average
      }
    ulngLastPPAvgTimeUs  = ulngCurrentElapsedTimeUs;
  } // End of Code to handle computation of p-p measurements from ppmixIQOut

  //Code to handle sliding Window rms Calculation of I channel
  
    //Code to handle Channel Types: MPG, MPM, MPP, MPD (only used in multipath modes NOT WGN)
  //Determine if time to update delays (as a function of Ch Type:  (based on update rate of 64 times the Doppler spread in Hz) 
  if (((intMode == 4) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (7812))) //Every 7.812 ms for MPD  
      || ((intMode == 3) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (15625))) //Every 15.625ms MPP
      || ((intMode == 2) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (31250))) //Every 31.250ms MPM
      || ((intMode == 1) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (156250)))) //Every 156.25ms MPG
      {
        //Serial.print("Channel Updates: "); Serial.println( ulngLastDelayUpdateUs);
        if (intMultipaths == 2) 
          {
            QuadGauss12FIR128( intMode);// Updates WGN and FIR filters for I1, Q1, I2, Q2 Paths (FIR Filter bandwidth proportional to Update Rate) 
          }
        if (intMultipaths == 4) 
          {
            QuadGauss12FIR128( intMode);// Updates WGN and FIR filters for I1, Q1, I2, Q2 Paths (FIR Filter bandwidth proportional to Update Rate) 
            QuadGauss34FIR128( intMode);// Updates WGN and FIR filters for I3, Q3, I4, Q4 Paths (FIR Filter bandwidth proportional to Update Rate) 
          }  
        ulngLastDelayUpdateUs = ulngCurrentElapsedTimeUs;
      }
    else if( ulngCurrentElapsedTimeUs < ulngLastDelayUpdateUs)// handles micros() rollover [about once ever 70 minutes]
       {
          ulngLastDelayUpdateUs = ulngCurrentElapsedTimeUs; ulngLastSNUpdateUs = ulngCurrentElapsedTimeUs;
       }
     
        
  // Adjust  S:N only every 2-5 ms 
  if (int(ulngCurrentElapsedTimeUs - ulngLastSNUpdateUs) > random(2000, 5000)); // Only adjust S:N every 2-5ms
    {
      if (blnSim) { AdjustS_N ( intTargetSN, fltppLPInputMeasAvg);}
      ulngLastSNUpdateUs = ulngCurrentElapsedTimeUs;
      // Code for Fading a Channel (modes 0-4) WGN,MPG,MPM,MPP,MPD
      if ((intFadeDepth_dB > 0) && (fltFadeRate > .01) && blnSim)
        {
          Fade (intTargetSN, intFadeDepth_dB, fltFadeRate);//Fade also adjust S:N on peak value averages
        }
      else if (blnSim) 
        {
          AdjustS_N ( intTargetSN, fltppLPInputMeasAvg);
        }
    }// End of Code to handle S:N adjustment

  // Plot Spectrum for Sim mode
  if (millis() < ulngLastSpectrumUpdateMs){ulngLastSpectrumUpdateMs = millis();}//handles possible millis() rollover
  if ((blnSim) && ( (millis() - ulngLastSpectrumUpdateMs) > 100) && blnPlotSpectrum) //Plot only every 100 ms in Sim mode (no averaging)
    {
      if (fft1024.available())
        {
          if (intBandwidth == 6000) {intNumofBinsToPlot = 150;} //6503 Hz
          if (intBandwidth == 3000) {intNumofBinsToPlot = 82;} //3531 Hz
          for (int i = 0; i <= intNumofBinsToPlot; i++) { fltFFT[i] = 4000.0 * fft1024.read(i);}
          PlotSpectrum(fltFFT, intSpecLow, intSpecHigh, blnInitSpectrum, intBandwidth);
          blnInitSpectrum = false;// only init on first call to print headers.
          ulngLastSpectrumUpdateMs = millis();
        }
    }

  // Process/Plot Spectrum for BUSY Detect mode  (uses spectrum averaging for busy detection)
  if (!blnSim)
  {
    if (fft1024.available())
    {
      AvgFFTBins((intBinsAveraged == 0), .2 , intBusyBWLoHz, intBusyBWHiHz); //Average the fltFFT[] bins, initialize if intBinsAveraged == 0
      intBinsAveraged += 1;
      if (intBinsAveraged > intAvg)//If enough averages completed
      {
        intDetectSN = SearchRatioDetect(intBusyBWLoHz, intBusyBWHiHz, 258, intThresh);//Search and detect Region of Interest for 258 Hz BW)
        if ((intDetectSN >= intThresh ) && (!blnChanBusySent) && blnEnbBusyDetect)
        {
          Serial.print("DETECT:"); Serial.println(intDetectSN);
          blnChanBusySent = true;
          blnChanClearSent = false;
        }
        else if ((intDetectSN == 0)  && (!blnChanClearSent) && blnEnbBusyDetect)
        {
          Serial.println("DETECT:0");
          blnChanClearSent = true;
          blnChanBusySent = false;
        }
      }
      if (0 == (intBinsAveraged % intAvg))// Only plot every intAvg averages (about 11ms /average)
      {
        if (blnPlotSpectrum)
        {
          PlotSpectrum(fltFFT, intBusyBWLoHz, intBusyBWHiHz, blnInitSpectrum, intBandwidth);
          blnInitSpectrum = false;// only init on first call to print headers.
          ulngLastSpectrumUpdateMs = millis();
        }
        if (intBinsAveraged > 100000) {
          intBinsAveraged = 0; //To prevent overflow of intBinsAveraged
        }
      }
    }
  }
  // Code for updating Encoder push buttons
  debouncer1.update();//right Encoder
  debouncer2.update();//left Encoder
  if (debouncer1.fell())//Right Encoder "push"
    {
      blnEnableTestTone = !blnEnableTestTone;//Toggle Test tone
      if (intMode < 5)
        {
          if (blnEnableTestTone)
            { //Set mixInpSel mux gains and enable InputTestWaveform @ 1500 Hz
              mixInpSel.gain(0, 2.0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
              InputTestWaveform.amplitude(.251880522);
              intTestFreqHz = 1500; //sine_VLF_Dnmix_Mod.amplitude(0);
              InputTestWaveform.frequency (intTestFreqHz);
              blnResetrmsMixIQCount = true;
            }
          else
            { //Restore mixInputSel gains and turn off InputTestWaveform
              mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, 100 * fltLogs[intGainLevel[0]]); mixInpSel.gain(3, 100 * fltLogs[intGainLevel[1]]);
              InputTestWaveform.amplitude(0);
            }
        }
      if ((intMode >= 5) && (intMode <= 16)) //Gain and output level settings)setting to  EEPROM
        {
          SaveParametersToEEPROM();
           tft.setCursor(0,70);tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3); tft.println(" SAVED TO EEPROM");
        }
      delay(100);  
    }
 
  if (debouncer2.fell())//Left Encoder Push
    {
      if (blnTestMode3K || blnTestMode6K)
        {
          blnPlotSpectrum = ! blnPlotSpectrum;
          if (blnPlotSpectrum) { blnInitSpectrum = true; blnModes = false;}
        }
      else if (blnModes)
        {
          blnModes = false; blnPlotSpectrum = true; blnInitSpectrum = true;
        }
      else if (blnPlotSpectrum)
        {
          blnPlotSpectrum = false;     
          blnInitModes = true; blnModes = true;
        }
      delay(100);
    }
  
  // This code to receive serial commands via USB and return status via USB.
  while  (Serial.available() > 0)//While there are any unprocessed characters received via the serial port
    {
      char inChar = Serial.read();//Read one char
      if (inChar == '\n')//if it is a New Line (Cr)
        {
          //Serial.print("Mode=");//Debugging code ...remove
          //Serial.println(strMode.toUpperCase());//Shift to upper makes all case insensitive
          //Serial.print("Parameter=");
          //Serial.println(strParameter.toUpperCase());//Shift to upper makes all case insensitive
          intSerialCmdMode = -1; intSerialCmdParam = -1;
          if (blnSim)
            {
              intSerialCmdMode = ParseSimMode(strMode);
              if (intSerialCmdMode == 18)//  Moved to 18 to accomodate TEST3K, TEST6K
                {
                  Serial.println("OK");//Serial Command is OK
                  blnSim = false;  blnInitialized = false;  blnEnableTestTone = false;
                  InitializeBusy();
                }
              else if (intSerialCmdMode > -1)
                {
                  //Serial.print("Line 2240: intSerialCmdMode = ");Serial.println(intSerialCmdMode);
                  if (ParseSetSimParameter(strParameter, intSerialCmdMode))
                    {
                      Serial.println("OK"); //Serial Command is OK
                      if (intSerialCmdMode < 5){ ParseSetParameter(strParameter, intSerialCmdMode);}
                    }
                  else { Serial.println("?"); }//Serial Command fail}
                }
              else {Serial.println("?");}
            }
          else   
            {
              intSerialCmdMode = ParseBusyMode(strMode);
              if (intSerialCmdMode == (intNumBusyModes - 1))
                {
                  Serial.println("OK");//Serial Command is OK
                  strMode = ""; strParameter = ""; blnColon = false; //Initialize
                  setup();
                }
              else if (intSerialCmdMode > -1)
                {
                  if (ParseSetBusyParameter(strParameter, intSerialCmdMode)){ Serial.println("OK");} //Serial Command is OK
                  else {Serial.println("?");} //Serial Command fail     
                }
              else {Serial.println("?");}
            }
          // Process the command by setting mode and parameter Set display to show just received mode/parameter
          strMode = ""; strParameter = ""; blnColon = false; //Initialize
        }
    else if (inChar == ':') { blnColon = true; }//Set the blnColon flag separating Mode from Parameter
    else if (blnColon == true) {strParameter += inChar;}//Accumulate the parameter
    else {strMode += inChar;} //Accumulate the mode
  }//End while Serial.available
  intSerialCmdMode = -1; intSerialCmdParam = -1;
}// End Main Loop *******************************************************************************
