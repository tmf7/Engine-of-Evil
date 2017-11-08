#ifndef EVIL_BOX_H
#define EVIL_BOX_H

#include "Bounds.h"

//**********************************
//			eBox
// 2D Oriented bounding box
// TODO: inherit from a eCollider class
// for more general collision shape assignment
//**********************************
class eBox : public eClass {
public:
					eBox();
	explicit		eBox(const eVec2 & center, const eVec2 & extents, const eVec2 axis[2]);
	explicit		eBox(const eVec2 & point);
	explicit		eBox(const eVec2 points[3]);
	explicit		eBox(const eBounds & bounds);
	explicit		eBox(const eVec2 & corner, const eVec2 edges[2]);

	eBox			operator+(const eVec2 & translation) const;
	eBox &			operator+=(const eVec2 & translation);						

	eBox			Translate(const eVec2 & translation) const;
	eBox &			TranslateSelf(const eVec2 & translation);
	eBox			Expand(const float range) const;
	eBox &			ExpandSelf(const float range);

	void			Zero();											
	const eVec2 &	Center() const;
	const eVec2 &	Extents() const;
	const eVec2 *	Axes() const;

	virtual int		GetClassType() const override { return CLASS_BOX; }

private:

	eVec2			center;			// world-space center
	eVec2			extents;		// positive distance along local x and y axes
	eVec2			axes[2];		// unit-length locally oriented x and y axes
									// DEBUG: column-major matrix
};

//*************
// eBox::eBox
//*************
inline eBox::eBox()
	: center(vec2_zero),
	  extents(vec2_zero) {
	axes[0] = vec2_oneZero;
	axes[1] = vec2_zeroOne;
}

//*************
// eBox::eBox
//*************
inline eBox::eBox(const eVec2 & point)  
	: center(point),
	extents(vec2_zero) {
	axes[0] = vec2_oneZero;
	axes[1] = vec2_zeroOne;
}


//*************
// eBox::eBox
//*************
inline eBox::eBox(const eVec2 & center, const eVec2 & extents, const eVec2 axes[2])
	: center(center),
	  extents(extents) {
	this->axes[0] = axes[0];
	this->axes[1] = axes[1];
}

//*************
// eBox::eBox
//*************
inline eBox::eBox(const eBounds & bounds) {
	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;
	axes[0] = vec2_oneZero;
	axes[1] = vec2_zeroOne;
}

//*************
// eBox::operator+
// returns the translated box
//*************
inline eBox eBox::operator+(const eVec2 & translation) const {
	return eBox(center + translation, extents, axes);
}

//*************
// eBox::operator+
// translates this box
//*************
inline eBox & eBox::operator+=(const eVec2 & translation) {
	center += translation;
	return *this;
}

//*************
// eBox::Zero
// single point at origin
//*************
inline void eBox::Zero() {
	center.Zero();
	extents.Zero();
	axes[0] = vec2_oneZero;
	axes[1] = vec2_zeroOne;
}

//*************
// eBox::Center
//*************
inline const eVec2 & eBox::Center() const {
	return center;
}

//*************
// eBox::Extents
//*************
inline const eVec2 & eBox::Extents() const {
	return extents;
}

//*************
// eBox::Axes
//*************
inline const eVec2 * eBox::Axes() const {
	return axes;
}

//*************
// eBox::Expand
//*************
inline eBox eBox::Expand(const float d) const {
	return eBox(center, extents + eVec2(d, d), axes);
}

//*************
// eBox::ExpandSelf
//*************
inline eBox & eBox::ExpandSelf(const float d) {
	extents[0] += d;
	extents[1] += d;
	return *this;
}

//*************
// eBox::Translate
//*************
inline eBox eBox::Translate(const eVec2 & translation) const {
	return eBox(center + translation, extents, axes);
}

//*************
// eBox::TranslateSelf
//*************
inline eBox & eBox::TranslateSelf(const eVec2 & translation) {
	center += translation;
	return *this;
}

#endif /* EVIL_BOX_H */