//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1Β της OpenGL
//
//ΑΜ:4662                        Όνομα:Γεώργιος Δεληγιώργης 
//ΑΜ:4819                         Όνομα:Ιάσων Τσάτσης 

//*********************************
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "stb_image.h"

#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}


//*******************************************************************************
// Η παρακάτω συνάρτηση είναι από http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// H συνάρτηση loadOBJ φορτώνει ένα αντικείμενο από το obj αρχείο του και φορτώνει και normals kai uv συντεταγμένες
// Την χρησιμοποιείτε όπως το παράδειγμα που έχω στην main
// Very, VERY simple OBJ loader.
// 

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
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
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
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
			// Probably a comment, eat up the rest of the line
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
	fclose(file);
	return true;
}
//************************************
// Η LoadShaders είναι black box για σας
//************************************
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
	
///****************************************************************
//  Εδω θα υλοποιήσετε την συνάρτηση της κάμερας
//****************************************************************
void camera_function(GLFWwindow* window,glm::vec3& rotation,glm::vec3& scale)
{
	//rotation upwards on x 
	if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS){
    		 rotation.x += 1.0f; 
	}
	//rotation downwards on x
	if (glfwGetKey(window, GLFW_KEY_X ) == GLFW_PRESS){
    		 rotation.x -= 1.0f;
	}
	//rotation clockwise on z
	if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS){
   		 rotation.z += 1.0f;
	}
	//rotation left turn on z
	if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS){
    		 rotation.z -= 1.0f;
	}
	//scale up
	if (glfwGetKey(window, GLFW_KEY_KP_ADD ) == GLFW_PRESS){
   		 scale += 0.05f;
	}
	//scale down
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT ) == GLFW_PRESS){
    		 scale -= 0.05f;
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1000, 1000, "Εργασία 1Γ-Καταστροφή", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("ProjCVertexShader.vertexshader", "ProjCFragmentShader.fragmentshader");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	//////////////////////////////////////////////////////////////////////////////////////////// LOAD GRID+TEXTURE //////////////////////////////////////////////////////////////////////////////////////////
	//load grid.obj file 
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
    bool res = loadOBJ("grid.obj", vertices, uvs, normals);
	//load ground2.jpg file 
	int width, height, channel;
    unsigned char* data = stbi_load("ground2.jpg", &width, &height, &channel, 0);
    if (data)
    {

    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
	//create texture with ground2.jpg
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);     
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//create a vertex for the grid 
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	//rotation,scale
	glm::vec3 rotation(0.0f);
	glm::vec3 scale(1.0f);
	//************************************************
	// **Προσθέστε κώδικα για την κάμερα
    //*************************************************
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 10000.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(-30.0f, -60.0f, 40.0f), 
		glm::vec3(50.0f,50.0f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 1.0f)    
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
	///////////////////////////////////////////////////////////////////////// LOAD BALL + TEXTURE //////////////////////////////////////////////////////////////////////////
	// load ball.obj file
    std::vector<glm::vec3> vertices1;
    std::vector<glm::vec3> normals1;
    std::vector<glm::vec2> uvs1;
    bool Sres = loadOBJ("ball.obj", vertices1, uvs1, normals1);
	//load fire.jpg file 
	int Swidth, Sheight, Schannel;
    unsigned char* Sdata = stbi_load("fire.jpg", &Swidth, &Sheight, &Schannel, 0);
    if (Sdata)
    {

    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
	//create texture with fire.jpg
    GLuint StextureID;
    glGenTextures(1, &StextureID);
    glBindTexture(GL_TEXTURE_2D, StextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Sdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint STextureID  = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);     
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //create a vertex for the grid 
	GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, vertices1.size() * sizeof(glm::vec3), &vertices1[0], GL_STATIC_DRAW);
	GLuint uvbuffer2;
	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, uvs1.size() * sizeof(glm::vec2), &uvs1[0], GL_STATIC_DRAW);
	///////////////////////////////////////////////////////////// LOAD CRATER +TEXTURE ///////////////////////////////////////////////////////
	//crater
	// load crater.obj file
    std::vector<glm::vec3> vertices2;
    std::vector<glm::vec3> normals2;
    std::vector<glm::vec2> uvs2;
    bool Cres = loadOBJ("crater.obj", vertices2, uvs2, normals2);
	//load crater2.jpg file
	int width2, height2, channel2;
    unsigned char* Cdata = stbi_load("crater2.jpg", &width2, &height2, &channel2, 0);
    if (data)
    {

    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
	//create texture with crater2.jpg
    GLuint CtextureID;
    glGenTextures(1, &CtextureID);
    glBindTexture(GL_TEXTURE_2D, CtextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, Cdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint CTextureID  = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);     
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//create a vertex for the grid 
    GLuint Cvertexbuffer;
    glGenBuffers(1, &Cvertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Cvertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);
    GLuint Cuvbuffer;
    glGenBuffers(1, &Cuvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Cuvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2), &uvs2[0], GL_STATIC_DRAW);
	do {
		//generate random position 
		srand (static_cast <unsigned> (time(0)));
		float x = 0.0 + static_cast <float> (rand()) /(static_cast <float> (RAND_MAX/(100.0 )));
		float y = 0.0 + static_cast <float> (rand()) /(static_cast <float> (RAND_MAX/(100.0 )));
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 10000.0f);
		// Camera matrix
		glm::mat4 View = glm::lookAt(
			glm::vec3(-30.0f, -60.0f, 40.0f), 
			glm::vec3(50.0f,50.0f, 0.0f), 
			glm::vec3(0.0f, 0.0f, 1.0f)    
		);
		View = glm::rotate(View,glm::radians(rotation.x),glm::vec3(1.0f,0.0f,0.0f));
		View = glm::rotate(View,glm::radians(rotation.z),glm::vec3(0.0f,0.0f,1.0f));
		View = glm::scale(View,scale);
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);
		//camera with keyboard
		camera_function(window,rotation,scale);
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
		//*************************************************
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		///////////draw grid ////////////////////
		glEnableVertexAttribArray(0);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		glDrawArrays(GL_TRIANGLES, 0 ,vertices.size()); // 3 indices starting at 0 -> 1 triangle
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//Ball
			float z = 20.0;
			int counter = 0 ;
			if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS ){
				while(z >= 0.5){
					
					glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(x,y,z)) * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
					MVP = Projection * View * ModelMatrix;
					glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
					
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, StextureID);
					glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(
						0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
						3,                  // size
						GL_FLOAT,           // type
						GL_FALSE,           // normalized?
						0,                  // stride
						(void*)0            // array buffer offset
					);

					// 2nd attribute buffer : colors
					glEnableVertexAttribArray(1);
					glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
					glVertexAttribPointer(
						1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
						2,                                // size
						GL_FLOAT,                         // type
						GL_FALSE,                         // normalized?
						0,                                // stride
						(void*)0                          // array buffer offset
					);
					glDrawArrays(GL_TRIANGLES, 0 , vertices1.size()); // 3 indices starting at 0 -> 1 triangle
					
					glDisableVertexAttribArray(0);
					glDisableVertexAttribArray(1);	
					
					z = z - 3.9;
					counter = counter + 1;

					// Swap buffers
					glfwSwapBuffers(window);
					glfwPollEvents();
					usleep(300000);

				
					if(counter == 5){
						//Crater
						glm::mat4 CraterMatrix = glm::translate(glm::mat4(1.0), glm::vec3(x, y, 0.5f)) *glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 8.0f, 8.0f));
						CraterMatrix = glm::rotate(CraterMatrix,90.0f,glm::vec3(1.0f,1.0f,1.0f));
						MVP = Projection * View * CraterMatrix;
						glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
						
						glEnableVertexAttribArray(0);
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, CtextureID);
						glBindBuffer(GL_ARRAY_BUFFER, Cvertexbuffer);
						glVertexAttribPointer(
							0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
							3,                  // size
							GL_FLOAT,           // type
							GL_FALSE,           // normalized?
							0,                  // stride
							(void*)0            // array buffer offset
						);
			
						// 2nd attribute buffer : colors
						glEnableVertexAttribArray(1);
						glBindBuffer(GL_ARRAY_BUFFER, Cuvbuffer);
						glVertexAttribPointer(
							1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
							2,                                // size
							GL_FLOAT,                         // type
							GL_FALSE,                         // normalized?
							0,                                // stride
							(void*)0                          // array buffer offset
						);
						glDrawArrays(GL_TRIANGLES, 0 ,vertices2.size()); // 3 indices starting at 0 -> 1 triangle
						
						glDisableVertexAttribArray(0);
						glDisableVertexAttribArray(1);
						}
				}
			
			}
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the Space key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}

