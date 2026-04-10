#include <iostream>
#include <portaudio.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 256

typedef float SAMPLE;

// Simple callback: mic → output
static int audioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
)
{
    const SAMPLE* in = (const SAMPLE*)inputBuffer;
    SAMPLE* out = (SAMPLE*)outputBuffer;

    if (inputBuffer == nullptr)
    {
        // if no input, output silence
        for (unsigned long i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
        }
        return paContinue;
    }

    // stereo copy (assumes 2 channels)
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        *out++ = *in++;  // left
        *out++ = *in++;  // right
    }

    return paContinue;
}

int main()
{
    PaError err = Pa_Initialize();
    int inputDevice=12;
    int outputDevice=1;
    const PaDeviceInfo* info = Pa_GetDeviceInfo(inputDevice);
    
    if (err != paNoError)
    {
        std::cerr << "PortAudio init failed\n";
        return -1;
    }

    PaStream* stream;
    
    err = Pa_OpenDefaultStream(
        &stream,
        10,              // input channels (mic)
        10,              // output channels (headphones)
        paFloat32,      // sample format
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        audioCallback,
        nullptr
    );

    if (err != paNoError)
    {
        std::cerr << "Failed to open stream. Error: " << Pa_GetErrorText(err) << std::endl;
        

        std::cout << "Max input channels: " << info->maxInputChannels << std::endl;
        std::cout << "Default sample rate: " << info->defaultSampleRate << std::endl;
        return -1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "Failed to start stream\n";
        return -1;
    }

    std::cout << "Running... press Enter to stop\n";
    std::cin.get();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}