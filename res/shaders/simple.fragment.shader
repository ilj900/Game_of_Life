#version 440 core

out vec4 FragColor;

uniform isamplerBuffer sceneTexture;
uniform int WORLD_WIDTH;
uniform int WORLD_HEIGHT;
uniform int CELL_WIDTH;
uniform int CELL_HEIGHT;

void main()
{
    int x = int(gl_FragCoord.x) / CELL_WIDTH;
    int y = int(gl_FragCoord.y) / CELL_HEIGHT;
    int fragIndex = WORLD_WIDTH * y + x;
    ivec4 color = texelFetch(sceneTexture, fragIndex);

    //Just ignore alpha... for now
    if (color.r + color.g + color.b + color.a == 0)
        discard;
    FragColor = vec4(color.r, color.g, color.b, 1.0);
}
