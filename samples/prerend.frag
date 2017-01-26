#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;
in vec2 out_TexCoord;

uniform sampler2D tex;

void main() 
{
	// k1 and k2 are parameters that impact pincushion distortion. k2
	// impacts the edges of the screen more than the middle. Each of
	// the k1 values are applied to each of the color channels. Use
	// the same value for all three color channels if you do not want
	// to account for chromatic aberration.
	float k1[3];
	k1[0] = .3;
	k1[1] = k1[0]+.1;
	k1[2] = k1[0]+.28;
	float texScale = 1;

	float k2[3];
	k2[0] = 0;
	k2[1] = k2[0] - .18;
	k2[2] = k2[0] - .48;
	
	
	//		fragColor = vec4(1.0,1.0,1.0,1.0); // White
//		fragColor = vec4(out_TexCoord.x, out_TexCoord.y, 1.0, 1.0);
//		fragColor = texture(tex, out_TexCoord);

	vec2 screenCenter = vec2(0.5, 0.5);
	float norm = length(screenCenter);  /* Distance between corner and center */

	vec2 radial_vector = ( out_TexCoord.st - screenCenter ) / norm;
	float radial_vector_len = length(radial_vector);
	vec2 radial_vector_unit = radial_vector / radial_vector_len;


	vec2 warp_coord[3];
	for(int i=0; i<3; i++)
	{
	
		// Compute the new distance from the screen center.	 
		float new_dist = radial_vector_len + k1[i] * pow(radial_vector_len,3.0) + k2[i]  * pow(radial_vector_len,5.0);

		// Find the coordinate we want to lookup
		warp_coord[i] = radial_vector_unit * (new_dist * norm);

		// Scale the image vertically and horizontally to get it to fill the screen
		warp_coord[i] = warp_coord[i] * texScale;

		// Translate the coordinte such that the (0,0) is back at the screen center
		warp_coord[i] = warp_coord[i] + screenCenter;
	}

	for(int i=0; i<3; i++)
	{
		/* If we lookup a texture coordinate that is not on the texture, return a solid color */
		if ((warp_coord[i].s > 1  || warp_coord[i].s < 0.0) ||
		    (warp_coord[i].t > 1 || warp_coord[i].t < 0.0))
		{
			if(i==0)
				fragColor.r = 0;
			if(i==1)
				fragColor.g = 0;
			if(i==2)
				fragColor.b = 0;
			fragColor.a = 1;
			}
			
		else
		{
			if(i==0)
				fragColor.r = texture(tex, warp_coord[i]).r;  // lookup into the texture
			if(i==1)
				fragColor.g = texture(tex, warp_coord[i]).g;  // lookup into the texture
			if(i==2)
				fragColor.b = texture(tex, warp_coord[i]).b;  // lookup into the texture
		}
	}

	
	

}
