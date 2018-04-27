uniform mat4 ModelMatrix;
uniform mat4 ViewPerspectiveMatrix;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexcoord;

out vec3 world_pos;
out vec3 world_normal;
out vec2 texcoord;

void main()
{
    world_pos = vec3(ModelMatrix * vec4(vPosition, 1.0));
    world_normal = vNormal;
    texcoord = vTexcoord;
    gl_Position = ViewPerspectiveMatrix*ModelMatrix*vec4(vPosition, 1.0);
}

