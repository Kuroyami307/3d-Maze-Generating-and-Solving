#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For transformations like translate, rotate, scale
#include <glm/gtc/type_ptr.hpp>

#include "custom/game.h"
#include "custom/shader.h"


#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h> 
#endif

const int ROW = 16;
const int GRIDSIZE = ROW * ROW;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void glfwErrorCallback(int error, const char* description);

unsigned int SCR_WIDTH = 700;
unsigned int SCR_HEIGHT = 700;

glm::vec3 cameraPos(0.0f, 0.0f, 420.0f), targetPos(0.0f), cameraUp(0.0f, 1.0f, 0.0f);
light lightSource = {glm::vec3(0.0f, 0.0f, 200.0f), glm::vec4(1.0f), 1.0f};
glm::mat4 rotMat(1.0f);

// Forward declare classes
class cell;
class grid;

class cell {
public:
    bool hasWall[4]; // 0-top 1-bottom 2-left 3-right
    bool visited;
    obstacle walls[4];
    ground base;
    float startTime;

    cell()
    {
        initialize();
    }

    void initialize()
    {
        visited = false;
        startTime = -1;
        
        for(int i = 0; i < 4; i++)
        {
            hasWall[i] = true;
            walls[i].setCollisionStatus(true);
        }
    }

    void initializeWalls(shader *mainShader)
    {
        base.block3D(40, 40, 4);
        base.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)));
        base.setShader(mainShader);
        
        for (int i = 0; i < 4; i++) {
            walls[i].setShader(mainShader);
        }
        walls[0].block3D(45, 5, 30);
        walls[0].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 15.0f)));

        walls[1].block3D(45, 5, 30);
        walls[1].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, 15.0f)));

        walls[2].block3D(5, 45, 30);
        walls[2].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 15.0f)));

        walls[3].block3D(5, 45, 30);
        walls[3].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, 15.0f)));
    }

    void draw()
    {
        base.setUniform("startTime", startTime);

        for (int i = 0; i < 4; i++) {
            if (hasWall[i]) walls[i].draw(lightSource, cameraPos);
        }
        base.draw(lightSource, cameraPos);
    }

    void setPosition(glm::vec3 position)
    {
        for (int i = 0; i < 4; i++) {
            walls[i].setPosition(position);
        }
        base.setPosition(position);
    }

    void checkCollision(player *pPtr)
    {
        // gameObject* objPtr = &walls[0];
        for(int i = 0; i < 4; i++)
        {
            if(hasWall[i]) pPtr->collision(&walls[i]);
        }
        // pPtr->collision(&base);
    }

    void removeWall(int index)
    {
        hasWall[index] = false;
        walls[index].setCollisionStatus(false);
    }
};

class grid {
private:
    cell blocks[GRIDSIZE];
    cell endBlock;

    std::stack<int> visitedCells;
    float generationCompleteTime = -1.0;
    bool isGenerating = true;
    bool isCompleted = true;

    int startBlockNum;

public:
    grid(shader *mainShader)
    {   
        srand((unsigned int)glfwGetTime());
        // startBlockNum = rand() % GRIDSIZE;
        int i, j;
        for (int k = 0; k < GRIDSIZE; k++) {
            blocks[k].initializeWalls(mainShader);

            i = k / ROW; // row
            j = k % ROW; // column

            blocks[k].setPosition(glm::vec3((float)j * 40 - (ROW/2 * 40 - 20), -(float)i * 40 + (ROW/2 * 40 - 20), 0.0f));
        }
        endBlock.initializeWalls(mainShader);
        endBlock.setPosition(glm::vec3((float)ROW * 40 - (ROW/2 * 40 - 20), -(float)(ROW-1) * 40 + (ROW/2 * 40 - 20), 0.0f));
        endBlock.removeWall(2);
        endBlock.base.setColor(glm::vec4(0.1f, 0.8f, 0.5f, 1.0f));
    }

    void resetGrid(shader *mainShader)
    {
        srand((unsigned int)glfwGetTime());
        // startBlockNum = rand() % GRIDSIZE;

        for (int k = 0; k < GRIDSIZE; k++) {
            blocks[k].initialize();
        }
    }

    void removeWall(int block_num, int index) // index: 0-top 1-bottom 2-left 3-right
    {
        int i = block_num / ROW; // xrow
        int j = block_num % ROW; // column

        blocks[block_num].hasWall[index] = false;
        blocks[block_num].walls[index].setCollisionStatus(false);

        switch(index)
        {
            case 0:
                if(i > 0)
                {
                    blocks[(i-1) * ROW + j].hasWall[1] = false;
                    blocks[(i-1) * ROW + j].walls[1].setCollisionStatus(false);
                }
                break;

            case 1:
                if(i < ROW - 1)
                {
                    blocks[(i + 1) * ROW + j].hasWall[0] = false;
                    blocks[(i + 1) * ROW + j].walls[0].setCollisionStatus(false);
                }
                break;

            case 2:
                if(j > 0)
                {
                    blocks[i * ROW + j - 1].hasWall[3] = false;
                    blocks[i * ROW + j - 1].walls[3].setCollisionStatus(false);
                }
                break;

            case 3:
                if(j < ROW - 1)
                {
                    blocks[i * ROW + j + 1].hasWall[2] = false;
                    blocks[i * ROW + j + 1].walls[2].setCollisionStatus(false);
                }
                break;
        }
    }

    void draw()
    {
        for(int i = 0; i < GRIDSIZE; i++)
        {
            blocks[i].draw();
        }

        if(!isGenerating) endBlock.draw();
    }

    bool hasUnvisitedNeighbours(int block_num)
    {
        int i = block_num / ROW;
        int j = block_num % ROW;

        if(i > 0) if(!blocks[(i - 1) * ROW + j].visited) return true;
        if(i < ROW - 1) if(!blocks[(i + 1) * ROW + j].visited) return true;
        if(j > 0) if(!blocks[i * ROW + (j - 1)].visited) return true;
        if(j < ROW - 1) if(!blocks[i * ROW + (j + 1)].visited) return true;

        return false;
    }

    void generateMaze(int block = -1)
    {
        static int block_num = rand() % GRIDSIZE;
        int next_block = -1;

            // draw();
        blocks[block_num].visited = true;
        blocks[block_num].startTime = glfwGetTime();

        visitedCells.push(block_num);

        if(hasUnvisitedNeighbours(block_num))
        {
            int direction;
            while(true)
            {
                int i = block_num / ROW, j = block_num % ROW;
                direction = rand() % 4;

                next_block = -1;
                switch(direction)
                {
                    case 0:
                        if(i > 0) next_block = (i - 1) * ROW + j;
                        break;

                    case 1:
                        if(i < ROW - 1) next_block = (i + 1) * ROW + j;
                        break;

                    case 2:
                        if(j > 0) next_block = i * ROW + (j - 1);
                        break;

                    case 3:
                        if(j < ROW - 1) next_block = i * ROW + (j + 1);
                        break;
                }
                if(next_block == -1) continue;

                if(blocks[next_block].visited == false) break;
            }

            removeWall(block_num, direction);
            block_num = next_block;
            // recursiveBacktrack(next_block);
        }
        else
        {
            while(!visitedCells.empty())
            {
                next_block = visitedCells.top();
                visitedCells.pop();

                if(hasUnvisitedNeighbours(next_block))
                {
                    // recursiveBacktrack(next_block);
                    block_num = next_block;
                    break;
                }
            }
        }
        
        if(!hasUnvisitedNeighbours(block_num) && visitedCells.empty())
        {
            isGenerating = false;
            generationCompleteTime = glfwGetTime();
            blocks[GRIDSIZE - 1].removeWall(3);
        }
    }

    void setGenerationStatus(bool status)
    {
        isGenerating = status;
    }

    bool getGenerationStatus()
    {
        return isGenerating;
    }

    float getGenerationCompleteTime()
    {
        return generationCompleteTime;
    }

    void checkCollision(player* pPtr)
    {
        for(int i = 0; i < GRIDSIZE; i++)
        {
            blocks[i].checkCollision(pPtr);
        }
        endBlock.checkCollision(pPtr);
    }

    bool isMazeComplete(player* pPtr)
    {
        float x = pPtr->getPosition().x - endBlock.base.getPosition().x;
        float y = pPtr->getPosition().y - endBlock.base.getPosition().y;

        if(abs(x) <= 10.0f && abs(y) <= 10.0f) isCompleted = true;
        else isCompleted = false;
        
        return isCompleted;
    }

    void setCompleteStatus(bool status)
    {
        isCompleted = status;
    }
};

// Globals needed for emscripten main loop
GLFWwindow* window = nullptr;
grid* gPtr = nullptr;
player* pPtr = nullptr;
shader *shaderPtr = nullptr;


void main_loop()
{
    static float lastFrame = 0.0f;
    static float deltaTime = 0.0f; 
    static int i = 0;

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    shaderPtr->setFloat("time", currentFrame);

    if (!window)
        return;

    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(gPtr)
    {
        gPtr->draw();
        if(pPtr && !gPtr->getGenerationStatus())
        {
            if(currentFrame - gPtr->getGenerationCompleteTime() >= 2.0)
            {
                pPtr->movements(window);
                // pPtr->updatePhysics(deltaTime);
                pPtr->draw(lightSource, cameraPos);
                gPtr->checkCollision(pPtr);
            }
        }
    }

    if(gPtr->getGenerationStatus())
    {
        if(i == 3)
        {
            gPtr -> generateMaze();
            i = 0;
        }
        i++;
    }

    if(gPtr->isMazeComplete(pPtr))
    {
        gPtr->resetGrid(shaderPtr);
        gPtr->setCompleteStatus(false);
        gPtr->setGenerationStatus(true);
        pPtr->setPosition(glm::vec3(-300.0f, 300.0f, 10.0f));
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main()
{
    glfwSetErrorCallback(glfwErrorCallback);
    glfwInit();

    #ifdef __EMSCRIPTEN__
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    #else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #endif
    

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    #ifdef __EMSCRIPTEN__
    if (!gladLoadGLLoader((GLADloadproc)emscripten_webgl_get_proc_address))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    #else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    #endif

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    shader mainShader;
    shaderPtr = &mainShader;

    #ifdef __EMSCRIPTEN__
        mainShader.loadShaders("docs/vS3dWeb.shader", "docs/fS3DWeb.shader");
    #else
        mainShader.loadShaders("Shaders/vS3D.shader", "Shaders/fS3D.shader");
    #endif

    gPtr = new grid(&mainShader);
    // gPtr->generateMaze();

    pPtr = new player(&mainShader);
    pPtr->block3D(25, 25, 25);
    pPtr->setPosition(glm::vec3(-300.0f, 300.0f, 10.0f));

    view  = glm::lookAt(cameraPos, targetPos, cameraUp);
    float aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
    // projection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -100.0f, 100.0f);
    projection = glm::perspective(glm::radians(85.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1200.0f);
    float time;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!glfwWindowShouldClose(window))
    {
        main_loop();
    }
#endif

    delete gPtr;
    glDeleteProgram(mainShader.progID);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void glfwErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}