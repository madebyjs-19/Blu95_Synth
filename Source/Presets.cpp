#include "Presets.h"

namespace blu
{
namespace
{
    constexpr OpPreset silent { true, 1, 0, 0, 0.0f, 0.005f, 1.0f, 100.0f, 0.3f };
}

const std::vector<FactoryPreset>& getFactoryPresets()
{
    //                                 on  crs fin det  level  attack  decay  sus    release
    static const std::vector<FactoryPreset> presets = {
        { "SINE WAVE", 1, 0.0f, 0.0f, 30.0f,
          { { { true, 1, 0, 0, 90.0f, 0.005f, 1.0f, 100.0f, 0.3f }, silent, silent, silent } },
          u8"지금은 OP1(캐리어) 하나만 소리를 내고 있어요. 배음이 전혀 없는 가장 순수한 소리, 사인파입니다.\n\n"
          u8"• 해볼 것: OP2의 LEVEL을 천천히 올려보세요. OP2가 OP1의 주파수를 흔들면서(= 모듈레이션) 배음이 생겨납니다.\n"
          u8"• SPECTRUM 화면에서 막대가 하나에서 여러 개로 늘어나는 걸 확인하세요.",
          "Only OP1 (a carrier) is sounding: a pure sine wave with no harmonics at all.\n\n"
          "• Try: slowly raise OP2 LEVEL. OP2 starts wobbling OP1's frequency (= modulation) and harmonics appear.\n"
          "• Watch the SPECTRUM: one bar grows into many." },

        { "FIRST FM", 1, 0.0f, 30.0f, 30.0f,
          { { { true, 1, 0, 0, 90.0f, 0.005f, 1.0f, 100.0f, 0.3f },
              { true, 1, 0, 0, 50.0f, 0.005f, 1.0f, 100.0f, 0.3f }, silent, silent } },
          u8"OP2(모듈레이터)가 OP1(캐리어)을 1:1 비율로 변조합니다.\n"
          u8"캐리어:모듈레이터(C:M) 비율이 1:1이면 1, 2, 3, 4… 모든 정수배 배음이 생겨 톱니파처럼 밝은 소리가 납니다.\n\n"
          u8"• OP2 LEVEL = 변조 깊이(Modulation Index). 올릴수록 배음이 많아져 밝아집니다.\n"
          u8"• LCD에 표시되는 MOD INDEX 값을 보면서 돌려보세요.",
          "OP2 (modulator) modulates OP1 (carrier) at a 1:1 ratio.\n"
          "With a carrier:modulator (C:M) ratio of 1:1 every harmonic 1, 2, 3, 4... appears, giving a bright saw-like tone.\n\n"
          "• OP2 LEVEL = modulation depth (modulation index). More index, more harmonics, brighter sound.\n"
          "• Watch the MOD INDEX value on the LCD while turning it." },

        { "HOLLOW 1:2", 1, 0.0f, 30.0f, 30.0f,
          { { { true, 1, 0, 0, 90.0f, 0.01f, 1.0f, 100.0f, 0.3f },
              { true, 2, 0, 0, 48.0f, 0.01f, 1.0f, 100.0f, 0.3f }, silent, silent } },
          u8"OP2의 RATIO를 2로 바꿨어요. C:M = 1:2 이면 배음이 1, 3, 5, 7… 홀수 자리에만 생깁니다. 클라리넷이나 사각파처럼 속이 빈 소리죠.\n\n"
          u8"• OP2 RATIO를 3, 4, 5로 바꿔보세요.\n"
          u8"• 정수 비율이면 SPECTRUM의 막대가 항상 숫자 눈금 위에 섭니다 = 음정감 있는 '악기' 소리.",
          "OP2 RATIO is now 2. With C:M = 1:2 only the odd harmonics 1, 3, 5, 7... appear - a hollow, clarinet or square-like tone.\n\n"
          "• Try OP2 RATIO 3, 4, 5.\n"
          "• With whole-number ratios the SPECTRUM bars always land on the numbered marks = a pitched, 'musical' sound." },

        { "BELL", 1, 0.0f, 40.0f, 40.0f,
          { { { true, 1, 0, 0, 90.0f, 0.002f, 4.0f, 0.0f, 3.0f },
              { true, 3, 50, 0, 55.0f, 0.002f, 2.5f, 0.0f, 2.5f }, silent, silent } },
          u8"C:M = 1:3.5 처럼 정수가 아닌 비율을 쓰면 배음이 정수배 자리에서 벗어납니다(비화성 배음). 그래서 종, 실로폰, 금속을 두드린 소리가 됩니다.\n\n"
          u8"• SPECTRUM에서 막대들이 숫자 눈금 '사이'에 서는 걸 보세요.\n"
          u8"• OP2 FINE을 돌려 다른 금속 소리를 찾아보세요.",
          "A ratio that is not a whole number, like C:M = 1:3.5, pushes the partials off the harmonic series (inharmonic partials). That is the sound of bells, chimes and struck metal.\n\n"
          "• Notice the SPECTRUM bars standing BETWEEN the numbered marks.\n"
          "• Turn OP2 FINE to find other metallic tones." },

        { "BRASS", 1, 0.0f, 60.0f, 30.0f,
          { { { true, 1, 0, 0, 90.0f, 0.06f, 0.3f, 85.0f, 0.2f },
              { true, 1, 0, 0, 62.0f, 0.12f, 0.4f, 70.0f, 0.2f }, silent, silent } },
          u8"FM에서 모듈레이터의 엔벨로프는 '음량'이 아니라 '밝기'가 시간에 따라 변하는 모양을 만듭니다.\n"
          u8"OP2의 ATTACK이 조금 느려서 소리가 어둡게 시작해서 밝게 열립니다. 브라스 특유의 '빠-앙' 느낌이에요.\n\n"
          u8"• OP2 ATTACK과 DECAY를 바꿔가며 차이를 들어보세요.\n"
          u8"• 건반을 세게/약하게 쳐보세요 (VEL>BRT).",
          "In FM, a modulator's envelope shapes BRIGHTNESS over time, not volume.\n"
          "OP2's attack is a bit slow, so the tone starts dark and opens up - the classic brass 'blare'.\n\n"
          "• Change OP2 ATTACK and DECAY and listen.\n"
          "• Play soft and hard (VEL>BRT)." },

        { "E.PIANO", 5, 0.0f, 70.0f, 50.0f,
          { { { true, 1, 0, 0, 88.0f, 0.002f, 3.5f, 0.0f, 0.5f },
              { true, 1, 0, 0, 38.0f, 0.002f, 1.6f, 10.0f, 0.5f },
              { true, 1, 0, 3, 55.0f, 0.002f, 1.0f, 0.0f, 0.4f },
              { true, 14, 0, 0, 42.0f, 0.001f, 0.25f, 0.0f, 0.2f } } },
          u8"알고리즘 5는 2→1, 4→3 두 쌍(스택)이 각자 소리를 냅니다. DX7 일렉트릭 피아노의 비밀이에요.\n"
          u8"• OP1+OP2: 부드러운 몸통 소리\n"
          u8"• OP3+OP4: OP4의 RATIO 14가 만드는 '팅' 하는 금속성 어택\n\n"
          u8"• OP4의 ON 버튼을 꺼보세요. 어택이 사라집니다!\n"
          u8"• 세게/약하게 연주해보세요.",
          "Algorithm 5 runs two independent pairs (stacks): 2>1 and 4>3. This is the secret of the DX7 electric piano.\n"
          "• OP1+OP2: the soft body of the tone\n"
          "• OP3+OP4: the metallic 'tine' attack made by OP4 at RATIO 14\n\n"
          "• Switch OP4 OFF - the attack disappears!\n"
          "• Play soft and hard." },

        { "FM BASS", 1, 0.0f, 50.0f, 30.0f,
          { { { true, 1, 0, 0, 95.0f, 0.001f, 0.6f, 60.0f, 0.12f },
              { true, 1, 0, 0, 66.0f, 0.001f, 0.25f, 25.0f, 0.1f },
              { true, 1, 0, 0, 32.0f, 0.001f, 0.15f, 0.0f, 0.1f }, silent } },
          u8"모듈레이터의 짧은 DECAY가 '퉁' 하는 펑키한 어택을 만듭니다.\n"
          u8"알고리즘 1은 4→3→2→1 직렬 연결이라 OP3가 OP2를 또 변조해서 소리가 더 거칠고 풍부해집니다.\n\n"
          u8"• OP2 LEVEL과 DECAY로 어택의 세기와 길이를 조절해보세요.\n"
          u8"• 낮은 옥타브에서 연주하세요.",
          "A short modulator DECAY creates the punchy, funky attack.\n"
          "Algorithm 1 is a 4>3>2>1 chain, so OP3 also modulates OP2, making the tone grittier and richer.\n\n"
          "• Shape the attack with OP2 LEVEL and DECAY.\n"
          "• Play in a low octave." },

        { "ORGAN", 8, 0.0f, 0.0f, 10.0f,
          { { { true, 0, 0, 0, 70.0f, 0.005f, 0.1f, 100.0f, 0.08f },
              { true, 1, 0, 0, 80.0f, 0.005f, 0.1f, 100.0f, 0.08f },
              { true, 2, 0, 0, 60.0f, 0.005f, 0.1f, 100.0f, 0.08f },
              { true, 3, 0, 0, 50.0f, 0.005f, 0.1f, 100.0f, 0.08f } } },
          u8"알고리즘 8은 네 오퍼레이터가 모두 캐리어입니다. 변조 없이 사인파를 겹쳐 쌓는 '가산 합성'이에요. 해먼드 오르간의 드로우바와 같은 원리죠.\n\n"
          u8"• 각 LEVEL을 드로우바처럼 조절해보세요.\n"
          u8"• RATIO 0.5 / 1 / 2 / 3 은 16' / 8' / 4' / 2 2/3' 드로우바와 같습니다.\n"
          u8"• SPECTRUM에 막대 4개가 보이죠?",
          "Algorithm 8 makes all four operators carriers. No modulation - just stacking sine waves: additive synthesis, the same idea as Hammond organ drawbars.\n\n"
          "• Use each LEVEL like a drawbar.\n"
          "• RATIO 0.5 / 1 / 2 / 3 = the 16' / 8' / 4' / 2 2/3' drawbars.\n"
          "• Can you see exactly four bars in the SPECTRUM?" },

        { "MARIMBA", 1, 0.0f, 50.0f, 50.0f,
          { { { true, 1, 0, 0, 92.0f, 0.001f, 0.9f, 0.0f, 0.6f },
              { true, 4, 0, 0, 45.0f, 0.001f, 0.12f, 0.0f, 0.1f }, silent, silent } },
          u8"캐리어의 SUSTAIN을 0으로 하면, 건반을 누르고 있어도 소리가 저절로 사라지는 타악기형 소리가 됩니다.\n"
          u8"OP2(RATIO 4)의 아주 짧은 DECAY가 나무를 두드리는 '톡' 소리를 만듭니다.\n\n"
          u8"• OP1 DECAY를 늘리면 비브라폰처럼 길게 울립니다.\n"
          u8"• OP2 RATIO를 바꿔보세요.",
          "With the carrier's SUSTAIN at 0 the sound fades away by itself even while you hold the key - a percussive shape.\n"
          "OP2's very short DECAY at RATIO 4 creates the wooden 'tock' of the mallet.\n\n"
          "• Lengthen OP1 DECAY for a vibraphone-like ring.\n"
          "• Try other OP2 RATIO values." },

        { "FEEDBACK", 8, 60.0f, 20.0f, 30.0f,
          { { { true, 1, 0, 0, 0.0f, 0.005f, 1.0f, 100.0f, 0.3f },
              { true, 1, 0, 0, 0.0f, 0.005f, 1.0f, 100.0f, 0.3f },
              { true, 1, 0, 0, 0.0f, 0.005f, 1.0f, 100.0f, 0.3f },
              { true, 1, 0, 0, 90.0f, 0.005f, 1.0f, 100.0f, 0.3f } } },
          u8"OP4는 자기 출력으로 자기 자신을 변조할 수 있습니다. 이걸 '피드백'이라고 해요. 지금은 OP4 하나만 소리 납니다.\n\n"
          u8"• FEEDBACK을 0까지 내렸다가 천천히 올려보세요. 사인파 → 톱니파처럼 변하고, 끝까지 올리면 노이즈에 가까워집니다.\n"
          u8"• SCOPE에서 파형이 기울어지는 모습을 보세요.",
          "OP4 can modulate itself with its own output. This is called feedback. Right now only OP4 is sounding.\n\n"
          "• Take FEEDBACK down to 0, then raise it slowly: sine > saw-like, and near the top it turns noisy.\n"
          "• Watch the waveform lean over in the SCOPE." },

        { "STRINGS PAD", 5, 0.0f, 20.0f, 30.0f,
          { { { true, 1, 0, -3, 78.0f, 0.6f, 1.2f, 85.0f, 1.2f },
              { true, 1, 0, 0, 30.0f, 0.8f, 1.2f, 70.0f, 1.2f },
              { true, 1, 0, 3, 78.0f, 0.6f, 1.2f, 85.0f, 1.2f },
              { true, 2, 0, 0, 22.0f, 0.8f, 1.2f, 70.0f, 1.2f } } },
          u8"두 스택을 DETUNE으로 서로 반대 방향으로 살짝 어긋나게 해서, 코러스처럼 넓게 퍼지는 소리를 만들었어요.\n"
          u8"캐리어의 ATTACK과 RELEASE가 길어서 패드처럼 부드럽게 들어오고 빠집니다.\n\n"
          u8"• OP1과 OP3의 DETUNE을 0으로 맞춰보세요. 소리가 좁아집니다.\n"
          u8"• 화음을 길게 눌러보세요.",
          "The two stacks are detuned slightly in opposite directions, so the sound spreads out like a chorus.\n"
          "Long carrier ATTACK and RELEASE make it fade in and out softly, like a pad.\n\n"
          "• Set OP1 and OP3 DETUNE to 0 - the sound gets narrow.\n"
          "• Hold some long chords." },

        { "SPACE BELLS", 6, 0.0f, 50.0f, 40.0f,
          { { { true, 1, 0, 0, 80.0f, 0.002f, 5.0f, 0.0f, 3.0f },
              { true, 2, 0, 2, 55.0f, 0.002f, 3.5f, 0.0f, 2.5f },
              { true, 4, 0, -2, 45.0f, 0.002f, 2.0f, 0.0f, 2.0f },
              { true, 1, 41, 0, 36.0f, 0.002f, 3.0f, 0.0f, 2.5f } } },
          u8"알고리즘 6은 OP4 하나가 세 캐리어를 동시에 변조합니다.\n"
          u8"RATIO 1.41(≈√2) 같은 무리수 비율은 아주 신비롭고 금속적인 배음을 만들어요.\n\n"
          u8"• 이제 마지막 레슨입니다. 알고리즘을 바꿔가며 같은 오퍼레이터 설정이 얼마나 다른 소리가 되는지 들어보세요.\n"
          u8"• 그리고 나만의 소리를 만들어보세요!",
          "In algorithm 6 a single OP4 modulates three carriers at once.\n"
          "An irrational ratio like 1.41 (~ square root of 2) gives mysterious, metallic partials.\n\n"
          "• This is the last lesson. Switch algorithms and hear how differently the same operator settings sound.\n"
          "• Then go make your own sounds!" },
    };

    return presets;
}

void applyPreset (const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state)
{
    auto set = [&state] (const juce::String& id, float value)
    {
        if (auto* p = state.getParameter (id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (value));
            p->endChangeGesture();
        }
    };

    set (pid::algorithm, static_cast<float> (preset.algorithm));
    set (pid::feedback,  preset.feedback);
    set (pid::velBright, preset.velBright);
    set (pid::velVolume, preset.velVolume);

    for (int op = 0; op < kNumOps; ++op)
    {
        const auto& o = preset.ops[static_cast<size_t> (op)];
        set (opParamID (op, OpParam::on),      o.on ? 1.0f : 0.0f);
        set (opParamID (op, OpParam::coarse),  static_cast<float> (o.coarse));
        set (opParamID (op, OpParam::fine),    static_cast<float> (o.fine));
        set (opParamID (op, OpParam::detune),  static_cast<float> (o.detune));
        set (opParamID (op, OpParam::level),   o.level);
        set (opParamID (op, OpParam::attack),  o.attack);
        set (opParamID (op, OpParam::decay),   o.decay);
        set (opParamID (op, OpParam::sustain), o.sustain);
        set (opParamID (op, OpParam::release), o.release);
    }
}
}
