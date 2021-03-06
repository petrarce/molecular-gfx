/*	Transform.h

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
SOFTWARE.
*/

#ifndef MOLECULAR_TRANSFORM_H
#define MOLECULAR_TRANSFORM_H

#include <molecular/gfx/RenderFunction.h>
#include <molecular/util/Matrix4.h>

namespace molecular
{
namespace gfx
{

class Transform : public SingleCalleeRenderFunction
{
public:
	template<class TRenderManager>
	Transform(TRenderManager &manager) :
		SingleCalleeRenderFunction(manager),
		mBoundsChangedFramecounter(manager.GetFramecounter())
	{
	}

	molecular::util::AxisAlignedBox GetBounds() const override;
	bool BoundsChangedSince(int framecounter) const override;

	void SetTransform(const Matrix4& transform)
	{
		mTransform = transform;
//		mBoundsChangedFramecounter = mRenderManager.GetFramecounter();
	}

protected:
	void HandleExecute(Scope& scope) override;

private:
	Matrix4 mTransform;
	int mBoundsChangedFramecounter;
};

}
}

#endif // MOLECULAR_TRANSFORM_H
