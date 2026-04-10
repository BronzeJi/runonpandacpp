#include <iostream>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

typedef float SAMPLE;

// ===================== AUDIO CALLBACK =====================
static int audioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void*
)
{
    const SAMPLE* in = (const SAMPLE*)inputBuffer;
    SAMPLE* out = (SAMPLE*)outputBuffer;

    if (!inputBuffer)
    {
        for (unsigned long i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
        }
        return paContinue;
    }

    // === SIMPLE ROUTING: stereo pass-through ===
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        float left  = *in++;   // channel 0
        float right = *in++;   // channel 1

        *out++ = left;
        *out++ = right;
    }

    return paContinue;
}

// ===================== MAIN =====================
int main()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio init failed\n";
        return -1;
    }

    // 🔥 SELECT YOUR DEVICES HERE
    int inputDevice = 1;   // Microphone
    int outputDevice = 3;   // Headphones

    const PaDeviceInfo* inInfo = Pa_GetDeviceInfo(inputDevice);
    const PaDeviceInfo* outInfo = Pa_GetDeviceInfo(outputDevice);

    std::cout << "Input Device: " << inInfo->name << std::endl;
    std::cout << "  Max input channels: " << inInfo->maxInputChannels << std::endl;
    std::cout << "  Default SR: " << inInfo->defaultSampleRate << std::endl;

    std::cout << "Output Device: " << outInfo->name << std::endl;
    std::cout << "  Max output channels: " << outInfo->maxOutputChannels << std::endl;

    // ===================== PARAMETERS =====================
    PaStreamParameters inputParams;
    inputParams.device = inputDevice;
    inputParams.channelCount = 2;  // try 1 if error
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = inInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStreamParameters outputParams;
    outputParams.device = outputDevice;
    outputParams.channelCount = 2;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = outInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    // ===================== FORMAT CHECK =====================
    PaError formatErr = Pa_IsFormatSupported(
        &inputParams,
        &outputParams,
        SAMPLE_RATE
    );

    if (formatErr != paFormatIsSupported)
    {
        std::cerr << "Format NOT supported: "
                  << Pa_GetErrorText(formatErr) << std::endl;

        std::cerr << "👉 Try:\n";
        std::cerr << "  - input channels = 1\n";
        std::cerr << "  - or paInt16 instead of paFloat32\n";
        return -1;
    }

    // ===================== OPEN STREAM =====================
    PaStream* stream;

    err = Pa_OpenStream(
        &stream,
        &inputParams,
        &outputParams,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        audioCallback,
        nullptr
    );

    if (err != paNoError)
    {
        std::cerr << "Failed to open stream: "
                  << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    // ===================== START =====================
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "Failed to start stream\n";
        return -1;
    }

    std::cout << "\n🎧 Running... speak into mic (press ENTER to stop)\n";
    std::cin.get();

    // ===================== CLEANUP =====================
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}