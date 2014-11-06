#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
in vec3 in_Color;

in vec4 in_BoneIndex;
in vec4 in_BoneWeight;
uniform mat4 BoneMat[128];
uniform int NumBones;

uniform float farPlane;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 GeomTransform;

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

	mat4 actualModelView;
	if(NumBones > 0)
	{
		mat4 m = in_BoneWeight.x * BoneMat[int(in_BoneIndex.x)] +
			in_BoneWeight.y * BoneMat[int(in_BoneIndex.y)] +
			in_BoneWeight.z * BoneMat[int(in_BoneIndex.z)] +
			in_BoneWeight.w * BoneMat[int(in_BoneIndex.w)];
		actualModelView = ModelView * m;
	}
	else
		actualModelView = ModelView * GeomTransform;

	// Transform normal from object coordinates to camera coordinates
	//out_Normal = normalize(NormalMat * in_Normal);
	out_Normal = transpose(inverse(mat3(actualModelView)))*in_Normal.xyz;

	// Transform vertex from object to unhomogenized Normalized Device
	// Coordinates (NDC).
	gl_Position = Projection * actualModelView * vec4(in_Position.xyz, 1);

	// For rendering depth onto screen:
	// To avoid dealing with issues from non-linear z in perspective
	// projection, we simply transform our point into camera
	// coordinates and divide by the far plane. When the point is at
	// the far plane, it will be white. When it is at the camera (it
	// will be black). This calculation doesn't account for the near
	// plane.
	out_Depth = ((actualModelView*vec4(in_Position.xyz, 1)).z)/-farPlane ;

	// Calculate the position of the vertex in eye coordinates:
	out_EyeCoord = vec3(actualModelView * vec4(in_Position.xyz, 1));
}
