#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in vec2 out_TexCoord; // Vertex texture coordinate
in vec3 out_Color;    // Vertex color
in vec3 out_Normal;   // Normal vector in camera coordinates
in vec3 out_CamCoord; // Position of fragment in camera coordinates

uniform int HasTex;    // Is there a texture in tex?
uniform sampler2D tex; // Diffuse texture
uniform int renderStyle;

/** Calculate diffuse shading. Normal and light direction do not need
 * to be normalized. */
float diffuseScalar(vec3 normal, vec3 lightDir, bool frontBackSame)
{
	/* Basic equation for diffuse shading */
	float diffuse = dot(normalize(lightDir), normalize(normal.xyz));

	/* The diffuse value will be negative if the normal is pointing in
	 * the opposite direction of the light. Set diffuse to 0 in this
	 * case. Alternatively, we could take the absolute value to light
	 * the front and back the same way. Either way, diffuse should now
	 * be a value from 0 to 1. */
	if(frontBackSame)
		diffuse = abs(diffuse);
	else
		diffuse = clamp(diffuse, 0, 1);

	/* Keep diffuse value in range from .5 to 1 to prevent any object
	 * from appearing too dark. Not technically part of diffuse
	 * shading---however, you may like the appearance of this. */
	diffuse = diffuse/2 + .5;

	return diffuse;
}


void main() 
{
	/* Get position of light in camera coordinates. When we do
	 * headlight style rendering, the light will be at the position of
	 * the camera! */
	vec3 lightPos = vec3(0,0,0);

	/* Calculate a vector pointing from our current position (in
	 * camera coordinates) to the light position. */
	vec3 lightDir = lightPos - out_CamCoord.xyz;

	/* Calculate diffuse shading */
	float diffuse = diffuseScalar(out_Normal, lightDir, true);

	
	if(renderStyle == 0)
	{
		fragColor = vec4(diffuse,diffuse,diffuse, 1);
	}
	else if(renderStyle == 1)
	{
		/* Color value from the texture */
		if(bool(HasTex))
			fragColor = texture(tex, out_TexCoord);
		else
			fragColor = vec4(out_Color, 1);
	}
	else if(renderStyle == 2)
	{
		/* Color value from the texture */
		if(bool(HasTex))
			fragColor = texture(tex, out_TexCoord);
		else
			fragColor = vec4(out_Color, 1);
		// include diffuse
		fragColor.xyz = fragColor.xyz * diffuse;
	}
	else if(renderStyle == 3)
	{
		/* Color value embedded in the file */
		fragColor.xyz = out_Color;
		fragColor.a = 1;
	}
	else if(renderStyle == 4)
	{
		/* Diffuse (headlamp style) with vertex colors */
		vec3 camLook = normalize(-out_CamCoord.xyz);
		float diffuse = clamp(dot(camLook, normalize(out_Normal.xyz)), 0,1) / 2+.5;
		fragColor = vec4(out_Color * diffuse, 1);
	}
	else if(renderStyle == 5)
	{
		/* Normal coloring: Each component in the normals ranges from -1 to 1. Make them range from 0 to 1. */
		fragColor.xyz = (normalize(out_Normal) + 1)/2;
		fragColor.a = 1;
	}
	else if(renderStyle == 6)
	{
		/* Color based on texture coordinates */
		fragColor = vec4(out_TexCoord, 0, 1);
	}
	else if(renderStyle == 7)
	{
		if(gl_FrontFacing) // based on triangle winding
			fragColor = vec4(0,.3,0,1); // green are front faces
		else
			fragColor = vec4(.3,0,0,1); // red are back faces
	}
	else if(renderStyle == 8)
	{
		/* out_CamCoord is the position of this fragment in camera
		 * coordinates. Since the camera is at 0,0,0 in camera
		 * coordinates, we can interpret it as a vector pointing from
		 * the camera to the fragment. out_Normal is a normal vector
		 * on the triangle. If the sign of the dot product is
		 * negative, the angle between the vectors is greater than 90
		 * degrees---telling us that they are pointing in "opposite"
		 * directions. */
		if(dot(out_Normal, out_CamCoord) < 0)
			fragColor = vec4(0,.3,0,1); // green are front faces
		else
			fragColor = vec4(.3,0,0,1); // red are back faces
	}
	
	else if(renderStyle == 9)
	{
		/* gl_FragCoord is the position of the fragment in
		 * NDC. However, the Z value is also linearly transformed to
		 * be within a range specified by glDepthRange()---which
		 * defaults to 0=near and 1=far. Despite the transformation,
		 * there Z value is still non-linear (as it is in NDC). We use
		 * pow() to trasnform the values so that the depth value
		 * differences are easier to see. Without it, all values that
		 * aren't very close to the camera have values close to 1
		 * (because there is a lot of resolution near the camera). */
		float depth = pow(gl_FragCoord.z, 50);
		fragColor = vec4(depth, depth, depth, 1);
	}
}
