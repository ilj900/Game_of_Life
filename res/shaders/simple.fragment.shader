#version 440 core

out vec4 FragColor;

uniform usamplerBuffer sceneTexture;
uniform int fieldWidth;
uniform int fieldHeight;
uniform int cellWidth;
uniform int cellHeight;

void main()
{
    int x = int(gl_FragCoord.x) / cellWidth;
    int y = int(gl_FragCoord.y) / cellHeight;
    int fragIndex = fieldWidth * y + x;
    uvec4 color = texelFetch(sceneTexture, fragIndex);
    FragColor = vec4(color.rgb, 1.0);
}
