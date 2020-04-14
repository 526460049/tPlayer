
#include <iostream>
#include <thread>

#include "LivePage2.h"
#include "ClientUtil.h"
//#include "pcClient/ffmpegUtil.h"

extern "C" {
#include "SDL.h"
};



using namespace ffmpegUtil;

void sdlAudioCallback(void* userdata, Uint8* stream, int len) {
	AudioProcessor* receiver = (AudioProcessor*)userdata;
	receiver->writeAudioData(stream, len);
}

void pktReader(PacketGrabber& pGrabber, AudioProcessor* aProcessor,
	VideoProcessor* vProcessor) {
	const int CHECK_PERIOD = 10;

	cout << "INFO: pkt Reader thread started." << endl;
	int audioIndex = aProcessor->getAudioIndex();
	int videoIndex = vProcessor->getVideoIndex();

	while (!pGrabber.isFileEnd() && !aProcessor->isClosed() && !vProcessor->isClosed()) {
		while (aProcessor->needPacket() || vProcessor->needPacket()) {
			AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
			int t = pGrabber.grabPacket(packet);
			if (t == -1) {
				cout << "INFO: file finish." << endl;
				aProcessor->pushPkt(nullptr);
				vProcessor->pushPkt(nullptr);
				break;
			}
			else if (t == audioIndex && aProcessor != nullptr) {
				unique_ptr<AVPacket> uPacket(packet);
				aProcessor->pushPkt(std::move(uPacket));
			}
			else if (t == videoIndex && vProcessor != nullptr) {
				unique_ptr<AVPacket> uPacket(packet);
				vProcessor->pushPkt(std::move(uPacket));
			}
			else {
				av_packet_free(&packet);
				cout << "WARN: unknown streamIndex: [" << t << "]" << endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_PERIOD));
	}
	cout << "[THREAD] INFO: pkt Reader thread finished." << endl;
}

void picRefresher(int timeInterval, bool& exitRefresh, bool& faster) {
	cout << "picRefresher timeInterval[" << timeInterval << "]" << endl;
	while (!exitRefresh) {
		if (faster) {
			std::this_thread::sleep_for(std::chrono::milliseconds(timeInterval / 2));
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(timeInterval));
		}
	}
	cout << "[THREAD] picRefresher thread finished." << endl;
}

void playVideo(VideoProcessor& vProcessor, AudioProcessor* audio = nullptr, Fl_Image* image = nullptr) {
	//--------------------- GET SDL window READY -------------------
	Sleep(1000);
	auto width = vProcessor.getWidth();
	auto height = vProcessor.getHeight();
	auto frameRate = vProcessor.getFrameRate();
	cout << "frame rate [" << frameRate << "]" << endl;//��Ƶ֡�ʣ�
	cout << "get Width [" << width << "]" << endl;// ��Ƶ��
	cout << "get Height [" << height << "]" << endl;// ��Ƶ�ߣ�
	bool faster = false;
	//std::thread refreshThread{ picRefresher, (int)(1000 / frameRate), std::ref(exitRefresh),
	//						  std::ref(faster) };

	int failCount = 0;
	while (!vProcessor.isStreamFinished()) {
		if (!vProcessor.isClosed()) {
			Sleep(15); // 30fps ���������ʱ
			if (vProcessor.isStreamFinished()) {
				continue;  // skip REFRESH event.
			}
			if (audio != nullptr) {
				auto vTs = vProcessor.getPts();
				auto aTs = audio->getPts();
				if (vTs > aTs && vTs - aTs > 30) {
					//video faster
					continue;
				}
				else if (vTs < aTs && aTs - vTs > 30) {
					// video slower
				}
				else {
					//faster = false;
				}
			}
			auto pic = vProcessor.getFrame();
	//		cout << "��ǰ֡" << pic << endl;
			if (pic != nullptr) {
				Fl_RGB_Image* image2 = new Fl_RGB_Image(pic->data[0], width, height);
				image = image2->copy(640, 360);
				image->draw(290, 20);
				delete (image);
				delete (image2);
				if (!vProcessor.refreshFrame()) {
					cout << "WARN: vProcessor.refreshFrame false" << endl;
				}
			}
			else {
				failCount++;
				cout << "WARN: getFrame fail. failCount = " << failCount << endl;
			}

		}
		else{
			cout << "close video." << endl;
			// close window.
			break;
		}
	}
	//refreshThread.join();
	cout << "[THREAD] Sdl video thread finish: failCount = " << failCount << endl;
}

void startSdlAudio(SDL_AudioDeviceID& audioDeviceID, AudioProcessor& aProcessor) {
	//--------------------- GET SDL audio READY -------------------
	Sleep(500);
	// audio specs containers
	SDL_AudioSpec wanted_specs;
	SDL_AudioSpec specs;

	cout << "aProcessor.getSampleFormat() = " << aProcessor.getSampleFormat() << endl;
	cout << "aProcessor.getSampleRate() = " << aProcessor.getOutSampleRate() << endl;
	cout << "aProcessor.getChannels() = " << aProcessor.getOutChannels() << endl;
	cout << "++" << endl;

	int samples = -1;
	while (true) {
		cout << "getting audio samples." << endl;
		samples = aProcessor.getSamples();
		if (samples <= 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else {
			cout << "get audio samples:" << samples << endl;
			break;
		}
	}

	// set audio settings from codec info
	wanted_specs.freq = aProcessor.getOutSampleRate();
	wanted_specs.format = AUDIO_S16SYS;
	wanted_specs.channels = aProcessor.getOutChannels();
	wanted_specs.samples = samples;
	wanted_specs.callback = sdlAudioCallback;
	wanted_specs.userdata = &aProcessor;

	// open audio device
	audioDeviceID = SDL_OpenAudioDevice(nullptr, 0, &wanted_specs, &specs, 0);

	// SDL_OpenAudioDevice returns a valid device ID that is > 0 on success or 0 on failure
	if (audioDeviceID == 0) {
		string errMsg = "Failed to open audio device:";
		errMsg += SDL_GetError();
		cout << errMsg << endl;
		throw std::runtime_error(errMsg);
	}

	cout << "wanted_specs.freq:" << wanted_specs.freq << endl;
	// cout << "wanted_specs.format:" << wanted_specs.format << endl;
	std::printf("wanted_specs.format: Ox%X\n", wanted_specs.format);
	cout << "wanted_specs.channels:" << (int)wanted_specs.channels << endl;
	cout << "wanted_specs.samples:" << (int)wanted_specs.samples << endl;

	cout << "------------------------------------------------" << endl;

	cout << "specs.freq:" << specs.freq << endl;
	// cout << "specs.format:" << specs.format << endl;
	std::printf("specs.format: Ox%X\n", specs.format);
	cout << "specs.channels:" << (int)specs.channels << endl;
	cout << "specs.silence:" << (int)specs.silence << endl;
	cout << "specs.samples:" << (int)specs.samples << endl;


	SDL_PauseAudioDevice(audioDeviceID, 0);
	//cout << "[THREAD] audio start thread finish." << endl;
}

LivePage2::LivePage2(Fl_Window* window) : Fl_Group(0, 0, window->w(), window->h()) {

	window->label(ClientUtil::fl_str("ֱ������"));

	info = new Fl_Group(30,20,230,500);
	roomId = new Fl_Box(40, 40, 0, 0, ClientUtil::fl_str("�����: 100001"));
	roomId->align(FL_ALIGN_RIGHT);
	roomId->labelsize(18);
	roomUser = new Fl_Box(40, 70, 0, 0, ClientUtil::fl_str("����: ��������"));
	roomUser->align(FL_ALIGN_RIGHT);
	roomUser->labelsize(18);
	btn1 = new Fl_Button(100, 480, 90, 30, ClientUtil::fl_str("�˳�"));
	btn1->box(_FL_ROUND_UP_BOX);
	btn1->clear_visible_focus();
	info->end();

	video = new Fl_Box(290, 20, 640, 360);
	video->box(FL_BORDER_FRAME);

	box_cmt_image = new Fl_Text_Display(290, 400, 640, 120);
	box_cmt_image->align(FL_ALIGN_TOP_LEFT);
	box_cmt_image->box(FL_BORDER_BOX);
	text_buffer = new Fl_Text_Buffer();
	text_buffer->text(ClientUtil::fl_str("��û������"));
	box_cmt_image->buffer(text_buffer);

	//������Ƶ�ĳ�ʼ��
	auto formatCtx = packetGrabber.getFormatCtx();
	av_dump_format(formatCtx, 0, "", 0);
	videoProcessor = new VideoProcessor(formatCtx);
	videoProcessor->start();
	audioProcessor = new AudioProcessor(formatCtx);
	audioProcessor->start();
	SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		string errMsg = "Could not initialize SDL -";
		errMsg += SDL_GetError();
		cout << errMsg << endl;
		throw std::runtime_error(errMsg);
	}
	SDL_AudioDeviceID audioDeviceID;

	std::thread readerThread{pktReader, std::ref(packetGrabber), audioProcessor, videoProcessor };
	std::thread startAudioThread(startSdlAudio, std::ref(audioDeviceID),std::ref(*audioProcessor));
	std::thread videoThread{playVideo, std::ref(*videoProcessor), audioProcessor, image};
	readerThread.detach();
	startAudioThread.detach();
	videoThread.detach();
	PlayUtil* util = new PlayUtil{ window, videoProcessor, audioProcessor };
	btn1->callback((Fl_Callback*)switchPage, util);
	this->end();
}

void LivePage2::switchPage(Fl_Button* btn, PlayUtil* util) {
	//Fl::remove_timeout(play);
	SDL_Quit();
	SDL_CloseAudioDevice(0);
	cout << util->videoProcessor->close() << endl;
	cout << util->audioProcessor->close() << endl;
	Sleep(100);
	util->window->clear();
	util->window->begin();
//	Fl_Group* group = new LiveList(util->window);
	util->window->end();
	util->window->redraw();
}


