#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
//#include <SOIL.h>
#include <assert.h>
#include <math.h>
#include "png.h"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

#define len(a) ( sizeof ( a ) / sizeof ( *a ) )

#define PNG_SIG_BYTES 8
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;

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

// Here are the fonts: 
GLvoid * glutFonts[7] = { 
    GLUT_BITMAP_9_BY_15, 
    GLUT_BITMAP_8_BY_13, 
    GLUT_BITMAP_TIMES_ROMAN_10, 
    GLUT_BITMAP_TIMES_ROMAN_24, 
    GLUT_BITMAP_HELVETICA_10, 
    GLUT_BITMAP_HELVETICA_12, 
    GLUT_BITMAP_HELVETICA_18 
}; 

// Here is the function 
void glutPrint(float x, float y, GLvoid *font, char* text, float r, float g, float b, float a) { 
    if(!text || !strlen(text)) return; 
    bool blending = false; 
    if(glIsEnabled(GL_BLEND)) blending = true; 
    glEnable(GL_BLEND); 
    glColor4f(r,g,b,a); 
	glDisable(GL_TEXTURE_2D);
    glRasterPos2f(x,y); 
    while (*text) { 
        glutBitmapCharacter(font, *text); 
        text++; 
    } 
    if(!blending) glDisable(GL_BLEND); 
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

Img key_cf_img;
Img key_dga_img;
Img key_eb_img;
Img key_black_img;
Img key_cf_pressed_img;
Img key_dga_pressed_img;
Img key_eb_pressed_img;
Img key_black_pressed_img;

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
int KEYMAP [512];
bool playing = false;
bool recording = false;
float pitchbend = 1.0;
int SIDE_WIDTH = 150;
int lastx = 0;
int lasty = 0;

float note(int val) {
	val-=36;
	if (0<=val and val<12) {
		return NOTE_RATIOS[val];
	} else {
		return (pow(2,(int)(val)/12))*NOTE_RATIOS[val%12];
	}
}

class Label {
	public:
		float x;
		float y;
		char * text;
		GLvoid *font;
		int font_size;
		float r;
		float g;
		float b;
		float a;
		void draw () {
			glutPrint(x, y, glutFonts[5], text, r, g, b, a);
		}
		Label();
};
Label::Label(){
	font = GLUT_BITMAP_TIMES_ROMAN_24;
	font_size = 12;
	r = 1.0;
	g = 1.0;
	b = 1.0;
	a = 0.5;
	text = (char *)"Mana oh mana";
}

class NotePlayer {
	public:
		void play_note(int noteval,float volume) {
			/*self.notes[noteval].stop()
			self.notes[noteval].play()
			self.acnotes[noteval] = true
			if (self.fadein) {
				fadein_note(None,self.notes,noteval,self.acnotes,self.fadein,velocity)
			} else {
				self.notes[noteval].volume = velocity
			}*/
		};
		void stop_note(int noteval) {
			
		};
};

class Silence {
	int n;
	
	Silence(int n);
	void play();
	bool is_silence() { return true; };
	int length() { return n; }

};
void Silence::play() {
	
}

class Subtrack {
	public:
		//vector<vector<int[3]> > data;
		vector<vector<int> > data;
		bool is_silence() { return false; };
		int __len__() {return data.size();/*return 30;*/};
		void play(int INDEX, NotePlayer * player) {
			(*player).play_note(35,1.0);
			if (recording) {
				vector<int> datum(3);
				//data[data.size()-1].push_back(datum);
				data[data.size()].resize(data[data.size()].size()+1);
				data[data.size()].at(0) = 35;
				data[data.size()].at(1) = 255;
				data[data.size()].at(2) = 0;
				//self.recnotes[noteval]=[data[-1],len(data)]
			}
			/*if self.data[index]:
				for [i,j,k] in self.data[index]:
					#player = pyglet.media.ManagedSoundPlayer()
					#player.queue(c)
					#player.pitch = note(i)
					#player.play()
					self.track.notes[i].stop()
					self.track.notes[i].play()
					self.track.acnotes[i] = True
					if self.track.fadein:
						fadein_note(None,self.track.notes,i,self.track.acnotes,self.track.fadein,j)
					else:
						self.track.notes[i].volume = j
					pyglet.clock.schedule_once(self.stop_note,k,i)
			return (index==len(self.data)-1)*/
		}
};

/*Multiline comment for code folding to preserve line #s with Python

















*/
class Track {
	private:
		Img * int_img;
		char * name;
	public:
		Img * cimg;
		Img * leftimg;
		Img * rightimg;
		Label label;
		Label labelshadow;
		Track();
		NotePlayer player;
		vector<Subtrack> tracks;
		vector<int> trackslen;
		Img img() {
			return *int_img;
		};
		void play_note(int noteval,float volume) {
			player.play_note(noteval, volume);
		};
		void stop_note(int noteval) {
			player.stop_note(noteval);
		};
		void add(Subtrack subtrack) {
			tracks.push_back(subtrack);
			trackslen.push_back(subtrack.__len__());
		};
		void play(int INDEX) {
			int b = 0;
			for (uint16_t i=0; i<tracks.size(); i++) {
				if (INDEX-b<=tracks[i].__len__()) {
					tracks[i].play(INDEX-b, &player);
				} else {
					b+=tracks[i].__len__();
				}
			}
		};
};
Track::Track () {
  int_img = &tsa_img;
  cimg = &tbg_s_img;
  leftimg = &tbg_s_l_img;
  rightimg = &tbg_s_r_img;
  label.r = 1.0;
  label.g = 1.0;
  label.b = 1.0;
  label.a = 1.0;
  labelshadow.r = 0.0;
  labelshadow.g = 0.0;
  labelshadow.b = 0.0;
  labelshadow.a = 0.5;
  name = (char *)"Untitled Track";
  label.text = name;
  labelshadow.text = name;
}

/*









































*/
vector<Track> TRACKS;
Track * ACTIVETRACK;

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
	
	key_cf_img = load_img("resources/image-textures/key_cf.png");
	key_dga_img = load_img("resources/image-textures/key_dga.png");
	key_eb_img = load_img("resources/image-textures/key_eb.png");
	key_black_img = load_img("resources/image-textures/key_black.png");
	key_cf_pressed_img = load_img("resources/image-textures/key_cf_pressed.png");
	key_dga_pressed_img = load_img("resources/image-textures/key_dga_pressed.png");
	key_eb_pressed_img = load_img("resources/image-textures/key_eb_pressed.png");
	key_black_pressed_img = load_img("resources/image-textures/key_black_pressed.png");

}

void init_keys() {
	KEYMAP['a']=36;
	KEYMAP['w']=37;
	KEYMAP['s']=38;
	KEYMAP['e']=39;
	KEYMAP['d']=40;
	KEYMAP['f']=41;
	KEYMAP['t']=42;
	KEYMAP['g']=43;
	KEYMAP['y']=44;
	KEYMAP['h']=45;
	KEYMAP['u']=46;
	KEYMAP['j']=47;
	KEYMAP['k']=48;
	KEYMAP['o']=49;
	KEYMAP['l']=50;
	KEYMAP['p']=51;
	KEYMAP[';']=52;
	KEYMAP['\'']=53;
}

/*
  
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 */
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
			(*track.leftimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+4);
			(*track.cimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+5,BASE_HEIGHT+4,i.__len__()*VIEW_SCALE-8);
			(*track.rightimg).blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+i.__len__()*VIEW_SCALE-4,BASE_HEIGHT+4);
			for (uint32_t j=0; j<i.data.size(); j++) {
				if (!(i.data[j].size()==0)){
					//I think it is safe to make this 8 bits. There's only 88 possible keys.
					for (uint8_t r=0; r<i.data[j].size(); r++) {
						int k = i.data[j][0];
						//int l = *i.data[j][1];
						int m = i.data[j][2];
						draw_rect((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+8+k%28,m*VIEW_SCALE,1,0.33,0.33,0.33);
					}
				}
			}
		}
		time_index+=i.__len__();
		track_index+=1;
	}
	track.img().blit(0,BASE_HEIGHT,SIDE_WIDTH,track.img().height);
	track.labelshadow.x = 15;
	track.labelshadow.y = (vbarloc-barimg.height)-(num*track.img().height+track.label.font_size)-2;
	track.label.x = 16;
	track.label.y = (vbarloc-barimg.height)-(num*track.img().height+track.label.font_size)-3;
	track.labelshadow.draw();
	track.label.draw();
}


void on_draw(void) {

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
		float w = stageimg.width*(max(width/stageimg.width, height/stageimg.height));
		float h = stageimg.height*(max(width/stageimg.width, height/stageimg.height));
		draw_img(0.0f,vbarloc,w,h,stageimg.tex);
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
	draw_rect(150,0,1,vbarloc-barimg.height,0.33,0.33,0.33);
	//rgb084084084.blit(150,0,height=vbarloc-barimg.height)
	draw_rect(151+(INDEX-SCROLL)*VIEW_SCALE,0,1,vbarloc-barimg.height,1.0,0,0);
	//rgb255000000.blit(151+(INDEX-SCROLL)*VIEW_SCALE,0,height=vbarloc-barimg.height)
	draw_rect(151,0,width-150,16,0.93,0.93,0.93);
	//rgb237237237.blit(151,0,width=width-150,height=16)
	draw_rect(155,8,width-159,1,0.33,0.33,0.33);
	//rgb084084084.blit(155,8,width=width-159)
	slider_img.blit(min(VIEW_SCALE*64+154,(float)(width-8)),4.0);
	/*------------------------------*/
	
	glutSwapBuffers();
}
/*

 
 












































*/
void on_mouse_press(int x, int y, int button, int modifiers) {
	float ch = vbarloc-(barimg.height/2+play_img.height/2);
	float cx = width/2;
	if (y>vbarloc and PLAYMODE==KEYBOARD){
		float keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.25);
		int noteval = max(min((int)(x/(key_cf_img.width*keyscale)*12/7.0)+36,87),1);
		if (y<vbarloc+256*keyscale) {
			int rem = noteval%12;
			if (rem==1 || rem==3 || rem==6 || rem==8 || rem==10){
				noteval-=(int)((int)((x+31)/(key_cf_img.width*keyscale)*12/7.0)+36==noteval)*2-1;
			}
		}
		KEYPRESS_MASK[noteval]=true;
		(*ACTIVETRACK).play_note(noteval,1); //remember, this is a pointer
		return;
	}
	if (max(cx-100,(float)width/3)<x and x<max(cx-100,(float)width/3)+record_img.width 
		and vbarloc-(barimg.height/2+record_img.height/2)<y and y<vbarloc-(barimg.height/2)+record_img.height/2){
		if (recording){
			recording = false;
		} else {
			if (!playing) {
				playing = true;
			}
			recording = true;
			(*ACTIVETRACK).add(Subtrack());
		}
		return;
	}
	if (cx<x and x<cx+rw_beg_img.width and ch<y and y<ch+rw_beg_img.height){
		INDEX = 0;
		return;
	}
	cx+=rw_beg_img.width;
	if (cx<x and x<cx+rw_img.width and ch<y and y<ch+rw_img.height){
		INDEX-=1;
		INDEX = INDEX-INDEX%STEP;
		return;
	}
	cx+=rw_img.width;
	if (playing) {
		if (cx<x and x<cx+pause_img.width and ch<y and y<ch+pause_img.height){
			playing = false;
			recording = false;
			return;
		}
		cx+=pause_img.width;
	} else {
		if (cx<x and x<cx+play_img.width and ch<y and y<ch+play_img.height){
			playing = true;
			return;
		}
		cx+=play_img.width;
	}
	if (cx<x and x<cx+ff_img.width and ch<y and y<ch+ff_img.height){
		INDEX = INDEX-INDEX%STEP+STEP;
		return;
	}
	cx+=ff_img.width;
	if (cx<x and x<cx+ff_end_img.width and ch<y and y<ch+ff_end_img.height){
		//TODO: define this
		//fast_forward_end();
		return;
	}
	if (VIEW_SCALE*64+154<x and x<VIEW_SCALE*64+162 and 4<y and y<12){
		dragging_scale = true;
		return;
	}
	if (vbarloc-barimg.height<y and y<vbarloc) {
		dragging_vbar = true;
	} else {
		//
	}
}
/*


*/
void on_mouse_drag(int x, int y, int dx, int dy, int button, int modifiers) {
	if (dragging_vbar){
		vbarloc=max(min(vbarloc+dy,(float)height),(float)(barimg.height));
	} else if (dragging_scale){
		VIEW_SCALE = (x-154)/64.0;
	} else if (y>vbarloc){// and PLAYMODE==KEYBOARD){
		float keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.125);
		int noteval = max(min(int(x/(key_cf_img.width*keyscale)*12/7.0)+36,88),0);
		if (y<vbarloc+256*keyscale){
			int rem = noteval%12;
			if (rem==1 || rem==3 || rem==6 || rem==8 || rem==10){
				noteval-=(int)((int)((x+31)/(key_cf_img.width*keyscale)*12/7.0)+36==noteval)*2-1;
			}
		}
		if (!KEYPRESS_MASK[noteval]) {
			for (uint8_t i=0; i<len(KEYPRESS_MASK); i++){
				(*ACTIVETRACK).stop_note(i);
			}
			//clear the mask - can't figure out how to make this a function
			// (can you not pass a pointer to an array as an argument?)
			for (uint16_t i=0; i<len(KEYPRESS_MASK); i++){ KEYPRESS_MASK[i]=0; }
			KEYPRESS_MASK[noteval] = true;
			(*ACTIVETRACK).play_note(noteval,1);
		}
		return;
	}
}
void on_mouse_release(int x, int y, int button, int modifiers) {
	dragging_vbar = false;
	for (uint8_t i=0; i<len(KEYPRESS_MASK); i++) {
		(*ACTIVETRACK).stop_note(i);
	}
	for (uint16_t i=0; i<len(KEYPRESS_MASK); i++){ KEYPRESS_MASK[i]=0; }
	dragging_scale = false;
}
/*

*/
void on_key_press(int symbol, int modifiers) {
	if ((bool)KEYMAP[symbol]){
		int noteval = KEYMAP[symbol];
		if (not KEYPRESS_MASK[noteval]){
			KEYPRESS_MASK[noteval] = true;
		}
		(*ACTIVETRACK).play_note(noteval,1);
	}
}
void on_key_release(int symbol, int modifiers){
	if ((bool)KEYMAP[symbol]){
		if (KEYPRESS_MASK[KEYMAP[symbol]]) {
			KEYPRESS_MASK[KEYMAP[symbol]] = false;
			(*ACTIVETRACK).stop_note(KEYMAP[symbol]);
		}
	}
}

static void play(int dummy) {
	if (playing){
		for (uint8_t j=0; j<(TRACKS).size(); j++) {
			Track * i = &(TRACKS)[j];
			(*i).play(INDEX);
		}
		if (INDEX<256){	//TODO: calculate this value
			INDEX+=1;
		} else {
			playing = false;
			recording = false;
		}
		if (recording) {
			(*ACTIVETRACK).tracks[(*ACTIVETRACK).tracks.size()-1].data.resize((*ACTIVETRACK).tracks[(*ACTIVETRACK).tracks.size()-1].data.size()+1);
			//(*ACTIVETRACK).tracks[(*ACTIVETRACK).tracks.size()-1].data.resize(100);
		}
	}
	glutTimerFunc (1000/PLAYBACK_SPEED, play, 0);
};

void processNormalKeys(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0);
	} else {
		on_key_press(key, 0);
	}
}
void processNormalKeysUp(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0);
	} else {
		on_key_release(key, 0);
	}
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
void processSpecialKeysUp(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_F1 :
				red = 1.0;
				green = 0.0;
				blue = 0.0; break;
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

static void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		on_mouse_press(x, height-y, button, glutGetModifiers());
	} else if (state == GLUT_UP) {
		on_mouse_release(x, height-y, button, glutGetModifiers());
	}
	lastx = x;
	lasty = y;
	//draw();
}
static void drag(int x, int y) {
	//TODO: dynamically set the button if rught-click drags are possible
	on_mouse_drag(x, height-y, x-lastx, lasty-y, GLUT_LEFT_BUTTON, glutGetModifiers());
	lastx = x;
	lasty = y;
	//draw();
}

int main(int argc, char **argv) {

	// init tracks
	TRACKS.push_back(Track());
	ACTIVETRACK = &TRACKS[0];

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
	glutDisplayFunc(on_draw);
	glutReshapeFunc(changeSize);
	glutIdleFunc(on_draw);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutKeyboardUpFunc(processNormalKeysUp);
	glutSpecialUpFunc(processSpecialKeysUp);
	glutMotionFunc(&drag);
    glutMouseFunc(&mouse);

	glutTimerFunc (1000/PLAYBACK_SPEED, play, 0);
	//boost::thread playThread(boost::bind(play, &playing, &recording, 
	//							&TRACKS, &ACTIVETRACK, &INDEX));

	// enter GLUT event processing cycle
	glutMainLoop();
	//playThread.join();
	
	return 1;

}
