/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <type your USC username here>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 30.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

GLuint pointVertexBuffer, pointColorVertexBuffer;
GLuint lineVertexBuffer, lineColorVertexBuffer;
GLuint triVertexBuffer, triColorVertexBuffer;

GLuint smoothVertexBuffer, smoothColorVertexBuffer;

GLuint SmoothLeftBuffer, SmoothRightBuffer, SmoothUpBuffer, SmoothDownBuffer;

GLuint pointVertexArray;
GLuint lineVertexArray;
GLuint triVertexArray;

GLuint smoothVertexArray;

int sizePoint;
int sizeLine;
int sizeTri;
int sizeSmooth;

OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;

int mode = 1;
int AnimationFrameCount = 0;

//GLint h_mode; //handle to mode variable in shader
//SET_SHADER_VARIABLE_HANDLE(mode);

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc()
{
  // render some stuff...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.LookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);


  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Scale(landScale[0], landScale[1], landScale[2]);


  float m[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(m);

  float p[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(p);
  //
  // bind shader
  pipelineProgram->Bind();

  // set variable
  pipelineProgram->SetModelViewMatrix(m);
  pipelineProgram->SetProjectionMatrix(p);

  //glBindVertexArray(triVertexArray);
  //glDrawArrays(GL_TRIANGLES, 0, sizeTri);

  if (mode == 1) {
      glBindVertexArray(pointVertexArray);
      glDrawArrays(GL_POINTS, 0, sizePoint);
      pipelineProgram->SetMode(1);
  }
  else if (mode == 2) {
      glBindVertexArray(lineVertexArray);
      glDrawArrays(GL_LINES, 0, sizeLine);
      pipelineProgram->SetMode(1);
  }
  else if (mode == 3) {
      glBindVertexArray(triVertexArray);
      glDrawArrays(GL_TRIANGLES, 0, sizeTri);
      pipelineProgram->SetMode(1);
  }
  else if (mode == 4) {
      glBindVertexArray(smoothVertexArray);
      glDrawArrays(GL_TRIANGLES, 0, sizeSmooth);
      pipelineProgram->SetMode(2);
  }

  glutSwapBuffers();
}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 100.0f);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x': {
        // take a screenshot
        string zeros = "";
        if (AnimationFrameCount < 10) {
            zeros = "00";
        }
        else if (AnimationFrameCount < 100) {
            zeros = "0";
        }
        else {
            zeros = "";
        }
        string screenshotname = zeros + std::to_string(AnimationFrameCount) + ".jpg";
        AnimationFrameCount++;
        saveScreenshot(const_cast<char*>(screenshotname.c_str()));
        break; }

    case '1':
        //points
        //glUniform1i(GLint location,GLint v0);
        mode = 1;
            
    break;
    case '2':
        //lines
        mode = 2;
        break;
    case '3':
        //triangles
        mode = 3;
        break;
    case '4':
        //smooth
        mode = 4;
        break;
    /*
    case 'r':
        
        controlState = ROTATE;
        break;

    case 't':
        //cout << "RRR" << endl;
        controlState = TRANSLATE;
        break;

    case 's':
        controlState = SCALE;
        break;
        */
  }
}

void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();

  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  //if (heightmapImage->loadJPEG("heightmap/OhioPyle-128.jpg") != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
 
  // modify the following code accordingly

  int imageHeight = heightmapImage->getHeight();
  int imageWidth = heightmapImage->getWidth();
  //points
  int numVertices = imageHeight * imageWidth;
  glm::vec3 * vertPoints = new glm::vec3[imageHeight * imageWidth];
  glm::vec4 * colPoints = new glm::vec4[imageHeight * imageWidth];
  // (… fill the “vertices” array …)
  float scale = 1;
  float imageScale = 4.5;
  for (int i = 0; i < imageHeight; i++) {
      for (int j = 0; j < imageWidth; j++) {
        float height = scale * heightmapImage->getPixel(i, j, 0);
        //cout << height << endl;
        vertPoints[i * imageWidth + j] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale,(float)height/256 , -(float)(j - imageWidth / 2) / imageWidth * imageScale);
        colPoints[i * imageWidth + j] = glm::vec4((float)height/ 256, (float)height / 256, (float)height / 256, 0);
      }
  }
  // create the VBO:
  //GLuint vbo;

  glGenBuffers(1, &pointVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, pointVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVertices,
      vertPoints, GL_STATIC_DRAW);

  glGenBuffers(1, &pointColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, pointColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numVertices, colPoints, GL_STATIC_DRAW);

  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  if (ret != 0) abort();

  glGenVertexArrays(1, &pointVertexArray);
  glBindVertexArray(pointVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, pointVertexBuffer);

  GLuint loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, pointColorVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glEnable(GL_DEPTH_TEST);

  sizePoint = numVertices;
 


  //lines

  int numVerticesLine = (imageHeight -1 ) * (imageWidth-1) * 4 + (imageHeight-1) * 2 + (imageWidth-1)*2;
  glm::vec3* vertLines = new glm::vec3[numVerticesLine];
  glm::vec4* colLines = new glm::vec4[numVerticesLine];
  // (… fill the “vertices” array …)
  //float scale = 1;
  //float imageScale = 4.5;
  for (int i = 0; i < imageHeight - 1 ; i++) {
      for (int j = 0; j < imageWidth - 1; j++) {
          float height = scale * heightmapImage->getPixel(i, j, 0);
          float heighti = scale * heightmapImage->getPixel(i+1, j, 0);
          float heightj = scale * heightmapImage->getPixel(i, j+1, 0);
          //draw lines between (i,j),(i+1,j)  and (i,j)(i,j+1)
          vertLines[(i * (imageWidth-1) + j)*4] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertLines[(i * (imageWidth - 1) + j)*4 + 1] = glm::vec3(-(float)(i+1 - imageHeight / 2) / imageHeight * imageScale, (float)heighti / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          
          vertLines[(i * (imageWidth - 1) + j)*4 + 2] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertLines[(i * (imageWidth - 1) + j)*4 + 3] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)heightj / 256, -(float)(j+1 - imageWidth / 2) / imageWidth * imageScale);
          colLines[(i * (imageWidth - 1) + j) * 4] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colLines[(i * (imageWidth - 1) + j) * 4 + 2] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colLines[(i * (imageWidth - 1) + j) * 4 + 1] = glm::vec4((float)heighti / 256, (float)heighti / 256, (float)heighti / 256, 0);
          colLines[(i * (imageWidth - 1) + j) * 4 + 3] = glm::vec4((float)heightj / 256, (float)heightj / 256, (float)heightj / 256, 0);
      }
  }

  //lines on the edges:
  for (int i = 0; i < imageHeight - 1; i++) {
      float height = scale * heightmapImage->getPixel(i, imageWidth - 1, 0);
      float heighti = scale * heightmapImage->getPixel(i+1, imageWidth - 1, 0);
      vertLines[(imageHeight - 1) * (imageWidth - 1) * 4 + 2*i] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height/256, -(float)((imageWidth-1)-imageWidth / 2) / imageWidth * imageScale);
      vertLines[(imageHeight - 1) * (imageWidth - 1) * 4 + 2*i + 1] = glm::vec3(-(float)(i+1 - imageHeight / 2) / imageHeight * imageScale, (float)heighti/256, -(float)((imageWidth-1)-imageWidth / 2) / imageWidth * imageScale);
      colLines[(imageHeight - 1) * (imageWidth - 1) * 4 + 2 * i] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
      colLines[(imageHeight - 1) * (imageWidth - 1) * 4 + 2 * i + 1] = glm::vec4((float)heighti / 256, (float)heighti / 256, (float)heighti / 256, 0);
  }
  
  for (int j = 0; j < imageWidth - 1; j++) {
    float height = scale * heightmapImage->getPixel(imageHeight-1, j, 0);
    float heightj = scale * heightmapImage->getPixel(imageHeight-1, j+1, 0);
    vertLines[(imageHeight - 1) * (imageWidth - 1) * 4 + (imageHeight - 1) * 2 + 2*j] = glm::vec3(-(float)((imageHeight-1) - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j-imageWidth / 2) / imageWidth * imageScale);
    vertLines[(imageHeight - 1) * (imageWidth - 1) * 4 + (imageHeight - 1) * 2 + 2*j + 1] = glm::vec3(-(float)((imageHeight-1)-imageHeight / 2) / imageHeight * imageScale, (float)heightj / 256, -(float)(j+1-imageWidth / 2) / imageWidth * imageScale);
    colLines[(imageHeight - 1) * (imageWidth - 1) * 4 + (imageHeight - 1) * 2 + 2 * j] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
    colLines[(imageHeight - 1) * (imageWidth - 1) * 4 + (imageHeight - 1) * 2 + 2 * j + 1] = glm::vec4((float)heightj / 256, (float)heightj/ 256, (float)heightj / 256, 0);
  }

  // create the VBO:
  //GLuint vbo;

  //missing: lines on the edges.

  glGenBuffers(1, &lineVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesLine,
      vertLines, GL_STATIC_DRAW);

  glGenBuffers(1, &lineColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, lineColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numVerticesLine, colLines, GL_STATIC_DRAW);

  //pipelineProgram = new BasicPipelineProgram;
  //int ret = pipelineProgram->Init(shaderBasePath);
  //if (ret != 0) abort();

  glGenVertexArrays(1, &lineVertexArray);
  glBindVertexArray(lineVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);

  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, lineColorVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glEnable(GL_DEPTH_TEST);

  sizeLine = numVerticesLine;



  //triangles
  int numVerticesTri = (imageHeight - 1) * (imageWidth - 1) * 6;
  glm::vec3* vertTri = new glm::vec3[numVerticesTri];
  glm::vec4* colTri = new glm::vec4[numVerticesTri];
  // (… fill the “vertices” array …)
  //float scale = 1;
  //float imageScale = 4.5;
  for (int i = 0; i < imageHeight - 1; i++) {
      for (int j = 0; j < imageWidth - 1; j++) {
          float height = scale * heightmapImage->getPixel(i, j, 0);
          float heighti = scale * heightmapImage->getPixel(i + 1, j, 0);
          float heightj = scale * heightmapImage->getPixel(i, j + 1, 0);
          float heightij = scale * heightmapImage->getPixel(i + 1, j + 1, 0);
          //draw triangles between (i,j),(i+1,j),(i+1,j+1)  and (i,j)(i,j+1)(i+1,j+1)
          vertTri[(i * (imageWidth - 1) + j) * 6] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertTri[(i * (imageWidth - 1) + j) * 6 + 1] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heighti / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertTri[(i * (imageWidth - 1) + j) * 6+ 2] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heightij / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          vertTri[(i * (imageWidth - 1) + j) * 6 + 3] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertTri[(i * (imageWidth - 1) + j) * 6 + 4] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heightij / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          vertTri[(i * (imageWidth - 1) + j) * 6 + 5] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)heightj / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          colTri[(i * (imageWidth - 1) + j) * 6] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colTri[(i * (imageWidth - 1) + j) * 6 + 1] = glm::vec4((float)heighti / 256, (float)heighti / 256, (float)heighti / 256, 0);
          colTri[(i * (imageWidth - 1) + j) * 6 + 2] = glm::vec4((float)heightij / 256, (float)heightij / 256, (float)heightij / 256, 0);
          colTri[(i * (imageWidth - 1) + j) * 6 + 3] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colTri[(i * (imageWidth - 1) + j) * 6 + 4] = glm::vec4((float)heightij / 256, (float)heightij / 256, (float)heightij / 256, 0);
          colTri[(i * (imageWidth - 1) + j) * 6 + 5] = glm::vec4((float)heightj / 256, (float)heightj / 256, (float)heightj / 256, 0);
      }
  }
  // create the VBO:
  //GLuint vbo;

  //missing: lines on the edges.

  glGenBuffers(1, &triVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesTri,
      vertTri, GL_STATIC_DRAW);

  glGenBuffers(1, &triColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numVerticesTri, colTri, GL_STATIC_DRAW);

  //pipelineProgram = new BasicPipelineProgram;
  //int ret = pipelineProgram->Init(shaderBasePath);
  //if (ret != 0) abort();

  glGenVertexArrays(1, &triVertexArray);
  glBindVertexArray(triVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);

  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glEnable(GL_DEPTH_TEST);

  sizeTri = numVerticesTri;




  //smooth triangles


  int numVerticesSmooth = (imageHeight - 3) * (imageWidth - 3) * 6;
  glm::vec3* vertSmooth = new glm::vec3[numVerticesSmooth];
  glm::vec4* colSmooth = new glm::vec4[numVerticesSmooth];

  glm::vec3* left = new glm::vec3[numVerticesSmooth];
  glm::vec3* right = new glm::vec3[numVerticesSmooth];
  glm::vec3* up = new glm::vec3[numVerticesSmooth];
  glm::vec3* down = new glm::vec3[numVerticesSmooth];
  //float* right = new float[numVerticesSmooth];
  //float* up = new float[numVerticesSmooth];
  //float* down = new float[numVerticesSmooth];


  // (… fill the “vertices” array …)
  //float scale = 1;
  //float imageScale = 4.5;
  for (int i = 1; i < imageHeight - 2; i++) {
      for (int j = 1; j < imageWidth - 2; j++) {
          float height = scale * heightmapImage->getPixel(i, j, 0);
          float heighti = scale * heightmapImage->getPixel(i + 1, j, 0);
          float heightj = scale * heightmapImage->getPixel(i, j + 1, 0);
          float heightij = scale * heightmapImage->getPixel(i + 1, j + 1, 0);
          //float heightl = scale * heightmapImage->getPixel(i - 1, j, 0);
          //float heightd = scale * heightmapImage->getPixel(i , j - 1, 0);
          //draw triangles between (i,j),(i+1,j),(i+1,j+1)  and (i,j)(i,j+1)(i+1,j+1)
          vertSmooth[((i-1) * (imageWidth - 3) + j - 1) * 6] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heighti / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heightij / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          vertSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)height / 256, -(float)(j - imageWidth / 2) / imageWidth * imageScale);
          vertSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec3(-(float)(i + 1 - imageHeight / 2) / imageHeight * imageScale, (float)heightij / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          vertSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec3(-(float)(i - imageHeight / 2) / imageHeight * imageScale, (float)heightj / 256, -(float)(j + 1 - imageWidth / 2) / imageWidth * imageScale);
          
          //pass only the height. I tried to pass only float instead of vec3, but failed :(
          left[((i - 1) * (imageWidth - 3) + j - 1 ) * 6] = glm::vec3(scale * heightmapImage->getPixel(i - 1, j, 0)/256,0.0f, 0.0f);
          left[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec3(scale * heightmapImage->getPixel(i, j, 0)/256, 0.0f, 0.0f);
          left[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec3(heightj/256, 0.0f, 0.0f);
          left[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec3(scale * heightmapImage->getPixel(i - 1, j, 0)/256, 0.0f, 0.0f);
          left[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec3(heightj/256, 0.0f, 0.0f);
          left[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec3(scale * heightmapImage->getPixel(i - 1, j + 1, 0) / 256, 0.0f, 0.0f);

          right[((i - 1) * (imageWidth - 3) + j - 1) * 6] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j, 0) / 256, 0.0f, 0.0f);
          right[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec3(scale * heightmapImage->getPixel(i + 2, j, 0) / 256, 0.0f, 0.0f);
          right[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec3(scale * heightmapImage->getPixel(i+2, j+1, 0) / 256, 0.0f, 0.0f);
          right[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec3(scale * heightmapImage->getPixel(i+1, j, 0) / 256, 0.0f, 0.0f);
          right[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec3(scale * heightmapImage->getPixel(i+2, j+1, 0) / 256, 0.0f, 0.0f);
          right[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec3(scale * heightmapImage->getPixel(i+1, j+1, 0) / 256, 0.0f, 0.0f);

          up[((i - 1) * (imageWidth - 3) + j - 1) * 6] = glm::vec3(scale * heightmapImage->getPixel(i , j - 1, 0) / 256, 0.0f, 0.0f);
          up[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j-1, 0) / 256, 0.0f, 0.0f);
          up[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j, 0) / 256, 0.0f, 0.0f);
          up[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec3(scale * heightmapImage->getPixel(i, j-1, 0) / 256, 0.0f, 0.0f);
          up[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j, 0) / 256, 0.0f, 0.0f);
          up[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec3(scale * heightmapImage->getPixel(i , j , 0) / 256, 0.0f, 0.0f);

          down[((i - 1) * (imageWidth - 3) + j - 1) * 6] = glm::vec3(scale * heightmapImage->getPixel(i, j + 1, 0) / 256, 0.0f, 0.0f);
          down[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j + 1, 0) / 256, 0.0f, 0.0f);
          down[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j + 2, 0) / 256, 0.0f, 0.0f);
          down[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec3(scale * heightmapImage->getPixel(i, j + 1, 0) / 256, 0.0f, 0.0f);
          down[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec3(scale * heightmapImage->getPixel(i + 1, j + 2, 0) / 256, 0.0f, 0.0f);
          down[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec3(scale * heightmapImage->getPixel(i, j+2, 0) / 256, 0.0f, 0.0f);
          
          
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 1] = glm::vec4((float)heighti / 256, (float)heighti / 256, (float)heighti / 256, 0);
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 2] = glm::vec4((float)heightij / 256, (float)heightij / 256, (float)heightij / 256, 0);
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 3] = glm::vec4((float)height / 256, (float)height / 256, (float)height / 256, 0);
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 4] = glm::vec4((float)heightij / 256, (float)heightij / 256, (float)heightij / 256, 0);
          colSmooth[((i - 1) * (imageWidth - 3) + j - 1) * 6 + 5] = glm::vec4((float)heightj / 256, (float)heightj / 256, (float)heightj / 256, 0);
      }
  }
  // create the VBO:
  //GLuint vbo;

  glGenBuffers(1, &SmoothLeftBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, SmoothLeftBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* numVerticesSmooth, left,
      GL_STATIC_DRAW);

  glGenBuffers(1, &SmoothRightBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, SmoothRightBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesSmooth, right,
      GL_STATIC_DRAW);

  glGenBuffers(1, &SmoothUpBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, SmoothUpBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesSmooth, up,
      GL_STATIC_DRAW);

  glGenBuffers(1, &SmoothDownBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, SmoothDownBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesSmooth, down,
      GL_STATIC_DRAW);
  

  //missing: lines on the edges.

  glGenBuffers(1, &smoothVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, smoothVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesSmooth,
      vertSmooth, GL_STATIC_DRAW);

  glGenBuffers(1, &smoothColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, smoothColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numVerticesSmooth, colSmooth, GL_STATIC_DRAW);



  //pipelineProgram = new BasicPipelineProgram;
  //int ret = pipelineProgram->Init(shaderBasePath);
  //if (ret != 0) abort();
  

  glGenVertexArrays(1, &smoothVertexArray);
  glBindVertexArray(smoothVertexArray);
  

  glBindBuffer(GL_ARRAY_BUFFER, SmoothLeftBuffer);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "left");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, SmoothRightBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "right");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, SmoothUpBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "up");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, SmoothDownBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "down");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);



    glBindBuffer(GL_ARRAY_BUFFER, smoothVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);
  



  glBindBuffer(GL_ARRAY_BUFFER, smoothColorVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);



  glEnable(GL_DEPTH_TEST);

  sizeSmooth = numVerticesSmooth;
  
  /*
  glm::vec3 triangle[3] = {
    glm::vec3(0, 0, 0), 
    glm::vec3(0, 1, 0),
    glm::vec3(1, 0, 0)
  };

  glm::vec4 color[3] = {
    {0, 0, 1, 1},
    {1, 0, 0, 1},
    {0, 1, 0, 1},
  };
    */
  /*
  glGenBuffers(1, &triVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, triangle,
               GL_STATIC_DRAW);

  glGenBuffers(1, &triColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * 3, color, GL_STATIC_DRAW);
   */


  //pipelineProgram = new BasicPipelineProgram;
  //int ret = pipelineProgram->Init(shaderBasePath);
  //if (ret != 0) abort();

  //glGenVertexArrays(1, &triVertexArray);
  //glBindVertexArray(triVertexArray);
  //glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);

  //GLuint loc =
  //    glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  //glEnableVertexAttribArray(loc);
  //glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  //glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
  //loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  //glEnableVertexAttribArray(loc);
  //glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  //glEnable(GL_DEPTH_TEST);
  
  //sizeTri = 3;

  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


