/*
  ==============================================================================

    SamplerBox.cpp
    Author:  Al Ameer Asyraf

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SamplerBox.h"

//==============================================================================
SamplerBox::SamplerBox(SampleScientistAudioProcessor& p) : audioProcessor(p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

SamplerBox::~SamplerBox()
{
}

void SamplerBox::paint(Graphics& g)
{
    samplerArea.setBounds(0, 20, 750, 400);
    g.setColour(Colours::transparentBlack);
    g.fillRect(samplerArea);

    auto waveform = audioProcessor.getWaveForm();

    if (waveform.getNumSamples() > 0)
    {
        Path p;
        audioPoints.clear();

        auto ratio = waveform.getNumSamples() / samplerArea.getWidth();
        auto buffer = waveform.getReadPointer(0);

        //scale audio file to window on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio)
        {
            audioPoints.push_back(buffer[sample]);
        }

        g.setColour(Colours::yellow);
        p.startNewSubPath(samplerArea.getX(), samplerArea.getHeight() / 2);

        //scale on y axis
        for (int sample = 0; sample < audioPoints.size(); ++sample)
        {
            auto point = jmap<float>(audioPoints[sample], -1.0f, 1.0f, samplerArea.getHeight(), 0);
            p.lineTo(sample, point);
        }

        g.strokePath(p, PathStrokeType(2));

        g.setColour(Colours::white);
        g.setFont(15.0f);
        auto textBounds = getLocalBounds().reduced(20, 20);
        g.drawFittedText(mFileName, textBounds, Justification::topRight, 1);

        auto playHeadPosition = jmap<int>(audioProcessor.getSampleCount(), 0, audioProcessor.getWaveForm().getNumSamples(), 0, samplerArea.getWidth());

        // This draws the white cursor thingy
        g.drawLine(playHeadPosition, 20, playHeadPosition, samplerArea.getHeight(), 2.0f);

        /*g.setColour(Colours::black.withAlpha(0.2f));
        g.fillRect(playHeadPosition, 20, playHeadPosition, samplerArea.getHeight());*/
    }
    else
    {
        g.setColour(Colours::white);
        g.setFont(40.0f);
        g.drawFittedText("Drop an Audio File to Load", getLocalBounds(), Justification::centred, 1);
    }
}

void SamplerBox::resized()
{
    // This method is where you should set the bounds of any child components that your component contains.

}

bool SamplerBox::isInterestedInFileDrag(const StringArray& files)
{
    for (auto file : files)
    {
        if (file.contains(".wav") || file.contains(".mp3") || file.contains(".aif"))
            return true;
    }

    return false;
}

void SamplerBox::filesDropped(const StringArray& files, int x, int y)
{
    for (auto file : files)
    {
        if (isInterestedInFileDrag(file))
        {
            auto myFile = make_unique<File>(file);
            mFileName = myFile->getFileNameWithoutExtension();

            audioProcessor.loadFile(file);
        }
    }

    repaint();
}