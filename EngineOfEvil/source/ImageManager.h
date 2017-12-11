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
#ifndef EVIL_IMAGE_MANAGER_H
#define EVIL_IMAGE_MANAGER_H

#include "Renderer.h"
#include "ResourceManager.h"

//******************************************
//			eImageManager
// Handles all texture allocation and freeing
// see also: eResourceManager template
//******************************************
class eImageManager : public eResourceManager<eImage> {
public:

	virtual bool							Init() override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eImage> & result) override;

	virtual int								GetClassType() const override				{ return CLASS_IMAGE_MANAGER; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_IMAGE_MANAGER) 
													return true; 
												return eResourceManager<eImage>::IsClassType(classType); 
											}

	// subclass extension, does not obscure any base function
	bool									LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result);

private:

	bool									LoadSubframes(std::ifstream & read, std::shared_ptr<eImage> & result);
};

#endif /* EVIL_IMAGE_MANAGER_H */
