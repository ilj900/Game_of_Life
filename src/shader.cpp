#include <shader.h>
#include <glad.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>

std::vector<shaderProgram> shaderManager::shaderStorage;
unsigned int shaderManager::currentShaderProgram;

int shaderManager::readFromFile(const std::string path, char** src, int *size)
{
    std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        int l_size = in.tellg();
        in.seekg(0, std::ios::beg);
        char* newSrc = new char[l_size+1];
        in.read(newSrc, l_size);
        in.close();
        newSrc[l_size] = '\0';
        *size = l_size;
        *src = newSrc;
        return l_size;
    }
    return 0;
}

void shaderManager::querryGlParam(unsigned int param, std::string explanaition)
{
    int val = 0;
    glGetIntegerv(param, &val);
    std::cout<<explanaition<<val<<std::endl;
}

unsigned int shaderManager::compileShader(const std::string shaderPath, unsigned int type)
{
    char *shaderSrc;
    int size = 0;
    if(!shaderManager::readFromFile(shaderPath, &shaderSrc, &size))
    {
        std::cout<<"Failed to load shader code from path: "<<shaderPath<<std::endl;
        return 0;
    }
    unsigned int shader = glCreateShader(type);
    if (!shader)
    {
        std::cout<<"Failed to create a shader"<<std::endl;
        return 0;
    }
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    free(shaderSrc);
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        if (size)
        {
            char *log = (char *)malloc(size);
            glGetShaderInfoLog(shader, size, &size, log);
            std::cout<<"ERROR::SHADER:: "<<log<<std::endl;
            free(log);
        }
        glDeleteShader(shader);
    }
    return shader;
}

bool shaderManager::addShadervf(std::string vertexPath, std::string fragmentPath, std::string name)
{
    unsigned int vertexShader = shaderManager::compileShader(vertexPath, GL_VERTEX_SHADER);
    if(!vertexShader)
        return false;
    unsigned int fragmentShader = shaderManager::compileShader(fragmentPath, GL_FRAGMENT_SHADER);
    if (!fragmentShader)
    {
        glDeleteShader(vertexShader);
        return false;
    }
    shaderProgram prog;
    prog.ID = glCreateProgram();
    if (!prog.ID)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    glAttachShader(prog.ID, vertexShader);
    glAttachShader(prog.ID, fragmentShader);
    glLinkProgram(prog.ID);
    int status = 0;
    glGetProgramiv(prog.ID, GL_LINK_STATUS, &status);
    if (!status)
    {
        int logSize = 0;
        glGetProgramiv(prog.ID, GL_INFO_LOG_LENGTH, &logSize);
        if (logSize)
        {
            char *log = (char *)malloc(logSize);
            glGetProgramInfoLog(prog.ID, logSize, &logSize, log);
            std::cout<<"ERROR::LINKING:: "<<log<<std::endl;
            free(log);
        }
        glDeleteProgram(prog.ID);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    prog.name = name;
    shaderStorage.push_back(prog);
    return true;
}

bool shaderManager::addShaderc(std::string computePath, std::string name)
{
    unsigned int computeShader = shaderManager::compileShader(computePath, GL_COMPUTE_SHADER);
    if(!computeShader)
        return false;

    shaderProgram prog;
    prog.ID = glCreateProgram();
    if (!prog.ID)
    {
        glDeleteShader(computeShader);
        return false;
    }
    glAttachShader(prog.ID, computeShader);
    glLinkProgram(prog.ID);
    int status = 0;
    glGetProgramiv(prog.ID, GL_LINK_STATUS, &status);
    if (!status)
    {
        int logSize = 0;
        glGetProgramiv(prog.ID, GL_INFO_LOG_LENGTH, &logSize);
        if (logSize)
        {
            char *log = (char *)malloc(logSize);
            glGetProgramInfoLog(prog.ID, logSize, &logSize, log);
            std::cout<<"ERROR::LINKING:: "<<log<<std::endl;
            free(log);
        }
        glDeleteProgram(prog.ID);
        glDeleteShader(computeShader);
        return false;
    }
    glDeleteShader(computeShader);
    prog.name = name;
    shaderStorage.push_back(prog);
    return true;
}

unsigned int shaderManager::setCurrentShaderProgram(std::string name)
{
    for (unsigned int i = 0; i< shaderStorage.size(); i++)
        if (strcmp(name.c_str(), shaderStorage[i].name.c_str()) == 0)
        {
            currentShaderProgram = shaderStorage[i].ID;
            return shaderStorage[i].ID;
        }
    return 0;
}

unsigned int shaderManager::getProgrammId(std::string programName)
{
    for (unsigned int i = 0; i< shaderStorage.size(); i++)
        if (strcmp(programName.c_str(), shaderStorage[i].name.c_str()) == 0)
            return shaderStorage[i].ID;
    return 0;
}

void shaderManager::use()
{
    if (shaderManager::currentShaderProgram != 0)
        glUseProgram(currentShaderProgram);
}

unsigned int shaderManager::getCurrentProgramm()
{
    return currentShaderProgram;
}

unsigned int shaderManager::use(std::string name)
{
    for (unsigned int i = 0; i< shaderStorage.size(); i++)
        if (strcmp(name.c_str(), shaderStorage[i].name.c_str()) == 0)
        {
            glUseProgram(shaderStorage[i].ID);
            return shaderStorage[i].ID;
        }
    return 0;
}

unsigned int shaderManager::setAndUse(std::string name)
{
    for (unsigned int i = 0; i< shaderStorage.size(); i++)
        if (strcmp(name.c_str(), shaderStorage[i].name.c_str()) == 0)
        {
            glUseProgram(shaderStorage[i].ID);
            currentShaderProgram = shaderStorage[i].ID;
            return shaderStorage[i].ID;
        }
    return 0;
}

void shaderManager::setBool(const std::string name, bool value)
{
    if (currentShaderProgram != 0)
        glUniform1i(glGetUniformLocation(currentShaderProgram, name.c_str()), (int)value);
}

void shaderManager::setInt(const std::string name, int value)
{
    if (currentShaderProgram != 0)
        glUniform1i(glGetUniformLocation(currentShaderProgram, name.c_str()), value);
}

void shaderManager::setFloat(const std::string name, float value)
{
    if (currentShaderProgram != 0)
        glUniform1f(glGetUniformLocation(currentShaderProgram, name.c_str()), value);
}

void shaderManager::setMat3(const std::string name, float *values)
{
    if (currentShaderProgram != 0)
        glUniformMatrix3fv(glGetUniformLocation(currentShaderProgram, name.c_str()), 1, GL_FALSE, values);
}

void shaderManager::setMat4(const std::string name, float *values)
{
    if (currentShaderProgram != 0)
        glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, name.c_str()), 1, GL_FALSE, values);
}

void shaderManager::setVec2(const std::string name, float *values)
{
    if (currentShaderProgram != 0)
        glUniform2fv(glGetUniformLocation(currentShaderProgram, name.c_str()), 1, values);
}

void shaderManager::setVec3(const std::string name, float *values)
{
    if (currentShaderProgram != 0)
        glUniform3fv(glGetUniformLocation(currentShaderProgram, name.c_str()), 1, values);
}

void shaderManager::setVec3(const std::string name, float val1, float val2, float val3)
{
    if (currentShaderProgram != 0)
        glUniform3f(glGetUniformLocation(currentShaderProgram, name.c_str()), val1, val2, val3);
}

void shaderManager::setVec4(const std::string name, float *values)
{
    if (currentShaderProgram != 0)
        glUniform4fv(glGetUniformLocation(currentShaderProgram, name.c_str()), 1, values);
}

