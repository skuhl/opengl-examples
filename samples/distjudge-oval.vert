#version 150 // GLSL 150 = OpenGL 3.2

// in_Position was bound to attribute index 0("shaderAttribute")
in vec3 in_Position;
in vec2 in_TexCoord;

out vec2 out_TexCoord;

void main() 
{
	out_TexCoord = in_TexCoord;

	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	gl_Position = pos;
}