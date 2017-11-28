#include "ResourceManager.h"
// #include "Game.h"

//***************************
// eResourceManager::Load
// convenience function, 
// same as LoadAndGet except 
// it only checks load success/failure
//***************************
template<class type>
bool eResourceManager<type>::Load(const char * resourceFilename)  {
	std::shared_ptr<type> tempResult = nullptr;
	return LoadAndGet(resourceFilename, tempResult);			// FIXME(?): this will likely try to call the pure virtual LoadAndGet (not any derived LoadAndGet)
}

//*************************
// eResourceManager::Clear
// clears all pointers to the current set 
// of resources, which allows them
// to be deleted once no object is using them,
// allows for new resources to load
// to optimize runtime memory usage
//*************************
template<class type>
void eResourceManager<type>::Clear() {
	resourceList.clear(); 
	resourceHash.Clear(); 
}

//*************************
// eResourceManager::Unload
// selective unloading of the resource
// with param resourceID
// DEBUG: user can compare resources used between levels/scenes
// to unload only those resources that are no longer needed
// to optimize runtime memory usage
//*************************
template<class type>
void eResourceManager<type>::Unload(int resourceID) {
	resourceList.erase(resourceID);
	const int hashKey = resourceHash.GetHashKey(resourceList[resourceID]->GetSourceFilename());	// TODO: have each resource save its own hashKey/nameHash
	resourceHash.RemoveIndex(hashKey, resourceID);
}

//*************************
// eResourceManager::Get
// returns the resource with the given resourceID
// DEBUG: no range checking occurs
//*************************
template<class type>
std::shared_ptr<type> & eResourceManager<type>::Get(int resourceID) const {
	return resourceList[resourceID];  
}

//***************************
// eResourceManager::Get
// returns an resource pointer if it exists
// if param resourceFilename is null or the resource doesn't exist
// then it returns the default error resource pointer
//***************************
template<class type>
std::shared_ptr<type> & eResourceManager<type>::Get(const char * resourceFilename) {
	if (!resourceFilename) 
		return resourceList[0]; // default error resource

	// search for pre-existing texture
	int hashKey = resourceHash.GetHashKey(std::string(resourceFilename));
	for (int i = resourceHash.First(hashKey); i != -1; i = resourceHash.Next(i)) {
		if (resourceList[i]->GetSourceFilename() == resourceFilename)
			return resourceList[i];
	}
	return resourceList[0];		// default error resource
}