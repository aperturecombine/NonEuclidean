#version 330 core

in vec3 pos;
in vec3 normal;

uniform vec3 diffuseColor;
uniform vec3 lightPos;
uniform vec3 camPos;

out vec3 color;

void main() {
  vec3 lightDir = normalize(lightPos - pos);
  float diffuse = max(dot(normal, lightDir), 0.0);

  color = (0.2 + diffuse) * diffuseColor;
}