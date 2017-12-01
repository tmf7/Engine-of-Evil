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

	friend class eAnimationController;			// for direct access to Update
	friend class eAnimationControllerManager;	// for direct access to AddBlendNode


public:
												eBlendState(const std::string & name,
															int numAnimations,
															float * xBlendParameter,
															float * yBlendParameter = nullptr,
															AnimationBlendMode blendMode = AnimationBlendMode::SIMPLE_1D,
															float speed = 1.0f);

	virtual int									GetClassType() const override { return CLASS_BLENDSTATE; }

private:

	void										Init();
	bool										AddBlendNode(const std::string & animationName, float xPosition, float yPosition = 0.0f);
	virtual void								Update() override;
	void										SwapAnimation(int animationIndex);

private:

	std::vector<std::shared_ptr<eAnimation>>	animations;				// which animations this state plays
	AnimationBlendMode							blendMode;
	int											currentAnimationIndex;

	// DEBUG: blendNodes' indexes run parallel to animations' indexes,
	// however the blendNodes are (x,y) pairs of blendParameter values
	eHashIndex									blendNodesHash;		// indexed by eAnimation::name
	std::vector<eVec2>							blendNodes;

	// pointers to eAnimationController::floatParameters index to listen to for blendNodes comparison
	float *										xBlendParameter;
	float *										yBlendParameter;

};

#endif /* EVIL_BLENDSTATE_H */