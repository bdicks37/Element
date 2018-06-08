/*
    InternalFormat.cpp - This file is part of Element
    Copyright (C) 2016 Kushview, LLC.  All rights reserved.
*/

#include "ElementApp.h"
#include "engine/AudioEngine.h"
#include "engine/GraphProcessor.h"
#include "engine/InternalFormat.h"

#include "engine/AllPassFilterProcessor.h"
#include "engine/AudioMixerProcessor.h"
#include "engine/CombFilterProcessor.h"
#include "engine/MidiDeviceProcessor.h"
#include "engine/MidiSequenceProcessor.h"
#include "engine/PlaceholderProcessor.h"
#include "engine/ReverbProcessor.h"
#include "engine/SubGraphProcessor.h"
#include "engine/VolumeProcessor.h"
#include "engine/WetDryProcessor.h"

#include "session/Session.h"
#include "session/UnlockStatus.h"

#include "Globals.h"

namespace Element {
    typedef GraphProcessor::AudioGraphIOProcessor IOP;
    
    InternalFormat::InternalFormat (AudioEngine& e)
        : engine (e)
    {
        {
            IOP p (IOP::audioOutputNode);
            p.fillInPluginDescription (audioOutDesc);
        }

        {
            IOP p (IOP::audioInputNode);
            p.fillInPluginDescription (audioInDesc);
        }

        {
            IOP p (IOP::midiOutputNode);
            p.fillInPluginDescription (midiOutDesc);
        }
        
        {
            IOP p (IOP::midiInputNode);
            p.fillInPluginDescription (midiInDesc);
        }
        
        {
            PlaceholderProcessor p;
            p.fillInPluginDescription (placeholderDesc);
        }

        {
            MidiDeviceProcessor in (true);
            in.fillInPluginDescription (midiInputDeviceDesc);
            MidiDeviceProcessor out (false);
            out.fillInPluginDescription (midiOutputDeviceDesc);
        }
    }

    AudioPluginInstance* InternalFormat::instantiatePlugin (const PluginDescription& desc, double, int)
    {
        if (desc.fileOrIdentifier == audioOutDesc.fileOrIdentifier)
        {
            return new IOP (IOP::audioOutputNode);
        }
        else if (desc.fileOrIdentifier == audioInDesc.fileOrIdentifier)
        {
            return new IOP (IOP::audioInputNode);
        }
        else if (desc.fileOrIdentifier == midiInDesc.fileOrIdentifier)
        {
            return new IOP (IOP::midiInputNode);
        }
        else if (desc.fileOrIdentifier == midiOutDesc.fileOrIdentifier)
        {
            return new IOP (IOP::midiOutputNode);
        }
        else if (desc.fileOrIdentifier == "element.midiInputDevice")
        {
            return new MidiDeviceProcessor (true);
        }
        else if (desc.fileOrIdentifier == "element.midiOutputDevice")
        {
            return new MidiDeviceProcessor (false);
        }
        else if (desc.fileOrIdentifier == "element.placeholder")
        {
            return new PlaceholderProcessor();
        }

        return nullptr;
    }

    const PluginDescription* InternalFormat::description (const InternalFormat::ID type)
    {
        switch (type)
        {
            case audioInputDevice:      return &audioInDesc;
            case audioOutputDevice:     return &audioOutDesc;
            case midiInputDevice:       return &midiInDesc;
            case midiOutputDevice:      return &midiOutDesc;
            case samplerProcessor:      return &samplerDesc;
            case sequenceProcessor:     return &sequencerDesc;
            case patternProcessor:      return &patternDesc;
            case midiSequence:          return &metroDesc;
            default:                    break;
        }

        return nullptr;
    }

    void InternalFormat::getAllTypes (OwnedArray <PluginDescription>& results)
    {
        for (int i = 0; i < (int) audioOutputPort; ++i)
            results.add (new PluginDescription (*description ((InternalFormat::ID) i)));
        results.add (new PluginDescription (placeholderDesc));
        results.add (new PluginDescription (midiInputDeviceDesc));
        results.add (new PluginDescription (midiOutputDeviceDesc));
    }
    
    void InternalFormat::createPluginInstance (const PluginDescription& d, double initialSampleRate,
                                               int initialBufferSize, void* userData,
                                               void (*callback) (void*, AudioPluginInstance*, const String&))
    {
        if (auto* i = instantiatePlugin (d, initialSampleRate, initialBufferSize))
            callback (userData, i, String::empty);
    }
    
    bool InternalFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept
    {
        return false;
    }
    
    
    // MARK: Element Format
    
    ElementAudioPluginFormat::ElementAudioPluginFormat (Globals& g)
        : world (g)
    { }
    
    void ElementAudioPluginFormat::findAllTypesForFile (OwnedArray <PluginDescription>& ds, const String& fileOrId)
    {
        if (fileOrId == "element.reverb")
        {
            auto* const desc = ds.add (new PluginDescription());
            desc->pluginFormatName = getName();
            desc->name = "eVerb";
            desc->manufacturerName = "Element";
            desc->category = "Effect";
            desc->fileOrIdentifier = fileOrId;
            desc->numInputChannels = 2;
            desc->numInputChannels = 2;
        }
        else if (fileOrId == "element.comb")
        {
            auto* desc = ds.add (new PluginDescription());
            desc->pluginFormatName = getName();
            desc->name = "Comb Filter (mono)";
            desc->manufacturerName = "Element";
            desc->category = "Effect";
            desc->fileOrIdentifier = fileOrId + ".mono";
            desc->numInputChannels = 1;
            desc->numOutputChannels = 1;
            
            desc = ds.add (new PluginDescription (*desc));
            desc->name = "Comb Filter (stereo)";
            desc->fileOrIdentifier = fileOrId + ".stereo";
            desc->numInputChannels = 2;
            desc->numOutputChannels = 2;
        }
        else if (fileOrId == "element.allPass")
        {
            auto* desc = ds.add (new PluginDescription());
            desc->pluginFormatName = getName();
            desc->name = "AllPass Filter (mono)";
            desc->manufacturerName = "Element";
            desc->category = "Effect";
            desc->fileOrIdentifier = fileOrId + ".mono";
            desc->numInputChannels = 1;
            desc->numOutputChannels = 1;
            
            desc = ds.add (new PluginDescription (*desc));
            desc->name = "AllPass Filter (stereo)";
            desc->fileOrIdentifier = fileOrId + ".stereo";
            desc->numInputChannels = 2;
            desc->numOutputChannels = 2;
        }
        else if (fileOrId == "element.volume")
        {
            auto* desc = ds.add (new PluginDescription());
            desc->pluginFormatName = getName();
            desc->name = "Volume (mono)";
            desc->manufacturerName = "Element";
            desc->category = "Effect";
            desc->fileOrIdentifier = fileOrId + ".mono";
            desc->numInputChannels = 1;
            desc->numOutputChannels = 1;
            
            desc = ds.add (new PluginDescription (*desc));
            desc->name = "Volume (stereo)";
            desc->fileOrIdentifier = fileOrId + ".stereo";
            desc->numInputChannels = 2;
            desc->numOutputChannels = 2;
        }
        else if (fileOrId == "element.wetDry")
        {
            auto* desc = ds.add (new PluginDescription());
            WetDryProcessor wetDry;
            wetDry.fillInPluginDescription (*desc);
        }
        else if (fileOrId == "element.reverb")
        {
            auto* desc = ds.add (new PluginDescription());
            ReverbProcessor reverb;
            reverb.fillInPluginDescription (*desc);
        }
        else if (fileOrId == "element.graph")
        {
            auto* const desc = ds.add (new PluginDescription());
            SubGraphProcessor().fillInPluginDescription (*desc);
        }
        else if (fileOrId == "element.midiSequencer")
        {
            auto* const desc = ds.add (new PluginDescription());
            MidiSequenceProcessor().fillInPluginDescription (*desc);
        }
        else if (fileOrId == "element.audioMixer")
        {
            auto* const desc = ds.add (new PluginDescription());
            AudioMixerProcessor(4).fillInPluginDescription (*desc);
        }
    }
    
    StringArray ElementAudioPluginFormat::searchPathsForPlugins (const FileSearchPath&, bool /*recursive*/, bool /*allowAsync*/)
    {
        StringArray results;
        results.add ("element.reverb");
        results.add ("element.comb");
        results.add ("element.allPass");
        results.add ("element.volume");
        results.add ("element.wetDry");
        results.add ("element.reverb");

       #if EL_USE_MIDI_SEQUENCER
        results.add ("element.midiSequencer");
       #endif
       
       #if EL_USE_SUBGRAPHS
        results.add ("element.graph");
       #endif

        results.add ("element.audioMixer");
        return results;
    }
    
    AudioPluginInstance* ElementAudioPluginFormat::instantiatePlugin (const PluginDescription& desc, 
                                                                      double sampleRate, int blockSize)
    {
        ScopedPointer<AudioPluginInstance> base;
        
        if (desc.fileOrIdentifier == "element.comb.mono")
            base = new CombFilterProcessor (false);
        
        else if (desc.fileOrIdentifier == "element.comb.stereo")
            base = new CombFilterProcessor (true);
        
        else if (desc.fileOrIdentifier == "element.allPass.mono")
            base = new AllPassFilterProcessor (false);
        
        else if (desc.fileOrIdentifier == "element.allPass.stereo")
            base = new AllPassFilterProcessor (true);
        
        else if (desc.fileOrIdentifier == "element.volume.mono")
            base = new VolumeProcessor (-30.0, 12.0, false);
        
        else if (desc.fileOrIdentifier == "element.volume.stereo")
            base = new VolumeProcessor (-30.0, 12.0, true);
        
        else if (desc.fileOrIdentifier == "element.wetDry")
            base = new WetDryProcessor();
        
        else if (desc.fileOrIdentifier == "element.reverb")
            base = new ReverbProcessor();
        
        else if (desc.fileOrIdentifier == "element.graph")
            base = (world.getUnlockStatus().isFullVersion() ? new SubGraphProcessor() : nullptr);
        
        else if (desc.fileOrIdentifier == "element.midiSequencer")
            base = (world.getUnlockStatus().isFullVersion() ? new MidiSequenceProcessor() : nullptr);
        
        else if (desc.fileOrIdentifier == "element.audioMixer")
            base = (world.getUnlockStatus().isFullVersion() ? new AudioMixerProcessor (4, sampleRate, blockSize) : nullptr);

        return base != nullptr ? base.release() : nullptr;
    }
    
    void ElementAudioPluginFormat::createPluginInstance (const PluginDescription& d, double initialSampleRate,
                                                         int initialBufferSize, void* userData,
                                                         void (*callback) (void*, AudioPluginInstance*, const String&))
    {
        if (auto* i = instantiatePlugin (d, initialSampleRate, initialBufferSize))
            callback (userData, i, String::empty);
        else
            callback (userData, nullptr, String::empty);
    }
    
    bool ElementAudioPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept
    {
        return false;
    }
}