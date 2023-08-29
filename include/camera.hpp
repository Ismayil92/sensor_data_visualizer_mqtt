#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>


class Camera{
    public:
        Camera()
        {
            
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
        static inline glm::vec3 cameraUp{glm::vec3(0.0f, 1.0f, 0.0f)};
        static inline float cameraSpeed{0.05f};
        static inline float deltaTime;
        static inline float lastFrame;
        static inline float currentFrame;

        

};
