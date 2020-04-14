#pragma once

#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Box.H>
#include<FL/Fl_Button.H>
#include<FL/Fl_Output.H>
#include<FL/FL_Text_Display.H>
#include<FL/Fl_Help_View.H>
#include<FL/Fl_PNG_Image.H>
#include<FL/Fl_Shared_Image.H>
#include<FL/fl_draw.H>
#include"MediaProcessor.hpp"
#include"ffmpegUtil.h"

struct PlayUtil {
	Fl_Window* window;
	VideoProcessor* videoProcessor;
	AudioProcessor* audioProcessor;
};

class LivePage2 : public Fl_Group {
public:
	LivePage2(Fl_Window* window);
	//void playRGB(void*);
//	void playAudio(void*);
private:
	static void switchPage(Fl_Button* btn, PlayUtil*);
	Fl_Button* btn1;
	Fl_Box* video;
	Fl_Group* info;
	Fl_Box* roomId;
	Fl_Box* roomUser;
	Fl_Image* image;
	Fl_Text_Display* box_cmt_image;
	Fl_Text_Buffer* text_buffer;

	ffmpegUtil::PacketGrabber packetGrabber{ "C:/Users/mwcxy/Downloads/ccb.mp4" };
	VideoProcessor* videoProcessor;
	AudioProcessor* audioProcessor;
	//TestMedia media;
};

