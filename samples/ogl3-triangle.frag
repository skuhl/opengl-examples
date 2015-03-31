#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;
uniform int red;

void main() 
{
	if(red > 0)
		fragColor = vec4(1.0,.2,.1,1.0); // Red
	else
		fragColor = vec4(1.0,1.0,1.0,1.0); // White
}
