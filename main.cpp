#include <iostream>
#include <cstdlib>
#include <stack>

#include "game.h"

struct cell{
    bool hasWall[4] = {1, 1, 1, 1}; // 0-top 1-bottom 2-left 3-right
    int value = 15; // just for printing purpose before adding graphics stuffs
    bool visited = false;
};

class grid{
    private:
    cell blocks[64];
    // obstacle walls[4];
    std::stack<int> visitedCells;

    public:
    void removeWall(int block_num, int index) //index: 0-top 1-bottom 2-left 3-right
    {
        int i = block_num / 8; // row
        int j = block_num % 8; // column

        blocks[block_num].hasWall[index] = false;

        switch(index)
        {
            case 0:
                if(i > 0) blocks[(i-1) * 8 + j].hasWall[1] = false;
                break;

            case 1:
                if(i < 7) blocks[(i + 1) * 8 + j].hasWall[0] = false;
                break;

            case 2:
                if(j > 0) blocks[i * 8 + j - 1].hasWall[3] = false;
                break;

            case 3:
                if(j < 7) blocks[i * 8 + j + 1].hasWall[2] = false;
                break;

            default:
                break;
        }
    }

    void updateValues()
    {
        int value;
        for(int i = 0; i < 64; i++)
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
        for(int i = 1; i <= 64; i++)
        {
            std::cout << blocks[i-1].value << "  ";

            if(i % 8 == 0) std::cout << std::endl;
        }
    }

    bool hasUnvisitedNeighbours(int block_num)
    {
        int i = block_num / 8;
        int j = block_num % 8;

        if(i > 0) if(!blocks[(i - 1) * 8 + j].visited) return true;
        if(i < 7) if(!blocks[(i + 1) * 8 + j].visited) return true;
        if(j > 0) if(!blocks[i * 8 + (j - 1)].visited) return true;
        if(j < 7) if(!blocks[i * 8 + (j + 1)].visited) return true;

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
                int i = block_num / 8, j = block_num % 8;
                direction = rand() % 4;

                next_block = -1;
                switch(direction)
                {
                    case 0:
                        if(i > 0) next_block = (i - 1) * 8 + j;
                        break;

                    case 1:
                        if(i < 7) next_block = (i + 1) * 8 + j;
                        break;

                    case 2:
                        if(j > 0) next_block = i * 8 + (j - 1);
                        break;

                    case 3:
                        if(j < 7) next_block = i * 8 + (j + 1);
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
        int start_block = rand() % 64;
        recursiveBacktrack(start_block);
    }
};

int main()
{
    grid g;

    g.generateMaze();
    g.updateValues();
    g.displayGrid(); 
}