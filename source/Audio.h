#ifndef EVIL_AUDIO_H
#define EVIL_AUDIO_H

#include "Definitions.h"

//*********************************
//			eAudio
// handles music and sound fx output channels
//*********************************
class eAudio {
public:

    bool			Init();
    void			Shutdown();
    bool			IsMusicPlaying();
    bool			IsMusicPaused();
    void			PauseMusic();
    void			UnpauseMusic();
    void			StopMusic();
    void			StopChannel(int channel);
};

#endif /* EVIL_AUDIO_H */
