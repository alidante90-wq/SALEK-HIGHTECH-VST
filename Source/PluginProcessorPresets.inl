void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };

    add("Init/Init", {});

    // ===== HIGHTECH / HITEK 2026 =====
    add("HIGHTECH/Neon Razor", {
        {"osc1_table",0.62f},{"osc1_level",0.95f},{"osc1_warp",0.18f},{"osc1_fold",0.12f},
        {"osc1_unison",5},{"osc1_udet",16},{"osc1_uspread",0.9f},{"osc1_drive",0.25f},
        {"osc2_level",0.4f},{"osc2_table",0.48f},{"osc2_semi",7},{"fm_2to1",0.28f},
        {"filter_cutoff",4200},{"filter_reso",0.48f},{"filter_env",0.72f},{"filter_drive",0.2f},
        {"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.55f},{"amp_release",0.28f},
        {"lfo_rate",4.2f},{"lfo_amount",0.35f},{"delay_mix",0.28f},{"delay_mode",1},{"delay_fb",0.35f},
        {"reverb_mix",0.18f},{"chorus_mix",0.15f},{"master_drive",0.38f},{"comp_mix",1},{"comp_threshold",-16}
    });
    add("HIGHTECH/Plasma Core", {
        {"osc1_table",0.78f},{"osc1_fold",0.35f},{"osc1_unison",4},{"osc1_udet",12},
        {"osc2_level",0.55f},{"osc2_table",0.2f},{"osc2_octave",-1},{"fm_2to1",0.55f},
        {"filter_cutoff",2800},{"filter_reso",0.7f},{"filter_env",0.88f},
        {"amp_attack",0.001f},{"amp_decay",0.18f},{"amp_sustain",0.4f},{"amp_release",0.2f},
        {"lfo_rate",0.4f},{"lfo_amount",0.5f},{"lfo_wave",7},
        {"dist_mix",0.22f},{"dist_drive",0.35f},{"delay_mix",0.25f},{"master_drive",0.42f}
    });
    add("HIGHTECH/Tehran Night", {
        {"osc1_table",0.55f},{"osc1_warp",0.3f},{"osc1_unison",6},{"osc1_udet",22},{"osc1_uspread",1.0f},
        {"osc2_level",0.35f},{"osc2_table",0.7f},{"osc2_semi",5},{"osc3_level",0.25f},{"osc3_octave",-1},
        {"filter_cutoff",3500},{"filter_reso",0.55f},{"filter_env",0.65f},
        {"amp_attack",0.004f},{"amp_decay",0.3f},{"amp_sustain",0.5f},{"amp_release",0.35f},
        {"chorus_mix",0.28f},{"delay_mix",0.3f},{"reverb_mix",0.25f},{"reverb_size",0.7f},{"master_drive",0.35f}
    });
    add("HITECH/Crystal Grid", {
        {"osc1_table",0.4f},{"osc1_unison",3},{"osc1_udet",8},{"osc1_fold",0.15f},
        {"filter_cutoff",6500},{"filter_reso",0.25f},{"filter_env",0.4f},
        {"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.0f},{"amp_release",0.4f},
        {"reverb_mix",0.45f},{"reverb_size",0.85f},{"delay_mix",0.35f},{"delay_mode",1},{"chorus_mix",0.2f}
    });
    add("HITEK/Wireframe Lead", {
        {"osc1_table",0.88f},{"osc1_drive",0.4f},{"osc1_unison",4},{"osc1_udet",14},
        {"filter_cutoff",5200},{"filter_reso",0.4f},{"filter_env",0.6f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.65f},{"amp_release",0.3f},
        {"phaser_mix",0.2f},{"phaser_rate",0.8f},{"delay_mix",0.22f},{"master_drive",0.4f}
    });

    // ===== PSY / PSYTRANCE / FULLON / DARKPSY =====
    add("PSY/Forest Morning", {
        {"osc1_table",0.22f},{"osc1_unison",5},{"osc1_udet",22},{"osc1_uspread",0.85f},
        {"filter_cutoff",900},{"filter_reso",0.72f},{"filter_env",0.85f},
        {"amp_attack",0.004f},{"amp_decay",0.28f},{"amp_sustain",0.35f},{"amp_release",0.18f},
        {"delay_mix",0.22f},{"reverb_mix",0.18f},{"master_drive",0.28f},{"comp_mix",1}
    });
    add("PSY/Goa Whisper", {
        {"osc1_table",0.35f},{"osc2_level",0.45f},{"fm_2to1",0.4f},
        {"filter_cutoff",1400},{"filter_reso",0.8f},{"filter_env",0.9f},
        {"lfo_rate",0.35f},{"lfo_amount",0.45f},
        {"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.25f},{"amp_release",0.15f},
        {"delay_mix",0.3f},{"delay_mode",1},{"master_drive",0.32f}
    });
    add("PSYTRANCE/Twisted Lead", {
        {"osc1_table",0.55f},{"osc1_fold",0.25f},{"osc1_unison",4},{"osc1_udet",18},
        {"filter_cutoff",3200},{"filter_reso",0.55f},{"filter_env",0.7f},
        {"amp_attack",0.003f},{"amp_decay",0.2f},{"amp_sustain",0.55f},{"amp_release",0.25f},
        {"chorus_mix",0.2f},{"delay_mix",0.28f},{"master_drive",0.4f}
    });
    add("PSYTRANCE/Squelch Acid", {
        {"osc1_table",0.7f},{"filter_cutoff",380},{"filter_reso",0.94f},{"filter_drive",0.6f},{"filter_env",1.0f},
        {"amp_attack",0.001f},{"amp_decay",0.16f},{"amp_sustain",0.08f},{"amp_release",0.1f},
        {"master_drive",0.45f},{"dist_mix",0.15f},{"lfo_rate",6.0f},{"lfo_amount",0.4f}
    });
    add("PSYTRANCE/Night Ride", {
        {"osc1_table",0.42f},{"osc2_table",0.6f},{"osc2_level",0.35f},{"fm_2to1",0.35f},
        {"filter_cutoff",2200},{"filter_reso",0.65f},{"lfo_rate",5.5f},{"lfo_amount",0.55f},
        {"amp_attack",0.005f},{"amp_decay",0.3f},{"amp_sustain",0.4f},{"amp_release",0.22f},
        {"delay_mix",0.25f},{"reverb_mix",0.2f}
    });
    add("FULLON/Peak Hour", {
        {"osc1_table",0.5f},{"osc1_unison",6},{"osc1_udet",28},{"osc1_uspread",1.0f},{"osc1_drive",0.3f},
        {"filter_cutoff",2800},{"filter_reso",0.6f},{"filter_env",0.75f},
        {"amp_attack",0.002f},{"amp_decay",0.18f},{"amp_sustain",0.45f},{"amp_release",0.15f},
        {"bassify",0.35f},{"delay_mix",0.2f},{"master_drive",0.48f},{"comp_threshold",-14},{"comp_mix",1}
    });
    add("FULLON/Stadium Kickstab", {
        {"osc1_table",0.15f},{"osc1_level",1.0f},{"osc1_octave",-1},{"osc1_drive",0.5f},
        {"filter_cutoff",180},{"filter_reso",0.3f},{"filter_env",0.5f},
        {"amp_attack",0.001f},{"amp_decay",0.12f},{"amp_sustain",0.2f},{"amp_release",0.08f},
        {"bassify",0.55f},{"comp_mix",1},{"comp_threshold",-10},{"comp_ratio",6},{"master_drive",0.5f}
    });
    add("DARKPSY/Void Crawl", {
        {"osc1_table",0.85f},{"osc1_fold",0.4f},{"osc1_warp",0.25f},{"osc2_level",0.5f},{"fm_2to1",0.6f},
        {"filter_cutoff",600},{"filter_reso",0.85f},{"filter_env",0.95f},
        {"amp_attack",0.01f},{"amp_decay",0.4f},{"amp_sustain",0.3f},{"amp_release",0.5f},
        {"reverb_mix",0.4f},{"reverb_size",0.9f},{"delay_mix",0.35f},{"dist_mix",0.3f},{"master_drive",0.4f}
    });
    add("DARKPSY/Ritual Bass", {
        {"osc1_table",0.1f},{"osc1_octave",-1},{"osc1_unison",4},{"osc1_udet",25},
        {"filter_cutoff",220},{"filter_reso",0.5f},{"filter_drive",0.4f},
        {"amp_attack",0.02f},{"amp_decay",0.35f},{"amp_sustain",0.7f},{"amp_release",0.3f},
        {"bassify",0.4f},{"comp_mix",1},{"master_drive",0.45f}
    });

    // ===== TECHNO / HARD =====
    add("TECHNO/Berlin Basement", {
        {"osc1_table",0.05f},{"osc1_level",1.0f},{"filter_cutoff",280},{"filter_reso",0.3f},
        {"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.6f},{"amp_release",0.15f},
        {"dist_mix",0.2f},{"dist_mode",1},{"master_drive",0.4f},{"comp_mix",1},{"comp_threshold",-18}
    });
    add("TECHNO/Industrial Pulse", {
        {"osc1_table",0.9f},{"osc1_drive",0.55f},{"osc1_unison",2},
        {"filter_cutoff",800},{"filter_reso",0.4f},{"filter_env",0.55f},
        {"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.15f},{"amp_release",0.1f},
        {"dist_mix",0.4f},{"dist_crush",0.25f},{"delay_mix",0.15f}
    });
    add("HARD TECHNO/Rave Saw", {
        {"osc1_table",0.5f},{"osc1_unison",7},{"osc1_udet",32},{"osc1_uspread",1.0f},{"osc1_drive",0.35f},
        {"filter_cutoff",4500},{"filter_reso",0.5f},{"filter_env",0.8f},
        {"amp_attack",0.001f},{"amp_decay",0.15f},{"amp_sustain",0.5f},{"amp_release",0.12f},
        {"dist_mix",0.25f},{"master_drive",0.5f},{"comp_mix",1}
    });

    // ===== BASS =====
    add("BASS/Sub Pressure", {
        {"osc1_table",0.0f},{"osc1_octave",-2},{"osc1_level",1.0f},
        {"filter_cutoff",120},{"filter_reso",0.15f},
        {"amp_attack",0.01f},{"amp_decay",0.3f},{"amp_sustain",0.85f},{"amp_release",0.25f},
        {"bassify",0.6f},{"comp_mix",1},{"comp_threshold",-12},{"master_drive",0.3f}
    });
    add("BASS/Reese Wide", {
        {"osc1_table",0.3f},{"osc1_unison",6},{"osc1_udet",30},{"osc1_uspread",1.0f},{"osc1_octave",-1},
        {"filter_cutoff",750},{"amp_attack",0.04f},{"amp_decay",0.45f},{"amp_sustain",0.75f},{"amp_release",0.4f},
        {"chorus_mix",0.25f},{"master_drive",0.3f}
    });
    add("BASS/Hoover 2026", {
        {"osc1_table",0.45f},{"osc1_unison",8},{"osc1_udet",35},{"osc1_uspread",0.95f},
        {"osc2_level",0.4f},{"osc2_semi",7},{"osc2_table",0.5f},
        {"filter_cutoff",1200},{"filter_reso",0.45f},{"filter_env",0.5f},
        {"amp_attack",0.005f},{"amp_decay",0.4f},{"amp_sustain",0.7f},{"amp_release",0.35f},
        {"chorus_mix",0.35f},{"master_drive",0.38f}
    });

    // ===== LEAD / PLUCK / PAD / ARP =====
    add("LEAD/Hi-Tech Solo", {
        {"osc1_table",0.6f},{"osc1_fold",0.2f},{"osc1_unison",3},{"osc1_udet",10},
        {"filter_cutoff",5500},{"filter_reso",0.4f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.6f},{"amp_release",0.3f},
        {"delay_mix",0.3f},{"delay_mode",1},{"reverb_mix",0.2f}
    });
    add("LEAD/FM Splitter", {
        {"osc1_table",0.5f},{"osc2_level",0.7f},{"osc2_table",0.3f},{"fm_2to1",0.75f},{"fm_3to1",0.2f},
        {"osc3_level",0.3f},{"filter_cutoff",4000},{"filter_reso",0.35f},{"filter_env",0.55f},
        {"amp_attack",0.002f},{"amp_decay",0.2f},{"amp_sustain",0.45f},{"amp_release",0.25f},
        {"delay_mix",0.25f},{"master_drive",0.35f}
    });
    add("PLUCK/Crystal Drop", {
        {"osc1_table",0.7f},{"filter_cutoff",6000},{"filter_reso",0.3f},
        {"amp_attack",0.001f},{"amp_decay",0.28f},{"amp_sustain",0.0f},{"amp_release",0.25f},
        {"reverb_mix",0.4f},{"delay_mix",0.2f}
    });
    add("PLUCK/Zap Sequence", {
        {"osc1_table",0.95f},{"osc1_drive",0.3f},{"filter_cutoff",8000},{"filter_reso",0.2f},{"filter_env",0.7f},
        {"amp_attack",0.001f},{"amp_decay",0.12f},{"amp_sustain",0.0f},{"amp_release",0.08f},
        {"delay_mix",0.4f},{"delay_mode",1},{"delay_fb",0.45f}
    });
    add("PAD/Starfield", {
        {"osc1_table",0.25f},{"osc1_unison",5},{"osc1_udet",20},{"osc2_level",0.4f},{"osc2_table",0.4f},{"osc2_semi",7},
        {"filter_cutoff",2500},{"amp_attack",0.4f},{"amp_decay",0.6f},{"amp_sustain",0.85f},{"amp_release",1.2f},
        {"chorus_mix",0.4f},{"reverb_mix",0.5f},{"reverb_size",0.8f}
    });
    add("PAD/Iranian Dawn", {
        {"osc1_table",0.4f},{"osc2_level",0.35f},{"osc2_semi",5},{"filter_cutoff",3000},
        {"amp_attack",0.25f},{"amp_decay",0.5f},{"amp_sustain",0.8f},{"amp_release",0.9f},
        {"reverb_mix",0.45f},{"delay_mix",0.2f},{"chorus_mix",0.25f}
    });
    add("ARP/Space Laser", {
        {"osc1_table",0.65f},{"osc1_unison",3},{"filter_cutoff",5000},{"filter_reso",0.35f},{"filter_env",0.6f},
        {"amp_attack",0.001f},{"amp_decay",0.15f},{"amp_sustain",0.2f},{"amp_release",0.1f},
        {"arp_on",1},{"delay_mix",0.35f},{"delay_mode",1},{"reverb_mix",0.2f}
    });

    // ===== RETRO / FX / SALEK =====
    add("RETRO/80s Brass", {
        {"osc1_table",0.2f},{"osc1_unison",4},{"osc1_udet",12},{"osc2_level",0.45f},{"osc2_semi",7},
        {"filter_cutoff",2800},{"filter_reso",0.25f},
        {"amp_attack",0.08f},{"amp_decay",0.35f},{"amp_sustain",0.7f},{"amp_release",0.4f},
        {"chorus_mix",0.45f},{"reverb_mix",0.3f}
    });
    add("FX/Alien Sweep", {
        {"osc1_table",0.9f},{"osc1_warp",0.5f},{"noise",0.3f},
        {"filter_cutoff",200},{"filter_reso",0.9f},{"filter_env",1.0f},
        {"amp_attack",0.5f},{"amp_decay",1.0f},{"amp_sustain",0.5f},{"amp_release",1.5f},
        {"lfo_rate",0.15f},{"lfo_amount",0.8f},{"lfo_wave",1},
        {"reverb_mix",0.55f},{"reverb_size",0.95f},{"delay_mix",0.4f}
    });
    add("FX/Glitch Stutter", {
        {"osc1_table",0.75f},{"osc1_fold",0.5f},{"filter_cutoff",3000},{"filter_reso",0.6f},
        {"amp_attack",0.001f},{"amp_decay",0.05f},{"amp_sustain",0.0f},{"amp_release",0.05f},
        {"dist_mix",0.35f},{"dist_crush",0.4f},{"delay_mix",0.5f},{"delay_fb",0.55f},{"delay_mode",1}
    });
    add("SALEK/Persian Glass", {
        {"osc1_table",0.67f},{"osc1_warp",0.19f},{"osc2_level",0.63f},{"osc2_table",0.16f},{"osc3_table",0.66f},
        {"filter_cutoff",5100},{"filter_reso",0.0f},{"filter_env",0.4f},
        {"amp_attack",0.01f},{"amp_decay",0.07f},{"amp_sustain",0.32f},{"amp_release",0.04f},
        {"chorus_mix",0.25f},{"delay_mix",0.2f},{"reverb_mix",0.25f}
    });
    add("SALEK/Toronowla Night", {
        {"osc1_fold",0.45f},{"osc2_level",0.6f},{"fm_2to1",0.55f},
        {"filter_cutoff",3400},{"filter_reso",0.5f},{"filter_env",0.75f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.5f},{"amp_release",0.35f},
        {"delay_mix",0.28f},{"reverb_mix",0.25f},{"master_drive",0.4f}
    });
    add("SALEK/Isatis Chrome", {
        {"osc1_table",0.5f},{"osc1_unison",5},{"osc1_udet",22},
        {"filter_cutoff",4800},{"amp_attack",0.008f},{"amp_decay",0.3f},{"amp_sustain",0.6f},{"amp_release",0.4f},
        {"chorus_mix",0.35f},{"delay_mix",0.22f},{"phaser_mix",0.15f}
    });
    add("SALEK/Milad Tower", {
        {"osc1_table",0.58f},{"osc1_unison",4},{"osc1_udet",15},{"osc2_level",0.4f},{"osc2_semi",12},
        {"filter_cutoff",3800},{"filter_reso",0.42f},{"filter_env",0.6f},
        {"amp_attack",0.005f},{"amp_decay",0.28f},{"amp_sustain",0.55f},{"amp_release",0.4f},
        {"delay_mix",0.32f},{"reverb_mix",0.35f},{"chorus_mix",0.2f},{"master_drive",0.36f}
    });
}
