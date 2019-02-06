#version 440 core

layout (local_size_x = 1) in;

layout (std430, binding = 0) buffer storage0 {
    ivec4 todayWorld[];
};

layout (std430, binding = 1) buffer storage1 {
    ivec4 tomorrowWorld[];
};

layout (std430, binding = 2) buffer storage2 {
    ivec4 yesterdayWorld[];
};

layout (std430, binding = 3) buffer storage3 {
    ivec4 theDayBeforeYesterdayWorld[];
};

uniform int WORLD_WIDTH;
uniform int WORLD_HEIGHT;

void main()
{
    int idx = int(gl_GlobalInvocationID.y) * WORLD_WIDTH + int(gl_GlobalInvocationID.x);
    ivec4 color = todayWorld[idx];

    int topIdx = idx + WORLD_WIDTH;
    int botIdx = idx - WORLD_WIDTH;
    int rightIdx = idx + 1;
    int leftIdx = idx - 1;
    int topLeftIdx = topIdx - 1;
    int topRightIdx = topIdx + 1;
    int botLeftIdx = botIdx - 1;
    int botRightIdx = botIdx + 1;
    ///This ifs transform the world into eclosed one
    if (gl_GlobalInvocationID.y == 0)
    {
        botIdx += WORLD_WIDTH * WORLD_HEIGHT;
        botLeftIdx = botIdx - 1;
        botRightIdx = botIdx + 1;
    }
    if (gl_GlobalInvocationID.y == WORLD_HEIGHT -1)
    {
        topIdx -= WORLD_WIDTH * WORLD_HEIGHT;
        topLeftIdx = topIdx - 1;
        topRightIdx = topIdx + 1;
    }
    if (gl_GlobalInvocationID.x  == 0)
    {
        leftIdx += WORLD_WIDTH;
        topLeftIdx += WORLD_WIDTH;
        botLeftIdx += WORLD_WIDTH;
    }
    if (gl_GlobalInvocationID.x == WORLD_WIDTH - 1)
    {
        rightIdx -= WORLD_WIDTH;
        topRightIdx -= WORLD_WIDTH;
        botRightIdx -= WORLD_WIDTH;
    }

    /// Original rules
    int aliveNeighbors = todayWorld[topIdx].r + todayWorld[botIdx].r + todayWorld[rightIdx].r + todayWorld[leftIdx].r + todayWorld[topLeftIdx].r + todayWorld[topRightIdx].r + todayWorld[botLeftIdx].r + todayWorld[botRightIdx].r;
    if (aliveNeighbors == 3)
        tomorrowWorld[idx] = ivec4(1, 0, 0, 0);
    if (aliveNeighbors == 2)
        tomorrowWorld[idx] = todayWorld[idx].rgba;
    if (aliveNeighbors < 2 || aliveNeighbors > 3)
        tomorrowWorld[idx] = ivec4(0, 0, 0, 0);
}
