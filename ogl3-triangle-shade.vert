#version 130 // Specify which version of GLSL we are using.

in vec3 in_Position;
in vec3 in_Normal;

out vec4 out_Normal;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 NormalMat;

uniform int red;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	vec3 transformedNormal = normalize(NormalMat * in_Normal);
	out_Normal = vec4(transformedNormal.x, transformedNormal.y, transformedNormal.z, 0);
	
	
	gl_Position = Projection * ModelView * pos;
}
