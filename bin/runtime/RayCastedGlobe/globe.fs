in vec3 world_pos;
in vec3 world_normal;
in vec2 texcoord;

uniform vec3 lightColor;
uniform vec3 lightPosition;

uniform mat4 Projection;
uniform mat4 ViewMatrix;
uniform mat4 WorldMatrix;

uniform vec3 eyepos;

uniform vec4 diffuse_color;
uniform sampler2D diffuse_texture;

uniform vec4 specular_color;
uniform sampler2D specular_texture;

uniform vec4 ambient_color;
uniform sampler2D ambient_texture;
uniform mat4 proj_inv;

out vec4 fragment_color;

#include "../gamma.h"

void main()
{
    vec3 N = normalize(world_normal);
    vec3 L = normalize(lightPosition - world_pos);
    vec3 V = normalize(eyepos - world_pos);
    vec3 H = normalize(V + N);
    float fakeGI = 0.2;
    float diffuse_factor = max(0.0, mix(dot(L, N), 1.0, fakeGI)) + (1.0 - fakeGI) * 0.01;

    vec3 direct = diffuse_factor * texture(diffuse_texture, texcoord).rgb;
    direct = GammaCorrect(direct);
    fragment_color = vec4(direct, 1.0);
}

