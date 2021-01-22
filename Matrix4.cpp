//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  Matrix4.h
//  ------------------------
//  
//  A minimal class for a homogeneous 4x4 matrix
//  
//  Note: the emphasis here is on clarity, not efficiency
//  A number of the routines could be implemented more
//  efficiently but aren't
//  
///////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include "Matrix4.h"
#include "Quaternion.h"
#include <memory.h>
#include <limits>
#include <math.h>
#include <algorithm>
// constructor - default to the zero matrix
Matrix4::Matrix4()
    { // default constructor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
    } // default constructor

Matrix4::Matrix4(const Matrix4 &other)
    { // copy constructor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = other.coordinates[row][col];
    } // copy constructor

// equality operator
bool Matrix4::operator ==(const Matrix4 &other) const
    { // operator ==()
    // loop through, testing for mismatches
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (coordinates[row][col] != other.coordinates[row][col])
                return false;
    // if no mismatches, matrices are the same
    return true;
    } // operator ==()


// indexing - retrieves the beginning of a line
// array indexing will then retrieve an element
float * Matrix4::operator [](const int rowIndex)
    { // operator *()
    // return the corresponding row
    return coordinates[rowIndex];
    } // operator *()

// similar routine for const pointers
const float * Matrix4::operator [](const int rowIndex) const
    { // operator *()
    // return the corresponding row
    return coordinates[rowIndex];
    } // operator *()

// scalar operations
// multiplication operator (no division operator)
Matrix4 Matrix4::operator *(float factor) const
    { // operator *()
    // start with a zero matrix
    Matrix4 returnMatrix;
    // multiply by the factor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnMatrix.coordinates[row][col] = coordinates[row][col] * factor;
    // and return it
    return returnMatrix;
    } // operator *()

// vector operations on homogeneous coordinates
// multiplication is the only operator we use
Homogeneous4 Matrix4::operator *(const Homogeneous4 &vector) const
    { // operator *()
    // get a zero-initialised vector
    Homogeneous4 productVector;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            productVector[row] += coordinates[row][col] * vector[col];
    
    // return the result
    return productVector;
    } // operator *()

// and on Cartesian coordinates
Cartesian3 Matrix4::operator *(const Cartesian3 &vector) const
    { // cartesian multiplication
    // convert to Homogeneous coords and multiply
    Homogeneous4 productVector = (*this) * Homogeneous4(vector);

    // then divide back through
    return productVector.Point();
    } // cartesian multiplication

// matrix operations
// addition operator
Matrix4 Matrix4::operator +(const Matrix4 &other) const
    { // operator +()
    // start with a zero matrix
    Matrix4 sumMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            sumMatrix.coordinates[row][col] = coordinates[row][col] + other.coordinates[row][col];

    // return the result
    return sumMatrix;
    } // operator +()

// subtraction operator
Matrix4 Matrix4::operator -(const Matrix4 &other) const
    { // operator -()
    // start with a zero matrix
    Matrix4 differenceMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            differenceMatrix.coordinates[row][col] = coordinates[row][col] + other.coordinates[row][col];

    // return the result
    return differenceMatrix;
    } // operator -()

// multiplication operator
Matrix4 Matrix4::operator *(const Matrix4 &other) const
{   // operator *()
    // start with a zero matrix
    Matrix4 productMatrix;

    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            for (int entry = 0; entry < 4; entry++)
                productMatrix.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];

    // return the result
    return productMatrix;
} // operator *()

// matrix transpose
Matrix4 Matrix4::transpose() const
    { // transpose()
    // start with a zero matrix
    Matrix4 transposeMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            transposeMatrix.coordinates[row][col] = coordinates[col][row];

    // return the result
    return transposeMatrix;
    } // transpose()


Matrix4 Matrix4::inverse() noexcept
{
    auto m = reinterpret_cast<float*>(&coordinates[0][0]);
    const float a0 = m[0] * m[5] - m[1] * m[4];
    const float a1 = m[0] * m[6] - m[2] * m[4];
    const float a2 = m[0] * m[7] - m[3] * m[4];
    const float a3 = m[1] * m[6] - m[2] * m[5];
    const float a4 = m[1] * m[7] - m[3] * m[5];
    const float a5 = m[2] * m[7] - m[3] * m[6];
    const float b0 = m[8] * m[13] - m[9] * m[12];
    const float b1 = m[8] * m[14] - m[10] * m[12];
    const float b2 = m[8] * m[15] - m[11] * m[12];
    const float b3 = m[9] * m[14] - m[10] * m[13];
    const float b4 = m[9] * m[15] - m[11] * m[13];
    const float b5 = m[10] * m[15] - m[11] * m[14];


    const float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;


    if (std::fabs(det) <= std::numeric_limits<float>::min()) return *this;

    Matrix4 inverse;
    auto m2 = reinterpret_cast<float*>(&inverse.coordinates[0][0]);


    m2[0] = m[5] * b5 - m[6] * b4 + m[7] * b3;
    m2[1] = -m[1] * b5 + m[2] * b4 - m[3] * b3;
    m2[2] = m[13] * a5 - m[14] * a4 + m[15] * a3;
    m2[3] = -m[9] * a5 + m[10] * a4 - m[11] * a3;

    m2[4] = -m[4] * b5 + m[6] * b2 - m[7] * b1;
    m2[5] = m[0] * b5 - m[2] * b2 + m[3] * b1;
    m2[6] = -m[12] * a5 + m[14] * a2 - m[15] * a1;
    m2[7] = m[8] * a5 - m[10] * a2 + m[11] * a1;

    m2[8] = m[4] * b4 - m[5] * b2 + m[7] * b0;
    m2[9] = -m[0] * b4 + m[1] * b2 - m[3] * b0;
    m2[10] = m[12] * a4 - m[13] * a2 + m[15] * a0;
    m2[11] = -m[8] * a4 + m[9] * a2 - m[11] * a0;

    m2[12] = -m[4] * b3 + m[5] * b1 - m[6] * b0;
    m2[13] = m[0] * b3 - m[1] * b1 + m[2] * b0;
    m2[14] = -m[12] * a3 + m[13] * a1 - m[14] * a0;
    m2[15] = m[8] * a3 - m[9] * a1 + m[10] * a0;
    inverse = inverse * (1.f / det);
    return inverse;
}

// returns a column-major array of 16 values
// for use with OpenGL
columnMajorMatrix Matrix4::columnMajor() const
    { // columnMajor()
    // start off with an unitialised array
    columnMajorMatrix returnArray;
    // loop to fill in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnArray.coordinates[4 * col + row] = coordinates[row][col];
    // now return the array
    return returnArray;
    } // columnMajor()

// factory methods that create specific matrices
// the zero matrix
void Matrix4::SetZero()
    { // SetZero()
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
    } // SetZero()

// the identity matrix
void Matrix4::SetIdentity()
{ // SetIdentity()
    // start with a zero matrix
    SetZero();
    // fill in the diagonal with 1's
    for (int row = 0; row < 4; row++)
            coordinates[row][row] = 1.0;
} // SetIdentity()

void Matrix4::SetTranslation(const Cartesian3 &vector)
{ // SetTranslation()
// start with an identity matrix
    SetIdentity();

    // put the translation in the w column
    for (int entry = 0; entry < 3; entry++)
        coordinates[entry][3] = vector[entry];
} // SetTranslation()

void Matrix4::SetRotation(const Cartesian3 &axis, float theta)
{ // SetRotation()
// This is derived from quaternions, so we invoke them
    Quaternion rotationQuaternion(axis.unit(), theta * 0.5);
    (*this) = rotationQuaternion.GetMatrix();
} // SetRotation()

void Matrix4::SetScale(float xScale, float yScale, float zScale)
{ // SetScale()
// start off with a zero matrix
    SetZero();

    // set the scale factors
    coordinates[0][0] = xScale;
    coordinates[1][1] = yScale;
    coordinates[2][2] = zScale;
    coordinates[3][3] = 1.0;

} // SetScale()

// scalar operations
// additional scalar multiplication operator
Matrix4 operator *(float factor, const Matrix4 &matrix)
    { // operator *()
    // since this is commutative, call the other version
    return matrix * factor;
    } // operator *()

Matrix4& Matrix4::operator*=(const Matrix4 &other) noexcept
{   
    multiply(other,*this);
    return *this;
}

void Matrix4::multiply(const Matrix4 & other) noexcept
{
    multiply(other,*this);
}

void Matrix4::multiply(const Matrix4 & other,Matrix4 & dst) noexcept
{

    Matrix4 productMatrix;

    for (int32_t row = 0; row < 4; row++)
        for (int32_t col = 0; col < 4; col++)
            for (int32_t entry = 0; entry < 4; entry++)
                productMatrix.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];

    memcpy(dst.coordinates,productMatrix.coordinates,sizeof(float) * 16);

    
}
     

// stream input
std::istream & operator >> (std::istream &inStream, Matrix4 &matrix)
    { // operator >>()
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            inStream >> matrix.coordinates[row][col];   
    // and return the stream
    return inStream;
    } // operator >>()

// stream output
std::ostream & operator << (std::ostream &outStream, const Matrix4 &matrix)
    { // operator <<()
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            outStream << std::setprecision(4) << std::setw(8) << matrix.coordinates[row][col] << ((col == 3) ? "\n" : " "); 
    // and return the stream
    return outStream;
    } // operator <<()
