layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexcoord;

out vec3 worldPosition;
out vec2 texcoord;
void main()                     
{
	gl_Position=vec4(vPosition.xy,0.0,1.0);
	worldPosition=vPosition;
    texcoord=vTexcoord;
}
