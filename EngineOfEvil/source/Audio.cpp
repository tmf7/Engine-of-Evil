#include "Audio.h"


//*********************
// eAudio::Init
// TODO: allow user to configure the number of audio channels
//*********************
bool eAudio::Init() {
    return (Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 2048 ) != -1 );
}

//*********************
// eAudio::Shutdown
//*********************
void eAudio::Shutdown() {
    Mix_CloseAudio();
}

//*********************
// eAudio::IsMusicPlaying
//*********************
bool eAudio::IsMusicPlaying() {
    return Mix_PlayingMusic();
}

//*********************
// eAudio::IsMusicPaused
//*********************
bool eAudio::IsMusicPaused() {
    return Mix_PausedMusic();
}

//*********************
// eAudio::PauseMusic
//*********************
void eAudio::PauseMusic() {
    Mix_PauseMusic();
}

//*********************
// eAudio::UnpauseMusic
//*********************
void eAudio::UnpauseMusic() {
    Mix_ResumeMusic();
}

//*********************
// eAudio::StopMusic
//*********************
void eAudio::StopMusic() {
    Mix_HaltMusic();
}

//*********************
// eAudio::StopChannel
//*********************
void eAudio::StopChannel(int channel) {
    Mix_HaltChannel(channel);
}
