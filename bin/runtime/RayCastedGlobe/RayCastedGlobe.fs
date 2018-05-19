in vec3 worldPosition;
out vec3 fragmentColor;

//uniform mat4x2 og_modelZToClipCoordinates;
uniform mat4 og_modelViewPerspectiveMatrix;//yes
uniform sampler2D diffuse_texture;//yes
uniform sampler2D specular_texture;//yes
uniform vec3 og_cameraLightPosition;//no
uniform vec3 og_cameraEye;//yes
uniform vec3 u_cameraEyeSquared;//yes
uniform vec3 u_globeOneOverRadiiSquared;//yes
uniform vec3 u_atmosOneOverRadiiSquared;//yes
uniform vec3 u_lightDir;//yes
uniform bool u_useAverageDepth;//no
uniform vec3 u_rte;//yes
struct Intersection
{
    bool  Intersects;
    float NearTime;         // Along ray
    float FarTime;          // Along ray
};

//
// Assumes ellipsoid is at (0, 0, 0)
//
Intersection RayIntersectEllipsoid(vec3 rayOrigin, vec3 rayOriginSquared, vec3 rayDirection, vec3 oneOverEllipsoidRadiiSquared)
{
    float a = dot(rayDirection * rayDirection, oneOverEllipsoidRadiiSquared);
    float b = 2.0 * dot(rayOrigin * rayDirection, oneOverEllipsoidRadiiSquared);
    float c = dot(rayOriginSquared, oneOverEllipsoidRadiiSquared) - 1.0;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
    {
        return Intersection(false, 0.0, 0.0);
    }
    else if (discriminant == 0.0)
    {
        float time = -0.5 * b / a;
        return Intersection(true, time, time);
    }

    float t = -0.5 * (b + (b > 0.0 ? 1.0 : -1.0) * sqrt(discriminant));
    float root1 = t / a;
    float root2 = c / t;

    return Intersection(true, min(root1, root2), max(root1, root2));
}

float ComputeWorldPositionDepth(vec3 position)
{ 
    position -= u_rte;
    vec4 v = og_modelViewPerspectiveMatrix * vec4(position, 1.0);   // clip coordinates
    v.z /= v.w;                                             // normalized device coordinates
    v.z = (v.z + 1.0) * 0.5;
    return v.z;
}

vec3 GeodeticSurfaceNormal(vec3 positionOnEllipsoid, vec3 oneOverEllipsoidRadiiSquared)
{
    return normalize(positionOnEllipsoid * oneOverEllipsoidRadiiSquared);
}

float LightIntensity(vec3 normal, vec3 toLight, vec3 toEye, vec4 diffuseSpecularAmbientShininess)
{
    vec3 toReflectedLight = reflect(-toLight, normal);

    float diffuse = max(dot(toLight, normal), 0.0);
    float specular = max(dot(toReflectedLight, toEye), 0.0);
    specular = pow(specular, diffuseSpecularAmbientShininess.w);

    return (diffuseSpecularAmbientShininess.x * diffuse) +
           (diffuseSpecularAmbientShininess.y * specular) +
            diffuseSpecularAmbientShininess.z;
}

vec2 ComputeTextureCoordinates(vec3 normal)
{
    float og_oneOverTwoPi = 1.0 / (2.0 * 3.14159);
    float og_oneOverPi = og_oneOverTwoPi * 2.0;
    return vec2(atan(normal.y, normal.x) * og_oneOverTwoPi + 0.5, asin(normal.z) * og_oneOverPi + 0.5);
}

void main()
{
    vec3 rayDirection = normalize(worldPosition - og_cameraEye);
    Intersection i = RayIntersectEllipsoid(og_cameraEye, u_cameraEyeSquared, rayDirection, u_globeOneOverRadiiSquared);

    if (i.Intersects)
    {
        vec3 position = og_cameraEye + (i.NearTime * rayDirection);
        vec3 normal = GeodeticSurfaceNormal(position, u_globeOneOverRadiiSquared);

        vec3 toLight = u_lightDir;//normalize(og_cameraLightPosition - position);
        vec3 toEye = normalize(og_cameraEye - position);
        float intensity = LightIntensity(normal, toLight, toEye, vec4(1,0,0.5,0));
        intensity =1.0;
        //fragmentColor = intensity * texture(diffuse_texture, ComputeTextureCoordinates(normal)).rgb;
	    //if(intensity< 1)
	    //	intensity = 1.0;
	    fragmentColor = intensity* texture(diffuse_texture, ComputeTextureCoordinates(normal)).rgb;
       // if (u_useAverageDepth)
        //{
       //     position = og_cameraEye + (mix(i.NearTime, i.FarTime, 0.5) * rayDirection);
       // }

        //gl_FragDepth = ComputeWorldPositionDepth(position, og_modelZToClipCoordinates);
	    float altitude = length(og_cameraEye)- length(position);
	    float pushVal = 200000.0;
        if (altitude < 1000000.0)
	    {
		    pushVal = 100000.0;
	    }   
	         position = og_cameraEye + ((i.NearTime+pushVal) * rayDirection);//push a little to write depth
	    gl_FragDepth = ComputeWorldPositionDepth(position);
    }
    else
    {
	    Intersection i = RayIntersectEllipsoid(og_cameraEye, u_cameraEyeSquared, rayDirection, u_atmosOneOverRadiiSquared);
	    if (i.Intersects)//atmos ring
	    {
	        vec3 position = og_cameraEye + (i.NearTime * rayDirection);
	        float SurfaceRadius = 6378137.0;
	        float AtmosphereRadius = 6441918.0;
	        float StretchAmt = 0.5;
	        vec3 camPos = og_cameraEye;
	        float radius = length(position);
	        float radius2 = radius * radius; 
	        float camHeight = length(og_cameraEye);
	        vec3 camToPos = position - og_cameraEye;
	        float farDist = length(camToPos);
	 
	        vec3 lightDir =normalize(u_lightDir);
	        vec3 normal = normalize(position);
	 
	        vec3 rayDir = camToPos / farDist;
	        float camHeight2 = camHeight * camHeight;
	        // Calculate the closest intersection of the ray with the outer atmosphere
	        float B = 2.0 * dot(og_cameraEye.xyz, rayDirection);
	        float C = camHeight2 - radius2;
	        float det = max(0.0, B*B - 4.0 * C);
	        float nearDist = 0.5 * (-B - sqrt(det));
	        vec3 nearPos = camPos.xyz + (rayDir * nearDist);
	        vec3 nearNormal = normalize(nearPos);
	 
	        // get dot products we need
	        float lc = dot(lightDir, camPos / camHeight);
	        float ln = dot(lightDir, normal);
	        float lnn = dot(lightDir, nearNormal);
	 
	        // get distance to surface horizon
	        float altitude = camHeight - SurfaceRadius;
	        if(altitude < 65000.0)
		    discard;//小于这个有问题
	        float horizonDist = sqrt((altitude*altitude) + (2.0 * SurfaceRadius * altitude));
	        float maxDot = horizonDist / camHeight;
	 
	        // get distance to atmosphere horizon - use max(0,...) because we can go into the atmosphere
            altitude = max(0.0, camHeight - AtmosphereRadius);
	        horizonDist = sqrt((altitude*altitude) + (2.0 * AtmosphereRadius * altitude));
	 
	        // without this, the shift between inside and outside atmosphere is  jarring
	        float tweakAmount = 0.1;
            float minDot = max(tweakAmount, horizonDist / camHeight);
	 
	        // scale minDot from 0 to -1 as we enter the atmosphere
	        float minDot2 = ((camHeight - SurfaceRadius) * (1.0 / (AtmosphereRadius  - SurfaceRadius))) - (1.0 - tweakAmount);
	        minDot = min(minDot, minDot2);
	        //if(altitude < AtmosphereRadius  - SurfaceRadius)
	         //  minDot = tweakAmount;
	        // get dot product of the vertex we're looking out
	        float posDot = dot(camToPos / farDist,-camPos.xyz / camHeight) - minDot;
	 
	        // calculate the height from surface in range 0..1
	        float height = posDot * (1.0 / (maxDot - minDot));
	   
	        // push the horizon back based on artistic taste
            ln = max(0.0, ln + StretchAmt);
            lnn = max(0.0, lnn + StretchAmt);
	 
	        // the front color is the sum of the near and far normals
	        float brightness = clamp(ln + (lnn * lc), 0.0, 1.0);
	        vec2 auv;
	        // use "saturate(lc + 1.0 + StretchAmt)" to make more of the sunset side color be used when behind the planet
            auv.x = brightness * clamp(lc + 1.0 + StretchAmt, 0.0, 1.0);
	        auv.y = 1.0 - height;//dx->ogl texcoord
	 
	        // as the camera gets lower in the atmosphere artificially increase the height
	        // so that the alpha value gets raised and multiply the increase amount
	        // by the dot product of the light and the vertex normal so that 
	        // vertices closer to the sun are less transparent than vertices far from the sun.
            height -= min(0.0, minDot2 + (ln * minDot2));
	        float alpha = height * brightness;
	 
	        // normalised camera to position ray
	        camToPos = -rayDir;
	 
	        //lightDir = normalize(lightPos.xyz - position.xyz); 
	        float Atmosphere_G = -0.95f;
 
	        const float fExposure = 1.5;
	        float g = Atmosphere_G;
	        float g2 = g * g;
	 
	        // atmosphere color
	        vec4 diffuse = texture(specular_texture,auv);
	 
	        // sun outer color - might could use atmosphere color
	        vec4 diffuse2 = texture(specular_texture,vec2(min(0.5,auv.x),1));
	 
	        // this is equivilant but faster than fCos = dot(normalize(lightDir.xyz),normalize(camToPos));
	        float fCos = dot(lightDir.xyz,camToPos) /dot(lightDir.xyz,lightDir.xyz) * dot(camToPos,camToPos);
	        float fCos2 = fCos * fCos;
	 
	        // apply alpha to atmosphere
	        vec4 diffuseColor = diffuse * alpha;
	 
	        // sun glow color
	        float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) /(1.0 + g2 - 2.0*g*fCos);
	        vec4 mieColor = diffuse2 * fMiePhase * alpha;
	 
	        // use exponential falloff because mie color is in high dynamic range
	        // boost diffuse color near horizon because it gets desaturated by falloff
	        fragmentColor= (1.0 - exp((diffuseColor * (1.0 + auv.y) + mieColor) * -fExposure)).xyz;
            position = og_cameraEye + ((i.NearTime + 10000.0) * rayDirection);//push a little to write depth
	        gl_FragDepth = ComputeWorldPositionDepth(position);
	    }
	    else 
	    {
	       discard;
	    }
    }
}
