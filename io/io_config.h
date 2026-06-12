
#ifndef USER__CONFIG_IO_H
#define USER__CONFIG_IO_H

#define WIDTH   160 
#define HEIGHT  80

/*
To confirm sampling frequency,
measure clock on DAC's BCK (13) pin
 16  1.024 
 32  2.048 
 44  2.8224
 48  3.072 
 96  6.144 
192  12.288
384  24.576
*/

// select one
//#define SAMPLING_FREQ 384
//#define SAMPLING_FREQ 192
//#define SAMPLING_FREQ 96
#ifndef SAMPLING_FREQ
#define SAMPLING_FREQ 48
#endif

#if SAMPLING_FREQ == 48
#define SAMPLE_RATE 48000.0f
#elif SAMPLING_FREQ == 96
#define SAMPLE_RATE 96000.0f
#elif SAMPLING_FREQ == 192
#define SAMPLE_RATE 192000.0f
#elif SAMPLING_FREQ == 384
#define SAMPLE_RATE 384000.0f
#endif

#define BUFFER_DURATION       50 //ms
#define AUDIO_CHANNELS        2  // 1 or 2
#define SAMPLES_PER_CHANNEL   (SAMPLING_FREQ * BUFFER_DURATION)
#define SAMPLES               (SAMPLES_PER_CHANNEL * AUDIO_CHANNELS)

#endif

