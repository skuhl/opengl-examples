#version 130 // Specify which version of GLSL we are using.
 
out vec4 fragColor;

in vec4 out_VertexPos; // vertex position (camera coordinates)
in vec4 out_Normal;    // normal vector   (camera coordinates)

uniform int red;

void main() 
{
	// A vector pointing from the position of the fragment to the
	// camera (both in camera coordinates).
	vec3 camLook = normalize(vec3(0,0,0) - out_VertexPos.xyz);

	// Calculate diffuse lighting:
	float diffuse = dot(camLook, out_Normal.xyz);
	diffuse = abs(diffuse); // Light front and back the same way:

	// Don't let the diffuse term get too small (otherwise object
	// would be black!). We do this by making the diffuse term range
	// from .5 to 1 instead of 0 to 1
	diffuse = diffuse / 2 + .5;  

	if(red > 0)
		fragColor = diffuse * vec4(1.0, 0.2, 0.1, 1.0); // Red
	else
		fragColor = diffuse * vec4(1.0, 1.0, 1.0, 1.0); // White
}
