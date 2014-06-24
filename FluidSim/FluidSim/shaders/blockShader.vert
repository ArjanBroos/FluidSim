#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
uniform mat4 mvpMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform vec4 specularColor;
uniform vec3 lightDir;

out vec4 drawColor;

void main() {
	vec4 cameraPos = viewMatrix * vec4(0, 0, 0, 1);
	vec4 eyeVertexPos = modelViewMatrix * position;
	vec4 eyeLookDir = normalize(eyeVertexPos - cameraPos);
	vec4 eyeNormal = normalMatrix * vec4(normal, 0);
	vec4 eyeLightDir = viewMatrix * vec4(-lightDir, 0);
	vec4 eyeReflectDir = reflect(-eyeLightDir, eyeNormal);

	vec4 lightColor = vec4(1, 1, 1, 1);
	float cosTheta = clamp(dot(eyeNormal, eyeLightDir), 0, 1);
	float cosAlpha = clamp(dot(eyeLookDir, eyeReflectDir), 0, 1);

	gl_Position = mvpMatrix * position;
	drawColor = ambientColor +
		diffuseColor * cosTheta * lightColor +
		specularColor * pow(cosAlpha, 10);
}