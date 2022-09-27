/*
  ==============================================================================

    SamplerBox.h
    Author:  Al Ameer Asyraf

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//=======================================================================================SAMPLERBOX CLASS==============================================//
class SamplerBox : public Component, public FileDragAndDropTarget
{
public:
    SamplerBox(SampleScientistAudioProcessor& p);
    ~SamplerBox();

    void paint(Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    SampleScientistAudioProcessor& audioProcessor;
    Rectangle<int> samplerArea;


private:

    vector<float> audioPoints;
    bool mShouldBePainting{ false };

    String mFileName{ "" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerBox)
};
