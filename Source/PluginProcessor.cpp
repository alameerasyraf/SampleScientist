/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SampleScientistAudioProcessor::SampleScientistAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                      #endif
                        ), parameters(*this, nullptr, "Parameter", createParameters())
                       
#endif
{
    formatManager.registerBasicFormats();

    for (int i = 0; i < mNumVoices; i++)
        sampler.addVoice(new SamplerVoice());

    pitch_shifter_L = make_unique<SoundTouch>();
    pitch_shifter_R = make_unique<SoundTouch>();

    tremoloState = false;
    distortState = false;
    rvbState = false;
    phsrState = false;
    chorusState = false;

}

SampleScientistAudioProcessor::~SampleScientistAudioProcessor()
{

}

//==============================================================================
const String SampleScientistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SampleScientistAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SampleScientistAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SampleScientistAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SampleScientistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SampleScientistAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SampleScientistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SampleScientistAudioProcessor::setCurrentProgram (int index)
{
}

const String SampleScientistAudioProcessor::getProgramName (int index)
{
    return {};
}

void SampleScientistAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void SampleScientistAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SampleScientistAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

bool SampleScientistAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SampleScientistAudioProcessor::createEditor()
{
    return new SampleScientistAudioProcessorEditor(*this);
}

void SampleScientistAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SampleScientistAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

AudioProcessorValueTreeState::ParameterLayout SampleScientistAudioProcessor::createParameters()
{
    // Parameter Vector
    vector<unique_ptr<RangedAudioParameter>> parameterVector;

    // Pitch and Tempo
    parameterVector.push_back(make_unique<AudioParameterInt>(   "pitch", "Pitch Control",   -12,    12,     0));

    // Low Cutoff, Mid Freq, High Cutoff
    parameterVector.push_back(make_unique<AudioParameterFloat>("lowCutOff",     "Low Shelf Center Freq.",   150.0f,     1000.0f, 350.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("midFreq",       "Mid Band Frequency",       801.0f,     2000.0f, 1300.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("highCutOff",    "High-Shelf Center Freq.",  1001.0f,    6000.0f, 3000.0f));

    // Advanced Options
    // Low Gain, Mid BW, Mid Gain, High Gain
    parameterVector.push_back(make_unique<AudioParameterFloat>("lowGain",   "Low Gain Value",       -20.00, 20.00,  -12.00));
    parameterVector.push_back(make_unique<AudioParameterFloat>("midBW",     "Mid Bandwidth Value",  0.025,  5,      0.6));
    parameterVector.push_back(make_unique<AudioParameterFloat>("midGain",   "Mid Gain Value",       -20.00, 20.00,  3.00));
    parameterVector.push_back(make_unique<AudioParameterFloat>("highGain",  "High Gain Value",      -20.00, 20.00,  4.00));

    // Effects Variables
    // DELAY
	parameterVector.push_back(make_unique<AudioParameterFloat>("tremoloDepth",     "Depth",       0.0f, 1.0f, 0.5f));
	parameterVector.push_back(make_unique<AudioParameterFloat>("tremoloLFOFreq", "LFO Frequency",   0.0f, 10.0f, 2.0f));

	// DISTORTION
	parameterVector.push_back(make_unique<AudioParameterFloat>("distThresh",    "Threshold",    0.0f, 1.0f, 0.8f));
	parameterVector.push_back(make_unique<AudioParameterFloat>("distMix",       "Mix",          0.0f, 100.0f, 50.0f));

	// REVERB
	parameterVector.push_back(make_unique<AudioParameterFloat>("rvbRoomSize",   "Size",     0.0f, 1.0f,     0.8f));
	parameterVector.push_back(make_unique<AudioParameterFloat>("rvbDamp",       "Damp",     0.0f, 1.0f,     0.5f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("rvbWidth",      "Width",    0.0f, 1.0f,     0.8f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("rvbMix",        "Mix",      0.0f, 100.0f,   50.0f));

    // PHASER
    parameterVector.push_back(make_unique<AudioParameterFloat>("phaserDepth",       "Depth",            0.0f,   1.0f,       1.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("phaserFeedback",    "Feedback",         -1.0f,  1.0f,      0.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("phaserCntrFreq",    "Center Freq.",     50.0f,  1000.0f,    80.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("phaserLFOFreq",     "LFO Frequency",    0.0f,   2.0f,       0.05f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("phaserMix",         "Mix",              0.0f,   1.0f,       1.0f));

    // CHORUS
    parameterVector.push_back(make_unique<AudioParameterFloat>("chorusDepth",       "Depth",            0.0f, 1.0f, 1.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("chorusFeedback",    "Feedback",         0.0f, 0.9f, 0.5f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("chorusCntrDelay",   "Center Delay",     0.0f, 5.0f, 2.0f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("chorusLFOFreq",     "LFO Frequency",    0.0f, 2.0f, 0.05f));
    parameterVector.push_back(make_unique<AudioParameterFloat>("chorusMix",         "Mix",              0.0f, 1.0f, 1.0f));

	// Final Gain
	parameterVector.push_back(make_unique<AudioParameterFloat>("finalGain", "Gain", 0.0, 2.00, 1.00));

    return { parameterVector.begin(), parameterVector.end() };
}

//==============================================================================
void SampleScientistAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback initialisation that you need..

    // Processes Spec
    ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    spec.sampleRate = sampleRate;

    // Sampler
    sampler.setCurrentPlaybackSampleRate(sampleRate);

    // Pitch Shifting
    pitch_shifter_L->setChannels(1);
    pitch_shifter_L->setSampleRate(sampleRate);

    pitch_shifter_R->setChannels(1);
    pitch_shifter_R->setSampleRate(sampleRate);

    // Filter
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    updateEQFilters();

    // Tremolo
    lfoPhase = 0.0f;
    inverseSampleRate = 1.0f / (float)sampleRate;
    twoPi = 2.0f * 3.142;

    // Reverb
    reverb.prepare(spec);

    // Phaser
    phaser.prepare(spec);

    // Chorus
    chorus.prepare(spec);
}

void SampleScientistAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    //===========================VARIABLES======================================

    // Buffer Information
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    //==========================PROCESSING======================================
    // This is here to avoid people getting screaming feedback
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Play Audio as MIDI
    midiPlayingFunctions(buffer, midiMessages);

    // Changing Pitch of Audio Using Slider
    pitchProcessing(buffer);

    // Changing Tempo of Audio Using Slider
    tempoProcessing(buffer);

    // Delay Using Slider
    tremoloProcessing(buffer);

    // EQ Processing Using Slider
    filterProcessing(buffer);

    //// Distortion Processing
    distortionProcessing(buffer);

    // Reverb Processings
    reverbProcessing(buffer);
    
    // Phaser Processings
    phaserProcessing(buffer);

    // Chorus Processings
    chorusProcessing(buffer);
}

//==============================================================================
// SAMPLER FUNCTIONS
void SampleScientistAudioProcessor::midiPlayingFunctions(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // Playing sample using MIDI Keyboard
    MidiMessage m;
    MidiBuffer::Iterator it{ midiMessages };
    int sample;

    while (it.getNextEvent(m, sample))
    {
        if (m.isNoteOn())
            notePlayed = true;
        else if (m.isNoteOff())
            notePlayed = false;
    }

    sampleCount = notePlayed ? sampleCount += buffer.getNumSamples() : 0;

    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void SampleScientistAudioProcessor::loadFile()
{
    sampler.clearSounds();

    auto soundChooser = make_unique<FileChooser>("Please load a File");
    soundChooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
        {
            auto file = chooser.getResult();
            // the reader can be a local variable here since it's not needed by the SamplerSound after this
            unique_ptr<AudioFormatReader> reader{ formatManager.createReaderFor(file) };
            if (reader)
            {
                BigInteger range;
                range.setRange(0, 128, true);
                sampler.addSound(new SamplerSound("Sample", *reader, range, 60, 0.1, 0.1, 10.0));
            }
        });
}

void SampleScientistAudioProcessor::loadFile(const String& path)
{
    sampler.clearSounds();

    auto file = File(path);
    // the reader can be a local variable here since it's not needed by the other classes after this
    unique_ptr<AudioFormatReader> reader{ formatManager.createReaderFor(file) };
    if (reader)
    {
        BigInteger range;
        range.setRange(0, 128, true);
        sampler.addSound(new SamplerSound("Sample", *reader, range, 60, 0.1, 0.1, 10.0));
    }
}

AudioBuffer<float>& SampleScientistAudioProcessor::getWaveForm()
{
    auto sound = dynamic_cast<SamplerSound*>(sampler.getSound(sampler.getNumSounds() - 1).get());
    if (sound)
    {
        return *sound->getAudioData();
    }
    // just in case it somehow happens that the sound doesn't exist or isn't a SamplerSound, return a static instance of an empty AudioBuffer here...
    static AudioBuffer<float> dummybuffer;
    return dummybuffer;
}

//==============================================================================
// PITCH PROCESSING
void SampleScientistAudioProcessor::pitchProcessing(AudioBuffer<float>& buffer)
{
    // Pitch Information
    auto newPitchVal = parameters.getRawParameterValue("pitch")->load();

    float* pitch_buffer_L = buffer.getWritePointer(0);
    float* pitch_buffer_R = buffer.getWritePointer(1);

    pitch_shifter_L->setPitchSemiTones(newPitchVal);
    pitch_shifter_R->setPitchSemiTones(newPitchVal);

    pitch_shifter_L->putSamples(pitch_buffer_L, buffer.getNumSamples());
    pitch_shifter_L->receiveSamples(pitch_buffer_L, buffer.getNumSamples());

    pitch_shifter_R->putSamples(pitch_buffer_R, buffer.getNumSamples());
    pitch_shifter_R->receiveSamples(pitch_buffer_R, buffer.getNumSamples());
}

//==============================================================================
// TEMPO PROCESSING
void SampleScientistAudioProcessor::tempoProcessing(AudioBuffer<float>& buffer)
{
    
}

//================  ==============================================================
// FILTER PROCESSING
void SampleScientistAudioProcessor::filterProcessing(AudioBuffer<float>& buffer)
{
    updateEQFilters();
    AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    ProcessContextReplacing<float> leftContext(leftBlock);
    ProcessContextReplacing<float> rightContext(rightBlock);
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

// MID BOOST FILTER
void SampleScientistAudioProcessor::updatePeakFilter(const Filters& filterSettings)
{
    auto peakCoefficients = makePeakFilter(filterSettings, getSampleRate());
    updateCoefficients(leftChain.get<FilterPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<FilterPositions::Peak>().coefficients, peakCoefficients);
}

// LOW CUT FILTER
void SampleScientistAudioProcessor::updateLowShelfFilters(const Filters& filterSettings)
{
    auto lowCutCoefficients = makeLowShelfFilter(filterSettings, getSampleRate());

    auto& leftLowCut = leftChain.get<FilterPositions::LowCut>();
    auto& rightLowCut = rightChain.get<FilterPositions::LowCut>();

    updateCutFilter(leftLowCut, lowCutCoefficients);
    updateCutFilter(rightLowCut, lowCutCoefficients);
}

// HIGH CUT FILTER
void SampleScientistAudioProcessor::updateHighShelfFilters(const Filters& filterSettings)
{
    auto highCutCoefficients = makeHighShelfFilter(filterSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<FilterPositions::HighCut>();
    auto& rightHighCut = rightChain.get<FilterPositions::HighCut>();

    updateCutFilter(leftHighCut, highCutCoefficients);
    updateCutFilter(rightHighCut, highCutCoefficients);
}

// UPDATE FILTERS
void SampleScientistAudioProcessor::updateEQFilters()
{
    auto filterSettings = getFilters(parameters);
    updatePeakFilter(filterSettings);
    updateLowShelfFilters(filterSettings);
    updateHighShelfFilters(filterSettings);
}

//==============================================================================
// TREMOLO PROCESSING
void SampleScientistAudioProcessor::tremoloProcessing(AudioBuffer<float>& buffer)
{
    if (getButtonState("Tremolo"))
    {
        float depth = parameters.getParameter("tremoloDepth")->getValue();
        float lfoFreq = parameters.getParameter("tremoloLFOFreq")->getValue();

        float phase;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            phase = lfoPhase;

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                const float in = channelData[sample];
                float modulation = lfo(phase, 1);
                float out = in * (1 - depth + depth * modulation);

                channelData[sample] = out;

                phase += lfoFreq * inverseSampleRate;
                if (phase >= 1.0f)
                    phase -= 1.0f;
            }
        }

        lfoPhase = phase;
    }
}

float SampleScientistAudioProcessor::lfo(float phase, int waveform)
{
    float out = 0.0f;

    switch (waveform) {
    case 0: {
        out = 0.5f + 0.5f * sinf(2 * 3.142 * phase);
        break;
    }
    case 1: {
        if (phase < 0.25f)
            out = 0.5f + 2.0f * phase;
        else if (phase < 0.75f)
            out = 1.0f - 2.0f * (phase - 0.25f);
        else
            out = 2.0f * (phase - 0.75f);
        break;
    }
    case 2: {
        if (phase < 0.5f)
            out = 0.5f + phase;
        else
            out = phase - 0.5f;
        break;
    }
    case 3: {
        if (phase < 0.5f)
            out = 0.5f - phase;
        else
            out = 1.5f - phase;
        break;
    }
    case 4: {
        if (phase < 0.5f)
            out = 0.0f;
        else
            out = 1.0f;
        break;
    }
    case 5: {
        if (phase < 0.48f)
            out = 1.0f;
        else if (phase < 0.5f)
            out = 1.0f - 50.0f * (phase - 0.48f);
        else if (phase < 0.98f)
            out = 0.0f;
        else
            out = 50.0f * (phase - 0.98f);
        break;
    }
    }

    return out;
}

//==============================================================================
// DISTORTION PROCESSING
void SampleScientistAudioProcessor::distortionProcessing(AudioBuffer<float>& buffer)
{
    if (getButtonState("Distortion"))
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto threshold = parameters.getRawParameterValue("distThresh")->load();

                auto mix = parameters.getRawParameterValue("distMix")->load() / 100;

                const float inSample = buffer.getSample(channel, sample);
                
                float val = inSample;

                for (int i = 0; i < 5; i++)
                {
                    if (val < -threshold)
                        val = -threshold;
                    else if (val > threshold)
                        val = threshold;
                    else
                        val = inSample - ((inSample * inSample * inSample) / 3.f);
                }

                float out = (inSample * (1.0 - mix)) + (mix * val);

                buffer.setSample(channel, sample, out);
            }
        }
    }
}

//==============================================================================
// REVERB PROCESSING
void SampleScientistAudioProcessor::reverbProcessing(AudioBuffer<float>& buffer)
{
    if (getButtonState("Reverb"))
    {
        updateReverbSettings();

        AudioBlock<float> block(buffer);
        ProcessContextReplacing<float> ctx(block);
        reverb.process(ctx);
    }
}

void SampleScientistAudioProcessor::updateReverbSettings()
{
    auto mixLevel = parameters.getRawParameterValue("rvbMix")->load();
    auto wetLevel = mixLevel / 100;
    auto dryLevel = 1.0 - wetLevel;

    rvbParams.roomSize = parameters.getRawParameterValue("rvbRoomSize")->load();
    rvbParams.damping = parameters.getRawParameterValue("rvbDamp")->load();
    rvbParams.width = parameters.getRawParameterValue("rvbWidth")->load();
    rvbParams.wetLevel = wetLevel;
    rvbParams.dryLevel = dryLevel;

    reverb.setParameters(rvbParams);
}

//==============================================================================
// PHASER PROCESSING
void SampleScientistAudioProcessor::phaserProcessing(AudioBuffer<float>& buffer)
{
    if (getButtonState("Phaser"))
    {
        updatePhaserSettings();

        AudioBlock<float> block(buffer);
        ProcessContextReplacing<float> ctx(block);
        phaser.process(ctx);
    }
}

void SampleScientistAudioProcessor::updatePhaserSettings()
{
    auto depth = parameters.getRawParameterValue("phaserDepth")->load();
    auto feedback = parameters.getRawParameterValue("phaserFeedback")->load();
    auto minFreq = parameters.getRawParameterValue("phaserCntrFreq")->load();
    auto lfoFreq = parameters.getRawParameterValue("phaserLFOFreq")->load();
    auto mix = parameters.getRawParameterValue("phaserMix")->load();

    phaser.setDepth(depth);
    phaser.setFeedback(feedback);
    phaser.setCentreFrequency(minFreq);
    phaser.setRate(lfoFreq);
    phaser.setMix(mix);
}

//==============================================================================
// CHORUS PROCESSING
void SampleScientistAudioProcessor::chorusProcessing(AudioBuffer<float>& buffer)
{
    if (getButtonState("Chorus"))
    {
        updateChorusrSettings();

        AudioBlock<float> block(buffer);
        ProcessContextReplacing<float> ctx(block);
        chorus.process(ctx);
    }
}

void SampleScientistAudioProcessor::updateChorusrSettings()
{
    auto depth = parameters.getRawParameterValue("chorusDepth")->load();
    auto feedback = parameters.getRawParameterValue("chorusFeedback")->load();
    auto centerDelay = parameters.getRawParameterValue("chorusCntrDelay")->load();
    auto lfoFreq = parameters.getRawParameterValue("chorusLFOFreq")->load();
    auto mix = parameters.getRawParameterValue("chorusMix")->load();

    chorus.setDepth(depth);
    chorus.setFeedback(feedback);
    chorus.setCentreDelay(centerDelay);
    phaser.setRate(lfoFreq);
    chorus.setMix(mix);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleScientistAudioProcessor();
}

// OTHER FUNCTIONS
Filters getFilters(AudioProcessorValueTreeState& parameters)
{
    Filters settings;

    settings.lowCutFreq     = parameters.getRawParameterValue("lowCutOff")->load();
    settings.lowGain        = parameters.getRawParameterValue("lowGain")->load();

    settings.midBoostFreq   = parameters.getRawParameterValue("midFreq")->load();
    settings.midGain        = parameters.getRawParameterValue("midGain")->load();
    settings.midBandWidth   = parameters.getRawParameterValue("midBW")->load();

    settings.highCutFreq    = parameters.getRawParameterValue("highCutOff")->load();
    settings.highGain       = parameters.getRawParameterValue("highGain")->load();

    return settings;
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}