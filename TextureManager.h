#ifndef EVIL_TEXTURE_MANAGER_H
#define EVIL_TEXTURE_MANAGER_H

#include "Definitions.h"
#include "Image.h"
#include "HashIndex.h"
#include "Renderer.h"

//**********************************
//			eTextureManager
// Handles all texture allocation and feeing
// DEBUG: --no other object/system should allocate/free textures--
//**********************************
class eTextureManager {
public:

					eTextureManager();

	SDL_Texture *	GetTexture(const char * name);
	void			Free();

private:

	typedef struct eTexture_s {
		SDL_Texture * texture;
		std::string name;
	} eTexture_t;

	eTexture_t		textures[MAX_TEXTURES];
	eHashIndex		textureHash;
	int				numTextures;
};

//***************
// eTextureManager::eTextureManager
//***************
inline eTextureManager::eTextureManager()
	: numTextures(0) {
}

#endif /* EVIL_TEXTURE_MANAGER_H */

