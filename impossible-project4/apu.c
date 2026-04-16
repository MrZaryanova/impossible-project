//
//  apu.c
//  impossible-project4
//
//  Created by luis on 4/1/26.
//

#include "mathseries.h"
#include <SDL3/SDL.h>
/*
 1.implement 0x4000-0x4017 in storeb and 0x4015 in func_operand
 2. Implement pulses and mixer
 3. Implement triangle and noise
 4. Implement frame sequencer
 5. Implement dmc and dmc irq
 6. Implement resampling
 */

static SDL_AudioStream *stream = NULL;
static int current_sine_sample = 0;

SDL_AppResult create_audio(void) {
    SDL_AudioSpec spec;
    
    spec.channels = 1;
        spec.format = SDL_AUDIO_F32;
        spec.freq = 8000;
        stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (!stream) {
            SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        /* SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start! */
        SDL_ResumeAudioStreamDevice(stream);
    return SDL_APP_CONTINUE;
}

/*
 SDL_AppResult play_audio(double freq) {
 
 // freq is in hertz
 const int minimum_audio = (8000 * sizeof (float)) / 2;  // 8000 float samples per second. Half of that.
 if (SDL_GetAudioStreamQueued(stream) < minimum_audio) {
 static float samples[512];  // this will feed 512 samples each frame until we get to our maximum.
 int i;
 
 for (i = 0; i < SDL_arraysize(samples); i++) {
 const float phase = current_sine_sample * freq / 8000.0f;
 samples[i] = sin(phase * 2.0 * PI);
 // samples[i] = phase * 2.0 * PI;
 //  samples[i] = arcsin(sin( * phase * 2.0 * PI));
 current_sine_sample++;
 }
 
 // wrapping around to avoid floating-point errors
 current_sine_sample %= 8000;
 
 // feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data.
 SDL_PutAudioStreamData(stream, samples, sizeof (samples));
 }
 
 return SDL_APP_CONTINUE;
 
 }
 */

SDL_AppResult play_audio(void) { // will problably not take void parameters later hahaha
    // this could work as a apu_loop() function
    return SDL_APP_CONTINUE;
}
