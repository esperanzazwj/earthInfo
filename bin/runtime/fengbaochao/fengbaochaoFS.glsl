in vec4 _col;
uniform vec4 func;//裁切专用的，如果不裁切，直接前三项为0就ok
out vec4 fragmentColor;
in vec3 worldPosition;
void main()
{
    if(abs(func.x + func.y + func.z) >0.01)
    {
		if(worldPosition.x * func.x + worldPosition.y* func.y + worldPosition.z* func.z + func.w < 0)
			discard;
    }
    fragmentColor = abs(_col);
    gl_FragDepth = 0.001;
	//fragmentColor = vec4(1);
}
