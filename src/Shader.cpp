#ifndef SHADER_H
#define SHADER_H

/// We used several preprocessor directives at the top of the header file. Using these little 
/// lines of code informs your compiler to only include and compile this header file if it hasn't
/// been included yet, even if multiple files include the shader header. This prevents linking conflicts.

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>; // Include glew to get all the required OpenGL headers

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/// Shaders
// The fragment shader only requires one output variable and that is a vector of size 4 that defines the 
// final color output that we should calculate ourselves.

/// The vertex shader differs in its input, in that it receives its input straight from the vertex data. 
/// To define how the vertex data is organized we specify the input variables with location metadata so we 
/// can configure the vertex attributes on the CPU. We've seen this in the previous tutorial as layout 
/// (location = 0). The vertex shader thus requires an extra layout specification for its inputs so we can 
/// link it with the vertex data.

/// The other exception is that the fragment shader requires a vec4 color output variable, since the fragment 
/// shaders needs to generate a final output color. If you'd fail to specify an output color in your fragment shader 
/// OpenGL will render your object black (or white).

/// So if we want to send data from one shader to the other we'd have to declare an output in the sending 
/// shader and a similar input in the receiving shader. When the types and the names are equal on both sides 
/// OpenGL will link those variables together and then it is possible to send data between shaders 

/// If you declare a uniform that isn't used anywhere in your GLSL code the compiler will silently remove the 
/// variable from the compiled version which is the cause for several frustrating errors; keep this in mind!


class Shader	/// a shader class that reads shaders from disk, compiles and links them, checks for errors 

{
public:
	// The program ID
	GLuint Program;

	// Constructor reads and builds the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensures ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into GLchar array
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();

		// 2. Compile shaders
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];

		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Check for compilation errors
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Shader Program
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// Use the program
	void Use() { glUseProgram(this->Program); }

	void setVec3(const GLchar* attribute, glm::vec3 v)
	{
		GLint attributeLoc = glGetUniformLocation(Program, attribute);
		glUniform3f(attributeLoc, v.x, v.y, v.z);
	}

	void setVec3(const GLchar* attribute, GLfloat x, GLfloat y, GLfloat z)
	{
		GLint attributeLoc = glGetUniformLocation(Program, attribute);
		glUniform3f(attributeLoc, x, y, z);
	}

	void setFloat(const GLchar* attribute, GLfloat x)
	{
		GLint attributeLoc = glGetUniformLocation(Program, attribute);
		glUniform1f(attributeLoc, x);
	}

	void setMat4(const GLchar* attribute, glm::mat4 x)
	{
		GLint attributeLoc = glGetUniformLocation(Program, attribute);
		glUniformMatrix4fv(attributeLoc, 1, GL_FALSE, glm::value_ptr(x));
	}

};

#endif
