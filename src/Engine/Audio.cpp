#include "Engine/Audio.hpp"
#include "Engine/miniaudio.h"
#include <cmath>
#include <cstdlib>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    (void)pInput;
    Audio* audio = (Audio*)pDevice->pUserData;
    float* fOutput = (float*)pOutput;
    
    SfxType currentSfx = audio->activeSfx.load();
    int frames = audio->framesRemaining.load();
    float currentPhase = audio->phase.load();
    int sampleRate = pDevice->sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        if (frames <= 0) {
            fOutput[i * 2] = 0.0f;
            fOutput[i * 2 + 1] = 0.0f;
            continue;
        }

        float sample = 0.0f;
        float freq = 0.0f;

        if (currentSfx == SfxType::STEP) {
            freq = 150.0f; 
            sample = (sin(currentPhase) > 0.0f) ? 0.2f : -0.2f; 
        } 
        else if (currentSfx == SfxType::TURN) {
            freq = 400.0f;
            sample = (sin(currentPhase) > 0.0f) ? 0.15f : -0.15f; 
        } 
        else if (currentSfx == SfxType::BUMP) {
            freq = 60.0f;
            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            float saw = fmod(currentPhase, 2.0f * M_PI) / (2.0f * M_PI) * 2.0f - 1.0f;
            sample = saw * 0.3f + noise * 0.2f;
        } 
        else if (currentSfx == SfxType::NECK) {
            float progress = 1.0f - ((float)frames / (sampleRate * 0.3f));
            freq = 200.0f + progress * 400.0f;
            sample = sin(currentPhase) * 0.2f; 
        }

        currentPhase += 2.0f * M_PI * freq / sampleRate;
        if (currentPhase >= 2.0f * M_PI) currentPhase -= 2.0f * M_PI;
        
        fOutput[i * 2] = sample;
        fOutput[i * 2 + 1] = sample;
        frames--;
    }

    audio->phase.store(currentPhase);
    audio->framesRemaining.store(frames);
    if (frames <= 0) audio->activeSfx.store(SfxType::NONE);
}

// Запускаем либо синтезатор (0), либо загрузчик файлов (1), но не оба сразу!
Audio::Audio(int mode) : currentMode(mode), device(nullptr), isMusicLoaded(false) {
    if (currentMode == 0) {
        device = new ma_device;
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format   = ma_format_f32;
        config.playback.channels = 2;
        config.sampleRate        = 44100;
        config.dataCallback      = data_callback;
        config.pUserData         = this;

        ma_device_init(NULL, &config, device);
        ma_device_start(device);
    } else {
        ma_engine_init(NULL, &engine);
    }
}

Audio::~Audio() {
    if (currentMode == 1) {
        if (isMusicLoaded) ma_sound_uninit(&bgm);
        ma_engine_uninit(&engine);
    } else if (currentMode == 0) {
        if (device) {
            ma_device_stop(device);
            ma_device_uninit(device);
            delete device;
        }
    }
}

void Audio::play(SfxType type) {
    if (currentMode != 0) return;
    activeSfx.store(type);
    phase.store(0.0f);
    
    if (type == SfxType::STEP) framesRemaining.store(44100 * 0.08f);
    else if (type == SfxType::TURN) framesRemaining.store(44100 * 0.05f);
    else if (type == SfxType::BUMP) framesRemaining.store(44100 * 0.15f);
    else if (type == SfxType::NECK) framesRemaining.store(44100 * 0.3f);
}

void Audio::playSound(const char* filepath) {
    if (currentMode == 1) ma_engine_play_sound(&engine, filepath, NULL);
}

void Audio::playMusic(const char* filepath) {
    if (currentMode != 1) return;
    if (isMusicLoaded) ma_sound_uninit(&bgm);
    
    if (ma_sound_init_from_file(&engine, filepath, MA_SOUND_FLAG_STREAM, NULL, NULL, &bgm) == MA_SUCCESS) {
        ma_sound_set_looping(&bgm, MA_TRUE);
        ma_sound_start(&bgm);
        isMusicLoaded = true;
    }
}

void Audio::stopMusic() {
    if (currentMode == 1 && isMusicLoaded) {
        ma_sound_stop(&bgm);
    }
}