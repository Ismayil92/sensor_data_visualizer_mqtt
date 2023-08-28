cmake_minimum_required(VERSION 3.10)
project(sensor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)
find_package(Boost 1.82.0 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)




message(STATUS "OpenCV imported to the Project: " ${OpenCV_INCLUDE_DIRS}/opencv2)
message(STATUS "OpenSSL linking directory: " ${OPENSSL_SSL_LIBRARIES})
message(STATUS "GLFW found")

add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)



set(OpenCV_INCLUDE_DIR {OpenCV_INCLUDE_DIRS/opencv2})

add_executable(${PROJECT_NAME}_mqtt_subscriber src/subscriber.cpp src/shader.cpp src/window.cpp)
target_include_directories(${PROJECT_NAME}_mqtt_subscriber PRIVATE 
    include 
    ${PahoMqttCpp_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME}_mqtt_subscriber PUBLIC /usr/local/lib/)

target_link_libraries(${PROJECT_NAME}_mqtt_subscriber Threads::Threads 
        ${OPENSSL_SSL_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARIES}
        paho-mqttpp3
        paho-mqtt3as
        paho-mqtt3a
        paho-mqtt3c
        glfw
        ${OPENGL_LIBS}
        glad)



set(CPACK_PROJECT_NAME ${PROJECT_NAME}_mqtt_subscriber)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION}_mqtt_subscriber)
include(CPack)
