// tPlayer.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include"LivePage2.h"


using namespace std;

int main(int argc, char** argv) {
	Fl_Window* window = new Fl_Window(960, 540);
	Fl_Group* btn1 = new LivePage2(window);
	window->end();
	window->show(argc, argv);
	return Fl::run();


}