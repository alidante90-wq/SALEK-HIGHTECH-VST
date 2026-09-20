void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };

    // ---------- INIT ----------
    add("Init/Init", {});

    // ---------- PSY / PSYTRANCE ----------
    add("PSY/Forest Morning", {{"osc1_table",0.22f},{"osc1_unison",5},{"osc1_udet",22},{"osc1_uspread",0.85f},{"filter_cutoff",900},{"filter_reso",0.72f},{"filter_env",0.85f},{"amp_attack",0.004f},{"amp_decay",0.28f},{"amp_sustain",0.35f},{"amp_release",0.18f},{"delay_mix",0.22f},{"reverb_mix",0.18f},{"master_drive",0.28f}});
    add("PSY/Goa Whisper", {{"osc1_table",0.35f},{"osc2_level",0.45f},{"fm_2to1",0.4f},{"filter_cutoff",1400},{"filter_reso",0.8f},{"filter_env",0.9f},{"lfo_rate",0.35f},{"lfo_amount",0.45f},{"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.25f},{"amp_release",0.15f},{"delay_mix",0.3f},{"delay_mode",1},{"master_drive",0.32f}});
    add("PSYTRANCE/Twisted Lead", {{"osc1_table",0.55f},{"osc1_fold",0.25f},{"osc1_unison",4},{"osc1_udet",18},{"filter_cutoff",3200},{"filter_reso",0.55f},{"filter_env",0.7f},{"amp_attack",0.003f},{"amp_decay",0.2f},{"amp_sustain",0.55f},{"amp_release",0.25f},{"chorus_mix",0.2f},{"delay_mix",0.28f},{"master_drive",0.4f}});
    add("PSYTRANCE/Squelch Acid", {{"osc1_table",0.7f},{"filter_cutoff",380},{"filter_reso",0.94f},{"filter_drive",0.6f},{"filter_env",1.0f},{"amp_attack",0.001f},{"amp_decay",0.16f},{"amp_sustain",0.08f},{"amp_release",0.1f},{"master_drive",0.45f},{"dist_mix",0.15f}});
    add("PSYTRANCE/Night Ride", {{"osc1_table",0.42f},{"osc2_table",0.6f},{"osc2_level",0.35f},{"fm_2to1",0.35f},{"filter_cutoff",2200},{"filter_reso",0.65f},{"lfo_rate",5.5f},{"lfo_amount",0.55f},{"amp_attack",0.005f},{"amp_decay",0.3f},{"amp_sustain",0.4f},{"amp_release",0.22f},{"delay_mix",0.25f},{"reverb_mix",0.2f}});

    // ---------- FULLON ----------
    add("FULLON/Peak Hour", {{"osc1_table",0.5f},{"osc1_unison",6},{"osc1_udet",28},{"osc1_uspread",1.0f},{"filter_cutoff",2800},{"filter_reso",0.6f},{"filter_env",0.75f},{"amp_attack",0.002f},{"amp_decay",0.18f},{"amp_sustain",0.45f},{"amp_release",0.15f},{"bassify",0.35f},{"master_drive",0.5f},{"delay_mix",0.2f}});
    add("FULLON/Rolling Bass", {{"osc1_table",0.12f},{"osc1_level",1.0f},{"osc3_octave",-1},{"osc3_level",0.7f},{"filter_cutoff",550},{"filter_reso",0.45f},{"filter_env",0.55f},{"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.7f},{"amp_release",0.2f},{"bassify",0.5f},{"master_drive",0.35f}});
    add("FULLON/Laser Stab", {{"osc1_fold",0.45f},{"osc1_drive",0.4f},{"filter_cutoff",4500},{"filter_reso",0.5f},{"filter_env",0.95f},{"amp_attack",0.001f},{"amp_decay",0.12f},{"amp_sustain",0.0f},{"amp_release",0.1f},{"master_drive",0.55f},{"delay_mix",0.15f}});

    // ---------- DARKPSY ----------
    add("DARKPSY/Abyss", {{"osc1_table",0.18f},{"osc2_level",0.55f},{"fm_3to1",0.55f},{"filter_cutoff",700},{"filter_reso",0.78f},{"filter_env",0.85f},{"amp_attack",0.01f},{"amp_decay",0.4f},{"amp_sustain",0.35f},{"amp_release",0.35f},{"reverb_mix",0.35f},{"delay_mix",0.25f},{"master_drive",0.4f},{"dist_mix",0.2f}});
    add("DARKPSY/Ritual", {{"osc1_table",0.08f},{"osc1_fold",0.2f},{"filter_cutoff",450},{"filter_reso",0.85f},{"lfo_rate",0.15f},{"lfo_amount",0.6f},{"amp_attack",0.02f},{"amp_decay",0.5f},{"amp_sustain",0.5f},{"amp_release",0.6f},{"reverb_mix",0.45f},{"reverb_size",0.75f},{"master_drive",0.3f}});
    add("DARKPSY/Horror Reese", {{"osc1_table",0.28f},{"osc1_unison",7},{"osc1_udet",35},{"osc1_uspread",1.0f},{"filter_cutoff",600},{"filter_reso",0.55f},{"amp_attack",0.08f},{"amp_decay",0.5f},{"amp_sustain",0.8f},{"amp_release",0.5f},{"chorus_mix",0.3f},{"master_drive",0.35f}});

    // ---------- HIGHTECH / HITECH / HITEK ----------
    add("HIGHTECH/Crystal Edge", {{"osc1_table",0.75f},{"osc1_fold",0.35f},{"osc1_unison",5},{"osc1_udet",15},{"filter_cutoff",6500},{"filter_reso",0.4f},{"filter_env",0.6f},{"amp_attack",0.002f},{"amp_decay",0.2f},{"amp_sustain",0.5f},{"amp_release",0.25f},{"delay_mix",0.3f},{"delay_mode",1},{"chorus_mix",0.25f},{"master_drive",0.35f}});
    add("HIGHTECH/Glitch Blade", {{"osc1_table",0.88f},{"osc1_warp",0.4f},{"osc2_level",0.4f},{"fm_2to1",0.5f},{"filter_cutoff",5000},{"filter_reso",0.55f},{"amp_attack",0.001f},{"amp_decay",0.15f},{"amp_sustain",0.3f},{"amp_release",0.12f},{"dist_mix",0.25f},{"dist_mode",3},{"delay_mix",0.22f},{"master_drive",0.45f}});
    add("HITECH/Neon Pulse", {{"osc1_table",0.62f},{"osc1_drive",0.3f},{"filter_cutoff",4000},{"filter_reso",0.5f},{"lfo_rate",8.0f},{"lfo_amount",0.4f},{"amp_attack",0.002f},{"amp_decay",0.18f},{"amp_sustain",0.4f},{"amp_release",0.15f},{"phaser_mix",0.25f},{"delay_mix",0.28f},{"master_drive",0.38f}});
    add("HITEK/Shredder", {{"osc1_fold",0.55f},{"osc1_table",0.9f},{"filter_cutoff",3800},{"filter_reso",0.65f},{"filter_drive",0.45f},{"filter_env",0.8f},{"amp_attack",0.001f},{"amp_decay",0.14f},{"amp_sustain",0.2f},{"amp_release",0.1f},{"dist_mix",0.35f},{"dist_drive",0.5f},{"master_drive",0.55f}});
    add("HITEK/Wireframe", {{"osc1_table",0.95f},{"osc1_warp",0.55f},{"osc1_unison",3},{"osc1_udet",10},{"filter_cutoff",7200},{"filter_reso",0.35f},{"amp_attack",0.001f},{"amp_decay",0.25f},{"amp_sustain",0.35f},{"amp_release",0.3f},{"delay_mix",0.35f},{"reverb_mix",0.2f},{"master_drive",0.3f}});

    // ---------- TECHNO / HARD TECHNO ----------
    add("TECHNO/Berlin Basement", {{"osc1_table",0.05f},{"osc1_level",1.0f},{"filter_cutoff",280},{"filter_reso",0.3f},{"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.6f},{"amp_release",0.15f},{"dist_mix",0.2f},{"dist_mode",1},{"master_drive",0.4f},{"comp_mix",1},{"comp_threshold",-18}});
    add("TECHNO/Warehouse Stab", {{"osc1_table",0.4f},{"osc1_fold",0.2f},{"filter_cutoff",1800},{"filter_reso",0.45f},{"filter_env",0.7f},{"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.1f},{"amp_release",0.12f},{"master_drive",0.45f},{"delay_mix",0.15f}});
    add("HARDTECHNO/Industrial Kick Lead", {{"osc1_fold",0.6f},{"osc1_drive",0.55f},{"filter_cutoff",900},{"filter_reso",0.4f},{"filter_env",0.9f},{"amp_attack",0.001f},{"amp_decay",0.15f},{"amp_sustain",0.0f},{"amp_release",0.1f},{"dist_mix",0.45f},{"dist_mode",2},{"master_drive",0.65f},{"bassify",0.4f}});
    add("HARDTECHNO/Acid Hoover", {{"osc1_table",0.65f},{"filter_cutoff",500},{"filter_reso",0.9f},{"filter_drive",0.5f},{"filter_env",0.95f},{"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.15f},{"amp_release",0.12f},{"dist_mix",0.3f},{"master_drive",0.5f}});
    add("HARDTECHNO/Rave Siren", {{"osc1_table",0.8f},{"osc1_unison",4},{"osc1_udet",12},{"filter_cutoff",5500},{"filter_reso",0.55f},{"lfo_rate",6.0f},{"lfo_amount",0.7f},{"amp_attack",0.002f},{"amp_decay",0.25f},{"amp_sustain",0.45f},{"amp_release",0.2f},{"delay_mix",0.25f},{"master_drive",0.45f}});

    // ---------- RETRO / FAMOUS ----------
    add("RETRO/JP8 Brass", {{"osc1_table",0.2f},{"osc2_table",0.25f},{"osc2_level",0.5f},{"osc2_detune",7},{"filter_cutoff",2200},{"filter_reso",0.25f},{"amp_attack",0.02f},{"amp_decay",0.4f},{"amp_sustain",0.7f},{"amp_release",0.5f},{"chorus_mix",0.35f},{"reverb_mix",0.25f}});
    add("RETRO/DX Electric", {{"osc1_level",0.85f},{"osc2_level",0.7f},{"osc2_octave",1},{"fm_2to1",0.75f},{"filter_cutoff",5000},{"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.2f},{"amp_release",0.25f},{"reverb_mix",0.2f}});
    add("RETRO/303 Classic", {{"osc1_table",0.85f},{"filter_cutoff",450},{"filter_reso",0.92f},{"filter_drive",0.4f},{"filter_env",0.9f},{"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.1f},{"amp_release",0.1f},{"master_drive",0.35f}});
    add("RETRO/SuperSaw Anthem", {{"osc1_table",0.48f},{"osc1_unison",7},{"osc1_udet",32},{"osc1_uspread",1.0f},{"filter_cutoff",4200},{"filter_reso",0.3f},{"amp_attack",0.01f},{"amp_decay",0.3f},{"amp_sustain",0.7f},{"amp_release",0.45f},{"chorus_mix",0.4f},{"delay_mix",0.25f},{"reverb_mix",0.3f}});
    add("RETRO/Hoover Classic", {{"osc1_table",0.55f},{"osc1_unison",5},{"osc1_udet",25},{"filter_cutoff",1800},{"filter_reso",0.5f},{"filter_env",0.6f},{"amp_attack",0.005f},{"amp_decay",0.35f},{"amp_sustain",0.5f},{"amp_release",0.3f},{"chorus_mix",0.3f},{"master_drive",0.4f}});

    // ---------- BASS ----------
    add("BASS/Sub Foundation", {{"osc1_table",0.0f},{"osc1_level",1.0f},{"sub_level",0.7f},{"filter_cutoff",180},{"filter_reso",0.15f},{"amp_attack",0.005f},{"amp_decay",0.4f},{"amp_sustain",0.85f},{"amp_release",0.3f},{"bassify",0.4f}});
    add("BASS/Neuro Growl", {{"osc1_table",0.35f},{"osc1_fold",0.3f},{"osc2_level",0.5f},{"fm_2to1",0.55f},{"filter_cutoff",800},{"filter_reso",0.7f},{"lfo_rate",4.0f},{"lfo_amount",0.65f},{"amp_attack",0.004f},{"amp_decay",0.3f},{"amp_sustain",0.5f},{"amp_release",0.2f},{"dist_mix",0.25f},{"master_drive",0.4f}});
    add("BASS/Reese Wide", {{"osc1_table",0.3f},{"osc1_unison",6},{"osc1_udet",30},{"osc1_uspread",1.0f},{"filter_cutoff",750},{"amp_attack",0.04f},{"amp_decay",0.45f},{"amp_sustain",0.75f},{"amp_release",0.4f},{"chorus_mix",0.25f},{"master_drive",0.3f}});

    // ---------- LEAD / PLUCK / PAD ----------
    add("LEAD/Hi-Tech Solo", {{"osc1_table",0.6f},{"osc1_fold",0.2f},{"osc1_unison",3},{"filter_cutoff",5500},{"filter_reso",0.4f},{"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.6f},{"amp_release",0.3f},{"delay_mix",0.3f},{"delay_mode",1},{"reverb_mix",0.2f}});
    add("PLUCK/Crystal Drop", {{"osc1_table",0.7f},{"filter_cutoff",6000},{"filter_reso",0.3f},{"amp_attack",0.001f},{"amp_decay",0.28f},{"amp_sustain",0.0f},{"amp_release",0.25f},{"reverb_mix",0.4f},{"delay_mix",0.2f}});
    add("PAD/Starfield", {{"osc1_table",0.25f},{"osc1_unison",5},{"osc1_udet",20},{"osc2_level",0.4f},{"osc2_table",0.4f},{"filter_cutoff",2500},{"amp_attack",0.4f},{"amp_decay",0.6f},{"amp_sustain",0.85f},{"amp_release",1.2f},{"chorus_mix",0.4f},{"reverb_mix",0.5f},{"reverb_size",0.8f}});
    add("PAD/Iranian Dawn", {{"osc1_table",0.4f},{"osc2_level",0.35f},{"filter_cutoff",3000},{"amp_attack",0.25f},{"amp_decay",0.5f},{"amp_sustain",0.8f},{"amp_release",0.9f},{"reverb_mix",0.45f},{"delay_mix",0.2f},{"chorus_mix",0.25f},{"scale_mode",1}});

    // ---------- SALEK / SIGNATURE ----------
    add("SALEK/Persian Glass", {{"osc1_table",0.67f},{"osc1_warp",0.19f},{"osc2_level",0.63f},{"osc2_table",0.16f},{"osc3_table",0.66f},{"filter_cutoff",5100},{"filter_reso",0.0f},{"filter_env",0.4f},{"amp_attack",0.01f},{"amp_decay",0.07f},{"amp_sustain",0.32f},{"amp_release",0.04f},{"chorus_mix",0.25f},{"delay_mix",0.2f},{"reverb_mix",0.25f}});
    add("SALEK/Toronowla Night", {{"osc1_fold",0.45f},{"osc2_level",0.6f},{"fm_2to1",0.55f},{"filter_cutoff",3400},{"filter_reso",0.5f},{"filter_env",0.75f},{"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.5f},{"amp_release",0.35f},{"delay_mix",0.28f},{"reverb_mix",0.25f},{"master_drive",0.4f}});
    add("SALEK/Isatis Chrome", {{"osc1_table",0.5f},{"osc1_unison",5},{"osc1_udet",22},{"filter_cutoff",4800},{"amp_attack",0.008f},{"amp_decay",0.3f},{"amp_sustain",0.6f},{"amp_release",0.4f},{"chorus_mix",0.35f},{"delay_mix",0.22f},{"phaser_mix",0.15f}});
}
