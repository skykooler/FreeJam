#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <SOIL.h>
#include <assert.h>
#include "png.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PNG_SIG_BYTES 8
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;

class silence {
	int n;
	
	silence(int n);
	void play();
	bool is_silence() { return true; };
	int length() { return n; }

};
void silence::play() {
	
}
float angle = 0.0f;
float red = 1.0f;
float green = 1.0f;
float blue = 1.0f;
int width = 1024;
int height = 600;


char * load_png(char *name, int *width, int *height) {
	FILE *png_file = fopen(name, "rb");
	assert(png_file);

	uint8_t header[PNG_SIG_BYTES];

	fread(header, 1, PNG_SIG_BYTES, png_file);
	assert(!png_sig_cmp(header, 0, PNG_SIG_BYTES));

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(png_ptr);

	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);

	png_infop end_info = png_create_info_struct(png_ptr);
	assert(end_info);

	assert(!setjmp(png_jmpbuf(png_ptr)));
	png_init_io(png_ptr, png_file);
	png_set_sig_bytes(png_ptr, PNG_SIG_BYTES);
	png_read_info(png_ptr, info_ptr);

	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);

	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
			
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	if (bit_depth == 16)
			png_set_strip_16(png_ptr);
			
	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	else
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 numbytes = rowbytes*(*height);
	png_byte* pixels = (png_byte*)malloc(numbytes);
	png_byte** row_ptrs = (png_byte**)malloc((*height) * sizeof(png_byte*));

	int i;
	for (i=0; i<(*height); i++)
	  row_ptrs[i] = pixels + ((*height) - 1 - i)*rowbytes;

	png_read_image(png_ptr, row_ptrs);

	free(row_ptrs);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(png_file);

	return (char *)pixels;
}

static GLuint load_img(const char *filename) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	int w, h;
	GLubyte *pixels = (GLubyte *)load_png((char*)filename, &w, &h);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
	
	return tex;
}

GLuint stageimg = load_img("resources/image-textures/floor.png");


void draw_img(float x, float y, float w, float h, GLuint tex) {
	glColor3f(0.0,0.0,0.5);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f( x, y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(w+x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f( w+x, h+y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f( x, h+y);
	glEnd();
}

void draw_rect(float x, float y, float w, float h, float r, float g, float b) {
	glColor3f(r,g,b);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f( x, y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(w+x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f( w+x, h+y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f( x, h+y);
	glEnd();
}

void draw(void) {

	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	//gluLookAt(	0.0f, 0.0f, 10.0f,
	//			0.0f, 0.0f,  0.0f,
	//			0.0f, 1.0f,  0.0f);
				
	gluOrtho2D(0, 1024, 0, 600);
	glMatrixMode(GL_MODELVIEW);
	
	draw_rect(200,200,50,100,red,green,blue);

	draw_img(100.0f,100.0f,100.0f,100.0f, stageimg);

	angle+=0.1f;

	glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {

	if (key == 27)
		exit(0);
}

void processSpecialKeys(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_F1 :
				red = 1.0;
				green = 0.0;
				blue = 0.0; break;
		case GLUT_KEY_F2 :
				red = 0.0;
				green = 1.0;
				blue = 0.0; break;
		case GLUT_KEY_F3 :
				red = 0.0;
				green = 0.0;
				blue = 1.0; break;
	}
	glutPostRedisplay();
}

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;
	//float ratio = 1.0* w / h;
	width = w;
	height = h;

	/* Skip this for now 
	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

        // Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,1,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW); */
	glutPostRedisplay();
}

int main(int argc, char **argv) {

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(1024, 600);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("FreeJam - Experimental C++ Backend");
	
	// register callbacks
	glutDisplayFunc(draw);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(draw);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;

}
