uniform mat4 Projection;
uniform mat4 ViewMatrix;
uniform mat4 WorldMatrix;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexcoord;

out vec3 world_pos;
out vec3 world_normal;
out vec2 texcoord;

void main()
{
    world_pos = vec3(WorldMatrix * vec4(vPosition, 1.0));
    world_normal = vNormal;
    texcoord = vTexcoord;
    gl_Position = Projection * ViewMatrix * WorldMatrix * vec4(vPosition, 1.0);
	gl_Position = vec4(vPosition.xy,0.0,1.0);
}

