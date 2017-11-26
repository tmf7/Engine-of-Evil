#ifndef EVIL_BLENDSTATE_H
#define EVIL_BLENDSTATE_H

#include "StateNode.h"
#include "HashIndex.h"
#include "Vector.h"

class eAnimationController;

enum class AnimationBlendMode {
	SIMPLE_1D,
	FREEFORM_2D
//	DIRECT			// TODO: directly control node weights, and ignore squared distance auto-weights
};

//******************************
//		eBlendState
// used by eAnimationController
// as part of its state machine
// controls a vector of animations
// according to blend paramaters and mode
// see also: eAnimationState
//*******************************
class eBlendState : public eStateNode {
public:

	friend class eAnimationController;			// sole access to Update

public:
												// FIXME: if blendMode == 2D, then two controller floats MUST be defined at load-time
												// or just default the second one to equal the first and ignore it during Update
												eBlendState(const std::string & name, 
															const std::vector<std::shared_ptr<eAnimation>> & animations,
															float * xBlendParameter,
															float * yBlendParameter = nullptr,
															AnimationBlendMode blendMode = AnimationBlendMode::SIMPLE_1D,
															float speed = 1.0f);

	virtual int									GetClassType() const override { return CLASS_BLENDSTATE; }

private:

	virtual void								Update() override;
	void										SwapAnimation(int animationIndex);

private:

	std::vector<std::shared_ptr<eAnimation>>	animations;				// which animations this state plays
	AnimationBlendMode							blendMode;
	int											currentAnimationIndex	= 0;

	// DEBUG: blendNodes' indexes run parallel to animations' indexes
	eHashIndex									blendNodesHash;		// indexed by eAnimation::name
	std::vector<eVec2>							blendNodes;

	// pointers to eAnimationController::floatParameters index to listen to for blendNodes comparison
	float *										xBlendParameter;
	float *										yBlendParameter;

	bool										PositionBlendNode(const std::string & animationName, float xPosition, float yPosition = 0.0f);
	bool										PositionBlendNode(int animationNameHash, float xPosition, float yPosition = 0.0f);

};

//*********************
// eBlendState::eBlendState
//*********************
inline eBlendState::eBlendState(const std::string & name, const std::vector<std::shared_ptr<eAnimation>> & animations, float * xBlendParameter, float * yBlendParameter, AnimationBlendMode blendMode, float speed)
	: animations(animations),
	  xBlendParameter(xBlendParameter),
	  yBlendParameter(yBlendParameter),
	  blendMode(blendMode) {
	this->speed = speed;
	this->name = name;

	currentFrame = &animations[0]->GetFrame(0);
	duration = animations[0]->Duration() * speed;

	nameHash = std::hash<std::string>()(name);

	blendNodes.assign(animations.size(), vec2_zero);
	blendNodesHash.ClearAndResize(animations.size());
	for (size_t index = 0; index < animations.size(); ++index)
		blendNodesHash.Add(animations[index]->NameHash(), index);
}

//*********************
// eBlendState::PositionBlendNode
// assigns the values to which the eAnimationController 
// stateMachine's paramaters will be compared
// returns true if the animationName is correct and therefore has a corresponding blendNode
// returns false if animationName is not in eBlendState::animations
//*********************
inline bool eBlendState::PositionBlendNode(const std::string & animationName, float xPosition, float yPosition) {
	const int index = blendNodesHash.First(blendNodesHash.GetHashKey(animationName));
	if (index == -1)
		return false;

	blendNodes[index].x = xPosition;
	blendNodes[index].y = yPosition;
	return true;
}

//*********************
// eBlendState::PositionBlendNode
// same as PositionBlendNode(std::string)
// except it assumes the user has cached the hashKey
//*********************
inline bool eBlendState::PositionBlendNode(int animationNameHash, float xPosition, float yPosition) {
	const int index = blendNodesHash.First(animationNameHash);
	if (index == -1)
		return false;

	blendNodes[index].x = xPosition;
	blendNodes[index].y = yPosition;
	return true;
}

//*********************
// eBlendState::SwapAnimation
// switches which animation this state is playing
// using the same normalized time
//*********************
inline void eBlendState::SwapAnimation(int animationIndex) {
	const float normalizedTime = (time / duration);
	currentAnimationIndex = animationIndex;
	duration = animations[currentAnimationIndex]->Duration() * speed;
	time = normalizedTime * duration;
}

#endif /* EVIL_BLENDSTATE_H */