#include "Music.h"

//*******************
// eMusic::Load
//*******************
bool eMusic::Load(const char * sourceFilename) {
    music = Mix_LoadMUS(sourceFilename);
    if (music == nullptr) {
		std::string message = "Unable to load music file: ";
		message += sourceFilename;
		EVIL_ERROR_LOG.LogError(message.c_str(), __FILE__, __LINE__);
        return false;
    }

    return true;
}

//*******************
// eMusic::Free
// releases the currently handled music file
//*******************
void eMusic::Free() {
    if (music != nullptr) {
        Mix_FreeMusic(music);
        music = nullptr;
    }
}

//*******************
// eMusic::Play
// outputs audio of the assigned music file
// param numLoops replays the audio file that number of times
// DEBUG: -1 loops the music near-infinitely
// and 0 never plays the music, default is -1
//*******************
void eMusic::Play(int numLoops) {
    if (music != nullptr)
        Mix_PlayMusic(music, numLoops);
}

//*******************
// eMusic::IsLoaded
//*******************
bool eMusic::IsLoaded() {
    return (music != nullptr);
}
