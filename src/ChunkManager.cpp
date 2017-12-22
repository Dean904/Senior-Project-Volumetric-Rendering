#pragma once
#include <vector>
#include <Chunk.cpp>
#include <iostream>
#include <Shader.cpp>
//#include <SparseVoxelOctree.h>
#include <OctreeVoxel.h>
#include <tuple>
#include <unordered_set>

#include <iomanip>		// setprecision
#include <sstream>      // std::stringstream


class ChunkManager 
{

private:

	Chunk* ** chunkList;
	Voxel octreeRoot;

public:

	static const int CHUNK_NUM = 8;
	#define octreeScale 8


	ChunkManager() {}

	void initialize() 
	{
		double startTime = glfwGetTime();


		Chunk::square.initSquare();

		noise::module::Perlin myModule;
		noise::utils::NoiseMap heightMap;
		noise::utils::NoiseMapBuilderPlane heightMapBuilder;

		//myModule.SetOctaveCount(6); // [0-6]  default 6 - effects how 'busy' map is
		myModule.SetFrequency(0.2); // [1-16] default 1 - determines how many changes occur along a unit length
		//myModule.SetPersistence(1.0); // [0-1] default .5 - effects smoothness/roughness


		heightMapBuilder.SetSourceModule(myModule);
		heightMapBuilder.SetDestNoiseMap(heightMap);
		//heightMapBuilder.set
		heightMapBuilder.SetDestSize(Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE);

		chunkList = new Chunk**[CHUNK_NUM];
		for (int i = 0; i < CHUNK_NUM; i++)
		{
			chunkList[i] = new Chunk*[CHUNK_NUM];

			for (int j = 0; j < CHUNK_NUM; j++)
			{
				heightMapBuilder.SetBounds(2.0 + (4 * i), 6.0 + (4 * i), 2.0 + (4 * j), 6.0 + (4 * j));
				heightMapBuilder.Build();

				Chunk *tempChunk = new Chunk(i, j, heightMap);
				chunkList[i][j] = tempChunk;
				
			}
		}

		// Set adjacent chunk references for marching cube seams
		for (int i = 0; i < CHUNK_NUM; i++) {
			for (int j = 0; j < CHUNK_NUM; j++) 
			{
				if (i < CHUNK_NUM - 1) chunkList[i][j]->setRightAdjacentChunks(chunkList[i + 1][j]);
				if (j < CHUNK_NUM - 1) chunkList[i][j]->setTopAdjacentChunks(chunkList[i][j + 1]);
			}
		}


		std::cout << "Chunks generated in " << glfwGetTime() - startTime << " seconds." << std::endl;
		startTime = glfwGetTime();


		// Build Octree
		heightMapBuilder.SetBounds(2.0, 6.0 + (4 * (CHUNK_NUM - 1)), 2.0, 6.0 + (4 * (CHUNK_NUM - 1)));
		heightMapBuilder.SetDestSize(Chunk::CHUNK_SIZE * CHUNK_NUM, Chunk::CHUNK_SIZE * CHUNK_NUM);
		heightMapBuilder.Build();

		//octreeRoot = constructTestOctree();
		std::cout << "Octree generated in... ";
		octreeRoot = constructOctree(heightMap);

		std::cout << glfwGetTime() - startTime << " seconds." << std::endl;


	}

	void manipulateVoxelWithRay(glm::vec3 rayOrigin, glm::vec3 rayDirecion, bool BuildOrBreak)
	{
		glm::ivec3 lastPos;
		bool lastPosInitialized = false;
		for (int i = 0; i < 100; i++)
		{
			rayOrigin += rayDirecion * 0.2f;
			Chunk* chunk = getChunkForLocation(rayOrigin);
			if (chunk != NULL)
			{
				glm::ivec3 localPos;
				localPos.x = rayOrigin.x - (chunk->locX * chunk->CHUNK_SIZE);
				localPos.y = rayOrigin.y;
				localPos.z = rayOrigin.z - (chunk->locY * chunk->CHUNK_SIZE);

				if (localPos == lastPos) continue;

				if (localPos.x > -1 && localPos.y > -1 && localPos.z > -1)
				{
					if (localPos.x < chunk->CHUNK_SIZE && localPos.y < chunk->CHUNK_SIZE && localPos.z < chunk->CHUNK_SIZE)
					{
						bool*** voxelData = chunk->getChunkData();
						if (voxelData[localPos.x][localPos.y][localPos.z])
						{
							if (!BuildOrBreak) {	// destroy
								std::cout << "Destroying @ " << localPos.x << " " << localPos.y << " " << localPos.z << std::endl;
								chunk->destroyVoxel(localPos.x, localPos.y, localPos.z);
							}
							else {	// create
								std::cout << "Creating @ " << lastPos.x << " " << lastPos.y << " " << lastPos.z << std::endl;
								chunk->createVoxel(lastPos.x, lastPos.y, lastPos.z);
								//voxelData[lastPos.x][lastPos.y][lastPos.z] = true;
							}
							break;
						}
						else {
							lastPos = glm::ivec3(localPos.x, localPos.y, localPos.z);
							lastPosInitialized = true;
							//std::cout << "LastPos: " << lastPos.x << " " << lastPos.y << " " << lastPos.z << std::endl;
						}
					}
				}
			}
		}
	}

	Chunk* getChunkForLocation(glm::vec3 pos)
	{
		for (int i = 0; i < CHUNK_NUM; i++) {
			for (int j = 0; j < CHUNK_NUM; j++) 
			{
				Chunk* chunk = chunkList[i][j];
				if ((chunk->locX * chunk->CHUNK_SIZE <= pos.x) && ((chunk->locX * chunk->CHUNK_SIZE) + chunk->CHUNK_SIZE >= pos.x)) {
					if ((chunk->locY * chunk->CHUNK_SIZE <= pos.z) && ((chunk->locY * chunk->CHUNK_SIZE) + chunk->CHUNK_SIZE >= pos.z)) {
						return chunk;
					}
				}
			}
		}
		return NULL;
	}


	void octree_raycast(glm::vec3 origin, glm::vec3 dir, bool verbose)
	{
		cast_ray(&octreeRoot, origin, dir, verbose);
	}

	//void Draw(Shader shader, int renderingMode, glm::vec3 pos)
	//{
	//	std::cout << "ooooooooooooooh($#&*y$@&*#t%@#$)*&#! $(*^!#@$! \n\n";
	//	Chunk* centerChunk = getChunkForLocation(pos);
	//	if (centerChunk != NULL) 
	//	{
	//		int x = centerChunk->locX;
	//		int y = centerChunk->locY;

	//	}
	//}

	/////////////////////////////////////////
	//		Render Calls				   //
	/////////////////////////////////////////

	void Draw(Shader shader) {
		//	std::cout << chunkList.size() << " ";
		//renderer.drawCube(glm::vec3(0, -10, 0));
		for (int i = 0; i < CHUNK_NUM; i++) {
			for (int j = 0; j < CHUNK_NUM; j++)
			{
				chunkList[i][j]->Draw(shader);
			}
		}
	}

	void MarchingCubesDraw(Shader shader) {
		for (int i = 0; i < CHUNK_NUM; i++) {
			for (int j = 0; j < CHUNK_NUM; j++)
			{
				chunkList[i][j]->MarchingCubesDraw(shader);
			}
		}
	}


	// SVO Rendering
	void drawOctree(Shader shader) // 
	{
		renderOctree(shader);
	}

	void computerFullRaycast(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight) 
	{
		GLfloat ltime = glfwGetTime();
		raycastingLeafs.clear();

		std::cout << "Computing ray results for each pixel. Time elapsed... ";

		for (int y = 0; y < screenHeight; y++) 
		{
			for (int x = 0; x < screenWidth; x++)
			{
				glm::vec3 origin = glm::unProject(glm::vec3(x, y, 0.0f), view, projection, glm::vec4(0, 0, screenWidth, screenHeight));
				glm::vec3 end = glm::unProject(glm::vec3(x, y, 1.0f), view, projection, glm::vec4(0, 0, screenWidth, screenHeight));

				octree_raycast(origin, glm::normalize(end - origin), false);
			}
		}
		std::cout << glfwGetTime() - ltime << "s \n";
	}

	void drawFullRaycast(Shader shader) 
	{
		for (glm::vec4 v : raycastingLeafs)
		{
			glm::vec3 grey = glm::vec3(.21, .21, .21);
			glm::vec3 green = glm::vec3(.17, .3, .17);
			glm::vec3 white = glm::vec3(.9, .9, .9);

			glm::vec3 color;
			if (v.y < 10) color = green;
			else if (v.y < 27) color = grey;
			else color = white;

			Chunk::square.Draw(shader, glm::vec3(v), v.w, color);
		}
	}

	void renderRaycast(Shader shader)
	{
		for(glm::vec4 v : octreeBoxes) 
		{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				Chunk::square.Draw(shader, glm::vec3(v), v.w, glm::vec3((exp2(octreeScale) - v.w) / exp2(octreeScale), 0, 0));
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			
		}
		if(raycastCube.x != -1)
			Chunk::square.Draw(shader, raycastCube, 1.1, glm::vec3(0, 1, 0));
	}

private:

	std::vector<glm::vec4> octreeBoxes;
	glm::vec3 raycastCube;



	//std::vector<glm::vec4> raycastingLeafs;
	struct hash_vec4
	{
		size_t operator()(const glm::vec4& v) const {
			return v.x + v.y * 10 + v.z * 100 + v.w * 1000;
		}
	};

	std::unordered_set<glm::vec4, hash_vec4> raycastingLeafs;


	Voxel constructOctree(noise::utils::NoiseMap heightMap)
	{
		Voxel root;

		// Bounding Box
		glm::vec3 minBB;
		glm::vec3 maxBB = glm::vec3(heightMap.GetWidth());

		std::cout << "Construction SVO - Scale: " << octreeScale << " @ [" << minBB.x << "," << minBB.y << "," << minBB.z << "] -> [" << maxBB.x << "," << maxBB.y << "," << maxBB.z << "] \n";

		constructChildren(&root, heightMap, 1, octreeScale, minBB, maxBB);


		return root; 

	}

	Voxel constructTestOctree() 
	{
		Voxel root;
		Voxel* currentNode = &root;

		for (int idx = 0; idx < 8; idx++) 
		{
			int max = 3;
			currentNode = &root;
			for (int i = 0; i <= max; i++)
			{
				currentNode->childPointers.push_back(new Voxel);
				currentNode->validMask += exp2(idx);
				if (i == max) currentNode->leafMask = exp2(idx);
				else if(i == 0) currentNode = currentNode->childPointers[idx];
				else currentNode = currentNode->childPointers[0];
			}
		}
		return root;
	}

	void constructChildren(Voxel* node, noise::utils::NoiseMap heightmap, int currDepth, int maxDepth, glm::vec3 min, glm::vec3 max)
	{
		// for each voxel check if it contains topology, mark true if it does
		// To determine if each voxel contains voxels (for a heightmap) check the bottom 4 corners
		// build children until depth > 4

		// Child Voxel Bounding Boxes
			// 0 = min (beginning)
			// 1 = max / 2 (middle)
			// 2 = max (end)

		glm::vec3 mid = glm::vec3((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2);

		if (currDepth <= maxDepth)
		{
			float offset = 0.001f;
			constructNode(node, 0, heightmap, glm::vec3(min.x + offset, min.y, min.z + offset), glm::vec3(mid.x - offset, min.y, min.z + offset), glm::vec3(mid.x - offset, min.y, mid.z - offset), glm::vec3(min.x + offset, min.y, mid.z - offset));
			constructNode(node, 1, heightmap, glm::vec3(mid.x + offset, min.y, min.z + offset), glm::vec3(max.x - offset, min.y, min.z + offset), glm::vec3(max.x - offset, min.y, mid.z - offset), glm::vec3(mid.x + offset, min.y, mid.z - offset));
			constructNode(node, 2, heightmap, glm::vec3(mid.x + offset, min.y, mid.z + offset), glm::vec3(max.x - offset, min.y, mid.z + offset), glm::vec3(max.x - offset, min.y, max.z - offset), glm::vec3(mid.x + offset, min.y, max.z - offset));
			constructNode(node, 3, heightmap, glm::vec3(min.x + offset, min.y, mid.z + offset), glm::vec3(mid.x - offset, min.y, mid.z + offset), glm::vec3(mid.x - offset, min.y, max.z - offset), glm::vec3(min.x + offset, min.y, max.z - offset));
			constructNode(node, 4, heightmap, glm::vec3(min.x + offset, mid.y, min.z + offset), glm::vec3(mid.x - offset, mid.y, min.z + offset), glm::vec3(mid.x - offset, mid.y, mid.z - offset), glm::vec3(min.x + offset, mid.y, mid.z - offset));
			constructNode(node, 5, heightmap, glm::vec3(mid.x + offset, mid.y, min.z + offset), glm::vec3(max.x - offset, mid.y, min.z + offset), glm::vec3(max.x - offset, mid.y, mid.z - offset), glm::vec3(mid.x + offset, mid.y, mid.z - offset));
			constructNode(node, 6, heightmap, glm::vec3(mid.x + offset, mid.y, mid.z + offset), glm::vec3(max.x - offset, mid.y, mid.z + offset), glm::vec3(max.x - offset, mid.y, max.z - offset), glm::vec3(mid.x + offset, mid.y, max.z - offset));
			constructNode(node, 7, heightmap, glm::vec3(min.x + offset, mid.y, mid.z + offset), glm::vec3(mid.x - offset, mid.y, mid.z + offset), glm::vec3(mid.x - offset, mid.y, max.z - offset), glm::vec3(min.x + offset, mid.y, max.z - offset));
		}


		if (currDepth == maxDepth)
		{ // set leaf mask 
		  // for each node, if the valid mask is set, set the leaf mask
			for (int i = 0; i < 8; i++)
			{
				if (node->validMask & (int)exp2(i))
				{
					node->leafMask += exp2(i);
					//if (i == 0) std::cout << currDepth << " : " << i << " [" << min.x << ", " << min.y << ", " << min.z << "] -> [" << mid.x << ", " << mid.y << ", " << mid.z << "]" << std::endl;
					//if (i == 1) std::cout << currDepth << " : " << i << " [" << mid.x << ", " << min.y << ", " << min.z << "] -> [" << max.x << ", " << mid.y << ", " << mid.z << "]" << std::endl;
					//if (i == 2) std::cout << currDepth << " : " << i << " [" << mid.x << ", " << min.y << ", " << mid.z << "] -> [" << max.x << ", " << mid.y << ", " << max.z << "]" << std::endl;
					//if (i == 3) std::cout << currDepth << " : " << i << " [" << min.x << ", " << min.y << ", " << mid.z << "] -> [" << mid.x << ", " << mid.y << ", " << max.z << "]" << std::endl;
					//if (i == 4) std::cout << currDepth << " : " << i << " [" << min.x << ", " << mid.y << ", " << min.z << "] -> [" << mid.x << ", " << max.y << ", " << mid.z << "]" << std::endl;
					//if (i == 5) std::cout << currDepth << " : " << i << " [" << mid.x << ", " << mid.y << ", " << min.z << "] -> [" << max.x << ", " << max.y << ", " << mid.z << "]" << std::endl;
					//if (i == 6) std::cout << currDepth << " : " << i << " [" << mid.x << ", " << mid.y << ", " << mid.z << "] -> [" << max.x << ", " << max.y << ", " << max.z << "]" << std::endl;
					//if (i == 7) std::cout << currDepth << " : " << i << " [" << min.x << ", " << mid.y << ", " << mid.z << "] -> [" << mid.x << ", " << max.y << ", " << max.z << "]" << std::endl;
				}
			}

		}
		else {
			// Construct the children of every node		
			int childCounter = 0;	// used to index node->childPointers

			if (node->validMask & 1)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, min, mid), childCounter++;
			if (node->validMask & 2)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(mid.x, min.y, min.z), glm::vec3(max.x, mid.y, mid.z)), childCounter++;
			if (node->validMask & 4)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(mid.x, min.y, mid.z), glm::vec3(max.x, mid.y, max.z)), childCounter++;
			if (node->validMask & 8)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(min.x, min.y, mid.z), glm::vec3(mid.x, mid.y, max.z)), childCounter++;
			if (node->validMask & 16)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(min.x, mid.y, min.z), glm::vec3(mid.x, max.y, mid.z)), childCounter++;
			if (node->validMask & 32)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(mid.x, mid.y, min.z), glm::vec3(max.x, max.y, mid.z)), childCounter++;
			if (node->validMask & 64)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, mid, max), childCounter++;
			if (node->validMask & 128)	 constructChildren(node->childPointers[childCounter], heightmap, currDepth + 1, maxDepth, glm::vec3(min.x, mid.y, mid.z), glm::vec3(mid.x, max.y, max.z)), childCounter++;
		}

	}



	void constructNode(Voxel* node, int idx, noise::utils::NoiseMap heightmap, glm::vec3 c0, glm::vec3 c1, glm::vec3 c2, glm::vec3 c3)
	{
		float heightVal0 = ((heightmap.GetValue(c0.x, c0.z) + 1) * Chunk::CHUNK_SIZE / 2);
		float heightVal1 = ((heightmap.GetValue(c1.x, c1.z) + 1) * Chunk::CHUNK_SIZE / 2);
		float heightVal2 = ((heightmap.GetValue(c2.x, c2.z) + 1) * Chunk::CHUNK_SIZE / 2);
		float heightVal3 = ((heightmap.GetValue(c3.x, c3.z) + 1) * Chunk::CHUNK_SIZE / 2);

		//int tempMask = node->validMask;
		//std::cout << "t: " << tempMask << " += " << exp2(idx) << " = " << (tempMask += exp2(idx)) << std::endl;
		//tempMask = node->validMask;
		//std::cout << "t: " << tempMask << " |= " << exp2(idx) << " = " << (tempMask |= (int) exp2(idx)) << std::endl;

		float pointSpan = c1.x - c0.x;		// + offset * 2
		if (heightVal0 > c0.y && heightVal0 < (c0.y + pointSpan))  {
			node->childPointers.push_back(new Voxel);
			node->validMask += exp2(idx);
		}
		else if (heightVal1 > c1.y && heightVal1 < (c1.y + pointSpan)) {
			node->childPointers.push_back(new Voxel);
			node->validMask += exp2(idx);
		}
		else if (heightVal2 > c2.y && heightVal2 < (c2.y + pointSpan)) {
			node->childPointers.push_back(new Voxel);
			node->validMask += exp2(idx);
		}
		else if (heightVal3 > c3.y && heightVal3 < (c3.y + pointSpan)) {
			node->childPointers.push_back(new Voxel);
			node->validMask += exp2(idx);
		}
	}

	void renderOctree(Shader shader) 
	{
		renderNode(&octreeRoot, glm::vec3(0), glm::vec3(Chunk::CHUNK_SIZE * CHUNK_NUM), shader);
	}


	void renderNode(Voxel* node, glm::vec3 min, glm::vec3 max, Shader shader) 
	{
		glm::vec3 mid = glm::vec3((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2);
		int childCount = 0;
		for (int i = 0; i < 8; i++) 
		{
			if (node->validMask & (int) exp2(i))
			{
				if (node->leafMask & (int)exp2(i))
				{
					if (i == 0) Chunk::square.Draw(shader, min);
					if (i == 1) Chunk::square.Draw(shader, glm::vec3(mid.x, min.y, min.z));
					if (i == 2) Chunk::square.Draw(shader, glm::vec3(mid.x, min.y, mid.z));
					if (i == 3) Chunk::square.Draw(shader, glm::vec3(min.x, min.y, mid.z));
					if (i == 4) Chunk::square.Draw(shader, glm::vec3(min.x, mid.y, min.z));
					if (i == 5) Chunk::square.Draw(shader, glm::vec3(mid.x, mid.y, min.z));
					if (i == 6) Chunk::square.Draw(shader, mid);
					if (i == 7) Chunk::square.Draw(shader, glm::vec3(min.x, mid.y, mid.z));
				}
				else //if (currDepth <= maxDepth)
				{
					if (i == 0) renderNode(node->childPointers[childCount], min, mid, shader), childCount++;
					if (i == 1) renderNode(node->childPointers[childCount], glm::vec3(mid.x, min.y, min.z), glm::vec3(max.x, mid.y, mid.z), shader), childCount++;
					if (i == 2) renderNode(node->childPointers[childCount], glm::vec3(mid.x, min.y, mid.z), glm::vec3(max.x, mid.y, max.z), shader), childCount++;
					if (i == 3) renderNode(node->childPointers[childCount], glm::vec3(min.x, min.y, mid.z), glm::vec3(mid.x, mid.y, max.z), shader), childCount++;
					if (i == 4) renderNode(node->childPointers[childCount], glm::vec3(min.x, mid.y, min.z), glm::vec3(mid.x, max.y, mid.z), shader), childCount++;
					if (i == 5) renderNode(node->childPointers[childCount], glm::vec3(mid.x, mid.y, min.z), glm::vec3(max.x, max.y, mid.z), shader), childCount++;
					if (i == 6) renderNode(node->childPointers[childCount], mid, max, shader), childCount++;
					if (i == 7) renderNode(node->childPointers[childCount], glm::vec3(min.x, mid.y, mid.z), glm::vec3(mid.x, max.y, max.z), shader), childCount++;
				}
			}
		}
	}



	//// ============================================================================================================

	bool pointIsInVoxel(glm::dvec3 point, glm::vec3 pos, float max)
	{
		if (point.x >= pos.x && point.x <= pos.x + max && point.y >= pos.y &&
			point.y <= pos.y + max && point.z >= pos.z && point.z <= pos.z + max)
		{
			return true;
		}
		return false;
	}

	void findMinMax(glm::dvec3 origin, glm::dvec3 dir, glm::vec3 pos, float size, double* t_min, double* t_max)
	{
		// Calculate t_min and t_max

		double tx_0 = (pos.x - origin.x) / dir.x;
		double ty_0 = (pos.y - origin.y) / dir.y;
		double tz_0 = (pos.z - origin.z) / dir.z;

		double tx_mid = (pos.x + (size / 2) - origin.x) / dir.x;
		double ty_mid = (pos.y + (size / 2) - origin.y) / dir.y;
		double tz_mid = (pos.z + (size / 2) - origin.z) / dir.z;

		double tx_max = (pos.x + size - origin.x) / dir.x;
		double ty_max = (pos.y + size - origin.y) / dir.y;
		double tz_max = (pos.z + size - origin.z) / dir.z;

		*t_min = 9999;
		*t_max = -1;

		if (pointIsInVoxel(glm::vec3(origin + (tx_0 * dir)), pos, size) && tx_0 >= 0) {
			if (tx_0 < *t_min && tx_0 < tx_mid) *t_min = tx_0;
			if (tx_0 > *t_max && tx_0 > tx_mid) *t_max = tx_0;
		}
		if (pointIsInVoxel(glm::vec3(origin + (ty_0 * dir)), pos, size) && ty_0 >= 0) {
			if (ty_0 < *t_min && ty_0 < ty_mid) *t_min = ty_0;
			if (ty_0 > *t_max && ty_0 > ty_mid) *t_max = ty_0;
		}
		if (pointIsInVoxel(glm::vec3(origin + (tz_0 * dir)), pos, size) && tz_0 >= 0) {
			if (tz_0 < *t_min && tz_0 < tz_mid) *t_min = tz_0;
			if (tz_0 > *t_max && tz_0 > tz_mid) *t_max = tz_0;
		}

		if (pointIsInVoxel(glm::vec3(origin + (tx_max * dir)), pos, size) && tx_max >= 0) {
			if (tx_max < *t_min && tx_max < tx_mid) *t_min = tx_max;
			if (tx_max > *t_max && tx_max > tx_mid) *t_max = tx_max;
		}
		if (pointIsInVoxel(glm::vec3(origin + (ty_max * dir)), pos, size) && ty_max >= 0) {
			if (ty_max < *t_min && ty_max < ty_mid) *t_min = ty_max;
			if (ty_max > *t_max && ty_max > ty_mid) *t_max = ty_max;
		}
		if (pointIsInVoxel(glm::vec3(origin + (tz_max * dir)), pos, size) && tz_max >= 0) {
			if (tz_max < *t_min && tz_max < tz_mid) *t_min = tz_max;
			if (tz_max > *t_max && tz_max > tz_mid) *t_max = tz_max;
		}
	}

	int getIdx(glm::dvec3 point, glm::vec3* pos, int size)
	{
		int index = 0;
		if (pointIsInVoxel(point, *pos, size * 2))
		{
			// Mismatch on sign between tx_mid and dir.x indicates past mid line
			if ((pos->x + size - point.x) < 0.0f) index += 2, pos->x += size;		//	BTM:  2 | 3		TOP:  6 | 7
			if ((pos->y + size - point.y) < 0.0f) index += 4, pos->y += size;		//		 -------	     -------
			if ((pos->z + size - point.z) < 0.0f) index += 1, pos->z += size;		//		  0 | 1			  4 | 5
		}
		else
		{
			return -1;
		}

		// index -> idx;
		int getIdx[8] = { 0, 1, 3, 2, 4, 5, 7, 6 }; // Refactor voxel construction to create uniform indexing
		return getIdx[index];
	}


	// return vec4 (vec3 pos & scale) ?
	void cast_ray(Voxel* root, glm::dvec3 origin, glm::dvec3 dir, bool verbose)
	{
		Voxel* parent = root;				// parent & idx = current voxel
		int idx = 0;						// Initialized to a child of root by comparing t_min against tx, 
											// ty, and tz at the center of the octree
		glm::vec3 pos(0);					// cube corresponding to current voxel

		int size = CHUNK_NUM * Chunk::CHUNK_SIZE;	// 2^s_max
		const int s_max = octreeScale;				// max scale
		int scale = s_max - 1;						// Initialized to represent initial voxel
		const float epsilon = exp2f(-s_max);		// 2^(-6) = .015625

		float ray_size_coef;				// In: LOD at ray origin (shared mem).
		float ray_size_bias;				// In: LOD increase along ray (register).

		std::tuple<Voxel*, float, glm::ivec3> parentStack[s_max + 1];
		glm::vec3 lastPos(0);			// store last pos for stack
		double h;					// ℎ = t'max is a threshold value for max used to prevent unnecessary 
									// writes to the stack
		
		// index -> idx;
		int convertIndex[8] = { 0, 1, 3, 2, 4, 5, 7, 6 }; // Refactor voxel construction to create uniform indexing

		// Get rid of small ray direction components to avoid division by zero.
		if (fabsf(dir.x) < epsilon) dir.x = copysignf(epsilon, dir.x);
 		if (fabsf(dir.y) < epsilon) dir.y = copysignf(epsilon, dir.y);
		if (fabsf(dir.z) < epsilon) dir.z = copysignf(epsilon, dir.z);

		// Calculate t_min and t_max
		double t_min;
		double t_max;

		findMinMax(origin, dir, pos, size, &t_min, &t_max);

		if (t_min != 9999)
		{
			// Move the origin to the nearest face; smallest positive t value
			origin += t_min * dir;
			t_max -= t_min;
			t_min = 0;
		}

		h = t_max;
		size /= 2;
		idx = getIdx(origin, &pos, size);

		octreeBoxes.clear();
		raycastCube = glm::vec3(-1);

		if(verbose) std::cout << "Scale\t idx\t size\t t_min\t t_max\t C_min\t C_max\t h\t pos \n" << "¯¯¯¯¯\t ¯¯¯\t ¯¯¯¯\t ¯¯¯¯¯\t ¯¯¯¯¯\t ¯¯¯¯¯\t ¯¯¯¯¯\t ¯\t ¯¯¯ \n";

		// while we havent left the octree or hit a voxel
		while (scale < s_max)
		{
			double c_min;
			double c_max;
			findMinMax(origin, dir, pos, size, &c_min, &c_max);

			// Add pos progression to list
			octreeBoxes.push_back(glm::vec4(pos, exp2f(scale)));

			if (verbose) {
				std::stringstream ss;
				ss << std::fixed << std::setprecision(2);
				ss << scale << "\t " << idx << "\t " << size << "\t " << t_min << "\t " << t_max << "\t " << c_min << "\t " << c_max << "\t " << h << "\t [" << pos.x << "," << pos.y << "," << pos.z << "] \n";
				std::cout << ss.str();
			}

			if (c_max == -1) return;
			if (idx == -1) return;

			// _______PUSH________
			// 	check if we need to process current voxel or skip it 
			t_min = fmaxf(t_min, c_min);
			double tc_max = fminf(c_max, t_max);
			if (parent->validMask & (int)exp2(idx) && t_min <= tc_max)
			{
				/// check whether the voxel is small enough to justify termination of the traversal
				//std::cout << "PUSHING" << std::endl;

				// If the current voxel is a leaf, terminate search
				if (parent->leafMask & (int)exp2(idx)) {
					if (verbose) std::cout << "Leaf found! \n";
					break;
				}


				// store the old values of parent and tmax to the stack if necessary. 
				if (c_max < h) parentStack[scale] = std::make_tuple(parent, t_max, lastPos);
				//parentStack[scale] = std::make_tuple(parent, t_max, lastPos);

				lastPos = pos;
				h = c_max;
				t_max = c_max;
				size /= 2;
				scale--;

				// replace parent with the current voxel and set idx, pos, and scale to match the first child voxel
				// that the ray enters
				int pointerIndex = -1;
				for (int i = 0; i <= idx; i++) // determine index child in childpointer vector
				{
					if (parent->validMask & (int)exp2(i)) pointerIndex++;
				}
				parent = parent->childPointers[pointerIndex];

				// Get next idx in child voxel
				glm::dvec3 point = origin + (c_min + 0.00001f) * dir;
				idx = getIdx(point, &pos, size);

				continue;
			}
	
			int index = convertIndex[idx];
			if (index & 2) pos.x -= size;
			if (index & 4) pos.y -= size;
			if (index & 1) pos.z -= size;

			glm::dvec3 point = origin + (c_max + 0.00001f) * dir; // Nudge to move past 0 plane
			idx = getIdx(point, &pos, size);
			
			t_min = c_max;

			// _______POP________
			if (idx == -1) 
			{
				//std::cout << "POPPING" << std::endl;

				scale++;
				if (scale >= s_max) {
					if (verbose) std::cout << "Scale exceeded.  \n";
					return;
				}

				if (std::get<0>(parentStack[scale]) != NULL) 
				{
					parent = std::get<0>(parentStack[scale]);
					t_max = std::get<1>(parentStack[scale]);
					pos = std::get<2>(parentStack[scale]);
				}
				h = 0;
				size *= 2;

				while (t_max <= c_max) 
				{
					scale++;
					if (scale >= s_max) {
						if (verbose) std::cout << "Scale exceeded. \n";
						return;
					}					
					size *= 2;

					parent = std::get<0>(parentStack[scale]);
					t_max = std::get<1>(parentStack[scale]);
					pos = std::get<2>(parentStack[scale]);
				}

				idx = getIdx(point, &pos, size);
			}
		}

		raycastCube = pos;
		raycastingLeafs.insert(glm::vec4(pos, exp2f(scale)));
		//Chunk::square.Draw(shader, pos, 1);
	}
};