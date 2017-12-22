#pragma once
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.cpp>


class SquareModelStd
{	
	glm::vec3 position;
	glm::vec3 color = glm::vec3(0, 0, .2);

private:

	GLuint SquareVAO, SquareVBO;
	float vertices[216] = {
		// Back face
		0, 0, 0,	0.0f, 0.0f, -1.0f, // Bottom-left
		1,  1, 0,		0.0f, 0.0f, -1.0f, // top-right
		1, 0, 0,		0.0f, 0.0f, -1.0f, // bottom-right         
		1,  1, 0,		0.0f, 0.0f, -1.0f, // top-right
		0, 0, 0,	0.0f, 0.0f, -1.0f, // bottom-left
		0,  1, 0,	0.0f, 0.0f, -1.0f, // top-left
												   // Front face
												   0, 0,  1,	0.0f, 0.0f, 1.0f,  // bottom-left
												   1, 0,  1,		0.0f, 0.0f, 1.0f,  // bottom-right
												   1,  1,  1,		0.0f, 0.0f, 1.0f,  // top-right
												   1,  1,  1,		0.0f, 0.0f, 1.0f,  // top-right
												   0,  1,  1,	0.0f, 0.0f, 1.0f,  // top-left
												   0, 0,  1,	0.0f, 0.0f, 1.0f,  // bottom-left
																						   // Left face
																						   0,  1,  1,	-1.0f, 0.0f, 0.0f, // top-right
																						   0,  1, 0,	-1.0f, 0.0f, 0.0f, // top-left
																						   0, 0, 0,	-1.0f, 0.0f, 0.0f, // bottom-left
																						   0, 0, 0,	-1.0f, 0.0f, 0.0f, // bottom-left
																						   0, 0,  1,	-1.0f, 0.0f, 0.0f, // bottom-right
																						   0,  1,  1,	-1.0f, 0.0f, 0.0f, // top-right
																																   // Right face
																																   1,  1,  1,		1.0f, 0.0f, 0.0f, // top-left
																																   1, 0, 0,		1.0f, 0.0f, 0.0f, // bottom-right
																																   1,  1, 0,		1.0f, 0.0f, 0.0f, // top-right         
																																   1, 0, 0,		1.0f, 0.0f, 0.0f, // bottom-right
																																   1,  1,  1,		1.0f, 0.0f, 0.0f, // top-left
																																   1, 0,  1,		1.0f, 0.0f, 0.0f, // bottom-left     
																																											  // Bottom face
																																											  0, 0, 0,	0.0f, -1.0f, 0.0f, // top-right
																																											  1, 0, 0,		0.0f, -1.0f, 0.0f, // top-left
																																											  1, 0,  1,		0.0f, -1.0f, 0.0f, // bottom-left
																																											  1, 0,  1,		0.0f, -1.0f, 0.0f, // bottom-left
																																											  0, 0,  1,	0.0f, -1.0f, 0.0f, // bottom-right
																																											  0, 0, 0,	0.0f, -1.0f, 0.0f, // top-right
																																																					   // Top face
																																																					   0,  1, 0,	0.0f, 1.0f, 0.0f, // top-left
																																																					   1,  1,  1,		0.0f, 1.0f, 0.0f, // bottom-right
																																																					   1,  1, 0,		0.0f, 1.0f, 0.0f, // top-right     
																																																					   1,  1,  1,		0.0f, 1.0f, 0.0f, // bottom-right
																																																					   0,  1, 0,	0.0f, 1.0f, 0.0f, // top-left
																																																					   0,  1,  1,	0.0f, 1.0f, 0.0f  // bottom-left        
	};


	void Draw() 
	{
		glBindVertexArray(SquareVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
public:

	SquareModelStd(glm::vec3 position) 
	{
		this->position = position;
		//initSquare();
	}

	SquareModelStd() 
	{
		this->position = glm::vec3(0);
		//initSquare();
	}

	void initSquare() 
	{
		
		glGenVertexArrays(1, &SquareVAO);
		glGenBuffers(1, &SquareVBO);

		glBindVertexArray(SquareVAO);

		glBindBuffer(GL_ARRAY_BUFFER, SquareVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//void Draw(Shader shader)
	//{
	//	//std::cout << "hit" << std::endl;
	//	glm::mat4 model;
	//	model = glm::mat4();
	//	model = glm::translate(model, position);

	//	shader.setMat4("model", model);		

	//	Draw();
	//}

	void Draw(Shader shader)
	{
		//std::cout << "hit" << std::endl;
		glm::mat4 model;
		model = glm::mat4();
		model = glm::translate(model, position);

		shader.setVec3("inColor", color);
		shader.setMat4("model", model);

		Draw();
	}

	void Draw(Shader shader, float scale)
	{
		//std::cout << "hit" << std::endl;
		glm::mat4 model;
		model = glm::mat4();
		model = glm::translate(model, position);
		model = glm::scale(model, glm::vec3(scale));

		shader.setVec3("inColor", color);
		shader.setMat4("model", model);

		Draw();
	}
	void Draw(Shader shader, glm::vec3 pos)
	{
		position = pos;
		Draw(shader);
	}
	void Draw(Shader shader, glm::vec3 pos, glm::vec3 clr)
	{
		position = pos;

		glm::vec3 tmpclr = color;

		color = clr;
		Draw(shader);

		color = tmpclr;

	}
	void Draw(Shader shader, glm::vec3 pos, float scale, glm::vec3 clr)
	{
		position = pos;

		glm::vec3 tmpclr = color;

		color = clr;
		Draw(shader, scale);

		color = tmpclr;
	}
	void Draw(Shader shader, glm::vec3 pos, float scale)
	{
		position = pos;
		Draw(shader, scale);
	}
	void Draw(Shader shader, GLfloat x, GLfloat y, GLfloat z)
	{
		position = glm::vec3(x, y, z);
		Draw(shader);
	}
};