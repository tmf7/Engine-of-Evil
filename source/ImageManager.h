#ifndef EVIL_TEXTURE_MANAGER_H
#define EVIL_TEXTURE_MANAGER_H

#include "Definitions.h"
#include "Image.h"
#include "HashIndex.h"
#include "Renderer.h"

//**********************************
//			eImageManager
// Handles all texture allocation and freeing
// DEBUG: --no other object/system should allocate/free textures--
//**********************************
class eImageManager {
public:

	bool			Init();
	bool			BatchLoadImages(const char * imageBatchFile);
	bool			GetImage(const char * filename, std::shared_ptr<eImage> & result);
	bool			GetImage(int imageID, std::shared_ptr<eImage> & result);
	bool			LoadImage(const char * filename, SDL_TextureAccess accessType, std::shared_ptr<eImage> & result);
	bool			LoadConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result);
	bool			LoadImageSubframes(const char * filename);
	bool			BatchLoadSubframes(const char * subframeBatchFile);
	int				GetNumImages() const;
	void			Clear();

private:

	std::vector<std::shared_ptr<eImage>>		imageList;			// dynamically allocated image resources
	eHashIndex									imageFilenameHash;	// quick access to imageList
};

//***************
// eImageManager::GetNumImages
//***************
inline int eImageManager::GetNumImages() const {
	return imageList.size();
}

#endif /* EVIL_TEXTURE_MANAGER_H */

