/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;
using namespace juce;

//==============================================================================
SampleScientistAudioProcessorEditor::SampleScientistAudioProcessorEditor (SampleScientistAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), samplerBox (p)
{
    buildElements();
    startTimerHz(120);
    setSize(1700, 800);

    startTimerHz(60);
}

SampleScientistAudioProcessorEditor::~SampleScientistAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void SampleScientistAudioProcessorEditor::paint (Graphics& g)
{
    Image background = ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    g.drawImageAt(background, 0, 0);

    //g.fillAll(Colours::whitesmoke);

    //Draw the semi-transparent rectangle around components
    Rectangle<float> backgroundShade;

    if (getHeight() == 800)
        backgroundShade.setBounds(10, 10, 1680, 780);
    else
        backgroundShade.setBounds(10, 10, 1680, 1000);

    g.setColour(Colours::ghostwhite);
    g.drawRoundedRectangle(backgroundShade, 5.0f, 3.0f);

    //Draw background for rectangle
    g.setColour(Colours::black);
    g.setOpacity(0.75f);
    g.fillRoundedRectangle(backgroundShade, 5.0f);


    //=======================LABLES===============================//
    g.setFont(18.0f);
    g.setColour(Colours::snow);

    for (int i = 0; i < slider_objects.size(); i++)
    {
        auto text = audioProcessor.getParameterName(i);

        if(i < 1)
            g.drawText(text, slider_objects[i]->getRight() - 180, slider_objects[i]->getBottom(), 180, 30, Justification::centred, false);
        else if (i > 7)
        {
            if((*tremoloButton).getToggleState() && i >= 8 && i <= 9)
                g.drawText(text, slider_objects[i]->getRight() - 130, slider_objects[i]->getBottom(), 130, 30, Justification::centred, false);
            else if ((*distortionButton).getToggleState() && i >= 10 && i <= 11)
                g.drawText(text, slider_objects[i]->getRight() - 130, slider_objects[i]->getBottom(), 130, 30, Justification::centred, false);
            else if ((*reverbButton).getToggleState() && i >= 12 && i <= 15)
                g.drawText(text, slider_objects[i]->getRight() - 130, slider_objects[i]->getBottom(), 130, 30, Justification::centred, false);
            else if ((*phaserbButton).getToggleState() && i >= 16 && i <= 20)
                g.drawText(text, slider_objects[i]->getRight() - 130, slider_objects[i]->getBottom(), 130, 30, Justification::centred, false);
            else if ((*chorusButton).getToggleState() && i >= 21)
                g.drawText(text, slider_objects[i]->getRight() - 130, slider_objects[i]->getBottom(), 130, 30, Justification::centred, false);
        }
        else
            g.drawText(text, slider_objects[i]->getRight() - 175, slider_objects[i]->getBottom(), 200, 30, Justification::centred, false);
    }

    // Advanced Options
    g.setFont(27.0f);
    g.drawText((*advancedOpButton).getName(), 65, 734, 220, 40, Justification::centred, false);

    //g.drawText((String)audioProcessor.bpmValue, 1365, 734, 220, 40, Justification::centred, false);

}

void SampleScientistAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any subcomponents in your editor.
    samplerBox.setBounds(20, 20, 750, 400);

    // Pitch and Tempo
    pitchSlider.setBounds       (595, 430, 180, 50);

    // Cutoff Frequency Sliders
    lowCutFreqSlider.setBounds  (50,  530, 150, 150);
    midFreqSlider.setBounds     (300, 530, 150, 150);
    highCutFreqSlider.setBounds (550, 530, 150, 150);

    (*advancedOpButton).setBounds(40, 740, 30, 30);

    // Cutoff Frequency Sliders
    lowCutGainSlider.setBounds    (39,  800, 150, 150);
    midBWSlider.setBounds       (227, 800, 150, 150);
    midGainSlider.setBounds     (415, 800, 150, 150);
    highCutGainSlider.setBounds   (603, 800, 150, 150);

    // Delay Sliders
    (*tremoloButton).setBounds        (800, 55, 100, 60);
    tremoloDepthSlider.setBounds       (930, 20, 130, 130);
    tremoloLFOSlider.setBounds   (1130, 20, 130, 130);

    // Delay Sliders
    (*distortionButton).setBounds(800, 230, 100, 60);
    distThreshold.setBounds(930, 200, 130, 130);
    distMixSlider.setBounds(1130, 200, 130, 130);

    // Reverb Sliders
    (*reverbButton).setBounds(800, 415, 100, 60);
    rvbRoomSize.setBounds(930, 380, 130, 130);
    rvbDamp.setBounds(1080, 380, 130, 130);
    rvbWidth.setBounds(1230, 380, 130, 130);
    rvbMix.setBounds(1380, 380, 130, 130);

    // Phaser Sliders
    (*phaserbButton).setBounds(800, 595, 100, 60);
    phaserDepth.setBounds(930, 560, 130, 130);
    phaserFeedback.setBounds(1080, 560, 130, 130);
    phaserCntrFrequency.setBounds(1230, 560, 130, 130);
    phaserLFOfrequency.setBounds(1380, 560, 130, 130);
    phaserMix.setBounds(1530, 560, 130, 130);

    // Phaser Sliders
    (*chorusButton).setBounds(800, 840, 100, 60);
    chorusDepth.setBounds(930, 800, 130, 130);
    chorusFeedback.setBounds(1080, 800, 130, 130);
    chorusCntrDelay.setBounds(1230, 800, 130, 130);
    chorusLFOFreq.setBounds(1380, 800, 130, 130);
    chorusMix.setBounds(1530, 800, 130, 130);

}

void SampleScientistAudioProcessorEditor::buildElements()
{
    Image normalLogo = ImageCache::getFromMemory(BinaryData::normalLogo_png, BinaryData::normalLogo_pngSize);
    Image belowLogo = ImageCache::getFromMemory(BinaryData::belowLogo_png, BinaryData::belowLogo_pngSize);

    // Definition of Slider Components
    slider_objects =
    {
        //Pitch and Tempo
        &pitchSlider,

        // Low Freq, Mid Freq, High Freq
        &lowCutFreqSlider,  &midFreqSlider, &highCutFreqSlider,

        // Low Gain, Mid BW & Gain, High Gain
        &lowCutGainSlider,    &midBWSlider,   &midGainSlider, &highCutGainSlider,

        // Delay Time, Feedback, Mix
        &tremoloDepthSlider,   &tremoloLFOSlider,

        // Distortion Threshold, Mix
        &distThreshold,     &distMixSlider,

        // Reverb Room Size, Damp, Width, Mix
        &rvbRoomSize, &rvbDamp, &rvbWidth, &rvbMix,

        //Phaser Depth, Feedback, Center Freq, LFO Freq, Mix
        &phaserDepth, &phaserFeedback, &phaserCntrFrequency, &phaserLFOfrequency, &phaserMix,

        // Chorus Depth, Feedback, Center Delay, LFO Freq, Mix
        &chorusDepth, &chorusFeedback, &chorusCntrDelay, &chorusLFOFreq, &chorusMix,
    };
    attachment_objects =
    {
        //Pitch and Tempo
        &pitchVal,
        
        // Low Freq, Mid Freq, High Freq
        &lowCutFreqVal,     &midFreqVal,    &highCutFreqVal,
        
        // Low Gain, Mid BW & Gain, High Gain
        &lowCutGainVal,       &midBWVal,      &midGainVal,    &highCutGainVal,
        
        // Delay Time, Feedback, Mix
        &tremoloDepthVal,      &tremoloLFOVal,
        
        // Distortion Threshold, Mix
        &distortThreshVal,  &distortMixVal,

        // Reverb Room Size, Damp, Width, Mix
        &rvbRoomSizeVal, &rvbDampVal, &rvbWidthVal, &rvbMixVal,

        // Phaser Depth, Feedback, Min Freq, LFO Freq, Mix
        &phaserDepthVal, &phaserFeedbackVal, &phaserCntrFrequencyVal, &phaserLFOfrequencyVal, &phaserMixVal,

        // Chorus Depth, Feedback, Center Delay, LFO Freq, Mix
        &chorusDepthVal, &chorusFeedbackVal, &chorusCntrDelayVal, &chorusLFOfrequencyVal, &chorusMixVal,
    };

    for (int i = 0; i < slider_objects.size(); i++)
    {
        *attachment_objects[i] = make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValueTree(), audioProcessor.getParameterID(i), *slider_objects[i]);

        if (i < 1)
            slider_objects[i]->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        else
            slider_objects[i]->setSliderStyle(Slider::SliderStyle::Rotary);

        if ((i >= 1 && i < 4) || i == 9 || i == 18 || i == 19 || i == 24)
            slider_objects[i]->setTextValueSuffix(" Hz");
        else if (i >= 4 && i != 5 && i < 8)
            slider_objects[i]->setTextValueSuffix(" dB");
        else if (i == 11 || i == 15 || i == 20 || i == 25)
            slider_objects[i]->setTextValueSuffix(" %");
        else if (i == 23)
            slider_objects[i]->setTextValueSuffix(" ms");
        
        slider_objects[i]->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);

        if (i < 4)
            addAndMakeVisible(slider_objects[i]);
        else
            addChildComponent(slider_objects[i]);
    }


    addAndMakeVisible(advancedOpButton = new ImageButton("Advanced Options"));
    (*advancedOpButton).setImages(true, true, true, normalLogo, 1.0, Colours::white, normalLogo, 1.0, Colours::white,
                                belowLogo, 1.0, Colours::white, 0);
    (*advancedOpButton).setClickingTogglesState(true);
    (*advancedOpButton).addListener(this);

    addAndMakeVisible(samplerBox);

    //Delay State
    addAndMakeVisible(tremoloButton = new TextButton("Tremolo"));
    (*tremoloButton).setButtonText(TRANS("Tremolo"));
    (*tremoloButton).setColour(TextButton::buttonOnColourId, Colours::lightblue);
    (*tremoloButton).setColour(TextButton::textColourOnId, Colours::black);
    (*tremoloButton).setColour(TextButton::textColourOffId, Colours::white);
    (*tremoloButton).addListener(this);
    (*tremoloButton).setClickingTogglesState(true);

     //Distortion State
    addAndMakeVisible(distortionButton = new TextButton("Distortion"));
    (*distortionButton).setButtonText(TRANS("Distortion"));
    (*distortionButton).setColour(TextButton::buttonOnColourId, Colours::lightblue);
    (*distortionButton).setColour(TextButton::textColourOnId, Colours::black);
    (*distortionButton).setColour(TextButton::textColourOffId, Colours::white);
    (*distortionButton).addListener(this);
    (*distortionButton).setClickingTogglesState(true);

     //Reverb State
    addAndMakeVisible(reverbButton = new TextButton("Reverb"));
    (*reverbButton).setButtonText(TRANS("Reverb"));
    (*reverbButton).setColour(TextButton::buttonOnColourId, Colours::lightblue);
    (*reverbButton).setColour(TextButton::textColourOnId, Colours::black);
    (*reverbButton).setColour(TextButton::textColourOffId, Colours::white);
    (*reverbButton).addListener(this);
    (*reverbButton).setClickingTogglesState(true);

    //Phaser State
    addAndMakeVisible(phaserbButton = new TextButton("Phaser"));
    (*phaserbButton).setButtonText(TRANS("Phaser"));
    (*phaserbButton).setColour(TextButton::buttonOnColourId, Colours::lightblue);
    (*phaserbButton).setColour(TextButton::textColourOnId, Colours::black);
    (*phaserbButton).setColour(TextButton::textColourOffId, Colours::white);
    (*phaserbButton).addListener(this);
    (*phaserbButton).setClickingTogglesState(true);

    //Chorus State
    addChildComponent(chorusButton = new TextButton("Chorus"));
    (*chorusButton).setButtonText(TRANS("Chorus"));
    (*chorusButton).setColour(TextButton::buttonOnColourId, Colours::lightblue);
    (*chorusButton).setColour(TextButton::textColourOnId, Colours::black);
    (*chorusButton).setColour(TextButton::textColourOffId, Colours::white);
    (*chorusButton).addListener(this);
    (*chorusButton).setClickingTogglesState(true);
}

void SampleScientistAudioProcessorEditor::buttonClicked(Button* buttonClicked)
{
    if (buttonClicked == advancedOpButton)
    {
        // If Advanced Options was pressed
        if ((*advancedOpButton).getToggleState())
            setSize(1700, 1020);
        else
            setSize(1700, 800);

        for (int i = 4; i < 8; i++)
        {
            slider_objects[i]->setVisible((*advancedOpButton).getToggleState());
            (*chorusButton).setVisible((*advancedOpButton).getToggleState());
        }
    }
    else if (buttonClicked == tremoloButton)
    {
        audioProcessor.setButtonState("Tremolo", (*tremoloButton).getToggleState());

        for (int i = 8; i < 10; i++) { slider_objects[i]->setVisible((*tremoloButton).getToggleState()); }
    }
    else if (buttonClicked == distortionButton)
    {
        audioProcessor.setButtonState("Distortion", (*distortionButton).getToggleState());

        for (int i = 10; i < 12; i++) { slider_objects[i]->setVisible((*distortionButton).getToggleState()); }
    }
    else if (buttonClicked == reverbButton)
    {
        audioProcessor.setButtonState("Reverb", (*reverbButton).getToggleState());

        for (int i = 12; i < 16; i++) { slider_objects[i]->setVisible((*reverbButton).getToggleState()); }
    }
    else if (buttonClicked == phaserbButton)
    {
        audioProcessor.setButtonState("Phaser", (*phaserbButton).getToggleState());

        for (int i = 16; i < 21; i++)
        {
            slider_objects[i]->setVisible((*phaserbButton).getToggleState()); 
        }
    }
    else if (buttonClicked == chorusButton)
    {
        audioProcessor.setButtonState("Chorus", (*chorusButton).getToggleState());

        for (int i = 21; i < slider_objects.size(); i++)
        {
            slider_objects[i]->setVisible((*chorusButton).getToggleState());
        }
    }
}

void SampleScientistAudioProcessorEditor::timerCallback()
{
    (*tremoloButton).setToggleState(audioProcessor.getButtonState("Tremolo"), dontSendNotification);
    (*distortionButton).setToggleState(audioProcessor.getButtonState("Distortion"), dontSendNotification);
    (*reverbButton).setToggleState(audioProcessor.getButtonState("Reverb"), dontSendNotification);
    (*phaserbButton).setToggleState(audioProcessor.getButtonState("Phaser"), dontSendNotification);
    (*chorusButton).setToggleState(audioProcessor.getButtonState("Chorus"), dontSendNotification);

    repaint();
}