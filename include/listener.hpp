#ifndef LISTENER_H
#define LISTENER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <future>
#include <mutex>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mqtt/client.h"
#include "mqtt/topic.h"

using namespace std::chrono_literals;

class MQTTListener{
    private:
        std::mutex mx;
        std::condition_variable cv;
        bool ready;
        uint8_t QOS;
        const std::string client_id;
        const std::string server_address;
        std::string topic_name;
        mqtt::client cli;
        mqtt::connect_options connOpts;
    
    public:
        MQTTListener(const std::string _address,
                    const std::string _client_id,
                    const std::string _topic_name,
                    const uint8_t _qos);

        ~MQTTListener();

        glm::vec3 decodeBuffer(const char* _str);
        bool data_handler(const mqtt::message& _msg, glm::vec3& _angles);
        bool setupMQTT();
        bool listen(glm::vec3& _angles);

};

#endif
