#include "listener.hpp"
#include <exception>


MQTTListener::MQTTListener(const std::string _address,
                    const std::string _client_id,
                    const std::string _topic_name,
                    const uint8_t _qos):server_address{_address},
                                        client_id{_client_id},
                                        topic_name{_topic_name},
                                        QOS{_qos},
                                        cli{
                                            mqtt::client(server_address, 
                                                        client_id,
                                                        mqtt::create_options(MQTTVERSION_5))
                                        }
{            
    std::cout<<"MQTTListener instance created successfully!!!\n";
}


MQTTListener::~MQTTListener()
{


}


bool MQTTListener::setupMQTT()
{
    std::unique_lock<std::mutex> lc_{mx};
    std::cout<<"MQTTListener client connecting to the MQTT server\n";

    connOpts = mqtt::connect_options_builder()
                            .keep_alive_interval(20s)
                            .automatic_reconnect(2s, 30s)
                            .clean_session(true)
                            .finalize();


    
    mqtt::connect_response rsp{cli.connect(connOpts)};
    
    if(!rsp.is_session_present()){
        std::cout<<"Subscribing to topics\n";            
        cli.subscribe(topic_name, QOS);
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

bool MQTTListener::listen(glm::vec3& view_angles)
{
    try
    {     
        while(true)
        {
            std::unique_lock<std::mutex> lc_{mx};          
            cv.wait(lc_, [&](){return ready;});

            auto msg = cli.consume_message();

            if(msg)            
            {               
                if(!data_handler(*msg, view_angles))
                {
                    throw std::runtime_error("Payload is empty!!!\n");
                }                
            }
            else if(!cli.is_connected())
            {
                std::cout<<"Lost connection\n";
                while(!cli.is_connected())
                {
                    std::this_thread::sleep_for(250ms);
                }
                std::cout<<"Re-established connection\n";
            }
            std::cout<<"----------------------------------\n";
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

glm::vec3 MQTTListener::decodeBuffer(const char* _str)
{
    std::vector<float> vec;   
    std::stringstream ss(_str);
    std::string value_str;
    char delimiter{','};     
    while(!ss.eof())
    {
        getline(ss, value_str, delimiter);
        vec.push_back(stof(value_str));       
    }
    
    return glm::vec3(vec[0],
                        vec[1],
                        vec[2]);
}

bool MQTTListener::data_handler(const mqtt::message& msg, glm::vec3& angles)
{
    const auto now {std::chrono::system_clock::now()};
    const auto t_c = std::chrono::system_clock::to_time_t(now);

    mqtt::binary payload = msg.get_payload();
    
    if(payload.empty()) 
    {
      std::cerr<<"MQTTListener::data_handler: Payload is empty.\n";
      return false;      
    } 

    const char* input = payload.c_str();
    
    try
    {
        angles = decodeBuffer(input);
        std::cout<<std::ctime(&t_c)<<angles[0]<<","<<
                                angles[1]<<","<<
                                angles[2]<<std::endl;
    }
    catch(const std::exception e)    
    {
       std::cerr<<"MQTTListener::data_handler(): Input is not suitable for the vector format!!!\n";
    }
 
    
    return true;
}
