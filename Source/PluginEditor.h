/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SamplerBox.h"

using namespace std;
using namespace juce;

struct AtomicLabel : Component, Timer
{
    AtomicLabel(atomic<double>& valueToUse) : value(valueToUse)
    {
        startTimerHz(60);
        addAndMakeVisible(label);
    }

    void resized() override
    {
        label.setBounds(getLocalBounds());
    }

    void timerCallback() override
    {
        label.setText(String(value.load()), dontSendNotification);
    }

    Label label;
    atomic<double>& value;

};

//==============================================================================
/**
*/
class SampleScientistAudioProcessorEditor  : public AudioProcessorEditor, public Button::Listener, public Timer
{
public:
    SampleScientistAudioProcessorEditor (SampleScientistAudioProcessor&);
    ~SampleScientistAudioProcessorEditor() override;

    //==============================================================================
    // FUNCTIONS
    void paint (Graphics&) override;
    void resized() override;
    void buildElements();
    void buttonClicked(Button* buttonClicked) override;
    void timerCallback() override;

    //==============================================================================
    // VARIABLES

    // Pitch and Tempo Values
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> pitchVal;            // Attachment for Pitch Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> tempoVal;            // Attachment for Tempo Value

    // Low Cut, Mid Boost and High Cut Frequency Values
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowCutFreqVal;       // Attachment for Low Cut Freq Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midFreqVal;          // Attachment for Mid Boost Freq Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> highCutFreqVal;      // Attachment for High Cut Freq Value

    // Low, Mid, High Bandwidth Values
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowCutGainVal;         // Attachment for Low Cut Bandwidth Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midBWVal;            // Attachment for Mid Bandwidth Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midGainVal;          // Attachment for Mid Gain Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> highCutGainVal;        // Attachment for High Cut Bandwidth Value

    // Delay
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> tremoloDepthVal;       // Attachment for Delay Time Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> tremoloLFOVal;   // Attachment for Delay Feedback Value

    // Distortion
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> distortThreshVal;   // Attachment for Distortion Threshold Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> distortMixVal;      // Attachment for Distortion Mix Value

    // Reverb
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> rvbRoomSizeVal;     // Attachment for Reverb Room Size Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> rvbDampVal;         // Attachment for Reverb Dampness Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> rvbWidthVal;        // Attachment for Reverb Width Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> rvbMixVal;          // Attachment for Reverb Mix Value

    // Phaser
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> phaserDepthVal;            // Attachment for Reverb Room Size Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> phaserFeedbackVal;         // Attachment for Reverb Dampness Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> phaserCntrFrequencyVal;     // Attachment for Reverb Width Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> phaserLFOfrequencyVal;       // Attachment for Reverb Mix Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> phaserMixVal;       // Attachment for Reverb Mix Value

    // Chorus
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> chorusDepthVal;            // Attachment for Reverb Room Size Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> chorusFeedbackVal;         // Attachment for Reverb Dampness Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> chorusCntrDelayVal;     // Attachment for Reverb Width Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> chorusLFOfrequencyVal;       // Attachment for Reverb Mix Value
    unique_ptr<AudioProcessorValueTreeState::SliderAttachment> chorusMixVal;       // Attachment for Reverb Mix Value

private:
    // This reference is provided as a quick way for your editor to access the processor object that created it.
    //==============================================================================
    // VARIABLES

    SampleScientistAudioProcessor& audioProcessor;

    SamplerBox samplerBox;

    Array<Slider*> slider_objects;
    Array<unique_ptr<AudioProcessorValueTreeState::SliderAttachment>*> attachment_objects;

    // Pitch and Tempo Sliders
    Slider pitchSlider;
    Slider tempoSlider;

    // EQ Sliders
    Slider lowCutFreqSlider;                        // Low Cut Frequency Slider
    Slider midFreqSlider;                           // Mid Frequency Slider
    Slider highCutFreqSlider;                       // High Cut Frequency Slider

    ScopedPointer<ImageButton> advancedOpButton;    // Advanced Option Button

    Slider lowCutGainSlider;                          // Low Cut Gain Value Slider
    Slider midBWSlider;                             // Mid Band Gain Slider
    Slider midGainSlider;                           // Mid Gain Slider
    Slider highCutGainSlider;                         // High Cut Gain Value Slider

    // Tremolo Sliders
    Slider tremoloDepthSlider;                               // Delay Time
    Slider tremoloLFOSlider;                                // Delay Feedback

    // Distortion Sliders
    Slider distThreshold;
    Slider distMixSlider;

    // Reverb Sliders
    Slider rvbRoomSize;
    Slider rvbDamp;
    Slider rvbWidth;
    Slider rvbMix;

    // Phaser Sliders
    Slider phaserDepth;
    Slider phaserFeedback;
    Slider phaserCntrFrequency;
    Slider phaserSweepWidth;
    Slider phaserLFOfrequency;
    Slider phaserMix;

    // Chorus Sliders
    Slider chorusDepth;
    Slider chorusFeedback;
    Slider chorusCntrDelay;
    Slider chorusLFOFreq;
    Slider chorusMix;

    // Buttons to Switch the Effects states to ON/OFF
    ScopedPointer<TextButton> tremoloButton;
    ScopedPointer<TextButton> distortionButton;
    ScopedPointer<TextButton> reverbButton;
    ScopedPointer<TextButton> phaserbButton;
    ScopedPointer<TextButton> chorusButton;

    /** Timer callback rate Hz */
    int timerValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleScientistAudioProcessorEditor)
};
