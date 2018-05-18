attribute vec4 position;
varying vec3 worldPosition;

uniform vec3 u_rte;
uniform mat4 og_modelViewPerspectiveMatrix;

void main()                     
{
    vec4 pos;
    pos.xyz = position.xyz - u_rte;
    pos.w = 1;
    gl_Position = og_modelViewPerspectiveMatrix * pos; 
    worldPosition = position.xyz;
}
