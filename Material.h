#ifndef MATERIAL_H
#define MATERIAL_H
#include "RGBAValue.h"
#include "Color.h"

class Material
{
public:
    Material() = default;


    //inline all getter and setter

    inline auto getAmbient() const -> const Color&{ return ambient; }
    inline auto getDiffuse() const -> const Color&{ return diffuse; }
    inline auto getSpecular() const -> const Color&{ return specular; }
    inline auto getShininess() const -> double { return shininess; }
    inline auto getEmission() const -> const Color&{ return emission; }

    inline auto setAmbient(const Color& value ) -> void { ambient =  value;}
    inline auto setDiffuse(const Color& value ) -> void { diffuse =  value;}
    inline auto setSpecular(const Color& value ) -> void { specular =  value;}

    inline auto setEmission(const Color& value ) -> void { emission =  value;}
    inline auto setShininess(double value ) -> void { shininess =  value;}
private:
    Color ambient;
    Color diffuse;
    Color specular;
    Color reflect;
    Color emission;
    double shininess ;

};

#endif // MATERIAL_H
