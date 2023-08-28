#include <cstdlib>
#include <iostream>
#include "window.hpp"

using namespace gl;


Window::Window(GLFWwindow* window_, const uint height_, const uint width_, const std::string window_name_):
                                                                window{window_}, 
                                                                height{height_},
                                                                width{width_}
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->init();
    glfwSetErrorCallback(Window::error_callback);
    this->createWindow(window_name_);
    //create context of the rendering window in the main thread
    glfwMakeContextCurrent(window);
    //load all HW and system specific GL functions
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr<<"Failed to initialize GLAD\n";
        std::exit(EXIT_FAILURE);
    }
    this->setViewPort();
    //mount callback function to the event of window resize
    glfwSetFramebufferSizeCallback(window, Window::framebuffer_size_callback);    
    glfwSwapInterval(1);  
}

Window::~Window()
{

}

void Window::init()
{
    // initialize GLFW
    if (!glfwInit())
    {
        std::cerr<<"GLFW failed to initialize!!!\n";
        return std::exit(EXIT_FAILURE);
    }    
    std::cout<<"GL initialized successfully!!!\n";
}


void Window::createWindow(const std::string window_name)
{
    window = glfwCreateWindow(width, height, window_name.c_str(), NULL, NULL);
    if(!window)
    {
        std::cerr<<"Window or OpenGL context creation failed\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    std::cout<<"Window:\""<<window_name<<"\" created successfully!!!\n";
}

void Window::setViewPort()
{
    //tell OpenGL the size of the rendering window,
    //so OpenGL knows how we want to display the data and coordinates
    //wrt the window. 
    glViewport(0,0, width, height);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
}

void Window::error_callback(int error, const char* description)
{
    std::cout<<description<<std::endl;
}