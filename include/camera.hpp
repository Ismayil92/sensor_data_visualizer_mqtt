#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>


class Camera{
    public:
        Camera()
        {
            cameraDirection = glm::normalize(cameraPos-cameraTarget);
            glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);
            cameraRight = glm::normalize(glm::cross(up, cameraDirection));
            cameraUp = glm::cross(cameraDirection, cameraRight);
        }
        ~Camera(){
            std::cout<<"Camera object deleted!!!\n";
        }

        glm::mat4 setCameraViewMatrix()
        {
            return glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);
        }
       
    protected:
        static inline glm::vec3 cameraPos{glm::vec3(0.0f,  0.0f, 3.0f)};
        static inline glm::vec3 cameraFront{glm::vec3(0.0f, 0.0f, -1.0f)};
        static inline glm::vec3 cameraUp;
        static inline glm::vec3 cameraTarget{glm::vec3(0.0f, 0.0f, 0.0f)};
        static inline glm::vec3 cameraDirection; 
        static inline glm::vec3 cameraRight;

        static inline float cameraSpeed{0.05f};
        static inline float deltaTime;
        static inline float lastFrame;
        static inline float currentFrame;
        static inline float yaw{-90.0f};
        static inline float pitch{0.0f};
        

        

};
