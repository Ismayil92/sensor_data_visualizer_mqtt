#include <iostream>
#include <string>
#include <exception>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <future>
#include "mqtt/client.h"
#include "mqtt/topic.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>




using namespace std::chrono_literals;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"sensor_listener"};

static void error_callback(int error, const char* description)
{
    std::cout<<description<<std::endl;
}

bool data_handler(const mqtt::message& msg)
{
    std::cout<< msg.to_string()<<std::endl;
    return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
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

void listen()
{

}

void setupMQTT()
{
    const std::string TOPIC_NAME{"coords"};
    const int QOS{0};

    std::cout<<"MQTT Subscriber Client connecting to the MQTT server!!!\n";
    mqtt::client cli(SERVER_ADDRESS, CLIENT_ID, 
        mqtt::create_options(MQTTVERSION_5));

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
        std::cout<<"Session already present\n";
    }

    try
    {        
        while(true)
        {
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
}


int main(int argc, char** argv)
{

    std::future<void> async_thr_mqtt_receiver{std::async(std::launch::async,
                    setupMQTT)};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // initialize GLFW
    if (!glfwInit())
    {
        std::cerr<<"GLFW failed to initialize!!!\n";
        return -1;
    }
    else
    {
        std::cout<<"GL initialized successfully!!!\n";
    }
    
    GLFWwindow *window;
   
    glfwSetErrorCallback(error_callback);
    
    window = glfwCreateWindow(640,480, "Coordinate axes", NULL, NULL);
    if(!window)
    {
        std::cerr<<"Window or OpenGL context creation failed\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    //create context of the rendering window in the main thread
    glfwMakeContextCurrent(window);
    //load all HW and system specific GL functions
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr<<"Failed to initialize GLAD\n";
        std::exit(EXIT_FAILURE);
    }

    //tell OpenGL the size of the rendering window,
    //so OpenGL knows how we want to display the data and coordinates
    //wrt the window. 
    glViewport(0,0, 640, 800);
    //mount callback function to the event of window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    
    glfwSwapInterval(1);
    //render loop
    while (!glfwWindowShouldClose(window))
    {
        //check the input key at each iteration
        processInput(window);

        //rendering commands 
        //at each frame cycle, we need to clear the screen otherwise old colors
        //from old frame will still hold on the viewport
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //we specify which buffer we would like to clear
        glClear(GL_COLOR_BUFFER_BIT);

        
        //rendering is shown on the display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
   

    

    glfwTerminate();
    return 0;

}