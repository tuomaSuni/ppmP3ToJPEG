#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

struct PPMImage {
    int width = 0;
    int height = 0; 
    std::vector<unsigned char> data;
};


PPMImage loadPPM(const std::string& filename);
void initOpenGL();
GLuint createTexture(const PPMImage& img);
std::string readShaderSource(const std::string& filename);

void view();