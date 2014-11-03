#version 130 // Specify which version of GLSL we are using.

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

/* GLSL in OpenGL 3.2 and higher provides a matrix inverse
   function that we can call. This code uses GLSL 3.0 and
   we must define our own matrix inversion function in
   order to call inverse(). For more information about this
   function, see the bottom of this file.
*/
mat3 inverse(mat3 m)
{
   mat3 adj;
   adj[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]);
   adj[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]);
   adj[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]);
   adj[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]);
   adj[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]);
   adj[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]);
   adj[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
   adj[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]);
   adj[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]);
   float det = (+ m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
   return adj / det;
}

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



/* inverse() information
 
   The inverse function in this file and other matrix inverse routines written in GLSL can be found at:
   https://chromium.googlesource.com/chromium/deps/mesa/+/f2ba7591b1407a7ee9209f842c50696914dc2ded/src/glsl/builtins/glsl/inverse.glsl

 * Licensing information for inverse():
 * Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
