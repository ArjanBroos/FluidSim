#version 330

uniform vec3 cameraSpherePos;
uniform float sphereRadius;
uniform mat4 projectionMatrix;

out vec2 mapping;

void main() {
	switch(gl_VertexID) {
	case 0: // Bottom left
		mapping = vec2(-1, -1);
		break;
	case 1: // Top left
		mapping = vec2(-1, 1);
		break;
	case 2: // Bottom right
		mapping = vec2(1, -1);
		break;
	case 3: // Top right
		mapping = vec2(1, 1);
		break;
	}

	vec4 cameraCornerPos = vec4(cameraSpherePos, 1);
	cameraCornerPos.xy += mapping * sphereRadius;

	gl_Position = projectionMatrix * cameraCornerPos;
}