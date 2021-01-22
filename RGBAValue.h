//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  RGBAValue.h
//  ------------------------
//  
//  A minimal class for a colour in RGB space
//  
///////////////////////////////////////////////////

#ifndef RGBAVALUE_H
#define RGBAVALUE_H

#include <iostream>

class RGBAValue
    { // RGBAValue
    public:
    // just a container for the components
    unsigned char red, green, blue, alpha;

    // default constructor
    RGBAValue();

    // value constructor with default alpha
    RGBAValue(unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha = 255);

    // value constructor with floats
    // values outside 0.0-255.0 are clamped
    RGBAValue(float Red, float Green, float Blue, float Alpha);

    RGBAValue(const float * buffer);

    // copy constructor
    RGBAValue(const RGBAValue &other);
    
    // colour modulation routine:
    // NB: this routine scales each component by 1/255.0, multiplies then inverts
    RGBAValue modulate(const RGBAValue &right) const;
    
    }; // RGBAValue

// convenience routines for scalar multiplication and addition
RGBAValue operator *(float scalar, const RGBAValue &colour);

//override by prime zeng
RGBAValue operator *(const RGBAValue &colour,float scalar);
RGBAValue operator *(const RGBAValue &a,const RGBAValue &b);
/////

RGBAValue operator -(const RGBAValue &left, const RGBAValue &right);


RGBAValue operator +(const RGBAValue &left, const RGBAValue &right);

// stream input
// WARNING - always sets alpha to 255
std::istream & operator >> (std::istream &inStream, RGBAValue &value);

// stream output
// WARNING - does not output alpha
std::ostream & operator << (std::ostream &outStream, const RGBAValue &value);

#endif
