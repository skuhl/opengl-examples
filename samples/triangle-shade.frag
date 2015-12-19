#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;

/* These "in" variables are interpolated from the values calculated
 * per vertex in the vertex program. They are the output of our vertex
 * program. For this program, I have named the variables so they are
 * correct in the vertex program---and they must have the same name in
 * the fragment program. */
in vec4 out_VertexPos; /* Fragment position (in camera
                        * coordinates). Although it is named
                        * VertexPos, is actually the fragment position
                        * because it has been interpolated across the
                        * face of the triangle. */
in vec4 out_Normal;    // normal vector   (camera coordinates)

uniform int red;


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
	vec3 lightDir = lightPos - out_VertexPos.xyz;

	/* Calculate diffuse shading */
	float diffuse = diffuseScalar(out_Normal.xyz, lightDir, true);

	if(red > 0)
		fragColor = diffuse * vec4(1.0, 0.2, 0.1, 1.0); // Red
	else
		fragColor = diffuse * vec4(1.0, 1.0, 1.0, 1.0); // White
}
