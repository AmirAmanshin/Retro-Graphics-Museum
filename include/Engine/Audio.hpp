#pragma once
#include <atomic>
#include "miniaudio.h"

enum class SfxType { NONE, STEP, TURN, BUMP, NECK };

class Audio {
public:
    Audio(int mode = 0);
    ~Audio();
    
    void play(SfxType type);

    void playSound(const char* filepath);
    void playMusic(const char* filepath);
    void stopMusic();

    // Атомарные переменные для потокобезопасного доступа из аудио-коллбека
    std::atomic<SfxType> activeSfx{SfxType::NONE};
    std::atomic<int> framesRemaining{0};
    std::atomic<float> phase{0.0f};

private:
    int currentMode;
    ma_device* device;     // Для синтезатора (Уровень 1)
    ma_engine engine;      // Для файлов (Уровень 2)
    ma_sound bgm;          // Объект фоновой музыки для управления (пауза/луп)
    bool isMusicLoaded;
};