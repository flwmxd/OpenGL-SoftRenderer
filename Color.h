
#pragma once
#include <array>
#include <cstdint>
#include "RGBAValue.h"

class Color
{
public:
    static constexpr size_t LENGTH = 4;

    // [0.0f, 1.0f]
    constexpr Color(const std::array<float, LENGTH> & comps)
        : rgba(comps)
    {
    }

    constexpr Color(const float * comps)
        : Color(comps[0], comps[1], comps[2], comps[3])
    {
    }


    // [0.0f, 1.0f]
    constexpr Color(float red, float green, float blue, float alpha)
        : Color(std::array<float, LENGTH>{red, green, blue, alpha})
    {
    }

    // [0, 255]
    constexpr Color(const std::array<uint8_t, LENGTH> & comps)
        : Color(comps[0], comps[1], comps[2], comps[3])
    {
    }

    // [0, 255]
    constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
        : Color(
            static_cast<float>(red) / 255,
            static_cast<float>(green) / 255,
            static_cast<float>(blue) / 255,
            static_cast<float>(alpha) / 255
        )
    {
    }


    constexpr Color()
        : Color(0.f,0.f,0.f,0.f)
    {

    }



    auto r() const -> float
    {
        return rgba[0];
    }

    auto g() const -> float
    {
        return rgba[1];
    }

    auto b() const -> float
    {
        return rgba[2];
    }

    auto a() const -> float
    {
        return rgba[3];
    }

    inline auto data() const -> const float*
    {
        return rgba.data();
    }

    inline auto data() -> float*
    {
        return rgba.data();
    }


    auto operator*(const Color& o) const -> Color
    {
        return {
            r() * o.r(),
            g() * o.g(),
            b() * o.b(),
            a() * o.a()
        };
    }

    auto operator+(const Color& o) const -> Color
    {
        return {
            r() + o.r(),
            g() + o.g(),
            b() + o.b(),
            a() + o.a()
        };
    }

    auto operator*(float o) const -> Color
    {
        return {
            r() * o,
            g() * o,
            b() * o,
            a() * o
        };
    }

    auto operator/(const Color& o) const -> Color
    {
        return {
            r() / o.r(),
            g() / o.g(),
            b() / o.b(),
            a() / o.a()
        };
    }

    inline auto operator[](int32_t v) const -> const float&{
        return this->rgba[v];
    }

    auto operator==(const Color& o) const -> bool
    {
        return r() == o.r() && g() == o.g() && b() == o.b() && a() == o.a();
    }

    auto operator!=(const Color& o) const -> bool
    {
        return r() != o.r() || g() != o.g() || b() != o.b() || a() != o.a();
    }

    inline auto toRGBAValue() -> RGBAValue
    {
        return RGBAValue(this->rgba[0] * 255,this->rgba[1]* 255,this->rgba[2]* 255,this->rgba[3]* 255);
    }


public:
     std::array<float, LENGTH> rgba;
};

auto operator<< (std::ostream &outStream, const Color &color) -> std::ostream &;

