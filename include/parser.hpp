#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <cxxopts.hpp>
#include <cstdlib>
#include <mutex>



enum ConnectionType : uint8_t{
    TCP,
    SSL,
    WS,
    WSL
};

class ArgParser{

    protected:
            ArgParser(): options_{"client_parser", "Parsing arguments"}
            {
                options_.add_options()
                ("t, type", "type of the MQTT connection, i.e tcp, ssl, ws, wsl",
                cxxopts::value<std::string>()->default_value("tcp"))
                ("server", "IP address of the remote device",
                cxxopts::value<std::string>()->default_value("127.0.0.1"))
                ("server_port", "port number of the remote device",
                cxxopts::value<uint16_t>()->default_value("1883"))
                ("q, qos", "Quality of service level",
                cxxopts::value<uint8_t>()->default_value("0"))
                ("topic", "the name of the topic at which MQTT broker service runs",
                cxxopts::value<std::string>()->default_value("coords"))
                ("client_id", "name of the client project",
                cxxopts::value<std::string>()->default_value("sensor_listener"))
                ("h,help", "Print usage");
            }
            ~ArgParser()
            {
                delete parser_;
            }

    public:
        ArgParser(ArgParser& _parser) = delete;
        void operator=(const ArgParser&) = delete;
        
        static ArgParser* GetInstance()
        {
            std::lock_guard<std::mutex> lock{mx_};
            if(parser_ == nullptr)
            {
                parser_ = new ArgParser();
            }
            return parser_;
        }
        
        void parse(int argc, char** argv)
        {
            result_ = options_.parse(argc, argv);
            server_ip = result_["server"].as<std::string>();
            server_port = result_["server_port"].as<uint16_t>();
            qos = result_["qos"].as<uint8_t>();
            topic = result_["topic"].as<std::string>();
            cn = result_["type"].as<std::string>();
            client_id = result_["client_id"].as<std::string>();
        }


        void help()
        {
            if (result_.count("help"))
            {
                std::cout << options_.help() << std::endl;
                std::exit(EXIT_SUCCESS);
            }
        }

        std::string getServer() const 
        {
            return cn+"://"+server_ip+":"+std::to_string(server_port);
        }
        
        uint16_t getServerPort() const {return server_port;}
        uint8_t getQualityLevel() const {return qos;}
        std::string getTopic() const {return topic;}
        std::string getConnectionType() const {return cn;}
        std::string getClientID() const {return client_id;}


    private:
            cxxopts::Options options_;
            cxxopts::ParseResult result_;
            std::string server_ip;
            uint16_t server_port;
            uint8_t qos;
            std::string topic;
            std::string cn;
            std::string client_id;       
            static inline ArgParser* parser_{nullptr};
            static std::mutex mx_; 
};


std::mutex ArgParser::mx_;


#endif