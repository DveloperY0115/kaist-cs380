#version 130
		
in vec3 vColor;
out vec4 FragColor;

void main() 
{
	FragColor = vec4(vColor.x, vColor.y, vColor.z, 1);
}