#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "Homogeneous4.h"
#include "RGBAValue.h"

namespace MathUtils {


    template<typename T>
    inline auto lerp(const T & a,const T & b,float delta) -> T{
         return a * (1-delta)  + b * delta ;
    }
};


#endif // MATHUTILS_H
