#ifndef SHADER_H
#define SHADER_H
extern "C"{
    #include <glad/glad.h> // include glad to get all the required OpenGL headers
}
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


static enum SHADER_STAGE {
    VERTEX_SHADER,
    FRAGMENT_SHADER
} shader_type; 

class BaseShader{
    public:
        // the program ID
        uint ID;
        //constructor
        explicit BaseShader(const std::string shader_path, 
                        SHADER_STAGE shader_);
        virtual ~BaseShader();
        //use/activate the shader
        //configure and compile shaders
        //in modern OpenGL we are required to define at least  a custom  vertex and fragment shader.
        virtual bool compile() = 0; 
        //utility functions
        virtual const uint& get() const = 0;
        virtual void setBool(const uint& ID,const std::string& name,bool value)  = 0;
        virtual void setInt(const uint& ID,const std::string& name,int value)  = 0;
        virtual void setFloat(const uint& ID,const std::string& name,float value)  = 0;
    protected:
        int success;
        char infoLog[512];
        uint shader;
        std::string code;
        
};

class VertexShader : public BaseShader
{
    public:        
        explicit VertexShader(const std::string vertex_path);
        virtual ~VertexShader();
        bool compile() override;
        const uint& get() const override {return shader;}
        void setBool(const uint& ID,const std::string& name,bool value)  override;
        void setInt(const uint& ID,const std::string& name,int value)  override;
        void setFloat(const uint& ID,const std::string& name,float value)  override;
};

class FragmentShader : public BaseShader
{
    public:
        
        explicit FragmentShader(const std::string fragment_path);
        virtual ~FragmentShader();
        bool compile() override;
        const uint& get() const override{return shader;}
        void setBool(const uint& ID,const std::string& name,bool value)  override;
        void setInt(const uint& ID,const std::string& name,int value)  override;
        void setFloat(const uint& ID,const std::string& name,float value)  override;
};



#endif