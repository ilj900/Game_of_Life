#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

typedef struct
{
    unsigned int ID;
    std::string name;
}shaderProgram;

class shaderManager
{

public:
    static bool addShadervf(std::string vertexPath, std::string fragmentPath, std::string name);
    static bool addShaderc(std::string computePath, std::string name);
    static unsigned int compileShader(const std::string shaderPath, unsigned int type);
    static int readFromFile(const std::string path, char **src, int *size);
    static unsigned int setCurrentShaderProgram(std::string name);
    static void querryGlParam(unsigned int param, std::string explanaition);
    static void checkForErrors(unsigned int ID);
    static void setBool(const std::string name, bool value);
    static void setInt(const std::string name, int value);
    static void setFloat(const std::string name, float value);
    static void setMat3(const std::string name, float *values);
    static void setMat4(const std::string name, float *values);
    static void setVec2(const std::string name, float *values);
    static void setVec3(const std::string name, float *values);
    static void setVec3(const std::string name, float val1, float val2, float val3);
    static void setVec4(const std::string name, float *values);
    static void use();
    static unsigned int use(std::string name);
    static unsigned int setAndUse(std::string name);
    static unsigned int getCurrentProgramm();
    static unsigned int getProgrammId(std::string programName);

private:
    static std::vector<shaderProgram> shaderStorage;
    static unsigned int currentShaderProgram;
};

#endif
