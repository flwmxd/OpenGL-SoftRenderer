//////////////////////////////////////////////////////////////////////
//
//  Univerity of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  -----------------------------
//  FakeGL Render Widget
//  -----------------------------
//  
//  Since the render code is in the geometric object class
//  this widget primarily sets up the transformation matrices and 
//  lighting.
//
//  It implements the UI for an arcball controller, but in an
//  abstractable way that allows it to share an arcball with another
//  widget (the arcball controller) - thus you can manipulate the
//  object either directly in the widget or indirectly through the
//  arcball displayed visually.
//  
//  It also supports translation by visual dragging.  This results in
//  needing three mouse buttons, and not all systems have them, so it
//  will be up to the user to make sure that they can use it on their
//  own machine (especially on OSX)
//
//  Since the controls are (potentially) shared with other widgets, 
//  this widget is only responsible for scaling the x,y of mouse events
//  then passing them to the controller
//  
////////////////////////////////////////////////////////////////////////

#include <math.h>

// include the header file
#include "FakeGLRenderWidget.h"

// constructor
FakeGLRenderWidget::FakeGLRenderWidget
        (   
        // the geometric object to show
        TexturedObject      *newTexturedObject,
        // the render parameters to use
        RenderParameters    *newRenderParameters,
        // parent widget in visual hierarchy
        QWidget             *parent
        )
    // the : indicates variable instantiation rather than arbitrary code
    // it is considered good style to use it where possible
    : 
    // start by calling inherited constructor with parent widget's pointer
    QOpenGLWidget(parent),
    // then store the pointers that were passed in
    texturedObject(newTexturedObject),
    renderParameters(newRenderParameters)
    { // constructor
    // leaves nothing to put into the constructor body
    } // constructor    

// destructor
FakeGLRenderWidget::~FakeGLRenderWidget()
    { // destructor
    // empty (for now)
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    // and OpenGL cleanup is taken care of by Qt
    } // destructor                                                                 

// called when OpenGL context is set up
void FakeGLRenderWidget::initializeGL()
    { // FakeGLRenderWidget::initializeGL()
    // set lighting parameters (may be reset later)
    fakeGL.Enable(FAKEGL_LIGHTING);
    
    // background is yellowish-grey
    fakeGL.ClearColor(0.8, 0.8, 0.6, 1.0);

    // now transfer assets (ie texture) to the library
    texturedObject->TransferAssetsToFakeGL(&fakeGL);
    } // FakeGLRenderWidget::initializeGL()

// called every time the widget is resized
void FakeGLRenderWidget::resizeGL(int w, int h)
    { // FakeGLRenderWidget::resizeGL()
    // reset the viewport
    fakeGL.Viewport(0, 0, w, h);
    
    // resize the render image
    fakeGL.frameBuffer.Resize(w, h);

    // set projection matrix to be fakeGL.Ortho based on zoom & window size
    fakeGL.MatrixMode(FAKEGL_PROJECTION);
    fakeGL.LoadIdentity();
    
    // compute the aspect ratio of the widget
    float aspectRatio = (float) w / (float) h;
    
    // we want to capture a sphere of radius 1.0 without distortion
    // so we set the ortho projection based on whether the window is portrait (> 1.0) or landscape
    // portrait ratio is wider, so make bottom & top -1.0 & 1.0
    if (aspectRatio > 1.0)
        fakeGL.Ortho(-aspectRatio, aspectRatio, -1.0, 1.0, -1.0, 1.0);
    // otherwise, make left & right -1.0 & 1.0
    else
        fakeGL.Ortho(-1.0, 1.0, -1.0/aspectRatio, 1.0/aspectRatio, -1.0, 1.0);

    } // FakeGLRenderWidget::resizeGL()
    
// called every time the widget needs painting
void FakeGLRenderWidget::paintGL()
    { // FakeGLRenderWidget::paintGL()
    // set background colour to white
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // call the paintFakeGL() routine to prepare the image
    paintFakeGL();
    
    // and display the image
    glDrawPixels(fakeGL.frameBuffer.width, fakeGL.frameBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, fakeGL.frameBuffer.block);
    
    } // FakeGLRenderWidget::paintGL()
    
// routine that runs the fake GL library
void FakeGLRenderWidget::paintFakeGL()
{ // FakeGLRenderWidget::paintFakeGL()
// enable depth-buffering

    if (renderParameters->depthTestOn)
        fakeGL.Enable(FAKEGL_DEPTH_TEST);
    else
        fakeGL.Disable(FAKEGL_DEPTH_TEST);

    // clear the buffer
    fakeGL.Clear(FAKEGL_COLOR_BUFFER_BIT | (renderParameters->depthTestOn ? FAKEGL_DEPTH_BUFFER_BIT: 0));

    // set model view matrix based on stored translation, rotation &c.
    fakeGL.MatrixMode(FAKEGL_MODELVIEW);
    fakeGL.LoadIdentity();

    // start with lighting turned off
    fakeGL.Disable(FAKEGL_LIGHTING);

    // if lighting is turned on
    if (renderParameters->useLighting)
        { // use lighting
        // make sure lighting is on
        fakeGL.Enable(FAKEGL_LIGHTING);

        // set light position first, pushing/popping matrix so that it the transformation does
        // not affect the position of the geometric object
        fakeGL.PushMatrix();
        fakeGL.MultMatrixf(renderParameters->lightMatrix.columnMajor().coordinates);
        fakeGL.Light(FAKEGL_POSITION, renderParameters->lightPosition);
        fakeGL.PopMatrix();

        // now set the lighting parameters (assuming all light is white)
        float ambientColour[4];
        float diffuseColour[4];
        float specularColour[4];

        // now copy the parameters
        ambientColour[0]    = ambientColour[1]  = ambientColour[2]  = renderParameters->ambientLight;
        diffuseColour[0]    = diffuseColour[1]  = diffuseColour[2]  = renderParameters->diffuseLight;
        specularColour[0]   = specularColour[1] = specularColour[2] = renderParameters->specularLight;
        ambientColour[3]    = diffuseColour[3]  = specularColour[3] = 1.0; // don't forget alpha

        // and set them in OpenGL
        fakeGL.Light(FAKEGL_AMBIENT,    ambientColour);
        fakeGL.Light(FAKEGL_DIFFUSE,    diffuseColour);
        fakeGL.Light(FAKEGL_SPECULAR,   specularColour);

        // notice that emission and the specular exponent belong to the material
        // not to the light. So, even though we are treating them as global,
        // they belong in the TexturedObject render code

        // test for Phong shading
        if (renderParameters->phongShadingOn)
            fakeGL.Enable(FAKEGL_PHONG_SHADING);
        else
            fakeGL.Disable(FAKEGL_PHONG_SHADING);
        } // use lighting

    // translate by the visual translation
    fakeGL.Translatef(renderParameters->xTranslate, renderParameters->yTranslate, 0.0f);

    // apply rotation matrix from arcball
    fakeGL.MultMatrixf(renderParameters->rotationMatrix.columnMajor().coordinates);

    // now we start using the render parameters
    if (renderParameters->showAxes)
        { // show axes
        // start with lighting turned off
        fakeGL.Disable(FAKEGL_LIGHTING);

        // set the lines to be obvious in width
        fakeGL.LineWidth(4.0);

        // now draw one line for each axis in different colours
        fakeGL.Begin(FAKEGL_LINES);

        // X axis is red
        fakeGL.Color3f(1.0, 0.0, 0.0);
        fakeGL.Vertex3f(0.0, 0.0, 0.0);
        fakeGL.Vertex3f(1.0, 0.0, 0.0);

        // Y axis is green
        fakeGL.Color3f(0.0, 1.0, 0.0);
        fakeGL.Vertex3f(0.0, 0.0, 0.0);
        fakeGL.Vertex3f(0.0, 1.0, 0.0);

        // Z axis is red
        fakeGL.Color3f(0.0, 0.0, 1.0);
        fakeGL.Vertex3f(0.0, 0.0, 0.0);
        fakeGL.Vertex3f(0.0, 0.0, 1.0);

        // now reset the color, just in case
        fakeGL.Color3f(1.0, 1.0, 1.0);
        fakeGL.End();

        // reset lighting on if needed
        if (renderParameters->useLighting)
            fakeGL.Enable(FAKEGL_LIGHTING);
        } // show axes

    // tell the object to draw itself,
    // passing in the render parameters for reference
    if (renderParameters->showObject)
        texturedObject->FakeGLRender(renderParameters, &fakeGL);


} // FakeGLRenderWidget::paintFakeGL()

// mouse-handling
void FakeGLRenderWidget::mousePressEvent(QMouseEvent *event)
    { // FakeGLRenderWidget::mousePressEvent()
    // store the button for future reference
    int whichButton = event->button();
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0 * event->x() - size) / size;
    float y = (size - 2.0 * event->y() ) / size;

    
    // and we want to force mouse buttons to allow shift-click to be the same as right-click
    int modifiers = event->modifiers();
    
    // shift-click (any) counts as right click
    if (modifiers & Qt::ShiftModifier)
        whichButton = Qt::RightButton;
    
    // send signal to the controller for detailed processing
    emit BeginScaledDrag(whichButton, x,y);
    } // FakeGLRenderWidget::mousePressEvent()
    
void FakeGLRenderWidget::mouseMoveEvent(QMouseEvent *event)
    { // FakeGLRenderWidget::mouseMoveEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0 * event->x() - size) / size;
    float y = (size - 2.0 * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit ContinueScaledDrag(x,y);
    } // FakeGLRenderWidget::mouseMoveEvent()
    
void FakeGLRenderWidget::mouseReleaseEvent(QMouseEvent *event)
    { // FakeGLRenderWidget::mouseReleaseEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0 * event->x() - size) / size;
    float y = (size - 2.0 * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit EndScaledDrag(x,y);
    } // FakeGLRenderWidget::mouseReleaseEvent()
