#ifndef EVIL_BOUNDS3D_H
#define EVIL_BOUNDS3D_H

#include "Vector.h"
#include "Class.h"

//**********************************
//			eBounds3D
// 3D Axis-Aligned bounding box
// TODO: inherit from a eCollider class
// for more general collision shape assignment
//**********************************
class eBounds3D : public eClass {
public:
					eBounds3D();
	explicit		eBounds3D(const eVec3 & mins, const eVec3 & maxs);
	explicit		eBounds3D(const eVec3 & point);

	const eVec3 &	operator[](const int index) const;
	eVec3 &			operator[](const int index);
	eBounds3D		operator+(const eVec3 & translation) const;
	eBounds3D &		operator+=(const eVec3 & translation);
	bool			operator==(const eBounds3D & a) const;
	bool			operator!=(const eBounds3D & a) const;
	
	bool			Compare(const eBounds3D & a) const;
	bool			Compare(const eBounds3D & a, const float epsilon) const;

	eBounds3D		Translate(const eVec3 & translation) const;		
	eBounds3D &		TranslateSelf(const eVec3 & translation);		
	eBounds3D		Expand(const float range) const;				
	eBounds3D &		ExpandSelf(const float range);

	void			Zero();											
	eVec3			Center() const;									
	float			Radius() const;	
	const eVec3 &	Size() const;
	float			Width() const;
	float			Height() const;
	float			Depth() const;

	void			FromIsometricRotation(const eVec3 & origin);
	void			FromPoints(const eVec3 * points, const int numPoints);
	void			ToPoints(eVec3 points[8]) const;

	virtual int		GetClassType() const override				{ return CLASS_BOUNDS3D; }
	virtual bool	IsClassType(int classType) const override	{ 
						if(classType == CLASS_BOUNDS3D) 
							return true; 
						return eClass::IsClassType(classType); 
					}

private:

	eVec3			bounds[2];			// mins at [0] and maxs at [1]
	eVec3			quickSize;			// cached x=width, y=height, and z=depth
};

//*************
// eBounds3D::eBounds3D
// single point on origin (0, 0, 0)
//*************
inline eBounds3D::eBounds3D() {
	bounds[0] = vec3_zero;
	bounds[1] = vec3_zero;
}

//*************
// eBounds3D::eBounds3D
//*************
inline eBounds3D::eBounds3D(const eVec3 & mins, const eVec3 & maxs) {
	bounds[0] = mins;
	bounds[1] = maxs;
	quickSize = eVec3(Width(), Height(), Depth());
}

//*************
// eBounds3D::eBounds3D
//*************
inline eBounds3D::eBounds3D(const eVec3 & point) {
	bounds[0] = point;
	bounds[1] = point;
}

//*************
// eBounds3D::operator[]
// returns mins for [0] and maxs for [1] 
// undefined behavior for index out of bounds
//*************
inline const eVec3 & eBounds3D::operator[](const int index) const {
	return bounds[index];
}

//*************
// eBounds3D::operator[]
// returns mins for [0] and maxs for [1] 
// undefined behavior for index out of bounds
//*************
inline eVec3 & eBounds3D::operator[](const int index) {
	return bounds[index];
}

//*************
// eBounds3D::operator+
// returns translated bounds
//*************
inline eBounds3D eBounds3D::operator+(const eVec3 & translation) const {
	return eBounds3D(bounds[0] + translation, bounds[1] + translation);
}


//*************
// eBounds3D::operator+=
// translate the bounds
//*************
inline eBounds3D & eBounds3D::operator+=(const eVec3 & translation) {
	bounds[0] += translation;
	bounds[1] += translation;
	return *this;
}

//*************
// eBounds3D::Compare
//*************
inline bool eBounds3D::Compare(const eBounds3D & a) const {
	return (bounds[0].Compare(a.bounds[0]) && bounds[1].Compare(a.bounds[1]));
}

//*************
// eBounds3D::Compare
//*************
inline bool eBounds3D::Compare(const eBounds3D & a, const float epsilon) const {
	return (bounds[0].Compare(a.bounds[0], epsilon) && bounds[1].Compare(a.bounds[1], epsilon));
}

//*************
// eBounds3D::operator==
//*************
inline bool eBounds3D::operator==(const eBounds3D & a) const {
	return Compare(a);
}

//*************
// eBounds3D::operator!=
//*************
inline bool eBounds3D::operator!=(const eBounds3D & a) const {
	return !Compare(a);
}

//*************
// eBounds3D::Zero
// single point at origin
//*************
inline void eBounds3D::Zero() {
	bounds[0].x = bounds[0].y = bounds[1].x = bounds[1].y = bounds[0].z = bounds[1].z = 0.0f;
	quickSize = eVec3(Width(), Height(), Depth());
}

//*************
// eBounds3D::Center
// returns center of bounds
//*************
inline eVec3 eBounds3D::Center() const {
	return eVec3((bounds[1].x + bounds[0].x) * 0.5f, (bounds[1].y + bounds[0].y) * 0.5f, (bounds[1].z + bounds[0].z) * 0.5f);
}

//*************
// eBounds3D::Radius
// returns the radius relative to the bounds origin (0,0,0)
//*************
inline float eBounds3D::Radius() const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for (i = 0; i < 3; i++) {
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
//	eVec3 center = (bounds[0] + bounds[1]) * 0.5f;
//	eVec3 extents = bounds[1] - center;
//	return extents.Length();
}

//*************
// eBounds3D::Translate
// return translated bounds
//*************
inline eBounds3D eBounds3D::Translate(const eVec3 & translation) const {
	return eBounds3D(bounds[0] + translation, bounds[1] + translation);
}

//*************
// eBounds3D::TranslateSelf
// translate this bounds
//*************
inline eBounds3D & eBounds3D::TranslateSelf(const eVec3 & translation) {
	bounds[0] += translation;
	bounds[1] += translation;
	return *this;
}

//*************
// eBounds3D::Expand
// return a new bounds equal to *this expanded in all directions with the given value
//*************
inline eBounds3D eBounds3D::Expand(const float range) const {
	return eBounds3D(eVec3(bounds[0].x - range, bounds[0].y - range, bounds[0].z - range),
		eVec3(bounds[1].x + range, bounds[1].y + range, bounds[1].z + range));
}

//*************
// eBounds3D::ExpandSelf
// expand bounds in all directions with the given value
//*************
inline eBounds3D & eBounds3D::ExpandSelf(const float range) {
	bounds[0].x -= range;
	bounds[0].y -= range;
	bounds[0].z -= range;
	bounds[1].x += range;
	bounds[1].y += range;
	bounds[1].z += range;
	quickSize = eVec3(Width(), Height(), Depth());
	return *this;
}

//*************
// eBounds3D::Size
//*************
inline const eVec3 & eBounds3D::Size() const {
	return quickSize;
}

//*************
// eBounds3D::Width
//*************
inline float eBounds3D::Width() const {
	return bounds[1].x - bounds[0].x;
}

//*************
// eBounds3D::Height
//*************
inline float eBounds3D::Height() const {
	return bounds[1].y - bounds[0].y;
}

//*************
// eBounds3D::Depth
//*************
inline float eBounds3D::Depth() const {
	return bounds[1].z - bounds[0].z;
}

//*************
// eBounds3D::ToPoints
// points is filled in a clockwise winding order from topleft 
//*************
inline void eBounds3D::ToPoints(eVec3 points[8]) const {
	for (int i = 0; i < 8; i++) {
		points[i][0] = bounds[(i ^ (i >> 1)) & 1][0];
		points[i][1] = bounds[(i >> 1) & 1][1];
		points[i][2] = bounds[( i >> 2 ) & 1][2];
	}
}

#endif  /* EVIL_BOUNDS3D_H */