#ifndef EVIL_IMAGE_TILER_MANAGER_H
#define EVIL_IMAGE_TILER_MANAGER_H

#include "Definitions.h"
#include "ImageTiler.h"
#include "HashIndex.h"
#include "Renderer.h"

//**********************************
//			eImageTilerManager
// Handles all texture allocation and freeing
// DEBUG: --no other object/system should allocate/free textures--
//**********************************
class eImageTilerManager {
public:

	bool			Init();
	bool			BatchLoad(const char * tilerBatchLoadFile);
	bool			GetTiler(const char * filename, std::shared_ptr<eImageTiler> & result);
	bool			GetTiler(int tilerID, std::shared_ptr<eImageTiler> & result);
	bool			LoadTiler(const char * filename, std::shared_ptr<eImageTiler> & result);
	int				GetNumTilers() const;
	void			Clear();

private:

	std::vector<std::shared_ptr<eImageTiler>>		tilerList;			// dynamically allocated tiler resources
	eHashIndex										tilerFilenameHash;	// quick access to tilerList
};

//***************
// eImageTilerManager::GetNumImages
//***************
inline int eImageTilerManager::GetNumTilers() const {
	return tilerList.size();
}

#endif /* EVIL_IMAGE_TILER_MANAGER_H */