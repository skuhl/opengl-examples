#version 130 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec2 out_TexCoord;
in vec3 out_Normal;
in vec3 out_Color;

uniform sampler2D tex;

void main() 
{
	/* Color value from the texture */
	vec3 textureColor = vec3(texture(tex, out_TexCoord));

	/* Color value embedded in the file */
	vec3 color = out_Color;
	
	/* Normal coloring: Each component in the normals ranges from -1 to 1. Make them range from 0 to 1. */
	vec3 normalColor = (out_Normal + vec3(1,1,1))/2;
	/* Color based on texture coordinates */
	vec3 texcoordColor = vec3(out_TexCoord, 0);

	// Change this depending on how you want the image to be rendered.
	fragColor.xyz = textureColor;

}
