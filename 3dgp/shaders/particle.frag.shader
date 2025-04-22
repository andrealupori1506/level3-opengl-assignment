#version 330

in float age;
uniform sampler2D texture0;

in vec2 texCoord0;
out vec4 outColor;

uniform vec3 camPos;
in vec3 particlePos;

void main()
{
    if(length(particlePos-camPos)>10)discard;

    outColor = vec4(0.9, 0.85, 0.9, 1 - age);
    outColor *= texture(texture0, gl_PointCoord);
    outColor.a *= 1 - age;
}