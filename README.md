# Sensor MQTT Project

In this project, I have tried to establish publisher/subscriber connection via MQTT protocol in IoT devices. 
Mosquito MQTT broker was installed on Ubuntu environment and orchestrate all data transfer.
MQTT publisher and subscriber nodes has been written using Eclipse Paho C++ client library.  
The Subscriber node listens to  the channel asynchronously and can rely on  different protocols such as "tcp", "websocket", "SSL/TSL".

The aim in the project to fetch data from gyro sensor and send over the network to visualizer which is located in the Subscriber node. 

GLFW library has been used to visualize simulatenously motion of the sensor. 
 
The project is still in Progress!!!