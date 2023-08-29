#ifndef WINDOW_H
#define WINDOW_H

extern "C"{
    #include <glad/glad.h>
}
#include <GLFW/glfw3.h>
#include <string_view>
#include "camera.hpp"

namespace gl{
    class Window: public Camera{
        public:
            Window(GLFWwindow*, const uint,const uint, const std::string);
            ~Window();
            void init();
            void createWindow(const std::string window_name);
            void setViewPort();
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
            static void error_callback(int error, const char* description);
            static void processInput(GLFWwindow *window);
            GLFWwindow* get() const {return window;} 


        private:
            uint height;
            uint width;
            GLFWwindow* window;
    };
}
#endif