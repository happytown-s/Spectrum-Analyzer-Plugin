#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

//==============================================================================
class SpectrumAnalyzerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    static constexpr int fftOrder = 12;  // 2^12 = 4096
    static constexpr int fftSize = 1 << fftOrder;

    //==============================================================================
    SpectrumAnalyzerAudioProcessor();
    ~SpectrumAnalyzerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Thread-safe FIFO for passing data to GUI
    void pushNextSampleIntoFifo(float sample) noexcept;
    bool isNextFFTBlockReady() const noexcept { return nextFFTBlockReady.load(); }
    void resetFFTBlockReady() noexcept { nextFFTBlockReady.store(false); }
    
    // Get FFT data for visualization
    const std::array<float, fftSize * 2>& getFFTData() const noexcept { return fftData; }
    std::array<float, fftSize * 2>& getFFTDataForWriting() noexcept { return fftData; }
    
    // FFT object for GUI thread
    juce::dsp::FFT& getFFT() noexcept { return fft; }
    
    // Hann window
    const std::array<float, fftSize>& getHannWindow() const noexcept { return hannWindow; }

private:
    //==============================================================================
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, fftSize> hannWindow;
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady { false };

    void initializeHannWindow();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
