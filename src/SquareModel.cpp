#pragma once
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Mesh.cpp>
#include <Model.cpp>
#include <Shader.cpp>

class SquareModel : public Model
{
	glm::vec3 position;

public:

	SquareModel(glm::vec3 position)
	{
		this->position = position;
		initMesh();
	}
	SquareModel() {}

	void Draw(Shader shader)
	{
		//std::cout << "hit" << std::endl;
		glm::mat4 model;
		model = glm::mat4();
		model = glm::translate(model, position);

		shader.setMat4("model", model);

		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw();
	}
	void Draw(Shader shader, glm::vec3 pos)
	{
		//std::cout << "hit" << std::endl;
		glm::mat4 model;
		model = glm::mat4();
		model = glm::translate(model, pos);

		shader.setMat4("model", model);

		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw();
	}
	void Draw(Shader shader, GLfloat x, GLfloat y, GLfloat z)
	{
		//std::cout << "hit" << std::endl;
		glm::mat4 model;
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(x,y,z));

		shader.setMat4("model", model);

		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw();
	}

	void initMesh() 
	{
		GLfloat vertices[] = {
			// position		    // normals
			// front
			-0.5, -0.5, 0.5, 0.0f, 0.0f, 1.0f,
			0.5, -0.5, 0.5, 0.0f, 0.0f, 1.0f,
			0.5, 0.5, 0.5, 0.0f, 0.0f, 1.0f,
			-0.5, 0.5, 0.5, 0.0f, 0.0f, 1.0f,
			// back
			-0.5, -0.5, -0.5, 0.0f, 0.0f, -1.0f,
			0.5, -0.5, -0.5, 0.0f, 0.0f, -1.0f,
			0.5, 0.5, -0.5, 0.0f, 0.0f, -1.0f,
			-0.5, 0.5, -0.5, 0.0f, 0.0f, -1.0f
		};

		unsigned int indices[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// top
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// bottom
			4, 0, 3,
			3, 7, 4,
			// left
			4, 5, 1,
			1, 0, 4,
			// right
			3, 2, 6,
			6, 7, 3,
		};

		Mesh mesh = Mesh();
		// fill vertices
		for (int i = 0; i < 8; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			// Position
			vector.x = vertices[i * 6];
			vector.y = vertices[i * 6 + 1];
			vector.z = vertices[i * 6 + 2];
			vertex.Position = vector;
			// Normal
			vector.x = vertices[i * 6 + 3];
			vector.y = vertices[i * 6 + 4];
			vector.z = vertices[i * 6 + 5];
			vertex.Normal = vector;

			mesh.vertices.push_back(vertex);
		}
		// set indices
		mesh.indices = std::vector<unsigned int>(std::begin(indices), std::end(indices));
		mesh.setupMesh();
		// Create model with mesh
		meshes.push_back(mesh);
	}
};


