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

#define GRIDSIZE 256
#define ROW 16

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void glfwErrorCallback(int error, const char* description);

unsigned int SCR_WIDTH = 700;
unsigned int SCR_HEIGHT = 700;

glm::vec3 cameraPos(0.0f, 0.0f, 500.0f), targetPos(0.0f), cameraUp(0.0f, 1.0f, 0.0f);
light lightSource = {glm::vec3(0.0f, 0.0f, 200.0f), glm::vec4(1.0f), 1.0f};
glm::mat4 rotMat(1.0f);

// Forward declare classes
class cell;
class grid;

class cell {
public:
    bool hasWall[4] = {1, 1, 1, 1}; // 0-top 1-bottom 2-left 3-right
    int value = 15; // just for printing purpose before adding graphics stuffs
    bool visited = false;
    obstacle walls[4];
    ground base;

    void initializeWalls(shader *mainShader)
    {
        base.block3D(40, 40, 4);
        base.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f)));
        base.setShader(mainShader);
        
        for (int i = 0; i < 4; i++) {
            walls[i].setShader(mainShader);
        }
        walls[0].block3D(45, 5, 50);
        walls[0].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 0.0f)));

        walls[1].block3D(45, 5, 50);
        walls[1].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, 0.0f)));

        walls[2].block3D(5, 45, 50);
        walls[2].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 0.0f)));

        walls[3].block3D(5, 45, 50);
        walls[3].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, 0.0f)));
    }

    void draw()
    {
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
};

class grid {
private:
    cell blocks[GRIDSIZE];
    std::stack<int> visitedCells;

public:
    grid(shader *mainShader)
    {
        int i, j;
        for (int k = 0; k < GRIDSIZE; k++) {
            blocks[k].initializeWalls(mainShader);

            i = k / ROW; // row
            j = k % ROW; // column

            blocks[k].setPosition(glm::vec3((float)j * 40 - (ROW/2 * 40 - 20), -(float)i * 40 + (ROW/2 * 40 - 20), 0.0f));
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

    void updateValues()
    {
        int value;
        for(int i = 0; i < GRIDSIZE; i++)
        {
            value = 0;
            if(blocks[i].hasWall[0]) value += 1;
            if(blocks[i].hasWall[1]) value += 2;
            if(blocks[i].hasWall[2]) value += 4;
            if(blocks[i].hasWall[3]) value += 8;

            blocks[i].value = value;
        }
    }

    void displayGrid()
    {
        for(int i = 1; i <= GRIDSIZE; i++)
        {
            std::cout << blocks[i-1].value << "  ";
            if(i % 8 == 0) std::cout << std::endl;
        }
    }

    void draw()
    {
        for(int i = 0; i < GRIDSIZE; i++)
        {
            blocks[i].draw();
        }
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

    void recursiveBacktrack(int block_num)
    {
        int next_block;

        blocks[block_num].visited = true;
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
            recursiveBacktrack(next_block);
        }
        else
        {
            while(!visitedCells.empty())
            {
                next_block = visitedCells.top();
                visitedCells.pop();

                if(hasUnvisitedNeighbours(next_block))
                {
                    recursiveBacktrack(next_block);
                    return;
                }
            }
        }
    }

    void generateMaze()
    {
        int start_block = 37;
        recursiveBacktrack(start_block);
    }

    void checkCollision(player* pPtr)
    {
        for(int i = 0; i < GRIDSIZE; i++)
        {
            blocks[i].checkCollision(pPtr);
        }
    }
};

// Globals needed for emscripten main loop
GLFWwindow* window = nullptr;
grid* gPtr = nullptr;
player* pPtr = nullptr;


void main_loop()
{
    static float lastFrame = 0.0f;
    static float deltaTime = 0.0f; 

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    if (!window)
        return;

    processInput(window);

    // static float angle = 0.0f;
    // angle += glm::radians(0.5f); // smooth constant speed

    // // ❗ Fixed position — this never changes
    // glm::vec3 baseLight(200.0f, 200.0f, 200.0f);

    // // ✅ Rotate the base vector instead of current light position
    // glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
    // lightSource.position = glm::vec3(rotMat * glm::vec4(baseLight, 1.0f));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(pPtr)
    {
        pPtr->movements(window);
        // pPtr->updatePhysics(deltaTime);
        pPtr->draw(lightSource, cameraPos);
    }

    if (gPtr)
    {
        gPtr->draw();
        gPtr->checkCollision(pPtr);
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
    
    #ifdef __EMSCRIPTEN__
        mainShader.loadShaders("Emiscripten/vS3dWeb.shader", "Emiscripten/fS3DWeb.shader");
    #else
        mainShader.loadShaders("Shaders/vS3D.shader", "Shaders/fS3D.shader");
    #endif

    gPtr = new grid(&mainShader);
    gPtr->generateMaze();

    pPtr = new player(&mainShader);
    pPtr->block3D(25, 25, 25);
    pPtr->setPosition(glm::vec3(-300.0f, 290.0f, 10.0f));

    view  = glm::lookAt(cameraPos, targetPos, cameraUp);
    float aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
    // projection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -100.0f, 100.0f);
    projection = glm::perspective(glm::radians(75.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1200.0f);

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