#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      spectrumComponent(p)
{
    // Setup Peak Hold button
    peakHoldButton.setButtonText("Peak Hold");
    peakHoldButton.setToggleState(true, juce::dontSendNotification);
    peakHoldButton.onClick = [this]()
    {
        spectrumComponent.setPeakHoldEnabled(peakHoldButton.getToggleState());
    };
    addAndMakeVisible(peakHoldButton);
    
    // Add spectrum component
    addAndMakeVisible(spectrumComponent);
    
    // Set window size
    setSize(defaultWidth, defaultHeight);
    setResizable(true, true);
    setResizeLimits(300, 200, 1200, 800);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    const float w = static_cast<float>(getWidth());
    
    // Header gradient background
    juce::ColourGradient headerGradient(
        juce::Colour(0xFF0D0D1A), 0, 0,
        juce::Colour(0xFF1A0A2E), w, static_cast<float>(headerHeight), false);
    headerGradient.addColour(0.5, juce::Colour(0xFF150D20));
    g.setGradientFill(headerGradient);
    g.fillRect(0, 0, getWidth(), headerHeight);
    
    // Neon glow behind title
    g.setColour(juce::Colour(0x3000FFAA));
    g.fillRoundedRectangle(8, 5, 195, headerHeight - 10, 3.0f);
    
    // Title with glow effect
    g.setFont(juce::Font(17.0f, juce::Font::bold));
    
    // Glow layers
    g.setColour(juce::Colour(0x4000FFAA));
    g.drawText("SPECTRUM ANALYZER", 13, 1, 190, headerHeight, juce::Justification::centredLeft);
    
    g.setColour(juce::Colour(0xFF00FFAA));
    g.drawText("SPECTRUM ANALYZER", 12, 0, 190, headerHeight, juce::Justification::centredLeft);
    
    // Neon separator line with glow
    const float sepY = static_cast<float>(headerHeight - 1);
    
    // Glow
    g.setColour(juce::Colour(0x4000FFFF));
    g.drawLine(0, sepY - 1, w, sepY - 1, 2.0f);
    
    // Core line
    g.setColour(juce::Colour(0xFF00FFFF));
    g.drawLine(0, sepY, w, sepY, 1.0f);
}

void SpectrumAnalyzerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Header area
    auto headerArea = bounds.removeFromTop(headerHeight);
    
    // Peak Hold button - right side of header
    peakHoldButton.setBounds(headerArea.removeFromRight(120).reduced(10, 8));
    
    // Spectrum component takes the rest
    spectrumComponent.setBounds(bounds);
}
