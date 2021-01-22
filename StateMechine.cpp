#include "StateMechine.h"
#include "FakeGL.h"
#include <cassert>

auto StateMechine::getCurrentSelectedMatrix() -> Matrix4 *
{
    Matrix4 * matrix = nullptr;
    switch (matrixMode)
    {
    case FAKEGL_MODELVIEW:
        matrix = &modelViewMatrixStack.top();
    break;
    case FAKEGL_PROJECTION:
        matrix =  &projectionMatrixStack.top();
    break;

    }
    assert(matrix);
    return matrix;
}