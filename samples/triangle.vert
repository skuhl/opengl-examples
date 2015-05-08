#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;

uniform mat4 ModelView;
uniform mat4 Projection;

uniform int red;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	gl_Position = Projection * ModelView * pos;
}
