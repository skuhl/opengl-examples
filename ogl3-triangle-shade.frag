#version 130 // Specify which version of GLSL we are using.
 
out vec4 fragColor;

in vec4 out_Normal;

uniform int red;

void main() 
{
	vec4 camLook = vec4(0,0,-1,0);
	float diffuse = dot(camLook, out_Normal);
	// Light front and back of triangle the same way:
	diffuse = abs(diffuse);

	// Don't let the diffuse term get too small (otherwise object
	// would be black!). We do this by making the diffuse term range
	// from .5 to 1 instead of 0 to 1
	diffuse = diffuse / 2 + .5;  

	if(red > 0)
		fragColor = diffuse * vec4(1.0, 0.2, 0.1, 1.0); // Red
	else
		fragColor = diffuse * vec4(1.0, 1.0, 1.0, 1.0); // White
}
