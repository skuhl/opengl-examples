#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec3 in_Color;

uniform mat4 ModelView;
uniform mat4 Projection;

out vec3 color;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	color = in_Color;
	gl_Position = Projection * ModelView * pos;
}
