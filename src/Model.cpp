#pragma once
// GLM
#include <glm/glm.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Etc
#include <iostream>
#include <vector>
#include <string>
#include <Mesh.cpp>
#include <Shader.cpp>
#include <SquareModelStd.cpp>

class Model
{
public:
	/*  Functions   */
	Model() {
	
		// Initialize voxel data
		square.initSquare();
		voxelData = new bool**[SAMPLE_RATE + 1];
		for (int x = 0; x <= SAMPLE_RATE; x++)
		{
			voxelData[x] = new bool*[SAMPLE_RATE + 1];
			for (int y = 0; y <= SAMPLE_RATE; y++)
			{
				voxelData[x][y] = new bool[SAMPLE_RATE + 1];
				//for (int z = 0; z < SAMPLE_RATE; z++)
				//{
				//}
			}
		}

	}

	Model(char *path)
	{
		loadModel(path);

		// Initialize voxel data
		square.initSquare();
		voxelData = new bool**[SAMPLE_RATE + 1];
		for (int x = 0; x <= SAMPLE_RATE; x++)
		{
			voxelData[x] = new bool*[SAMPLE_RATE + 1];
			for (int y = 0; y <= SAMPLE_RATE; y++)
			{
				voxelData[x][y] = new bool[SAMPLE_RATE + 1];
				//for (int z = 0; z < SAMPLE_RATE; z++)
				//{
				//}
			}
		}

	}

	void Draw()
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw();
	}


	void Draw(Shader shader)
	{
		for (GLint z = 0; z < SAMPLE_RATE; z++)
		{
			for (GLint y = 0; y < SAMPLE_RATE; y++)
			{
				for (GLint x = 0; x < SAMPLE_RATE; x++)
				{
					if (voxelData[x][y][z] == true)
					{
						//std::cout << x + (locX * CHUNK_SIZE) << " " << y << " " << z + (locY * CHUNK_SIZE) << std::endl;
						square.Draw(shader, glm::vec3(x + position.x, y + position.y, z + position.z));
					}
				}
			}
		}

		//square.Draw(shader, glm::vec3(minX, minY, minZ));
		//square.Draw(shader, glm::vec3(maxX, maxY, maxZ));
	}


	void voxelize()
	{

		// For each triangle, fill in the voxel occupied.
		// min -> max : 0 -> 32
		float XindexSpan = abs(minX - maxX) / SAMPLE_RATE;
		float YindexSpan = abs(minY - maxY) / SAMPLE_RATE;
		float ZindexSpan = abs(minZ - maxZ) / SAMPLE_RATE;
		for (Mesh m : meshes) {
			for (Vertex v : m.vertices)
			{
				int x = (v.Position.x + abs(minX))/ XindexSpan;
				int y = (v.Position.y + abs(minY))/ YindexSpan;
				int z = (v.Position.z + abs(minY))/ ZindexSpan;

				voxelData[x][y][z] = true;
			}
		}

	}

	void setPosition(glm::vec3 pos) {
		position.x = pos.x;
		position.y = pos.y;
		position.z = pos.z;
	}

	// Debug functions
	void printVertices() {
		for (int i = 0; i < meshes.size() && i < 10; i++)
			meshes[i].print();
	}
protected:
	/*  Model Data  */
	float maxX = -99999999, maxY = -99999999, maxZ = -99999999;
	float minX = 99999999, minY = 99999999, minZ = 99999999;

	std::vector<Mesh> meshes;
	std::string directory;

	// Voxel Data
	bool*** voxelData;
	static const int SAMPLE_RATE = 32;

	SquareModelStd square;
	glm::vec3 position;

private:
	/*  Functions   */
	void loadModel(std::string path)
	{
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);

		std::cout << path << std::endl;
		std::cout << "Min: " << minX << ", " << minY << ", " << minZ << std::endl; 
		std::cout << "Max: " << maxX << ", " << maxY << ", " << maxZ << std::endl;

	}

	void processNode(aiNode *node, const aiScene *scene)
	{
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		//std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)	// fill vertices
		{
			Vertex vertex;
			glm::vec3 vector;
			// process vertex positions, normals and texture coordinates
			// positions
			if (mesh->HasPositions())
			{
				vector.x = mesh->mVertices[i].x;
				if (vector.x > maxX) maxX = vector.x;
				else if (vector.x < minX) minX = vector.x;

				vector.y = mesh->mVertices[i].y;
				if (vector.y > maxY) maxY = vector.y;
				else if (vector.y < minY) minY = vector.y;

				vector.z = mesh->mVertices[i].z;
				if (vector.z > maxZ) maxZ = vector.z;
				else if (vector.z < minZ) minZ = vector.z;

				vertex.Position = vector;
			}
			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			// Colors
			if (mesh->HasVertexColors(0)) {

			}
			vertices.push_back(vertex);

		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)	// fill indices
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return Mesh(vertices, indices);
	}
	//std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

};