#include "Color.h"



auto operator<< (std::ostream &outStream, const Color &color) -> std::ostream &
{
    outStream<<"R:"<<color.r()<<" G:"<<color.g()<<" B:"<<color.b()<<" A:"<<color.a()<<std::endl;
    return outStream;
}

