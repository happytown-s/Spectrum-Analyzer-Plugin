#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      fft(fftOrder)
{
    initializeHannWindow();
    fifo.fill(0.0f);
    fftData.fill(0.0f);
}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor()
{
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::initializeHannWindow()
{
    for (int i = 0; i < fftSize; ++i)
    {
        hannWindow[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
    }
}

//==============================================================================
const juce::String SpectrumAnalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAnalyzerAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SpectrumAnalyzerAudioProcessor::producesMidi() const
{
    return false;
}

bool SpectrumAnalyzerAudioProcessor::isMidiEffect() const
{
    return false;
}

double SpectrumAnalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAnalyzerAudioProcessor::getNumPrograms()
{
    return 1;
}

int SpectrumAnalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAnalyzerAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SpectrumAnalyzerAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SpectrumAnalyzerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    fifoIndex = 0;
    fifo.fill(0.0f);
    fftData.fill(0.0f);
}

void SpectrumAnalyzerAudioProcessor::releaseResources()
{
}

bool SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SpectrumAnalyzerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have corresponding input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Mix all channels to mono and push to FIFO
    if (totalNumInputChannels > 0)
    {
        const int numSamples = buffer.getNumSamples();
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float monoSample = 0.0f;
            
            for (int channel = 0; channel < totalNumInputChannels; ++channel)
            {
                monoSample += buffer.getSample(channel, sample);
            }
            
            monoSample /= static_cast<float>(totalNumInputChannels);
            pushNextSampleIntoFifo(monoSample);
        }
    }
}

void SpectrumAnalyzerAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady.load())
        {
            // Copy FIFO data to FFT data buffer with Hann window applied
            for (int i = 0; i < fftSize; ++i)
            {
                fftData[i] = fifo[i] * hannWindow[i];
            }
            // Zero-pad the second half
            for (int i = fftSize; i < fftSize * 2; ++i)
            {
                fftData[i] = 0.0f;
            }
            nextFFTBlockReady.store(true);
        }
        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

//==============================================================================
bool SpectrumAnalyzerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerAudioProcessorEditor(*this);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void SpectrumAnalyzerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerAudioProcessor();
}
