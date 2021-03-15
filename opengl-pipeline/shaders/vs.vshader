#version 130

in vec3 aPos;
in vec3 aColor;

out vec3 vColor;
void main() 
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vColor = aColor;
}