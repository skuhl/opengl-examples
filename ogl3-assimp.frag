#version 130 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec2 out_TexCoord;
in vec3 out_Normal;
in vec3 out_Color;

uniform bool texPresent;
uniform sampler2D tex;

void main() 
{
	if(texPresent)
	{
		fragColor = texture(tex, out_TexCoord);
	}
	else
	{
	fragColor.xyz = out_Color;
	
	/* Normal coloring: Each component in the normals ranges from -1 to 1. Make them range from 0 to 1. */
	//fragColor.xyz = (out_Normal + vec3(1,1,1))/2;

	/* Texture coordinates */
	// fragColor.xy = out_TexCoord;

	//fragColor.xyz = vec3(1,1,0); // yellow

	}

}
