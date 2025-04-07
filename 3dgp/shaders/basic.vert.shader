// VERTEX SHADER

#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;		
uniform mat4 matrixModelView;
uniform vec3 material;				// TO REMOVE

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;

in vec3 aVertex;
in vec3 aNormal;

// Texturing variables
// for base maps
in vec2 aTexCoord;
out vec2 texCoord0;

// for normal maps
in vec3 aTangent;
in vec3 aBiTangent;
out mat3 matrixTangent;

out vec4 color;
out vec4 pos;		
out vec3 N;			

// Light declarations
struct AMBIENT
{
	vec3 color;
};
uniform AMBIENT lightAmbient;

vec4 AmbientLight(AMBIENT light)
{
	return vec4(materialAmbient * light.color,1);
}


void main(void) 
{
	// calculate position
	pos =  matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * pos;
	
    N = normalize(mat3(matrixModelView) * aNormal);
	
	// pass through texture co-ordinates for the fragment shader
    texCoord0 = aTexCoord;
	
	// calculate tangent local system transformation
    vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
    vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
    matrixTangent = mat3(tangent, biTangent, N);

	// calculate Light
	color = vec4(0,0,0,0);
	color += AmbientLight(lightAmbient);
	
}
