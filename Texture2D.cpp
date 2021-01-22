#include "Texture2D.h"
#include "MathUtils.h"
#include <math.h>

auto Texture2D::sample(const std::pair<float,float> & texCoord) const -> RGBAValue
{
    if(image == nullptr){
        return {};
    }

    if(image->width * image->height == 0){
       return {};
    }
//in here,
//we can use bilinear interpolation.
    int32_t u = std::min(static_cast<long>(texCoord.first * (image->width - 1)),image->width- 1);
    int32_t v = std::min(static_cast<long>((texCoord.second) * (image->height - 1)),image->height- 1);
    return (*image)[v][u];
}

auto Texture2D::setImage(const RGBAImage * image) -> void
{
    this->image = image;
}
