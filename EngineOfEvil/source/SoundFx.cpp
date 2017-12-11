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
#include "SoundFx.h"

//*******************
// eSoundFx::Load
//*******************
bool eSoundFx::Load(const char * sourceFilename) {
    soundFx = Mix_LoadWAV(sourceFilename);

    if (soundFx == nullptr) {
		std::string message = "Unable to load sound effect file: ";
		message += sourceFilename;
		EVIL_ERROR_LOG.LogError(message.c_str(), __FILE__, __LINE__);
        return false;
    }

    return true;
}

//*******************
// eSoundFx::Free
// releases the currently handled soundFx file
//*******************
void eSoundFx::Free() {
    if (soundFx != nullptr) {
        Mix_FreeChunk(soundFx);
        soundFx = nullptr;
    }
}

//*******************
// eSoundFx::Play
// outputs audio of the assigned soundfx file
// param numLoops replays the audio file that number of times
// DEBUG: -1 loops the sound near-infinitely
// and 0 never plays the sound, default is 1
//*******************
int eSoundFx::Play(int loops) {
    if (soundFx != nullptr)
        return Mix_PlayChannel(-1, soundFx, loops);
    else
        return -1;
}

//*******************
// eSoundFx::Load
//*******************
bool eSoundFx::IsLoaded() {
    return (soundFx != nullptr);
}
