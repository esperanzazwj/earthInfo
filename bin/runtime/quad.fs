

in vec2 texcoord;
uniform sampler2D test_tex;
uniform sampler2D test_tex2;
uniform samplerCube test_tex_cube;
out vec4 color;

void main() {
  vec4 q = textureLod(test_tex, texcoord, 0.0) ;
  vec4 c = textureLod(test_tex2, texcoord, 0.0) ;
  //vec4 q = textureLod(test_tex2, texcoord, 0) * textureLod(test_tex, texcoord, 0);
  // vec4 p = texture(test_tex2, texcoord);
  // vec4 g=texture(test_tex_cube, vec3(texcoord.xy,1.0));
  color=vec4(q.xyz,1.0);
  //color = vec4(1,1,0,1);
  // color=vec4(c.xyz,1.0);
  // color = vec4(1,1,0,1);
 // color=vec4(1-1*(1-q.x));
}
