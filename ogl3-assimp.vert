#version 130 // Specify which version of GLSL we are using.

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
in vec3 in_Color;

uniform float farPlane;
uniform mat3 NormalMat;
uniform mat4 ModelView;
uniform mat4 Projection;

out vec2 out_TexCoord;
out vec3 out_Color;
out float out_Depth;
out vec3 out_Normal;   // normal vector (camera/eye coordinates)
out vec3 out_EyeCoord; // vertex position (camera/eye coordinates)

void main() 
{
	// Copy texture coordinates and color to fragment program
	out_TexCoord = in_TexCoord;
	out_Color = in_Color;

	// Transform normal from object coordinates to camera coordinates
	out_Normal = normalize(NormalMat * in_Normal);

	// Transform vertex from object to unhomogenized Normalized Device
	// Coordinates (NDC).
	gl_Position = Projection * ModelView * vec4(in_Position.xyz, 1);

	// For rendering depth onto screen:
	// To avoid dealing with issues from non-linear z in perspective
	// projection, we simply transform our point into camera
	// coordinates and divide by the far plane. When the point is at
	// the far plane, it will be white. When it is at the camera (it
	// will be black). This calculation doesn't account for the near
	// plane.
	out_Depth = ((ModelView*vec4(in_Position.xyz, 1)).z)/-farPlane ;

	// Calculate the position of the vertex in eye coordinates:
	out_EyeCoord = vec3(ModelView * vec4(in_Position.xyz, 1));
}
