#pragma once

#include <iostream>

#include <Camera.cpp>


class Animator
{

private:

	Camera* camera;
	bool currentlyCirculating = false;

	glm::vec3 centerOfMap;
	float movementSpeed;

	glm::vec3 originalPos;
	//float distance;



public:

	Animator() {}
	Animator(Camera* _camera, glm::vec3 _centerofMap)
	{
		camera = _camera;
		centerOfMap = _centerofMap;
		movementSpeed = camera->MovementSpeed;
	}


	void startCirculationAnimation()
	{
		currentlyCirculating = true;
		originalPos = camera->Position;
	}

	void tick(GLfloat deltaTime, glm::vec3 cameraPos)
	{
		if (currentlyCirculating)
		{
			glm::vec3 dir = glm::normalize(centerOfMap - cameraPos);
			glm::vec3 tangent = glm::normalize(glm::cross(dir, cameraPos));

			GLfloat velocity = movementSpeed * deltaTime;
			
			std::cout << (int)cameraPos.x << " " << (int)cameraPos.y << " " << (int)cameraPos.z << "\n";
			camera->move(camera->Position + (tangent * velocity));
			std::cout << (int)camera->Position.x << " " << (int)camera->Position.y << " " << (int)camera->Position.z << "\n";


			if ((int)camera->Position.x == (int)originalPos.x && (int)camera->Position.z == (int)originalPos.z)
				currentlyCirculating = false;
		}



	}
};

