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
