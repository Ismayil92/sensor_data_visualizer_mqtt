find_package(Threads REQUIRED)
find_package(Boost 1.82.0 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(OpenSSL REQUIRED)


message(STATUS "OpenCV imported to the Project: " ${OpenCV_INCLUDE_DIRS}/opencv2)
message(STATUS "OpenSSL linking directory: " ${OPENSSL_SSL_LIBRARIES})
set(OpenCV_INCLUDE_DIR {OpenCV_INCLUDE_DIRS/opencv2})

add_executable(${PROJECT_NAME}_mqtt_subscriber src/main.cpp)
target_include_directories(${PROJECT_NAME}_mqtt_subscriber PRIVATE 
    include 
    ${PahoMqttCpp_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME}_mqtt_subscriber PUBLIC /usr/local/lib/)
target_link_libraries(${PROJECT_NAME}_mqtt_subscriber Threads::Threads 
        ${OPENSSL_SSL_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARIES}
        paho-mqttpp3
        paho-mqtt3as
        paho-mqtt3a
        paho-mqtt3c)


set(CPACK_PROJECT_NAME ${PROJECT_NAME}_mqtt_subscriber)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION}_mqtt_subscriber)
include(CPack)
