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
bool checkWindow(GLFWwindow *window);
bool initGLEW();
void RenderQuad();

// Camera
Camera  camera(glm::vec3( 0.0f, 0.0f, 10.0f ) );
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


const int boxnum = 400;
const int lightnum = 4;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}

int main()
{
    
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
    
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "Diferred Rendering", nullptr, nullptr );
    checkWindow(window);
    
    glfwMakeContextCurrent( window );
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    glfwSetKeyCallback( window, KeyCallback );
    glfwSetCursorPosCallback( window, MouseCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    initGLEW();
    std::cout << glGetError() << std::endl; //Retreve GLEW always returns 1280
    
    // Positions all boxes
    glm::vec3 cubePositions[boxnum];
    {
        random_device rnd;
        mt19937 mt(rnd());
        std::uniform_int_distribution<> x(-24.0, 24.0);
        std::uniform_int_distribution<> y(-24.0, 24.0);
        std::uniform_int_distribution<> z(-24.0, 24.0);
        for(int i = 0; i < boxnum; i++){ cubePositions[i] = glm::vec3(x(mt), y(mt), z(mt)); }
    }
    
    glm::vec3 cubeSize[boxnum];
    {
        random_device rnd;
        mt19937 mt(rnd());
        std::uniform_int_distribution<> x(1.0, 5.5);
        for(int i = 0; i < boxnum; i++){ cubeSize[i] = glm::vec3(x(mt), x(mt), x(mt)); }
    }
    
    glm::vec3 lightPositions[lightnum];
    {
        random_device rnd;
        mt19937 mt(rnd());
        std::uniform_int_distribution<> x(-35.0, 35.0);
        std::uniform_int_distribution<> y(-35.0, 35.0);
        std::uniform_int_distribution<> z(-35.0, 35.0);
        for(int i = 0; i < lightnum; i++){ lightPositions[i] = glm::vec3(x(mt), y(mt), z(mt)); }
    }
    
    glm::vec3 lightDiffuse[lightnum];
    {
        random_device rnd;
        mt19937 mt(rnd());
        std::uniform_int_distribution<> x(0.0, 1.0);
        std::uniform_int_distribution<> y(0.0, 1.0);
        std::uniform_int_distribution<> z(0.0, 1.0);
        for(int i = 0; i < lightnum; i++){ lightDiffuse[i] = glm::vec3(x(mt), y(mt), z(mt)); }
    }
    
    Box box;
    box.setup();
    
    Box lightbox;
    lightbox.setup();
    
    Rect drawRect;
    drawRect.setup();
    
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    glEnable( GL_DEPTH_TEST );
    
    // Build and compile our shader program
    Shader GeometryPass( "res/shaders/gBuffer.vs", "res/shaders/gBuffer.frag" );
    GLint modelLoc = glGetUniformLocation( GeometryPass.Program, "model" );
    GLint viewLoc  = glGetUniformLocation( GeometryPass.Program, "view" );
    GLint projLoc  = glGetUniformLocation( GeometryPass.Program, "projection" );
    GLint textureLoc = glGetUniformLocation( GeometryPass.Program, "tex" );
    GLint normaltextureLoc = glGetUniformLocation( GeometryPass.Program, "normaltex" );
    
    Shader ssaoPass( "res/shaders/ssao.vs", "res/shaders/ssao.frag" );
    GLint sPositionLoc = glGetUniformLocation(ssaoPass.Program, "gPosition");
    GLint sNormalLoc = glGetUniformLocation(ssaoPass.Program, "gNormal");
    GLint sprojLoc  = glGetUniformLocation(ssaoPass.Program, "projection");
    GLint stexNoiseLoc = glGetUniformLocation(ssaoPass.Program, "texNoise");
    
    Shader ssaoBlurPass( "res/shaders/ssaoBlur.vs", "res/shaders/ssaoBlur.frag" );
    GLint ssaoBlurloc = glGetUniformLocation(ssaoBlurPass.Program, "ssao");

    Shader LightingPass( "res/shaders/lighting.vs", "res/shaders/lighting.frag" );
    GLint gPositionLoc = glGetUniformLocation(LightingPass.Program, "gPosition");
    GLint gNormalLoc = glGetUniformLocation(LightingPass.Program, "gNormal");
    GLint gAlbedoloc = glGetUniformLocation(LightingPass.Program, "gAlbedo");
    //GLint ssaoloc = glGetUniformLocation(LightingPass.Program, "ssao");
    GLint viewPosLoc = glGetUniformLocation(LightingPass.Program, "viewPos");
    GLint lightAmbloc[lightnum], lightDiffloc[lightnum], lightSpecloc[lightnum], lightPosLoc[lightnum];
    for(int i = 0; i < lightnum; i++)
    {
        lightAmbloc[i] = glGetUniformLocation(LightingPass.Program, ("light["+to_string(i)+"].ambient").c_str());
        lightDiffloc[i] = glGetUniformLocation(LightingPass.Program, ("light["+to_string(i)+"].diffuse").c_str());
        lightSpecloc[i] = glGetUniformLocation(LightingPass.Program, ("light["+to_string(i)+"].specular").c_str());
        lightPosLoc[i] = glGetUniformLocation(LightingPass.Program, ("light["+to_string(i)+"].position").c_str());
    }
    
    // Load and create a texture
    GLuint tex;
    
    int width, height;
    // ===================
    // Texture
    // ===================
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    unsigned char *image = SOIL_load_image("res/images/concrete.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap( GL_TEXTURE_2D );
    SOIL_free_image_data( image );
    glBindTexture( GL_TEXTURE_2D, 0 );

    
    // Load and create a texture
    GLuint normaltex;
    int normalwidth, normalheight;
    // ===================
    // Texture
    // ===================
    glGenTextures( 1, &normaltex );
    glBindTexture( GL_TEXTURE_2D, normaltex );
    // Set our texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    // Set texture filtering
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // Load, create texture and generate mipmaps
    unsigned char *normalimage = SOIL_load_image( "res/images/random.png", &normalwidth, &normalheight, 0, SOIL_LOAD_RGBA );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalimage );
    glGenerateMipmap( GL_TEXTURE_2D );
    SOIL_free_image_data( normalimage );
    glBindTexture( GL_TEXTURE_2D, 0 );
    

    
    GLuint gBUffer;
    glGenFramebuffers(1, &gBUffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBUffer);
    
    GLuint gPosition, gNormal, gAlbedo;
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
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
    

    
    
    // Also create framebuffer to hold SSAO processing stage
    GLuint ssaoFbo, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFbo); glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
    GLuint  ssaoColorBuffer, ssaoColorBufferBlur;
    
    // - SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    
    // Sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;
        
        // Scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    
    // Noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    GLuint noiseTexture; glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Loop
    while ( !glfwWindowShouldClose( window ) )
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        
        glfwPollEvents();
        DoMovement();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view = camera.GetViewMatrix();
        

        //1.Geometry Pass
        glBindFramebuffer(GL_FRAMEBUFFER, gBUffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        GeometryPass.Use();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(textureLoc, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normaltex);
        glUniform1i(normaltextureLoc, 1);

        for(int i = 0; i < boxnum; i++)
        {
            //cubePositions[i].x += 0.05;
            glm::mat4 model = glm::mat4();
            model = glm::translate(model, cubePositions[i]);
            //GLfloat angle = 20.0f + currentFrame * 0.3;
            //model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, cubeSize[i]);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            box.draw();
            
            //if(cubePositions[i].x > 30) cubePositions[i].x = -30;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        //2.SSAO Pass
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
            glClear(GL_COLOR_BUFFER_BIT);
            ssaoPass.Use();
            for(int i = 0; i < 64; i++)
            {
                glUniform3fv(glGetUniformLocation(ssaoPass.Program, ("samples["+to_string(i)+"]").c_str()), 1, &ssaoKernel[i][0]);
            }
            glUniformMatrix4fv(sprojLoc, 1, GL_FALSE, glm::value_ptr(projection));
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glUniform1i(sPositionLoc, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glUniform1i(sNormalLoc, 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            glUniform1i(stexNoiseLoc, 2);
            
            drawRect.draw();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        
        // 3.5. Processing Blur
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            ssaoBlurPass.Use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            glUniform1i(ssaoBlurloc, 0);
            
            drawRect.draw();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        
        // 4.Lighting Pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LightingPass.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glUniform1i(gPositionLoc, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glUniform1i(gNormalLoc, 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glUniform1i(gAlbedoloc, 2);
        
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        glUniform1i(glGetUniformLocation(LightingPass.Program, "ssao"), 3);
        
        
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        
        glm::vec3 lightAmbient = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 lightSpecular = glm::vec3(1.0, 1.0, 1.0);
        glm::vec3 lightPosView[lightnum];
        for(int i = 0; i < lightnum; i++)
        {
            lightPosView[i] = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPositions[i], 1.0));
            
            glUniform3fv(lightPosLoc[i], 1, &lightPosView[i][0]);
            glUniform3fv(lightAmbloc[i], 1, glm::value_ptr(lightAmbient));
            glUniform3fv(lightDiffloc[i], 1, &lightDiffuse[i][0]);
            glUniform3fv(lightSpecloc[i], 1, glm::value_ptr(lightSpecular));
        }
        
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.constant" ), 1.2 );
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.linear" ), 0.19 );
        glUniform1f( glGetUniformLocation( LightingPass.Program, "light.quadratic" ), 0.001 );
        
        drawRect.draw();
        
//
        // 2.5. Copy content of geometry's depth buffer to default framebuffer's depth buffer
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBUffer);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
//        glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    box.release();
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gAlbedo);
    glDeleteFramebuffers(1, &gBUffer);
    glDeleteFramebuffers(1, &ssaoFbo);
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate( );
    
    return 0;
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

