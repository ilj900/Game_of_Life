#include <iostream>
#include <string>
#include <math.h>
#include <glad.h>
#include <glfw3.h>
#include <shader.h>
#include <random>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
bool parseParameters(unsigned int nrOfParameters, char** parameters);
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void querryGlParams();
GLFWmonitor* getMonitor();

static unsigned int SCREEN_WIDTH = 800;
static unsigned int SCREEN_HEIGHT = 600;
static int monitor = 1;
static bool vSync = true;
static float desiredFrameLength = 1000.0f/60.0f;

float screenQuad[] = {
    -1.0f, -1.0f,
    1.0f,  1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f
};

/// Available arguments to pass are:
/// -r resolution in format width height, so if you want to set 1920 x 1080 you have to pass "-r 1920 1080"
/// -w world size. "-w 192 108" will create world with width 192 cells and height 108. With resoluton 1920x1080 that means every cell takes 10 pixels
/// -s allows you to choose the screen. 0 stands for main monitor, 1, 2, 3, etc for all other. "-s 1" means you will use your second monitor, if you have one
/// -f set's fps, like "-f 60". If not set fps will be unlocked.
/// No more than 10 args in total for now
int main(int arg, char** args)
{
    bool parametersParsed = parseParameters(arg, args);

    glfwInit();

    GLFWmonitor *bestMonitor = getMonitor();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpwnGL", bestMonitor, NULL);
    if (window == NULL)
    {
        const char *description;
        glfwGetError(&description);
        std::cout<<description<<std::endl;
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout <<"Failed to initialize GLAD"<<std::endl;
        return -1;
    }
    querryGlParams();
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!shaderManager::addShadervf("./res/shaders/simple.vertex.shader", "./res/shaders/simple.fragment.shader", "Draw Shader"))
        return -1;
    if (!shaderManager::addShaderc("./res/shaders/simple.compute.shader", "Compute Shader"))
        return -1;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    unsigned int squareVBO, squareVAO;
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), screenQuad, GL_STATIC_DRAW);
    glBindVertexArray(squareVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    float rateOfInitialPopulation = 10;
    unsigned int fieldWidth = 384;
    unsigned int fieldHeight = 216;
    unsigned int bufferSize = fieldWidth * fieldHeight * 4; /// I have to use 4 components because there are no such thing as GL_RGB8UI for GL_TEXTURE_BUFFER
    unsigned char* initialFrame = new unsigned char[bufferSize]();
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist2(0, rateOfInitialPopulation);
    for(unsigned int i = 0; i < bufferSize; i+=4)
        if (dist2(rng) == rateOfInitialPopulation)
            initialFrame[i] = 255;
        else
            initialFrame[i] = 0;

    unsigned int nrOfFrames = 2;
    unsigned int* buffers = new unsigned int[nrOfFrames];
    glGenBuffers(nrOfFrames, buffers);
    for(unsigned int i = 0; i < nrOfFrames; i++)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, buffers[i]);
        if (i==0)
            glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, initialFrame, GL_DYNAMIC_DRAW);
        else
            glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
    }
    unsigned int *textures = new unsigned int[nrOfFrames];
    glGenTextures(nrOfFrames, textures);
    for (unsigned int i = 0; i < nrOfFrames; i++)
    {
        glBindTexture(GL_TEXTURE_BUFFER, textures[i]);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, buffers[i]);
    }


    static float deltaT = 0.0f;
    static float currentFrame = 0.0f;
    static float previousFrame = 0.0f;

    shaderManager::setAndUse("Draw Shader");
    shaderManager::setInt("sceneTexture", 0);

    unsigned int textureToDisplay = 0;
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaT = currentFrame - previousFrame;
        previousFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);

        shaderManager::setAndUse("Draw Shader");
        shaderManager::setInt("fieldWidth", fieldWidth);
        shaderManager::setInt("fieldHeight", fieldHeight);
        shaderManager::setInt("cellWidth", 10);
        shaderManager::setInt("cellHeight", 10);
        glBindVertexArray(squareVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, textures[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        textureToDisplay++;
        if (textureToDisplay == nrOfFrames)
            textureToDisplay = 0;
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    static float lastInput = 0.0f;
    static float currentInput = 0.0f;

    currentInput = glfwGetTime();
    float deltaT = currentInput - lastInput;
    lastInput = currentInput;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static double mouseSensitivity = 0.0005f;
    static double previosPosX = xpos;
    static double previosPosY = ypos;

    float deltax = (previosPosX - xpos)*mouseSensitivity;
    previosPosX = xpos;
    float deltay = (previosPosY - ypos)*mouseSensitivity;
    previosPosY = ypos;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
}

void querryGlParams()
{
    unsigned int params[] = {GL_MAX_VERTEX_ATTRIBS, GL_MAX_UNIFORM_BLOCK_SIZE, GL_MAX_TEXTURE_SIZE};
    std::string explanations[] = {  "Maximum number of vertex attribures supported: ",
                                    "Maximum uniform block size supported: ",
                                    "Maximum texture size is: "};
    for (unsigned int i = 0; i < sizeof(params)/sizeof(int); i++)
        shaderManager::querryGlParam(params[i], explanations[i]);
}

GLFWmonitor* getMonitor()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    GLFWmonitor* chosenMonitor;
    if (monitor < count)
        chosenMonitor = monitors[monitor];
    else
        chosenMonitor = monitors[count-1];
    const GLFWvidmode *mode = glfwGetVideoMode(chosenMonitor);
    SCREEN_WIDTH = mode->width;
    SCREEN_HEIGHT = mode->height;
    if (vSync)
        desiredFrameLength = 1000.0/mode->refreshRate;
    return chosenMonitor;
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

bool parseParameters(unsigned int nrOfParameters, char** parameters)
{
    return true;
}
