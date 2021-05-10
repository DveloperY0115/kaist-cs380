#version 130

uniform vec3 uLight, uLight2, uColor;

in vec3 vNormal;
in vec3 vPosition;

out vec4 fragColor;

void main() {
	// to light vectors
	vec3 toLight = normalize(uLight - vPosition);
	vec3 toLight2 = normalize(uLight2 - vPosition);

	vec3 normal = normalize(vNormal);
	vec3 toEye = normalize(-vPosition);

	float nDotLight = dot(normal, toLight);
	float nDotLight2 = dot(normal, toLight2);

	// bounce-ray vectors
	vec3 bounce = normalize(2.0 * normal * nDotLight - toLight);
	vec3 bounce2 = normalize(2.0 * normal * nDotLight2 - toLight2);

	float rDotBounce = max(0.0, dot(bounce, toEye));
	float rDotBounce2 = max(0.0, dot(bounce2, toEye));

	// contribution of diffuse & specular to final output
	float specular = pow(rDotBounce, 64.0) + pow(rDotBounce2, 64.0);
	float diffuse = max(0.0, nDotLight) + max(0.0, nDotLight2);

	vec3 intensity = uColor * (diffuse + 0.2) + vec3(0.4, 0.4, 0.4) * specular;
	
	fragColor = vec4(intensity.x, intensity.y, intensity.z, 1);
}