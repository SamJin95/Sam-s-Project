/************************
SID: 1155046896 & 1155091693
Name :Jin Xiamu & Cheng Brian Wing Hang
**************************/

#define _CRT_SECURE_NO_WARNINGS
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <mmsystem.h>
#include <time.h>

using namespace std;
using glm::vec3;
using glm::mat4;

GLint PID;

const int Objectnum = 20;
const int Texturenum = 20;

//constant bit flags for indicating status of objects
const int seeable = 2;
const int hittable = 4;
const int outside = 8;


//storing the index of the entities
int SpaceCraft;

int Planet1, Planet2;
int RingOri, RingStop;
int RockOri, RockStop;
int lightsource, lightsource2;
float planevelocity = 0.4f;
const float rockvelocity = 0.02f;
const float planeRotAng = 0.002f;
const float ringspin = 0.002f;
const float rockspin = 0.001f;

int Planet1_ID;
int Planet1_texture;
vec3 Planet1_location;
glm::mat4 Planet1_transform;
float Planet1_colRadius;
int Planet1_colHandler;
int Planet1_status;
glm::mat4 Planet1_scale;


//stroring gobal game state variables

glm::vec3 camPos = glm::vec3(0.0, 20.0, 20.0);
float ax = 0.0f;
vec3 lightPosition;
vec3 lightPosition2;
float diffuse = 0.8; //diffuse light intensity
float specular = 0.8; //specular light intensity
float diffuse2 = 0.8; //diffuse light intensity
float specular2 = 0.8;

//vao vbos
GLuint textureID[Texturenum];
GLuint VertexArrayID[Objectnum];
GLuint vertexbuffer[Objectnum];
GLuint uvbuffer[Objectnum];
GLuint normalbuffer[Objectnum];
GLuint drawSize[Objectnum];

//entities system
typedef struct structure {
	int ObjId;
	int texture;
	float collisionRadius;
	int collisionHandler;
	vec3 location;
	glm::mat4 transform; //for aditional scaling or rotational visual transform
	int status;
	float orbitRadius;
	float radian;
	float orbitVOffset;
	vec3 orbitCentre;
	glm::mat4 scale;
}structure;

structure* structureList[400];
int structureCount = 0;

//custom function prototypes
void bufferObject(int objectID, const char * Path);
void LightSetup();
void drawTextureObject(int index, int Texture, glm::mat4 transformMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
int initstructure(int ObjId, int texture, int x, int y, int z, float radius, int collisionHandler);
void drawstructure(structure* e, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
int checkCollision(structure * e1, structure * e2);
int handleCollision(structure * primary, structure * secondary);
int handleExit(structure * primary, structure * secondary);
int initstructure(int ObjId, int texture, int x, int y, int z, glm::mat4 transform, float radius, int collisionHandler);
int initRock(float radiusMin, float radiusMax, float vOffset, vec3 centre);



//a series utilities for setting shader parameters 
void setMat4(const std::string &name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(PID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void setVec4(const std::string &name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(PID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string &name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(PID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string &name, float value)
{
	glUniform1f(glGetUniformLocation(PID, name.c_str()), value);
}
void setInt(const std::string &name, int value)
{
	glUniform1i(glGetUniformLocation(PID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint PID)
{
	return checkStatus(PID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

//need installCubeShader *Brian

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	//adapter[0] = vertexShaderCode;
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	PID = glCreateProgram();
	glAttachShader(PID, vertexShaderID);
	glAttachShader(PID, fragmentShaderID);
	glLinkProgram(PID);

	if (!checkProgramStatus(PID))
		return;



	glUseProgram(PID);
}



void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) { // esc
		exit(0);
	}
	if (key == 'j') {
		specular -= 0.1;
	}
	if (key == 'k') {
		specular += 0.1;
	}
	if (key == 'w')
		lightPosition = lightPosition + vec3(0.0f, 0.0f, 3.0f);
	if (key == 'a')
		lightPosition = lightPosition + vec3(-3.0f, 0.0f, 0.0f);
	if (key == 's')
		lightPosition = lightPosition + vec3(0.0f, 0.0f, -3.0f);
	if (key == 'd')
		lightPosition = lightPosition + vec3(3.0f, 0.0f, 0.0f);
	if (key == 'u') {
			diffuse -= 0.1;
	}
    if (key == 'i') {
			diffuse += 0.1;
	}
	
}

void move(int key, int x, int y) {
	if (key == GLUT_KEY_RIGHT) {
		structureList[SpaceCraft]->location =
			vec3(glm::translate(glm::mat4(), vec3(structureList[SpaceCraft]->location)) * structureList[SpaceCraft]->transform *  glm::vec4(planevelocity, 0.0f, 0.0f, 1.0f));
	}
	if (key == GLUT_KEY_LEFT) {
		structureList[SpaceCraft]->location =
			vec3(glm::translate(glm::mat4(), vec3(structureList[SpaceCraft]->location)) * structureList[SpaceCraft]->transform *  glm::vec4(-planevelocity, 0.0f, 0.0f, 1.0f));
	}
	if (key == GLUT_KEY_UP) {
		structureList[SpaceCraft]->location =
			vec3(glm::translate(glm::mat4(), vec3(structureList[SpaceCraft]->location)) * structureList[SpaceCraft]->transform *  glm::vec4(0.0f, 0.0f, -planevelocity, 1.0f));
	}
	if (key == GLUT_KEY_DOWN) {
		structureList[SpaceCraft]->location =
			vec3(glm::translate(glm::mat4(), vec3(structureList[SpaceCraft]->location)) * structureList[SpaceCraft]->transform *  glm::vec4(0.0f, 0.0f, planevelocity, 1.0f));
	}
}
int oldx = 0;
float r = 0.0f;
void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	int xoffset = x - glutGet(GLUT_WINDOW_WIDTH) / 2;
	float angleX = xoffset / (glutGet(GLUT_WINDOW_WIDTH) / 2.0) * 90.0;
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	structureList[SpaceCraft]->transform *= glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(0.0f, 1.0f, 0.0f));

}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}



GLuint loadBMP_custom(const char * imagepath) {


	printf("Read bmp %s\n", imagepath);
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	fread(header, 1, 54, file);

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;


	glGenTextures(1, &textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// OpenGL has now copied the data. Free our own version
	delete[] data;


	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}


void sendDataToOpenGL()
{
	//Generate buffers
	glGenVertexArrays(Objectnum, VertexArrayID);
	glGenBuffers(Objectnum, vertexbuffer);
	glGenBuffers(Objectnum, uvbuffer);
	glGenBuffers(Objectnum, normalbuffer);

	//Load texture
	//textureID[0] = loadBMP_custom("objecttexture\\jeep_texture.bmp");
	//textureID[1] = loadBMP_custom("objecttexture\\block_texture.bmp");
	textureID[2] = loadBMP_custom("objecttexture\\texture\\spacecraftTexture.bmp");
	textureID[3] = loadBMP_custom("objecttexture\\texture\\WonderStarTexture.bmp");
	textureID[4] = loadBMP_custom("objecttexture\\texture\\RockTexture.bmp");
	textureID[5] = loadBMP_custom("objecttexture\\texture\\earthTexture.bmp");
	//textureID[6] = loadBMP_custom("objecttexture\\white_texture.bmp");
	textureID[7] = loadBMP_custom("objecttexture\\texture\\ringTexture.bmp");
	textureID[8] = loadBMP_custom("objecttexture\\texture\\RockTexture.bmp");
	textureID[9] = loadBMP_custom("objecttexture\\texture\\RockTexture.bmp");
	//Load obj files
	//bufferObject(0, "objecttexture\\block.obj");
	bufferObject(1, "objecttexture\\spaceCraft.obj");
	//bufferObject(2, "objecttexture\\plane.obj");
	bufferObject(3, "objecttexture\\rock.obj");
	bufferObject(4, "objecttexture\\planet.obj");
	bufferObject(5, "objecttexture\\ring.obj");
	bufferObject(6, "objecttexture\\planet.obj");

	glutSetCursor(GLUT_CURSOR_NONE);
}

mat4 LookAtRH(vec3 eye, vec3 target, vec3 up)
{
	vec3 zaxis = glm::normalize(eye - target);    // The "forward" vector.
	vec3 xaxis = glm::normalize(cross(up, zaxis));// The "right" vector.
	vec3 yaxis = cross(zaxis, xaxis);     // The "up" vector.

										  // Create a 4x4 orientation matrix from the right, up, and forward vectors
										  // This is transposed which is equivalent to performing an inverse 
										  // if the matrix is orthonormalized (in this case, it is).
	mat4 orientation = {
		glm::vec4(xaxis.x, yaxis.x, zaxis.x, 0),
		glm::vec4(xaxis.y, yaxis.y, zaxis.y, 0),
		glm::vec4(xaxis.z, yaxis.z, zaxis.z, 0),
		glm::vec4(0,       0,       0,     1)
	};

	// Create a 4x4 translation matrix.
	// The eye position is negated which is equivalent
	// to the inverse of the translation matrix. 
	// T(v)^-1 == T(-v)
	mat4 translation = {
		glm::vec4(1,      0,      0,   0),
		glm::vec4(0,      1,      0,   0),
		glm::vec4(0,      0,      1,   0),
		glm::vec4(-eye.x, -eye.y, -eye.z, 1)
	};

	// Combine the orientation and translation to compute 
	// the final view matrix. Note that the order of 
	// multiplication is reversed because the matrices
	// are already inverted.
	return (orientation * translation);
}

void drawPlanet1(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(Planet1_location.x, Planet1_location.y, Planet1_location.z)) * Planet1_transform * Planet1_scale;
	drawTextureObject(Planet1_ID, Planet1_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}



void paintGL(void)
{
	//General Upkeepings
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ax += 0.1;
	LightSetup();

	//Set transformation matrix

	//set up projection matrix
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective((float)glm::radians(90.0f), 1.0f / 1.0f, 0.5f, 200.0f);

	//update structure state and location etc
	//make a sphere follow the light source
	structureList[lightsource]->location = lightPosition;
	structureList[lightsource2]->location = lightPosition2;
	//make the camera follow the plane
	//camPos = vec3(glm::translate(glm::mat4(), vec3(0.0f, +10.0f, +10.0f)) * glm::vec4(structureList[SpaceCraft]->location,0.0));
	camPos = vec3(structureList[SpaceCraft]->transform * glm::translate(glm::mat4(), vec3(0.0f, +5.0f, +5.0f)) * glm::vec4(1.0));
	camPos = vec3(glm::translate(glm::mat4(), structureList[SpaceCraft]->location) * glm::vec4(camPos, 1.0));
	//camPos = vec3(glm::translate(glm::mat4(), structureList[SpaceCraft]->location) * structureList[SpaceCraft]->transform *  glm::vec4(0.0f, +10.0f, +10.0f,1.0));
	//camPos = vec3(x, x, x);

	//send eye position
	GLint eyePosUniformLocation = glGetUniformLocation(PID, "eyePositionWorld");
	glm::vec4 campos4v = glm::vec4(camPos, 0.0);
	glUniform4fv(eyePosUniformLocation, 1, &campos4v[0]);

	//set up view matrix
	glm::mat4 viewMatrix = LookAtRH(camPos, structureList[SpaceCraft]->location, vec3(0.0f, 1.0f, 0.0f));//glm::mat4(1.0f);
																										 //viewMatrix = glm::translate(mat4(), -camPos) * viewMatrix;
																										 //viewMatrix = glm::inverse(structureList[Plane]->transform) * viewMatrix;//glm::rotate(mat4(), glm::radians(camY), glm::vec3(0.0f, 0.0f, 1.0f));
																										 //viewMatrix = glm::rotate(mat4(), glm::radians(camX), glm::vec3(1.0f, 0.0f, 0.0f)) * viewMatrix;




																										 //make the rock oribts
	for (int i = RockOri; i <= RockStop; i++) {
		structureList[i]->radian += rockvelocity / structureList[i]->orbitRadius;
		structureList[i]->location = vec3(glm::translate(glm::mat4(), structureList[i]->orbitCentre)
			* glm::translate(glm::mat4(), vec3(0.0f, structureList[i]->orbitVOffset, 0.0f))
			* glm::rotate(mat4(), structureList[i]->radian, glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::mat4(), vec3(structureList[i]->orbitRadius, 0.0f, 0.0f))
			* glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		structureList[i]->transform = glm::rotate(mat4(), rockspin, glm::vec3(0.0f, 1.0f, 0.0f)) *  structureList[i]->transform;
	}

	//rotate the planets
	Planet1_transform = glm::rotate(mat4(), planeRotAng, glm::vec3(0.0f, 1.0f, 0.0f)) *  Planet1_transform;
	structureList[Planet2]->transform = glm::rotate(mat4(), planeRotAng, glm::vec3(0.0f, 1.0f, 0.0f)) *  structureList[Planet2]->transform;

	for (int i = RingOri; i <= RingStop; i++) {
		structureList[i]->transform = glm::rotate(mat4(), ringspin, glm::vec3(0.0f, 1.0f, 0.0f)) *  structureList[i]->transform;
	}

	//centralised drawing and collision detection for each structure
	for (int i = 0; i < structureCount; i++) {
		if (structureList[i]->status & seeable)
			drawstructure(structureList[i], viewMatrix, projectionMatrix);
		if (structureList[i]->status & hittable) {
			for (int j = i; j < structureCount; j++) {
				if (structureList[j]->status & hittable)
					checkCollision(structureList[i], structureList[j]);
			}
		}

	}

	if (Planet1_status & seeable) {
		drawPlanet1(viewMatrix, projectionMatrix);
	}

	//post drawing upkeeping
	glFlush();
	glutPostRedisplay();
}

void LightSetup()
{
	//Set up lighting information for source 1
	GLint lightPositonUniformLocation = glGetUniformLocation(PID, "lightPositionWorld");
	glUniform3fv(lightPositonUniformLocation, 1, &lightPosition[0]);

	GLint ambLightUniformLocation = glGetUniformLocation(PID, "ambientLight");
	glm::vec4 ambientLight(0.25f, 0.25f, 0.25f, 1.0f);
	glUniform4fv(ambLightUniformLocation, 1, &ambientLight[0]);


	GLint diffuseLightUniformLocation = glGetUniformLocation(PID, "diffuseLight");
	glm::vec4 diffuseLight(diffuse, diffuse, diffuse, 0.0f);
	glUniform4fv(diffuseLightUniformLocation, 1, &diffuseLight[0]);

	GLint specularLightUniformLocation = glGetUniformLocation(PID, "specularLight");
	glm::vec4 specularLight(specular, specular, specular, 0.0f);
	glUniform4fv(specularLightUniformLocation, 1, &specularLight[0]);

	//light souce 2
	GLint lightPositonUniformLocation2 = glGetUniformLocation(PID, "lightPositionWorld2");
	glUniform3fv(lightPositonUniformLocation2, 1, &lightPosition2[0]);

	GLint diffuseLightUniformLocation2 = glGetUniformLocation(PID, "diffuseLight2");
	glm::vec4 diffuseLight2(diffuse2 / 2, diffuse2 / 2, diffuse2, 0.0f);
	glUniform4fv(diffuseLightUniformLocation2, 1, &diffuseLight2[0]);

	GLint specularLightUniformLocation2 = glGetUniformLocation(PID, "specularLight2");
	glm::vec4 specularLight2(specular2 / 2, specular2 / 2, specular2 / 2, 0.0f);
	glUniform4fv(specularLightUniformLocation2, 1, &specularLight2[0]);
}


void setupCubeLight()
{
	//Set up lighting information
	GLint lightPositonUniformLocation = glGetUniformLocation(PID, "lightPositionWorld");
	glUniform3fv(lightPositonUniformLocation, 1, &lightPosition[0]);

	GLint ambLightUniformLocation = glGetUniformLocation(PID, "ambientLight");
	glm::vec4 ambientLight(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4fv(ambLightUniformLocation, 1, &ambientLight[0]);


	GLint diffuseLightUniformLocation = glGetUniformLocation(PID, "diffuseLight");
	//glm::vec4 diffuseLight(diff, diff, diff, 1.0f);
	glm::vec4 diffuseLight(0, 0, 0, 1.0f);
	glUniform4fv(diffuseLightUniformLocation, 1, &diffuseLight[0]);

	GLint specularLightUniformLocation = glGetUniformLocation(PID, "specularLight");
	glm::vec4 specularLight(0, 0, 0, 1.0f);
	glUniform4fv(specularLightUniformLocation, 1, &specularLight[0]);
}

void bufferObject(int objectID, const char* Path) {
	//read from an obj file and send its data to the VAO VBOS at the designated ID
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(Path, vertices, uvs, normals);

	glBindVertexArray(VertexArrayID[objectID]);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[objectID]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
		&vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[objectID]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0],
		GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[objectID]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0],
		GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	drawSize[objectID] = vertices.size();
}



void drawTextureObject(int index, int Texture, glm::mat4 transformMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	//function to help draw object, given obj ID, texture and various transform matrix
	glBindVertexArray(VertexArrayID[index]);

	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(PID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &transformMatrix[0][0]);

	GLint viewUniformLocation = glGetUniformLocation(PID, "viewMatrix");
	glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);

	GLint projectionMatrixUniformLocation = glGetUniformLocation(PID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	GLuint localTextureID = glGetUniformLocation(PID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID[Texture]);
	glUniform1i(localTextureID, 0);

	glDrawArrays(GL_TRIANGLES, 0, drawSize[index]);

}

int initstructure(int ObjId, int texture, int x, int y, int z, float radius, int collisionHandler) {
	return initstructure(ObjId, texture, x, y, z, glm::mat4(1.0f), radius, collisionHandler);
}

int initstructure(int ObjId, int texture, int x, int y, int z, glm::mat4 transform, float radius, int collisionHandler)
{
	//initialise entities
	structure* e = (structure*)malloc(sizeof(structure));
	e->ObjId = ObjId;
	e->texture = texture;
	e->location = vec3(x, y, z);
	e->transform = transform;
	e->collisionRadius = radius;
	e->collisionHandler = collisionHandler;
	structureList[structureCount] = e;
	structureCount++;
	e->status = seeable | hittable;
	e->scale = glm::mat4(1.0f);
	return structureCount - 1;
}


int initRock(float radiusMin, float radiusMax, float vOffset, vec3 centre) {
	float turnx, turny, turnz;
	turnx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	turny = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	turnz = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	int i = initstructure(3, 4, 0, 0, 0, 2, 2);
	structureList[i]->transform = glm::rotate(mat4(), glm::radians(turnx), glm::vec3(1.0f, 0.0f, 0.0f))
		*glm::rotate(mat4(), glm::radians(turny), glm::vec3(0.0f, 1.0f, 0.0f))
		*glm::rotate(mat4(), glm::radians(turnz), glm::vec3(0.0f, 0.0f, 1.0f));

	structureList[i]->orbitRadius = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * (radiusMax - radiusMin) + radiusMin;
	structureList[i]->radian = glm::radians(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360);
	structureList[i]->orbitVOffset = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 2 * vOffset - vOffset;
	structureList[i]->orbitCentre = centre;

	structureList[i]->location = vec3(glm::translate(glm::mat4(), structureList[i]->orbitCentre)
		* glm::translate(glm::mat4(), vec3(0.0f, structureList[i]->orbitVOffset, 0.0f))
		* glm::rotate(mat4(), structureList[i]->radian, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::mat4(), vec3(structureList[i]->orbitRadius, 0.0f, 0.0f))
		* glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	return i;
}

void drawstructure(structure* e, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(e->location.x, e->location.y, e->location.z)) * e->transform * e->scale;
	drawTextureObject(e->ObjId, e->texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	//need run installCubeShader ****Brian
	sendDataToOpenGL();
}


void initialiseEntities() {

	srand(time(NULL));
	SpaceCraft = initstructure(1, 2, 0, 10, 0, 4.5f, 1); //the plane
														 //structureList[Plane]->scale = glm::scale(glm::mat4(), glm::vec3(0.01, 0.01, 0.01));
	//Planet1 = initstructure(4, 5, -100, 10, 0, 23.0f, 2); // the earth
	Planet1_ID = 4;
	Planet1_texture = 5;
	Planet1_location = vec3(-100, 10, 0);
	glm::mat4 Plaent1_transform = glm::mat4(1.0f);
	Planet1_colRadius = 23.0f;
	Planet1_colHandler = 2;
	Planet1_status = seeable | hittable;
	Planet1_scale = mat4(1.0f);


	Planet2 = initstructure(4, 3, +100, 10, 0, 23.0f, 2); // the wonder planet
	lightsource = initstructure(6, 9, 0, 30, 0, 2.0f, 0); // sphere that indicate light position
	lightPosition = vec3(0.0f, 40.0f, 0.0f);
	structureList[lightsource]->scale = glm::scale(glm::mat4(), glm::vec3(0.15, 0.15, 0.15));

	lightsource2 = initstructure(6, 9, 0, 30, 0, 2.0f, 0); // sphere that indicate light position
	lightPosition2 = vec3(50.0f, 20.0f, 0.0f);
	structureList[lightsource2]->scale = glm::scale(glm::mat4(), glm::vec3(0.15, 0.15, 0.15));


	RockOri = structureCount; //initialise all the rocks
	for (int i = 0; i < 200; i++) {
		RockStop = initRock(60.0f, 90.0f, 7.0f, structureList[Planet2]->location);
	}

	RingOri = structureCount;//initialise all the rings
	for (int i = 0; i < 3; i++) {
		RingStop = initstructure(5, 7, 0, +10, -10 + -30 * i, glm::rotate(mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), 2.0f, 3);
	}
	//glm::scale(glm::mat4(), vec3(0.15,0.15,0.15))	*

}

int checkCollision(structure* e1, structure* e2) {
	vec3 distance = e1->location - e2->location;
	if (glm::length(distance) < e1->collisionRadius + e2->collisionRadius) {
		handleCollision(e1, e2);
		handleCollision(e2, e1);
		return 1;
	}
	else if (e1->status & outside || e2->status & outside) {
		handleExit(e1, e2);
		handleExit(e2, e1);
		return 2;
	}
	return 0;
}

int handleCollision(structure* primary, structure* secondary) {
	if (primary->collisionHandler == 1 && secondary->collisionHandler == 2) {
		secondary->status = 0;
	}
	if (primary->collisionHandler == 1 && secondary->collisionHandler == 3) {
		secondary->status = secondary->status | outside;
		secondary->texture = 8;
		primary->texture = 8;
		secondary->collisionHandler = 4;
	}
	if (primary->collisionHandler == 1 && secondary->collisionHandler == 6) {
		secondary->status = 0;
		secondary->collisionHandler = 0;
		planevelocity += 0.7f;
	}
	return 0;
}

int handleExit(structure* primary, structure* secondary) {
	if (primary->collisionHandler == 1 && secondary->collisionHandler == 4) {
		secondary->status = secondary->status ^ outside;
		secondary->texture = 7;
		primary->texture = 2;
		secondary->collisionHandler = 3;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(900, 900);
	glutCreateWindow("Assignment 2");
	initialiseEntities();
	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glutDisplayFunc(paintGL);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;

}