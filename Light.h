#ifndef LIGHT_H
#define LIGHT_H
#include "RGBAValue.h"
#include "Cartesian3.h"
#include "Material.h"
#include "Color.h"

#include <memory.h>

class Light
{
public:

    Light() = default;

    //inline all getter and setter

    inline auto getAmbient() const ->  const Color& { return ambient; }
    inline auto getDiffuse() const ->  const Color& { return diffuse; }
    inline auto getSpecular() const -> const Color& { return specular; }
    inline auto getPosition() const -> const Cartesian3&{ return position; }



    inline auto setAmbient(const float* value ) -> void {ambient = value;}
    inline auto setDiffuse(const float* value ) -> void { diffuse= value;}
    inline auto setSpecular(const float* value) -> void { specular= value;}
    inline auto setPosition(const Cartesian3& value ) -> void { position =  value;}


private:
    Cartesian3 position;
    Color ambient;
    Color diffuse;
    Color specular;
};


#endif // LIGHT_H
