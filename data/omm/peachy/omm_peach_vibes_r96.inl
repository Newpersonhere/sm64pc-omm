#if defined(R96A)

extern "C" {
#include "data/omm/omm_defines.h"
#include "data/omm/peachy/omm_peach_actions.h"
#include "data/r96/r96_defines.h"
extern void omm_add_routine(s32 type, void (*func)(void));
extern s32 *gOmmPeachVibeTypeRef;
}

//
// Macros
// The same code is used for both Musics and Jingles, and this file is included twice
//

#undef AudioData
#undef sVibeAudioFreqModifiers
#undef sVibeAudioVolumeModifiers
#undef sVibeAudioNames
#undef sLoadedAudios
#undef sPlayingAudio
#undef mIsVibeAudio
#undef softenAudioVolume
#undef configAudioMute
#undef omm_peach_vibe_load_audio
#undef DynOS_Music_Callback
#undef DynOS_Audio_Callback

#if defined(sLoadedJingles)
#define AudioData                   JingleData
#define sVibeAudioFreqModifiers     sVibeJingleFreqModifiers
#define sVibeAudioVolumeModifiers   sVibeJingleVolumeModifiers
#define sVibeAudioNames             sVibeJingleNames
#define sLoadedAudios               sLoadedJingles
#define sPlayingAudio               sPlayingJingle
#define mIsVibeAudio                mFadeIn
#define softenAudioVolume           softenJingleVolume
#define configAudioMute             0
#define DynOS_Jingle_Callback       DynOS_Jingle_Callback_
#define DynOS_Audio_Callback        DynOS_Jingle_Callback
#else
#define AudioData                   MusicData
#define sVibeAudioFreqModifiers     sVibeMusicFreqModifiers
#define sVibeAudioVolumeModifiers   sVibeMusicVolumeModifiers
#define sVibeAudioNames             sVibeMusicNames
#define sLoadedAudios               sLoadedMusics
#define sPlayingAudio               sPlayingMusic
#define mIsVibeAudio                mFadeIn
#define softenAudioVolume           softenVolume
#define configAudioMute             configMusicMute
#define DynOS_Music_Callback        DynOS_Music_Callback_
#define DynOS_Audio_Callback        DynOS_Music_Callback
#endif

#define SAMPLE_ALIGNMENT            (sizeof(s16) * MUSIC_CHANNELS)
#define ALIGN_TO_SAMPLE(x)          ((((s32) (x)) / SAMPLE_ALIGNMENT) * SAMPLE_ALIGNMENT)

//
// Constants
//

static const f32 sVibeAudioFreqModifiers[] = {
    1.f,
    OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_JOY,
    OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_RAGE,
    OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_GLOOM,
    OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_CALM,
};

static const f32 sVibeAudioVolumeModifiers[] = {
    1.f,
    OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_JOY,
    OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_RAGE,
    OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_GLOOM,
    OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_CALM,
};

static const char *sVibeAudioNames[] = {
    R96_LEVEL_BOB,
    R96_LEVEL_WF,
    R96_LEVEL_CCM,
    R96_LEVEL_CCM_SLIDE,
    R96_LEVEL_JRB_PHASE_1,
    R96_LEVEL_JRB_PHASE_2,
    R96_LEVEL_JRB_PHASE_3,
    R96_LEVEL_JRB_SHIP,
    R96_LEVEL_BBH_PHASE_1,
    R96_LEVEL_BBH_PHASE_2,
    R96_LEVEL_HMC_PHASE_1,
    R96_LEVEL_HMC_PHASE_2,
    R96_LEVEL_LLL_OUTSIDE,
    R96_LEVEL_LLL_VOLCANO,
    R96_LEVEL_SSL_OUTSIDE,
    R96_LEVEL_SSL_PYRAMID,
    R96_LEVEL_SSL_EYEROK,
    R96_LEVEL_DDD_PHASE_1,
    R96_LEVEL_DDD_PHASE_2,
    R96_LEVEL_DDD_PHASE_3,
    R96_LEVEL_SL_OUTSIDE,
    R96_LEVEL_SL_IGLOO,
    R96_LEVEL_WDW_MAIN,
    R96_LEVEL_WDW_TOWN,
    R96_LEVEL_TTM_OUTSIDE,
    R96_LEVEL_TTM_SLIDE,
    R96_LEVEL_THI_HUGE,
    R96_LEVEL_THI_TINY,
    R96_LEVEL_THI_WIGGLER,
    R96_LEVEL_TTC,
    R96_LEVEL_RR,
    R96_LEVEL_BITDW,
    R96_LEVEL_BITFS,
    R96_LEVEL_BITS,
    R96_LEVEL_SA,
    R96_LEVEL_WMOTR,
    R96_LEVEL_PSS,
    R96_LEVEL_COTMC,
    R96_LEVEL_TOTWC,
    R96_LEVEL_VCUTM,
    R96_LEVEL_BOWSER_1,
    R96_LEVEL_BOWSER_2,
    R96_LEVEL_BOWSER_3,
    R96_LEVEL_INSIDE_CASTLE_LOBBY,
    R96_LEVEL_INSIDE_CASTLE_TIPPY,
    R96_LEVEL_INSIDE_CASTLE_BASEMENT,
    R96_LEVEL_CASTLE_GROUNDS,
    R96_LEVEL_CASTLE_COURTYARD,
    R96_LEVEL_FOURTH_FLOOR,
    R96_EVENT_BOSS_THEME,
    R96_EVENT_ENDLESS_STAIRS,
    R96_EVENT_MERRY_GO_ROUND,
    R96_EVENT_RACE,
    R96_EVENT_PIRANHA_PLANT,
    R96_EVENT_GOT_MILK,
    R96_EVENT_SHELL,
};

//
// Load
//

#if defined(sLoadedJingles)
OMM_ROUTINE_GFX(omm_peach_vibe_load_jingles) {
#else
OMM_ROUTINE_GFX(omm_peach_vibe_load_musics) {
#endif

    // Loop through every Vibe variant (Joy, Rage, Gloom, Calm)
    for (s32 aType : { OMM_PEACH_VIBE_TYPE_JOY, OMM_PEACH_VIBE_TYPE_RAGE, OMM_PEACH_VIBE_TYPE_GLOOM, OMM_PEACH_VIBE_TYPE_CALM }) {

        // Loop through every loaded audio
        for (AudioData *aAudioData : sLoadedAudios) {

            // Don't load anything if the audio is too short or inexistant
            if (aAudioData->mLength < AUDIO_FREQUENCY_44100 * MUSIC_CHANNELS * sizeof(s16)) {
                continue;
            }

            // Loop through every compatible audio
            for (const auto &aVibeAudioName : sVibeAudioNames) {
                if (aAudioData->mName == aVibeAudioName) {
                    
                    // Add 5 pointers at the end of the mData buffer to store all Vibe variants
                    if (!aAudioData->mIsVibeAudio) {
                        u8 *extendedData = (u8 *) OMM_MEMNEW(u8, aAudioData->mLength + OMM_PEACH_VIBE_TYPE_COUNT * sizeof(u8 *));
                        OMM_MEMCPY(extendedData, aAudioData->mData, aAudioData->mLength);
                        OMM_MEMDEL(aAudioData->mData);
                        aAudioData->mData = extendedData;
                        aAudioData->mIsVibeAudio = 1;
                    }

                    // Check if the Vibe variant is loaded
                    u8 **dataStart = &(((u8 **) (aAudioData->mData + aAudioData->mLength))[aType]);
                    if (!*dataStart) {

                        // Resample audio data
                        const s16 *inputBuffer = (const s16 *) aAudioData->mData;
                        s32 inputSamples = aAudioData->mLength / (sizeof(s16) * MUSIC_CHANNELS);
                        s32 outputSamples = inputSamples / sVibeAudioFreqModifiers[aType];
                        *dataStart = (u8 *) OMM_MEMNEW(u8, outputSamples * sizeof(s16) * MUSIC_CHANNELS);
                        for (s32 i = 0; i != outputSamples; ++i) {
                            s32 i0 = omm_clamp_s(i * sVibeAudioFreqModifiers[aType], 0, inputSamples - 1);
                            s32 i1 = omm_clamp_s(i0 + 1, 0, inputSamples - 1);
                            f32 fi = (i * sVibeAudioFreqModifiers[aType]) - (f32) i0;
                            for (s32 j = 0; j != MUSIC_CHANNELS; ++j) {
                                ((s16 *) *dataStart)[MUSIC_CHANNELS * i + j] = (inputBuffer[MUSIC_CHANNELS * i0 + j] * (1.f - fi)) + (inputBuffer[MUSIC_CHANNELS * i1 + j] * fi);
                            }
                        }
#if defined(DEBUG)
                        printf("Loaded Vibe variant %d for %s\n", aType, aVibeAudioName);
                        fflush(stdout);
#endif
                    }
                }
            }
        }
    }
}

//
// Update
//

static void DynOS_Audio_Callback(UNUSED void *, u8 *aStream, s32 aLength) {
    bzero(aStream, aLength);
    if (sPlayingAudio == NULL) {
        return;
    }

    // Audio data
    u8 *_Data    = sPlayingAudio->mData;
    s32 _Loop    = sPlayingAudio->mLoop;
    s32 _Length  = sPlayingAudio->mLength;
    s32 _Current = sPlayingAudio->mCurrent;
    f32 _Volume  = sPlayingAudio->mVolume * (configMasterVolume / 127.f) * (configMusicVolume / 127.f) * softenAudioVolume * !configAudioMute;
    f32 _Mult    = 1.f;
    
    // Check if a Vibe is active and if it's a Vibe audio
    if (*gOmmPeachVibeTypeRef != OMM_PEACH_VIBE_TYPE_NONE && sPlayingAudio->mIsVibeAudio) {
        _Data    = ((u8 **) (sPlayingAudio->mData + sPlayingAudio->mLength))[*gOmmPeachVibeTypeRef];
        _Loop    = ALIGN_TO_SAMPLE(_Loop / sVibeAudioFreqModifiers[*gOmmPeachVibeTypeRef]);
        _Length  = ALIGN_TO_SAMPLE(_Length / sVibeAudioFreqModifiers[*gOmmPeachVibeTypeRef]);
        _Current = ALIGN_TO_SAMPLE(_Current / sVibeAudioFreqModifiers[*gOmmPeachVibeTypeRef]);
        _Volume  = omm_clamp_f(_Volume * sVibeAudioVolumeModifiers[*gOmmPeachVibeTypeRef], 0.f, 1.f);
        _Mult    = sVibeAudioFreqModifiers[*gOmmPeachVibeTypeRef];
    }
    
    // Mix audio
    s32 _LenTilEnd = _Length - _Current;
    if (_LenTilEnd < aLength) {
        DynOS_Audio_Mix(aStream, _Data + _Current, _LenTilEnd, _Volume, 0);
        if (sPlayingAudio->mLoop != 0) {
            DynOS_Audio_Mix(aStream + _LenTilEnd, _Data + _Loop, aLength - _LenTilEnd, _Volume, 0);
            _Current = (_Loop + (aLength - _LenTilEnd));
        } else {
            sPlayingAudio = NULL;
        }
    } else {
        DynOS_Audio_Mix(aStream, _Data + _Current, aLength, _Volume, 0);
        _Current += aLength;
    }

    // Update current
    if (sPlayingAudio) {
        sPlayingAudio->mCurrent = ALIGN_TO_SAMPLE(_Current * _Mult);
#if !defined(sLoadedJingles)
        mCurrentMultiTrack = sPlayingAudio->mCurrent;
#endif
    }
}

#endif
