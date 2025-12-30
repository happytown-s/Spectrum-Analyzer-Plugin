#include "SpectrumAnalyzerComponent.h"
#include "PluginProcessor.h"
#include <cmath>

//==============================================================================
SpectrumAnalyzerComponent::SpectrumAnalyzerComponent(SpectrumAnalyzerAudioProcessor& processor)
    : audioProcessor(processor)
{
    spectrumData.fill(-100.0f);
    peakData.fill(-100.0f);
    
    // Start timer at 60fps
    startTimerHz(60);
}

SpectrumAnalyzerComponent::~SpectrumAnalyzerComponent()
{
    stopTimer();
}

//==============================================================================
void SpectrumAnalyzerComponent::paint(juce::Graphics& g)
{
    drawBackground(g);
    drawGrid(g);
    
    if (peakHoldEnabled)
    {
        drawPeakHold(g);
    }
    
    drawSpectrum(g);
}

void SpectrumAnalyzerComponent::resized()
{
    // No child components to layout
}

//==============================================================================
void SpectrumAnalyzerComponent::setPeakHoldEnabled(bool enabled)
{
    peakHoldEnabled = enabled;
    
    if (!enabled)
    {
        resetPeakData();
    }
    
    repaint();
}

void SpectrumAnalyzerComponent::resetPeakData()
{
    peakData.fill(-100.0f);
}

//==============================================================================
void SpectrumAnalyzerComponent::timerCallback()
{
    if (audioProcessor.isNextFFTBlockReady())
    {
        updateSpectrumData();
        audioProcessor.resetFFTBlockReady();
        repaint();
    }
    else
    {
        // Still update peak decay even without new data
        if (peakHoldEnabled)
        {
            bool needsRepaint = false;
            for (size_t i = 0; i < peakData.size(); ++i)
            {
                if (peakData[i] > mindB + 1.0f)  // Only decay if above noise floor
                {
                    peakData[i] -= 0.3f;
                    if (peakData[i] < mindB)
                        peakData[i] = mindB;
                    needsRepaint = true;
                }
            }
            if (needsRepaint)
            {
                repaint();
            }
        }
    }
}

void SpectrumAnalyzerComponent::updateSpectrumData()
{
    // Get sample rate from processor
    currentSampleRate = audioProcessor.getSampleRate();
    if (currentSampleRate <= 0)
        currentSampleRate = 44100.0;
    
    // Get FFT data and perform FFT
    auto& fftData = audioProcessor.getFFTDataForWriting();
    audioProcessor.getFFT().performFrequencyOnlyForwardTransform(fftData.data());
    
    const int numBins = fftSize / 2;
    const float smoothingFactor = 0.7f;
    const float noiseFloor = -96.0f;  // Threshold below which we ignore
    
    for (int i = 0; i < numBins; ++i)
    {
        // Calculate magnitude
        float magnitude = fftData[i];
        
        // Normalize by FFT size
        magnitude /= static_cast<float>(fftSize);
        
        // Convert to dB
        float dB = magnitude > 0.0f 
                   ? 20.0f * std::log10(magnitude) 
                   : mindB;
        
        // Clamp to range
        dB = juce::jlimit(mindB, maxdB, dB);
        
        // Apply smoothing for less jittery display
        spectrumData[i] = spectrumData[i] * smoothingFactor + dB * (1.0f - smoothingFactor);
        
        // Update peak data
        if (peakHoldEnabled)
        {
            // Only update peak if signal is above noise floor
            if (spectrumData[i] > noiseFloor && spectrumData[i] > peakData[i])
            {
                peakData[i] = spectrumData[i];
            }
            else if (peakData[i] > mindB)
            {
                // Decay peak slowly (subtract dB, since this is a dB scale)
                peakData[i] -= 0.3f;  // About 0.3 dB per frame at 60fps = ~18dB/sec
                if (peakData[i] < mindB)
                    peakData[i] = mindB;
            }
        }
    }
}

//==============================================================================
void SpectrumAnalyzerComponent::drawBackground(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    
    // Gradient background - deep space to purple
    juce::ColourGradient bgGradient(backgroundColor1, 0, 0,
                                     backgroundColor2, 0, bounds.getHeight(), false);
    bgGradient.addColour(0.5, backgroundColor1.interpolatedWith(backgroundColor2, 0.3f));
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);
    
    // Scanline effect for CRT/hologram look
    g.setColour(scanlineColor);
    for (float y = 0; y < bounds.getHeight(); y += 3.0f)
    {
        g.drawHorizontalLine(static_cast<int>(y), 0, bounds.getWidth());
    }
    
    // Subtle vignette effect (darker corners)
    juce::ColourGradient vignetteH(juce::Colours::black.withAlpha(0.3f), 0, bounds.getCentreY(),
                                    juce::Colours::transparentBlack, bounds.getWidth() * 0.15f, bounds.getCentreY(), false);
    g.setGradientFill(vignetteH);
    g.fillRect(bounds);
    
    juce::ColourGradient vignetteH2(juce::Colours::black.withAlpha(0.3f), bounds.getWidth(), bounds.getCentreY(),
                                     juce::Colours::transparentBlack, bounds.getWidth() * 0.85f, bounds.getCentreY(), false);
    g.setGradientFill(vignetteH2);
    g.fillRect(bounds);
}

void SpectrumAnalyzerComponent::drawGrid(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    
    // Minor grid lines (magenta) - more frequency markers
    g.setColour(gridColor);
    
    // Frequency markers: 50, 100, 150, 200, 250, 300, 400, 500, 600, 800, 1k, 2k, 3k, 4k, 5k, 6k, 8k, 10k, 15k, 20k
    const std::array<float, 20> freqMarkers = { 
        50.0f, 100.0f, 150.0f, 200.0f, 250.0f, 300.0f, 400.0f, 500.0f, 600.0f, 800.0f,
        1000.0f, 2000.0f, 3000.0f, 4000.0f, 5000.0f, 6000.0f, 8000.0f, 10000.0f, 15000.0f, 20000.0f 
    };
    
    for (float freq : freqMarkers)
    {
        const float x = frequencyToX(freq);
        if (x > 0 && x < width)
        {
            g.drawVerticalLine(static_cast<int>(x), 0.0f, height);
        }
    }
    
    // Major grid lines with labels (cyan with glow)
    const std::array<float, 6> majorFreqs = { 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f };
    
    for (float freq : majorFreqs)
    {
        const float x = frequencyToX(freq);
        if (x > 0 && x < width)
        {
            // Glow effect
            g.setColour(gridColorMajor.withAlpha(0.2f));
            g.drawLine(x - 2, 0, x - 2, height, 1.0f);
            g.drawLine(x + 2, 0, x + 2, height, 1.0f);
            
            // Main line
            g.setColour(gridColorMajor);
            g.drawLine(x, 0, x, height, 1.5f);
            
            // Frequency label with glow
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            g.setColour(textColor);
            g.drawText(formatFrequency(freq), 
                      static_cast<int>(x) + 4, 
                      static_cast<int>(height) - 18, 
                      40, 15, 
                      juce::Justification::left);
        }
    }
    
    // Horizontal lines at dB markers - denser at bottom, sparser at top
    const std::array<float, 8> dBMarkers = { 0.0f, -6.0f, -12.0f, -24.0f, -40.0f, -60.0f, -80.0f, -100.0f };
    
    for (float dB : dBMarkers)
    {
        const float y = magnitudeToY(dB);
        if (y > 0 && y < height)
        {
            // Minor lines in magenta
            g.setColour(gridColor);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, width);
            
            // dB label
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.setColour(textColor.withAlpha(0.8f));
            juce::String label = juce::String(static_cast<int>(dB)) + " dB";
            g.drawText(label, 8, static_cast<int>(y) - 7, 50, 15, juce::Justification::left);
        }
    }
    
    // 0dB reference line - brighter
    g.setColour(gridColorMajor);
    g.drawLine(0, magnitudeToY(0.0f), width, magnitudeToY(0.0f), 2.0f);
}

void SpectrumAnalyzerComponent::drawSpectrum(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    
    juce::Path spectrumPath;
    juce::Path fillPath;
    
    bool pathStarted = false;
    float lastX = 0.0f;
    
    const int numBins = fftSize / 2;
    
    for (int i = 1; i < numBins; ++i)
    {
        const float freq = binToFrequency(i);
        
        // Skip frequencies outside our display range
        if (freq < minFreq || freq > maxFreq)
            continue;
        
        const float x = frequencyToX(freq);
        const float y = magnitudeToY(spectrumData[i]);
        
        if (!pathStarted)
        {
            spectrumPath.startNewSubPath(x, y);
            fillPath.startNewSubPath(x, height);
            fillPath.lineTo(x, y);
            pathStarted = true;
        }
        else
        {
            spectrumPath.lineTo(x, y);
            fillPath.lineTo(x, y);
        }
        
        lastX = x;
    }
    
    // Close fill path
    if (pathStarted)
    {
        fillPath.lineTo(lastX, height);
        fillPath.closeSubPath();
        
        // Multi-layer gradient fill for depth
        juce::ColourGradient fillGradient(
            spectrumColor.withAlpha(0.5f), 0, 0,
            spectrumColor.withAlpha(0.0f), 0, height, false);
        fillGradient.addColour(0.3, spectrumGlowColor.withAlpha(0.3f));
        fillGradient.addColour(0.7, spectrumColor.withAlpha(0.1f));
        g.setGradientFill(fillGradient);
        g.fillPath(fillPath);
        
        // Outer glow (wider, more transparent)
        g.setColour(spectrumGlowColor.withAlpha(0.15f));
        g.strokePath(spectrumPath, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Middle glow
        g.setColour(spectrumGlowColor.withAlpha(0.3f));
        g.strokePath(spectrumPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Inner glow
        g.setColour(spectrumColor.withAlpha(0.7f));
        g.strokePath(spectrumPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Core line (brightest)
        g.setColour(spectrumColor);
        g.strokePath(spectrumPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void SpectrumAnalyzerComponent::drawPeakHold(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float noiseFloorThreshold = mindB + 5.0f;  // Don't draw if near noise floor
    
    juce::Path peakPath;
    bool pathStarted = false;
    bool hasValidPeaks = false;
    
    const int numBins = fftSize / 2;
    
    for (int i = 1; i < numBins; ++i)
    {
        const float freq = binToFrequency(i);
        
        if (freq < minFreq || freq > maxFreq)
            continue;
        
        // Skip bins at noise floor to prevent flickering
        if (peakData[i] <= noiseFloorThreshold)
        {
            // If we were drawing, end this segment
            if (pathStarted)
            {
                pathStarted = false;
            }
            continue;
        }
        
        hasValidPeaks = true;
        const float x = frequencyToX(freq);
        const float y = magnitudeToY(peakData[i]);
        
        if (!pathStarted)
        {
            peakPath.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            peakPath.lineTo(x, y);
        }
    }
    
    // Draw peak line with neon glow
    if (hasValidPeaks)
    {
        // Outer glow
        g.setColour(peakGlowColor.withAlpha(0.2f));
        g.strokePath(peakPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Middle glow
        g.setColour(peakGlowColor.withAlpha(0.4f));
        g.strokePath(peakPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Core line
        g.setColour(peakColor);
        g.strokePath(peakPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

//==============================================================================
float SpectrumAnalyzerComponent::frequencyToX(float freq) const
{
    const float width = static_cast<float>(getWidth());
    
    // Logarithmic scale: x = width * log(freq/minFreq) / log(maxFreq/minFreq)
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logFreq = std::log10(std::max(freq, minFreq));
    
    return width * (logFreq - logMin) / (logMax - logMin);
}

float SpectrumAnalyzerComponent::magnitudeToY(float dB) const
{
    const float height = static_cast<float>(getHeight());
    
    // Non-linear dB scale with moderate compression above -60dB
    // Split the range: 0 to -60dB (slightly compressed) and -60 to -100dB (expanded)
    
    const float splitPoint = -60.0f;
    const float topRatio = 0.35f;  // Top 60dB uses 35% of height (was 15%)
    
    float y;
    if (dB >= splitPoint)
    {
        // 0dB to -60dB: use top 35% of display
        float normalized = (maxdB - dB) / (maxdB - splitPoint);  // 0 to 1
        y = height * topRatio * normalized;
    }
    else
    {
        // -60dB to -100dB: use remaining 65% of display
        float normalized = (splitPoint - dB) / (splitPoint - mindB);  // 0 to 1
        y = height * topRatio + height * (1.0f - topRatio) * normalized;
    }
    
    return y;
}

float SpectrumAnalyzerComponent::binToFrequency(int bin) const
{
    return static_cast<float>(bin) * static_cast<float>(currentSampleRate) / static_cast<float>(fftSize);
}

juce::String SpectrumAnalyzerComponent::formatFrequency(float freq) const
{
    if (freq >= 1000.0f)
    {
        return juce::String(freq / 1000.0f, 0) + "k";
    }
    return juce::String(static_cast<int>(freq));
}
