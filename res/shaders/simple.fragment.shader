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

    if (color.a == 0)
        discard;

    FragColor = vec4(color.r/255.0, color.g/255.0, color.b/255.0, 1.0);
}
