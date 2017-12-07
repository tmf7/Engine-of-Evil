#ifndef EVIL_MUSIC_H
#define EVIL_MUSIC_H

#include "Audio.h"

//*********************************
//			eMusic
// handles a single audio music file for output
// TODO: add more mixer wrapper functions to this class
//*********************************
class eMusic {
public:

    bool			Load(const char * sourceFilename);
    void			Free();
    void			Play(int numLoops = -1);
    bool			IsLoaded();

private:

    Mix_Music *		music = nullptr;
};

#endif /* EVIL_MUSIC_H */
