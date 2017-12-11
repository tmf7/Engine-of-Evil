/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
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
