#ifndef EVIL_SOUNDFX_H
#define EVIL_SOUNDFX_H

#include "Audio.h"

//*********************************
//			eSoundFx
// handles a single audio sound effect file for output
// TODO: add more mixer wrapper functions to this class
//*********************************
class eSoundFx {
public:

    bool				Load(const char * sourceFilename);
    void				Free();
    int					Play(int numLoops = 1);
    bool				IsLoaded();

private:

    Mix_Chunk *			soundFx;
};

#endif /* EVIL_SOUNDFX_H */
