//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  RGBAImage.cpp
//  ------------------------
//  
//  A minimal class for an image in single-byte RGBA format
//  Optimized for simplicity, not speed or memory
//  With read/write for ASCII RGBA files
//  
///////////////////////////////////////////////////

#define MAX_IMAGE_DIMENSION 4096
#define MAX_LINE_LENGTH 1024

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "string.h"

#include "RGBAImage.h"

// constructor
RGBAImage::RGBAImage()
    :
    block(NULL),
    width(0),
    height(0)
    { // RGBAImage constructor
    } // RGBAImage constructor

// copy constructor
RGBAImage::RGBAImage(const RGBAImage &other)
    { // copy constructor
    // resize to match the other image
    Resize(other.width, other.height);

    // now copy all of the pixels
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)
            (*this)[row][col] = other[row][col];

    } // copy constructor

//  destructor
RGBAImage::~RGBAImage()
    { // RGBAImage destructor
    // release the memory
    free(block);
    } // RGBAImage destructor

// resizes the image, destroying any contents
bool RGBAImage::Resize(long Width, long Height) 
    { // Resize()
    // check validity of dimensions
    if ((Width < 0) || (Width > MAX_IMAGE_DIMENSION) || (Height < 0) || (Height > MAX_IMAGE_DIMENSION))
        { // failure 
        std::cout << "Cannot handle image of size " << Width << " x " << Height << std::endl;
        return false;
        } // failure

    // if our old block is non-null
    if (block != NULL)
        // release the old pointer
        free(block);

    // use calloc() to allocate & zero memory
    block = (RGBAValue *) calloc(Height * Width, sizeof (RGBAValue));
    if (block == NULL)
        return false;

    // now that it's reallocated and copied, reset the parameters
    height = Height;
    width = Width;

    // done
    return true;
    } // Resize()

// indexing - retrieves the beginning of a line
// array indexing will then retrieve an element
RGBAValue * RGBAImage::operator [](const int rowIndex)
    { // row [] index operator
    // use pointer arithmetic to compute the row beginning
    return block+(rowIndex*width);
    } // [] row index operator
    
// similar routine for const pointers
const RGBAValue * RGBAImage::operator [](const int rowIndex) const
    { // row [] index operator
    // use pointer arithmetic to compute the row beginning
    return block+(rowIndex*width);
    } // [] row index operator

// file read routine
bool RGBAImage::ReadPPM(std::istream &inStream)
    { // ReadPPMFile()
    // buffer for reading in line
    char lineBuffer[MAX_LINE_LENGTH];

    // read in the first line
    inStream.getline(lineBuffer, MAX_LINE_LENGTH);
    
    // check for magic number (file code) in first two characters
    if (strcmp(lineBuffer,"P3") != 0)
        { // failed read
        std::cerr << "RGBA stream did not start with PPM code (P3)" << std::endl;
        return false;
        } // failed read

    while (inStream.good() && inStream.peek() == '#')
        inStream.getline(lineBuffer, MAX_LINE_LENGTH);

    // read in new width & height
    long newWidth, newHeight;
    inStream >> newWidth >> newHeight;

    // check the byte max value
    int maxValue;
    inStream >> maxValue;
    
    if (maxValue != 255)
        { // failure
        std::cerr << "RGBA stream did not specify 255 as the maximum colour value." << std::endl;
        return false;
        } // failure

    // check for stupid sizes
    if ((newWidth < 1)  || (newWidth > MAX_IMAGE_DIMENSION) ||
        (newHeight < 1) || (newHeight > MAX_IMAGE_DIMENSION))
        { // bad sizes
        std::cerr << "RGBA image dimensions " << newWidth << " x " << newHeight << " were outside range of 1 - " << MAX_IMAGE_DIMENSION << std::endl;
        return false;
        } // bad sizes

    // resize the image
    Resize(newWidth, newHeight);

    // loop through pixels, reading them:
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)       
            inStream >> (*this)[row][col];

    // done
    return true;
    } // ReadPPMFile()

// file write routine
void RGBAImage::WritePPM(std::ostream &outStream)
    { // WritePPMFile()
    // print out header information
    outStream << "P3" << std::endl;
    outStream << "# PPM File" << std::endl;
    outStream << width << " " << height << std::endl;
    outStream << 255 << std::endl;
        
    // loop through pixels, reading them:
    for (int row = 0; row < height; row++)
        { // row
        for (int col = 0; col < width; col++)
            { // col
            // put a space before each one except the first
            if (col != 0)
                outStream << " ";
            outStream << (*this)[row][col];
            } // col
        outStream << std::endl;
        } // row
    } // WritePPMFile()
