#include "Game.h"

//***************************
// eImageManager::Init
//***************************
bool eImageManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_IMAGES);

	// initialize error_image, a bright red texture the size of the current render target
	int width = game.GetRenderer().ViewArea().w;
	int height = game.GetRenderer().ViewArea().h;
	SDL_Texture * error_texture = SDL_CreateTexture(game.GetRenderer().GetSDLRenderer(),
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height);

	if (!error_texture)
		return false;

	SDL_PixelFormat * pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	Uint32 redPixel = SDL_MapRGBA(pixelFormat, 255, 0, 0, 255);
	SDL_FreeFormat(pixelFormat);

	// lock the pixels for writing
	void * pixels = nullptr;
	int pitch = 0;
	if (SDL_LockTexture(error_texture, NULL, &pixels, &pitch) == -1) {
		SDL_DestroyTexture(error_texture);
		return false;
	}

	// write the red pixels on the blank texture
	int lastPixel = pitch * height / SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888);
	for (int i = 0; i < lastPixel; i++)
		((Uint32*)pixels)[i] = redPixel;
	SDL_UnlockTexture(error_texture);

	// register the error_image as the first element of imageList
	int hashKey = resourceHash.GetHashKey(std::string("error_image"));
	resourceHash.Add(hashKey, resourceList.size());
	resourceList.emplace_back(std::make_shared<eImage>(error_texture, "invalid_file", resourceList.size()));	// error image
	return true;
}

//***************************
// eImageManager::BatchLoad
// loads a batch of image resources
// user can optionally call Clear() or Unload()
// prior to this to facilitate starting with a fresh set of images
// DEBUG (.bimg file format):
// imageFilename\n
// imageFilename\n
// (repeat)
//***************************
bool eImageManager::BatchLoad(const char * imageBatchFilename) {
	std::ifstream	read(imageBatchFilename);
	
	// unable to find/open file
	if(!read.good())
		return false;

	char resourceFilename[MAX_ESTRING_LENGTH];
	while (!read.eof()) {
		read >> resourceFilename;
		if (!VerifyRead(read))
			return false;

		if (!Load(resourceFilename, SDL_TEXTUREACCESS_STATIC))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//***************************
// eImageManager::LoadConstantText
// cache unchanging text images
// TODO: update to use a glyph atlas texture
// instead of loading a new texture each time
//***************************
bool eImageManager::LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result) {
	// check if the image already exists 
	// and set result to that if it does
	if ((result = Get(text)) != resourceList[0])
		return true;

	SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, color);
	if (surfaceText == NULL) {
		result = resourceList[0]; // error image
		return false;
	}

	SDL_Texture * renderedText = SDL_CreateTextureFromSurface(game.GetRenderer().GetSDLRenderer(), surfaceText);
	SDL_FreeSurface(surfaceText);
	if (renderedText == NULL) {
		result = resourceList[0]; // error image
		return false;
	}

	SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);

	// register the requested text image
	int hashKey = resourceHash.GetHashKey(std::string(text));
	resourceHash.Add(hashKey, resourceList.size());
	result = std::make_shared<eImage>(renderedText, text, resourceList.size());
	resourceList.emplace_back(result);
	return true;
}

//***************************
// eImageManager::LoadSubframes
// helper function for Loading .eimg files
//***************************
bool eImageManager::LoadSubframes(std::ifstream & read, std::shared_ptr<eImage> & result) {
	std::vector<SDL_Rect> frameList;

	// minimize dynamic allocations
	int numFrames = 0;
	read >> numFrames;
	if (!VerifyRead(read))
		return false;

	frameList.reserve(numFrames);

	// default subframe
	if (numFrames == 0) {
		read.close();
		frameList.emplace_back(SDL_Rect{ 0, 0, result->GetWidth(), result->GetHeight() });
		result->SetSubframes(std::move(frameList));
		return true;
	}

	while (!read.eof()) {
		// one subframe per line
		SDL_Rect frame;
		for (int targetData = 0; targetData < 4; targetData++) {
			switch (targetData) {
				case 0: read >> frame.x; break;
				case 1: read >> frame.y; break;
				case 2: read >> frame.w; break;
				case 3: read >> frame.h; break;
			}
		
			if (!VerifyRead(read))
				return false;
		}
		frameList.emplace_back(std::move(frame));
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	read.close();
	result->SetSubframes(std::move(frameList));

	// register the requested image
	const int hashKey = resourceHash.GetHashKey(std::string(result->GetSourceFilename()));
	resourceHash.Add(hashKey, resourceList.size());
	return true;
}

//***************************
// eImageManager::Load
// attempts to load the given .eimg file and
// returns true if the image was successfully loaded
// returns false otherwise
// DEBUG (.eimg file format):
// imageFilepath\n
// number of subframes to expect\n
// x y w h # frame number 0 comment for reference\n
// x y w h # this is ignored 1 this is ignored\n
// x y w h # ditto 2 ditto\n
// (repeat)
// DEBUG: if "number of subframes to expect" == 0, then the default subframe is the size of the image itself
// DEBUG: defaults the loaded SDL_Texture to SDL_TEXTUREACCESS_STATIC
//***************************
bool eImageManager::Load(const char * resourceFilename) {
	std::shared_ptr<eImage> result;

	// image already loaded
	if ((result = Get(resourceFilename)) != resourceList[0])
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) 
		return false;

	char imageFilepath[MAX_ESTRING_LENGTH];
	memset(imageFilepath, 0, sizeof(imageFilepath));
	read.getline(imageFilepath, sizeof(imageFilepath), '\n');
	if(!VerifyRead(read))
		return false;

	SDL_Texture * texture = NULL;
	texture = IMG_LoadTexture(game.GetRenderer().GetSDLRenderer(), imageFilepath);

	// unable to initialize texture
	if (texture == NULL)
		return false;

	result = std::make_shared<eImage>(texture, resourceFilename, resourceList.size());
	if (!LoadSubframes(read, result)) {
		result = nullptr;		// destroy recently allocated result
		return false;
	}

	resourceList.emplace_back(std::move(result));
	return true;
}

//***************************
// eImageManager::LoadAndGet
// attempts to load the given .eimg file and sets result to
// either the found image and returns true, 
// or default image and returns false
// param accessType can be:
// SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
// SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
// SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
// see also eImageManager::Load for file format
//***************************
bool eImageManager::LoadAndGet(const char * resourceFilename, SDL_TextureAccess accessType, std::shared_ptr<eImage> & result) {

	// image already loaded
	if ((result = Get(resourceFilename)) != resourceList[0])
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) 
		return false;

	char imageFilepath[MAX_ESTRING_LENGTH];
	memset(imageFilepath, 0, sizeof(imageFilepath));
	read.getline(imageFilepath, sizeof(imageFilepath), '\n');
	if(!VerifyRead(read))
		return false;

	if (accessType < SDL_TEXTUREACCESS_STATIC || accessType > SDL_TEXTUREACCESS_TARGET)
		accessType = SDL_TEXTUREACCESS_STATIC;
		
	SDL_Texture * texture = NULL;
	if (accessType != SDL_TEXTUREACCESS_STATIC) {
		SDL_Surface * source = IMG_Load(imageFilepath);

		// unable to load file
		if (source == NULL) {
			result = resourceList[0]; // error image
			return false;
		}

		texture = SDL_CreateTexture(game.GetRenderer().GetSDLRenderer(),
												  source->format->format,
												  accessType, 
												  source->w, 
												  source->h);
		// unable to initialize texture
		if (texture == NULL) {
			result = resourceList[0]; // error image
			SDL_FreeSurface(source);
			return false;
		}
	
		// attempt to copy data to the new texture
		if (SDL_UpdateTexture(texture, NULL, source->pixels, source->pitch)) {
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(source);
			result = resourceList[0]; // error image
			return false;
		}
		SDL_FreeSurface(source);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	} else {
		texture = IMG_LoadTexture(game.GetRenderer().GetSDLRenderer(), imageFilepath);

		// unable to initialize texture
		if (texture == NULL) {
			result = resourceList[0]; // error image
			return false;
		}
	}

	result = std::make_shared<eImage>(texture, resourceFilename, resourceList.size());
	if (!LoadSubframes(read, result)) {
		result = resourceList[0];	// error image, and destroy recently allocated result
		return false;
	}

	resourceList.emplace_back(result);
	return true;
}

//***************************
// eImageManager::Load
// convenience function, 
// same as LoadAndGet w/accessType except 
// it only checks load success/failure
//***************************
bool eImageManager::Load(const char * resourceFilename, SDL_TextureAccess accessType)  {
	std::shared_ptr<eImage> tempResult = nullptr;
	return LoadAndGet(resourceFilename, accessType, tempResult);
}

//***************************
// eImageManager::Load
// convenience function, 
// same as LoadAndGet except 
// defaults param accessType to SDL_TEXTUREACCESS_STATIC
//***************************
bool eImageManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eImage> & result) {
	return LoadAndGet(resourceFilename, SDL_TEXTUREACCESS_STATIC, result);
}