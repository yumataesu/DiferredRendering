//
//  Box.hpp
//  01-OpenGLProject
//
//  Created by Yuma Taesu on 2016/11/18.
//  Copyright © 2016年 Yuma Taesu. All rights reserved.
//

#ifndef Box_hpp
#define Box_hpp

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Box
{
private:

    GLuint VBO, boxVAO;
    
public:
    Box();
    ~Box();
    
    void setup();
    void update();
    void draw();
    void release();
};

#endif /* Box_hpp */
