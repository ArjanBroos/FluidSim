#version 330

uniform vec3 cameraSpherePos;
uniform float sphereRadius;
uniform vec3 cameraLightDir;
uniform mat4 projectionMatrix;

in vec2 mapping;
out vec4 outputColor;

void main() {
	// Discard pixels outside sphere
	float ls = dot(mapping, mapping);
	if (ls > 1) discard;

	// Calculate point on sphere
	vec3 cameraNormal = vec3(mapping, sqrt(1 - ls));
	vec3 cameraPos = (cameraNormal * sphereRadius) + cameraSpherePos;
	
	// Calculate lighting
	vec3 color = vec3(0.1, 0.1, 1);
	float ambient = 0.2f;
	float diffuse = max(0, dot(cameraNormal, -cameraLightDir));

	// Calculate depth for point on sphere
	vec4 clipPos = projectionMatrix * vec4(cameraPos, 1);
	float ndcDepth = clipPos.z / clipPos.w;
	gl_FragDepth = (gl_DepthRange.diff * ndcDepth +
		+ gl_DepthRange.near + gl_DepthRange.far) / 2;

	// Output fragment color
	outputColor = vec4(color * (ambient + diffuse), 0.5);
}