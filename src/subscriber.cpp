#include <iostream>
#include <string>
#include <exception>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <future>
#include <mutex>
#include <condition_variable>
extern "C" {
    #include <glad/glad.h>
}
#include <GLFW/glfw3.h>
#define GLM_FORCE_CXX20
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "mqtt/client.h"
#include "mqtt/topic.h"
#include "shader.hpp"
#include "window.hpp"
#include "listener.hpp"
#include "parser.hpp"


using namespace std::chrono_literals;


const std::string SHADER_PATHS[2]{
                                    "../shaders/vertex.txt", //vertex shader
                                    "../shaders/fragment.txt" //fragment shader
                                };

int main(int argc, char** argv)
{

    ArgParser* parser = ArgParser::GetInstance();    
    parser->parse(argc, argv);
    parser->help();

    glm::vec3 view_angles;
    //configure MQTTListener instance
    MQTTListener mqtt_client{parser->getServer(),
                            parser->getClientID(),
                            parser->getTopic(),
                            parser->getQualityLevel()};


    //declare a future object related to MQTT communication
    std::future<bool> mqtt_receiver_listen{std::async(std::launch::async,  
                                                    &MQTTListener::listen,
                                                    &mqtt_client,
                                                    std::ref(view_angles))};

    std::future<bool> mqtt_receiver_setup{std::async(std::launch::async,
                                                    &MQTTListener::setupMQTT,
                                                    &mqtt_client)};

    

    GLFWwindow* window;  
    uint shader_program;
    uint VBO; //vertex buffer object  
    uint VAO; //vertex array object 
    uint EBO; //element buffer object
    
    gl::Window frame{window, 600, 800, "subscriber_window"};
    
    //create Vertex and Fragment shader objects
    VertexShader vertex_shader{SHADER_PATHS[0]};
    FragmentShader fragment_shader{SHADER_PATHS[1]};
    //compile dynamically both shaders
    vertex_shader.compile();
    fragment_shader.compile();
    //create a shader program object 
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader.get());
    glAttachShader(shader_program, fragment_shader.get());
    glLinkProgram(shader_program);
    int shader_program_success;
    char infoLog[512];
    glGetProgramiv(shader_program,GL_LINK_STATUS, &shader_program_success);
    if(!shader_program_success)
    {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cerr<<"The program could not link to the shaders!!!\n";
        std::exit(EXIT_FAILURE);
    }   


    float vertices[] = {
        //positions             //colors
        0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // origin
        0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // origin
        0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // origin
        0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // x
        0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // y
        0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f //z
    }; 
    uint indices[] = {
        0, 3, //first axis 
        1, 4, //second axis
        2, 5 //third axis
    };


    //generate VAO object and bind it first of all
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
   
    //create element buffer object EBO
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    //then bind and set EBO and VBOs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //we have to specify how OpenGL should interpret vertex data.
    //it stores vertex data attributes including vertex buffer array in VAO object currently bound.
    //describe how an element as a vertice should seem
    //position attribute
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);    
    glEnableVertexAttribArray(0); //enable vertex position attribute array- arg is an attribute index
    //color attribute
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float))); //last arg is an offset
    glEnableVertexAttribArray(1); //enable vertex position attribute array- arg is an attribute index
    //offset above become 3*sizeof(float) because for each vertice, first 3 elements are position values. 
    //so color values starts from 4th element for each vertice.  

    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO after glVertexAttribPointer function.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST); 
    std::future_status setup_status_;
    bool listener_result;
    window = frame.get();
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f),glm::vec3(0.0f,1.0f,1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f,0.0f,-3.0f));
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(30.0f), 800.0f/600.0f, 0.1f, 100.0f);
    
    //render loop
    while (!glfwWindowShouldClose(window))
    {
        //check the input key at each iteration
        frame.processInput(window);

        //check asynchronous mqtt communication
        setup_status_ = mqtt_receiver_setup.wait_for(1ms);
        if(setup_status_==std::future_status::ready){
        }
        setup_status_ = mqtt_receiver_listen.wait_for(1ms);
        //rendering commands 
        //at each frame cycle, we need to clear the screen otherwise old colors
        //from old frame will still hold on the viewport
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //we specify which buffer we would like to clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
        
        view = glm::rotate(view, glm::radians(view_angles.y),glm::vec3(0.0f,1.0f,0.0f));

        int modelLoc = glGetUniformLocation(shader_program, "model");
        int viewLoc = glGetUniformLocation(shader_program, "view");
        int projectiionLoc = glGetUniformLocation(shader_program, "projection");
        //recalculate view matrix 
        //view = frame.setCameraViewMatrix();
        //now we are activating newly created program object 
        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectiionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        

        //draw axis lines
        glLineWidth(3);
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);  //take indices into a consideration here. because you have saved indices as EBO.        

       
        //rendering is shown on the display
        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(100ms);
    }
       
    //now we can delete shader program after linking them to program object    
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;

}

