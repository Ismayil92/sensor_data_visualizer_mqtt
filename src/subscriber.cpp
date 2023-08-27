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


static std::mutex mx;
static std::condition_variable cv;
static bool ready;

using namespace std::chrono_literals;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"sensor_listener"};
const std::string TOPIC_NAME{"coords"};
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



bool setupMQTT(mqtt::client& cli);
bool listen(mqtt::client& cli);


bool setVertexShader(uint& vertex_shader)
{
    int success;
    char infoLog[512];
    const char* vertexShaderSource = "#version 460 core\n"
    "layout(location=0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader,1,&vertexShaderSource, NULL);
    //shader source code must compile dynamically in run-time
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        std::cout<<"ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"<<infoLog<<std::endl;
        return false;
    }
    else
    {
        return true;
    }
}


bool setFragmentShader(uint& fragment_shader)
{
    int success;
    char infoLog[512];
    const char* fragmentShaderSource = "#version 330 core\n"
                                "out vec4 FragColor;\n"
                                "void main()\n"
                                "{\n"
                                "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                "}\0";

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        std::cout<<"ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"<<infoLog<<std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

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

    

    GLFWwindow *window;  
    uint vertex_shader;
    uint fragment_shader;
    uint shader_program;
    uint VBO; //vertex buffer object  
    uint VAO; //vertex array object 
    

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

    //configure and compile shaders
    //in modern OpenGL we are required to define at least  a custom  vertex and fragment shader.
    bool ver_shader_res{setVertexShader(vertex_shader)};
    bool frag_shader_res{setFragmentShader(fragment_shader)};
    //create a shader program object 
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
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
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        }; 
    

    
    //generate VAO object 
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //save vertices in gpu memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    //we have to specify how OpenGL should interpret vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);    

    std::future_status setup_status_;
    bool listener_result;
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
        //now we are activating newly created program object 
        
        glUseProgram(shader_program); 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);         
        
        //rendering is shown on the display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
       
    //now we can delete shader objects after linking them to program object
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteVertexArrays(1, &VAO);
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
    return true;
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