///////////////////////////////////////////////////
//
//  Hamish Carr
//  September, 2020
//
//  ------------------------
//  TexturedObject.cpp
//  ------------------------
//  
//  Base code for rendering assignments.
//
//  Minimalist (non-optimised) code for reading and 
//  rendering an object file
//  
//  We will make some hard assumptions about input file
//  quality. We will not check for manifoldness or 
//  normal direction, &c.  And if it doesn't work on 
//  all object files, that's fine.
//
//  While I could set it up to use QImage for textures,
//  I want this code to be reusable without Qt, so I 
//  shall make a hard assumption that textures are in 
//  ASCII PPM and use my own code to read them
//  
///////////////////////////////////////////////////

// include the header file
#include "TexturedObject.h"

// include the C++ standard libraries we want
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// include the Cartesian 3- vector class
#include "Cartesian3.h"

#define MAXIMUM_LINE_LENGTH 1024

// constructor will initialise to safe values
TexturedObject::TexturedObject()
    : centreOfGravity(0.0,0.0,0.0)
    { // TexturedObject()
    // force arrays to size 0
    vertices.resize(0);
    normals.resize(0);
    textureCoords.resize(0);
    } // TexturedObject()

// read routine returns true on success, failure otherwise
bool TexturedObject::ReadObjectStream(std::istream &geometryStream, std::istream &textureStream)
    { // ReadObjectStream()
    
    // create a read buffer
    char readBuffer[MAXIMUM_LINE_LENGTH];
    
    // the rest of this is a loop reading lines & adding them in appropriate places
    while (true)
        { // not eof
        // character to read
        char firstChar = geometryStream.get();
        
        // check for eof() in case we've run out
        if (geometryStream.eof())
            break;

        // otherwise, switch on the character we read
        switch (firstChar)
            { // switch on first character
            case '#':       // comment line
                // read and discard the line
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);
                break;
                
            case 'v':       // vertex data of some type
                { // some sort of vertex data
                // retrieve another character
                char secondChar = geometryStream.get();
                
                // bail if we ran out of file
                if (geometryStream.eof())
                    break;

                // now use the second character to choose branch
                switch (secondChar)
                    { // switch on second character
                    case ' ':       // space - indicates a vertex
                        { // vertex read
                        Cartesian3 vertex;
                        geometryStream >> vertex;
                        vertices.push_back(vertex);
                        break;
                        } // vertex read
                    case 'n':       // n indicates normal vector
                        { // normal read
                        Cartesian3 normal;
                        geometryStream >> normal;
                        normals.push_back(normal);
                        break;
                        } // normal read
                    case 't':       // t indicates texture coords
                        { // tex coord
                        Cartesian3 texCoord;
                        geometryStream >> texCoord;
                        textureCoords.push_back(texCoord);
                        break;                  
                        } // tex coord
                    default:
                        break;
                    } // switch on second character 
                break;
                } // some sort of vertex data
                
            case 'f':       // face data
                { // face
                // a face can have an arbitrary number of vertices, which is a pain
                // so we will create a separate buffer to read from
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);

                // turn into a C++ string
                std::string lineString = std::string(readBuffer);

                // create a string stream
                std::stringstream lineParse(lineString); 

                // create vectors for the IDs (with different names from the master arrays)
                std::vector<unsigned int> faceVertexSet;
                std::vector<unsigned int> faceNormalSet;
                std::vector<unsigned int> faceTexCoordSet;
                
                // now loop through the line
                while (!lineParse.eof())
                    { // lineParse isn't done
                    // the triple of vertex, normal, tex coord IDs
                    unsigned int vertexID;
                    unsigned int normalID;
                    unsigned int texCoordID;

                    // try reading them in, breaking if we hit eof
                    lineParse >> vertexID;
                    // retrieve & discard a slash
                    lineParse.get();
                    // check eof
                    if (lineParse.eof())
                        break;
                    
                    // and the tex coord
                    lineParse >> texCoordID;
                    lineParse.get();
                    if (lineParse.eof())
                        break;
                        
                    // read normal likewise
                    lineParse >> normalID;
                        
                    // if we got this far, we presumably have three valid numbers, so add them
                    // but notice that .obj uses 1-based numbering, where our arrays use 0-based
                    faceVertexSet.push_back(vertexID-1);
                    faceNormalSet.push_back(normalID-1);
                    faceTexCoordSet.push_back(texCoordID-1);
                    } // lineParse isn't done

                // as long as the face has at least three vertices, add to the master list
                if (faceVertexSet.size() > 2)
                    { // at least 3
                    faceVertices.push_back(faceVertexSet);
                    faceNormals.push_back(faceNormalSet);
                    faceTexCoords.push_back(faceTexCoordSet);
                    } // at least 3
                
                break;
                } // face
                
            // default processing: do nothing
            default:
                break;

            } // switch on first character

        } // not eof

    // compute centre of gravity
    // note that very large files may have numerical problems with this
    centreOfGravity = Cartesian3(0.0, 0.0, 0.0);

    // if there are any vertices at all
    if (vertices.size() != 0)
        { // non-empty vertex set
        // sum up all of the vertex positions
        for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
            centreOfGravity = centreOfGravity + vertices[vertex];
        
        // and divide through by the number to get the average position
        // also known as the barycentre
        centreOfGravity = centreOfGravity / vertices.size();

        // start with 0 radius
        objectSize = 0.0;

        // now compute the largest distance from the origin to a vertex
        for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
            { // per vertex
            // compute the distance from the barycentre
            float distance = (vertices[vertex] - centreOfGravity).length();         
            
            // now test for maximality
            if (distance > objectSize)
                objectSize = distance;
            } // per vertex
        } // non-empty vertex set

    // now read in the texture file
    texture.ReadPPM(textureStream);

    // return a success code
    return true;
    } // ReadObjectStream()

// write routine
void TexturedObject::WriteObjectStream(std::ostream &geometryStream, std::ostream &textureStream)
    { // WriteObjectStream()
    // output the vertex coordinates
    for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
        geometryStream << "v  " << std::fixed << vertices[vertex] << std::endl;
    geometryStream << "# " << vertices.size() << " vertices" << std::endl;
    geometryStream << std::endl;

    // and the normal vectors
    for (unsigned int normal = 0; normal < normals.size(); normal++)
        geometryStream << "vn " << std::fixed << normals[normal] << std::endl;
    geometryStream << "# " << normals.size() << " vertex normals" << std::endl;
    geometryStream << std::endl;

    // and the texture coordinates
    for (unsigned int texCoord = 0; texCoord < textureCoords.size(); texCoord++)
        geometryStream << "vt " << std::fixed << textureCoords[texCoord] << std::endl;
    geometryStream << "# " << textureCoords.size() << " texture coords" << std::endl;
    geometryStream << std::endl;

    // and the faces
    for (unsigned int face = 0; face < faceVertices.size(); face++)
        { // per face
        geometryStream << "f ";
        
        // loop through # of vertices
        for (unsigned int vertex = 0; vertex < faceVertices[face].size(); vertex++)
            geometryStream << faceVertices[face][vertex]+1 << "/" << faceTexCoords[face][vertex]+1 << "/" << faceNormals[face][vertex]+1 << " " ;
        
        geometryStream << std::endl;
        } // per face
    geometryStream << "# " << faceVertices.size() << " polygons" << std::endl;
    geometryStream << std::endl;
    
    // now output the texture
    texture.WritePPM(textureStream);
    } // WriteObjectStream()

// routine to transfer assets to GPU
void TexturedObject::TransferAssetsToGPU()
    { // TransferAssetsToGPU()
    // when this is called, it transfers assets to the GPU.
    // for now, it will only be to transfer the texture
    // this may not be efficient, but it supports arbitrary sizes best
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // create a texture ID (essentially a pointer)
    glGenTextures(1, &textureID);
    // now bind to it - i.e. all following code addresses this one
    glBindTexture(GL_TEXTURE_2D, textureID);
    // set these parameters to avoid dealing with mipmaps 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // now transfer the image
    glTexImage2D(   
        GL_TEXTURE_2D,      // it's a 2D texture
        0,                  // mipmap level of 0 (ie the largest one)
        GL_RGBA,            // we want the data stored as RGBA on GPU
        texture.width,      // width of the image
        texture.height,     // height of the image
        0,                  // width of border (in texels)
        GL_RGBA,            // format the data is stored in on CPU
        GL_UNSIGNED_BYTE,   // data type
        texture.block       // and a pointer to the data
        );
    } // TransferAssetsToGPU()

// routine to transfer assets to Fake GL
void TexturedObject::TransferAssetsToFakeGL(FakeGL *fakeGL)
    { // TransferAssetsToFakeGL()
    // this is much simpler in comparison because we only support one format
    fakeGL->TexImage2D(texture);
    } // TransferAssetsToFakeGL()

// routine to render
void TexturedObject::Render(RenderParameters *renderParameters)
    { // Render()
    // Ideally, we would apply a global transformation to the object, but sadly that breaks down
    // when we want to scale things, as unless we normalise the normal vectors, we end up affecting
    // the illumination.  Known solutions include:
    // 1.   Normalising the normal vectors
    // 2.   Explicitly dividing the normal vectors by the scale to balance
    // 3.   Scaling only the vertex position (slower, but safer)
    // 4.   Not allowing spatial zoom (note: sniper scopes are a modified projection matrix)
    //
    // Inside a game engine, zoom usually doesn't apply. Normalisation of normal vectors is expensive,
    // so we will choose option 2.  

    // if we have texturing enabled . . . 
    if (renderParameters->texturedRendering)
        { // textures enabled
        // enable textures
        glEnable(GL_TEXTURE_2D);
        // use our other flag to specify replace or modulate
        if (renderParameters->textureModulation)
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        else
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        // now bind the texture ID
        glBindTexture(GL_TEXTURE_2D, textureID);
        } // textures enabled
    else
        { // textures disabled
        // make sure that they are disabled
        glDisable(GL_TEXTURE_2D);
        } // textures disabled

    // Scale defaults to the zoom setting
    float scale = renderParameters->zoomScale;
    
    // if object scaling is requested, apply it as well 
    if (renderParameters->scaleObject)
        scale /= objectSize;
        
    //  now scale everything
    glScalef(scale, scale, scale);

    // apply the translation to the centre of the object if requested
    if (renderParameters->centreObject)
        glTranslatef(-centreOfGravity.x, -centreOfGravity.y, -centreOfGravity.z);

    // emissive glow from object
    float emissiveColour[4];
    // default ambient / diffuse / specular colour
    float surfaceColour[4] = { 0.7, 0.7, 0.7, 1.0 };
    // specular shininess
    float shininess[4];
    // copy the intensity into RGB channels
    emissiveColour[0]   = emissiveColour[1] = emissiveColour[2] = renderParameters->emissiveLight;
    emissiveColour[3]   = 1.0; // don't forget alpha
    
    // set the shininess from the specular exponent
    shininess[0]        = shininess[1]      = shininess[2]      = renderParameters->specularExponent;
    shininess[3]        = 1.0; // alpha

    // start rendering
    glBegin(GL_TRIANGLES);

    // we assume a single material for the entire object
    glMaterialfv(GL_FRONT, GL_EMISSION, emissiveColour);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, surfaceColour);
    glMaterialfv(GL_FRONT, GL_SPECULAR, surfaceColour);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    
    // repeat this for colour - extra call, but saves if statements
    glColor3fv(surfaceColour);

    // loop through the faces: note that they may not be triangles, which complicates life
    for (unsigned int face = 0; face < faceVertices.size(); face++)
        { // per face
        // on each face, treat it as a triangle fan starting with the first vertex on the face
        for (unsigned int triangle = 0; triangle < faceVertices[face].size() - 2; triangle++)
            { // per triangle
            // now do a loop over three vertices
            for (unsigned int vertex = 0; vertex < 3; vertex++)
                { // per vertex
                // we always use the face's vertex 0
                int faceVertex = 0;
                // so if it isn't 0, we want to add the triangle base ID
                if (vertex != 0)
                    faceVertex = triangle + vertex;

                // now we use that ID to lookup
                glNormal3f
                    (
                    normals         [faceNormals    [face][faceVertex]  ].x * scale,
                    normals         [faceNormals    [face][faceVertex]  ].y * scale,
                    normals         [faceNormals    [face][faceVertex]  ].z * scale
                    );
                    
                // if we're using UVW colours, set both colour and material
                if (renderParameters->mapUVWToRGB)
                    { // set colour and material
                    float *colourPointer = (float *) &(textureCoords[faceTexCoords[face][faceVertex]]);
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colourPointer);
                    glMaterialfv(GL_FRONT, GL_SPECULAR, colourPointer);
                    glColor3fv(colourPointer);
                    } // set colour and material
                // set the texture coordinate
                glTexCoord2f
                    (
                    textureCoords   [faceTexCoords  [face][faceVertex]  ].x,
                    textureCoords   [faceTexCoords  [face][faceVertex]  ].y
                    );
                    
                // and set the vertex position
                glVertex3f
                    (
                    vertices        [faceVertices   [face][faceVertex]].x,
                    vertices        [faceVertices   [face][faceVertex]].y,
                    vertices        [faceVertices   [face][faceVertex]].z
                    );
                } // per vertex
            } // per triangle
        } // per face

    // close off the triangles
    glEnd();

    // if we have texturing enabled, turn texturing back off 
    if (renderParameters->texturedRendering)
        glDisable(GL_TEXTURE_2D);
    } // Render()

// routine for students to use when rendering
void TexturedObject::FakeGLRender(RenderParameters *renderParameters, FakeGL *fakeGL)
    { // FakeGLRender()
    // Ideally, we would apply a global transformation to the object, but sadly that breaks down
    // when we want to scale things, as unless we normalise the normal vectors, we end up affecting
    // the illumination.  Known solutions include:
    // 1.   Normalising the normal vectors
    // 2.   Explicitly dividing the normal vectors by the scale to balance
    // 3.   Scaling only the vertex position (slower, but safer)
    // 4.   Not allowing spatial zoom (note: sniper scopes are a modified projection matrix)
    //
    // Inside a game engine, zoom usually doesn't apply. Normalisation of normal vectors is expensive,
    // so we will choose option 2.  

    // if we have texturing enabled . . . 
    if (renderParameters->texturedRendering)
        { // textures enabled
        // enable textures
        fakeGL->Enable(FAKEGL_TEXTURE_2D);
        // use our other flag to specify replace or modulate
        if (renderParameters->textureModulation)
            fakeGL->TexEnvMode(FAKEGL_MODULATE);
        else
            fakeGL->TexEnvMode(FAKEGL_REPLACE);
        } // textures enabled
    else
        { // textures disabled
        // make sure that they are disabled
        fakeGL->Disable(FAKEGL_TEXTURE_2D);
        } // textures disabled

    // Scale defaults to the zoom setting
    float scale = renderParameters->zoomScale;
    
    // if object scaling is requested, apply it as well 
    if (renderParameters->scaleObject)
        scale /= objectSize;
        
    //  now scale everything
    fakeGL->Scalef(scale, scale, scale);

    // apply the translation to the centre of the object if requested
    if (renderParameters->centreObject)
        fakeGL->Translatef(-centreOfGravity.x, -centreOfGravity.y, -centreOfGravity.z);

    // emissive glow from object
    float emissiveColour[4];
    // default ambient / diffuse / specular colour
    float surfaceColour[4] = { 0.7, 0.7, 0.7, 1.0 };

    // copy the intensity into RGB channels
    emissiveColour[0]   = emissiveColour[1] = emissiveColour[2] = renderParameters->emissiveLight;
    emissiveColour[3]   = 1.0; // don't forget alpha

    // start rendering
    fakeGL->Begin(FAKEGL_TRIANGLES);

    // we assume a single material for the entire object
    fakeGL->Materialfv(FAKEGL_EMISSION, emissiveColour);
    fakeGL->Materialfv(FAKEGL_AMBIENT_AND_DIFFUSE, surfaceColour);
    fakeGL->Materialfv(FAKEGL_SPECULAR, surfaceColour);
    fakeGL->Materialf(FAKEGL_SHININESS, renderParameters->specularExponent);
    
    // repeat this for colour - extra call, but saves if statements
    fakeGL->Color3f(surfaceColour[0], surfaceColour[1], surfaceColour[2]);

    // loop through the faces: note that they may not be triangles, which complicates life
    for (unsigned int face = 0; face < faceVertices.size(); face++)
        { // per face
        // on each face, treat it as a triangle fan starting with the first vertex on the face
        for (unsigned int triangle = 0; triangle < faceVertices[face].size() - 2; triangle++)
            { // per triangle
            // now do a loop over three vertices
            for (unsigned int vertex = 0; vertex < 3; vertex++)
                { // per vertex
                // we always use the face's vertex 0
                int faceVertex = 0;
                // so if it isn't 0, we want to add the triangle base ID
                if (vertex != 0)
                    faceVertex = triangle + vertex;

                // now we use that ID to lookup
                fakeGL->Normal3f
                    (
                    normals         [faceNormals    [face][faceVertex]  ].x * scale,
                    normals         [faceNormals    [face][faceVertex]  ].y * scale,
                    normals         [faceNormals    [face][faceVertex]  ].z * scale
                    );
                    
                // if we're using UVW colours, set both colour and material
                if (renderParameters->mapUVWToRGB)
                    { // set colour and material
                    float *colourPointer = (float *) &(textureCoords[faceTexCoords[face][faceVertex]]);
                    fakeGL->Materialfv(FAKEGL_AMBIENT_AND_DIFFUSE, colourPointer);
                    fakeGL->Materialfv(FAKEGL_SPECULAR, colourPointer);
                    fakeGL->Color3f(colourPointer[0], colourPointer[1], colourPointer[2]);
                    } // set colour and material

                // set the texture coordinate
                fakeGL->TexCoord2f
                    (
                    textureCoords   [faceTexCoords  [face][faceVertex]  ].x,
                    textureCoords   [faceTexCoords  [face][faceVertex]  ].y
                    );
                    
                // and set the vertex position
                fakeGL->Vertex3f
                    (
                    vertices        [faceVertices   [face][faceVertex]].x,
                    vertices        [faceVertices   [face][faceVertex]].y,
                    vertices        [faceVertices   [face][faceVertex]].z
                    );
                } // per vertex
            } // per triangle
        } // per face

    // close off the triangles
    fakeGL->End();

    // if we have texturing enabled, turn texturing back off 
    if (renderParameters->texturedRendering)
        fakeGL->Disable(FAKEGL_TEXTURE_2D);
    } // FakeGLRender()

