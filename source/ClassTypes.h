//*******************************************************************
//	all engine of evil classes that can have instantiated objects
//	and are designed to be part of an inheritance chain beyond eClass
//*******************************************************************
REGISTER_ENUM(CLASS_GAMEOBJECT)
REGISTER_ENUM(CLASS_COMPONENT)
REGISTER_ENUM(CLASS_ENTITY)
REGISTER_ENUM(CLASS_TILEIMPL)
REGISTER_ENUM(CLASS_TILE)
REGISTER_ENUM(CLASS_CAMERA)
REGISTER_ENUM(CLASS_MOVEMENT)
REGISTER_ENUM(CLASS_BOUNDS)
REGISTER_ENUM(CLASS_BOUNDS3D)
REGISTER_ENUM(CLASS_BOX)
REGISTER_ENUM(CLASS_COLLISIONMODEL)
REGISTER_ENUM(CLASS_PLAYER)
REGISTER_ENUM(CLASS_MAP)
REGISTER_ENUM(CLASS_STATENODE)
REGISTER_ENUM(CLASS_ANIMATIONSTATE)
REGISTER_ENUM(CLASS_BLENDSTATE)
REGISTER_ENUM(CLASS_STATETRANSITION)
REGISTER_ENUM(CLASS_ANIMATION)
REGISTER_ENUM(CLASS_ANIMATIONCONTROLLER)
REGISTER_ENUM(CLASS_GAME)

REGISTER_ENUM(CLASS_GRIDINDEX)
REGISTER_ENUM(CLASS_TILEKNOWLEDGE)
REGISTER_ENUM(CLASS_GRIDCELL)
REGISTER_ENUM(CLASS_SPATIALINDEXGRID)
REGISTER_ENUM(CLASS_IMAGE)
REGISTER_ENUM(CLASS_IMAGE_MANAGER)
REGISTER_ENUM(CLASS_ENTITYPREFAB_MANAGER)
REGISTER_ENUM(CLASS_ANIMATION_MANAGER)
REGISTER_ENUM(CLASS_ANIMATIONCONTROLLER_MANAGER)
REGISTER_ENUM(CLASS_RENDERER)
REGISTER_ENUM(CLASS_RENDERIMAGE)
REGISTER_ENUM(CLASS_INPUT)

REGISTER_ENUM(CLASS_SHERO)				// TODO: allow user to create a separate REGISTER_ENUM list

#if 0
// not designed for inheritance
REGISTER_ENUM(CLASS_VEC2)
REGISTER_ENUM(CLASS_VEC3)
REGISTER_ENUM(CLASS_QUAT)
REGISTER_ENUM(CLASS_DEQUE)
REGISTER_ENUM(CLASS_BINARYHEAP)
REGISTER_ENUM(CLASS_HASHINDEX)

// static classes
REGISTER_ENUM(CLASS_COLLISION)
REGISTER_ENUM(CLASS_SORT)
REGISTER_ENUM(CLASS_MATH)
#endif