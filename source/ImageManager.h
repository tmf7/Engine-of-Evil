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

	virtual bool							Load(const char * resourceFilename) override;
	virtual bool							Init() override;
	virtual bool							BatchLoad(const char * resourceBatchFilename) override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eImage> & result) override;

	virtual int								GetClassType() const override { return CLASS_IMAGE_MANAGER; }

	// subclass extensions, does not obscure base functions
	bool									Load(const char * resourceFilename, SDL_TextureAccess accessType);
	bool									LoadAndGet(const char * resourceFilename, SDL_TextureAccess accessType, std::shared_ptr<eImage> & result);
	bool									LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result);

private:

	bool									LoadSubframes(std::ifstream & read, std::shared_ptr<eImage> & result);
};

#endif /* EVIL_IMAGE_MANAGER_H */
