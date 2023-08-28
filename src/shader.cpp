#include "shader.hpp"


BaseShader::BaseShader(const std::string shader_path,
                        SHADER_STAGE shader_)
{
    std::ifstream shader_file;
    // ensure ifstream object can throw exceptions in case of failbit and badbit
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        //open file
        shader_file.open(shader_path, std::ifstream::in);
        std::stringstream shader_stream;
        if(shader_file.is_open())
        {
            shader_stream << shader_file.rdbuf();
        }
        //close file handler
        shader_file.close();
        //convert buffer stream into string 
        code = shader_stream.str();
        if(!code.empty())
        {
            std::cout<<"SHADER::"<<
                        ((shader_==VERTEX_SHADER)?"VERTEX_SHADER":"FRAGMENT_SHADER")<<
                        "_FILE_SUCCESSFULLY_READ\n";
        }
    }
    catch(const std::ifstream::failure& e)
    {
        std::cerr << "ERROR::"<<
                    ((shader_==VERTEX_SHADER)?"VERTEX_SHADER":"FRAGMENT_SHADER")<<
                    "_FILE_NOT_READ\n";
    } 
}

BaseShader::~BaseShader()
{
    glDeleteShader(shader);
}



VertexShader::VertexShader(const std::string vertex_path):
                                BaseShader{vertex_path, VERTEX_SHADER}
{
    std::cout<<"Vertex Shader constructor created!!!\n";
}

VertexShader::~VertexShader()
{

}

bool VertexShader::compile()
{
    shader = glCreateShader(GL_VERTEX_SHADER);
    const char* ccode = code.c_str();
    glShaderSource(shader,1,&ccode,NULL);
    //shader source code must compile dynamically in run-time
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout<<"ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"<<infoLog<<std::endl;
        return false;
    }     
    return true;   
}


void VertexShader::setBool(const uint& ID, const std::string& name, bool value) 
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void VertexShader::setInt(const uint& ID, const std::string& name, int value) 
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void VertexShader::setFloat(const uint& ID, const std::string& name, float value)   
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}


FragmentShader::FragmentShader(const std::string fragment_path):
                                    BaseShader{fragment_path, FRAGMENT_SHADER}
{
    std::cout<<"Fragment Shader constructor created!!!\n";
}

FragmentShader::~FragmentShader()
{

}
bool FragmentShader::compile()
{
    shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* ccode = code.c_str();
    glShaderSource(shader,1,&ccode,NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout<<"ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"<<infoLog<<std::endl;
        return false;
    }
    return true;
}

void FragmentShader::setBool(const uint& ID, const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void FragmentShader::setInt(const uint& ID, const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void FragmentShader::setFloat(const uint& ID, const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}



