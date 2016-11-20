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

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

const int boxnum = 5000;

int main()
{
    init();
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr );
    checkWindow(window);
    
    glfwMakeContextCurrent( window );
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    glfwSetKeyCallback( window, KeyCallback );
    glfwSetCursorPosCallback( window, MouseCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    initGLEW();
    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( GLfloat )SCREEN_WIDTH / ( GLfloat )SCREEN_HEIGHT, 0.1f, 100.0f );
    
    // Positions all containers
    glm::vec3 cubePositions[boxnum];
    random_device rnd;
    mt19937 mt(rnd());
    std::uniform_int_distribution<> x(-20.0, 20.0);
    std::uniform_int_distribution<> y(-20.0, 20.0);
    std::uniform_int_distribution<> z(-20.0, 20.0);
    
    for(int i = 0; i < boxnum; i++){ cubePositions[i] = glm::vec3(x(mt), y(mt), z(mt)); }
    
    // First, set the container's VAO (and VBO)
    Box box;
    box.setup();
    
    Rect drawRect;
    drawRect.setup();
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    glEnable( GL_DEPTH_TEST );
    
    // Build and compile our shader program
    Shader lightingShader( "res/shaders/lighting.vs", "res/shaders/lighting.frag" );
    Shader geometryPass( "res/shaders/deferred_shading.vs", "res/shaders/deferred_shading.frag" );
    Shader quad_program( "res/shaders/passthrough.vs", "res/shaders/passthrough.frag" );
    
    //================================
    //texture
    
    //フレームバッファオブジェクトを用意
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //color textureを用意
    GLuint gAlbedo;
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0); //フレームバッファにカラーテクスチャをアタッチ

    
    
    //position textureを用意
    GLuint gNormal;
    glGenTextures(1, &gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 1); //フレームバッファにカラーテクスチャをアタッチ
    
    
    //normal textureを用意
    GLuint gPosition;
    glGenTextures(1, &gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gPosition, 2); //フレームバッファにカラーテクスチャをアタッチ
    
    
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    
    //depth render textureを用意
    GLuint renderb;
    glGenRenderbuffers(1, &renderb);
    glBindRenderbuffer(GL_RENDERBUFFER, renderb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderb); //フレームバッファにデプスバッファとして、レンダーバッファをアタッチ
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //Unbind fbo
    //==================================
    
    
    //Load textures
    GLuint diffuseMap, specularMap;
    int imageWidth, imageHeight;
    const char* diffusepath = "res/images/concrete.jpg";
    const char* specularpath = "res/images/container2_specular.png";
    diffuseMap = loadTexture(diffuseMap, diffusepath, imageWidth, imageHeight);
    specularMap = loadTexture(diffuseMap, specularpath, imageWidth, imageHeight);
    
    
    // Set texture units
    lightingShader.Use();
    glUniform1i( glGetUniformLocation( lightingShader.Program, "material.diffuse" ), 0 );
    glUniform1i( glGetUniformLocation( lightingShader.Program, "material.specular" ), 1 );
    GLint lightPosLoc = glGetUniformLocation( lightingShader.Program, "light.position" );
    GLint viewPosLoc = glGetUniformLocation( lightingShader.Program, "viewPos" );
    GLint modelLoc = glGetUniformLocation( lightingShader.Program, "model" );
    GLint viewLoc  = glGetUniformLocation( lightingShader.Program, "view" );
    GLint projLoc  = glGetUniformLocation( lightingShader.Program, "projection" );
    
    
    while ( !glfwWindowShouldClose( window ) )
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glm::mat4 view;
        view = camera.GetViewMatrix();
        
        glfwPollEvents();
        DoMovement();
        
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        
        // Use cooresponding shader when setting uniforms/drawing objects
        lightingShader.Use();
        glUniform3f( lightPosLoc, lightPos.x, lightPos.y, lightPos.z );
        glUniform3f( viewPosLoc,  camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z );
        glUniform3f( glGetUniformLocation( lightingShader.Program, "light.ambient" ), 0.2f, 0.2f, 0.2f );
        glUniform3f( glGetUniformLocation( lightingShader.Program, "light.diffuse" ), 0.5f, 0.5f, 0.5f );
        glUniform3f( glGetUniformLocation( lightingShader.Program, "light.specular" ), 1.0f, 1.0f, 1.0f );
        glUniform1f( glGetUniformLocation( lightingShader.Program, "light.constant" ), 1.0f );
        glUniform1f( glGetUniformLocation( lightingShader.Program, "light.linear" ), 0.09 );
        glUniform1f( glGetUniformLocation( lightingShader.Program, "light.quadratic" ), 0.012 );
        glUniform1f( glGetUniformLocation( lightingShader.Program, "material.shininess"), 32.0f );
        glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
        
        // Bind diffuse map
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseMap );
        // Bind specular map
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, specularMap );
        
        for(int i = 0; i < boxnum; i++)
        {
            glm::mat4 model;
            model = glm::mat4();
            model = glm::translate( model, cubePositions[i] );
            GLfloat angle = 20.0f + currentFrame * 0.2;
            model = glm::rotate(model, angle, glm::vec3( 1.0f, 0.3f, 0.5f ) );
            glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
            
            box.draw();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        quad_program.Use();
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, gAlbedo );
//        glActiveTexture( GL_TEXTURE1 );
//        glBindTexture( GL_TEXTURE_2D, gNormal );
//        glActiveTexture( GL_TEXTURE2 );
//        glBindTexture( GL_TEXTURE_2D, gPosition );
        
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

