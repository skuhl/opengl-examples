#version 130 // Specify which version of GLSL we are using.

in vec3 in_Position;
in vec3 in_Normal;

out vec4 out_Normal;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 NormalMat;

uniform int red;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	vec4 normal = vec4(in_Normal.x, in_Normal.y, in_Normal.z, 0);
	out_Normal = normalize(NormalMat * normal);
	
	gl_Position = Projection * ModelView * pos;
}
