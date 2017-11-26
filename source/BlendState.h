#ifndef EVIL_BLENDSTATE_H
#define EVIL_BLENDSTATE_H

#include "Animation.h"

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
// which controls which animations
// are updating an eRenderImage
//*******************************
class eBlendState {
public:

	friend class eAnimationController;		// sole access to Update

public:

												eBlendState(const std::string & name, 
															const std::vector<std::shared_ptr<eAnimation>> & animations, //************DIFFERENT***********//
															AnimationBlendMode blendMode = AnimationBlendMode::FREEFORM_2D,
															float speed = 1.0f);

	void										SetAnimationController(eAnimationController * newStateMachine);
	float										GetNormalizedTime() const;
	void										SetNormalizedTime(float normalizedTime);
	float										Duration() const;
	float										Time() const;
	const std::string &							Name() const;
	int											NameHash() const;
	const AnimationFrame_t &					GetCurrentFrame() const;

private:

	void										Update();

public:

	float										speed;

private:

	eAnimationController *						stateMachine;			// back-pointer to handler, for access to the component's gameobject owner->renderImage
	std::vector<std::shared_ptr<eAnimation>>	animations;				// which animations this state plays		//************DIFFERENT***********//
	std::string									name;
	int											nameHash;
	AnimationBlendMode							blendMode;				//************DIFFERENT***********//
	float										duration;
	float										time			= 0.0f;
	AnimationFrame_t *							currentFrame	= nullptr;


	//************DIFFERENT***********//
	// FIXME: this needs to be ~ std::vector<std::pair<float, float>> blendNodes[animations.size()]
	// except... that not every eBlendState will use two parameters, perhaps just use a bool in ctor for BlendMode (1D, 2D)
	// and just ignore/zero the second float during compare tests
	std::vector<std::tuple<int, COMPARE_ENUM, float>>	blendNodes;

	// TODO: blendNodes' indexes should run parallel to animations' indexes
	// such that getting an animation by name or nameHash should also return the same index in blendNodes
	// ... which assumes theres an eHashIndex animationsHash (maybe???)
	bool										PositionBlendNode(const std::string & animationName, float xPosition, float yPosition = 0.0f);
	bool										PositionBlendNode(int animationNameHash, float xPosition, float yPosition = 0.0f);

};

//*********************
// eBlendState::eBlendState
//*********************
inline eBlendState::eBlendState(const std::string & name, const std::vector<std::shared_ptr<eAnimation>> & animations, AnimationBlendMode blendMode, float speed)
	: name(name),
	  animations(animations),
	  blendMode(blendMode),
	  speed(speed) {
	currentFrame = &animations[0]->frames[0];
	duration = animations[0]->duration * speed;
	nameHash = std::hash<std::string>()(name);
}

//*********************
// eBlendState::GetNormalizedTime
// returns the fraction of its duration that this state is currently at
// range [0, 1]
//*********************
inline float eBlendState::GetNormalizedTime() const {
	return (time / duration);
}

//*********************
// eBlendState::SetNormalizedTime
//*********************
inline void eBlendState::SetNormalizedTime(float normalizedTime) {
	time = normalizedTime * duration;
}

//*********************
// eBlendState::Duration
// returns the duration of this state in milliseconds
//*********************
inline float eBlendState::Duration() const {
	return duration;
}

//*********************
// eBlendState::Time
// returns the un-normalized time of this state in milliseconds
// range [0, duration]
//*********************
inline float eBlendState::Time() const {
	return time;
}

//*********************
// eBlendState::Name
//*********************
inline const std::string & eBlendState::Name() const {
	return name;
}

//*********************
// eBlendState::NameHash
//*********************
inline int eBlendState::NameHash() const {
	return nameHash;
}

//*********************
// eBlendState::GetCurrentFrame
//*********************
inline const AnimationFrame_t & eBlendState::GetCurrentFrame() const {
	return *currentFrame;
}

//*********************
// eBlendState::SetAnimationController
//*********************
inline void eBlendState::SetAnimationController(eAnimationController * newStateMachine) {
	stateMachine = newStateMachine;
}

#endif /* EVIL_BLENDSTATE_H */