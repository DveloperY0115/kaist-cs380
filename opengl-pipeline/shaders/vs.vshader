#version 130

in vec3 aPos;
in vec4 aColor;

out vec4 vColor;
void main() 
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vColor = aColor;
}