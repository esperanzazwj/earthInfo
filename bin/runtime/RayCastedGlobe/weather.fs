in vec3 world_pos;
in vec3 world_normal;
in vec2 texcoord;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 eyePosition;

out vec4 fragment_color;

void main()
{
    vec3 N = normalize(world_normal);
    vec3 L = normalize(lightDirection);
    vec3 V = normalize(eyePosition - world_pos);
    vec3 H = normalize(V + N);
    float fakeGI = 0.2;
    float diffuse_factor = max(0.0, mix(dot(L, N), 1.0, fakeGI)) + (1.0 - fakeGI) * 0.01;
	gl_FragDepth = 0.1;
    fragment_color = vec4(diffuse_factor, diffuse_factor, diffuse_factor, 1.0);
}

