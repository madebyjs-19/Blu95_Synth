#include "InfoText.h"
#include "Parameters.h"

namespace blu
{
namespace
{
    struct Text
    {
        const char* key;
        const char* title;
        const char* ko;
        const char* en;
    };

    const Text texts[] = {
        { "algorithm", "ALGORITHM",
          u8"4개의 오퍼레이터(사인파 발진기)를 어떻게 연결할지 정합니다.\n\n"
          u8"• 캐리어(청록색): 실제로 소리가 나는 오퍼레이터. 아래쪽 OUT으로 이어집니다.\n"
          u8"• 모듈레이터(적갈색): 직접 소리는 안 나지만, 아래에 연결된 오퍼레이터의 주파수를 흔들어 음색을 만듭니다.\n\n"
          u8"다이어그램을 클릭해도 알고리즘이 바뀝니다.",
          "Chooses how the four operators (sine oscillators) are wired together.\n\n"
          "• Carriers (teal) are what you hear - they connect to OUT at the bottom.\n"
          "• Modulators (maroon) make no sound themselves; they wobble the frequency of the operator below them to shape the tone.\n\n"
          "Click the diagram to step through algorithms." },

        { "feedback", "FEEDBACK",
          u8"OP4가 자기 자신의 출력으로 자기를 변조합니다(다이어그램의 OP4 위 고리).\n"
          u8"사인파 하나만으로 톱니파 같은 밝은 소리를 만들 수 있어요. 너무 높이면 노이즈처럼 거칠어집니다.",
          "OP4 modulates itself with its own output (the loop above OP4 in the diagram).\n"
          "A single sine can become a bright saw-like wave. Push it too far and it turns into noise." },

        { "volume", "VOLUME",
          u8"전체 출력 음량입니다.",
          "Master output level." },

        { "velBright", "VELOCITY > BRIGHTNESS",
          u8"건반을 세게 칠수록 모듈레이터 레벨이 커져서 소리가 밝아집니다.\n"
          u8"0이면 세기와 상관없이 같은 음색이에요. 어쿠스틱 악기 같은 표현력을 줍니다.",
          "The harder you play, the higher the modulator levels, so the sound gets brighter.\n"
          "At 0 every note has the same tone. This is what makes FM feel expressive, like an acoustic instrument." },

        { "velVolume", "VELOCITY > VOLUME",
          u8"건반 세기에 따라 캐리어 음량이 변하는 정도입니다. 0이면 항상 같은 크기로 소리납니다.",
          "How much the carrier volume follows how hard you play. At 0 every note has the same loudness." },

        { "on", "OPERATOR ON / OFF",
          u8"DX7처럼 오퍼레이터를 켜고 끌 수 있습니다.\n"
          u8"모듈레이터를 껐다 켜보면 그 오퍼레이터가 음색에서 어떤 역할을 하는지 바로 들을 수 있어요.",
          "Switch an operator on or off, just like on the DX7.\n"
          "Toggle a modulator to hear exactly what it adds to the tone." },

        { "coarse", "RATIO",
          u8"이 오퍼레이터의 주파수 = 누른 건반의 주파수 × RATIO\n\n"
          u8"• 캐리어: 음높이를 정합니다 (2 = 한 옥타브 위).\n"
          u8"• 모듈레이터: 캐리어와의 비율(C:M)이 배음이 생기는 자리를 정합니다.\n"
          u8"   정수 비율 → 악기 같은 화성적인 소리\n"
          u8"   비정수 비율 → 종, 금속 같은 소리",
          "This operator's frequency = the key's frequency x RATIO\n\n"
          "• Carrier: sets the pitch (2 = one octave up).\n"
          "• Modulator: its ratio to the carrier (C:M) decides where the harmonics appear.\n"
          "   Whole-number ratio > harmonic, instrument-like\n"
          "   Fractional ratio > bell-like, metallic" },

        { "fine", "FINE",
          u8"RATIO에 소수점 값을 더합니다 (예: 3 + .50 = 3.50).\n"
          u8"비정수 비율로 종소리나 금속성 소리를 만들 때 사용하세요.",
          "Adds a fraction to RATIO (e.g. 3 + .50 = 3.50).\n"
          "Use it to get fractional ratios for bells and metallic tones." },

        { "detune", "DETUNE",
          u8"음정을 아주 미세하게 어긋나게 합니다 (한 칸에 약 2센트).\n"
          u8"두 캐리어를 서로 반대 방향으로 디튠하면 코러스처럼 풍성해집니다.",
          "Shifts the pitch very slightly (about 2 cents per step).\n"
          "Detune two carriers in opposite directions for a rich, chorus-like sound." },

        { "level", "LEVEL",
          u8"오퍼레이터의 출력 크기입니다.\n\n"
          u8"• 캐리어라면: 그 소리의 음량\n"
          u8"• 모듈레이터라면: 변조 깊이(Modulation Index). 클수록 배음이 많아지고 소리가 밝아집니다.\n\n"
          u8"FM 사운드 디자인에서 가장 중요한 손잡이예요!",
          "The output amount of the operator.\n\n"
          "• On a carrier: the volume of that sound.\n"
          "• On a modulator: the modulation depth (modulation index). More index = more harmonics = brighter.\n\n"
          "This is the most important knob in FM sound design!" },

        { "attack", "ATTACK",
          u8"건반을 누른 뒤 최대 레벨까지 올라가는 시간입니다.\n"
          u8"모듈레이터에 쓰면 음색이 점점 밝아지고, 캐리어에 쓰면 음량이 서서히 커집니다.",
          "Time to rise to full level after a key is pressed.\n"
          "On a modulator the tone gets brighter over time; on a carrier the volume fades in." },

        { "decay", "DECAY",
          u8"최대 레벨에서 SUSTAIN 레벨까지 내려가는 시간입니다.\n"
          u8"모듈레이터의 짧은 DECAY는 '퉁', '틱' 같은 어택 소리를 만듭니다.",
          "Time to fall from full level to the SUSTAIN level.\n"
          "A short DECAY on a modulator creates plucky 'tick' or 'thump' attacks." },

        { "sustain", "SUSTAIN",
          u8"건반을 누르고 있는 동안 유지되는 레벨(%)입니다.\n"
          u8"캐리어의 SUSTAIN이 0이면 피아노나 타악기처럼 소리가 저절로 사라집니다.",
          "The level (%) held while the key stays down.\n"
          "With a carrier SUSTAIN of 0 the sound dies away by itself, like a piano or percussion." },

        { "release", "RELEASE",
          u8"건반에서 손을 뗀 뒤 소리가 사라지는 시간입니다.",
          "Time for the sound to fade out after the key is released." },

        { "preset", "PRESET / LESSON",
          u8"프리셋 1~12번은 순서대로 따라하는 FM 레슨입니다.\n"
          u8"1번부터 차례로 들어보고, 가이드의 '해볼 것'을 따라 손잡이를 돌려보세요.",
          "Presets 1-12 are step-by-step FM lessons.\n"
          "Start at 1, play each one, and follow the 'Try' notes in this guide." },

        { "visualizer", "SCOPE / SPECTRUM",
          u8"• SCOPE: 출력 파형의 모양을 보여줍니다.\n"
          u8"• SPECTRUM: 소리 안에 어떤 배음이 있는지 보여줍니다. 아래 숫자는 누른 음의 배수(1 = 기본음)예요.\n\n"
          u8"막대가 숫자 위에 서면 화성적인 소리, 숫자 사이에 서면 비화성적(금속성) 소리입니다.",
          "• SCOPE shows the shape of the output waveform.\n"
          "• SPECTRUM shows which harmonics are in the sound. The numbers are multiples of the played note (1 = fundamental).\n\n"
          "Bars on the numbers = harmonic sound. Bars between them = inharmonic (metallic) sound." },

        { "keyboard", "KEYBOARD",
          u8"마우스로 클릭하거나 컴퓨터 키보드(A W S E D F T G …)로 연주할 수 있습니다.\n"
          u8"MIDI 건반을 연결하면 그대로 연주됩니다.",
          "Click with the mouse or play with your computer keyboard (A W S E D F T G ...).\n"
          "A connected MIDI keyboard works too." },

        { "lang", "LANGUAGE",
          u8"가이드 언어를 한국어 / 영어로 바꿉니다.",
          "Switches the guide between Korean and English." },
    };

    const Text* find (const juce::String& key)
    {
        for (const auto& t : texts)
            if (key == t.key)
                return &t;
        return nullptr;
    }
}

InfoEntry getInfo (const juce::String& id, bool english)
{
    int op = -1;
    OpParam p {};
    juce::String key = id;
    juce::String prefix;

    if (parseOpParamID (id, op, p))
    {
        key = opParamSuffix (p);
        prefix = "OP" + juce::String (op + 1) + " ";
    }

    if (const auto* t = find (key))
        return { prefix + t->title, juce::String::fromUTF8 (english ? t->en : t->ko) };

    return {};
}

juce::String getLcdName (const juce::String& id)
{
    int op = -1;
    OpParam p {};

    if (parseOpParamID (id, op, p))
    {
        const juce::String names[] = { "ON/OFF", "RATIO", "FINE", "DETUNE", "LEVEL", "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
        return "OP" + juce::String (op + 1) + " " + names[static_cast<int> (p)];
    }

    if (id == pid::algorithm) return "ALGORITHM";
    if (id == pid::feedback)  return "FEEDBACK";
    if (id == pid::volume)    return "VOLUME";
    if (id == pid::velBright) return "VEL>BRIGHT";
    if (id == pid::velVolume) return "VEL>VOLUME";

    if (const auto* t = find (id))
        return t->title;

    return id.toUpperCase();
}
}
