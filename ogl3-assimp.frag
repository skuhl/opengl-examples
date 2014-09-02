#version 130 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec2 out_TexCoord;
in vec3 out_Normal;
in vec3 out_Color;
in float out_Depth;   // Depth of fragment (range 0 through 1)
in vec3 out_EyeCoord; // position of fragment in eye coordinates

uniform sampler2D tex;
uniform int renderStyle;


void main() 
{
	if(renderStyle == 0)
	{
		/* Color value from the texture */
		fragColor.xyz = vec3(texture(tex, out_TexCoord));
	}
	else if(renderStyle == 1)
	{
		/* Color value embedded in the file */
		fragColor.xyz = out_Color;
	}
	else if(renderStyle == 2)
	{
		/* Normal coloring: Each component in the normals ranges from -1 to 1. Make them range from 0 to 1. */
		fragColor.xyz = (out_Normal + vec3(1,1,1))/2;
	}
	else if(renderStyle == 3)
	{
		/* Color based on texture coordinates */
		fragColor.xyz = vec3(out_TexCoord, 0);
	}
	else if(renderStyle == 4)
	{
		if(gl_FrontFacing) // based on triangle winding
			fragColor.xyz = vec3(0,.3,0); // green are front faces
		else
			fragColor.xyz = vec3(.3,0,0); // red are back faces
	}
	else if(renderStyle == 5)
	{
		if(dot(out_Normal, out_EyeCoord) < 0)
//		if(out_Normal.z > 0)
			fragColor.xyz = vec3(0,.3,0); // green are front faces
		else
			fragColor.xyz = vec3(.3,0,0); // red are back faces
	}
	
	else if(renderStyle == 6)
	{
		fragColor.xyz = vec3(out_Depth, out_Depth, out_Depth);
	}
		

	// Change this depending on how you want the image to be rendered.
//	fragColor.xyz = out_Barycentric;
	
	
}
