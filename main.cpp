#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For transformations like translate, rotate, scale
#include <glm/gtc/type_ptr.hpp>

#include "game.h"
#include "shader.h"

#define GRIDSIZE 256
#define ROW 16

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
unsigned int SCR_WIDTH = 700;
unsigned int SCR_HEIGHT = 700;

class cell{

    public:
    bool hasWall[4] = {1, 1, 1, 1}; // 0-top 1-bottom 2-left 3-right
    int value = 15; // just for printing purpose before adding graphics stuffs
    bool visited = false;
    //gui part
    obstacle walls[4];

    void initializeWalls(shader *mainShader)
    {
        for(int i = 0; i < 4; i++)
        {
            walls[i].setShader(mainShader);
        }
        walls[0].block2D(45, 5);
        walls[0].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 0.0f)));

        walls[1].block2D(45, 5);
        walls[1].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, 0.0f)));

        walls[2].block2D(5, 45);
        walls[2].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 0.0f)));

        walls[3].block2D(5, 45);
        walls[3].setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, 0.0f)));
    }

    void draw()
    {
        for(int i = 0; i < 4; i++)
        {
            if(hasWall[i]) walls[i].draw();
        }
    }

    void setPosition(glm::vec3 position)
    {
        for(int i = 0; i < 4; i++)
        {
            walls[i].setPosition(position);
        }
    }
};

class grid{
    private:
    cell blocks[GRIDSIZE];
    std::stack<int> visitedCells;

    public:

    grid(shader *mainShader)
    {
        int i, j;
        for(int k = 0; k < GRIDSIZE; k++)
        {
            blocks[k].initializeWalls(mainShader);

            i = k / ROW; //row
            j = k % ROW; //column

            // blocks[k].setPosition(glm::vec3((float)i * 40, (float)j * 40, 0.0f));
            blocks[k].setPosition(glm::vec3((float)j * 40 - (ROW/2 * 40 - 20), -(float)i * 40 + (ROW/2 * 40 - 20), 0.0f));

        } 
    }

    void removeWall(int block_num, int index) //index: 0-top 1-bottom 2-left 3-right
    {
        int i = block_num / ROW; // row
        int j = block_num % ROW; // column

        blocks[block_num].hasWall[index] = false;

        switch(index)
        {
            case 0:
                if(i > 0) blocks[(i-1) * ROW + j].hasWall[1] = false;
                break;

            case 1:
                if(i < ROW - 1) blocks[(i + 1) * ROW + j].hasWall[0] = false;
                break;

            case 2:
                if(j > 0) blocks[i * ROW + j - 1].hasWall[3] = false;
                break;

            case 3:
                if(j < ROW - 1) blocks[i * ROW + j + 1].hasWall[2] = false;
                break;

            default:
                break;
        }
    }

    void updateValues()
    {
        int value;
        for(int i = 0; i < GRIDSIZE; i++)
        {
            value = 0; //index: 0-top 1-bottom 2-left 3-right
            if(blocks[i].hasWall[0])
                value += 1;

            if(blocks[i].hasWall[1])
                value += 2;

            if(blocks[i].hasWall[2])
                value += 4;

            if(blocks[i].hasWall[3])
                value += 8;

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
            // draw();
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
                    // draw();
                    recursiveBacktrack(next_block);
                    return;
                }
            }
        }
    }

    void generateMaze()
    {
        // int start_block = rand() % GRIDSIZE;
        int start_block = 20;
        recursiveBacktrack(start_block);
    }
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    // glEnable(GL_DEPTH_TEST);

    //make shaderObject
    shader mainShader("vShader.txt", "fShader.txt");

    //define model objects;

    grid g(&mainShader);
    g.generateMaze();
    // sphere.calculateNormals();

    glm::vec3 cameraPos(0.0f, 0.0f, 50.0f), targetPos(0.0f, 0.0f, 0.0f), cameraUp(0.0f, 1.0f, 0.0f);
    // model  = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f));

    view  = glm::lookAt(cameraPos, targetPos, cameraUp);

    float aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
    // float val = 1.2 * base.getAverageVertices();
    // float val = 600;
    projection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -100.0f, 100.0f);

    // float groundY = glm::vec4(model * glm::vec4(0.0f, ground.getBoundary()[3], 0.0f, 1.0f)).y;

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // float lastFrame = 0.0f;
    // float deltaTime = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // float currentFrame = glfwGetTime();
        // deltaTime = currentFrame - lastFrame;
        // lastFrame = currentFrame;
        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        g.draw();
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(mainShader.progID);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}