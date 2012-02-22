#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
//#include <SOIL.h>
#include <assert.h>
#include "png.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

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


void draw_img(float x, float y, float w, float h, GLuint tex) {
	glColor3f(1.0,1.0,1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f( x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(w+x, y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f( w+x, h+y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f( x, h+y);
	glEnd();
}

void draw_rect(float x, float y, float w, float h, float r, float g, float b) {
	glColor3f(r,g,b);
	glDisable(GL_TEXTURE_2D);
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


class Img {
	public:
		GLuint tex;
		int width;
		int height;
		void blit(float x, float y) {
			draw_img(x,y,width,height,tex);
		}
		void blit(float x, float y, float w) {
			draw_img(x,y,w,height,tex);
		}
		void blit(float x, float y, float w, float h) {
			draw_img(x,y,w,h,tex);
		}
};
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

static Img load_img(const char *filename) {
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
	Img img;
	img.tex = tex;
	img.width = w;
	img.height = h;
	return img;
}

Img stageimg;
Img barimg;
Img record_img;
Img recording_img;
Img rw_beg_img;
Img rw_img;
Img play_img;
Img pause_img;
Img ff_img;
Img ff_end_img;

Img slider_img;

Img tsa_img;
Img tra_img;
Img tla_img;
Img toa_img;
Img tg_img;

Img tbg_s_img;
Img tbg_s_l_img;
Img tbg_s_r_img;
Img tbg_r_img;
Img tbg_r_l_img;
Img tbg_r_r_img;
Img tbg_l_img;
Img tbg_l_l_img;
Img tbg_l_r_img;
Img tbg_o_img;
Img tbg_o_l_img;
Img tbg_o_r_img;

Img trackbg_img;

float vbarloc = 100;
bool dragging_vbar = false;
bool dragging_scale = false;
int STEP = 64; // maximum res is 64th-notes
float VIEW_SCALE = 1.0;
int SCROLL = 0;
int INDEX = 0;
float PLAYBACK_SPEED = 16.0;
int STAGE = 57; // 'ST'
int KEYBOARD = 80; // 'BO' - can't really do 'K'!
int PLAYMODE = STAGE; // use STAGE or KEYBOARD
bool KEYPRESS_MASK [88] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float NOTE_RATIOS [] = {1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887};
int VIEW_EXTENTS [2];
/*KEYMAP = {key.A:36,key.W:37,key.S:38,key.E:39,key.D:40,key.F:41,key.T:42,
			key.G:43,key.Y:44,key.H:45,key.U:46,key.J:47,key.K:48,
			key.O:49,key.L:50,key.P:51,key.SEMICOLON:52,key.APOSTROPHE:53}*/
bool playing = false;
bool recording = false;
float pitchbend = 1.0;
int SIDE_WIDTH = 150;

class Subtrack {
	public:
		bool is_silence() { return true; };
		int __len__() {/*return data.size()*/return 30;};
};

class Track {
	private:
		Img * int_img;
	public:
		Img * cimg;
		Img * leftimg;
		Img * rightimg;
		Track();
		vector<Subtrack> tracks;
		Img img() {
			return *int_img;
		};
};
Track::Track () {
  int_img = &tsa_img;
  cimg = &tbg_s_img;
  leftimg = &tbg_s_l_img;
  rightimg = &tbg_s_r_img;
}

vector<Track> TRACKS;

void load_all_images() {
	stageimg = load_img("resources/image-textures/floor.png");
	barimg = load_img("resources/image-textures/bar.png");
	record_img = load_img("resources/image-textures/record_b.png");
	recording_img = load_img("resources/image-textures/record_active.png");
	rw_beg_img = load_img("resources/image-textures/rewind_to_beginning.png");
	rw_img = load_img("resources/image-textures/rewind.png");
	play_img = load_img("resources/image-textures/play.png");
	pause_img = load_img("resources/image-textures/pause.png");
	ff_img = load_img("resources/image-textures/fastforward.png");
	ff_end_img = load_img("resources/image-textures/forward_to_end.png");

	slider_img = load_img("resources/image-textures/slider.png");
	

	tsa_img = load_img("resources/image-textures/track_software_active.png");
	tra_img = load_img("resources/image-textures/track_real_active.png");
	tla_img = load_img("resources/image-textures/track_loop_active.png");
	toa_img = load_img("resources/image-textures/track_other_active.png");
	tg_img = load_img("resources/image-textures/track_generic.png");

	tbg_s_img = load_img("resources/image-textures/track_software_bg.png");
	tbg_s_l_img = load_img("resources/image-textures/track_software_bg_left.png");
	tbg_s_r_img = load_img("resources/image-textures/track_software_bg_right.png");
	tbg_r_img = load_img("resources/image-textures/track_real_bg.png");
	tbg_r_l_img = load_img("resources/image-textures/track_real_bg_left.png");
	tbg_r_r_img = load_img("resources/image-textures/track_real_bg_right.png");
	tbg_l_img = load_img("resources/image-textures/track_loop_bg.png");
	tbg_l_l_img = load_img("resources/image-textures/track_loop_bg_left.png");
	tbg_l_r_img = load_img("resources/image-textures/track_loop_bg_right.png");
	tbg_o_img = load_img("resources/image-textures/track_other_bg.png");
	tbg_o_l_img = load_img("resources/image-textures/track_other_bg_left.png");
	tbg_o_r_img = load_img("resources/image-textures/track_other_bg_right.png");

	trackbg_img = load_img("resources/image-textures/trackbg.png");


}

void draw_track(Track track, int num, int width) {
	float BASE_HEIGHT=(vbarloc-barimg.height)-((num+1)*track.img().height);
	trackbg_img.blit(0,BASE_HEIGHT,width);
	for (int i=VIEW_EXTENTS[0]; i<VIEW_EXTENTS[1]; i++){
		if (i%STEP==0){
			draw_rect((i-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+4,1,tsa_img.height-4,0.33,0.33,0.33);
		}
	};
	int track_index;
	track_index = 0;
	int time_index;
	time_index = 0;
	for (uint16_t q=0; q<track.tracks.size(); q++) {
		Subtrack i = track.tracks[q];
		if (!i.is_silence()){
			//pass
			//print time_index
			(*track.leftimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+4);
			(*track.cimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+5,BASE_HEIGHT+4,i.__len__()*VIEW_SCALE-8);
			(*track.rightimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+i.__len__()*VIEW_SCALE-4,BASE_HEIGHT+4);
			/*for j in xrange(len(i.data)):
				if i.data[j]:
					for [k,l,m] in i.data[j]:
						rgb084084084.blit((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+8+k%28,width=m*VIEW_SCALE)
					}
				}
			}*/
		}
	}
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
				
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
	
	VIEW_EXTENTS[0] = 0;
	VIEW_EXTENTS[1] = width; //for now, at least
	if (PLAYMODE==STAGE) {
		draw_img(0.0f,vbarloc,width,height-vbarloc, stageimg.tex);
	}
	draw_img(0,vbarloc-barimg.height,width,barimg.height,barimg.tex);
	float controlheight = vbarloc-(barimg.height/2+play_img.height/2);
	float controlsx = width/2;
	if (recording){
		recording_img.blit(max(controlsx-100,width/3.0f),
					vbarloc-(barimg.height/2.0f+record_img.height/2.0f));
	} else {
		record_img.blit(max(controlsx-100,width/3.0f),
					vbarloc-(barimg.height/2.0f+record_img.height/2.0f));
	}
	rw_beg_img.blit(controlsx,controlheight);
	controlsx+=rw_beg_img.width;
	rw_img.blit(controlsx,controlheight);
	controlsx+=rw_img.width;
	if (playing){
		pause_img.blit(controlsx,controlheight);
		controlsx+=pause_img.width;
	} else {
		play_img.blit(controlsx,controlheight);
		controlsx+=play_img.width;
	}
	ff_img.blit(controlsx,controlheight);
	controlsx+=ff_img.width;
	ff_end_img.blit(controlsx,controlheight);
	/*------------------------------*/
	
	/*---------- Tracks ------------*/
	draw_rect(0,0,width,vbarloc-barimg.height,0.75f,0.75f,0.75f);
	//rgb184184184.blit(0,0,width=width,height=vbarloc-barimg.height)
	for (uint32_t i=0; i<TRACKS.size();i++){
		draw_track(TRACKS[i],i,width);
	}
	//rgb084084084.blit(150,0,height=vbarloc-barimg.height)
	//rgb255000000.blit(151+(INDEX-SCROLL)*VIEW_SCALE,0,height=vbarloc-barimg.height)
	//rgb237237237.blit(151,0,width=width-150,height=16)
	//rgb084084084.blit(155,8,width=width-159)
	//slider_img.blit(min(VIEW_SCALE*64+154,width-8),4)
	/*------------------------------*/
	
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

	// Set the viewport to be the entire window*/
	glViewport(0, 0, w, h);

	/*// Set the correct perspective.
	gluPerspective(45,ratio,1,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW); 
	glutPostRedisplay();*/
}

int main(int argc, char **argv) {

	// init tracks
	TRACKS.push_back(Track());

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(1024, 600);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("FreeJam - Experimental C++ Backend");
	load_all_images();
	glEnable( GL_TEXTURE_2D );
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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
