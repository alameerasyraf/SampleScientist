/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SoundTouch/SoundTouch.h"

using namespace std;
using namespace juce;
using namespace dsp;
using namespace soundtouch;

using Filter = IIR::Filter<float>;

using CutFilter = ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = ProcessorChain<CutFilter, Filter, CutFilter>;

using Coefficients = Filter::CoefficientsPtr;

//==============================================================================
// EQUALIZER CLASSES
struct Filters
{
    float lowCutFreq{ 0 }, lowGain{ 0 };
    float midBoostFreq{ 0 }, midGain{ 0 }, midBandWidth{};
    float highCutFreq{ 0 }, highGain{ 0 };
};

Filters getFilters(AudioProcessorValueTreeState& parameters);

enum FilterPositions
{
    LowCut, Peak, HighCut
};

template<int Index, typename FilterType, typename CoefficientType>
void update(FilterType& filter, const CoefficientType& coefficients)
{
    updateCoefficients(filter.template get<Index>().coefficients, coefficients[Index]);
    filter.template setBypassed<Index>(false);
}

template<typename FilterType, typename CoefficientType>
void updateCutFilter(FilterType& leftLowCut, const CoefficientType& cutCoefficients)
{
    leftLowCut.template setBypassed<0>(true);

    update<0>(leftLowCut, cutCoefficients);
}

inline auto makePeakFilter(const Filters& filterSettings, double sampleRate)
{
    ReferenceCountedArray<IIR::Coefficients<float>> coefArray;
    auto peakCoef = IIR::Coefficients<float>::makePeakFilter(sampleRate, filterSettings.midBoostFreq, filterSettings.midBandWidth, Decibels::decibelsToGain(filterSettings.midGain));

    return peakCoef;
}
inline auto makeLowShelfFilter(const Filters& filterSettings, double sampleRate)
{
    ReferenceCountedArray<IIR::Coefficients<float>> coefArray;
    auto lowShelfCoef = IIR::Coefficients<float>::makeLowShelf(sampleRate, filterSettings.lowCutFreq, 1.0, Decibels::decibelsToGain(filterSettings.lowGain));
    
    coefArray.insert(0, *lowShelfCoef);

    return coefArray;
}
inline auto makeHighShelfFilter(const Filters& filterSettings, double  sampleRate)
{
    ReferenceCountedArray<IIR::Coefficients<float>> coefArray;
    auto highShelfCoef = IIR::Coefficients<float>::makeHighShelf(sampleRate, filterSettings.highCutFreq, 1.0, Decibels::decibelsToGain(filterSettings.highGain));

    coefArray.insert(0, *highShelfCoef);

    return coefArray;
}
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

//=========================================================================================================
// SAMPLESCIENTIST CLASS
class SampleScientistAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SampleScientistAudioProcessor();
    ~SampleScientistAudioProcessor() override;
    void releaseResources() override;
    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    #endif
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    AudioProcessorValueTreeState& getValueTree() { return parameters; }

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    // Sampler Functions
    void midiPlayingFunctions(AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    void loadFile();
    void loadFile(const String& path);
    int getNumSamplerSounds() { return sampler.getNumSounds(); }
    AudioBuffer<float>& getWaveForm();
    atomic<bool>& isNotePlayed() { return notePlayed; }
    atomic<int>& getSampleCount() { return sampleCount; }
    AudioFormatManager& getAudioFormatManager() { return formatManager; }

    // TestingVariables
    atomic<double> currentPosition{ 0 };
    atomic<double> bpmValue{0};

    // Delay Functions
    void setButtonState(String button, bool buttonState)
    { 
        if (button == "Tremolo") { tremoloState = buttonState; }
        else if (button == "Distortion") { distortState = buttonState; }
        else if (button == "Reverb") { rvbState = buttonState; }
        else if (button == "Phaser") { phsrState = buttonState; }
        else if (button == "Chorus") { chorusState = buttonState; }

    }
    bool getButtonState(String button)
    {
        if (button == "Tremolo") { return tremoloState; }
        else if (button == "Distortion") { return distortState; }
        else if (button == "Reverb") { return rvbState; }
        else if (button == "Phaser") { return phsrState; }
        else if (button == "Chorus") { return chorusState; }
    }
    
private:

    //================================FUNCTIONS=====================================
    AudioProcessorValueTreeState::ParameterLayout createParameters();

    // PITCH AND TEMPO FUNCTIONS
    void pitchProcessing(AudioBuffer<float>& buffer);

    // TEMPO FUNCTIONS
    void tempoProcessing(AudioBuffer<float>& buffer);

    // FILTER FUNCTIONS
    void filterProcessing(AudioBuffer<float>& buffer);
    void updatePeakFilter(const Filters& filterSettings);
    void updateLowShelfFilters(const Filters& filterSettings);
    void updateHighShelfFilters(const Filters& filterSettings);
    void updateEQFilters();
    
    // DISTORTION FUCNTIONS
    void distortionProcessing(AudioBuffer<float>& buffer);

    // DISTORTION FUCNTIONS
    void reverbProcessing(AudioBuffer<float>& buffer);
    void updateReverbSettings();

    // PHASER FUCNTIONS
    void phaserProcessing(AudioBuffer<float>& buffer);
    void updatePhaserSettings();

    // CHORUS FUCNTIONS
    void chorusProcessing(AudioBuffer<float>& buffer);
    void updateChorusrSettings();

    // TREMOLO FUCNTIONS
    void tremoloProcessing(AudioBuffer<float>& buffer);
    float lfo(float phase, int waveform);

    //===============================VARIABLES=====================================
    AudioProcessorValueTreeState    parameters;

    // Filter Variables
    MonoChain               leftChain, rightChain;

    // Sampler Variables
    Synthesiser             sampler;
    const int               mNumVoices{ 3 };
    atomic<bool>            shouldUpdate { false };
    atomic<bool>            notePlayed { false };
    atomic<int>             sampleCount{ 0 };

    // Audio Format Manager (File Selections)
    AudioFormatManager      formatManager;

    // Pitch Shifting Variables
    unique_ptr<SoundTouch>  pitch_shifter_L;
    unique_ptr<SoundTouch>  pitch_shifter_R;

    // Tempo Variables


    // Tremolo Variables
    bool                    tremoloState;
    float lfoPhase;
    float inverseSampleRate;
    float twoPi;

    // Distortion Variables
    bool                    distortState;

    // Reverb Variables
    dsp::Reverb             reverb;
    dsp::Reverb::Parameters rvbParams;
    bool                    rvbState;

    // Phaser Variables
    Phaser<float>           phaser;
    bool                    phsrState;

    // Chorus Variables
    Chorus<float>            chorus;
    bool                    chorusState;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleScientistAudioProcessor)
};
