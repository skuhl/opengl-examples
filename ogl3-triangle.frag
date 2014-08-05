#version 130 // Specify which version of GLSL we are using.
 
out vec4 fragColor;
uniform int red;

void main() 
{
	if(red > 0)
		fragColor = vec4(1.0,.2,.1,1.0); // Red
	else
		fragColor = vec4(1.0,1.0,1.0,1.0); // White
}
