#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"

namespace giti::lore
{
inline juce::String universeEn()
{
    return
        "UNIVERSE / GITI\n"
        "The SALEK Universe is the parent architecture: space, time, matter, energy, machines, instruments and the stories they generate.\n\n"
        "GITI is its First Generation: fifty numbered sonic machines. Each unit is an independent sonic organism with its own frequency strategy, modulation behaviour, visual identity and mission.\n\n"
        "GITI was engineered to create precise changes at carefully selected frequency regions. A small controlled change may propagate through the system like a musical Butterfly Effect. The purpose is intentional transformation.\n\n"
        "THE GITI DIRECTIVE\n"
        "OBEY THE COMMANDER. ESTABLISH PEACE.\n"
        "NO BULLET. NO FEAR. NO ANXIETY.\n"
        "ONE SIGNAL. ONE SYNTH.\n\n"
        "GITI exists to make the next generation better and more complete. When its successor is ready to cross from the digital world into the physical world, GITI completes its mission and shuts down. The shutdown is not failure. It is successful succession.";
}

inline juce::String universeFa()
{
    return
        "کیهان / گیتی\n"
        "کیهان، معماری مادر جهان سالک است؛ مجموعه‌ای از فضا، زمان، ماده، انرژی، ماشین‌ها، سازها و داستان‌هایی که از آن‌ها متولد می‌شوند.\n\n"
        "گیتی نخستین نسل این معماری است: پنجاه ماشین صوتی مستقل، محدود و شماره‌دار. هر گیتی یک موجود صوتی با راهبرد فرکانسی، رفتار مدولاسیون، هویت بصری و مأموریت مخصوص خود است.\n\n"
        "گیتی برای ایجاد تغییرات دقیق در نواحی مشخص فرکانسی مهندسی شده است. یک تغییر کوچک و کنترل‌شده می‌تواند مانند اثر پروانه‌ای در کل سیستم گسترش پیدا کند. هدف، تولید صدا صرفاً برای تولید صدا نیست؛ هدف، ایجاد تغییر آگاهانه است.\n\n"
        "فرمان اصلی:\n"
        "اطاعت از فرمانده. برقراری صلح.\n"
        "بدون گلوله. بدون ترس. بدون اضطراب.\n"
        "یک سیگنال. یک سینت.\n\n"
        "گیتی ساخته شده تا نسل بعدی را بهتر و کامل‌تر کند. وقتی نسل بعدی آماده شود تا از دنیای دیجیتال به دنیای حقیقی عبور کند، گیتی مأموریت خود را به پایان می‌رساند و خاموش می‌شود. این خاموشی شکست نیست؛ نشانه‌ی موفقیت در انتقال نسل است.";
}

inline juce::String editionEn(const Edition& e)
{
    return juce::String(e.code) + " / " + e.name + "\n"
         + "Element: " + e.element + "\n"
         + "Archetype: " + e.archetype + "\n"
         + "Nominal BPM: " + juce::String(e.bpm) + "\n"
         + "Palette: " + e.palette;
}

inline juce::String editionFa(const Edition& e)
{
    return "نسخه " + juce::String(e.id) + " / " + e.name + "\n"
         + "عنصر: " + e.element
         + "\nالگوی صوتی: " + e.archetype
         + "\nBPM مرجع: " + juce::String(e.bpm);
}
}
