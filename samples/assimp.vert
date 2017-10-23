#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
in vec3 in_Color;

in vec4 in_BoneIndex;
in vec4 in_BoneWeight;
uniform mat4 BoneMat[128];
uniform int NumBones;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 GeomTransform;

out vec2 out_TexCoord;
out vec3 out_Color;
out vec3 out_Normal;   // normal vector (camera coordinates)
out vec3 out_CamCoord; // vertex position (camera coordinates)

void main() 
{
	// Copy texture coordinates and color to fragment program
	out_TexCoord = in_TexCoord;
	out_Color = in_Color;

	/* Calculate the actual modelview matrix: */
	mat4 actualModelView;
	if(NumBones > 0)
	{
		/* If we have an animated model/character that contains bones,
		   we need to account for the bone matrices. */
		mat4 m = in_BoneWeight.x * BoneMat[int(in_BoneIndex.x)] +
		         in_BoneWeight.y * BoneMat[int(in_BoneIndex.y)] +
		         in_BoneWeight.z * BoneMat[int(in_BoneIndex.z)] +
		         in_BoneWeight.w * BoneMat[int(in_BoneIndex.w)];
		actualModelView = ModelView * m;
	}
	else
		/* If we have a model without animation/bones in it, we simply
		 * need to account for the GeomTransform matrix embedded in
		 * the 3D model. */
		actualModelView = ModelView * GeomTransform;

	mat3 NormalMat = transpose(inverse(mat3(actualModelView)));
	
	// Transform normal from object coordinates to camera coordinates
	out_Normal = NormalMat * in_Normal.xyz;

	// Transform vertex from object to unhomogenized Normalized Device
	// Coordinates (NDC).
	gl_Position = Projection * actualModelView * vec4(in_Position.xyz, 1);

	// Calculate the position of the vertex in camera coordinates:
	out_CamCoord = vec3(actualModelView * vec4(in_Position.xyz, 1));
}
