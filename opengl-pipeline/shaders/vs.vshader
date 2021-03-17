#version 130

in vec3 aPos;
in vec4 aColor;
in float aOpacity;

out vec4 vColor;
out float vOpacity;
void main() 
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vColor = aColor;
	vOpacity = aOpacity;
}