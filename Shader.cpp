#include "Shader.h"
#include <math.h>
#include <algorithm>
#include "MathUtils.h"

auto Shader::bindTexture(const RGBAImage * img) -> void
{
    texture2D.setImage(img);
}

auto Shader::reflect(const Cartesian3 & vec,const Cartesian3 & normal) -> Cartesian3
{
    float dn = 2 * vec.dot(normal);
    return vec - normal * dn;
}

GouraudShadingShader::GouraudShadingShader()
{
    modelViewInverse.SetIdentity();
}

auto GouraudShadingShader::vertexShader(const vertexWithAttributes & vertex,const FakeGL & gl) -> screenVertexWithAttributes
{

    auto mdlvCoord = modelViewMatrix * vertex.position;
    auto projCoord = projectMatrix * mdlvCoord;

    auto mdlvNormal = modelViewInverse * vertex.normal;

    //auto projNormal = projectMatrix * mdlvNormal;

    screenVertexWithAttributes out;
    out.position = projCoord.Point();
    out.divZ = 1.0f / projCoord.w;
    out.normal = mdlvNormal;
    out.colour = vertex.colour;
    out.texCoord = vertex.texCoord;


    const Cartesian3 eyePos = {0,0,0};
    if(light != nullptr)
    {

//calculate the eye direction
        auto eyeDir = eyePos - mdlvCoord.Vector();
        eyeDir.normalize();
 //calculate the lighting direction
        auto lightDir = light->getPosition() - mdlvCoord.Vector();
        lightDir.normalize();
 //calculate the reflection direction
        auto ref = reflect(lightDir,mdlvNormal) ;
        ref.normalize();

        auto diffuse = light->getDiffuse() * gl.stateMechine.material.getDiffuse() * std::max(lightDir.dot(mdlvNormal), 0.0f);
        auto specular = light->getSpecular() * gl.stateMechine.material.getSpecular() * std::pow(std::max(eyeDir.dot(ref),0.0f),gl.stateMechine.material.getShininess());;
        auto emission = gl.stateMechine.material.getEmission();
        auto ambient = light->getAmbient() * gl.stateMechine.material.getAmbient();

//it looks like the OpenGL has a min color for object???
        out.colour =  out.colour * (emission + ambient+ diffuse + specular).toRGBAValue();
        out.colour.alpha = 255;
    }

    return out;
}

auto GouraudShadingShader::fragmentShader(const fragmentWithAttributes & vertex,const FakeGL & gl) -> RGBAValue
{
    if(texture2D.getImage()){
        return texture2D.sample({vertex.texCoord.x,vertex.texCoord.y});;
    }
    return vertex.colour;
}


PhongShadingShader::PhongShadingShader(){
    modelViewInverse.SetIdentity();
}

auto PhongShadingShader::vertexShader(const vertexWithAttributes & vertex,const FakeGL & gl) -> screenVertexWithAttributes
{

    screenVertexWithAttributes screen;
    auto mdlvCoord = modelViewMatrix * vertex.position;
    auto projCoord = projectMatrix * mdlvCoord;

    screen.modelViewCoord = mdlvCoord;
    screen.position = projCoord.Point();
    screen.divZ = 1.0f / projCoord.w;


    screen.normal = modelViewInverse * vertex.normal;
    screen.colour = vertex.colour;
    screen.texCoord = vertex.texCoord;
    return screen;
}

auto Shader::setLight(const Light * light) -> void
{
    this-> light = light;
}
auto Shader::setProjectMatrix(const Matrix4 &project) -> void
{
    projectMatrix = project;
}

auto Shader::setModelViewMatrix(const Matrix4 &modelView) -> void
{
    modelViewMatrix = modelView;
    modelViewInverse = modelViewMatrix.inverse().transpose();
};

auto PhongShadingShader::fragmentShader(const fragmentWithAttributes & fragment,const FakeGL & gl) -> RGBAValue
{
    auto color = fragment.colour;
    if(texture2D.getImage())
    {
        color = texture2D.sample({fragment.texCoord.x,fragment.texCoord.y});
    }

    if(light != nullptr)
    {
        Cartesian3 eyePos = {0,0,0};
        auto fragPos = fragment.modelViewCoord.Vector();
//assume our eye position is zero....
        auto normalizedNormal = fragment.normal;
        normalizedNormal.normalize();
//calculate the lighting direction
        auto lightDir = light->getPosition() - fragPos;
        lightDir.normalize();

        float diff = std::max(normalizedNormal.dot(lightDir), 0.0f);
        auto diffuse =  (light->getDiffuse()*  gl.stateMechine.material.getDiffuse())* diff;
//---------------------
//specular
//calculate the eye direction
        auto viewDir = eyePos - fragPos ;
        viewDir.normalize();

        auto reflectDir = reflect(lightDir, normalizedNormal);
        float spec = std::pow(std::max(viewDir.dot(reflectDir), 0.0f), gl.stateMechine.material.getShininess());
        auto specular = light->getSpecular() * gl.stateMechine.material.getSpecular() *  spec;
//---------------------
//ambient is easy
        auto ambient = light->getAmbient()* gl.stateMechine.material.getAmbient();
        auto & emission =  gl.stateMechine.material.getEmission();
        return  (ambient + diffuse + specular + emission).toRGBAValue() * color;;

    }
    return color;
}




