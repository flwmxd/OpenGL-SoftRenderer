#ifndef SHADER_H
#define SHADER_H
#include "FakeGL.h"
#include "Texture2D.h"


//interface for shader.

class Shader
{
public:
    Shader() = default;
    virtual ~Shader() = default;

    //Vertex/Fragment Shader
    virtual auto vertexShader(const vertexWithAttributes & vertex,const FakeGL & gl) -> screenVertexWithAttributes = 0;
    virtual auto fragmentShader(const fragmentWithAttributes & fragment,const FakeGL & gl) -> RGBAValue = 0;

    auto setModelViewMatrix(const Matrix4 &modelView) -> void;
    auto setProjectMatrix(const Matrix4 &project) -> void;
    auto bindTexture(const RGBAImage * img) -> void;
    auto setLight(const Light * light) -> void;

    //glsl build-in function
    auto reflect(const Cartesian3 & vec,const Cartesian3 & normal) -> Cartesian3;

protected:
    Matrix4 modelViewInverse;
    Matrix4 modelViewMatrix;
    Matrix4 projectMatrix;
    Texture2D texture2D;
    const Light * light = nullptr;
};

//made it like programable-pipeline

class GouraudShadingShader : public Shader
{
public:
    GouraudShadingShader();
    auto vertexShader(const vertexWithAttributes & vertex,const FakeGL & gl) -> screenVertexWithAttributes override;
    auto fragmentShader(const fragmentWithAttributes & fragment,const FakeGL & gl) -> RGBAValue override;
};


class PhongShadingShader : public Shader
{
 public:
    PhongShadingShader();
    auto vertexShader(const vertexWithAttributes & vertex,const FakeGL & gl) -> screenVertexWithAttributes override;
    auto fragmentShader(const fragmentWithAttributes & fragment,const FakeGL & gl) -> RGBAValue override;
};

#endif // SHADER_H
