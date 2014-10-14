#version 130 // Specify which version of GLSL we are using.
 
out vec4 fragColor;
in vec2 out_TexCoord;

uniform sampler2D tex;


void main() 
{
		float k1 = .7;
		float texScale = 1;

		//		fragColor = vec4(1.0,1.0,1.0,1.0); // White
//		fragColor = vec4(out_TexCoord.x, out_TexCoord.y, 1.0, 1.0);
//		fragColor = texture(tex, out_TexCoord);

		vec2 screenCenter = vec2(0.5, 0.5);
		float norm = length(screenCenter);  /* Distance between corner and center */

		vec2 radial_vector = ( out_TexCoord.st - screenCenter ) / norm;
		float radial_vector_len = length(radial_vector);
		vec2 radial_vector_unit = radial_vector / radial_vector_len;

		// Compute the new distance from the screen center.	 
		float new_dist = radial_vector_len + k1 * pow(radial_vector_len,3.0);

		// Find the coordinate we want to lookup
		vec2 warp_coord = radial_vector_unit * (new_dist * norm);
		
		// Scale the image vertically and horizontally to get it to fill the screen
		warp_coord = warp_coord * texScale;

		// Translate the coordinte such that the (0,0) is back at the screen center
		warp_coord = warp_coord + screenCenter;

	/* If we lookup a texture coordinate that is not on the texture, return a solid color */
	if ((warp_coord.s > 1  || warp_coord.s < 0.0) ||
	    (warp_coord.t > 1 || warp_coord.t < 0.0))
		fragColor = vec4(0,0,0,1); // black
	else
		fragColor = texture(tex, warp_coord);  // lookup into the texture


}
