/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Project)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155092438 & 1155093276
Student Name: Jin Xiamu & Cheng Brian Wing Hang
*********************************************************/

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

const int OBJECT_NUM = 20;
const int TEXTURE_NUM = 20;

//constant bit flags for indicating status of objects
const int VISIABLE = 2;
const int COLLIDABLE = 4;
const int OUTSIDE = 8;


//storing the index of the entities
float SC_SPEED = 0.6f;
const float ROCK_SPEED = 0.02f;
const float SC_ANG = 0.002f;
const float RING_ANG = 0.002f;
const float ROCK_ANG = 0.001f;

int Planet1_ID, Planet2_ID, SC_ID, LightSource1_ID, LightSource2_ID, Ring_ID, Rock_ID;
int Planet1_texture, Planet2_texture, SC_texture, LightSource1_texture, LightSource2_texture, Ring_texture[4], Rock_texture;
vec3 Planet1_location, Planet2_location, SC_location, LightSource1_location, LightSource2_location, Ring_location[4], Rock_location[200];
glm::mat4 Planet1_transform, Planet2_transform, SC_transform, LightSource1_transform, LightSource2_transform, Ring_transform, Rock_transform[200];
float Planet1_colRadius, Planet2_colRadius, SC_colRadius, Ring_colRadius, Rock_colRadius;
int Planet1_colHandler, Planet2_colHandler, SC_colHandler, Ring_colHandler[4], Rock_colHandler;
int Planet1_status, Planet2_status, SC_status, Ring_status[4], Rock_status[200];
glm::mat4 Planet1_scale, Planet2_scale, SC_scale, LightSource1_scale, LightSource2_scale, Ring_scale, Rock_scale;
float Rock_orbitRadius[200], Rock_radian[200], Rock_orbitVoffset[200];
vec3 Rock_orbitCentre[200];

//stroring global game state variables

glm::vec3 camPos = glm::vec3(0.0, 20.0, 20.0);
float ax = 0.0f;
float diffuse = 0.8; //diffuse light intensity
float specular = 0.8; //specular light intensity
float diffuse2 = 0.8; //diffuse light intensity
float specular2 = 0.8;

GLuint textureID[TEXTURE_NUM];
GLuint VertexArrayID[OBJECT_NUM];
GLuint vertexbuffer[OBJECT_NUM];
GLuint uvbuffer[OBJECT_NUM];
GLuint normalbuffer[OBJECT_NUM];
GLuint drawSize[OBJECT_NUM];

void bufferObject(int objectID, const char * Path);
void LightSetup();
void drawTextureObject(int index, int Texture, glm::mat4 transformMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawSpaceCraft(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawPlanet1(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawPlanet2(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawLightSource1(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawLightSource2(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawRock(int i, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void drawRing(int i, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
int check_SC_Planet1_Collision();
int check_SC_Planet2_Collision();
int check_SC_Rock_Collision(int i);
int check_SC_Ring_Collision(int i);
int initRock(int i, float min_radius, float max_raduis, float v_offset, vec3 orbitCentre);



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

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
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
		LightSource1_location = LightSource1_location + vec3(0.0f, 0.0f, 3.0f);
	if (key == 'a')
		LightSource1_location = LightSource1_location + vec3(-3.0f, 0.0f, 0.0f);
	if (key == 's')
		LightSource1_location = LightSource1_location + vec3(0.0f, 0.0f, -3.0f);
	if (key == 'd')
		LightSource1_location = LightSource1_location + vec3(3.0f, 0.0f, 0.0f);
	if (key == 'u') {
		diffuse -= 0.1;
	}
    if (key == 'i') {
		diffuse += 0.1;
	}

	if (key == 'q') {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, SC_SPEED, 0.0f, 1.0f));
	}
	if (key == 'e') {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, -SC_SPEED, 0.0f, 1.0f));
	}
	
}

void SpecialKeys(int key, int x, int y) {
	if (key == GLUT_KEY_RIGHT) {
        SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(SC_SPEED, 0.0f, 0.0f, 1.0f));
	}
	if (key == GLUT_KEY_LEFT) {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(-SC_SPEED, 0.0f, 0.0f, 1.0f));
	}
	if (key == GLUT_KEY_UP) {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, 0.0f, -SC_SPEED, 1.0f));
	}
	if (key == GLUT_KEY_DOWN) {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, 0.0f, SC_SPEED, 1.0f));
	}
}
 
void PassiveMouse(int x, int y)
{
	int xoffset = x - 450;
	float angleX = xoffset / 450.0 * 90.0;
	glutWarpPointer(450, 450);
	SC_transform *= glm::rotate(glm::mat4(1.0f), glm::radians(-angleX), glm::vec3(0.0f, 0.8f, 0.0f));
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Load OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Cannot open file\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
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
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
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
				printf("Cannot read File\n");
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

			char wrongbuff[1000];
			fgets(wrongbuff, 1000, file);
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
	glGenVertexArrays(OBJECT_NUM, VertexArrayID);
	glGenBuffers(OBJECT_NUM, vertexbuffer);
	glGenBuffers(OBJECT_NUM, uvbuffer);
	glGenBuffers(OBJECT_NUM, normalbuffer);

	textureID[0] = loadBMP_custom("objecttexture\\texture\\spacecraftTexture.bmp");
	textureID[1] = loadBMP_custom("objecttexture\\texture\\WonderStarTexture.bmp");
	textureID[2] = loadBMP_custom("objecttexture\\texture\\RockTexture.bmp");
	textureID[3] = loadBMP_custom("objecttexture\\texture\\earthTexture.bmp");
	textureID[4] = loadBMP_custom("objecttexture\\texture\\ringTexture.bmp");

	bufferObject(1, "objecttexture\\spaceCraft.obj");
	bufferObject(3, "objecttexture\\rock.obj");
	bufferObject(4, "objecttexture\\planet.obj");
	bufferObject(5, "objecttexture\\ring.obj");
	bufferObject(6, "objecttexture\\planet.obj");

	glutSetCursor(GLUT_CURSOR_NONE);
}

void paintGL(void)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ax += 0.1;
	LightSetup();
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective((float)glm::radians(90.0f), 1.0f / 1.0f, 0.5f, 200.0f);
	camPos = vec3(SC_transform * glm::translate(glm::mat4(), vec3(0.0f, +5.0f, +5.0f)) * glm::vec4(1.0));
	camPos = vec3(glm::translate(glm::mat4(), SC_location) * glm::vec4(camPos, 1.0));

	GLint eyePosUniformLocation = glGetUniformLocation(PID, "LookLocation");
	glm::vec4 campos4v = glm::vec4(camPos, 0.0);
	glUniform4fv(eyePosUniformLocation, 1, &campos4v[0]);

	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(camPos), glm::vec3(SC_location), glm::vec3(0.0f, 1.0f, 0.0f));


	for (int i = 0; i < 200; i++) {
		Rock_radian[i] += ROCK_SPEED / Rock_orbitRadius[i];
		Rock_location[i] = vec3(glm::translate(glm::mat4(), Rock_orbitCentre[i])
			* glm::translate(glm::mat4(), vec3(0.0f, Rock_orbitVoffset[i], 0.0f))
			* glm::rotate(mat4(), Rock_radian[i], glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::mat4(), vec3(Rock_orbitRadius[i], 0.0f, 0.0f))
			* glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		Rock_transform[i] = glm::rotate(mat4(), ROCK_ANG, glm::vec3(0.0f, 1.0f, 0.0f)) *  Rock_transform[i];
	}

	Planet1_transform = glm::rotate(mat4(), SC_ANG, glm::vec3(0.0f, 1.0f, 0.0f)) *  Planet1_transform;
	Planet2_transform = glm::rotate(mat4(), SC_ANG, glm::vec3(0.0f, 1.0f, 0.0f)) *  Planet2_transform;

	Ring_transform = glm::rotate(mat4(), RING_ANG, glm::vec3(0.0f, 1.0f, 0.0f)) *  Ring_transform;

	if (SC_status & VISIABLE) {
		drawSpaceCraft(viewMatrix, projectionMatrix);
	}

	if (Planet1_status & VISIABLE) {
		drawPlanet1(viewMatrix, projectionMatrix);
	}
	if (Planet1_status & COLLIDABLE) {
		check_SC_Planet1_Collision();
	}

	if (Planet2_status & VISIABLE) {
		drawPlanet2(viewMatrix, projectionMatrix);
	}
	if (Planet2_status & COLLIDABLE) {
		check_SC_Planet2_Collision();
	}
	drawLightSource1(viewMatrix, projectionMatrix);
	drawLightSource2(viewMatrix, projectionMatrix);

	for (int i = 0; i < 200; i++) {
		if (Rock_status[i] & VISIABLE) {
			drawRock(i, viewMatrix, projectionMatrix);
		}
		if (Rock_status[i] & COLLIDABLE) {
			check_SC_Rock_Collision(i);
		}
	}

	for (int i = 0; i < 4; i++) {
		if (Ring_status[i] & VISIABLE) {
			drawRing(i, viewMatrix, projectionMatrix);
		}
		if (Ring_status[i] & COLLIDABLE) {
			check_SC_Ring_Collision(i);
		}
	}

	glFlush();
	glutPostRedisplay();
}

void LightSetup()
{
	GLint lightPositonUniformLocation = glGetUniformLocation(PID, "LightLocation");
	glUniform3fv(lightPositonUniformLocation, 1, &LightSource1_location[0]);

	GLint ambLightUniformLocation = glGetUniformLocation(PID, "ambiLight");
	glm::vec4 ambiLight(0.25f, 0.25f, 0.25f, 1.0f);
	glUniform4fv(ambLightUniformLocation, 1, &ambiLight[0]);


	GLint difLightUniformLocation = glGetUniformLocation(PID, "difLight");
	glm::vec4 difLight(diffuse, diffuse, diffuse, 0.0f);
	glUniform4fv(difLightUniformLocation, 1, &difLight[0]);

	GLint speLightUniformLocation = glGetUniformLocation(PID, "speLight");
	glm::vec4 speLight(specular, specular, specular, 0.0f);
	glUniform4fv(speLightUniformLocation, 1, &speLight[0]);

	GLint lightPositonUniformLocation2 = glGetUniformLocation(PID, "LightLocation2");
	glUniform3fv(lightPositonUniformLocation2, 1, &LightSource2_location[0]);

	GLint difLightUniformLocation2 = glGetUniformLocation(PID, "difLight2");
	glm::vec4 difLight2(diffuse2 / 2, diffuse2 / 2, diffuse2, 0.0f);
	glUniform4fv(difLightUniformLocation2, 1, &difLight2[0]);

	GLint speLightUniformLocation2 = glGetUniformLocation(PID, "speLight2");
	glm::vec4 speLight2(specular2 / 2, specular2 / 2, specular2 / 2, 0.0f);
	glUniform4fv(speLightUniformLocation2, 1, &speLight2[0]);
}


void setupCubeLight()
{
	//Set up lighting information
	GLint lightPositonUniformLocation = glGetUniformLocation(PID, "LightLocation");
	glUniform3fv(lightPositonUniformLocation, 1, &LightSource1_location[0]);

	GLint ambLightUniformLocation = glGetUniformLocation(PID, "ambiLight");
	glm::vec4 ambiLight(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4fv(ambLightUniformLocation, 1, &ambiLight[0]);


	GLint difLightUniformLocation = glGetUniformLocation(PID, "difLight");
	//glm::vec4 difLight(diff, diff, diff, 1.0f);
	glm::vec4 difLight(0, 0, 0, 1.0f);
	glUniform4fv(difLightUniformLocation, 1, &difLight[0]);

	GLint speLightUniformLocation = glGetUniformLocation(PID, "speLight");
	glm::vec4 speLight(0, 0, 0, 1.0f);
	glUniform4fv(speLightUniformLocation, 1, &speLight[0]);
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

int initRock(int i, float min_radius, float max_radius, float v_offset, vec3 orbitCentre) {
	float ang_X, ang_Y, ang_Z;
	ang_X = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	ang_Y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	ang_Z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360;
	//int i = initstructure(3, 4, 0, 0, 0, 2, 2);
	Rock_transform[i] = glm::rotate(mat4(), glm::radians(ang_X), glm::vec3(1.0f, 0.0f, 0.0f))
		*glm::rotate(mat4(), glm::radians(ang_Y), glm::vec3(0.0f, 1.0f, 0.0f))
		*glm::rotate(mat4(), glm::radians(ang_Z), glm::vec3(0.0f, 0.0f, 1.0f));

	Rock_orbitRadius[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * (max_radius - min_radius) + min_radius;
	Rock_radian[i] = glm::radians(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360);
	Rock_orbitVoffset[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 2 * v_offset - v_offset;
	Rock_orbitCentre[i] = orbitCentre;

	Rock_location[i] = vec3(glm::translate(glm::mat4(), Rock_orbitCentre[i])
		* glm::translate(glm::mat4(), vec3(0.0f, Rock_orbitVoffset[i], 0.0f))
		* glm::rotate(mat4(), Rock_radian[i], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::mat4(), vec3(Rock_orbitRadius[i], 0.0f, 0.0f))
		* glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	return 0;
}

void drawPlanet1(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(Planet1_location.x, Planet1_location.y, Planet1_location.z)) * Planet1_transform * Planet1_scale;
	drawTextureObject(Planet1_ID, Planet1_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawPlanet2(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(Planet2_location.x, Planet2_location.y, Planet2_location.z)) * Planet2_transform * Planet2_scale;
	drawTextureObject(Planet2_ID, Planet2_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawSpaceCraft(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(SC_location.x, SC_location.y, SC_location.z)) * SC_transform * SC_scale;
	drawTextureObject(SC_ID, SC_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawLightSource1(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(LightSource1_location.x, LightSource1_location.y, LightSource1_location.z)) * LightSource1_transform * LightSource1_scale;
	drawTextureObject(LightSource1_ID, LightSource1_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawLightSource2(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(LightSource2_location.x, LightSource2_location.y, LightSource2_location.z)) * LightSource2_transform * LightSource2_scale;
	drawTextureObject(LightSource2_ID, LightSource2_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawRock(int i, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(Rock_location[i].x, Rock_location[i].y, Rock_location[i].z)) * Rock_transform[i] * Rock_scale;
	drawTextureObject(Rock_ID, Rock_texture, modelTransformMatrix, viewMatrix, projectionMatrix);
}

void drawRing(int i, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(mat4(), glm::vec3(Ring_location[i].x, Ring_location[i].y, Ring_location[i].z)) * Ring_transform * Ring_scale;
	drawTextureObject(Ring_ID, Ring_texture[i], modelTransformMatrix, viewMatrix, projectionMatrix);
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}


void initialiseEntities() {

	srand(time(NULL));
	SC_ID = 1;
	SC_texture = 0;
	SC_location = vec3(0, 10, 0);
	SC_transform = glm::mat4(1.0f);
	SC_colRadius = 4.5f;
	SC_colHandler = 1;
	SC_status = VISIABLE | COLLIDABLE;
	SC_scale = mat4(1.0f);

	Planet1_ID = 4;
	Planet1_texture = 3;
	Planet1_location = vec3(-100, 10, 0);
	Planet1_transform = glm::mat4(1.0f);
	Planet1_colRadius = 23.0f;
	Planet1_colHandler = 2;
	Planet1_status = VISIABLE | COLLIDABLE;
	Planet1_scale = mat4(1.0f);

	Planet2_ID = 4;
	Planet2_texture = 1;
	Planet2_location = vec3(+100, 10, 0);
	Planet2_transform = glm::mat4(1.0f);
	Planet2_colRadius = 23.0f;
	Planet2_colHandler = 2;
	Planet2_status = VISIABLE | COLLIDABLE;
	Planet2_scale = mat4(1.0f);

	LightSource1_ID = 6;
	LightSource1_texture = 2;
	LightSource1_location = vec3(0.0f, 40.0f, 0.0f);
	LightSource1_transform = glm::mat4(1.0f);
	LightSource1_scale = glm::scale(glm::mat4(), glm::vec3(0.15, 0.15, 0.15));

	LightSource2_ID = 6;
	LightSource2_texture = 2;
	LightSource2_location = vec3(50.0f, 20.0f, 0.0f);
	LightSource2_transform = glm::mat4(1.0f);
	LightSource2_scale = glm::scale(glm::mat4(), glm::vec3(0.15, 0.15, 0.15));

	Rock_ID = 3;
	Rock_texture = 2;
	Rock_colRadius = 2;
	Rock_colHandler = 2;
	Rock_scale = mat4(1.0f);
	for (int i = 0; i < 200; i++) {
		Rock_status[i] = VISIABLE | COLLIDABLE;
		initRock(i, 60.0f, 90.0f, 7.0f, Planet2_location);
	}

	Ring_ID = 5;
	Ring_transform = glm::rotate(mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Ring_colRadius = 2.0f;
	Ring_scale = mat4(1.0f);
	for (int i = 0; i < 4; i++) {
		Ring_texture[i] = 4;
		Ring_colHandler[i] = 3;
		Ring_status[i] = VISIABLE | COLLIDABLE;
		Ring_location[i] = vec3(0, +10, -10 + -30 * i);
	}

}

int check_SC_Planet1_Collision() {
	vec3 distance = SC_location - Planet1_location;
	if (glm::length(distance) < SC_colRadius + Planet1_colRadius) {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, 0.0f, SC_SPEED, 1.0f));
	}
	return 0;
}

int check_SC_Planet2_Collision() {
	vec3 distance = SC_location - Planet2_location;
	if (glm::length(distance) < SC_colRadius + Planet2_colRadius) {
		SC_location =
			vec3(glm::translate(glm::mat4(), vec3(SC_location)) * SC_transform *  glm::vec4(0.0f, 0.0f, SC_SPEED, 1.0f));
	}
	return 0;
}

int check_SC_Rock_Collision(int i) {
	vec3 distance = SC_location - Rock_location[i];
	if (glm::length(distance) < SC_colRadius + Rock_colRadius) {
		Rock_status[i] = 0;
		SC_SPEED *= 0.9;
	}
	return 0;
}

int check_SC_Ring_Collision(int i) {
	vec3 distance = SC_location - Ring_location[i];
	if (glm::length(distance) < SC_colRadius + Ring_colRadius) {
		if (Ring_colHandler[i] == 3) {
			Ring_status[i] = Ring_status[i] | OUTSIDE;
			Ring_texture[i] = 2;
			SC_texture = 2;
			Ring_colHandler[i] = 4;
		}
	}
	else if (Ring_status[i] & OUTSIDE) {
		if (Ring_colHandler[i] == 4) {
			Ring_status[i] = Ring_status[i] ^ OUTSIDE;
			Ring_texture[i] = 4;
			SC_texture = 0;
			Ring_colHandler[i] = 3;
			SC_SPEED += 0.1f;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(900, 900);
	glutCreateWindow("3260 project by Sam and Brian");
	initialiseEntities();
	
	initializedGL();
	glutDisplayFunc(paintGL);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(SpecialKeys);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;

}