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
    5/28/2020 .9.18     One path update done each 32 x spread.  Power on default multipaths set to 2 (was 4)
                        IQ Mixer gain emperical adjustments to keep level output level ~ constant vs # of multipaths
                        
    5/26/2020 .9.17A    Experimental.  all Multipaths (2,3,4) are updated at 32 x spread vs 1 of n multipaths
                        modified delay update timing to stagger updates.  All updates for 2, 3, or 4 paths now done at 32 x Spread but staggered
                        modifired S:N attack/release timing ~line 1826 was .99, .01  now .995, .005 
                        modified S:N update period ~line 2076  to 2 - 5 ms (was 10-20 ms)
    5/23/2020 .9.17     Corrected Lvl and CF blackout ~ line 1844
    
    5/19/2020 .9.16     Serial # display/printout changed to 8 digit Hex
    5/15/2020 .9.15B    changed gains on mixAudioSelect up by factor of 2 to extend signal range. now values of (0,1)or (1,4)
                        Corrected logic statement on shift/offset line #2191 was disabling intTuneOffset
                        Changed Level amplitude range to 300 to 1700 mv (was 300-1000) OK with new mixAudioSelect gains above.
                        Added emperical gain adjustments on MixIQ inputs for mulitpath to balance levels for various multipaths. ~ Line:2092 
                        Added printout of Teensy Serial Num (0 to 8191) on Revision splash screen
    5/13/2020 .9.15     Added measure and display of Crest Factor and PAPR for WGN mode. (required adding rmsLPOut) 
    5/9/2020  .9.15     New release with all changes included from .9.14A
        
    5/6/2020  .9.14A    Using new mechanism for combining I and Q magnitudes back to Real value
                        Gains of mixIQ adjusted with each I,Q delay change: IDelayToIGain, QDelayToQGain
                        One path delay updated each update cycle for 2, 3, 4 multipath
                        "SAVED TO EEPROM" update to screen modified. Cleaned up and commented line # Serial.print statements
                        Fixed error in offset and deviation mechanism that failed to set Up/Dn mixer L.O. amplitude/freq.  
                        
    5/4/2020  .9.13     Working on problem transitioning from BUSY back to SIM...
    
    5/2/2020  .9.12     Change mode order moving MULTIPATHS to #5 and pushing back others to allow multipath selection to follow path type selection
                        Updates and fixes to save/recall to EEPROM...Confirmed working for all modes 5-15.
                        Modification to mechanism for changing update times and mechanism in multipath modes (approx lines 2008-2030)
                        Fixed Test tone injection with OFFSET and FM
                        Fixed graphics over write on Spectrum display
                        Fixed bounce problme on encoder push with added delay
                        
    5/1/2020  .9.11     Several corrections having to do with added intMultipaths
                        Now supports multipath with 2, 3 or 4 audio paths (rays)
                        Delay updates now update I1, Q1 every cycle if intMultiPaths = 2  or
                        Every other cycle if intMultiPaths > 2  The cycle rate is 32x delay spread rate
                        
    5/1/2020  .9.10     Changing fltGains to fltLogs.
    4/30/2020 .9.9.3.Y  Test version for Tom W with I1,Q1 values contribution increased reduced from .25 to .34 and I2,Q2 values decreased from .25 to .16 in Multipath modes. 
    4/30/2020 .9.9.3.X  Test version for Tom W with Q values contribution reduced from .25 to .16 aand I values increased from .25 to .37 in Multipath modes. 
    4/29/202  .9.9.3    Tracing down distortion with "tone on" in Busy detect;
                        Added MIT Open Source Initiative notice above
                        Verified all Delay limits and calculations. for WGN through MPD.

    4/28/2020 .9.9.2    Fixed problem in .9.9.1 that cleared normal input gains upon exit from TEST3K or TEST6K (~ line 1159)
                        Removed Print.line  Tracing/debug  lines from code.
    4/28/2020 .9.9.1    Adjustments in the way p-p measurements taken: Changed rate from every 1 ms to every 20 ms. ~si1486
                        Modify fltppLPOutMeasAvg IIR attack constant ~ line 1500
                        Modify fltppLPOutMeasAvg IIR release constant when input > 200 mv p-p preseent  ~ line 1505
                        Modify fltppLPOutMeasAvg IIR release constant when input < 200 mv p-p present ~ line 1510
    4/27/2020 0.9.9     Changed Update rate for Multipath delays from 40x spread to 32x spread. One IQ Delay path updated per interval.
                        Modified logic for Mode Encoder Push.
                        Changed Starting and high/low limit frequency values in TEST3K and TEST6K
    4/25/2020 0.9.8     Inclusion of Check for nominal gain levels on TEST3K & TEST6K with Pass limits reduced from +/-1 to +-.5 dB.
                        Modified Low and high frequency limits on TEST3K (86 - 3225 Hz, step 86Hz, ) and TEST6K(68-6287 Hz, step 172 Hz)
                        No calibration required or anticipated with normal component tolerence.
                        Changed spectrum BW initialization values: intSpecLow =0, intSpecHigh =6500,intBusyBWLoHz =0,intBusyBWHiHz =6500 (eliminated yellow lines on default)
                        Comment out debugging Serial.print on Right and Left Encoder PUSH
    4/21/2020 0.9.7     Introduction of optional saving/recalling parameter default settings to EEPROM (except S:N on WGN-MPP)
    4/20/2020 0.9.6     Adjustment of calibration test limits to +/- 10% (+/-.82dB) from measured nominal values on Rev 2 PCB.
    4/19/2020 0.9.5     Adjustment of Test mode calibration.
                        Increased tone increments on TEST3K = 86 Hz/click,  TEST6K = 172Hz/Click
                        Insure WGN, TEST3K, and TEST6K disable Hilbert inputs in Mix IQ (caused calibration test issues with sensitivity to tone frequency)
    4/17/2020 0.9.4     Adjustment of Test mode calibration
    4/16/2020 0.9.3     Made changes to Up and Down mixer. Replace HP FIR Filter for up mix of 7500Hz and 120 taps.
                        Adjusted gain and threshold values for Test3K and Test6K
                        Set acceptance gain range to +/-.5 dB over full range on 3 and 6 KHz bandwidths
                        Replace 3 KHz and 6 KHz 100 tap LP FIR with 120 tap. Less ripple, higher attenuation
                        Modified AdjustS_N and AdjustBusyS_N to block out unused input to mixer (caused gain error).
    4/13/2020 0.9.2     Added mixAudioSelect to bypass Up/Dn mixers if not using FM or Offset function. MInimizes mixer and filter ripple.
    4/12/2020 0.9.1     Added Test modes TEST3K Sine  and TEST6K Square with pp measurements to confirm calibration and gains
    4/6/2020  7.3.6     Modified SearchRatioDetect to do search for max and min values over narrow freq range(258 Hz) (better Wide detects).
                        Measured time to plot spectrum ~200 ms. Increased speed when not enabled. (spectrum:0). Updated Docs.
                        Remove BLUE and Magenta Spectrum plots used for debugging. Now either GREEN (not busy) or RED (busy)
    4/2/2020  7.3.5     Changed input gains and output levels to cover a log range (0,.1,.2,.5,1,2,5,10,20,50,100). Changed THRESH: command to 3-40 dB
    4/1/2020  7.3.3     Modify busy detect SearchRatioDetect. Add blnEnbableBusyDet. Preset CH1 IN gain to 5 on busy detect entry.
    3/31/2020 7.3.2     Initial Busy detector operational. Serial commands oprerational. Wideband busy detection still needs some optimization.
    3/16/2020 7.2.2     Boost input gain settings (now Gain of .5  to 10 on ports 2 & 3 of mixInpSel) ~line 1226,999
    3/15/2020 7.2.1     Change/correct Input gain settings for mixInpSel ports 2 & 3 ~line 1208
    3/11/2020 7.2       Changes in *chrModes[]  case to Upper to accomodate Serial commands.
    3/9/2020  7.1       Corrected FM deviation calibration factor for new LO of 7400Hz Now .000135135125
    3/8/2020  7.0       Initial Alpha Release of 3KHz or 6 KHz BW. LO Changed. mixers changed. Spectrum plot changed.  All modes and display functions operational.

    To do:

    1) Investigate busy detector optimization for wide band modes (where the detection is for a signal that occupies
    > 50% of the bandwidth of interest. Currently works well for signals that are < 50% of the bandwidth of interest.
    2) Locate and fix problem with Serial command to re enter SIM mode from BUSY mode.  Distortion present. Busy mode and initial SIM mode looks OK.     

         
     
     
     For Ref:
      on Rev .9.14A  5/7/2020
      Program storage space = 6% (139768 bytes), Global variables 36% (193204 bytes)
      AudioProcessorUsage max 12.96%, AudioMemoryUsage: 26 Blocks, max 38 blocks with max delays of 13 ms
      Rick Muething, KN6KB 4/25/2020

    Summary of Simulation Modes:

    HF/VHF  Channel Modes
    Channel     Spread   Delay    Fade Depth      Frequency                     Notes:
    Type(mode)    Hz      ms
    WGN (0)       0        0                                                    Used with Flat fading on VHF/UHF modeling
    MPG (1)       .1       .5                                                   CCIR Multipath Good
    MPM (2)       .5       1                                                    CCIR Multipath Moderate
    MPP (3)       1        2                                                    CCIR Multipath Poor
    MPD (4)       2.5      5                                                    Multipath Disturbed
    FADE DEPTH(5) 0               0 to 40dB                                     Flat Fading (depth)
    FADE FREQ(6)                               0,.1,.2,.5,1,2,5,10,20,50,100 Hz Flat Fading (rate)
    OFFSET(7)                                  -200 to + 200 Hz                 Can be used with all ch types
    FM Deviation(8)                            0,.1,.2,.5,1,2,5,10,20,50,100 Hz Can be used with all ch types
    FM Rate(9)                                 0,.1,.2,.5,1,2,5,10,20,50,100 Hz Can be used with all ch types

    OFFSET and FM modeling can be used with any channel type and adds a fixed offset or
    VLF sine wave (FM Deviation) to the Up mixer LO
*/

//Libraries
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Bounce2.h>
#include <Encoder2.h>
#include "SPI.h"
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

// Teensy Audio components
AudioControlSGTL5000            sgtl5000_1;
AudioInputI2S                   i2s0;
AudioOutputI2S                  i2s1;

AudioFilterFIR                  filLPIn_0_FIR;  // Input LP Filter
AudioFilterFIR                  filLPOutFIR;  // Channels 1 & 2 output LP (reduces possible "clicks" from delay changes)
AudioFilterFIR                  filHilQ_12; //First Hilbert FIR filter for Q1 and Q2 channels
AudioFilterFIR                  filLPWhiteFIR; // Low pass filter for White noise
AudioFilterFIR                  filHP7500FIR; // Highpass for up mixing 7500 Hz Injection Freq
AudioFilterFIR                  filLPDownMix; // LowPass filter for Down Mixing 7500 nominal (no shift) injection freq.

AudioSynthWaveformSine          InputTestWaveform; // Used to provide single tone test signal to make measurements at various points.
AudioSynthWaveformSine          sine_Upmix;  // 7.5 KHz Upmix for Low Freq Offset/drift  testing
AudioSynthWaveformSineModulated sine_Dnmix;  // ~7.5 KHz Down mix injection for Low Freq Offset/drift testing
AudioSynthNoiseWhite            whiteOut;   // White noise to add to Channels for S:N
AudioSynthWaveformSine          sine_VLF_Dnmix_Mod;// Slow VLF modulation feeds sine_Dnmix modulation input.

AudioEffectDelay                delayI; // Delay for audio I paths 1, 2(multi tap)
AudioEffectDelay                delayQ; // Delay for audio Q paths 1, 2(multi tap)
AudioEffectMultiply             multUpmix;// Multiplier for Upmixing 7500 Hz
AudioEffectMultiply             multDnmix;// Multiplier for Downmixing ~ 7500 Hz

AudioMixer4                     mixIQ;// Mixer to combine I and Q Delays and Hilbert outputs
AudioMixer4                     mixChannels;// Mixer to combine ch 1, ch2, and White Noise
AudioMixer4                     mixInpSel;// Mixer to select InputTestWaveform, or external inputs 1(L) or 2 (R) 
AudioMixer4                     mixAudioSelect;// Mixer to select normal audio in or Up/Dn mix audio for offset/drift testing
AudioAmplifier                  ampLeftOut;// Amp for gain control Left Output
AudioAmplifier                  ampRightOut;//Amp for gain control Right Output

AudioAnalyzePeak                ppLPOut; //p-p value of Output of filLPOutFIR
AudioAnalyzePeak                ppWhiteOut; //p-p value of Output of Low Pass Filtered whiteOut
AudioAnalyzePeak                ppi2s0In;//  Probe to measure input i2s0 p-p value (Used in auto test)
AudioAnalyzePeak                ppi2s1In;// Probe to measure input i2s1 p-p value (Used in auto test)
AudioAnalyzePeak                ppAmpRightOut;// Probe to measure AmpRightOut p-p value (Used in auto test)
AudioAnalyzePeak                ppAmpLeftOut;// Probe to measure AmpRightOut p-p value (Used in auto test)
AudioAnalyzeRMS                 rmsLPOut;//  rms probe to compute PAPR on LP Out
AudioAnalyzeFFT1024             fft1024; //  FFT analysis and Spectrum plotting

// Audio Connections  (for a total of 2 audio paths through the simulator each path containing I and Q paths)
//Input through audio ch 0 to  LP input filter and Up mixer, Down Mixer, and Down Mixer LP Filter
AudioConnection           pcInp_1(InputTestWaveform, 0, mixInpSel, 0);//Test sine wave to input 0 of input mixer
AudioConnection           pcInp_2(i2s0, 0, mixInpSel, 2);// Input pin 0 of  i2s0 to input 2 of input mixer (Left)
AudioConnection           pcInp_3 (i2s0, 1, mixInpSel, 3);// Input pin 1 of  i2s0 to input 3 of input mixer (Right)
AudioConnection           pcInp_5(i2s0, 0, ppi2s0In, 0); // p-p measurement of input i2s0 pin 0
AudioConnection           pcInp_6(i2s0, 1, ppi2s1In, 0); // p-p measurement of input i2s0 pin 1

// Audio Connections mixInpSel through to inputs (normal and offset/modulated) t0 mixAudioSelect inputs 0(normal) and 1(offset/modulated)
AudioConnection           pc0_1(mixInpSel, filLPIn_0_FIR); //Input mixer to filLPIn_FIR;  Test source to evaluate complex wave input from SD
AudioConnection           pc0_2(filLPIn_0_FIR, 0, multUpmix, 0);// filLPIn to Upmixer
AudioConnection           pc0_3(sine_Upmix, 0, multUpmix, 1);// Upmix L.O. injection 7.5 KHz
AudioConnection           pc0_4(multUpmix, filHP7500FIR);// Upmix to HPFilter.
AudioConnection           pc0_5(sine_VLF_Dnmix_Mod, 0, sine_Dnmix, 0); // Modulation input to sineDnmix
AudioConnection           pc0_6(sine_Dnmix, 0, multDnmix, 1);// Dnmix L.). injection ~ 7.5 KHz
AudioConnection           pc0_7(filHP7500FIR, 0, multDnmix, 0);// HP7500FIR to DownMix mult
AudioConnection           pc0_8 (multDnmix, filLPDownMix);// Down Mix to filLPDnMix
AudioConnection           pc0_9(filLPDownMix, 0, mixAudioSelect, 1); // Down mix filter to mixAudioSelect input 1
AudioConnection           pc0_10(filLPIn_0_FIR, 0, mixAudioSelect, 0);// LPIn to mixAudioSel input 0 to bypass Offset Up/Dn mixiing

// mixAudioSelect output through summed I and Q Audio Out of Channels 1 & 2 (4 rays total)
AudioConnection           pc1_1(mixAudioSelect, filHilQ_12); // mixAudioSelect to First Hilbert FIR (filHilQ_12)
AudioConnection           pc1_2(filHilQ_12, delayQ); //  filHilbQ_12 to delay Q
AudioConnection           pc1_3(mixAudioSelect, delayI);// mixAudioSelect to delay I
AudioConnection           pc1_4(delayI, 1, mixIQ, 1);// delay1I tap #1 to mixIQ input 1
AudioConnection           pc1_5(delayQ, 1, mixIQ, 0);// Delay Q tap #11 to mixIQ input 0
AudioConnection           pc1_6(delayQ, 2, mixIQ, 2);// Delay Q tap 2 to mixIQ input 2
AudioConnection           pc1_7(delayI, 2, mixIQ, 3);// delay1I tap #2 to mixIQ input 3
AudioConnection           pc1_8(mixIQ, filLPOutFIR);// mixIQ to filLPOutFIR

// Channel outputs and Noise to mixerChannels and Amplifiers Left and Right outputs
AudioConnection           pc2_1(filLPOutFIR, 0, mixChannels, 1); // LP output to mixChannels input 1
AudioConnection           pc2_2(whiteOut, filLPWhiteFIR); //whiteOut to to filLPWhiteFIR
AudioConnection           pc2_3(filLPWhiteFIR, 0, mixChannels, 0); //Low pass filtered whiteOut to to mixChannels input 0 (inputs 2, 3 not used)
AudioConnection           pc2_4(filLPWhiteFIR, ppWhiteOut);// LP Filtered White to ppWhiteOut (note: p-p measurement)
AudioConnection           pc2_5(filLPOutFIR, ppLPOut); // Low Pass out of combined IQ to ppLPOut (note: p-p measurement)
AudioConnection           pc2_6(mixChannels, ampLeftOut); // mixChannels to LeftOutAmp for gain adjust
AudioConnection           pc2_7(mixChannels, ampRightOut); // mixChannels to RightOutAmp for gain adjust
AudioConnection           pc2_8(ampLeftOut, 0, i2s1, 0);// AmpLeftOut to headphones left ch
AudioConnection           pc2_9(ampRightOut, 0, i2s1, 1);// AmpRightOut to headphones right ch
AudioConnection           pc2_10(filLPIn_0_FIR, 0 , mixChannels, 2); //filLPIn_0_FIR to mixChannels for Busy detect
AudioConnection           pc2_11(mixChannels, fft1024);// mixChannels to FFT analysis
AudioConnection           pc2_12(ampRightOut, ppAmpRightOut); // p-p monitor for ampRightOut
AudioConnection           pc2_13(ampLeftOut, ppAmpLeftOut);// p-p monitor for ampLeftOut
AudioConnection           pc2_14(filLPOutFIR, rmsLPOut);// filLPOutFIR to rms measurement to allow PAPR computation.
//End of audio connections

#define SCL1  16 // pin 16 for Teensy 4.0 SCL1
#define SDA1  17 // pin 17 for Teensy 4.0 SDA1
#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD

// Static Arrays:
static float fltLogs[] = {0.0, .1, .2, .5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0}; // Log values (Used for all parameters with log steps) 
static char  *chrModes[] = {"WGN:            ", "MPG:            ", "MPM:            ", "MPP:            ", "MPD:            ","MULTIPATHS:     ",
                            "FADE DEPTH:     ", "FADE FREQ:      ", "OFFSET:         ", "FM DEVIATION:   ", "FM RATE:        ",
                            "CH1 IN:         ", "CH2 IN:         ", "CH1 OUT:        ", "CH2 OUT:        ", "BANDWIDTH:      ", 
                            "TEST3K Sine:    ", "TEST6K Sine:    ", "BUSY:           "};
static char  *chrBusyModes[] = {"ENB BUSY:       ", "DIS BUSY:       ", "LOW:            ", "HIGH:           ",
                                "THRESH:         ", "TONE ON:        ", "TONE OFF:       ", "CH1 IN          ", "BANDWIDTH       ", "SPECTRUM:       ", "SIM:            "
                               };

//Strings
String strMode = ""; String strParameter = ""; String strRevision = "    Rev .9.18";
String strLastCF = ""; String strLastLevel = "";

//Boolean
boolean blnSim = true;//Default for Simulator operation, false = busy detector mode.
boolean blnInitialized = false; boolean blnEnableTestTone = false;
boolean blnPlotSpectrum = false; boolean blnInitSpectrum = false; boolean blnPlotIQPlane = false;
boolean blnModes = true; boolean blnInitModes = true; boolean blnColon = false; boolean blnChanBusySent = false;
boolean blnChanClearSent = false; boolean blnPlotBusyRed = false; boolean blnEnbBusyDetect = false;
boolean blnTestMode3K = false; boolean blnTestMode6K = false; boolean blnInitializedFromEEPROM = false;

// Unsigned long
unsigned long ulngLastSpectrumUpdate = millis(); unsigned long ulngLastIQPlaneUpdate = millis();
unsigned long ulngLastDelayUpdateUs; unsigned long ulngCurrentElapsedTimeUs ; unsigned long ulngLastPPAvgTimeUs; unsigned long ulngLastSNUpdateUs;
unsigned long ulngPrevUpdateUs = micros(); unsigned long ulngBusyAvgStartMs = millis();

//Long
long lngENC1Old = 0; long lngENC2Old = 0; long lngENC1New = 0; long lngENC2New = 0; // For using Encoder2 library

//Int
int intA3history = 0;  int intTargetSN = 40; int intBandwidth = 3000;
int intMode = 0;// The main index 0 - 16. Always points to one of the 17 cases of chrModes
int intFadeRatePtr = 0; int intFMRatePtr = 0; int intFMDevPtr = 0;
int intFadeDepth_dB = 0;// Fade depth in dB (0 to 40 dB) bounded to Minimum S:N of -40dB
int intTuneOffset = 0;// Small (+/- 200 Hz max) frequency offset to allow check for tuning
int intGainLevel[] = {5, 5, 5, 5}; //Default gains to mid scale Gain = 2;
int intDelayToUpdate = 0; int intFFTCnt = 0; int intStartFFTus; int intBusyMode = 0; int intBinsAveraged = 0; int intNumofBinsToPlot = 0;
int intBusyFreqStep = 100; int intBusyBWLoHz = 0; int intBusyBWHiHz = 6500;  int intBusyGain = 10;
int intLastIQPlaneMode = -1; int intDetectSN = 0;
int intSerialCmdMode = -1; int intSerialCmdParam = -1; int intAvg = 10; int intSpecLow = 0; int intSpecHigh = 6500; int intThresh = 10;
int intTestFreqHz = 1548; int intDelayCount = 0; int intMultipaths = 2; 

static int intMaxSN = 40; static int intMinSN = -40; static int intMinGain = 0; static int intMaxGain = 10;  static int intMaxFade = 40;
static int intMinOffset = -200; static int intMaxOffset = 200; static int intOffsetStep = 10; static int intNumModes = 19;
static int intNumBusyModes = 11;

//Float
float fltppLPOutMeasAvg =0; float fltppLPOutMeasPk; float fltrmsLPOutMeasAvg =0; float fltrmsLPOutMeas;// Measurement of final LPOut filter (feeds mixChannels)
float fltVLF_UpFreq = 100; float fltVLF_Amp = 0; float fltFadeRate = 0;
float fltCurrentDelayI1Ms = 2.721088435374; float fltCurrentDelayI2Ms = 2.721088435374; //Minimum delays for I Channels (accomodates two 120 Tap Hilbert filters)
float fltCurrentDelayQ1Ms = 0; float fltCurrentDelayQ2Ms = 0;//minimum delays for Q channels (with two Hilbert filters)
float fltppWhiteOutMeasAvg; float fltUpdatedDelayMs; float fltMaxDelayMs;
float fltppInpSelectMeas; float fltppInpSelectMeasAvg; float fltPeakSum = 0; float fltNoiseSum = 0;
float fltFFT[151]; float fltMax = 0; float fltAvg = 0; float fltSum; float fltSortedBins[151];
float fltAvgBinsOfInt = 0.0;// the rolling average of all bins within the bins of interest (experimental for busy detector)
float fltppi2s0InAvg = 0.0; float fltppi2s1InAvg = 0.0; float fltppAmpRightOutAvg = 0.0; float fltppAmpLeftOutAvg = 0.0; //Testing and Cal
float fltMixI1Gain; float fltMixQ1Gain; float fltMixI2Gain; float fltMixQ2Gain;
static float fltCalTestLevel = .18796;// Value for calibration to set Peak measured level to 1500
static float fltNomInOutRatio = .41055;// Average calibration value for ratio Input to Output
static float fltMinimumIDelayMs = 1.360544217687; static float fltMinimumQDelayMs = 0;//  (~1.36 ms compensates for Hilbert Filter in Q path)

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

/* Hilbert Filter for 90 degree phase shift Designed with Scope FIR  using APM/Hilbert setup
  44100 Sampling freq, 120 taps, 350-6500 .5 dB ripple, 7500-22000 -66dB, -2dB@ 236 Hz
  These are half double presion Taps (other half is - mirror image.)*/

static double dblHilbert2FIR_120Tap_HalfCoeff[60] {
  0.000881953145261109, 0.002024424871630725, 0.003578420086304087, 0.004897684975669542,
  0.005243171847071985, 0.004221449919575678, 0.002257137562988763, 0.000520872693819941,
  0.000228893434912683, 0.001689755047144390, 0.003921959972007488, 0.005255726218007104,
  0.004639292854994107, 0.002595045001975806, 0.000960699993476102, 0.001419285244134233,
  0.003984560544401210, 0.006762245143275621, 0.007432617116201919, 0.005310300689905206,
  0.002207203843781383, 0.001094506652542682, 0.003520172792362594, 0.007965273505846096,
  0.010841535836344451, 0.009539156775449559, 0.005005829337420331, 0.001343844438067940,
  0.002422322403150740, 0.008223125596345290, 0.014338227741209422, 0.015486349079299244,
  0.010346423977458719, 0.003325955445192153, 0.001228307877826074, 0.007204195576890615,
  0.017311628071838642, 0.023222737952535349, 0.019431606723485428, 0.008812392562977806,
  0.001141764471221107, 0.004930460959419930, 0.019266225613026544, 0.033240967408886268,
  0.034665642965066079, 0.021336951060183018, 0.004819394947957467, 0.001989347053150126,
  0.020178557580697476, 0.048491049083078108, 0.064431056222591265, 0.052632444411865320,
  0.021175232268990984, 0.000448982458684410, 0.022074846941132709, 0.091787478858579630,
  0.177483996780199100, 0.224450246275797900, 0.190084888755790880, 0.074620640652942161
};

short shtHilbertFIR_120TapCoeff[120]; //This array will be populated from the above

void(* resetFunc)(void) = 0;//declare reset function at address 0

/*********************Function to compute IGain based on IDelay*******************************************/
 float IDelayToIGain(float fltIDelay)
 {  //Modified Rev 9_17B 5/28/2020 to center delay based on mode
    float fltNetDelay;
    if (intMode == 1){fltNetDelay = (fltIDelay-fltMinimumIDelayMs) -.5;}//Bias delay back to 0 center +/- 1 sigma
    else if (intMode == 2){fltNetDelay = (fltIDelay-fltMinimumIDelayMs) -1;}
    else if (intMode == 3){fltNetDelay = (fltIDelay-fltMinimumIDelayMs) -2;}
    else if (intMode == 4){fltNetDelay = (fltIDelay-fltMinimumIDelayMs) -5;}  
    else return 1; //abort case (shouldn't happen) 
    return cos(3.14159 * fltNetDelay/22.05);//compute gain factor based on Cos and 44100 sample rate
  }// End of Function IDelayToIGain************************************************************************* 

/*********************Function to compute QGain based on QDelay*******************************************/
 float QDelayToQGain(float fltQDelay)
 { //Modified Rev 9_17B 5/28/2020 to center delay based on mode
   float fltNetDelay; // delay biased to +/- 1 sigma of mode delay
    if (intMode == 1){fltNetDelay = (fltQDelay-fltMinimumIDelayMs) -.5;}//Bias delay back to 0 center +/- 1 sigma
    else if (intMode == 2){fltNetDelay = (fltQDelay-fltMinimumIDelayMs) -1;}
    else if (intMode == 3){fltNetDelay = (fltQDelay-fltMinimumIDelayMs) -2;}
    else if (intMode == 4){fltNetDelay = (fltQDelay-fltMinimumIDelayMs) -5;} 
    else return 0; //abort case (shouldn't happen) 
    return sin(3.14159 * fltNetDelay/22.05);//compute gain factor based on Sin and 44100 sample rate
 }// End of Function QDelayToQGain************************************************************************* 

/*****************Subroutine to adjust mixChannels gains to obtain desired S:N Simulation mode ***********/
void AdjustS_N (int intDesiredSN_dB, float fltppLPoutMeasAvg, float fltppWhiteMeasAvg)
{
  //Sets the appropriate gains of mixChannels inputs 0 and 1 to obtain the desired S:N
  double fltDesiredSNRatio = pow(10.000, (intDesiredSN_dB / 20.0));
  if (fltDesiredSNRatio >= 1)
  {
    //Signal >= Noise. (S:N db >= 0) fltDesiredSNRatio >= 1 Compute effective needed gain values for mixChannels Noise input
    mixChannels.gain(0, (fltppLPoutMeasAvg / fltppWhiteMeasAvg) / fltDesiredSNRatio);
    mixChannels.gain(1, 1); // may need to calibrate to insure no clipping/saturation
  }
  else
  {
    //Signal < Noise. (S:N db < 0) fltDesiredSNRatio < 1 Compute effective needed gain values for mixChannels Signal input
    mixChannels.gain(0, (fltppLPoutMeasAvg / fltppWhiteMeasAvg));
    mixChannels.gain(1, fltDesiredSNRatio);
  }
  mixChannels.gain(2, 0); //Insures no bleed through of mixChannels input 2
  return;
}//**End AdjustS_N******************************************************************************************

//*****************Subroutine to adjust mixChannels gains to obtain desired S:N for Busy Detector Testing ***
void AdjustBusyS_N (int intDesiredSN_dB, float fltppInpSelectMeasAvg , float fltppWhiteMeasAvg)
{
  //Serial.println("AdjustBusyS_N");
  //Sets the appropriate gains of mixChannels inputs 0 and 2 to obtain the desired S:N
  double fltDesiredSNRatio = pow(10.000, (intDesiredSN_dB / 20.0));
  if (fltDesiredSNRatio >= 1)
  {
    //Signal >= Noise. (S:N db >= 0) fltDesiredSNRatio >= 1 Compute effective needed gain values for mixChannels Noise input
    mixChannels.gain(0, (fltppInpSelectMeasAvg / fltppWhiteMeasAvg) / fltDesiredSNRatio);
    //Serial.print("AdjBusyS_N 0,A ");Serial.println((fltppInpSelectMeasAvg /fltppWhiteMeasAvg)/fltDesiredSNRatio);
    mixChannels.gain(2, 1); // may need to calibrate to insure no clipping/saturation
  }
  else
  {
    //Signal < Noise. (S:N db < 0) fltDesiredSNRatio < 1 Compute effective needed gain values for mixChannels Signal input
    mixChannels.gain(0, (fltppInpSelectMeasAvg / fltppWhiteMeasAvg));
    //Serial.print("AdjBusyS_N 0,B ");Serial.println(fltppInpSelectMeasAvg /fltppWhiteMeasAvg);
    mixChannels.gain(2, fltDesiredSNRatio);
  }
  mixChannels.gain(1, 0); // Insures no bleed through of mixChannels input 1
  return;
}//**End AdjustBusyS_N*************************************************************************************

//*********Subroutine For Fading **************************************************************************
//. Can be applied to all modes WGN-MPD. Adjusts mixChannels gains to obtain desired S:N and depth of fade
void Fade (int intMaxSN_dB, int intFadeDepth_dB, float fltFadeRateHz)
{
  //Sets the appropriate gains of mixerChannels inputs 0 and 1 to obtain the desired S:N
  // Appears OK 2_16 2020
  int intCurrentFadeSN_dB; 
  float fltRunTimeSec; fltRunTimeSec  = millis(); fltRunTimeSec = fltRunTimeSec / 1000
      ;//rollover only will happen every 50 days!
  intCurrentFadeSN_dB = intMaxSN_dB -  int((intFadeDepth_dB * .5 * (1 - cos(fltFadeRateHz * 2 * 3.14159 * fltRunTimeSec))));
  if (intCurrentFadeSN_dB < -40) {
    intCurrentFadeSN_dB = -40; // bound minimum fade to intMinSN
  }
  AdjustS_N (intCurrentFadeSN_dB , fltppLPOutMeasAvg, fltppWhiteOutMeasAvg);
  return;
}//**End Fade****************************************************************************************************

//********Function Update_Delay  (used to Update I1, I2, Q1, Q2 Delay line values)***********************************
float Update_Delay( int intChType, float fltMinDelayMs)
{
  // This function is used to compute the new delay of a delay line (fltCurrentChannelDelayMs) over a bounded range randomly
  // The frequency of updates has already been controlled by the Channel Type (MPG, MPM, MPP, MPD)
  // The absolute bounds of the calculated delay is bounded by the maximum delay for the Channel type and fltMinDelayMs.
  // The calculated delay will always be within the bounds and randomly vary within the 2 sigma range with a approximated normal (gaussian) distribution
  // When a new delay is computed (even if a no change) ulngTimeLastUpdateUs is updated

  if (intChType == 1)// MPG Channels
  {
    fltMaxDelayMs = fltMinDelayMs + 1;// Sets delay spread for 1ms for MPG (2 sigma)
    return  NormalDelay(fltMinDelayMs, fltMaxDelayMs);
  }
  if (intChType == 2)// MPM
  {
    fltMaxDelayMs = fltMinDelayMs + 2;// Sets delay spread for 2 ms for MPM (2 sigma)
    return NormalDelay(fltMinDelayMs, fltMaxDelayMs);
  }
  if (intChType == 3)// MPP
  {
    fltMaxDelayMs = fltMinDelayMs + 4;// Sets delay spread for 4 ms for MPP (2 sigma)
    return NormalDelay(fltMinDelayMs, fltMaxDelayMs);
  }
  if (intChType == 4)// MPD  (use 5 ms 1 sigma delay)
  {
    fltMaxDelayMs = fltMinDelayMs + 10;// Sets delay spread for 10ms for MPD (2 sigma)
    return NormalDelay(fltMinDelayMs, fltMaxDelayMs);
  }
  return fltMinDelayMs; // Return fltMinDelay if not intChType 1-4
}//******End Function Update_Delay*****************************************************************

//Function to return Delay with normal distribution ********************************************
// This generates a normal distributed (gaussian) delay between the 2 sigma range defined by fltMin to fltMax
float NormalDelay(float fltMin, float fltMax)
{
  // function to return a delay value in the range fltMin, to fltMax with normal distribution
  // Assumes fltMin and fltMax define the -1 sigma and + 1 sigma delays
  // This uses the Box-Muller method described in en.wikipedia.org/wiki/Normal_Ddistribution#Gererating_values_normal_distribution
  float fltRand1; float fltRand2;
  double dblFactor1;  double dblFactor2;
  float fltMean = .5 * (fltMin + fltMax);
  float fltTrial = -100000.0;// Initialize to a value outside acceptance range
  float flt3Sigma = abs(3 * (.5 * (fltMax - fltMin)));// 3 sigma value, always positive
  while ((fltTrial > fltMax) || (fltTrial < fltMin))// Exit while when fltTrial lies between fltMin and fltMax
  {
    fltRand1 = random(1, 10001) / 10000.0; //Random # >0 to 1
    fltRand2 = random(0, 10001) / 10000.0; // another different Random # 0 to 1
    dblFactor1  = sqrt(-2 * log(fltRand1)); //Note natural log.
    dblFactor2 = cos(2 * 3.14159 * fltRand2);// use Cos to spread dblFactor1 over a range -1 to +1 to approximate the gaussian distribution
    fltTrial = ((dblFactor1 * dblFactor2) *  flt3Sigma) + fltMean;// about 68% of fltTials should fall within the 2 sigma  range fltMin to fltMax
  }
  return fltTrial;
}//End function to return normal distribution **********************************************************

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
    if ((intParam >= 2) && (intParam <= 4))
    {
      intMultipaths = intParam;
      str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 = "";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
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
    for (int j = 0; j < 11; j += 1)
      if  ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMDevPtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, str3);
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
  if (intMode == 9)// FM DEVIATION  int intFMRatePtr = 0; int intFMDevPtr = 0;
    {
      for (int j = 0; j < 11; j ++)
        if  ((abs(fltLogs[j] - fltParam)) < .01)
          {
            intFMDevPtr = j;
            str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
            UpdateTFTModeParameter(str1, str2, str3);
            return true;
          }
    }
  if (intMode == 10)//FM RATE fltFMRates[]={0.0,.1,.3,1.0,3.0,10.0,30.0,100.0};
  {

    for (int j = 0; j < 8; j += 1)
      if ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMRatePtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, str3);
        return true;
      }
  }
  if ((intMode >= 11) && (intMode <= 14)) //IN and OUT levels
  {
    intParam = strParameter.toInt();
    if ((intParam >= 0) && (intParam <= 20))
    {
      intGainLevel[intMode - 11] = intParam;
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

//***Subroutine for Ploting I and Q delays ******************************************************************

void PlotIQPlane(float DlyI1, float DlyQ1, float DlyI2, float DlyQ2, int Mode)
// this plots two points on the IQ plane on the display by drawing the I and Q axis and labeling Mode and delay spreads.
// To eliminate/reduce "ficker" a full repaint is done only when IQinit is true
// Otherwise the last plotted points are blacked out (replotted with black), the new points are plotted, the Axis redrawn
{

  static int lastI1 = 0; static int lastQ1 = 0; static int lastI2 = 0; static int lastQ2 = 0; // The last plotted pair co ordinates.
  static int intPixPerMs = 0;  String strDisplay ;
  static int intPlotCtrY = 143; static int intPlotCtrX = 161; static float fltHalfRange;

  if ((!blnPlotIQPlane) || (Mode < 1) || (Mode > 4)) {
    return;
  }
  if (Mode != intLastIQPlaneMode )// Initialize screen on  mode change
  { // Set up the display to show IQ Plane, the mode, and delay spread
    if (Mode == 1) {
      strDisplay = "MPG +/-.5ms  Scale: +/-5ms";
      intPixPerMs =  80 / 5;
      fltHalfRange = .5;
    }
    else if (Mode == 2) {
      strDisplay = "MPM +/- 1ms  Scale: +/-5ms";
      intPixPerMs =  80 / 5;
      fltHalfRange = 1.0;
    }
    else if (Mode == 3) {
      strDisplay = "MPP +/- 2ms  Scale: +/-5ms";
      intPixPerMs = 80 / 5;
      fltHalfRange = 2.0;
    }
    else if (Mode == 4) {
      strDisplay = "MPD +/- 5ms  Scale: +/-5ms";
      intPixPerMs = 80 / 5;
      fltHalfRange = 5.0;
    }
    else return;//This subroutine is irrelevent for all other modes
    intLastIQPlaneMode = Mode;
    //black out the screen and lable the plot type, Mode, and delay spread
    tft.begin();  tft.setRotation(1);  tft.fillScreen(ILI9341_BLACK);   tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(3);  tft.println("IQ Plane:" + String(intMultipaths) + " paths");
    tft.setTextColor(ILI9341_WHITE);tft.setTextSize(2);  tft.println(strDisplay);
  }
  else if (lastI1 != 0) // Black out the old plots leaving the headers etc.
  {
    tft.fillCircle(lastI1, lastQ1, 5, ILI9341_BLACK);
    tft.fillCircle(lastI2, lastQ2, 5, ILI9341_BLACK);
  }
  //Plot the two delay values and draw the axis
  if (intMultipaths == 4)
    {
      lastI1 = (intPixPerMs * (DlyI1 - fltMinimumIDelayMs - fltHalfRange)) + intPlotCtrX; //Center of Plot X axis
      lastI2 = (intPixPerMs * (DlyI2 - fltMinimumIDelayMs - fltHalfRange)) + intPlotCtrX; //Center of Plot X axis
      lastQ1 = (intPixPerMs * (DlyQ1 - fltMinimumQDelayMs - fltHalfRange)) + intPlotCtrY; //Center of Plot Y axis
      lastQ2 = (intPixPerMs * (DlyQ2 - fltMinimumQDelayMs - fltHalfRange)) + intPlotCtrY; //Center of Plot Y axis
      tft.fillCircle(lastI1, lastQ1, 5, ILI9341_GREEN);// Chan 1 IQ delay plot filled green circle with radius =5
      tft.fillCircle(lastI2, lastQ2, 5, ILI9341_WHITE);// Chan 2 IQ delay plot filled white circle with radius =5
    } 
  if (intMultipaths == 3)
    {
      lastI1 = (intPixPerMs * (DlyI1 - fltMinimumIDelayMs - fltHalfRange)) + intPlotCtrX; //Center of Plot X axis
      lastI2 = (intPixPerMs * (DlyI2 - fltMinimumIDelayMs - fltHalfRange)) + intPlotCtrX; //Center of Plot X axis
      lastQ1 = (intPixPerMs * (DlyQ1 - fltMinimumQDelayMs - fltHalfRange)) + intPlotCtrY; //Center of Plot Y axis
      lastQ2 = intPlotCtrY;
      tft.fillCircle(lastI1, lastQ1, 5, ILI9341_GREEN);// Chan 1 IQ delay plot filled green circle with radius =5
      tft.fillCircle(lastI2, intPlotCtrY, 5, ILI9341_WHITE);// Chan 2 IQ delay plot filled white circle with radius =5
    }
  if (intMultipaths == 2)
    {
      lastI1 = (intPixPerMs * (DlyI1 - fltMinimumIDelayMs - fltHalfRange)) + intPlotCtrX; //Center of Plot X axis
      lastQ1 = (intPixPerMs * (DlyQ1 - fltMinimumQDelayMs - fltHalfRange)) + intPlotCtrY; //Center of Plot Y axis
      tft.fillCircle(lastI1, lastQ1, 5, ILI9341_GREEN);// Chan 1 IQ delay plot filled green circle with radius =5
    }
  
  //Draw the axis lines in yellow (axis will over ride plots but not block them out totally)[These axis centered well]
  for (int y = 142; y < 145; y += 1 ) tft.drawFastHLine(50, y, 210, ILI9341_YELLOW); //X axis 3 pixels wide
  for (int x = 160; x < 163; x += 1 ) tft.drawFastVLine(x, 50, 210, ILI9341_YELLOW); //Y axis 3 pixels wide
  return;
}// End PlotIQPlane *******************************************************************************

// Subroutine for Setting Filter Bandwith**********************************************************
void SetFilterBandwidth(int intBW)
{
  if (!((intBW == 3000) || (intBW == 6000))) {return;} // intBW must be 3000 or 6000
  if (intBW == 3000)
  {
    //Stop all LP filters
    //Serial.println("SetFilterBandwdith 3000 Hz");
    filLPIn_0_FIR.end();
    filLPOutFIR.end();
    filLPWhiteFIR.end();
    filLPDownMix.end();
    filLPIn_0_FIR.begin(sht3KHzLPFIRCoeffRev2, 120); // Start Input LP Filter in 3000 Hz BW mode
    filLPOutFIR.begin(sht3KHzLPFIRCoeffRev2, 120); // MixIQ output LP Filter
    filLPWhiteFIR.begin(sht3KHzLPFIRCoeffRev2, 120); // White Noise LP Filter
    filLPDownMix.begin(sht3KHzLPFIRCoeffRev2, 120); // Down Mix LP Filter
  }
  else
  {
    //Stop all LP Filters
    //Serial.println("SetFilterBandwdith 6000 Hz");
    filLPIn_0_FIR.end();
    filLPOutFIR.end();
    filLPWhiteFIR.end();
    filLPDownMix.end();
    filLPIn_0_FIR.begin(sht6KHzLPFIRCoeffRev2, 120); // Start Input LP Filter in 6000 Hz BW mode
    filLPOutFIR.begin(sht6KHzLPFIRCoeffRev2, 120); // MixIQ output LP Filter
    filLPWhiteFIR.begin(sht6KHzLPFIRCoeffRev2, 120); // White Noise LP Filter
    filLPDownMix.begin(sht6KHzLPFIRCoeffRev2, 120); // Down Mix LP Filter
  }
  return;
}//END SetFilterBandwidth ************************************************************************

//******Function to Parse Simulator Mode received via Serial Port *********************************
int ParseSimMode (String strMode)
{
  // Returns mode index (0-15) if strMode is legitimate, -1 other wise (case insensitive)
  for (int i = 0; i < (intNumModes); i++) //Search through the available Modes and Parameters TEST3K, TEST6K not used. 
  {
    String strTemp = chrModes[i];
    if  (strTemp.startsWith(strMode.toUpperCase()))
    {
      if (strTemp.startsWith("BUSY:"))
      {
        blnSim = false; //blnInitialized = false;  blnEnableTestTone = false;
        InitializeBusy();
        //blnPlotSpectrum = false; blnInitSpectrum = false; blnPlotIQPlane = false;
        //setup();
        //mixChannels.gain(0, 0.0); mixChannels.gain(1, 0.0); mixChannels.gain(2, 1.0); mixChannels.gain(3, 0.0);
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
        blnPlotSpectrum = false; blnInitSpectrum = false; blnPlotIQPlane = false;
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
  if (intMode < 5)//WGN thru MPD
  {
    intParam = strParameter.toInt();
    if ((-40 <= intParam) && (intParam <= 40))
    {
      //Serial.print(" ENC1 intTargetSN= ");Serial.println(intTargetSN);;
      intTargetSN = intParam;
      str1 = chrModes[intMode];  str2 = "   S:N= " + String(intTargetSN); str3 = " dB";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
  }
  if (intMode == 5)//MULTIPATHS
  {
    if ((intParam >= 2) && (intParam <= 4))
    {
      intMultipaths = intParam;
      str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 =  "";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
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
    for (int j = 0; j < 11; j ++)
      if  ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMDevPtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, str3);
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
  if (intMode == 9)// FM DEVIATION  int intFMRatePtr = 0; int intFMDevPtr = 0;
  {
    for (int j = 0; j < 11; j ++)
      if  ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMDevPtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, str3);
        return true;
      }
  }
  if (intMode == 10)//FM RATE 
  {

    for (int j = 0; j < 11; j ++)
      if ((abs(fltLogs[j] - fltParam)) < .01)
      {
        intFMRatePtr = j;
        str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[j]) ; str3 =  " Hz";
        UpdateTFTModeParameter(str1, str2, str3);
        return true;
      }
  }
  if ((intMode >= 11) && (intMode <= 14)) //IN and OUT levels
  {
    intParam = strParameter.toInt();
    if ((intParam >= 0) && (intParam <= 10))
    {
      intGainLevel[intMode - 11] = intParam;

      ampLeftOut.gain(fltLogs[intGainLevel[intMode - 11]]);

      str1 = chrModes[intMode]; str2 = "      " + String(fltLogs[intParam]) ; str3 =  "";
      UpdateTFTModeParameter(str1, str2, str3);
      if (intMode == 11) {
        mixInpSel.gain(2, fltLogs[intParam]);
        mixInpSel.gain(0, 0); //Serial.println("line 984 mixInpSel Gain(0,0)");
      }
      if (intMode == 12) {
        mixInpSel.gain(3, fltLogs[intParam]);
        mixInpSel.gain(0, 0);  //Serial.println("line 988 mixInpSel Gain(0,0)");
      }
      if (intMode == 13) {
        ampLeftOut.gain(fltLogs[intParam]);
      }
      if (intMode == 14) {
        ampRightOut.gain(fltLogs[intParam]);
      }
      return true;
    }
  }
  if (intMode == 15)//Bandwidth
  {
    intParam = strParameter.toInt();
    if ((intParam == 3000) || (intParam == 6000))
    {
      SetFilterBandwidth(intParam);
      intBandwidth = intParam;
      str1 = chrModes[intMode]; str2 = "      " + String(intParam) ; str3 =  " Hz";
      UpdateTFTModeParameter(str1, str2, str3);
      return true;
    }
    else {
      return false;
    }
  }
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
      //Serial.print(" ENB BUSY: ");Serial.println(intParam);
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
      //Serial.print(" DIS BUSY: ");Serial.println(intParam);
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
      InputTestWaveform.frequency (intParam); InputTestWaveform.amplitude(fltCalTestLevel);
      //Serial.print("line 1086 set InputTestWaveform amp = "); Serial.println(fltCalTestLevel);
      mixInpSel.gain(0, 2); mixInpSel.gain(1, 0); mixInpSel.gain(2, 0); mixInpSel.gain(3, 0);
      //Serial.println("line 1088 mixInpSel Gain(0,2)");
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
      mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intGainLevel[0]]); mixInpSel.gain(3, 0);// Serial.println("line 1099 mixInpSel Gain(0,0)");
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
      mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intParam]); mixInpSel.gain(3, 0);// Serial.println("line 1093 mixInpSel Gain(0,0)");
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
    //Serial.print("MaxS_N=");Serial.print(intMaxS_NdB);Serial.print("  MinS_N=");Serial.println(intMinS_NdB);
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
{ // Only called from manual Right Encoder Push While displaying modes 5-14
  int intAdd = intMode - 4;

  Serial.print("EEPROM SAVE: mode =");Serial.print(intMode);Serial.print(" val= ");
  //Modes 0-4 (WGN-MPD) not saved as Right Encoder Push toggles 1500 Hz tone on/off.  So S:N values not saved/restored.
  // These confirmed based on Serial.print (remove print statements after debug).
  if (intMode == 5) {
    EEPROM.write(intAdd, byte(intMultipaths));  //  intMultipaths 2, 3, 4
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
  for (int i = 1; i <= 11; i++) //Read 11 initialization values and verify in range
  {
    bytParameter = EEPROM.read(i);
    //Serial.print("Init From EEPROM(");Serial.print(i);Serial.print(") = ");Serial.println(bytParameter);//For debug
    if ((i == 1) && (bytParameter < 5)) {intMultipaths = int(bytParameter) ;}
    else if ((i == 2) && (bytParameter < 41)) { intFadeDepth_dB = int(bytParameter) ;}
    else if ((i == 3) && (bytParameter < 11)) {fltFadeRate = fltLogs[int(bytParameter)];}
    else if ((i == 4) && (bytParameter < 41)) {intTuneOffset = 10 * (int(bytParameter) - 20);}
    else if ((i == 5) && (bytParameter < 11)) {intFMDevPtr = int(bytParameter);}
    else if ((i == 6) && (bytParameter < 11)) {intFMRatePtr = int(bytParameter);}
    else if (((i > 6) && (i < 11)) && (bytParameter < 11)) {intGainLevel[i - 7] = int(bytParameter);}
    else if ((i == 11) &&  ((bytParameter == 30) || (bytParameter == 60))) {intBandwidth = 100 * int(bytParameter);}
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
  Serial.end(); Serial.begin(9600);

// Pins for Rotary Encoders RE1(modes) and RE2(parameters)
  pinMode(0, INPUT_PULLUP);//RENC1a
  pinMode(1, INPUT_PULLUP);//RENC1b
  pinMode(3, INPUT_PULLUP);//RENC2a
  pinMode(4, INPUT_PULLUP);//RENC2b
  sgtl5000_1.enable(); sgtl5000_1.volume(.2); // keep volume low for headphones
  AudioMemory(50);
  whiteOut.amplitude(1.0);
  fft1024.windowFunction(AudioWindowHanning1024); // FFT Testing Default Window is Hanning
 
  mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0); //Serial.println("Line 1364 mixAudioSelect");//SetmixAudioSeect to bypass SSB Shifter
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(4);  tft.println("IONOS SIM      ");
  tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(strRevision);
  //this prints out a 8 digit hex SN unique to the Specific teensy  
  tft.setTextColor(ILI9341_MAGENTA);tft.setCursor(0,180);tft.print(" Serial: "); tft.printf("%08X",OCOTP_CFG0);
  delay(4000);
  Serial.print("IONOS SIM ");Serial.print(strRevision); Serial.print(" Serial: ");Serial.printf("%08X",OCOTP_CFG0);
  //This sets up all non static variables as the would be upon power on to eanble clean startup from SIM: command
  strMode = ""; strParameter= "";
  
  blnSim = true;  blnInitialized = false; blnEnableTestTone = false; blnPlotSpectrum = false; blnInitSpectrum = false; blnPlotIQPlane = false;
  blnModes = true; blnInitModes = true; blnColon = false;  blnChanBusySent = false;  blnChanClearSent = false; blnPlotBusyRed = false;  blnEnbBusyDetect = false;
  blnTestMode3K = false; blnTestMode6K = false; blnInitializedFromEEPROM = false;

  ulngLastSpectrumUpdate = millis(); ulngLastIQPlaneUpdate = millis(); ulngPrevUpdateUs = micros(); ulngBusyAvgStartMs = millis();

  lngENC1Old = 0; lngENC2Old = 0; lngENC1New = 0; lngENC2New = 0; 

  intA3history = 0;  intTargetSN = 40; intBandwidth = 3000; intMode = 0; intFadeRatePtr = 0; intFMRatePtr = 0; intFMDevPtr = 0;intFadeDepth_dB = 0;
  intTuneOffset = 0; intGainLevel[0] = 5; intGainLevel[1] = 5; intGainLevel[2]= 5; intGainLevel[3]= 5;  
  intDelayToUpdate = 0; intFFTCnt = 0; intBusyMode = 0; intBinsAveraged = 0; intNumofBinsToPlot = 0;
  intBusyFreqStep = 100; intBusyBWLoHz = 0; intBusyBWHiHz = 6500;  intBusyGain = 10; intLastIQPlaneMode = -1; intDetectSN = 0; intSerialCmdMode = -1; 
  intSerialCmdParam = -1; intAvg = 10; intSpecLow = 300; intSpecHigh = 2800; intThresh = 10; intTestFreqHz = 1548; intDelayCount = 0; intMultipaths = 4;

  fltVLF_UpFreq = 100; fltVLF_Amp = 0; fltFadeRate = 0; fltCurrentDelayI1Ms = 2.721088435374; fltCurrentDelayI2Ms = 2.721088435374; 
  fltCurrentDelayQ1Ms = 0; fltCurrentDelayQ2Ms = 0; fltPeakSum = 0; fltNoiseSum = 0; fltMax = 0; fltAvg = 0; fltAvgBinsOfInt = 0.0;
  fltppi2s0InAvg = 0.0; fltppi2s1InAvg = 0.0; fltppAmpRightOutAvg = 0.0; fltppAmpLeftOutAvg = 0.0;
// End of basic initialization.
  InitializeParametersFromEEPROM();delay(1000);
  SetFilterBandwidth(intBandwidth);
 // Initialize the tap coefficients for the 120 tap Hilbert Filter
  for (int i = 0; i < 60; i ++)
  { // Compute mirror image coefficients and scale to work with Teensy FIR short values
    // Reverse order of coefficients for Teensy FIR (put negative values first) FIR will reverse order
    shtHilbertFIR_120TapCoeff[i] = dblHilbert2FIR_120Tap_HalfCoeff[i] * (-30880); //scale by -30880 for trial amplitude. Negative coeff first half (FIR will reverse order).
    shtHilbertFIR_120TapCoeff[120 - i] = -shtHilbertFIR_120TapCoeff[i]; // mirror positive values last half, FIR will reverse order
  }
  // Initialize the Hilbert FIR. A single 6.5 KHz Hilberter 120 tap filter for both 3KHz and 6KHz bandwidths.
  filHilQ_12.end();
  filHilQ_12.begin(shtHilbertFIR_120TapCoeff, 120); // Hilbert FIR filter for Ch1 & 2 Q paths
  if (intTuneOffset == 0)//Don't use Up/Dn Mixer
    {
      sine_Dnmix.frequency(7500);
      mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1413 mixAudioSelect");
    }
  else//Use Up/Dn mixer to shift frequency
    {
      filHP7500FIR.end(); sine_Dnmix.frequency(7500); sine_Dnmix.amplitude(1.0); sine_Upmix.frequency(7500); sine_Upmix.amplitude(1.0);
      filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
      sine_VLF_Dnmix_Mod.amplitude(0);
      sine_Dnmix.frequency(7500 - intTuneOffset);
      mixAudioSelect.gain(0, 0); mixAudioSelect.gain(1, 4); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1421 mixAudioSelect");
    }
  ampRightOut.gain(fltLogs[intGainLevel[3]]); ampLeftOut.gain(fltLogs[intGainLevel[2]]);// default gain (half scale)[range = 0 to 2]
  delayQ.delay(0, 13.017); delayQ.delay(1, 0); delayQ.delay(2, 0); // 0 delays on Q channels (Hilbert filters already contribute 2.721088 ms
  delayI.delay(0, 13.017); delayI.delay(1, 2.72108844); delayI.delay(2, 2.72108844); // 2.72108844 ms (120 taps) delay on I channels to offset the 2 Hilbert filters on Q channels
  fltCurrentDelayI1Ms = 2.72108844; fltCurrentDelayI2Ms = 2.721088444; //Minmum delays (2.72 ms accounts for two Hilbert Filters on Q legs)
  fltCurrentDelayQ1Ms = 0; fltCurrentDelayQ2Ms = 0;//Minimum Delays (0 delay equivalent to 2.72 ms on I paths due to Hilbert Filters on Q legs
  if (intMultipaths == 2){mixIQ.gain(0, -.5); mixIQ.gain(1, .5); mixIQ.gain(2, 0); mixIQ.gain(3, 0);}
  if (intMultipaths == 3){mixIQ.gain(0, -.333); mixIQ.gain(1, .333); mixIQ.gain(2, 0); mixIQ.gain(3,.333);}
  if (intMultipaths == 4){mixIQ.gain(0, -.25); mixIQ.gain(1, .25); mixIQ.gain(2, -.25); mixIQ.gain(3, .25);}
  mixChannels.gain(0, 1); mixChannels.gain(1, 1); mixChannels.gain(2, 0); mixChannels.gain(3, 0);//Only inputs 0 and 1 are used for SIM.
  mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intGainLevel[0]]); mixInpSel.gain(3, fltLogs[intGainLevel[1]]);
  ulngCurrentElapsedTimeUs = micros(); ulngLastDelayUpdateUs = ulngCurrentElapsedTimeUs; ulngLastPPAvgTimeUs = ulngCurrentElapsedTimeUs;
  intFMRatePtr = 0; 
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
  fltCurrentDelayI1Ms = 2.721088435374; 
  mixIQ.gain(0, 0); mixIQ.gain(1, 1); mixIQ.gain(2, 0); mixIQ.gain(3,0);
  mixInpSel.gain(0,0); mixInpSel.gain(1,0); mixInpSel.gain(2,fltLogs[intGainLevel[0]]); mixInpSel.gain(3,0);
  filLPWhiteFIR.begin(sht3KHzLPFIRCoeffRev2,120); // White Noise LP Filter
  intFMRatePtr = 0;
  fltFadeRate = 0;
  InputTestWaveform.frequency (1500); InputTestWaveform.amplitude(.5);
  SetFilterBandwidth(intBandwidth);
  filHilQ_12.end();
  filHP7500FIR.end(); 
} //End InitializeBusy ****************************************************************************************

//   Main Loop***********************************************************************************************
void loop()
{
   String str1, str2, str3;
  //Code to update Rotary Encoder 2 (modes)
  lngENC2New = ENC2.read();//Encoder 2 controls Modes (Left hand knob)
  //Serial.print("lngENC2Old = ");Serial.print(lngENC2Old); Serial.print("  lngENC2New = ");Serial.println(lngENC2New);
  if (lngENC2Old != lngENC2New)
    {
      if (blnSim)
        {
          intMode = intMode + (lngENC2New - lngENC2Old);
          if (intMode > 17) 
            {
              intMode = 17; //Hold at limit
            }
          if (intMode < 0 ) 
            {
              intMode = 0;
            }
          if (intMode < 16)
            {
              blnTestMode3K = false; blnTestMode6K = false;  blnEnableTestTone = false;
              InputTestWaveform.amplitude(0);
              mixInpSel.gain(0, 0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, fltLogs[intGainLevel[0]]); mixInpSel.gain(3, fltLogs[intGainLevel[1]]);
              ampLeftOut.gain(fltLogs[intGainLevel[2]]);//sets ampLeftOut gain (0-100 (log))
              ampRightOut.gain(fltLogs[intGainLevel[3]]);//sets ampRightOut gain (0-100 (log))
              if (intMode == 0)
                {
                  mixIQ.gain(0, 0); mixIQ.gain(1, 1); mixIQ.gain(2, 0); mixIQ.gain(3, 0);// Disable Hilbert transformers and set gains
                  blnModes = true;
                }
            }
          else if ((intMode >= 1) && (intMode <= 4))
            {
              if (intMultipaths == 2){mixIQ.gain(0, -.5); mixIQ.gain(1, .5); mixIQ.gain(2, 0); mixIQ.gain(3, 0);}
              if (intMultipaths == 3){mixIQ.gain(0, -.333); mixIQ.gain(1, .333); mixIQ.gain(2, 0); mixIQ.gain(3,.333);}
              if (intMultipaths == 4){mixIQ.gain(0, -.25); mixIQ.gain(1, .25); mixIQ.gain(2, -.25); mixIQ.gain(3, .25);}
              blnModes = true;
            }
        }
      //Serial.print("intMode= ");Serial.println(intMode);
      tft.begin();
      tft.setRotation(1);
      tft.fillScreen(ILI9341_BLACK);
      if ((intMode == 0) || (! blnInitialized))
        {
          fltCurrentDelayI1Ms = 2.72108844; delayI.delay(1, fltCurrentDelayI1Ms); delayI.delay(2, fltCurrentDelayI1Ms); //minimum delay
          delayQ.delay(1, 0.0); delayQ.delay(2, 0.0);
        }
      if ((intMode >= 0) && (intMode <= 4))//WGN,MPG,MPM,MPP,MPD
        {
          str1 = chrModes[intMode];  str2 = "   S:N= "; str3 = " dB";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTargetSN + str3);
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
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + fltFadeRate + str3);
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
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltLogs[intFMDevPtr]) + str3);
        }
      if (intMode == 10) //FM Rate
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltLogs[intFMRatePtr]) + str3);
        }

      if ((intMode >= 11) && (intMode < 15)) //Ch1, Ch2, IN; Ch1, Ch2 Out
        {
          str1 = chrModes[intMode];  str2 = "    ";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + fltLogs[intGainLevel[intMode - 11]]);
        }

      if (intMode == 15)//BANDWIDTH
        {
          str1 = chrModes[intMode];  str2 = "    "; str3 = " Hz";
          tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
          tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intBandwidth + str3);
        }
      if (intMode == 16)//TEST3K
        {
          blnTestMode3K = true; blnTestMode6K = false; intTestFreqHz = 1548; intTargetSN = 40;
          // Set gains to over ride other mode settings:
          ampLeftOut.gain(2.0); ampRightOut.gain(1.0);
          mixInpSel.gain(0, 2.0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
          mixIQ.gain(0, 0); mixIQ.gain(1, 1); mixIQ.gain(2, 0); mixIQ.gain(3, 0);// Disable Hilbert transformers and set gains
          if (!(intBandwidth == 3000)); intBandwidth = 3000; SetFilterBandwidth(intBandwidth);
          sine_VLF_Dnmix_Mod.amplitude(0);
          InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
          mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1548 mixAudioSelect");
        }
      if (intMode == 17)//TEST6K
        {
          blnTestMode6K = true; blnTestMode3K = false; intTestFreqHz = 3096; intTargetSN = 40;
          // Set gains to over ride other mode settings:
          ampLeftOut.gain(1.0); ampRightOut.gain(2.0);
          mixInpSel.gain(0, 2.00); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
          mixIQ.gain(0, 0); mixIQ.gain(1, 1); mixIQ.gain(2, 0); mixIQ.gain(3, 0);// Disable Hilbert transformers and set gains
          sine_VLF_Dnmix_Mod.amplitude(0);
          if (!(intBandwidth == 6000)); intBandwidth = 6000; SetFilterBandwidth(intBandwidth);
          InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
          mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1560 mixAudioSelect");
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
              if ((intMode == 0) || (intMode == 16) || (intMode == 17)) //Disable Hilbert filter inputs on modes (WGN, TEST3K, TEST6K)
                {
                  mixIQ.gain(0, 0); mixIQ.gain(1, 1); mixIQ.gain(2, 0); mixIQ.gain(3, 0); // Disable Hilbert transformers and set gains
                }
              else//Enable Hilbert transformers and set gains
                {
                  if (intMultipaths == 2){mixIQ.gain(0, -.5); mixIQ.gain(1, .5); mixIQ.gain(2, 0); mixIQ.gain(3, 0);}
                  if (intMultipaths == 3){mixIQ.gain(0, -.333); mixIQ.gain(1, .333); mixIQ.gain(2, 0); mixIQ.gain(3,.333);}
                  if (intMultipaths == 4){mixIQ.gain(0, -.25); mixIQ.gain(1, .25); mixIQ.gain(2, -.25); mixIQ.gain(3, .25);}
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
              str1 = chrModes[intMode];  str2 = "   S:N= "; str3 = " dB";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTargetSN + str3);
            }
          if (intMode == 5)//MULTIPATHS
            {
              intMultipaths = intMultipaths + (lngENC1New - lngENC1Old);
              if (intMultipaths> 4) { intMultipaths = 4;} //Hold at stops
              if (intMultipaths <2)  { intMultipaths = 2;} //Hold at stops
              str1 = chrModes[intMode]; str2 = "    " ; 
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3); tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intMultipaths );
              if (intMultipaths == 2){mixIQ.gain(0, -.5); mixIQ.gain(1, .5); mixIQ.gain(2, 0); mixIQ.gain(3, 0);} //Serial.println("#1633, -.5,.5, 0, 0");
              if (intMultipaths == 3){mixIQ.gain(0, -.333); mixIQ.gain(1, .333); mixIQ.gain(2, 0); mixIQ.gain(3,.333);}//Serial.println("#1634, -.33,.33, 0, .33");
              if (intMultipaths == 4){mixIQ.gain(0, -.25); mixIQ.gain(1, .25); mixIQ.gain(2, -.25); mixIQ.gain(3, .25);}//Serial.println("#1635, -.25,.25, -.25, .25");
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
              if (intFadeRatePtr > 10) {intFadeRatePtr = 10;} //Hold at stops
              if  (intFadeRatePtr < 0) {intFadeRatePtr = 0;}
              fltFadeRate = fltLogs[intFadeRatePtr];
              str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltFadeRate)  + str3);
            }
          if (intMode == 8)//OFFSET
            {
              intTuneOffset = intTuneOffset + (intOffsetStep * (lngENC1New - lngENC1Old));
              if (intTuneOffset > intMaxOffset) {intTuneOffset = intMaxOffset;} //Hold at stops
              if (intTuneOffset < intMinOffset) {intTuneOffset = intMinOffset;}
              if (intTuneOffset == 0)//Don't use Up/Dn Mixer
                {
                  sine_Dnmix.frequency(7500);
                  mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1666 mixAudioSelect");
                }
              else//Use Up/Dn mixer to shift frequency
                {
                  filHP7500FIR.end();
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  sine_VLF_Dnmix_Mod.amplitude(0);sine_Dnmix.frequency(7500 - intTuneOffset); sine_Dnmix.amplitude(1.0); sine_Upmix.frequency(7500); sine_Upmix.amplitude(1.0);
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  mixAudioSelect.gain(0, 0); mixAudioSelect.gain(1, 4); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1651 mixAudioSelect");
                }
              str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
              tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
              tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + intTuneOffset + str3);
            }
          if (intMode == 9) //FM DEVIATION
            {
              intFMDevPtr = intFMDevPtr + (lngENC1New - lngENC1Old);
              if (intFMDevPtr > 10) {intFMDevPtr = 10;} //Hold at stops
              if (intFMDevPtr < 0) {intFMDevPtr = 0;}
              if (fltLogs[intFMDevPtr] <.01)
                {
                  mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1664 mixAudioSelect");
                }
              else
                {
                  filHP7500FIR.end();// Serial.println("FM DEVIATION line 1668");
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .00013333333333 ); //sets max deviation in Hz e.g. .000133333333 * 7500  yields +/- 1 Hz peak deviation
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  mixAudioSelect.gain(0, 0); mixAudioSelect.gain(1, 4); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1672 mixAudioSelect");
                }
            str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltLogs[intFMDevPtr]) + str3);
          }
        if (intMode == 10)//FM RATE
          {
            intFMRatePtr = intFMRatePtr + (lngENC1New - lngENC1Old);
            if (intFMRatePtr > 10) {intFMRatePtr = 10;} //Hold at stops
            if (intFMRatePtr < 0) {intFMRatePtr = 0;}
            if (intFMRatePtr == 0)
              {
                mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1685 mixAudioSelect");
              }
            else
              {
                filHP7500FIR.end();
                filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .00013333333333 ); //sets max deviation in Hz e.g. .000133333333 * 7500  yields +/- 1 Hz peak deviation
                filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                sine_VLF_Dnmix_Mod.frequency(fltLogs[intFMRatePtr]);
                mixAudioSelect.gain(0, 0); mixAudioSelect.gain(1, 4); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 1694 mixAudioSelect");
              }
            str1 = chrModes[intMode]; str2 = "    " ; str3 = " Hz";
            tft.setTextColor(ILI9341_CYAN);  tft.setTextSize(3);  tft.println(str1);
            tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(str2 + String(fltLogs[intFMRatePtr]) + str3);
          }
        if ((intMode > 10) && (intMode < 15)) //GAINS
          {
            intGainLevel[intMode - 11] = intGainLevel[intMode - 11] + (lngENC1New - lngENC1Old);
            if (intGainLevel[intMode - 11] > intMaxGain) { intGainLevel[intMode - 11] = intMaxGain;} //Hold at stops
            if (intGainLevel[intMode - 11] < intMinGain ) {intGainLevel[intMode - 11] = intMinGain;}
            //This uses log gain values 0.0 to 100 starting Rev 7_3_4  on 4/2/2020
            if (intMode < 13) {mixInpSel.gain((intMode - 9), (fltLogs[intGainLevel[intMode - 11]]));} //sets input mixer gain (0-100 (log))
            else if (intMode == 13) {ampLeftOut.gain(fltLogs[intGainLevel[intMode - 11]]); }//sets ampLeftOut gain (0-100 (log))
            else if (intMode == 14) {ampRightOut.gain(fltLogs[intGainLevel[intMode - 11]]);} //sets ampRightOut gain (0-100 (log))
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
        if (intMode == 16)//TEST3K (Sine)
          {
            SetFilterBandwidth(3000);
            intTestFreqHz = intTestFreqHz + 86 * (lngENC1New - lngENC1Old); //Inc/Dec by 86 Hz per click
            if (intTestFreqHz > 3182) {intTestFreqHz = 3182; }//Hold at stops
            if (intTestFreqHz < 86) { intTestFreqHz = 86;} //Hold at 86 Hz. Lower limit is ~40 Hz at -.5dB (-1.5 dB @10 Hz)  with 10 uf Caps.
            intBandwidth = 3000; intTuneOffset = 0;
            InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
            mixInpSel.gain(0, 2); mixInpSel.gain(1, 0); mixInpSel.gain(2, 0); mixInpSel.gain(3, 0);
          }
        if (intMode == 17)//TEST6K (Sine)
          {
            SetFilterBandwidth(6000);
            intTestFreqHz = intTestFreqHz + 172 * (lngENC1New - lngENC1Old); //Inc/Dec by 172 Hz per click
            if (intTestFreqHz > 6192) {intTestFreqHz = 6192;} //Hold at stops
            if (intTestFreqHz < 172) {intTestFreqHz = 172;}
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
            if (intBusyGain > intMaxGain) {intBusyGain = 0;} //Circular wrap around
          }
        if (intBusyGain < 0) {intBusyGain = intMaxGain;}
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
 

  // Code to handle computation of p-p measurements from ppWhiteOut and ppLPOut
  ulngCurrentElapsedTimeUs = micros();
  if ((ulngCurrentElapsedTimeUs - ulngLastPPAvgTimeUs) > 100000)//now using p-p measurement every 100 ms  (was 1 ms)
  {
    if (ppWhiteOut.available() == true)//Update fltppWhiteOutMeasAvg  average every 20 ms if ppWhteOut available
    {
      if (fltppWhiteOutMeasAvg < .01) {
        fltppWhiteOutMeasAvg = 1000 * ppWhiteOut.readPeakToPeak();
      }
      else {
        fltppWhiteOutMeasAvg = (.99 * fltppWhiteOutMeasAvg) +  10* ppWhiteOut.readPeakToPeak(); //Long time constant for p-p White Noise average
      }
    }
    if (ppLPOut.available() == true)//Update fltppLPOutMeasAvg  average every 20 ms if ppLPOut available
      // This implements a fast attack slow release computation for fltppLPOutMeasAvg to follow signals with high crest factors
    {
      fltppLPOutMeasPk =  1000 * ppLPOut.readPeakToPeak();
      if (fltppLPOutMeasAvg < 10)
        {
          fltppLPOutMeasAvg =  fltppLPOutMeasPk;
        }
      else  if (fltppLPOutMeasPk > fltppLPOutMeasAvg)
        {
          fltppLPOutMeasAvg = (.5 * fltppLPOutMeasAvg) + (.5 * fltppLPOutMeasPk); // Fast attack: Modified 0.9.17A (was .2, .8)
        }
      else if ((fltppi2s0InAvg > 200) || (fltppi2s1InAvg > 200))// If there is input to the Simulator
        {
          fltppLPOutMeasAvg = (.995 * fltppLPOutMeasAvg) + (.005 * fltppLPOutMeasPk);//slow release while input present
        }
      else // No input to the simulator so implement different time constant.
        {
          fltppLPOutMeasAvg = (.9 * fltppLPOutMeasAvg) + (.1 * fltppLPOutMeasPk);// faster release if no input
        }
     }

    
    if (rmsLPOut.available() == true)//Update fltrmsLPOutMeasAvg  average every 20 ms if ppLPOut available
      {
        fltrmsLPOutMeas =  1000 * rmsLPOut.read();
        if (fltrmsLPOutMeasAvg < 10) {fltrmsLPOutMeasAvg =  fltrmsLPOutMeas;}
        else {fltrmsLPOutMeasAvg = (.95 * fltrmsLPOutMeasAvg) + (.05 * fltrmsLPOutMeas);}
        if ((intMode <5)&&(!(blnPlotSpectrum || blnPlotIQPlane))) 
          {
            tft.setCursor(0,140); tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);  tft.println(strLastLevel);
            tft.setCursor(0,180); tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);  tft.println(strLastCF);
            str1 = "   Lvl=";  str2 =  String(int(fltppLPOutMeasAvg));str3 = " mvp-p"; strLastLevel = str1 + str2 + str3;
            if ((299<fltppLPOutMeasAvg)&&(fltppLPOutMeasAvg < 1701))//"Green" range 300-1700 mv p-p
              {tft.setCursor(0,140); tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(3);  tft.println(strLastLevel);}
            else{tft.setCursor(0,140); tft.setTextColor(ILI9341_RED);  tft.setTextSize(3);  tft.println(strLastLevel);}
            if (intMode == 0)
              {
                str1 = "   CF= ";  str2 =  String(.01 * int(50 * fltppLPOutMeasAvg/fltrmsLPOutMeasAvg));
                tft.setCursor(0,180); tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(3);  tft.println(str1 + str2); strLastCF = str1 + str2;
              }
          }
       }  
    if ((blnTestMode3K || blnTestMode6K) && (!blnPlotSpectrum))
    {
      if (ppi2s0In.available() == true)//Update ppi2s0In average if available
      {
        if (fltppi2s0InAvg < .01) {
          fltppi2s0InAvg = 1000 * ppi2s0In.readPeakToPeak();
        }
        else {
          fltppi2s0InAvg = (.9 * fltppi2s0InAvg) +  100 * ppi2s0In.readPeakToPeak(); //moderate time constant for ppi2s1In  average
        }
      }
      if (ppi2s1In.available() == true)//Update In average if available
      {
        if (fltppi2s1InAvg < .01) {
          fltppi2s1InAvg = 1000 * ppi2s1In.readPeakToPeak();
        }
        else {
          fltppi2s1InAvg = (.9 * fltppi2s1InAvg) +  100 * ppi2s1In.readPeakToPeak(); //moderate time constant for ppi2s1In  average
        }
      }
      if (ppAmpRightOut.available() == true)//Update ppAmpRightOut average if available
      {
        if (fltppAmpRightOutAvg < .01) {
          fltppAmpRightOutAvg = 1000 * ppAmpRightOut.readPeakToPeak();
        }
        else {
          fltppAmpRightOutAvg = (.9 * fltppAmpRightOutAvg) +  100 * ppAmpRightOut.readPeakToPeak(); //moderate time constant for ppAmpRight  average
        }
      }
      if (ppAmpLeftOut.available() == true)//Update ppAmpLeftOut average if available
      {
        if (fltppAmpLeftOutAvg < .01) {
          fltppAmpLeftOutAvg = 1000 * ppAmpLeftOut.readPeakToPeak();
        }
        else {
          fltppAmpLeftOutAvg = (.9 * fltppAmpLeftOutAvg) +  100 * ppAmpLeftOut.readPeakToPeak(); //moderate time constant for ppAmpLeft  average
        }
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
        if ((fltppAmpLeftOutAvg > (1500 * .944)) && (fltppAmpLeftOutAvg < (1500 * 1.06))) //Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
        }
        if ((fltppi2s0InAvg > (1500 * fltNomInOutRatio * .944)) && (fltppi2s0InAvg < (1500 * fltNomInOutRatio * 1.06))) //Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s0InAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s0InAvg)));
        }
        str2 =  " Ch2 Out = "; str3 = " Ch2 In  = ";

        if ((fltppAmpRightOutAvg > (1500 * .5 * .944)) && (fltppAmpRightOutAvg < (1500 * .5 * 1.06)))// //Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
        }
        if ((fltppi2s1InAvg > (1500 * .5 * .944 * fltNomInOutRatio )) && (fltppi2s1InAvg < 1500 * .5 * 1.06 * fltNomInOutRatio ))// Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s1InAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s1InAvg)));
        }
      }
      if (blnTestMode6K)
      {
        if ((fltppAmpLeftOutAvg > (1500 * .5 * .944 )) && (fltppAmpLeftOutAvg < (1500 * .5 * 1.06  )))//Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpLeftOutAvg)));
        }
        if ((fltppi2s0InAvg > (1500 * .5 * .944 * fltNomInOutRatio )) && (fltppi2s0InAvg < 1500 * .5 * 1.06 * fltNomInOutRatio ))//Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s0InAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s0InAvg)));
        }

        str2 =  " Ch2 Out = "; str3 = " Ch2 In  = ";
        if ((fltppAmpRightOutAvg > (1500 * .944)) && (fltppAmpRightOutAvg < (1500 * 1.06))) //Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str2 +  String(int(fltppAmpRightOutAvg)));
        }
        if ((fltppi2s1InAvg > (1500 * fltNomInOutRatio * .944)) && (fltppi2s1InAvg < (1500 * fltNomInOutRatio * 1.06))) //Nominal +/- .5 dB
        {
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s1InAvg)));
        }
        else {
          tft.setTextColor(ILI9341_RED);
          tft.setTextSize(3);
          tft.println(str3 +  String(int(fltppi2s1InAvg)));
        }
      }
      delay(50);
    }
    if (!(blnTestMode3K || blnTestMode6K))//Test code to measure input levels (not during TEST3K or TEST6K)
    {
      if (ppi2s0In.available() == true)//Update ppi2s0In average if available
      {
        if (fltppi2s0InAvg < 10) {
          fltppi2s0InAvg = 1000 * ppi2s0In.readPeakToPeak();
        }
        else {
          fltppi2s0InAvg = (.5 * fltppi2s0InAvg) +  500 * ppi2s0In.readPeakToPeak(); //Fast time constant for ppi2s1In  average
        }
      }
      if (ppi2s1In.available() == true)//Update In average if available
      {
        if (fltppi2s1InAvg < 10) {
          fltppi2s1InAvg = 1000 * ppi2s1In.readPeakToPeak();
        }
        else {
          fltppi2s1InAvg = (.5 * fltppi2s1InAvg) +  500 * ppi2s1In.readPeakToPeak(); //Fast time constant for ppi2s1In  average
        }
      }
    }
    ulngLastPPAvgTimeUs  = ulngCurrentElapsedTimeUs;
  } // End of Code to handle computation of p-p measurements from ppWhiteOut and ppLPOut
    //Code to handle Channel Types: MPG, MPM, MPP, MPD
  //Determine if time to update delays (as a function of Ch Type:  (based on update rate of 32 times the spread in Hz. (CCIR Guideline is >= 32 x)
  if ((intMode == 4) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (12500)) //Every 12.5 ms/# of paths for MPD  
      || ((intMode == 3) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (31250))) //Every 31.25ms/# of paths for MPP
      || ((intMode == 2) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (62500))) //Every 62.5 ms/#of paths for MPM
      || ((intMode == 1) && ((ulngCurrentElapsedTimeUs - ulngLastDelayUpdateUs) >= (312500)))) //Every 312.5 ms/#of paths for MPG
    {
      float fltGainAdj = 1.00/intMultipaths;//Normalize gains based on number of paths: 1, .5, .333, .25
      //noInterrupts(); // No interrupts while changes being made (Only 1 path delay value is changed for each count of intDelaytCount). 
      if (intDelayCount==0)//Update I1Path
        {
          fltCurrentDelayI1Ms = Update_Delay(intMode, fltMinimumIDelayMs); delayI.delay(1, fltCurrentDelayI1Ms);//I1 path, used for intMultipaths,2,3,4
          fltMixI1Gain= fltGainAdj * IDelayToIGain(fltCurrentDelayI1Ms); mixIQ.gain(1,fltMixI1Gain); //Serial.println("I1Path");
        }
      if (intDelayCount==1)//Update Q1Path
        {
          fltCurrentDelayQ1Ms = Update_Delay(intMode, fltMinimumQDelayMs); delayQ.delay(1, fltCurrentDelayQ1Ms);//Q1 path, used for intMultipaths,2,3,4
          fltMixQ1Gain= fltGainAdj * QDelayToQGain(fltCurrentDelayQ1Ms); mixIQ.gain(0,fltMixQ1Gain); //Serial.println("Q1Path");
        } 
      if (intDelayCount==2)//Update I2Path
        {
          fltCurrentDelayI2Ms = Update_Delay(intMode, fltMinimumIDelayMs); delayI.delay(2, fltCurrentDelayI2Ms);//I2 path, used for intMultipaths,3,4
          fltMixI2Gain= fltGainAdj * IDelayToIGain(fltCurrentDelayI2Ms); mixIQ.gain(3,fltMixI2Gain); //Serial.println("I2Path");
        }
      if (intDelayCount >=3)//Update Q2Path 
        {
          fltCurrentDelayQ2Ms = Update_Delay(intMode, fltMinimumQDelayMs); delayQ.delay(2, fltCurrentDelayQ2Ms);//Q2 path, used for intMultipaths,4
          fltMixQ2Gain= fltGainAdj * QDelayToQGain(fltCurrentDelayQ2Ms); mixIQ.gain(2,fltMixQ2Gain); //Serial.println("Q2Path");
        } 
      intDelayCount +=1;
      ulngCurrentElapsedTimeUs = micros();ulngLastDelayUpdateUs = ulngCurrentElapsedTimeUs;
      if (intDelayCount >= intMultipaths)
        {
          intDelayCount = 0;
         //Gain adjust (These emperical adjustments meant to balance out the gain to keep signal levels in the nominal range)
          if (intMultipaths == 2){mixIQ.gain(1, fltMixI1Gain * 2.0); mixIQ.gain(0, fltMixQ1Gain *2.0);}
          else if (intMultipaths == 3){mixIQ.gain(1, fltMixI1Gain * 2.5); mixIQ.gain(0, fltMixQ1Gain *2.5);mixIQ.gain(3,fltMixI2Gain*2.5);}
          else if (intMultipaths == 4){mixIQ.gain(1, fltMixI1Gain * 3.0); mixIQ.gain(0, fltMixQ1Gain *3.0);
            mixIQ.gain(3,fltMixI2Gain*3.0); mixIQ.gain(2,fltMixQ2Gain * 3.0);}
        } 
      //interrupts(); 
      //Only plot IQPlane when enabled and all delays have been updated
      if (blnPlotIQPlane && (intDelayCount == 0)){PlotIQPlane(fltCurrentDelayI1Ms, fltCurrentDelayQ1Ms, fltCurrentDelayI2Ms, fltCurrentDelayQ2Ms, intMode); }
   }
  // Adjust  S:N only every 2-5 ms 
  if (int(ulngCurrentElapsedTimeUs - ulngLastSNUpdateUs) > random(2000, 5000)); // Only adjust S:N every 2-5ms
    {
      if (blnSim) { AdjustS_N ( intTargetSN, fltppLPOutMeasAvg, fltppWhiteOutMeasAvg);}
      ulngLastSNUpdateUs = ulngCurrentElapsedTimeUs;
      // Code for Fading a Channel (modes 0-4) WGN,MPG,MPM,MPP,MPD
      if ((intFadeDepth_dB > 0) && (fltFadeRate > .01) && blnSim)
        {
          Fade (intTargetSN, intFadeDepth_dB, fltFadeRate);//Fade also adjust S:N on peak value averages
        }
      else if (blnSim) 
        {
          AdjustS_N ( intTargetSN, fltppLPOutMeasAvg, fltppWhiteOutMeasAvg);
        }
    }// End of Code to handle S:N adjustment

  // Plot Spectrum for Sim mode
  if ((blnSim) && ( (millis() - ulngLastSpectrumUpdate) > 100) && blnPlotSpectrum) //Plot only every 100 ms in Sim mode (no averaging)
    {
      if (fft1024.available())
        {
          if (intBandwidth == 6000) {intNumofBinsToPlot = 150;} //6503 Hz
          if (intBandwidth == 3000) {intNumofBinsToPlot = 82;} //3531 Hz
          for (int i = 0; i <= intNumofBinsToPlot; i++) { fltFFT[i] = 4000.0 * fft1024.read(i);}
          PlotSpectrum(fltFFT, intSpecLow, intSpecHigh, blnInitSpectrum, intBandwidth);
          blnInitSpectrum = false;// only init on first call to print headers.
          ulngLastSpectrumUpdate = millis();
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
          ulngLastSpectrumUpdate = millis();
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
  //int btn1 = debouncer1.read();
  //int btn2 = debouncer2.read();
  //static boolean blnTone1500 = true;
  if (debouncer1.fell())//Right Encoder "push"
    {
      blnEnableTestTone = !blnEnableTestTone;//Toggle Test tone
      if (intMode < 5)
        {
          if (blnEnableTestTone)
            { //Set mixInpSel mux gains and enable InputTestWaveform @ 1500 Hz
              mixInpSel.gain(0, 2.0); mixInpSel.gain(1, 0.0); mixInpSel.gain(2, 0.0); mixInpSel.gain(3, 0.0);
              InputTestWaveform.amplitude(fltCalTestLevel);
              intTestFreqHz = 1548; sine_VLF_Dnmix_Mod.amplitude(0);
              InputTestWaveform.frequency (intTestFreqHz); InputTestWaveform.amplitude(fltCalTestLevel);
              if (((intFMDevPtr == 0 ) || (intFMRatePtr == 0)) && (intTuneOffset == 0))
                {
                  mixAudioSelect.gain(0, 1); mixAudioSelect.gain(1, 0); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 2141 Tone on no FM");
                }
              else
                {
                  filHP7500FIR.end();
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  sine_VLF_Dnmix_Mod.amplitude(fltLogs[intFMDevPtr] * .00013333333333 ); //sets max deviation in Hz e.g. .000133333333 * 7500  yields +/- 1 Hz peak deviation
                  filHP7500FIR.begin(sht7500HzHPCoef, 120); // LP Downmix already set
                  sine_VLF_Dnmix_Mod.frequency(fltLogs[intFMRatePtr]);
                  mixAudioSelect.gain(0, 0); mixAudioSelect.gain(1, 4); mixAudioSelect.gain(2, 0); mixAudioSelect.gain(3, 0);//Serial.println("Line 2150 FM Rate, Deviation Set");
                }
             }
          else
            { //Restore mixInputSel gains and turn off InputTestWaveform
              mixInpSel.gain(0, 0); mixInpSel.gain(1, 0); mixInpSel.gain(2, fltLogs[intGainLevel[0]]); mixInpSel.gain(3, fltLogs[intGainLevel[1]]);
              InputTestWaveform.amplitude(0);
            }
        }
      if ((intMode >= 5) && (intMode <= 15)) //Gain and output level settings)setting to  EEPROM
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
          if ((intMode >= 1) && (intMode <= 4) )
            {
              blnPlotIQPlane = true; intLastIQPlaneMode = -1;
            } // Disable blnPlotIQPlane if on mode =0
          else
            {
              blnPlotIQPlane = false;  blnInitModes = true; blnModes = true;
            }
        }
      else if (blnPlotIQPlane)
        {
          blnPlotIQPlane = false; blnInitModes = true; blnModes = true;
        }
      delay(100);
    }
  
  // This code to receive serial commands via USB and return status via USB.
  while  (Serial.available() > 0)//While there are any unprocessed characters received via the serial port
    {
      char inChar = Serial.read();//Read one char
      if (inChar == '\n')//if it is a New Line (Cr)
        {
          Serial.print("Mode=");//Debugging code ...remove
          Serial.println(strMode.toUpperCase());//Shift to upper makes all case insensitive
          Serial.print("Parameter=");
          Serial.println(strParameter.toUpperCase());//Shift to upper makes all case insensitive
          intSerialCmdMode = -1; intSerialCmdParam = -1;
          if (blnSim)
            {
              intSerialCmdMode = ParseSimMode(strMode);// Serial.print("Line 2263 ParseSimMode, intSerialCmdMode = "); Serial.println(intSerialCmdMode);
              if (intSerialCmdMode == 18)//  Moved to 18 to accomodate TEST3K, TEST6K
                {
                  Serial.println("OK");//Serial Command is OK
                  blnSim = false;  blnInitialized = false;  blnEnableTestTone = false;
                  InitializeBusy();
                }
              else if (intSerialCmdMode > -1)
                {
                  if (ParseSetSimParameter(strParameter, intSerialCmdMode)){Serial.println("OK");} //Serial Command is OK
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
