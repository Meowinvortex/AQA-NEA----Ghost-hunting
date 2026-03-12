#ifndef AUDIO_RECORDING_H
#define AUDIO_RECORDING_H

#include "portaudio.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <atomic>
#include <thread>
#include <chrono>

namespace GH{
namespace AUDIOREC{
    bool error(PaError& err);
    
    bool start_recording(int sample_rate, int channels);
    
    bool stop_recording();
    
    bool write_audio();
    
    void thread_audio();
}
}
#endif