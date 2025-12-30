#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

// Forward declaration
class SpectrumAnalyzerAudioProcessor;

//==============================================================================
class SpectrumAnalyzerComponent : public juce::Component,
                                   private juce::Timer
{
public:
    //==============================================================================
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;

    //==============================================================================
    explicit SpectrumAnalyzerComponent(SpectrumAnalyzerAudioProcessor& processor);
    ~SpectrumAnalyzerComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void setPeakHoldEnabled(bool enabled);
    bool isPeakHoldEnabled() const { return peakHoldEnabled; }

private:
    //==============================================================================
    void timerCallback() override;
    
    void drawBackground(juce::Graphics& g);
    void drawGrid(juce::Graphics& g);
    void drawSpectrum(juce::Graphics& g);
    void drawPeakHold(juce::Graphics& g);
    
    void updateSpectrumData();
    void resetPeakData();
    
    float frequencyToX(float freq) const;
    float magnitudeToY(float dB) const;
    float binToFrequency(int bin) const;
    juce::String formatFrequency(float freq) const;

    //==============================================================================
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    // Spectrum data
    std::array<float, fftSize / 2> spectrumData;
    std::array<float, fftSize / 2> peakData;
    
    // State
    bool peakHoldEnabled = true;
    double currentSampleRate = 44100.0;
    
    // Futuristic Cyberpunk Colors
    const juce::Colour backgroundColor1 { 0xFF0D0D1A };  // Deep space black
    const juce::Colour backgroundColor2 { 0xFF1A0A2E };  // Dark purple
    const juce::Colour gridColor { 0x40FF00FF };         // Neon magenta grid
    const juce::Colour gridColorMajor { 0x6000FFFF };    // Cyan major grid
    const juce::Colour spectrumColor { 0xFF00FFAA };     // Neon green/cyan
    const juce::Colour spectrumGlowColor { 0xFF00FF88 }; // Bright glow
    const juce::Colour peakColor { 0xFFFF00AA };         // Hot pink/magenta
    const juce::Colour peakGlowColor { 0x80FF00FF };     // Magenta glow
    const juce::Colour textColor { 0xFF00CCFF };         // Cyan text
    const juce::Colour scanlineColor { 0x0800FFFF };     // Subtle scanlines

    // Frequency range
    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    
    // dB range
    static constexpr float mindB = -100.0f;
    static constexpr float maxdB = 0.0f;
    
    // Peak decay rate (per frame at 60fps)
    static constexpr float peakDecayRate = 0.985f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerComponent)
};
