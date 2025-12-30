#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SpectrumAnalyzerComponent.h"

class SpectrumAnalyzerAudioProcessor;

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    // UI Components
    SpectrumAnalyzerComponent spectrumComponent;
    juce::ToggleButton peakHoldButton;

    // Constants
    static constexpr int headerHeight = 32;
    static constexpr int defaultWidth = 500;
    static constexpr int defaultHeight = 300;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
