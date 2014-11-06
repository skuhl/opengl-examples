#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in vec2 out_TexCoord;
in vec3 out_Normal;   // normal vector in eye coordinates
in vec3 out_Color;
in float out_Depth;   // Depth of fragment (range 0 through 1)
in vec3 out_EyeCoord; // position of fragment in eye coordinates

uniform sampler2D tex;
uniform int renderStyle;


void main() 
{
	if(renderStyle == 0)
	{
		// head-lamp style diffuse shading
		vec3 camLook = normalize(-out_EyeCoord.xyz);
		float diffuse = clamp(dot(camLook, normalize(out_Normal.xyz)), 0,1) / 2+.5;
		fragColor = vec4(diffuse,diffuse,diffuse, 1);
	}
	else if(renderStyle == 1)
	{
		/* Color value from the texture */
		fragColor = texture(tex, out_TexCoord);
	}
	else if(renderStyle == 2)
	{
		/* Color value embedded in the file */
		fragColor.xyz = out_Color;
		fragColor.a = 1;
	}
	else if(renderStyle == 3)
	{
		/* Normal coloring: Each component in the normals ranges from -1 to 1. Make them range from 0 to 1. */
		fragColor.xyz = (normalize(out_Normal) + 1)/2;
		fragColor.a = 1;
	}
	else if(renderStyle == 4)
	{
		/* Color based on texture coordinates */
		fragColor = vec4(out_TexCoord, 0, 1);
	}
	else if(renderStyle == 5)
	{
		if(gl_FrontFacing) // based on triangle winding
			fragColor = vec4(0,.3,0,1); // green are front faces
		else
			fragColor = vec4(.3,0,0,1); // red are back faces
	}
	else if(renderStyle == 6)
	{
		/* out_EyeCoord is the position of this fragment in eye
		 * coordinates. Since the camera is at 0,0,0 in eye
		 * coordinates, we can interpret it as a vector pointing from
		 * the camera to the fragment. out_Normal is a normal vector
		 * on the triangle. If the sign of the dot product is
		 * negative, the angle between the vectors is greater than 90
		 * degrees---telling us that they are pointing in "opposite"
		 * directions. */
		if(dot(out_Normal, out_EyeCoord) < 0)
			fragColor = vec4(0,.3,0,1); // green are front faces
		else
			fragColor = vec4(.3,0,0,1); // red are back faces
	}
	
	else if(renderStyle == 7)
	{
		fragColor = vec4(out_Depth, out_Depth, out_Depth, 1);
	}

}
