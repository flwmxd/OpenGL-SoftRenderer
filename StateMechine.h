#ifndef STATEMECHINE_H
#define STATEMECHINE_H

#include <cstdint>
#include "RGBAValue.h"
#include "RGBAImage.h"
#include "Cartesian3.h"
#include "Matrix4.h"
#include "Material.h"
#include "Light.h"

#include <stack>
#include <memory>

class Shader;

struct GLViewport
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    float zNear = 0;
    float zFar = 0;
};


struct CurrentSurface
{
    Cartesian3 normal;
    RGBAValue color;
    Cartesian3 textCoord;
};


//current opengl state mechine
class StateMechine
{
public:
    GLViewport viewport;
    CurrentSurface currentSurface;

    //stack for mvp matrix
    std::stack<Matrix4> modelViewMatrixStack;
    std::stack<Matrix4> projectionMatrixStack;

    uint32_t matrixMode = 0;
    int32_t drawType = -1;
    int32_t envMode = -1;
    int32_t lineWidth = 1;
    int32_t pointSize = 1;

    //flags for indicating whether it open
    bool enables[5] = {false};

    Material material;

    //GL Default Z range 
    float zNear = 0;
    float zFar = 1;

    //param for light
    Light light;


    //current shader
    //Gouraud or Phong
    std::shared_ptr<Shader> currentShader;

    Matrix4 viewportMatrix;
    auto getCurrentSelectedMatrix() -> Matrix4 *;

    RGBAImage texture;
    RGBAValue clearColor;

};


#endif // STATEMECHINE_H
