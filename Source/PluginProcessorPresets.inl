void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };

    add("Init", {});
    add("Kick/Psy Kick Stab", {{"osc1_fold",0.3f},{"filter_cutoff",500.0f},{"filter_env",1.0f},{"filter_reso",0.5f},{"amp_attack",0.001f},{"amp_decay",0.18f},{"amp_sustain",0.0f},{"amp_release",0.1f},{"master_drive",0.45f}});
    add("Kick/808 Boom", {{"osc1_table",0.0f},{"osc3_octave",-2.0f},{"osc3_level",1.0f},{"filter_cutoff",400.0f},{"amp_attack",0.001f},{"amp_decay",0.8f},{"amp_sustain",0.0f},{"amp_release",0.5f},{"master_drive",0.35f}});
    add("Kick/Hardstyle K", {{"osc1_fold",0.55f},{"filter_cutoff",350.0f},{"filter_env",1.0f},{"filter_reso",0.4f},{"amp_attack",0.001f},{"amp_decay",0.25f},{"amp_sustain",0.0f},{"amp_release",0.15f},{"master_drive",0.6f}});
    add("Bass/FM Bass", {{"osc1_level",0.9f},{"osc2_level",0.7f},{"osc2_octave",1.0f},{"fm_2to1",0.65f},{"filter_cutoff",1200.0f},{"filter_reso",0.55f},{"filter_env",0.7f},{"amp_attack",0.005f},{"amp_decay",0.35f},{"amp_sustain",0.4f},{"amp_release",0.2f},{"master_drive",0.25f}});
    add("Bass/Reese Wide", {{"osc1_table",0.4f},{"unison_voices",5.0f},{"unison_detune",28.0f},{"unison_spread",1.0f},{"filter_cutoff",700.0f},{"amp_attack",0.05f},{"amp_decay",0.5f},{"amp_sustain",0.7f},{"amp_release",0.4f},{"chorus_mix",0.25f}});
    add("Bass/Neuro Wobble", {{"osc1_table",0.3f},{"lfo_rate",4.5f},{"lfo_amount",0.7f},{"filter_cutoff",600.0f},{"filter_reso",0.75f},{"amp_attack",0.005f},{"amp_decay",0.3f},{"amp_sustain",0.5f},{"amp_release",0.2f},{"master_drive",0.35f}});
    add("Bass/Moog Sub", {{"osc1_table",0.0f},{"osc1_level",1.0f},{"filter_cutoff",200.0f},{"filter_reso",0.2f},{"amp_attack",0.01f},{"amp_decay",0.5f},{"amp_sustain",0.8f},{"amp_release",0.4f}});
    add("Bass/Dark Psy", {{"osc1_table",0.15f},{"fm_3to1",0.5f},{"filter_cutoff",800.0f},{"filter_reso",0.7f},{"filter_env",0.8f},{"amp_attack",0.005f},{"amp_decay",0.3f},{"amp_sustain",0.4f},{"amp_release",0.2f},{"master_drive",0.3f}});
    add("Acid/Reso Slide", {{"osc1_table",0.55f},{"filter_cutoff",400.0f},{"filter_reso",0.92f},{"filter_drive",0.55f},{"filter_env",0.85f},{"amp_attack",0.001f},{"amp_decay",0.22f},{"amp_sustain",0.15f},{"amp_release",0.12f},{"master_drive",0.35f}});
    add("Acid/Squelch", {{"osc1_table",0.7f},{"filter_cutoff",280.0f},{"filter_reso",0.95f},{"filter_drive",0.65f},{"filter_env",1.0f},{"amp_attack",0.001f},{"amp_decay",0.18f},{"amp_sustain",0.05f},{"amp_release",0.1f},{"master_drive",0.4f}});
    add("Acid/303 Square", {{"osc1_table",0.9f},{"filter_cutoff",500.0f},{"filter_reso",0.88f},{"filter_drive",0.45f},{"filter_env",0.75f},{"amp_attack",0.001f},{"amp_decay",0.25f},{"amp_sustain",0.2f},{"amp_release",0.15f},{"master_drive",0.3f}});
    add("Lead/Hi-Tech", {{"osc1_fold",0.25f},{"fm_2to1",0.4f},{"filter_cutoff",6000.0f},{"amp_attack",0.005f},{"amp_decay",0.2f},{"amp_sustain",0.7f},{"amp_release",0.25f},{"delay_mix",0.2f}});
    add("Lead/Supersaw", {{"osc1_table",0.4f},{"unison_voices",7.0f},{"unison_detune",30.0f},{"unison_spread",1.0f},{"filter_cutoff",6500.0f},{"amp_attack",0.02f},{"amp_decay",0.3f},{"amp_sustain",0.8f},{"amp_release",0.5f},{"chorus_mix",0.3f}});
    add("Lead/Trance Pluck", {{"osc1_table",0.35f},{"filter_cutoff",8000.0f},{"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.2f},{"amp_release",0.4f},{"delay_mix",0.35f},{"reverb_mix",0.25f}});
    add("Lead/Acid Screech", {{"osc1_fold",0.45f},{"filter_cutoff",900.0f},{"filter_reso",0.85f},{"filter_env",0.9f},{"filter_drive",0.4f},{"amp_attack",0.001f},{"amp_decay",0.25f},{"amp_sustain",0.3f},{"amp_release",0.15f}});
    add("FM/Bell DX", {{"osc2_octave",3.0f},{"fm_2to1",0.9f},{"filter_cutoff",9000.0f},{"amp_attack",0.001f},{"amp_decay",1.2f},{"amp_sustain",0.2f},{"amp_release",1.5f},{"reverb_mix",0.4f}});
    add("FM/Deep Operator", {{"osc2_octave",2.0f},{"fm_2to1",0.85f},{"filter_cutoff",3000.0f},{"amp_attack",0.01f},{"amp_decay",0.4f},{"amp_sustain",0.5f},{"amp_release",0.3f}});
    add("Pad/Warm Strings", {{"osc1_table",0.35f},{"unison_voices",5.0f},{"unison_detune",18.0f},{"filter_cutoff",2200.0f},{"amp_attack",0.6f},{"amp_decay",0.8f},{"amp_sustain",0.9f},{"amp_release",2.0f},{"chorus_mix",0.35f},{"reverb_mix",0.4f}});
    add("Pad/Alien", {{"osc1_table",0.7f},{"osc2_table",0.9f},{"amp_attack",0.8f},{"amp_decay",0.5f},{"amp_sustain",0.8f},{"amp_release",2.5f},{"delay_mix",0.35f},{"reverb_mix",0.35f}});
    add("Pad/Space Choir", {{"osc1_table",0.6f},{"osc2_table",0.75f},{"osc2_semi",5.0f},{"amp_attack",1.2f},{"amp_decay",0.8f},{"amp_sustain",0.9f},{"amp_release",3.0f},{"reverb_mix",0.55f},{"chorus_mix",0.3f}});
    add("Retro/Juno Pad", {{"osc1_table",0.4f},{"unison_voices",4.0f},{"unison_detune",15.0f},{"filter_cutoff",2500.0f},{"amp_attack",0.4f},{"amp_decay",0.6f},{"amp_sustain",0.85f},{"amp_release",1.8f},{"chorus_mix",0.45f},{"reverb_mix",0.3f}});
    add("FX/Riser", {{"osc1_table",0.6f},{"unison_voices",4.0f},{"filter_cutoff",300.0f},{"filter_env",0.9f},{"amp_attack",2.0f},{"amp_sustain",0.8f},{"amp_release",1.0f},{"reverb_mix",0.45f}});
    add("FX/Impact Hit", {{"osc1_fold",0.7f},{"filter_cutoff",150.0f},{"filter_env",1.0f},{"amp_attack",0.001f},{"amp_decay",0.5f},{"amp_sustain",0.0f},{"amp_release",0.8f},{"master_drive",0.7f},{"reverb_mix",0.3f}});
    add("Arp/Glass Arp", {{"osc1_table",0.85f},{"filter_cutoff",9000.0f},{"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.0f},{"amp_release",0.25f},{"delay_mix",0.4f},{"arp_on",1.0f},{"arp_rate",4.0f}});
    add("Salek/Toronowla Core", {{"osc1_fold",0.55f},{"osc2_level",0.8f},{"fm_2to1",0.7f},{"filter_cutoff",2200.0f},{"filter_reso",0.8f},{"filter_env",0.9f},{"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.35f},{"amp_release",0.18f},{"master_drive",0.5f},{"delay_mix",0.25f}});
    add("Salek/Persian Neon Saw", {{"osc1_table",0.48f},{"unison_voices",6.0f},{"unison_detune",40.0f},{"unison_spread",1.0f},{"filter_cutoff",4800.0f},{"amp_attack",0.008f},{"amp_decay",0.28f},{"amp_sustain",0.65f},{"amp_release",0.4f},{"chorus_mix",0.4f},{"delay_mix",0.2f}});
    add("Salek/Final Toronowla", {{"osc1_fold",0.5f},{"osc2_level",0.7f},{"fm_2to1",0.65f},{"filter_cutoff",3500.0f},{"filter_reso",0.55f},{"filter_env",0.8f},{"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.5f},{"amp_release",0.4f},{"master_drive",0.45f},{"delay_mix",0.3f},{"reverb_mix",0.25f}});

    // Iranian traditional-inspired arp / sequence presets
    add("Arp/Santur Cascade", {{"osc1_table",0.55f},{"filter_cutoff",7000.0f},{"amp_attack",0.001f},{"amp_decay",0.18f},{"amp_sustain",0.05f},{"amp_release",0.22f},{"delay_mix",0.35f},{"reverb_mix",0.3f},{"arp_on",1.0f},{"arp_rate",8.0f},{"arp_octaves",2.0f},{"scale_mode",1.0f}});
    add("Arp/Ney Whisper", {{"osc1_table",0.25f},{"filter_cutoff",2500.0f},{"filter_reso",0.35f},{"amp_attack",0.02f},{"amp_decay",0.4f},{"amp_sustain",0.2f},{"amp_release",0.5f},{"reverb_mix",0.45f},{"arp_on",1.0f},{"arp_rate",4.0f},{"arp_octaves",1.0f},{"scale_mode",2.0f}});
    add("Arp/Tar Ostinato", {{"osc1_fold",0.15f},{"filter_cutoff",4000.0f},{"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.1f},{"amp_release",0.2f},{"delay_mix",0.25f},{"arp_on",1.0f},{"arp_rate",6.0f},{"arp_octaves",2.0f},{"scale_mode",3.0f}});
    add("Arp/Setar Pluck", {{"osc1_table",0.4f},{"filter_cutoff",5500.0f},{"amp_attack",0.001f},{"amp_decay",0.28f},{"amp_sustain",0.0f},{"amp_release",0.35f},{"reverb_mix",0.35f},{"arp_on",1.0f},{"arp_rate",5.0f},{"arp_octaves",1.0f},{"scale_mode",4.0f}});
    add("Arp/Daf Pulse", {{"osc1_fold",0.35f},{"filter_cutoff",800.0f},{"filter_env",0.7f},{"amp_attack",0.001f},{"amp_decay",0.12f},{"amp_sustain",0.0f},{"amp_release",0.1f},{"master_drive",0.4f},{"arp_on",1.0f},{"arp_rate",12.0f},{"arp_octaves",1.0f}});

    #include "PluginProcessorPresetsExtra.inl"
}
