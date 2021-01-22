//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  FakeGL.cpp
//  ------------------------
//  
//  A unit for implementing OpenGL workalike calls
//  
///////////////////////////////////////////////////

#include "FakeGL.h"
#include <math.h>
#include <cassert>
#include <memory.h>
#include <limits>
#include "MathUtils.h"
#include "Shader.h"

//-------------------------------------------------//
//                                                 //
// CONSTRUCTOR / DESTRUCTOR                        //
//                                                 //
//-------------------------------------------------//

// constructor
FakeGL::FakeGL()
{ // constructor
    stateMechine.matrixMode = FAKEGL_MODELVIEW;
    stateMechine.envMode = FAKEGL_REPLACE;
    stateMechine.modelViewMatrixStack.push({});
    stateMechine.projectionMatrixStack.push({});
    gouraudShader = std::shared_ptr<GouraudShadingShader>(new GouraudShadingShader());
    phongShader = std::shared_ptr<PhongShadingShader>(new PhongShadingShader());
    stateMechine.currentShader = gouraudShader;
} // constructor

// destructor
FakeGL::~FakeGL()
{ // destructor
} // destructor

//-------------------------------------------------//
//                                                 //
// GEOMETRIC PRIMITIVE ROUTINES                    //
//                                                 //
//-------------------------------------------------//

// starts a sequence of geometric primitives
void FakeGL::Begin(unsigned int primitiveType)
{ // Begin()
    vertexQueue.clear();
    rasterQueue.clear();
    fragmentQueue.clear();
    stateMechine.drawType = primitiveType;
} // Begin()

// ends a sequence of geometric primitives
void FakeGL::End()
{ // End()

    if(stateMechine.enables[FAKEGL_TEXTURE_2D])
    {
        stateMechine.currentShader->bindTexture(&stateMechine.texture);
    }
    else
    {
        stateMechine.currentShader->bindTexture(nullptr);
    }

    TransformVertex();
    if(RasterisePrimitive()){
        switch ( stateMechine.drawType)
        {
        case FAKEGL_POINTS:{
            while(rasterQueue.size() > 0){
                auto & a = rasterQueue.front();
                RasterisePoint(a);
                rasterQueue.pop_front();
            }
        }
            break;
        case FAKEGL_LINES:
            while(rasterQueue.size() > 1){
                auto a = rasterQueue.front();
                rasterQueue.pop_front();
                auto b = rasterQueue.front();
                rasterQueue.pop_front();
                RasteriseLineSegment(a,b);
            }
        break;

        case FAKEGL_TRIANGLES:
            while(rasterQueue.size() > 2){
                auto a = rasterQueue.front();
                rasterQueue.pop_front();
                auto b = rasterQueue.front();
                rasterQueue.pop_front();
                auto c = rasterQueue.front();
                rasterQueue.pop_front();
                RasteriseTriangle(a,b,c);
            }
        break;
        default:
            break;
        }
        ProcessFragment();
    }
    stateMechine.drawType = -1;
} // End()

// sets the size of a point for drawing
void FakeGL::PointSize(float size)
{ // PointSize()
    stateMechine.pointSize = size;
} // PointSize()

// sets the width of a line for drawing purposes
void FakeGL::LineWidth(float width)
{ // LineWidth()
    stateMechine.lineWidth = width;
} // LineWidth()

//-------------------------------------------------//
//                                                 //
// MATRIX MANIPULATION ROUTINES                    //
//                                                 //
//-------------------------------------------------//

// set the matrix mode (i.e. which one we change)   
void FakeGL::MatrixMode(unsigned int whichMatrix)
{ // MatrixMode()
    stateMechine.matrixMode = whichMatrix;
} // MatrixMode()

// pushes a matrix on the stack
void FakeGL::PushMatrix()
{ // PushMatrix()
    Matrix4 identity;
    identity.SetIdentity();
    switch (stateMechine.matrixMode) {
    case  FAKEGL_MODELVIEW:
        stateMechine.modelViewMatrixStack.push(identity);
    break;
    case FAKEGL_PROJECTION:
        stateMechine.projectionMatrixStack.push(identity);
    break;
    }
} // PushMatrix()

// pops a matrix off the stack
void FakeGL::PopMatrix()
{ // PopMatrix()
    switch (stateMechine.matrixMode) {
    case  FAKEGL_MODELVIEW:
        stateMechine.modelViewMatrixStack.pop();
    break;
    case FAKEGL_PROJECTION:
        stateMechine.projectionMatrixStack.pop();
    break;
    }
} // PopMatrix()

// load the identity matrix
void FakeGL::LoadIdentity()
{ // LoadIdentity()
    Matrix4 * matrix = stateMechine.getCurrentSelectedMatrix();
    matrix->SetIdentity();
} // LoadIdentity()

// multiply by a known matrix in column-major format
void FakeGL::MultMatrixf(const float *columnMajorCoordinates)
{ // MultMatrixf()
        //get the top matrix 
    auto &matrix = *stateMechine.getCurrentSelectedMatrix();
    Matrix4 newMatix;
    memcpy(&newMatix.coordinates[0],columnMajorCoordinates,sizeof(float) * 16);
    newMatix = newMatix.transpose();
    matrix *= newMatix;
} // MultMatrixf()

// sets up a perspective projection matrix
void FakeGL::Frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{ // Frustum()

    Matrix4 mat;
    mat.SetZero();

    mat[0][0] = 2.f * zNear /(right - left);
    mat[1][1] = 2.f * zNear /(top - bottom);

    mat[2][0] = (left + right) / (right - left);
    mat[2][1] = (top + bottom) / (top - bottom);
    mat[2][2] = (zFar + zNear) / (zFar - zNear);
    mat[2][3] = -1;

    mat[3][2] = -(2 * zNear * zFar)/(zFar - zNear);


    auto & current = *stateMechine.getCurrentSelectedMatrix();
    current*= mat;

} // Frustum()

// sets an orthographic projection matrix
void FakeGL::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{ // Ortho()
    Matrix4 mat;
    mat.SetIdentity();
    mat[0][0] = 2.f/(right - left);
    mat[1][1] = 2.f/(top - bottom);
    mat[2][2] = -2.f/(zFar - zNear);

    mat[3][0] = (left + right) / (left - right);
    mat[3][1] = (bottom + top) / (bottom - top);
    mat[3][2] = (zNear + zFar)/(zNear - zFar);


    auto & current = *stateMechine.getCurrentSelectedMatrix();
    current*= mat;

    /*mat.coordinates = {
        {2.f/(right - left),0.f,             0.f,               0.f},
        {0.f,               2.f/(top-bottom),0.f,               0.f},
        {0.f,               0.f,             2.f/(zFar - zNear),0.f},
        {(left + right) / (left - right), (bottom + top) / (bottom - top) , (zNear + zFar)/(zNear - zFar) ,0.f}
    };*/

} // Ortho()

// rotate the matrix
void FakeGL::Rotatef(float angle, float axisX, float axisY, float axisZ)
{ // Rotatef()
    //degree to arc
    auto theta = angle * M_PI / 180.f;
    Matrix4 mat;
    mat.SetRotation({axisX,axisY,axisZ},theta);
    auto & current = *stateMechine.getCurrentSelectedMatrix();
    current*= mat;
} // Rotatef()

// scale the matrix
void FakeGL::Scalef(float xScale, float yScale, float zScale)
{ // Scalef()
//x,0,0,0,
//0,y,0,0,
//0,0,z,0,
//0,0,0,1
    Matrix4 scaleMat;
    scaleMat.SetIdentity();
    scaleMat.SetScale(xScale,yScale,zScale);
    auto & current = *stateMechine.getCurrentSelectedMatrix();
    current*= scaleMat;

} // Scalef()

// translate the matrix
void FakeGL::Translatef(float xTranslate, float yTranslate, float zTranslate)
{ // Translatef()
//1,0,0,0,
//0,1,0,0,
//0,0,1,0,
//x,y,z,1
    Matrix4 translateMat;
    translateMat.SetIdentity();
    translateMat.SetTranslation({xTranslate,yTranslate,zTranslate});
    auto & current = *stateMechine.getCurrentSelectedMatrix();
    current*= translateMat;

} // Translatef()

// sets the viewport
void FakeGL::Viewport(int x, int y, int width, int height)
{ // Viewport()
        
    stateMechine.viewport.x = x;
    stateMechine.viewport.y = y;
    stateMechine.viewport.width = width;
    stateMechine.viewport.height = height;


    stateMechine.viewportMatrix.SetIdentity();
    stateMechine.viewportMatrix[0][0] = width / 2;
    stateMechine.viewportMatrix[1][1] = -height / 2;
    stateMechine.viewportMatrix[3][0] = x + width / 2;
    stateMechine.viewportMatrix[3][1] = y + height / 2;

    //if viewport was described as matrix, the matrix is 
    /** 
     * w/2      0      0      0 
     * 0      -h/2     0      0
     * 0        0      1      0 
     * x+w/2   y+h/2   0      1
     */

} // Viewport()

//-------------------------------------------------//
//                                                 //
// VERTEX ATTRIBUTE ROUTINES                       //
//                                                 //
//-------------------------------------------------//

// sets colour with floating point
void FakeGL::Color3f(float red, float green, float blue)
{ // Color3f()
    stateMechine.currentSurface.color = {red * 255,green * 255,blue * 255,255};
} // Color3f()

// sets material properties
void FakeGL::Materialf(unsigned int parameterName, const float parameterValue)
{ // Materialf()
    if(parameterName & FAKEGL_SHININESS)
    {
        stateMechine.material.setShininess(parameterValue);
    }
} // Materialf()

void FakeGL::Materialfv(unsigned int parameterName, const float *parameterValues)
{ // Materialfv()

    if(parameterName & FAKEGL_AMBIENT)
    {
        stateMechine.material.setAmbient({parameterValues});
    }
    if(parameterName & FAKEGL_DIFFUSE)
    {
        stateMechine.material.setDiffuse({parameterValues});
    }
    if(parameterName & FAKEGL_SPECULAR)
    {
        stateMechine.material.setSpecular({parameterValues});
    }
    if(parameterName & FAKEGL_EMISSION)
    {
        stateMechine.material.setEmission({parameterValues});
    }
} // Materialfv()

// sets the normal vector
void FakeGL::Normal3f(float x, float y, float z)
{ // Normal3f()
    stateMechine.currentSurface.normal = {x,y,z};
} // Normal3f()

// sets the texture coordinates
void FakeGL::TexCoord2f(float u, float v)
{ // TexCoord2f()
    stateMechine.currentSurface.textCoord = {u,v,0};
} // TexCoord2f()

// sets the vertex & launches it down the pipeline
void FakeGL::Vertex3f(float x, float y, float z)
{ // Vertex3f()
    vertexWithAttributes v;
    v.position = {x,y,z,1.f};
    v.colour = stateMechine.currentSurface.color;
    v.normal = stateMechine.currentSurface.normal;
    v.texCoord = stateMechine.currentSurface.textCoord;
    v.divZ = 1.f;
    vertexQueue.emplace_back(v);
} // Vertex3f()

//-------------------------------------------------//
//                                                 //
// STATE VARIABLE ROUTINES                         //
//                                                 //
//-------------------------------------------------//

// disables a specific flag in the library
void FakeGL::Disable(unsigned int property)
{ // Disable()
   stateMechine.enables[property] = false;
   if(property == FAKEGL_LIGHTING){
       stateMechine.currentShader->setLight(nullptr);
   }

   if(property == FAKEGL_PHONG_SHADING)
   {
       stateMechine.currentShader = gouraudShader;
   }
} // Disable()

// enables a specific flag in the library
void FakeGL::Enable(unsigned int property)
{ // Enable()

    //use a array indicate all flags;
    stateMechine.enables[property] = true;
    if(property == FAKEGL_DEPTH_TEST){
        if(depthBuffer.width != frameBuffer.width && depthBuffer.height != frameBuffer.height)
            depthBuffer.Resize(frameBuffer.width,frameBuffer.height);
    }

    if(property == FAKEGL_LIGHTING){
        stateMechine.currentShader = gouraudShader;
        if(stateMechine.enables[FAKEGL_PHONG_SHADING]){
            stateMechine.currentShader = phongShader;
        }
        stateMechine.currentShader->setLight(&stateMechine.light);
    }


} // Enable()

//-------------------------------------------------//
//                                                 //
// LIGHTING STATE ROUTINES                         //
//                                                 //
//-------------------------------------------------//

// sets properties for the one and only light
void FakeGL::Light(int parameterName, const float *parameterValues)
{ // Light()

    if(parameterName & FAKEGL_AMBIENT)
    {
        stateMechine.light.setAmbient(parameterValues);
    }
    if(parameterName & FAKEGL_DIFFUSE)
    {
        stateMechine.light.setDiffuse(parameterValues);
    }
    if(parameterName & FAKEGL_SPECULAR)
    {
        stateMechine.light.setSpecular(parameterValues);
    }
    if(parameterName & FAKEGL_POSITION)
    {
        auto & curr = *stateMechine.getCurrentSelectedMatrix();
        auto mat = curr * Cartesian3{parameterValues[0],parameterValues[1],parameterValues[2]};
        stateMechine.light.setPosition(mat);
    }

} // Light()

//-------------------------------------------------//
//                                                 //
// TEXTURE PROCESSING ROUTINES                     //
//                                                 //
// Note that we only allow one texture             //
// so glGenTexture & glBindTexture aren't needed   //
//                                                 //
//-------------------------------------------------//

// sets whether textures replace or modulate
void FakeGL::TexEnvMode(unsigned int textureMode)
{ // TexEnvMode()
    stateMechine.envMode = textureMode;
} // TexEnvMode()

// sets the texture image that corresponds to a given ID
void FakeGL::TexImage2D(const RGBAImage &textureImage)
{ // TexImage2D()
    stateMechine.texture = textureImage;
} // TexImage2D()

//-------------------------------------------------//
//                                                 //
// FRAME BUFFER ROUTINES                           //
//                                                 //
//-------------------------------------------------//

void FakeGL::clearFramebuffer()
{
    for(auto row = 0;row < frameBuffer.height;++ row)
    {
        for(auto col = 0;col < frameBuffer.width;++ col)
        {
            frameBuffer.block[row * frameBuffer.width + col] = stateMechine.clearColor;
        }
    }
}

void FakeGL::clearDepth()
{
    for(auto row = 0;row < depthBuffer.height;++ row)
    {
        for(auto col = 0;col < depthBuffer.width;++ col)
        {
            depthBuffer[row][col].alpha = 255;
        }
    }
}


void FakeGL::normalizeToWindow(screenVertexWithAttributes & v) const
{
    auto halfWidth = stateMechine.viewport.width >> 1;
    auto halfHeight = stateMechine.viewport.height >> 1;
    auto centerX = stateMechine.viewport.x + halfWidth;
    auto centerY = stateMechine.viewport.y + halfHeight;

    v.position.x =  centerX + (float)halfWidth * v.position.x;
    //opengl Y axis is differet with framebuffer
    //the framebuffer is up to down
    //opengl framebuffer is down to up
    v.position.y =  stateMechine.viewport.height - centerY + (float)halfHeight * v.position.y;

    //constraint the z to (Near -> Far)
    //v.position.z = v.position.z;
    v.position.z = ((stateMechine.zFar - stateMechine.zNear) * v.position.z+ (stateMechine.zNear + stateMechine.zFar)) * 0.5f;
}



// clears the frame buffer
void FakeGL::Clear(unsigned int mask)
{ // Clear()

    if(mask & FAKEGL_COLOR_BUFFER_BIT){
        clearFramebuffer();
    }

    if(mask & FAKEGL_DEPTH_BUFFER_BIT){
        clearDepth();
    }

} // Clear()

// sets the clear colour for the frame buffer
void FakeGL::ClearColor(float red, float green, float blue, float alpha)
{ // ClearColor()
    stateMechine.clearColor = {red*255,green*255,blue*255,alpha*255};
} // ClearColor()

//-------------------------------------------------//
//                                                 //
// MAJOR PROCESSING ROUTINES                       //
//                                                 //
//-------------------------------------------------//

// transform one vertex & shift to the raster queue
void FakeGL::TransformVertex()
{ // TransformVertex()
    stateMechine.currentShader->setModelViewMatrix(stateMechine.modelViewMatrixStack.top());
    stateMechine.currentShader->setProjectMatrix(stateMechine.projectionMatrixStack.top());

    //Transform in vertex shader;
    while(!vertexQueue.empty()){
        rasterQueue.emplace_back(stateMechine.currentShader->vertexShader(vertexQueue.front(),*this));
        vertexQueue.pop_front();
    }

} // TransformVertex()

// rasterise a single primitive if there are enough vertices on the queue
bool FakeGL::RasterisePrimitive()
{ // RasterisePrimitive()
    return stateMechine.drawType != -1;
} // RasterisePrimitive()

// rasterises a single point
void FakeGL::RasterisePoint(screenVertexWithAttributes &vertex0)
{ // RasterisePoint()

    //convert this vertex into screen coord system.
    normalizeToWindow(vertex0);

    fragmentWithAttributes tmp;

    tmp.colour = vertex0.colour;
    tmp.normal = vertex0.normal;
    tmp.texCoord = vertex0.texCoord;
    tmp.divZ = vertex0.divZ;
    tmp.modelViewCoord  = vertex0.modelViewCoord;
    int32_t startX = vertex0.position.x - stateMechine.pointSize / 2;
    int32_t startY = vertex0.position.y - stateMechine.pointSize / 2;

    if(stateMechine.pointSize > 0)
    {
       for(auto i = 0;i<stateMechine.pointSize;i++){
          for(auto j = 0;j<stateMechine.pointSize;j++){
              if(isDepthPassed(startX,startY,vertex0.position.z * 255.f)){
                  if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
                      depthBuffer[startY][startX].alpha = vertex0.position.z * 255.f;
                  }
                  tmp.row = startY;
                  tmp.col = startX;
                  fragmentQueue.emplace_back(tmp);//a fragment ......
              }
              startX++;
          }
          startY++;
       }
    }
    else
    {
        if(isDepthPassed(startX,startY,vertex0.position.z* 255.f)){
            if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
                depthBuffer[startY][startX].alpha = vertex0.position.z * 255.f;
            }
            tmp.row = startY;
            tmp.col = startX;
            fragmentQueue.emplace_back(tmp);//a fragment ......
        }
    }

} // RasterisePoint()


bool FakeGL::isDepthPassed(float x,float y, float z)
{
    if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
        if(z > depthBuffer[(int32_t)y][(int32_t)x].alpha)
        {
           return false;
        }
    }
    return true;//default as true
}

// rasterises a single line segment
void FakeGL::RasteriseLineSegment(screenVertexWithAttributes &vertex0, screenVertexWithAttributes &vertex1)
{ // RasteriseLineSegment()

//convert this vertex into screen coord system.
    normalizeToWindow(vertex0);
    normalizeToWindow(vertex1);


//RasteriseLine with bresenham
    auto dx = vertex1.position.x - vertex0.position.x;
    auto dy = vertex1.position.y - vertex0.position.y;
    auto stepX = 1;
    auto stepY = 1;
    if(dx < 0){
        stepX = -1;
        dx = -dx;
    }

    if(dy < 0)
    {
        stepY = -1;
        dy = -dy;
    }

    auto d2x = 2*dx;
    auto d2y = 2*dy;
    auto d2yd2x = d2y - d2x;
    auto sx = vertex0.position.x;
    auto sy = vertex0.position.y;
    fragmentWithAttributes tmp;
    //means slope < 1
    if(dy <= dx)
    {
        auto flag = d2y - dx;

        for(auto i = 0;i <= dx;++i)
        {
            auto lerped = MathUtils::lerp(vertex0,vertex1,static_cast<double>(i)/dx);
            tmp.row = sy;
            tmp.col = sx;
            tmp.colour = lerped.colour;
            tmp.normal = lerped.normal;
            tmp.texCoord = lerped.texCoord;
            tmp.divZ = lerped.divZ;
            tmp.modelViewCoord  = lerped.modelViewCoord;
            for(auto j = 0;j<stateMechine.lineWidth;j++){
                tmp.col = sx+j;
                tmp.row = sy+j;
                if(isDepthPassed(tmp.col,tmp.row,lerped.position.z * 255.f)){
                    if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
                        depthBuffer[tmp.row][tmp.col].alpha = lerped.position.z * 255.f;
                    }
                    fragmentQueue.emplace_back(tmp);//a fragment ......
                }
            }

            sx += stepX;
            if(flag <= 0)
                flag += d2y;
            else
            {
                sy += stepY;
                flag += d2yd2x;
            }
      }
    }
    else
    {
        int flag = d2x - dy;
        for(auto i = 0;i <= dy;++ i)
        {

            auto lerped = MathUtils::lerp(vertex0,vertex1,static_cast<double>(i)/dy);
            tmp.row = sy;
            tmp.col = sx;
            tmp.colour = lerped.colour;
            tmp.normal = lerped.normal;
            tmp.texCoord = lerped.texCoord;
            tmp.divZ = lerped.divZ;
            tmp.modelViewCoord  = lerped.modelViewCoord;

            for(auto j = 0;j<stateMechine.lineWidth;j++){
                tmp.col = sx+j;
                tmp.row = sy+j;
                if(isDepthPassed(tmp.col,tmp.row,lerped.position.z * 255.f)){
                    if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
                        depthBuffer[tmp.row][tmp.col].alpha = lerped.position.z * 255.f;
                    }
                    fragmentQueue.emplace_back(tmp);//a fragment ......
                }
            }

            sy += stepY;
            if(flag <= 0)
                flag += d2x;
            else
            {
                sx += stepX;
                flag -= d2yd2x;
            }
        }
    }
} // RasteriseLineSegment()

// rasterises a single triangle
void FakeGL::RasteriseTriangle(screenVertexWithAttributes &vertex0, screenVertexWithAttributes &vertex1, screenVertexWithAttributes &vertex2)
    { // RasteriseTriangle()
    // compute a bounding box that starts inverted to frame size
    // clipping will happen in the raster loop proper


//convert this vertex into screen coord system.
    normalizeToWindow(vertex0);
    normalizeToWindow(vertex1);
    normalizeToWindow(vertex2);

    float minX = frameBuffer.width, maxX = 0.0;
    float minY = frameBuffer.height, maxY = 0.0;


    // test against all vertices
    if (vertex0.position.x < minX) minX = vertex0.position.x;
    if (vertex0.position.x > maxX) maxX = vertex0.position.x;
    if (vertex0.position.y < minY) minY = vertex0.position.y;
    if (vertex0.position.y > maxY) maxY = vertex0.position.y;
    
    if (vertex1.position.x < minX) minX = vertex1.position.x;
    if (vertex1.position.x > maxX) maxX = vertex1.position.x;
    if (vertex1.position.y < minY) minY = vertex1.position.y;
    if (vertex1.position.y > maxY) maxY = vertex1.position.y;
    
    if (vertex2.position.x < minX) minX = vertex2.position.x;
    if (vertex2.position.x > maxX) maxX = vertex2.position.x;
    if (vertex2.position.y < minY) minY = vertex2.position.y;
    if (vertex2.position.y > maxY) maxY = vertex2.position.y;



    // now for each side of the triangle, compute the line vectors
    Cartesian3 vector01 = vertex1.position - vertex0.position;
    Cartesian3 vector12 = vertex2.position - vertex1.position;
    Cartesian3 vector20 = vertex0.position - vertex2.position;

    // now compute the line normal vectors
    Cartesian3 normal01(-vector01.y, vector01.x, 0.0);  
    Cartesian3 normal12(-vector12.y, vector12.x, 0.0);  
    Cartesian3 normal20(-vector20.y, vector20.x, 0.0);  

    // we don't need to normalise them, because the square roots will cancel out in the barycentric coordinates
    float lineConstant01 = normal01.dot(vertex0.position);
    float lineConstant12 = normal12.dot(vertex1.position);
    float lineConstant20 = normal20.dot(vertex2.position);

    // and compute the distance of each vertex from the opposing side
    float distance0 = normal12.dot(vertex0.position) - lineConstant12;
    float distance1 = normal20.dot(vertex1.position) - lineConstant20;
    float distance2 = normal01.dot(vertex2.position) - lineConstant01;



    // if any of these are zero, we will have a divide by zero error
    // but notice that if they are zero, the vertices are collinear in projection and the triangle is edge on
    // we can render that as a line, but the better solution is to render nothing.  In a surface, the adjacent
    // triangles will eventually take care of it
    if ((distance0 == 0) || (distance1 == 0) || (distance2 == 0))
        return; 

    // create a fragment for reuse
    fragmentWithAttributes rasterFragment;

    // loop through the pixels in the bounding box
    for (rasterFragment.row = minY; rasterFragment.row <= maxY; rasterFragment.row++)
        { // per row
        // this is here so that clipping works correctly
        if (rasterFragment.row < 0) continue;
        if (rasterFragment.row >= frameBuffer.height) continue;
        for (rasterFragment.col = minX; rasterFragment.col <= maxX; rasterFragment.col++)
            { // per pixel
            // this is also for correct clipping
            if (rasterFragment.col < 0) continue;
            if (rasterFragment.col >= frameBuffer.width) continue;
            
            // the pixel in cartesian format
            Cartesian3 pixel(rasterFragment.col, rasterFragment.row, 0.0);
            
            // right - we have a pixel inside the frame buffer AND the bounding box
            // note we *COULD* compute gamma = 1.0 - alpha - beta instead
            float alpha = (normal12.dot(pixel) - lineConstant12) / distance0;           
            float beta = (normal20.dot(pixel) - lineConstant20) / distance1;            
            float gamma = (normal01.dot(pixel) - lineConstant01) / distance2;           

            // now perform the half-plane test
            if ((alpha < 0.0) || (beta < 0.0) || (gamma < 0.0))
                continue;


            // compute colour

            rasterFragment.colour = alpha * vertex0.colour + beta * vertex1.colour + gamma * vertex2.colour; 
            rasterFragment.texCoord =  (alpha * vertex0.texCoord + beta * vertex1.texCoord + gamma * vertex2.texCoord);
            rasterFragment.modelViewCoord = alpha * vertex0.modelViewCoord + beta * vertex1.modelViewCoord + gamma * vertex2.modelViewCoord;
            rasterFragment.normal =alpha * vertex0.normal + beta * vertex1.normal + gamma * vertex2.normal;

            auto vertex = alpha * vertex0.position + beta * vertex1.position + gamma * vertex2.position;


            if(isDepthPassed(rasterFragment.col,rasterFragment.row,vertex.z * 255.f)){
                if(stateMechine.enables[FAKEGL_DEPTH_TEST]){
                    depthBuffer[rasterFragment.row][rasterFragment.col].alpha = vertex.z * 255.f;
                }
                // now we add it to the queue for fragment processing
                fragmentQueue.push_back(rasterFragment);
            }
        } // per pixel
    } // per row
} // RasteriseTriangle()

// process a single fragment
void FakeGL::ProcessFragment()
{ // ProcessFragment()

    //process every fragment in fragment shader.
    while (!fragmentQueue.empty())
    {   
        auto & top = fragmentQueue.front();
        if(top.row <= frameBuffer.height && top.col <= frameBuffer.width && top.row >= 0 && top.col >= 0)
        {
            if(stateMechine.envMode == FAKEGL_REPLACE)
            {
                frameBuffer[top.row][top.col] = stateMechine.currentShader->fragmentShader(top,*this);
            }
            else
            {
               frameBuffer[top.row][top.col] = stateMechine.currentShader->fragmentShader(top,*this) * top.colour;
            }
        }
        fragmentQueue.pop_front();
    }

} // ProcessFragment()


void FakeGL::Flush(){

}

// standard routine for dumping the entire FakeGL context (except for texture / image)
std::ostream &operator << (std::ostream &outStream, FakeGL &fakeGL)
    { // operator <<
    outStream << "=========================" << std::endl;
    outStream << "Dumping FakeGL Context   " << std::endl;
    outStream << "=========================" << std::endl;


    outStream << "-------------------------" << std::endl;
    outStream << "Vertex Queue:            " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto vertex = fakeGL.vertexQueue.begin(); vertex < fakeGL.vertexQueue.end(); vertex++)
        { // per matrix
        outStream << "Vertex " << vertex - fakeGL.vertexQueue.begin() << std::endl;
        outStream << *vertex;
        } // per matrix


    outStream << "-------------------------" << std::endl;
    outStream << "Raster Queue:            " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto vertex = fakeGL.rasterQueue.begin(); vertex < fakeGL.rasterQueue.end(); vertex++)
        { // per matrix
        outStream << "Vertex " << vertex - fakeGL.rasterQueue.begin() << std::endl;
        outStream << *vertex;
        } // per matrix


    outStream << "-------------------------" << std::endl;
    outStream << "Fragment Queue:          " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto fragment = fakeGL.fragmentQueue.begin(); fragment < fakeGL.fragmentQueue.end(); fragment++)
        { // per matrix
        outStream << "Fragment " << fragment - fakeGL.fragmentQueue.begin() << std::endl;
        outStream << *fragment;
        } // per matrix


    return outStream;
    } // operator <<

// subroutines for other classes
std::ostream &operator << (std::ostream &outStream, vertexWithAttributes &vertex)
    { // operator <<
    std::cout << "Vertex With Attributes" << std::endl;
    std::cout << "Position:   " << vertex.position << std::endl;
    std::cout << "Colour:     " << vertex.colour << std::endl;

	// you

    return outStream;
    } // operator <<

std::ostream &operator << (std::ostream &outStream, screenVertexWithAttributes &vertex) 
    { // operator <<
    std::cout << "Screen Vertex With Attributes" << std::endl;
    std::cout << "Position:   " << vertex.position << std::endl;
    std::cout << "Colour:     " << vertex.colour << std::endl;
  //-------------
    std::cout << "Normal:     " << vertex.normal << std::endl;
    std::cout << "TexCoord:     " << vertex.texCoord << std::endl;
    std::cout << "ModelViewCoord:     " << vertex.modelViewCoord << std::endl;

    return outStream;
    } // operator <<

std::ostream &operator << (std::ostream &outStream, fragmentWithAttributes &fragment)
    { // operator <<
    std::cout << "Fragment With Attributes" << std::endl;
    std::cout << "Row:        " << fragment.row << std::endl;
    std::cout << "Col:        " << fragment.col << std::endl;
    std::cout << "Colour:     " << fragment.colour << std::endl;


    std::cout << "Normal:     " << fragment.normal << std::endl;
    std::cout << "TexCoord:     " << fragment.texCoord << std::endl;
    std::cout << "ModelViewCoord:     " << fragment.modelViewCoord << std::endl;

    return outStream;
    } // operator <<


    
    
