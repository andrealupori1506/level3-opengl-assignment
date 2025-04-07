// FRAGMENT SHADER
#version 330

// Materials

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// View Matrix
uniform mat4 matrixView;

in vec3 aVertex;
in vec3 aNormal;

// Textures -----------------------
// base map
uniform sampler2D texture0;
in vec2 texCoord0;

// normal map
uniform sampler2D textureNormal;
uniform bool useNormalMap;	// true enables normal mapping and false disables
in mat3 matrixTangent;

// Cell shading -------------------
const int toon_color_levels = 9;
const float toon_scale_factor = 1.0f / toon_color_levels; 

in vec4 pos;		
in vec3 N;
vec3 newNormal;

struct DIRECTIONAL
{
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	vec4 color = vec4(0,0,0,0);
	vec3 L = normalize(mat3(matrixView)*light.direction);
	float NdotL = dot(N,L);
	color += vec4(materialDiffuse * light.diffuse,1) * max(NdotL,0);
	color = ceil(color*toon_color_levels)*toon_scale_factor;
	return color;
}

struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;

vec4 PointLight(POINT light)
{
	vec4 color = vec4(0,0,0,0);
	vec3 L = normalize(matrixView*vec4(light.position,1)-pos).xyz;
    float NdotL = dot(newNormal, L);
	color += vec4(materialDiffuse * light.diffuse,1) * max(NdotL,0);

	// specular
	vec3 V = normalize(-pos.xyz);			// view vector
    vec3 R = reflect(-L, newNormal); // reflection vector
	float RdotV = dot(R,V);					// dot product - measures intensity.
	color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);
    color = ceil(color*toon_color_levels)*toon_scale_factor;
	return color;
}

struct SPOT
{
    vec3 position;
    vec3 diffuse;
    vec3 specular;
    
    // specific variables for spot lights
    vec3 direction;
    float cutoff;
    float attenuation;          // how much the light is concentrated around the centre
    
    // animation
    mat4 matrix;
};
uniform SPOT spotLight;

vec4 SpotLight(SPOT light)
{
    vec4 color = vec4(0, 0, 0, 0);
    vec3 L = normalize(light.matrix * vec4(light.position, 1) - pos).xyz;
    float NdotL = dot(newNormal, L);
    color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);

	// specular
    vec3 V = normalize(-pos.xyz); // view vector
    vec3 R = reflect(-L, newNormal); // reflection vector
    float RdotV = dot(R, V); // dot product - measures intensity.
    color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);

    // Determine the spot factor
    vec3 D = normalize(mat3(light.matrix) * light.direction);
    float spotFactor = dot(-L, D);
    float angle = acos(spotFactor);
    if (angle <= light.cutoff)
        spotFactor = pow(spotFactor, light.attenuation);
    else
        spotFactor = 0;

    // assuming that the Point Light value is stored as color and we have calculated spotFactor:
    color = spotFactor * color;

    color = ceil(color*toon_color_levels)*toon_scale_factor;
    return color;
}

in vec4 color;
out vec4 outColor;

void main(void) 
{
	// textures
	// normal map
    if (useNormalMap)
    {
        newNormal = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
        newNormal = normalize(matrixTangent * newNormal);
    }
    else
    {
        newNormal = N;
    }
	
	
	outColor = ceil(color*toon_color_levels)*toon_scale_factor;
    outColor += DirectionalLight(lightDir);
	outColor += PointLight(lightPoint1);
	outColor += PointLight(lightPoint2);
    outColor += SpotLight(spotLight);
	
	// base map
    outColor *= texture(texture0, texCoord0);

}
