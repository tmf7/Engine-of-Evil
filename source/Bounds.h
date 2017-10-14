#ifndef EVIL_BOUNDS_H
#define EVIL_BOUNDS_H

#include "Vector.h"

//**********************************
//			eBounds
// 2D Axis-Aligned bounding box
//**********************************
class eBounds {
public:
					eBounds();
	explicit		eBounds(const eVec2 & mins, const eVec2 & maxs);
	explicit		eBounds(const eVec2 & point);

	const eVec2 &	operator[](const int index) const;
	eVec2 &			operator[](const int index);
	eBounds			operator+(const eVec2 & translation) const;
	eBounds &		operator+=(const eVec2 & translation);
	bool			operator==(const eBounds & a) const;
	bool			operator!=(const eBounds & a) const;
	
	bool			Compare(const eBounds & a) const;
	bool			Compare(const eBounds & a, const float epsilon) const;

	eBounds			Translate(const eVec2 & translation) const;		
	eBounds &		TranslateSelf(const eVec2 & translation);		
	eBounds			Expand(const float range) const;				
	eBounds &		ExpandSelf(const float range);

	void			Zero();		
	void			Clear();
	bool			IsEmpty() const;									
	eVec2			Center() const;									
	float			Radius() const;									
	float			Width() const;
	float			Height() const;

	void			FromIsometricRotation(const eVec2 & origin);
	void			FromPoints(const eVec2 * points, const int numPoints);
	void			ToPoints(eVec2 points[4]) const;

private:

	eVec2			bounds[2];			// mins at [0] and maxs at [1]
};

//*************
// eBounds::eBounds
// single point on origin (0, 0)
//*************
inline eBounds::eBounds() {
	bounds[0] = vec2_zero;
	bounds[1] = vec2_zero;
}

//*************
// eBounds::eBounds
//*************
inline eBounds::eBounds(const eVec2 & mins, const eVec2 & maxs) {
	bounds[0] = mins;
	bounds[1] = maxs;
}

//*************
// eBounds::eBounds
//*************
inline eBounds::eBounds(const eVec2 & point) {
	bounds[0] = point;
	bounds[1] = point;
}

//*************
// eBounds::operator[]
// returns mins for [0] and maxs for [1] 
// undefined behavior for index out of bounds
//*************
inline const eVec2 & eBounds::operator[](const int index) const {
	return bounds[index];
}

//*************
// eBounds::operator[]
// returns mins for [0] and maxs for [1] 
// undefined behavior for index out of bounds
//*************
inline eVec2 & eBounds::operator[](const int index) {
	return bounds[index];
}

//*************
// eBounds::operator+
// returns translated bounds
//*************
inline eBounds eBounds::operator+(const eVec2 & translation) const {
	return eBounds(bounds[0] + translation, bounds[1] + translation);
}


//*************
// eBounds::operator+=
// translate the bounds
//*************
inline eBounds & eBounds::operator+=(const eVec2 & translation) {
	bounds[0] += translation;
	bounds[1] += translation;
	return *this;
}

//*************
// eBounds::Compare
//*************
inline bool eBounds::Compare(const eBounds & a) const {
	return (bounds[0].Compare(a.bounds[0]) && bounds[1].Compare(a.bounds[1]));
}

//*************
// eBounds::Compare
//*************
inline bool eBounds::Compare(const eBounds & a, const float epsilon) const {
	return (bounds[0].Compare(a.bounds[0], epsilon) && bounds[1].Compare(a.bounds[1], epsilon));
}

//*************
// eBounds::operator==
//*************
inline bool eBounds::operator==(const eBounds & a) const {
	return Compare(a);
}

//*************
// eBounds::operator!=
//*************
inline bool eBounds::operator!=(const eBounds & a) const {
	return !Compare(a);
}

//*************
// eBounds::Zero
// single point at origin
//*************
inline void eBounds::Zero() {
	bounds[0].x = bounds[0].y = bounds[1].x = bounds[1].y = 0.0f;
}

//*************
// eBounds::Clear
// inside-out bounds (mins > maxs)
//*************
inline void eBounds::Clear() {
	bounds[0][0] = bounds[0][1] = 1.0f;
	bounds[1][0] = bounds[1][1] = -1.0f;

}

//*************
// eBounds::IsEmpty
// true if the bounds is inside-out
//*************
inline bool eBounds::IsEmpty() const {
	return (bounds[0] == -bounds[1]);
}

//*************
// eBounds::Center
// returns center of bounds
//*************
inline eVec2 eBounds::Center() const {
	return eVec2((bounds[1].x + bounds[0].x) * 0.5f, (bounds[1].y + bounds[0].y) * 0.5f);
}

//*************
// eBounds::Radius
// returns the radius relative to the bounds origin (0,0)
//*************
inline float eBounds::Radius() const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for (i = 0; i < 2; i++) {
		b0 = (float)SDL_fabs(bounds[0][i]);
		b1 = (float)SDL_fabs(bounds[1][i]);

		if (b0 > b1)
			total += b0 * b0;
		else 
			total += b1 * b1;
	}
	return SDL_sqrtf(total);

	// TODO: alternatively (test equivalence)
	// (performance) may be slower due to the extra function call overhead
	// same for eVec3
//	eVec2 center = (bounds[0] + bounds[1]) * 0.5f;
//	eVec2 extents = bounds[1] - center;
//	return extents.Length();
}

//*************
// eBounds::Translate
// return translated bounds
//*************
inline eBounds eBounds::Translate(const eVec2 & translation) const {
	return eBounds(bounds[0] + translation, bounds[1] + translation);
}

//*************
// eBounds::TranslateSelf
// translate this bounds
//*************
inline eBounds & eBounds::TranslateSelf(const eVec2 & translation) {
	bounds[0] += translation;
	bounds[1] += translation;
	return *this;
}

//*************
// eBounds::Expand
// return bounds expanded in all directions with the given value
//*************
inline eBounds eBounds::Expand(const float range) const {
	return eBounds(eVec2(bounds[0].x - range, bounds[0].y - range),
		eVec2(bounds[1].x + range, bounds[1].y + range));
}

//*************
// eBounds::ExpandSelf
// expand bounds in all directions with the given value
//*************
inline eBounds & eBounds::ExpandSelf(const float range) {
	bounds[0].x -= range;
	bounds[0].y -= range;
	bounds[1].x += range;
	bounds[1].y += range;
	return *this;
}

//*************
// eBounds::Width
//*************
inline float eBounds::Width() const {
	return bounds[1].x - bounds[0].x;
}

//*************
// eBounds::Height
//*************
inline float eBounds::Height() const {
	return bounds[1].y - bounds[0].y;
}

//*************
// eBounds::ToPoints
// points is filled in a clockwise winding order from topleft 
//*************
inline void eBounds::ToPoints(eVec2 points[4]) const {
	for (int i = 0; i < 4; i++) {
		points[i][0] = bounds[(i ^ (i >> 1)) & 1][0];
		points[i][1] = bounds[(i >> 1) & 1][1];
	}
}

#endif  /* EVIL_BOUNDS_H */