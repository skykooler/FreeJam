#pragma once

#include "ofMain.h"
#include <algorithm>
#include <vector>
#include <map>
#include <math.h>
#include <string>

#ifndef NOTE_ON
#define NOTE_ON 1
#endif
#ifndef NOTE_OFF
#define NOTE_OFF 255
#endif

using namespace std;


class NotePlayer {
	public:
		NotePlayer ();
		void set_continuous(bool cont);
		void set_sound(string path);
		void play_note(int noteval, float volume);
		void stop_note(int noteval);
		bool continuous;
		bool fadein;
	private:
		vector<ofSoundPlayer> notes;
};

class Subtrack {
	public:
		Subtrack ();
		int size();
		void truncate(int length);
		bool play(unsigned int index, NotePlayer * player);
		void draw(int track_index, int time_index, float TOP_HEIGHT);
		void add_data();
		void set_data(int index, int noteval, int value);
	private:
		vector<vector<int> > data; // Inside one is 176 ints long
};

class LiveSubtrack : public Subtrack {
	public:
		LiveSubtrack();
};

class Track {
public:
	Track ();
	void add(Subtrack subtrack);
	bool play(int INDEX);
	void set_text(string text);
	void play_note(int noteval, float volume);
	void stop_note(int noteval);
	void draw (int num);
	// ofImage * img();

	vector<Subtrack> tracks;
	vector<float> trackslen;
	int tracksindex;
	bool playing;
	string label;
	// Subtrack * currenttrack;
	int currenttrack;
	ofImage int_img;
private:
	NotePlayer player;

};

class LiveTrack : public Track {
public:
	LiveTrack();
	// bool play(int INDEX);
	// void play_note(int noteval, float volume);
	// void stop_note(int noteval);
};

// extern 
extern Track * ACTIVETRACK;
extern const float NOTE_RATIOS[12];
extern bool recording;
extern bool playing;
extern unsigned int INDEX;	// we can't have sound before the start of a song anyway, so this is unsigned
extern float SCROLL;
extern float VIEW_SCALE;
extern int STEP;
extern float SIDE_WIDTH;

extern	ofImage trackbg_img;
extern	ofImage tsa_img;
extern	ofImage tra_img;
extern	ofImage tla_img;
extern	ofImage toa_img;
extern	ofImage tg_img;

extern	ofImage tbg_s_img;
extern	ofImage tbg_s_l_img;
extern	ofImage tbg_s_r_img;
extern	ofImage tbg_r_img;
extern	ofImage tbg_r_l_img;
extern	ofImage tbg_r_r_img;
extern	ofImage tbg_l_img;
extern	ofImage tbg_l_l_img;
extern	ofImage tbg_l_r_img;
extern	ofImage tbg_o_img;
extern	ofImage tbg_o_l_img;
extern	ofImage tbg_o_r_img;

extern float vbarloc;
extern ofTrueTypeFont font;
extern ofImage barimg;

		
extern float note(float val);

class FreeJamApp: public ofBaseApp{

	public:
		void setPlayMode(int mode);
		void setup();
		void update();
		void draw();
		void draw_track(Track * track, int num, float width);
		void play();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		
		ofSoundPlayer  beats;
		ofSoundPlayer  synth;
		ofSoundPlayer  vocals;

		float 			synthPosition;

		ofImage stageimg;
		ofImage record_img;
		ofImage recording_img;
		ofImage rw_beg_img;
		ofImage rw_img;
		ofImage play_img;
		ofImage pause_img;
		ofImage ff_img;
		ofImage ff_end_img;
		ofImage slider_img;

		ofImage key_cf;
		ofImage key_dga;
		ofImage key_eb;
		ofImage key_black;
		ofImage key_cf_pressed;
		ofImage key_dga_pressed;
		ofImage key_eb_pressed;
		ofImage key_black_pressed;

		int				playMode;
		float			keyscalex;
		float			keyscaley;
		float			keyscalexb;
		float			keyscaleyb;
		bool			dragging_vbar;
		float			vbar_offset_y;

		// vector<int> KEYPRESS_MASK;
		map<int, bool> KEYPRESS_MASK;

		std::vector<Track *> TRACKS;
		
};

enum {
	STAGE,
	KEYBOARD
};