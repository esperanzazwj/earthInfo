layout(location = 0) in vec4 position;
uniform vec3 u_rte;
out vec3 worldPosition;
uniform mat4 og_modelViewPerspectiveMatrix;

void main()                     
{
    vec4 pos;
    pos.xyz = position.xyz - u_rte;
    pos.w = 1.0;
    gl_Position = og_modelViewPerspectiveMatrix * pos; 
    worldPosition = position.xyz;
}
