#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#define _USE_MATH_DEFINES
#include <math.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// GLSL Program

// access the shaders
C3dglProgram program, programEffect, programParticle;

// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

// triangle parts
float vertices[] = {
-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,
-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,
-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 };

float normals[] = {
0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };

unsigned indices[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };

// 3D models
C3dglModel cat, wallSegment, wallWindow, floorTile, table, teapot, vase, mug, lamp, ceilingLamp, entireKitchenSet;
// skinless animations
C3dglModel walk, swat;
float animTime, swattingAnimTime, currentCatRot;
bool catMoving, catSwatting;
bool catCamOn = true;
vec3 currentCatPos;

// Textures
// null texture
GLuint idTexNone;
// base maps
GLuint idTexCat, idTexWall, idTexTable, idTexChair, idTexTeapot, idTexSmokeParticle;

// normal maps
GLuint idTexTableNormal, idTexChairNormal, idTexTeapotNormal;

// FBO variables 
GLuint idTexScreen, idFBO, bufQuad;
GLuint WImage = 1280, HImage = 720;

// particle systems
GLuint idBufferVelocity, idBufferStartTime;

// The View Matrix
mat4 matrixView;

// Particle System Params
const float PERIOD = 0.01f;
const float LIFETIME = 2.f;
const int NPARTICLES = (int)(LIFETIME / PERIOD);

// bulb variables
// bulb locations
vec3 bulbLoc1 = vec3(-12.0f + (2 * 12), 5, -11.6f);
vec3 bulbLoc2 = vec3(3.055f, 5.55f, 0.052f);
vec3 bulbLoc3 = vec3(-18.8f, 5, 15);
vec3 bulbLoc4 = vec3(12.0f, 5, 38);
vec3 bulbLoc5 = vec3(38.0f, 5, 15);

// bulb lights
bool bulbOff1 = false;
bool bulbOff2 = true;
vec3 bulbOnV = vec3(0.0001, 0.0001, 0.0001);
vec3 bulbOffV = vec3(0, 0, 0);

// spot light position
vec3 spotLightLoc = vec3(0.f, 12.f, 0.f);

// ambient room light
vec3 ambientRoomLight = vec3(0.15f, 0.01f, 0.15f);
vec3 ambientRoomMaterial = vec3(0.0f, 0.0f, 0.0f);

// Toon toogle
bool toonLinesOn = false;



// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

void onReshape(int w, int h);

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Shaders ------------------------------------------------------------------------------------------------
	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	// load BASIC shaders
	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert.shader")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag.shader")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// load POST PROCESSING shaders ------------------------------------------------
	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/effect.vert.shader")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/colour.frag.shader")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programEffect.create()) return false;
	if (!programEffect.attach(vertexShader)) return false;
	if (!programEffect.attach(fragmentShader)) return false;
	if (!programEffect.link()) return false;
	if (!programEffect.use(true)) return false;

	// load particle shaders ---------------------------------------------------------
	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/particle.vert.shader")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/particle.frag.shader")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programParticle.create()) return false;
	if (!programParticle.attach(vertexShader)) return false;
	if (!programParticle.attach(fragmentShader)) return false;
	if (!programParticle.link()) return false;
	if (!programParticle.use(true)) return false;
	// switch on: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	program.use();

	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Load 3D models 
	if (!table.load("models\\table.obj")) return false;
	if (!teapot.load("models\\utah_teapot_hires.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!mug.load("models\\mug.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!floorTile.load("models\\floor_tiles_kitchen.obj")) return false;
	if (!wallSegment.load("models\\wall_tiles_kitchen_straight.obj")) return false;
	if (!wallWindow.load("models\\wall_tiles_kitchen_window.obj")) return false;
	if (!ceilingLamp.load("models\\ceilinglamp.3ds")) return false;
	if (!entireKitchenSet.load("models\\test.fbx")) return false;

	// cat model and animations
	walk.load("models\\Animated Model\\Cat_Original_walk_cycle.fbx");
	//jump.load("models\\Animated Model\\Cat_Jump_No_ref.fbx");
	swat.load("models\\Animated Model\\Cat_swat_No_ref.fbx");

	if (!cat.load("models\\Animated Model\\CatModel.fbx")) return false;
	cat.loadAnimations(&walk);
	//cat.loadAnimations(&jump);
	cat.loadAnimations(&swat);
	// Load Textures ----------------------------------------------------------------------------------------------------------------------------------
	// Base maps

	// cat texture
	C3dglBitmap catTexture;
	catTexture.load("models/Animated Model/Tex_Cat_Carrot.jpg", GL_RGBA);
	if (!catTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexCat);
	glBindTexture(GL_TEXTURE_2D, idTexCat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, catTexture.getWidth(), catTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, catTexture.getBits());

	// wall AND floor texture
	C3dglBitmap wallTexture;
	wallTexture.load("models/textures/tiny_treats_texture_1.png", GL_RGBA);
	if (!wallTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWall);
	glBindTexture(GL_TEXTURE_2D, idTexWall);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wallTexture.getWidth(), wallTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, wallTexture.getBits());

	// table texure
	C3dglBitmap tableTexture;
	tableTexture.load("models/textures/oak.bmp", GL_RGBA);
	if (!tableTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexTable);
	glBindTexture(GL_TEXTURE_2D, idTexTable);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tableTexture.getWidth(), tableTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tableTexture.getBits());

	// chair texure
	C3dglBitmap chairTexture;
	chairTexture.load("models/textures/Metal_Steel_Brushed_001_diffuse.jpg", GL_RGBA);
	if (!chairTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexChair);
	glBindTexture(GL_TEXTURE_2D, idTexChair);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, chairTexture.getWidth(), chairTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, chairTexture.getBits());

	// teapot texure
	C3dglBitmap teapotTexture;
	teapotTexture.load("models/textures/Glass_Pattern_002_basecolor.jpg", GL_RGBA);
	if (!teapotTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexTeapot);
	glBindTexture(GL_TEXTURE_2D, idTexTeapot);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, teapotTexture.getWidth(), teapotTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, teapotTexture.getBits());

	// NULL texture - to counter the previous textures
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	

	// NORMAL maps -------------------------------------------------------------------------------------
	// table normal texture
	C3dglBitmap tableNormal;
	tableNormal.load("models/textures/normal1.jpg", GL_RGBA);
	if (!tableNormal.getBits()) return false;
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexTableNormal);
	glBindTexture(GL_TEXTURE_2D, idTexTableNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tableNormal.getWidth(), tableNormal.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tableNormal.getBits());

	// chair normal texture
	C3dglBitmap chairNormal;
	chairNormal.load("models/textures/Metal_Steel_Brushed_001_normal.jpg", GL_RGBA);
	if (!chairNormal.getBits()) return false;
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexChairNormal);
	glBindTexture(GL_TEXTURE_2D, idTexChairNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, chairNormal.getWidth(), chairNormal.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, chairNormal.getBits());

	// teapot normal texture
	C3dglBitmap teapotNormal;
	teapotNormal.load("models/textures/Glass_Pattern_002_normal.jpg", GL_RGBA);
	if (!teapotNormal.getBits()) return false;
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexTeapotNormal);
	glBindTexture(GL_TEXTURE_2D, idTexTeapotNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, teapotNormal.getWidth(), teapotNormal.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, teapotNormal.getBits());

	// Send the texture info to the shaders
	program.sendUniform("texture0", 0);
	program.sendUniform("textureNormal", 1);
	programEffect.sendUniform("texture0", 0);

	// SET UP LIGHTS HERE --------------------------------------------------------------------------------------------------------------------------------
	// ambient light
	program.sendUniform("lightAmbient.color", ambientRoomLight);

	// directional light
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 0.5));
	program.sendUniform("lightDir.diffuse", vec3(1, 1, 1));
	//program.sendUniform("lightDir.diffuse", vec3(0, 0, 0));

	// point lightS
	program.sendUniform("lightPoint1.position", bulbLoc1);
	program.sendUniform("lightPoint2.position", bulbLoc2);
	program.sendUniform("lightPoint3.position", bulbLoc3);
	program.sendUniform("lightPoint4.position", bulbLoc4);
	program.sendUniform("lightPoint5.position", bulbLoc5);

	// spot light
	program.sendUniform("spotLight.position", spotLightLoc);
	program.sendUniform("spotLight.diffuse", vec3(1,1,1));
	program.sendUniform("spotLight.specular", vec3(0.5, 0.5, 0.5));

	//program.sendUniform("spotLight.diffuse", vec3(0, 0, 0));
	//program.sendUniform("spotLight.specular", vec3(0, 0, 0));

	program.sendUniform("spotLight.direction", vec3(0, -1, 0));
	program.sendUniform("spotLight.cutoff", radians(45.f));
	program.sendUniform("spotLight.attenuation", 7.f);

	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));

	// particle system set up ----------------------------------------------------------------------------------------------
	C3dglBitmap smokeParticleTexture;
	smokeParticleTexture.load("models/textures/smoke.png", GL_RGBA);
	if (!smokeParticleTexture.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexSmokeParticle);
	glBindTexture(GL_TEXTURE_2D, idTexSmokeParticle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, smokeParticleTexture.getWidth(), smokeParticleTexture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, smokeParticleTexture.getBits());
	glEnable(0x8642);
	glEnable(GL_POINT_SPRITE);

	programParticle.sendUniform("texture0", 0);

	programParticle.sendUniform("initialPos", vec3(1.3, 4.4f, 0.4f));
	programParticle.sendUniform("gravity", vec3(0.0, 1, 0.0));
	programParticle.sendUniform("particleLifetime", LIFETIME);

	// Prepare the particle buffers
	vector<float> bufferVelocity;
	vector<float> bufferStartTime;

	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = (float)M_PI / 1.5f * (float)rand() / (float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 0.1 + 0.1f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Post processing ---------------------------------------------------------------------------------------------
	// First Pass -----------------------------------------------------------------------------
	// Create screen space texture
	glGenTextures(1, &idTexScreen);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);

	// Texture parameters - to get nice filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This will allocate an uninitilised texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WImage, HImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, idFBO);

	// Attach a depth buffer
	GLuint depth_rb;
	glGenRenderbuffers(1, &depth_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WImage, HImage);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

	// attach the texture to FBO colour attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTexScreen, 0);

	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	// set toon toggle
	programEffect.sendUniform("toonLinesToggle", toonLinesOn);

	// Second pass --------------------------------------------------------------------------------
	// Create Quad
	float quadVertices[] = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	// Generate the buffer name
	glGenBuffers(1, &bufQuad);
	// Bind the vertex buffer and send data
	glBindBuffer(GL_ARRAY_BUFFER, bufQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Initialise the View Matrix (initial position of the camera) --------------------------------------------
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(-10.0, 2.65, 0.0),
		vec3(0.0, 1.0, 10.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  F to swat" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << "  1 to toggle day and night lighting" << endl;
	cout << "  2 to toggle table lamp" << endl;
	cout << "  3 to toggle cartoon shader" << endl;
	cout << "  4 to toggle cat cam" << endl;
	cout << endl;

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;
	// Camera position
	vec3 pos = getPos(matrixView);

	// setup material - grey

	// program.sendUniform("materialSpecular", vec3(0,0,0)); - will set ALL BELOW specular light to be OFF and so on - works like the previous material thing

	// wall segments -------------------------------------------------------------------------------
	// texturing
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexWall);

	// material and rendering
	program.sendUniform("materialAmbient", ambientRoomLight);
	program.sendUniform("materialDiffuse", vec3(0.2, 0.2, 0.2));
	program.sendUniform("materialSpecular", vec3(0.1, 0.1, 0.1));
	program.sendUniform("shininess", 10.0);
	for (int eachSide = 0; eachSide < 4; eachSide++)
	{
		for (int i = 0; i < 25; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				float xTranslation, zTranslation;
				if (j != 10&&i!=12) {		// creates empty spaces for windows
					if (eachSide == 0) { xTranslation = -14.0f + (2 * i); zTranslation = -8.0f; }		// facing south
					else if (eachSide == 1) { xTranslation = -14.0f; zTranslation = -8.0f + (2 * j); }	// facing east
					else if (eachSide == 2) { xTranslation = -14.0f + (2 * i); zTranslation = 30; }		// facing north
					else if (eachSide == 3) { xTranslation = 30.0f; zTranslation = -8.0f + (2 * j); }	// facing west
					m = matrixView;
					m = scale(m, vec3(1.2f, 2.2f, 1.2f));
					m = translate(m, vec3(xTranslation, 0.25f, zTranslation));
					m = rotate(m, radians(90.f * eachSide), vec3(0.0f, 1.0f, 0.0f));
					program.sendUniform("matrixModelView", m);
					wallSegment.render(m);
				}
				else if (j == 10 && i == 12)	// fills out windows
				{
					if (eachSide == 0) { xTranslation = -14.0f + (2 * i); zTranslation = -8.0f; }		// facing south
					else if (eachSide == 1) { xTranslation = -14.0f; zTranslation = -8.0f + (2 * j); }	// facing east
					else if (eachSide == 2) { xTranslation = -14.0f + (2 * i); zTranslation = 30; }		// facing north
					else if (eachSide == 3) { xTranslation = 30.0f; zTranslation = -8.0f + (2 * j); }	// facing west

					m = matrixView;
					m = scale(m, vec3(1.2f, 2.2f, 1.2f));
					m = translate(m, vec3(xTranslation, 0.25f, zTranslation));
					m = rotate(m, radians(90.f * eachSide), vec3(0.0f, 1.0f, 0.0f));
					program.sendUniform("matrixModelView", m);
					wallWindow.render(m);
				}
				
			}
		}
	}

	// floor tiles ------------------------------------------------------------------------------------------------
	for (int floorOrCeiling = 0; floorOrCeiling < 2; floorOrCeiling++) // make a floor and ceiling
	{
		float y;
		if (floorOrCeiling == 0) { y = 0.0f; }
		else { y = 7.8f; }
		for (int i = 0; i < 25; i++) // row of grid
		{
			for (int j = 0; j < 20; j++) // column of grid
			{
				m = matrixView;
				m = scale(m, vec3(1.2f, 1.2f, 1.2f));
				m = translate(m, vec3(-14.0f + (2 * i), y, -8.0f + (2 * j)));
				program.sendUniform("matrixModelView", m);
				floorTile.render(m);
			}
		}
	}

	m = matrixView;
	m = translate(m, vec3(5, 0.6f, 0.0f));
	m = scale(m, vec3(0.02f));
	entireKitchenSet.render(m);

	// table --------------------------------------------------------------------------------------------------------
	// texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexTable);
	glActiveTexture(GL_TEXTURE1);
	program.sendUniform("useNormalMap", true);
	glBindTexture(GL_TEXTURE_2D, idTexTableNormal);

	program.sendUniform("materialSpecular", vec3(0.9, 0.9, 0.9));

	m = matrixView;
	m = scale(m, vec3(0.005f, 0.005f, 0.005f));
	program.sendUniform("matrixModelView", m);
	table.render(1, m); // renders table

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexChair);
	glActiveTexture(GL_TEXTURE1);
	program.sendUniform("useNormalMap", true);
	glBindTexture(GL_TEXTURE_2D, idTexChairNormal);

	program.sendUniform("materialAmbient", ambientRoomLight);
	program.sendUniform("materialDiffuse", vec3(0.5, 0.5, 0.6));
	

	for (int i = 0; i < 4; i++)
	{
		m = rotate(m, radians(90.f * i), vec3(0.0f, 1.0f, 0.0f));
		program.sendUniform("matrixModelView", m);
		table.render(0, m); // renders chairs
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexTeapot);
	glActiveTexture(GL_TEXTURE1);
	program.sendUniform("useNormalMap", true);
	glBindTexture(GL_TEXTURE_2D, idTexTeapotNormal);

	// teapot
	m = matrixView;
	m = translate(m, vec3(2, 3.75f, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("matrixModelView", m);
	teapot.render(m);

	program.sendUniform("useNormalMap", false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// setup material - blue
	program.sendUniform("materialAmbient", ambientRoomMaterial);
	program.sendUniform("materialDiffuse", vec3(0.0784, 0.5411, 0.9804));
	program.sendUniform("shininess", 8.0);

	// vase
	m = matrixView;
	m = translate(m, vec3(0, 3.8f, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	program.sendUniform("matrixModelView", m);
	vase.render(m);

	
	// the lamp holder 1 ---------------------------------------------------------------------------
	// setup material - green
	program.sendUniform("materialDiffuse", vec3(0.0, 0.36222, 0.194117));
	m = matrixView;
	m = translate(m, vec3(2.5f, 3.8f, 1.0f));
	m = rotate(m, radians(240.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.03f, 0.03f, 0.03f));
	program.sendUniform("matrixModelView", m);
	lamp.render(m);

	program.sendUniform("materialAmbient", vec3(0.0, 0.0, 0.6));
	if (bulbOff1)
	{
		program.sendUniform("lightPoint1.diffuse", bulbOffV);
		program.sendUniform("lightPoint1.specular", bulbOffV);
		program.sendUniform("lightPoint3.diffuse", bulbOffV);
		program.sendUniform("lightPoint3.specular", bulbOffV);
		program.sendUniform("lightPoint4.diffuse", bulbOffV);
		program.sendUniform("lightPoint4.specular", bulbOffV);
		program.sendUniform("lightPoint5.diffuse", bulbOffV);
		program.sendUniform("lightPoint5.specular", bulbOffV);
	}
	else
	{
		// set bulb to emit as much light as possible with the ambient colour
		program.sendUniform("lightAmbient.color", vec3(1, 1, 1));
		program.sendUniform("materialAmbient", vec3(0.0784, 0.3411, 0.9804));
		program.sendUniform("lightPoint1.diffuse", bulbOnV);
		program.sendUniform("lightPoint1.specular", bulbOnV);
		program.sendUniform("lightPoint3.diffuse", bulbOnV);
		program.sendUniform("lightPoint3.specular", bulbOnV);
		program.sendUniform("lightPoint4.diffuse", bulbOnV);
		program.sendUniform("lightPoint4.specular", bulbOnV);
		program.sendUniform("lightPoint5.diffuse", bulbOnV);
		program.sendUniform("lightPoint5.specular", bulbOnV);
	}

	program.sendUniform("materialDiffuse", vec3(0.000005, 0.0005, 0.1));			// blue
	for (int i = 0; i < 4; i++)
	{
		m = matrixView;
		if (i==0) m = translate(m, bulbLoc1);								// south
		if (i==1) m = translate(m, bulbLoc3);								// east
		if (i==2) m = translate(m, bulbLoc4);						// north
		if (i==3) m = translate(m, bulbLoc5);						// west
		program.sendUniform("matrixModelView", m);
		glutSolidCube(4);
	}
	
	program.sendUniform("lightAmbient.color", vec3(0.1, 0.1, 0.1));
	program.sendUniform("materialAmbient", vec3(0.3, 0.3, 0.3));

	if (bulbOff2)
	{
		program.sendUniform("lightPoint2.diffuse", bulbOffV);
		program.sendUniform("lightPoint2.specular", bulbOffV);
	}
	else
	{
		program.sendUniform("lightAmbient.color", vec3(1, 1, 1));
		program.sendUniform("materialAmbient", vec3(0.9, 0.9, 0.9));
		program.sendUniform("lightPoint2.diffuse", bulbOnV);
		program.sendUniform("lightPoint2.specular", bulbOnV);

	}

	m = matrixView;
	m = translate(m, bulbLoc2);
	m = scale(m, vec3(0.15f, 0.15f, 0.15f));
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	program.sendUniform("lightAmbient.color", vec3(0.1, 0.1, 0.1));
	program.sendUniform("materialAmbient", vec3(0.3, 0.3, 0.3));

	// Spot light ------------------------------------------------------------------------
	// Pendulum mechanics
	static float alpha = 0; // angular position (swing)
	static float omega = 0.7f; // angular velocity
	deltaTime = glm::min(deltaTime, 0.2f); // remove time distortions (longer than 0.2s)
	omega -= alpha * 0.05f * deltaTime; // Hooke's law: acceleration proportional to swing
	alpha += omega * deltaTime * 50; // motion equation: swing += velocity * delta-time

	// Ceiling lamp
	program.sendUniform("materialAmbient", ambientRoomLight);
	program.sendUniform("materialDiffuse", vec3(0.2, 0.15, 0.1));	
	m = matrixView;
	m = translate(m, spotLightLoc);
	m = rotate(m, radians(alpha), vec3(0.5, 0, 1));
	m = translate(m, vec3(0, -9, 0));
	mat4 m1 = m;
	m = translate(m, vec3(0, 9, 0));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	ceilingLamp.render(m);

	// Bulb
	program.sendUniform("lightAmbient.color", vec3(1, 1, 0.9));
	program.sendUniform("materialAmbient", vec3(1, 1, 0.9));
	program.sendUniform("materialDiffuse", vec3(0.6, 0.6, 0.6));
	m = m1;
	m = translate(m, vec3(0, 3.96f, 0));
	m = scale(m, vec3(0.14f, 0.14f, 0.14f));
	program.sendUniform("matrixModelView", m);
	program.sendUniform("spotLight.matrix", m);
	glutSolidSphere(1, 32, 32);

	program.sendUniform("lightAmbient.color", ambientRoomLight);
	program.sendUniform("materialAmbient", ambientRoomMaterial);

	// ---------------------------------------------------------------------------------------

	// get vertex and normal attribute location
	GLuint attribVertex = program.getAttribLocation("aVertex");
	GLuint attribNormal = program.getAttribLocation("aNormal");

	// setup material - yellow
	program.sendUniform("materialAmbient", ambientRoomMaterial);
	program.sendUniform("materialDiffuse", vec3(0.937255f, 0.611765f, 0.05098f));

	// triangle
	// enable the vertex and normal attributes
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);

	// activate the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// activate the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// triangle animation
	m = matrixView;
	m = translate(m, vec3(-1.8, 4.5f, 1.0f));					// basic translation 
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));					// and scale
	m = rotate(m, radians(180.f), vec3(0.0f, 0.0f, 1.0f));	// and rotation - upside down
	float theta = 0.2f * time;
	m = rotate(m, theta, vec3(0.0f, 1.0f, 0.0f));	// the rotation animation

	program.sendUniform("matrixModelView", m);

	// DRAW - using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	// disable the vertex and normal attributes
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);

	// setup material - pink
	program.sendUniform("materialDiffuse", vec3(0.8745f, 0.2f, 0.6f));

	// mug that sits on the triangle
	m = matrixView;
	m = translate(m, vec3(-1.8f, 4.5f, 1.0f));
	m = rotate(m, -theta, vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(1.2f, 1.2f, 1.2f));
	program.sendUniform("matrixModelView", m);
	mug.render(m);

	// Cat Animation set up
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexCat);

	std::vector<mat4> walkTransforms;
	std::vector<mat4> swatTransforms;
	
	if (catMoving)
	{
		animTime = time;
		cat.getAnimData(0, animTime, walkTransforms);
		program.sendUniform("bones", &walkTransforms[0], walkTransforms.size());
	}
	else if (catSwatting)
	{
		swattingAnimTime += deltaTime;
		cat.getAnimData(1, swattingAnimTime, swatTransforms);
		// Resets the animation - With help from Niam Wong!! he's a legend 
		if (swattingAnimTime * cat.getAnimation(1)->getTicksPerSecond() >= cat.getAnimation(1)->getDuration())
		{
			catSwatting = false;
			swattingAnimTime = 0;
		}
		program.sendUniform("bones", &swatTransforms[0], swatTransforms.size());
	}
	else
	{
		if(animTime>0) animTime=0.5;
		cat.getAnimData(0, animTime, walkTransforms);
		program.sendUniform("bones", &walkTransforms[0], walkTransforms.size());
	}
	// setup material - pink
	program.sendUniform("materialDiffuse", vec3(1.f, 1.f, 1.f));



	// get current camera rotation
	float yaw = getYaw(matrixView);

	m = matrixView;
	if (catCamOn)
	{
		currentCatPos = vec3(pos.x, 0.6f, pos.z);
		currentCatRot = yaw + radians(90.f);
	}
	m = translate(m, currentCatPos);
	m = rotate(m, currentCatRot, vec3(0, 1, 0));
	m = rotate(m, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	m = scale(m, vec3(0.00005f));
	cat.render(m);


	// RENDER THE PARTICLE SYSTEM
	programParticle.use();

	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexSmokeParticle);

	m = matrixView;
	programParticle.sendUniform("matrixModelView", m);

	// render the buffer
	GLint aVelocity = programParticle.getAttribLocation("aVelocity");
	GLint aStartTime = programParticle.getAttribLocation("aStartTime");
	glEnableVertexAttribArray(aVelocity); // velocity
	glEnableVertexAttribArray(aStartTime); // start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(aVelocity, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(aStartTime, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(aVelocity);
	glDisableVertexAttribArray(aStartTime);

	glActiveTexture(GL_TEXTURE0);
	glDepthMask(GL_TRUE);
	program.use();


}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// Pass 1: off-screen rendering
	glBindFramebufferEXT(GL_FRAMEBUFFER, idFBO);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;


	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	programParticle.sendUniform("time", time);

	// render the scene objects
	program.use();
	renderScene(matrixView, time, deltaTime);
	
	
	// Pass 2: on-screen rendering
	glActiveTexture(GL_TEXTURE0);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	programEffect.use();

	// setup ortographic projection
	programEffect.sendUniform("matrixProjection", ortho(0, 1, 0, 1, -1, 1));
	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);
	// setup identity matrix as the model-view
	programEffect.sendUniform("matrixModelView", mat4(1));

	GLuint attribVertex = programEffect.getAttribLocation("aVertex");
	GLuint attribTextCoord = programEffect.getAttribLocation("aTexCoord");

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTextCoord);
	glBindBuffer(GL_ARRAY_BUFFER, bufQuad);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(attribTextCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribTextCoord);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
	programParticle.sendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; catMoving = true; break;
	case 's': _acc.z = -accel; catMoving = true; break;
	case 'a': _acc.x = accel; catMoving = true; break;
	case 'd': _acc.x = -accel; catMoving = true; break;
	//case 'e': _acc.y = accel; break;
	//case 'q': _acc.y = -accel; break;
	case '1': bulbOff1 = !bulbOff1; break;
	case '2': bulbOff2 = !bulbOff2; break;
	case '3':
	{
		toonLinesOn = !toonLinesOn; 
		programEffect.sendUniform("toonLinesToggle", toonLinesOn);
		break;
	}
	case '4': catCamOn = !catCamOn; break;
	case 'f': catSwatting = true; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': 
	case 's': catMoving = false; _acc.z = _vel.z = 0; break;
	case 'a': 
	case 'd': catMoving = false; _acc.x = _vel.x = 0; break;
	//case 'q':
	//case 'e': _acc.y = _vel.y = 0; break;
	case 'f': catSwatting = false; swattingAnimTime = 0; break;
	
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: First Example");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

