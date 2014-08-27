#version 130 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec2 out_TexCoord;

uniform sampler2D tex;

void main() 
{
	fragColor = texture(tex, out_TexCoord);
}
