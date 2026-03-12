#include "portaudio.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <atomic>
#include <thread>
#include <chrono>
#include "Audio_recording.hpp"

namespace GH{
namespace AUDIOREC{
    //All the global variables used for audio recording
    static std::vector<short> buffer;  //Holds the recorded audio samples
    static std::atomic<bool> is_recording(false);  //Tells the thread to stop or not
    static std::thread recording_thread;  //Background thread that grabs the audio
    static PaStream* stream = nullptr;  //The audio stream
    static int def_sample_rate = 44100;  //Stores the sample rate
    static int def_channels = 2;  //Stores the channels
    static PaSampleFormat sample_format = paInt16;  //16-bit integer samples, the standard for WAV files


    //This Function is called acrosss the audio recording to check for errors
    bool error(PaError& err){
        if(err != paNoError && err != paInputOverflowed){//If failed to start up return error and end recording
            std::cerr<<"Error: "<<Pa_GetErrorText(err)<<std::endl;
            return true;
        }
        else{
            return false;
        }
    }


    //This function starts the audio recording
    bool start_recording(int sample_rate, int channels){
    std::cout<<"Starting recording"<<std::endl;
    const PaDeviceInfo* deviceinfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    if (deviceinfo){
        std::cout<<"Using input device: "<<deviceinfo->name<<std::endl;
        std::cout<<"Max input channels: "<<deviceinfo->maxInputChannels<<std::endl;
    }
    
    def_sample_rate = sample_rate;  //Change sample rate
    def_channels = channels;  //Change channels

    PaError err = Pa_Initialize();  //Start up PortAudio

    if(error(err)){  //check for errors
        return false;
    }

    err = Pa_OpenDefaultStream(&stream, channels, 0, sample_format, sample_rate, 512, nullptr, nullptr);  //Open the input stream

    if(error(err)){  //check for errors
        return false;
    }

    err = Pa_StartStream(stream);  //Turn on the mic

    if(error(err)){  //check for errors
        Pa_CloseStream(stream);
        Pa_Terminate();
        return false;
    }

    is_recording = true;  //Ensure the thread will know recording should start
    buffer.clear();  //Clear the buffer of any old data

    recording_thread = std::thread([] {  //Start the thread
        while(is_recording){
            short temp[512 * 2];  //Support up to stereo
            PaError err = Pa_ReadStream(stream, temp, 512); //Grab 512 frames from the microphone
            
            if(error(err)){  //Check for errors
                break;
            }

            buffer.insert(buffer.end(), temp, temp + (512 * def_channels));  //Take the data just read from the microphone and add it to the main buffer
        }
    });
    

    return true;
    }

    // Function is called to stop the recording
    bool stop_recording(){
        std::cout<<"Stopping recording"<<std::endl;
        is_recording = false;
        if(recording_thread.joinable())recording_thread.join();  //Wait for the thread to finish

        //Shut down the stream
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        return true;
    }

    //This function handles writing all the given data into a valid structure for an mp3 file
    bool write_audio() {
        std::ofstream outFile(MISC_DIR"/audio.wav" ,std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open output file." << std::endl;
            return false;
        }
        uint16_t audioFormat = 1;  //PCM
        uint16_t bitsPerSample = 16;
        uint32_t byteRate = def_sample_rate * def_channels * bitsPerSample / 8;
        uint16_t blockAlign = def_channels * bitsPerSample / 8;
        uint32_t dataSize = buffer.size() * sizeof(short);
        uint32_t chunkSize = 36 + dataSize;

        //Write RIFF header
        outFile.write("RIFF", 4);
        outFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
        outFile.write("WAVE", 4);

        //Write fmt subchunk
        outFile.write("fmt ", 4);
        uint32_t subchunk1Size = 16;
        outFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
        outFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
        outFile.write(reinterpret_cast<const char*>(&def_channels), 2);
        outFile.write(reinterpret_cast<const char*>(&def_sample_rate), 4);
        outFile.write(reinterpret_cast<const char*>(&byteRate), 4);
        outFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
        outFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

        //Write data subchunk
        outFile.write("data", 4);
        outFile.write(reinterpret_cast<const char*>(&dataSize), 4);
        outFile.write(reinterpret_cast<const char*>(buffer.data()), dataSize);

        outFile.close();
        std::cout<<"Audio file written successfully"<<std::endl;
        return true;
    }

    void thread_audio(){
        std::thread thr(write_audio);
        thr.detach();
    }
}
}