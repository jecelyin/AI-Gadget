#pragma once
#include <stdint.h>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    
    bool begin();
    void end();
    void loop();
    bool isInitialized() const;
    bool isPlaying();
    uint16_t getDuration();
    uint16_t getPosition();
    void playFile(const char* filePath, uint16_t position = 0);
    void stop();
    void pause();
    void resume();
    void seek(uint16_t position);
    void setVolume(uint8_t volume_percent);
    uint8_t getVolume() const;

private:
    bool m_initialized = false;
};