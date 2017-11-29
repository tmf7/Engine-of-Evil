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

	virtual int								GetClassType() const override { return CLASS_IMAGE_MANAGER; }

	// subclass extension, does not obscure any base function
	bool									LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result);

private:

	bool									LoadSubframes(std::ifstream & read, std::shared_ptr<eImage> & result);
};

#endif /* EVIL_IMAGE_MANAGER_H */
