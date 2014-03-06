#include "FreeJamApp.h"


const float NOTE_RATIOS[12] = {1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887};
bool recording = false;
int INDEX = 0;

NotePlayer::NotePlayer(bool cont) {
	continuous = cont;
	for (int i=0; i<88; i++) {
		ofSoundPlayer player;
		notes.push_back(&player);
		// self.notes.append(audio.open_file('instruments/strings/basic strings/c.wav'))
		player.setSpeed(note(i));
		player.setLoop(continuous);
	}
}
void NotePlayer::play_note(int noteval, float volume) {
	(*notes.at(noteval)).stop();
	(*notes.at(noteval)).play();
	// if (fadein) {
	// 	fadein_note(None,self.notes,noteval,self.fadein,volume)
	// } else {
		(*notes.at(noteval)).setVolume(volume);
	// }
}
void NotePlayer::stop_note(int noteval){
	// if (cutoff) {
	// 	cutoff_note(None,self.notes,noteval,self.cutoff)
	// }
}

int Subtrack::size() {
	return data.size();
}
bool Subtrack::play(int index, NotePlayer * player) {
	for (int i=0; i<88; i++) {
		if ( data.at(index)[i] ) {
			float volume = data.at(index)[i+88];
			(*player).play_note(i,volume);
		}
	}
	return (index==data.size()-1);
}

void Track::init() {
	trackslen[0]=0; // List of lengths of all subtracks
	tracksindex=0;
	
	playing=true;
	
	// self.definition = xml_to_dict(etree.parse('instruments/strings/basic strings/instrument.xml').getroot())['instrument']
	// try:
	// 	self.player = NotePlayer(True if self.definition['continuous'].lower()=='yes' else False)
	// except:
		player = new NotePlayer(false);
	// try:
	// 	self.player.cutoff = int(self.definition['cutoff'])
	// except:
	// 	self.player.cutoff = 100
	// try:
	// 	if int(self.definition['fadein']):
	// 		self.player.set_fadein()
	// except:
	// 	pass # fadein is already False
	// try:
	// 	self.name = self.definition['name']
	// except:
	// 	self.name = 'Untitled instrument'
	// try:
	// 	self.player.pitchbend = True if self.definition['pitchbend'].lower()=='yes' else False
	// except:
	// 	self.player.pitchbend = False
	label = "";
	// self.int_img = tsa_img
	// self.cimg = tbg_s_img
	// self.leftimg = tbg_s_l_img
	// self.rightimg = tbg_s_r_img
}
		
// void Track::img() {
// 	return this==ACTIVETRACK ? int_img : tg_img;
// }
void Track::add(Subtrack subtrack) {
	tracks.push_back(subtrack);
	trackslen.push_back(subtrack.size());
	currenttrack = &subtrack;
}
bool Track::play(int INDEX) {
	int b = 0;
	// for i in self.tracks:
	for(std::vector<Subtrack>::iterator it = tracks.begin(); it != tracks.end(); ++it) {
		if (INDEX-b<((*it).size())) {
			return (*it).play(INDEX-b,player);
		} else {
			b+=(*it).size();
		}
	}
}
void Track::set_text(string text) {
	label = text;
}
void Track::play_note(int noteval, float volume) {
	if (recording) {
		(*currenttrack).data.at(INDEX)[noteval] = true;
		(*currenttrack).data.at(INDEX)[noteval+88] = floor(volume*255);
	}
	(*player).play_note(noteval,volume);
}
void Track::stop_note(int noteval){
	if (recording) {
		(*currenttrack).data.at(INDEX)[noteval] = 255;
	}
}

float note(float val){
	val-=36;
	if (0<=val and val<12) {
		return NOTE_RATIOS[(int)(floor(val))];
	} else {
		return (pow(2,(floor(val/12)))) * NOTE_RATIOS[(int)(floor(val))%12];
	}
}

//--------------------------------------------------------------
void FreeJamApp::setup(){
	synth.loadSound("sounds/synth.wav");
	beats.loadSound("sounds/1085.mp3");
	//vocals.loadSound("sounds/Violet.mp3");
	vocals.loadSound("sounds/c.wav");
	synth.setVolume(0.75f);
	beats.setVolume(0.75f);
	vocals.setVolume(0.5f);
	font.loadFont("Sudbury_Basin_3D.ttf", 32);
	beats.setMultiPlay(false);
	vocals.setMultiPlay(true);
	playMode = KEYBOARD;
	// playMode = STAGE;
	vbarloc = 256;
	// vbarloc = 512;
	dragging_vbar = false;
	vbar_offset_y = 0;

	//----------------------------------------------------------
	stageimg.loadImage("resources/image-textures/floor.png");
	barimg.loadImage("resources/image-textures/bar.png");
	key_cf.loadImage("resources/image-textures/key_cf.png");
	key_dga.loadImage("resources/image-textures/key_dga.png");
	key_eb.loadImage("resources/image-textures/key_eb.png");
	key_black.loadImage("resources/image-textures/key_black.png");
	key_cf_pressed.loadImage("resources/image-textures/key_cf_pressed.png");
	key_dga_pressed.loadImage("resources/image-textures/key_dga_pressed.png");
	key_eb_pressed.loadImage("resources/image-textures/key_eb_pressed.png");
	key_black_pressed.loadImage("resources/image-textures/key_black_pressed.png");
}

//--------------------------------------------------------------
void FreeJamApp::update(){

	ofBackground(255,255,255);

	// update the sound playing system:
	ofSoundUpdate();

}

//--------------------------------------------------------------
void FreeJamApp::draw(){
	keyscaley = max((ofGetHeight()-vbarloc),128.0f);
	keyscalex = keyscaley*key_cf.width/key_cf.height;
	keyscalexb = 0.5*keyscalex;
	keyscaleyb = 0.5*keyscaley;
	float y = min(0.0f,ofGetHeight()-(vbarloc+128));

	//----------------------------------------------------------
	// draw the background colors:
	float widthDiv = ofGetWidth() / 3.0f;
	ofSetHexColor(0xeeeeee);
	ofRect(0,0,widthDiv,ofGetHeight());
	ofSetHexColor(0xffffff);
	ofRect(widthDiv,0,widthDiv,ofGetHeight());
	ofSetHexColor(0xdddddd);
	ofRect(widthDiv*2,0,widthDiv,ofGetHeight());


	//---------------------------------- synth:
	if (synth.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("synth !!", 50,50);

	ofSetHexColor(0x000000);
	string tempStr = "click to play\npct done: "+ofToString(synth.getPosition())+"\nspeed: " + ofToString(synth.getSpeed()) + "\npan: " + ofToString(synth.getPan()) ;
	ofDrawBitmapString(tempStr, 50,ofGetHeight()-50);



	//---------------------------------- beats:
	if (beats.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("beats !!", widthDiv+50,50);

	ofSetHexColor(0x000000);
	tempStr = "click and drag\npct done: "+ofToString(beats.getPosition())+"\nspeed: " +ofToString(beats.getSpeed());
	ofDrawBitmapString(tempStr, widthDiv+50,ofGetHeight()-50);

	//---------------------------------- vocals:
	if (vocals.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("vocals !!", widthDiv*2+50,50);

	ofSetHexColor(0x000000);
	tempStr = "click to play (multiplay)\npct done: "+ofToString(vocals.getPosition())+"\nspeed: " + ofToString(vocals.getSpeed());	
	ofDrawBitmapString(tempStr, widthDiv*2+50,ofGetHeight()-50);

	//-------------------------------------------
	ofSetColor(255);
	//ofEnableAlphaBlending();
	if (playMode==KEYBOARD) {
		//int index = 0;
		// while (index*keyscalex<ofGetWidth()) {
		for (int index=0; index<ofGetWidth()/(keyscalex);) {
			key_cf.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_eb.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_cf.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_eb.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
		}
		// int index = 0;
		// while (index*keyscalexb<ofGetWidth()) {
		for (int index=0; index<ofGetWidth()/(keyscalex);) {
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			index+=1;
		}
		map<int,bool>::iterator iter;
		for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
			if (iter->second) {
				switch (iter->first%12) {
					case 1:
					case 3:
					case 6:
					case 8:
					case 10:
						key_black_pressed.draw((floor((iter->first-41)*7/12.0+3)*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
						break;
					case 0:
					case 5:
						key_cf_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
					case 2:
					case 7:
					case 9:
						key_dga_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
					default:
						key_eb_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
				}
			}
		}
	} else {
		float stagescale = max(ofGetWidth()/(stageimg.width), ofGetHeight()/(stageimg.height));
		stageimg.draw(0,ofGetHeight()-(vbarloc+stagescale*stageimg.height),stagescale*stageimg.width,stagescale*stageimg.height);
	}
	//---------------------------------------------- Control Bar
	barimg.draw(0,ofGetHeight()-vbarloc,ofGetWidth(),barimg.height);

}

//--------------------------------------------------------------
void FreeJamApp::keyPressed  (int key){
}

//--------------------------------------------------------------
void FreeJamApp::keyReleased(int key){

}

//--------------------------------------------------------------
void FreeJamApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void FreeJamApp::mouseDragged(int x, int y, int button){
	// continuously control the speed of the beat sample via drag,
	// when in the "beat" region:
	if (dragging_vbar) {
		// vbarloc=ofGetHeight()-max(min(vbar_offset_y+y,(float)ofGetHeight()),(float)barimg.height);
		vbarloc=max(barimg.height,ofGetHeight()-max(y+(int)vbar_offset_y,0));
		printf("Vbarloc: %f\n", vbarloc);
	// } else if {
	} else if (y<(ofGetHeight()-vbarloc) and playMode==KEYBOARD){
		int noteval = floor(x/(keyscalex)*12/7.0)+36;
		if (y>keyscaleyb) {
			if (noteval%12==1 or noteval%12==3 or noteval%12==6 or noteval%12==8 or noteval%12==10){
				noteval-=floor((float)(floor((x+31)/(keyscalex)*12/7.0)+36==noteval))*2-1;
			}
		}
		map<int,bool>::iterator iter = KEYPRESS_MASK.find(noteval);
		if (iter==KEYPRESS_MASK.end() or iter->second==false) {
			KEYPRESS_MASK[noteval] = true;
			// if not noteval in KEYPRESS_MASK:
			// 	KEYPRESS_MASK.append(noteval)

			// ACTIVETRACK.play_note(noteval,1)
			vocals.play();
			vocals.setSpeed(note(noteval));
		}
		for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
			if (iter->first!=noteval) {
				iter->second = false;
			}
		}
		return;
	}
}

//--------------------------------------------------------------
void FreeJamApp::mousePressed(int x, int y, int button){
	/*float widthStep = ofGetWidth() / 3.0f;
	if (x < widthStep){
		synth.play();
		synth.setSpeed( 0.1f + ((float)(ofGetHeight() - y) / (float)ofGetHeight())*10);
		synth.setPan(ofMap(x, 0, widthStep, -1, 1, true));
	} else if (x >= widthStep && x < widthStep*2){
		beats.play();
	} else {
		vocals.play();
		vocals.setSpeed( 0.1f + ((float)(ofGetHeight() - y) / (float)ofGetHeight())*3);
		//map x within the last third of window to -1 to 1 ( for left / right panning )
		vocals.setPan( ofMap(x, widthStep*2, widthStep*3, -1, 1, true) );
	}*/

	if (y<(ofGetHeight()-vbarloc) and playMode==KEYBOARD){
		//keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.25)
		int noteval = floor(x/(keyscalex)*12/7.0)+36;
		if (y>keyscaleyb) {
			if (noteval%12==1 or noteval%12==3 or noteval%12==6 or noteval%12==8 or noteval%12==10){
				noteval-=floor((float)(floor((x+31)/(keyscalex)*12/7.0)+36==noteval))*2-1;
			}
		}
		KEYPRESS_MASK[noteval] = true;
		// if not noteval in KEYPRESS_MASK:
		// 	KEYPRESS_MASK.append(noteval)

		// ACTIVETRACK.play_note(noteval,1)
		vocals.play();
		vocals.setSpeed(note(noteval));
		return;
	} 

	if (ofGetHeight()-vbarloc<y and y<ofGetHeight()+barimg.height-vbarloc) {
		dragging_vbar = true;
		vbar_offset_y = max(barimg.height,ofGetHeight()-max(y,0))-vbarloc;
	}
}

//--------------------------------------------------------------
void FreeJamApp::mouseReleased(int x, int y, int button){
	dragging_vbar = false;
	map<int,bool>::iterator iter;
	for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
		// ACTIVETRACK.stop_note(iter->first);
		iter->second = false;
	}
	vocals.stop();
	// KEYPRESS_MASK = []
	// dragging_scale = false;

}

//--------------------------------------------------------------
void FreeJamApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void FreeJamApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void FreeJamApp::dragEvent(ofDragInfo dragInfo){

}