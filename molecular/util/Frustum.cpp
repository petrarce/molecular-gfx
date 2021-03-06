/*	Frustum.cpp

MIT License

Copyright (c) 2019 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "Frustum.h"
#include <molecular/util/AxisAlignedBox.h>

namespace molecular
{
namespace util
{

Frustum::Frustum(const Matrix<4,4>& v)
{
	// http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	mPlanes[kLeft]  = Plane(Vector3(v(3, 0) + v(0, 0), v(3, 1) + v(0, 1), v(3, 2) + v(0, 2)), v(3, 3) + v(0, 3));
	mPlanes[kRight] = Plane(Vector3(v(3, 0) - v(0, 0), v(3, 1) - v(0, 1), v(3, 2) - v(0, 2)), v(3, 3) - v(0, 3));
	mPlanes[kTop]   = Plane(Vector3(v(3, 0) - v(1, 0), v(3, 1) - v(1, 1), v(3, 2) - v(1, 2)), v(3, 3) - v(1, 3));
	mPlanes[kBottom]= Plane(Vector3(v(3, 0) + v(1, 0), v(3, 1) + v(1, 1), v(3, 2) + v(1, 2)), v(3, 3) + v(1, 3));
	mPlanes[kNear]  = Plane(Vector3(v(3, 0) + v(2, 0), v(3, 1) + v(2, 1), v(3, 2) + v(2, 2)), v(3, 3) + v(2, 3));
	mPlanes[kFar]   = Plane(Vector3(v(3, 0) - v(2, 0), v(3, 1) - v(2, 1), v(3, 2) - v(2, 2)), v(3, 3) - v(2, 3));

	for(auto& plane: mPlanes)
	{
		plane.SetDistance(plane.GetDistance() * -1);
		plane.Normalize();
//		std::cerr << plane.GetNormal() << " " << plane.GetDistance() << std::endl;
	}
}

Frustum::Frustum(const Plane planes[6])
{
	for(int i = 0; i < 6; i++)
		mPlanes[i] = planes[i];
}

Plane::IntersectStatus Frustum::Check(const Vector3& point) const
{
	for(auto& plane: mPlanes)
	{
		if(plane.Check(point) == Plane::kOutside)	// outside of plane
			return Plane::kOutside;
	}
	return Plane::kInside;	// inside of all planes
}

Plane::IntersectStatus Frustum::Check(const Vector3& position, float radius) const
{
	bool intersecting = false;
	
	for(auto& plane: mPlanes)
	{
		Plane::IntersectStatus status = plane.Check(position, radius);
		if(status == Plane::kOutside)	// outside of plane
			return Plane::kOutside;
		if(status == Plane::kIntersect)
			intersecting = true;
	}
	if(intersecting)
		return Plane::kIntersect;
	else
		return Plane::kInside;	// inside of all planes
}

Plane::IntersectStatus Frustum::Check(const AxisAlignedBox& box) const
{
	bool intersecting = false;

	for(auto& plane: mPlanes)
	{
		Plane::IntersectStatus status = plane.Check(box);
		if(status == Plane::kOutside)	// outside of plane
			return Plane::kOutside;
		if(status == Plane::kIntersect)
			intersecting = true;
	}
	if(intersecting)
		return Plane::kIntersect;
	else
		return Plane::kInside;
}

std::ostream& operator<<(std::ostream& o, const Frustum& f)
{
	o << "(Near: " << f.GetPlane(Frustum::kNear)
	  << ", Far: " << f.GetPlane(Frustum::kFar)
	  << ", Left: " << f.GetPlane(Frustum::kLeft)
	  << ", Right: " << f.GetPlane(Frustum::kRight)
	  << ", Top: " << f.GetPlane(Frustum::kTop)
	  << ", Bottom: " << f.GetPlane(Frustum::kBottom) << ')';
	return o;
}

}
}
