#include <iostream>
#include <string>
#include <exception>
#include <chrono>
#include <vector>
#include <cstdlib>
#include "mqtt/client.h"
#include "mqtt/topic.h"


using namespace std::chrono_literals;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"sensor_listener"};



bool data_handler(const mqtt::message& msg)
{
    std::cout<< msg.to_string()<<std::endl;
    return true;
}
int main(int argc, char** argv)
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

    try
    {  
        
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
    return 0;

}