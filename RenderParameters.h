/////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  -----------------------------
//  Render Parameters
//  -----------------------------
//  
//  This is part of the "model" in the MVC paradigm
//  We separate out the render parameters from the object rendered
//
/////////////////////////////////////////////////////////////////

// include guard
#ifndef _RENDER_PARAMETERS_H
#define _RENDER_PARAMETERS_H

#include "Matrix4.h"

// class for the render parameters
class RenderParameters
    { // class RenderParameters
    public:
    
    // we have a widget with an arcball which stores the rotation
    // we'll be lazy and leave it there instead of moving it here
    
    // we store x & y translations
    float xTranslate, yTranslate;

    // and a zoom scale
    float zoomScale;
    
    // we have the position of the light
    float lightPosition[4];
    
    // we will want two homogeneous matrices holding the rotations for the
    // model and the light.
    Matrix4 rotationMatrix;
    Matrix4 lightMatrix;
    
    // and the various lighting parameters
    float emissiveLight;
    float ambientLight;
    float diffuseLight;
    float specularLight;
    float specularExponent;
    
    // and the booleans
    bool useLighting;
    bool texturedRendering;
    bool textureModulation;
    bool depthTestOn;
    
    bool showAxes;
    bool showObject;
    bool centreObject;
    bool scaleObject;
    bool mapUVWToRGB;
    bool phongShadingOn;

    // constructor
    RenderParameters()
        :
        xTranslate(0.0), 
        yTranslate(0.0),
        zoomScale(1.0),
        emissiveLight(0.0),
        ambientLight(0.2),
        diffuseLight(0.6),
        specularLight(0.3),
        specularExponent(4.0),
        useLighting(false),
        texturedRendering(false),
        textureModulation(false),
        depthTestOn(false),
        showAxes(false),
        showObject(false),
        centreObject(false),
        scaleObject(false),
        mapUVWToRGB(false),
        phongShadingOn(false)
        { // constructor
        
        // start the lighting at the viewer's direction
        lightPosition[0] = 0.0;
        lightPosition[1] = 0.0;
        lightPosition[2] = 1.0;
        lightPosition[3] = 0.0;

        // because we are paranoid, we will initialise the matrices to the identity
        rotationMatrix.SetIdentity();
        lightMatrix.SetIdentity();
        } // constructor

    // accessor for scaledXTranslate

    }; // class RenderParameters

// now define some macros for bounds on parameters
#define TRANSLATE_MIN -1.0
#define TRANSLATE_MAX 1.0

#define ZOOM_SCALE_LOG_MIN -2.0
#define ZOOM_SCALE_LOG_MAX 2.0
#define ZOOM_SCALE_MIN 0.01
#define ZOOM_SCALE_MAX 100.0

#define LIGHTING_MIN 0.0
#define LIGHTING_MAX 1.0

#define SPECULAR_EXPONENT_LOG_MIN -2.0
#define SPECULAR_EXPONENT_LOG_MAX 2.0
#define SPECULAR_EXPONENT_MIN 0.01
#define SPECULAR_EXPONENT_MAX 100.0

// this is to scale to/from integer values
#define PARAMETER_SCALING 100


// end of include guard
#endif
