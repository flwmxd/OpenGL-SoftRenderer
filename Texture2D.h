#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include <cstdint>
#include "RGBAValue.h"
#include "RGBAImage.h"

class Texture2D
{
public:
    Texture2D() = default;
    auto sample(const std::pair<float,float> & texCoord) const -> RGBAValue;
    auto setImage(const RGBAImage * image) -> void;
    inline auto getImage() const -> const RGBAImage* { return image;}
private:
    //default as RGBA8888
    const RGBAImage * image = nullptr;
};

#endif // TEXTURE2D_H
