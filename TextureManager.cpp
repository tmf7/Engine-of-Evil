#include <functional>		// std::hash
#include "TextureManager.h"
#include "Game.h"

//***************
// eTextureManager::GetTexture
// returns a pointer to an SDL_Texture if it exists
// otherwise attempts to initialize an SDL_Texture from the name (filename)
// DEBUG: assert ( name != NULL )
//***************
SDL_Texture * eTextureManager::GetTexture(const char * name) {
	// search for pre-existing texture
	auto hasher = std::hash<const char *>{};
	int hashkey = hasher(name);
	for (int i = textureHash.First(hashkey); i != -1; i = textureHash.Next(i)) {
		if (textures[i].name == name) {
			return textures[i].texture;
		}
	}

	// no mathes found, initilize the image
	SDL_Texture * source = IMG_LoadTexture(game.GetRenderer().GetSDLRenderer(), name);

	// unable to initialize
	if (source == NULL)
		return nullptr;

	// register the requested texture
	textureHash.Add(hashkey, numTextures);
	textures[numTextures] = eTexture_t{ source, name };
	return textures[numTextures++].texture;
}

//***************
// eTextureManager::Free
// frees all resource image surfaces
//***************
void eTextureManager::Free() {
	for (int i = 0; i < numTextures; i++) {
		if (textures[i].texture)
			SDL_DestroyTexture(textures[i].texture);
	}
	numTextures = 0;
}