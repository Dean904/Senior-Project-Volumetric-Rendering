#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 ourColor;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 inColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	fragPos = vec3(model * vec4(aPos, 1.0f));
	normal = mat3(transpose(inverse(model))) * aNormal;  

	// Inversing matrices is a costly operation even for shaders so wherever possible, try to avoid doing inverse operations in shaders since they have to be done on each vertex of your scene. For learning purposes this is fine, but for an efficient application you'll likely want to calculate the normal matrix on the CPU and send it to the shaders via a uniform before drawing (just like the model matrix).

	if(aColor.x != 0 || aColor.y != 0 || aColor.z != 0) ourColor = aColor;
	else ourColor = inColor;
}