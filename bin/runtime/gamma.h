vec3 gamma=vec3(0.45);
vec3 invgamma=vec3(2.2);
vec3 GammaCorrect(vec3 color){
	return pow(color,gamma);
}

vec3 InvGamma(vec3 color){
	return pow(color,invgamma);
}