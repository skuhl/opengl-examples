#version 130 // Specify which version of GLSL we are using.
#pragma optimize (off)

// in_Position was bound to attribute index 0("shaderAttribute")
in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;

uniform mat4 ModelView;
uniform mat4 Projection;

out vec2 out_TexCoord;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	out_TexCoord = in_TexCoord;
	gl_Position = Projection * ModelView * pos;
}
