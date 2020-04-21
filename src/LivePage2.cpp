
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


void playVideo(VideoProcessor& vProcessor, AudioProcessor* audio = nullptr, Fl_Image* image = nullptr) {
	//--------------------- GET SDL window READY -------------------
	Sleep(1000);
	auto width = vProcessor.getWidth();
	auto height = vProcessor.getHeight();
	auto frameRate = vProcessor.getFrameRate();
	cout << "frame rate [" << frameRate << "]" << endl;//视频帧率？
	cout << "get Width [" << width << "]" << endl;// 视频宽？
	cout << "get Height [" << height << "]" << endl;// 视频高？
	bool faster = false;
	//std::thread refreshThread{ picRefresher, (int)(1000 / frameRate), std::ref(exitRefresh),
	//						  std::ref(faster) };

	int failCount = 0;
	while (!vProcessor.isStreamFinished()) {
		if (!vProcessor.isClosed()) {
			Sleep(15); // 30fps 尽量快的延时
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

			if (pic != nullptr) {
				Fl_RGB_Image* image2 = new Fl_RGB_Image(pic->data[0], width, height);
				image = image2->copy(640, 360);
				image->draw(0, 0);
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

void playAudio(SDL_AudioDeviceID& audioDeviceID, AudioProcessor& aProcessor) {
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

	window->label(ClientUtil::fl_str("player"));
	video = new Fl_Box(0,0,640,360);
	video->box(FL_BORDER_FRAME);
	//播放视频的初始化
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
	std::thread startAudioThread(playAudio, std::ref(audioDeviceID),std::ref(*audioProcessor));
	std::thread videoThread{playVideo, std::ref(*videoProcessor), audioProcessor, image};
	readerThread.detach();
	startAudioThread.detach();
	videoThread.detach();
	PlayUtil* util = new PlayUtil{ window, videoProcessor, audioProcessor };
	//btn1->callback((Fl_Callback*)switchPage, util);
	this->end();
}




