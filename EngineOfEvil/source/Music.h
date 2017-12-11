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
