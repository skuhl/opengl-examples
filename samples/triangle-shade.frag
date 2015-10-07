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

void main() 
{
	// A vector pointing from the position of the fragment to the
	// camera (both in camera coordinates).
	vec3 camLook = normalize(vec3(0,0,0) - out_VertexPos.xyz);

	// Calculate diffuse lighting:
	float diffuse = dot(camLook, out_Normal.xyz);
	diffuse = abs(diffuse); // Light front and back the same way

	// Don't let the diffuse term get too small (otherwise object
	// would be black!). We do this by making the diffuse term range
	// from .5 to 1 instead of 0 to 1
	diffuse = diffuse / 2 + .5;  

	if(red > 0)
		fragColor = diffuse * vec4(1.0, 0.2, 0.1, 1.0); // Red
	else
		fragColor = diffuse * vec4(1.0, 1.0, 1.0, 1.0); // White
}
