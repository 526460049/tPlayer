// tPlayer.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include"LivePage2.h"


using namespace std;

int main(int argc, char** argv) {
	Fl_Window* window = new Fl_Window(640, 360);
	Fl_Group* video = new LivePage2(window);
	window->end();
	window->show(argc, argv);
	return Fl::run();

}