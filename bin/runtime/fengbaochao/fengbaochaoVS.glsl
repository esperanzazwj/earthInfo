layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 col;

uniform mat4 og_modelViewPerspectiveMatrix;
uniform vec3 u_cam;
out vec4 _col;
out vec3 worldPosition;
void main()
{
    _col = col;
    worldPosition = pos;
    vec3 p = pos - u_cam;
    gl_Position = og_modelViewPerspectiveMatrix* vec4(p, 1.0);
}