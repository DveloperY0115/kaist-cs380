uniform float uColorTemp;
uniform float uTime;

varying vec3 vColor;

void main(void) {
  vec4 color = vec4(vColor.x, vColor.y, vColor.z, 1);
  gl_FragColor = color;
}
