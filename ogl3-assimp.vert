#version 130 // Specify which version of GLSL we are using.

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
in vec3 in_Color;

uniform float farPlane;
uniform mat4 NormalMat;
uniform mat4 ModelView;
uniform mat4 Projection;

out vec2 out_TexCoord;
out vec3 out_Normal;
out vec3 out_Color;
out float out_Depth;
out vec3 out_EyeCoord;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	out_TexCoord = in_TexCoord;
	out_Normal = vec3(NormalMat*vec4(in_Normal, 0)); // object -> eye coordinates
	out_Normal = normalize(out_Normal);
	out_Color = in_Color;
	gl_Position = Projection * ModelView * pos; // object -> unhomogenized canonical view volume
	out_Depth = gl_Position.z / farPlane;
	out_EyeCoord = vec3(ModelView * pos);
}
