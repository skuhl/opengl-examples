#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;
in vec2 out_TexCoord;


uniform sampler2D tex;

void main() 
{
		float k1 = .7;
		float aspectRatio = 1.27;
		float texScale = 0.283;

		float pixel_size_x = 85.6;
		float pixel_size_y = 82.4;

		int x_offset_pixel = 8;
		int y_offset_pixel = -32;

		float frame_edge_left = 334;
		float frame_edge_right = 848;
		float frame_edge_top = 526;
		float frame_edge_bottom = 938;

		// x:1182, y:1464
		if (gl_FragCoord.x < frame_edge_left || gl_FragCoord.y < frame_edge_top || gl_FragCoord.x > frame_edge_right || gl_FragCoord.y > frame_edge_bottom ) {

			float x_coord = floor((gl_FragCoord.x+x_offset_pixel)/pixel_size_x);
			float y_coord = floor((gl_FragCoord.y+y_offset_pixel)/pixel_size_y);

			float x_tex_coord;
			float y_tex_coord;

			vec4 tempColor = vec4(0,0,0,0);
			
			int sample_count = 0;

			for (int i=0; i<pixel_size_x/15; i++){
				for (int j=0; j<pixel_size_y/15; j++){
					x_tex_coord = x_coord * pixel_size_x + 10*i+5 - x_offset_pixel;
					y_tex_coord = y_coord * pixel_size_y + 10*j+5 - y_offset_pixel;
					tempColor = tempColor + texture(tex, vec2(x_tex_coord/1182.0, y_tex_coord/1464.0));
					sample_count++;
				}
			}
			fragColor = vec4(tempColor.x/sample_count, tempColor.y/sample_count, tempColor.z/sample_count, 1);
			//fragColor = vec4(0,0,0,1);
		}
		else{
			fragColor = texture(tex, out_TexCoord);
		}
}