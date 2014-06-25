#version 330

uniform vec3 cameraSpherePos;
uniform float sphereRadius;
uniform vec3 cameraLightDir;

in vec2 mapping;
out vec4 outputColor;

void main() {
	float ls = dot(mapping, mapping);
	if (ls > 1) discard;

	vec3 cameraNormal = vec3(mapping, sqrt(1 - ls));
	vec3 cameraPos = (cameraNormal * sphereRadius) + cameraSpherePos;
	
	vec3 color = vec3(0, 0, 1);
	float ambient = 0.2f;
	float diffuse = max(0, dot(cameraNormal, -cameraLightDir));

	outputColor = vec4(color * (ambient + diffuse), 1);
}