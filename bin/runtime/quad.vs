
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;							

out vec2 texcoord;
void main()
{
	vec4 Pos = vec4(vPosition,1.0);
	texcoord.x = 0.5 * (1.0 + Pos.x);
	texcoord.y = 0.5 * (1.0 + Pos.y);
	gl_Position = vec4(vPosition,1.0);
}
