#version 130

uniform float uColorTemp;
uniform float uTime;

in vec3 vColor;

out vec4 fragColor;

void main(void) {
vec4 color = vec4(vColor.x, vColor.y, vColor.z, 1);
if (uColorTemp == 1.0) {
	  // R
	  color.x = 1.0;
	  color.y = 0.0;
	  color.z = 0.0;
  }
  else if (uColorTemp == 2.0) {
	  // G
	  color.x = 0.0;
	  color.y = 1.0;
	  color.z = 0.0;
  }
  else if (uColorTemp == 3.0) {
	  // B
	  color.x = 0.0;
	  color.y = 0.0;
	  color.z = 1.0;
  }
  fragColor = color;
}
