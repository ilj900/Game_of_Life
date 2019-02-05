#version 440 core

layout (local_size_x = 1) in;

layout (std430, binding = 0) buffer storage1 {
    lowp uvec3 currentWorld[];
};

layout (std430, binding = 1) buffer storage2 {
    lowp uvec3 tomorrowWorld[];
};



void main()
{
    uvec2 idx = gl_GlobalInvocationID.xy;
}
