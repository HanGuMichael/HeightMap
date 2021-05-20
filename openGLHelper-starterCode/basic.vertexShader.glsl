#version 150

in vec3 position;
in vec4 color;
in vec3 left;
in vec3 right;
in vec3 up;
in vec3 down;

out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int mode;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  if(mode == 1){
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
	col = color;
  }else if(mode == 2){
    float smoothHeight = (left.x + right.x + up.x + down.x)/4; 
	vec3 smoothpos = vec3(position.x,smoothHeight,position.z);
	gl_Position = projectionMatrix * modelViewMatrix * vec4(smoothpos, 1.0f);
	col = smoothHeight * color / position.y;
  }
  
}

