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
bool setupEnvironment(int screenWidth, int screenHeight, int monitor, bool fullscreen, bool borderless, int worldWidth, int worldHeight, int fps);
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void querryGlParams();

static int SCREEN_WIDTH = 800;
static int SCREEN_HEIGHT = 600;
static int WORLD_WIDTH = 80;
static int WORLD_HEIGHT = 60;
static int CELL_WIDTH = SCREEN_WIDTH / WORLD_WIDTH;
static int CELL_HEIGHT = SCREEN_HEIGHT / WORLD_HEIGHT;
static bool FULL_SCREEN = false;
static bool BORDER = false;
static float DESIRED_FRAME_LENGTH = 1000.0f/60.0f;

static GLFWmonitor *primeMonitor = NULL;
static GLFWwindow *window = NULL;
static int windowPosX = 0;
static int windowPosY = 0;

static bool processComputeShader = false;
static bool goBackward = false;
static bool goForward = false;

float screenQuad[] = {
    -1.0f, -1.0f,
    1.0f,  1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f
};

int main()
{
    glfwInit();

    bool parametersParsed = setupEnvironment(1200, 800, 0, false, true, 120, 80, 60);
    if (!parametersParsed)
        exit (1);

    /// For now I don't have any parse function, so I'll set everything here

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (BORDER)
        glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (FULL_SCREEN)
        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game of Life", primeMonitor, NULL);
    else
        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game of Life", NULL, NULL);
    if (window == NULL)
    {
        const char *description;
        glfwGetError(&description);
        std::cout<<description<<std::endl;
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }

    if (!FULL_SCREEN)
    {
        glfwSetWindowPos(window, windowPosX, windowPosY);
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

    if (!shaderManager::addShadervf("./res/shaders/simple.vertex.shader", "./res/shaders/simple.fragment.shader", "Draw Shader"))
        return -1;
    if (!shaderManager::addShaderc("./res/shaders/simple.compute.shader", "Compute Shader"))
        return -1;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);

    unsigned int squareVBO, squareVAO;
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), screenQuad, GL_STATIC_DRAW);
    glBindVertexArray(squareVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    float rateOfInitialPopulation = 3;
    unsigned int bufferSize = WORLD_WIDTH * WORLD_HEIGHT * 4;
    unsigned int* initialFrame = new unsigned int[bufferSize]();
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist2(0, rateOfInitialPopulation);
    for(unsigned int i = 0; i < bufferSize; i+=4)
        if (dist2(rng) == rateOfInitialPopulation)
        {
            initialFrame[i] = 1;
            //            initialFrame[i+1] = 0;
            //            initialFrame[i+2] = 0;
            //            initialFrame[i+3] = 0;
        }

    ///Print out buffer of initial frame
    //    for(unsigned int i = 0; i < bufferSize; i+=4)
    //    {
    //        if (i%(WORLD_WIDTH * 4) == 0)
    //            std::cout<<std::endl;
    //        std::cout<<"|"<<initialFrame[i]<<":"<<initialFrame[i+1]<<":"<<initialFrame[i+2]<<":"<<initialFrame[i+3]<<"|";
    //    }
    //    std::cout<<std::endl;

    unsigned int historyLength = 4;
    unsigned int* buffers = new unsigned int[historyLength];
    glGenBuffers(historyLength, buffers);
    for(unsigned int i = 0; i < historyLength; i++)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, buffers[i]);
        if (i==0)
            glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize*4, initialFrame, GL_DYNAMIC_DRAW);
        else
            glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize*4, NULL, GL_DYNAMIC_DRAW);
    }

    /// I need only one texture, and I'll switch it's sourse with glTexBuffer();
    unsigned int mainTexture;
    glGenTextures(1, &mainTexture);
    glBindTexture(GL_TEXTURE_BUFFER, mainTexture);

    static float deltaT = 0.0f;
    static float currentFrame = 0.0f;
    static float previousFrame = 0.0f;
    static int frameOffset = 0;

    static int currentWorld = 0;
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaT = currentFrame - previousFrame;
        previousFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);

        shaderManager::setAndUse("Draw Shader");
        shaderManager::setInt("WORLD_WIDTH", WORLD_WIDTH);
        shaderManager::setInt("WORLD_HEIGHT", WORLD_HEIGHT);
        shaderManager::setInt("WORLD_SIZE", WORLD_WIDTH * WORLD_HEIGHT);
        shaderManager::setInt("CELL_WIDTH", CELL_WIDTH);
        shaderManager::setInt("CELL_HEIGHT", CELL_HEIGHT);
        glBindVertexArray(squareVAO);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32I, buffers[(currentWorld - frameOffset + historyLength)%historyLength]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (goBackward)
        {
            if (frameOffset < (historyLength - 1))
                frameOffset++;
            goBackward = false;
        }
        if (goForward)
        {
            if (frameOffset > 0)
                frameOffset--;
            goForward = false;
        }

        if (processComputeShader)
        {
            frameOffset = 0;
            shaderManager::setAndUse("Compute Shader");
            shaderManager::setInt("WORLD_WIDTH", WORLD_WIDTH);
            shaderManager::setInt("WORLD_HEIGHT", WORLD_HEIGHT);
            for(unsigned int i = 0; i < historyLength; i++)
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, buffers[(i+currentWorld)%historyLength]);
            glDispatchCompute(WORLD_WIDTH, WORLD_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            currentWorld++;
            if (currentWorld == historyLength)
                currentWorld = 0;
            //processComputeShader = false;
        }

        Sleep(40);

        glfwSwapBuffers(window);
        glfwPollEvents();
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
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        processComputeShader = !processComputeShader;
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        goBackward = !goBackward;
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        goForward = !goForward;
    if(key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        glfwGetWindowPos(window, &windowPosX, &windowPosY);
        std::cout<<"xPos: "<<windowPosX<<", yPos: "<<windowPosY<<std::endl;
    }
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

bool setupEnvironment(int screenWidth, int screenHeight, int monitor, bool fullscreen, bool borderless, int worldWidth, int worldHeight, int fps)
{
    /// Let's find the monitor
    int monitorsCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorsCount);
    if (monitorsCount > 0)
    {
        if (monitor < monitorsCount)
            primeMonitor = monitors[monitor];
        else
            primeMonitor = monitors[0];
    } else {
        std::cout<<"I couldn't find any monitors, how do you suppose to use me? You can't even read this message!"<<std::endl;
        return false;
    }

    /// Here we get monitor position and mode
    int monitorPosX, monitorPosY;
    glfwGetMonitorPos(primeMonitor, &monitorPosX, &monitorPosY);
    const GLFWvidmode *mode = glfwGetVideoMode(primeMonitor);

    FULL_SCREEN = fullscreen;
    if (FULL_SCREEN)
    {
        SCREEN_WIDTH = mode->width;
        SCREEN_HEIGHT = mode->height;
    } else
        if (screenWidth > mode->width || screenHeight > mode->height)
        {
            SCREEN_WIDTH = mode->width;
            SCREEN_HEIGHT = mode->height;
        } else {
            SCREEN_WIDTH = screenWidth;
            SCREEN_HEIGHT = screenHeight;
        }
    /// Let's check if screen dimensions fit the world dimensions
    if (SCREEN_WIDTH % worldWidth != 0 || SCREEN_HEIGHT % worldHeight != 0)
    {
        std::cout<<"Desired resolution can not be divided by one of the world dimension without a remainder."<<std::endl;
        return false;
    } else {
        WORLD_WIDTH = worldWidth;
        WORLD_HEIGHT = worldHeight;
    }

    CELL_WIDTH = SCREEN_WIDTH / WORLD_WIDTH;
    CELL_HEIGHT = SCREEN_HEIGHT / WORLD_HEIGHT;

    BORDER = !borderless;
    DESIRED_FRAME_LENGTH = 1000.0f/float(fps);

    if (!fullscreen)
    {
        /// IDK why this formula doesn't work with my setup
        /// This should position the frame directly into the center of the monitor
        /// But it doesn't work with my second monitor
        if (primeMonitor == monitors[0])
        {
            windowPosX = monitorPosX + (mode->width - SCREEN_WIDTH)/2;
            windowPosY = monitorPosY + (mode->height - SCREEN_HEIGHT)/2;
        } else {
            windowPosX = monitorPosX;
            windowPosY = monitorPosY+1; /// To allow users to move the window
        }
    }

    return true;
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
