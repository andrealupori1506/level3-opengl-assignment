#version 330

in float age;
uniform sampler2D texture0;
in vec2 texCoord0;
out vec4 outColor;

void main()
{
    outColor = vec4(0.9, 0.85, 0.9, 1 - age);
    outColor *= texture(texture0, gl_PointCoord);
    outColor.a *= 1 - age;
}