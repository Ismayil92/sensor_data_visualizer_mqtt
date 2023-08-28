#include <iostream>
#include <string>
#include <exception>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <future>
#include <mutex>
#include <condition_variable>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "mqtt/client.h"
#include "mqtt/topic.h"
#include "shader.hpp"
#include "window.hpp"

static std::mutex mx;
static std::condition_variable cv;
static bool ready;

using namespace std::chrono_literals;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"sensor_listener"};
const std::string TOPIC_NAME{"coords"};

const std::string SHADER_PATHS[2]{
                                    "../shaders/vertex.txt", //vertex shader
                                    "../shaders/fragment.txt" //fragment shader
                                };

const int QOS{0};

static void error_callback(int error, const char* description)
{
    std::cout<<description<<std::endl;
}

bool data_handler(const mqtt::message& msg)
{
    const auto now {std::chrono::system_clock::now()};
    const auto t_c = std::chrono::system_clock::to_time_t(now);
    std::cout<<std::ctime(&t_c)<<": "<<msg.to_string()<<std::endl;
    return true;
}




void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        std::cout<<"Exit button pressed!!!\n";
        std::exit(EXIT_SUCCESS);
    }
}


bool setupMQTT(mqtt::client& cli);
bool listen(mqtt::client& cli);


int main(int argc, char** argv)
{
    //create mqtt client object
    mqtt::client cli(SERVER_ADDRESS, CLIENT_ID, 
                    mqtt::create_options(MQTTVERSION_5));

    //declare a future object related to MQTT communication
    std::future<bool> mqtt_receiver_listen{std::async(std::launch::async, 
                    listen, std::ref(cli))};
    std::future<bool> mqtt_receiver_setup{std::async(std::launch::async,
                    setupMQTT, std::ref(cli))};

    

    GLFWwindow* window;  
    uint shader_program;
    uint VBO; //vertex buffer object  
    uint VAO; //vertex array object 
    uint EBO; //element buffer object
    
    gl::Window frame{window, 480, 640, "subscriber_window"};
    
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
        0.5f,  0.5f, 0.0f,      1.0f, 0.0f, 0.0f, // top right
        0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 1.1f  // top left 
    }; 
    uint indices[] = {
        0, 1, 3, //first triangle 
        1, 2, 3  //second triangle
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
    

    std::future_status setup_status_;
    bool listener_result;
    window = frame.get();
    //render loop
    while (!glfwWindowShouldClose(window))
    {
        //check the input key at each iteration
        processInput(window);
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
        glClear(GL_COLOR_BUFFER_BIT);
        //find uniform object location in shader_program
        int vertexColorLocation = glGetUniformLocation(shader_program, "inputcolor");
        //now we are activating newly created program object 
        glUseProgram(shader_program); 
        glBindVertexArray(VAO);
        glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.5f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  //take indices into a consideration here. because you have saved indices as EBO.        
        
        //rendering is shown on the display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
       
    //now we can delete shader program after linking them to program object    
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;

}


bool setupMQTT(mqtt::client& cli)
{
    std::unique_lock<std::mutex> lc_{mx};

    std::cout<<"MQTT Subscriber Client connecting to the MQTT server!!!\n";

    auto connOpts = mqtt::connect_options_builder()
        .keep_alive_interval(20s)
        .automatic_reconnect(2s, 30s)
        .clean_session(false)
        .finalize();

    mqtt::connect_response rsp {cli.connect(connOpts)};
        
    if(!rsp.is_session_present()){
        std::cout<<"Subscribing to topics\n";
            
        cli.subscribe(TOPIC_NAME, QOS);
        std::cout<<"OK\n";
    }
    else
    {
        std::cout<<"Session already established!!!\n";
    }
    lc_.unlock();   
    cv.notify_one();
    ready = true;          
    return ready;
}

bool listen(mqtt::client& cli)
{
    try
    {   
        std::cout<<"MQTT Listener started running\n"<<std::endl;  
        while(true)
        {
            std::unique_lock<std::mutex> lc_{mx};          
            cv.wait(lc_, [&](){return ready;});
            auto msg = cli.consume_message();
            if(msg)            
            {           
                
                if(!data_handler(*msg))
                {
                    break;
                }
                
            }
            else if(!cli.is_connected()){
                std::cout<<"Lost connection\n";
                while(!cli.is_connected())
                {
                    std::this_thread::sleep_for(250ms);
                }
                std::cout<<"Re-established connection\n";
            }
            lc_.unlock();
        }

        // disconnect the connection 
        std::cout<<"Disconnection from the MQTT server...\n";
        cli.disconnect();
        std::cout<<"Connection closed!!!\n";
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
    return true;
}