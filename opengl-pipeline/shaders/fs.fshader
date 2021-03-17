#version 130
		
in vec4 vColor;
in float vOpacity;

out vec4 FragColor;

void main() 
{
	FragColor = vec4(vColor.x, vColor.y, vColor.z, vColor.a - (vOpacity));
}