//
//  main.cpp
//  01-OpenGLProject
//
//  Created by Yuma Taesu on 2016/10/08.
//  Copyright © 2016年 Yuma Taesu. All rights reserved.
//

#include <iostream>
#include <string>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"

#include "Shader.h"
#include "Camera.h"
#include "Box.hpp"
#include "Rect.h"


using namespace std;

// Properties
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void ScrollCallback( GLFWwindow *window, double xOffset, double yOffset );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement();
void setObjects();
void init();
GLuint loadTexture(GLuint textureID, const char* path, int imageWidth, int imageHeight);
bool checkWindow(GLFWwindow *window);
bool initGLEW();
void RenderQuad();

// Camera
Camera  camera(glm::vec3( 0.0f, 0.0f, 6.0f ) );
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

const int boxnum = 5000;


int main()
{
    init();
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "Diferred Rendering", nullptr, nullptr );
    checkWindow(window);
    
    glfwMakeContextCurrent( window );
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    glfwSetKeyCallback( window, KeyCallback );
    glfwSetCursorPosCallback( window, MouseCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    initGLEW();
    
    
    // Positions all boxes
    glm::vec3 cubePositions[boxnum];
    random_device rnd;
    mt19937 mt(rnd());
    std::uniform_int_distribution<> x(-20.0, 20.0);
    std::uniform_int_distribution<> y(-20.0, 20.0);
    std::uniform_int_distribution<> z(-20.0, 20.0);
    for(int i = 0; i < boxnum; i++){ cubePositions[i] = glm::vec3(x(mt), y(mt), z(mt)); }
    
    Box box;
    box.setup();
    
    Rect drawRect;
    drawRect.setup();
    
    //Load textures
    GLuint diffuseMap, specularMap;
    int imageWidth, imageHeight;
    const char* diffusepath = "res/images/concrete.jpg";
    const char* specularpath = "res/images/container2_specular.png";
    diffuseMap = loadTexture(diffuseMap, diffusepath, imageWidth, imageHeight);
    specularMap = loadTexture(diffuseMap, specularpath, imageWidth, imageHeight);
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    glEnable( GL_DEPTH_TEST );
    
    // Build and compile our shader program
    Shader GeometryPass( "res/shaders/gBuffer.vs", "res/shaders/gBuffer.frag" );
    GLint modelLoc = glGetUniformLocation( GeometryPass.Program, "model" );
    GLint viewLoc  = glGetUniformLocation( GeometryPass.Program, "view" );
    GLint projLoc  = glGetUniformLocation( GeometryPass.Program, "projection" );
    GLint textureLoc = glGetUniformLocation( GeometryPass.Program, "tex" );
    
    
    Shader LightingPass( "res/shaders/passthrough.vs", "res/shaders/passthrough.frag" );
    GLint gPositionLoc = glGetUniformLocation(LightingPass.Program, "gPosition");
    GLint gNormalLoc = glGetUniformLocation(LightingPass.Program, "gNormal");
    GLint gAlbedoloc = glGetUniformLocation(LightingPass.Program, "gAlbedo");
    GLint lightAmbloc = glGetUniformLocation(LightingPass.Program, "light.ambient");
    GLint lightDiffloc = glGetUniformLocation(LightingPass.Program, "light.diffuse");
    GLint lightSpecloc = glGetUniformLocation(LightingPass.Program, "light.specular");
    GLint lightPosLoc = glGetUniformLocation(LightingPass.Program, "light.position");
    GLint viewPosLoc = glGetUniformLocation(LightingPass.Program, "viewPos");
    
    // Load and create a texture
    GLuint tex;
    
    int width, height;
    // ===================
    // Texture
    // ===================
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );
    // Set our texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    // Set texture filtering
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // Load, create texture and generate mipmaps
    unsigned char *image = SOIL_load_image( "res/images/concrete.jpg", &width, &height, 0, SOIL_LOAD_RGBA );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
    glGenerateMipmap( GL_TEXTURE_2D );
    SOIL_free_image_data( image );
    glBindTexture( GL_TEXTURE_2D, 0 );
    
    
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLuint gPosition, gNormal, gAlbedo;
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // - Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    //Loop
    while ( !glfwWindowShouldClose( window ) )
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        
        glfwPollEvents();
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        DoMovement();
        
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        
        GeometryPass.Use();
        glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f );
        glm::mat4 view = camera.GetViewMatrix();
        
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view) );
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(textureLoc, 0);
        for(int i = 0; i < boxnum; i++)
        {
            glm::mat4 model = glm::mat4();
            
            model = glm::translate(model, cubePositions[i]);
            GLfloat angle = 20.0f + currentFrame * 0.2;
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            box.draw();
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LightingPass.Use();
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(lightAmbloc, 0.0, 0.0, 0.0);
        glUniform3f(lightDiffloc, 0.3, 0.1, 0.3);
        glUniform3f(lightSpecloc, 1.0, 1.0, 1.0);
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.constant" ), 1.2 );
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.linear" ), 0.19 );
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.quadratic" ), 0.001 );
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glUniform1i(gPositionLoc, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glUniform1i(gNormalLoc, 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glUniform1i(gAlbedoloc, 2);
        
        drawRect.draw();
        
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    box.release();
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &specularMap);
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate( );
    
    return 0;
}

GLuint loadTexture(GLuint textureID, const char* path, int imageWidth, int imageHeight)
{
    unsigned char *image;
    glGenTextures( 1, &textureID );
    
    image = SOIL_load_image(path, &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB);
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
    glGenerateMipmap( GL_TEXTURE_2D );
    SOIL_free_image_data( image );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST );
    glBindTexture( GL_TEXTURE_2D, 0 );
    
    
    return textureID;
}



void DoMovement( )
{
    // Camera controls
    if( keys[GLFW_KEY_W] || keys[GLFW_KEY_UP] )
    {
        camera.ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if( keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN] )
    {
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if( keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT] )
    {
        camera.ProcessKeyboard( LEFT, deltaTime );
    }
    
    if( keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT] )
    {
        camera.ProcessKeyboard( RIGHT, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
    if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

void MouseCallback( GLFWwindow *window, double xPos, double yPos )
{
    if( firstMouse )
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement( xOffset, yOffset );
}


void ScrollCallback( GLFWwindow *window, double xOffset, double yOffset )
{
    camera.ProcessMouseScroll( yOffset );
}

void init()
{
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
}

bool initGLEW()
{
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if ( GLEW_OK != glewInit( ) )
    {
        cout << "Failed to initialize GLEW" << endl;
        return EXIT_FAILURE;
    }
    
    return true;
}

bool checkWindow(GLFWwindow* window)
{
    if ( nullptr == window )
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate( );
        
        return EXIT_FAILURE;
    }
    
    return true;
}

