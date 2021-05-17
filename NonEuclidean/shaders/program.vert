#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat3 MVit;

uniform int isClipping;

out vec3 pos;
out vec3 normal;

const vec4 plane1 = vec4(-1, 0, 0, 0);
const vec4 plane2 = vec4(1, 0, 0, 0);

void main(){
  gl_Position = MVP * vec4(aPosition, 1.0);
  normal = normalize(MVit * aNormal);
  pos = aPosition;

  if(isClipping == 1) {
    gl_ClipDistance[0] = dot(modelMat * vec4(pos, 1.0), plane1);
  }
  else if(isClipping == 2) {
    gl_ClipDistance[0] = dot(modelMat * vec4(pos, 1.0), plane2);
  }
  else {
    gl_ClipDistance[0] = 1.0;
  }

}