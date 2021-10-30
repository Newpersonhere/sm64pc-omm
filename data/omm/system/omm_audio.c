#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#define SOUND_ID(sound) ((sound >> 16) & 0xFFFF)
#define SOUND_PRIO(sound) ((sound >> 8) & 0xFF)

//
// Audio
//

#define OMM_AUDIO_FREQUENCY         32000
#define OMM_AUDIO_FREQUENCY_STR     STRINGIFY(OMM_AUDIO_FREQUENCY)
#define OMM_AUDIO_SEQUENCE_SIZE     (OMM_AUDIO_FREQUENCY / 10)
#define OMM_AUDIO_OVERLAP_SIZE      (OMM_AUDIO_FREQUENCY / 50)
#define OMM_AUDIO_WINDOW_SIZE       (OMM_AUDIO_FREQUENCY / 60)
#define OMM_AUDIO_FLAT_DURATION     (OMM_AUDIO_SEQUENCE_SIZE - 2 * (OMM_AUDIO_OVERLAP_SIZE))
#define OMM_AUDIO_SEQUENCE_SKIP     ((s32) ((OMM_AUDIO_SEQUENCE_SIZE - OMM_AUDIO_OVERLAP_SIZE) * (timeScale)))

// Resampling is done in a temporary buffer, so output and input can be the same buffer
// outputScale > 1 = time up, pitch down
static s32 omm_audio_resample(u8 **output, const u8 *input, s32 inputLength, f32 outputScale) {
    s32 inputSamples = inputLength / sizeof(s16);
    s32 outputSamples = inputSamples * outputScale;
    u8 *temp = OMM_MEMNEW(s16, outputSamples);

    // Resampling
    for (s32 i = 0; i != outputSamples; ++i) {
        s32 i0 = omm_clamp_s(((i / outputScale) + 0), 0, inputSamples - 1);
        s32 i1 = omm_clamp_s(((i / outputScale) + 1), 0, inputSamples - 1);
        f32 fi = (i / outputScale) - (f32) i0;
        ((s16 *) temp)[i] = (s16) ((((const s16 *) input)[i0] * (1.f - fi)) + (((const s16 *) input)[i1] * fi));
    }

    // Copy temp to output
    OMM_MEMDEL(*output);
    *output = OMM_MEMDUP(temp, outputSamples * sizeof(s16));
    OMM_MEMDEL(temp);
    return outputSamples * sizeof(s16);
}

// Time stretching is done in a temporary buffer, so output and input can be the same buffer
// timeStretch > 1 = time up, pitch stays the same
static s32 omm_audio_time_stretch(u8 **output, const u8 *input, s32 inputLength, f32 timeStretch) {
    s32 inputSamples = inputLength / sizeof(s16);
    s32 outputSamples = inputSamples * timeStretch;
    u8 *temp = OMM_MEMNEW(s16, outputSamples);

    // Synchronized OverLap-Add (SOLA) algorithm
    const s16 *inputBuffer = (const s16 *) input;
    const s16 *currOffset = inputBuffer;
    s16 *outputBuffer = (s16 *) temp;
    f32 timeScale = 1.f / timeStretch;
    for (s32 remaining = inputSamples; remaining > OMM_AUDIO_SEQUENCE_SKIP + OMM_AUDIO_WINDOW_SIZE; remaining -= OMM_AUDIO_SEQUENCE_SKIP) {
        OMM_MEMCPY(outputBuffer, currOffset, OMM_AUDIO_FLAT_DURATION * sizeof(s16));
        const s16 *prevOffset = currOffset + OMM_AUDIO_FLAT_DURATION;
        inputBuffer += OMM_AUDIO_SEQUENCE_SKIP - OMM_AUDIO_OVERLAP_SIZE;
        s32 bestOffset = 0;
        f32 bestCorr = -1e30f;
        f32 temp[OMM_AUDIO_OVERLAP_SIZE];

        // Precalculate overlapping slopes with prevOffset
        for (s32 i = 0; i < OMM_AUDIO_OVERLAP_SIZE; ++i) {
            temp[i] = (f32) (prevOffset[i] * i * (OMM_AUDIO_OVERLAP_SIZE - i));
        }

        // Find best overlap offset within [0..OMM_AUDIO_WINDOW_SIZE]
        for (s32 i = 0; i < OMM_AUDIO_WINDOW_SIZE; ++i) {
            f32 crossCorr = 0;
            for (s32 j = 0; j < OMM_AUDIO_OVERLAP_SIZE; ++j) {
                crossCorr += (f32) inputBuffer[i + j] * temp[j];
            }
            if (crossCorr > bestCorr) {
                bestCorr = crossCorr;
                bestOffset = i;
            }
        }
        currOffset = inputBuffer + bestOffset;

        // Do overlapping between previous and current, copy result to outputBuffer
        for (s32 i = 0; i < OMM_AUDIO_OVERLAP_SIZE; ++i) {
            outputBuffer[i + OMM_AUDIO_FLAT_DURATION] = (prevOffset[i] * (OMM_AUDIO_OVERLAP_SIZE - i) + currOffset[i] * i) / OMM_AUDIO_OVERLAP_SIZE;
        }

        // Update offsets
        currOffset += OMM_AUDIO_OVERLAP_SIZE;
        inputBuffer += OMM_AUDIO_OVERLAP_SIZE;
        outputBuffer += OMM_AUDIO_SEQUENCE_SIZE - OMM_AUDIO_OVERLAP_SIZE;
    }

    // Copy temp to output
    OMM_MEMDEL(*output);
    *output = OMM_MEMDUP(temp, outputSamples * sizeof(s16));
    OMM_MEMDEL(temp);
    return outputSamples * sizeof(s16);
}

static void omm_audio_resize(u8 **output, const u8 *input, s32 inputLength, s32 desiredLength) {
    u8 *temp = OMM_MEMNEW(s16, desiredLength);
    OMM_MEMCPY(temp, input, omm_min_s(inputLength, desiredLength));
    OMM_MEMDEL(*output);
    *output = OMM_MEMDUP(temp, desiredLength);
    OMM_MEMDEL(temp);
}

// Pitch shifting is done in a temporary buffer, so output and input can be the same buffer
// pitchScale > 1 = pitch up, time stays the same
static s32 omm_audio_pitch_shift(u8 **output, const u8 *input, s32 inputLength, f32 pitchScale) {
    if (pitchScale > 1.f) {
        s32 tempLength = omm_audio_time_stretch(output, input, inputLength, pitchScale);
        s32 finalLength = omm_audio_resample(output, *output, tempLength, 1.f / pitchScale);
        omm_audio_resize(output, *output, finalLength, inputLength);
    } else if (pitchScale > 0.f && pitchScale < 1.f) {
        s32 tempLength = omm_audio_resample(output, input, inputLength, 1.f / pitchScale);
        s32 finalLength = omm_audio_time_stretch(output, *output, tempLength, pitchScale);
        omm_audio_resize(output, *output, finalLength, inputLength);
    }
    return inputLength;
}

static u8 *omm_audio_mix(u8 *output, const u8 *input, s32 length, s32 volume, s32 distance) {
    s32 acrVolume = omm_clamp_s((1 << 7) - ((distance << 7) / sAcousticReachPerLevel[gCurrLevelNum]), 0, 1 << 7);
    s32 mixVolume = omm_clamp_s((volume * acrVolume) >> 7, 0, 1 << 7);
    for (s32 i = 0; i < length; i += sizeof(s16)) {
        *((s16 *) (output + i)) = (s16) ((((s32) *((s16 *) (input + i))) * mixVolume) >> 7);
    }
    return output;
}

//
// Data
//

// input[0] = raw data
// input[1] = joy data
// input[2] = rage data
// input[3] = gloom data
// input[4] = calm data
// output   = computed data
typedef struct {
    u8 *input[5];
    u8 *output;
    s32 length;
    s32 volume;
    u8 priority;
    u8 bank;
} OmmSoundData;

static OmmSoundData  sOmmSoundData[0x100] = { 0 };
static OmmSoundData *sOmmSoundPlaying[4] = { NULL };

static SDL_AudioDeviceID omm_sound_get_device(u8 bank) {
    static SDL_AudioDeviceID sSoundDeviceId[4] = { 0 };
    if (!sSoundDeviceId[bank]) {

        // Init SDL2 Audio
        if (!SDL_WasInit(SDL_INIT_AUDIO)) {
            if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
                sys_fatal("omm_sound_get_device: Could not init SDL Audio.");
            }
        }

        // Open sound device
        SDL_AudioSpec want, have;
        want.freq     = OMM_AUDIO_FREQUENCY;
        want.format   = AUDIO_S16SYS;
        want.channels = 1;
        want.samples  = 0x200;
        want.callback = NULL;
        want.userdata = NULL;
        sSoundDeviceId[bank] = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
        if (sSoundDeviceId == 0) {
            sys_fatal("omm_sound_get_device: Could not open sound device.");
        }
        SDL_PauseAudioDevice(sSoundDeviceId[bank], 0);
    }
    return sSoundDeviceId[bank];
}

//
// Play
//

void omm_sound_play(s32 id, f32 *pos) {
    if (!omm_sound_play_character_sound_omm(id, pos)) {
        OmmSoundData *sound = &sOmmSoundData[id];
        if (sound->input[0]) {

            // Update playing sound
            if (SDL_GetQueuedAudioSize(omm_sound_get_device(sound->bank)) == 0) {
                sOmmSoundPlaying[sound->bank] = NULL;
            }

            // Don't overwrite playing sounds with higher priority
            // Sounds with equal priority cancel out each other
            if (sOmmSoundPlaying[sound->bank] != NULL
                && sOmmSoundPlaying[sound->bank]->priority > sound->priority) {
                return;
            }

            s32 volume = (sound->volume * configMasterVolume * configSfxVolume) / (0x7F * 0x7F);
            s32 distance = (s32)(pos == NULL ? 0 : vec3f_length(pos));
            omm_audio_mix(sound->output, sound->input[gOmmData->mario->peach.vibeType], sound->length, volume, distance);
            SDL_ClearQueuedAudio(omm_sound_get_device(sound->bank));
            SDL_QueueAudio(omm_sound_get_device(sound->bank), sound->output, sound->length);
            sOmmSoundPlaying[sound->bank] = sound;
        }
    }
}

void omm_sound_stop(s32 id) {
    if (omm_sound_is_playing(id)) {
        OmmSoundData *sound = &sOmmSoundData[id];
        SDL_ClearQueuedAudio(omm_sound_get_device(sound->bank));
        sOmmSoundPlaying[sound->bank] = NULL;
    }
}

bool omm_sound_is_playing(s32 id) {
    OmmSoundData *sound = &sOmmSoundData[id];
    if (sound->input[0]) {

        // Update playing sound
        if (SDL_GetQueuedAudioSize(omm_sound_get_device(sound->bank)) == 0) {
            sOmmSoundPlaying[sound->bank] = NULL;
            return false;
        }

        // Playing
        return (sOmmSoundPlaying[sound->bank] == sound);
    }
    return false;
}

bool omm_sound_is_bank_playing(u8 bank) {
    return SDL_GetQueuedAudioSize(omm_sound_get_device(bank)) != 0;
}

//
// Load
//

static void omm_sound_load_wav(s32 id, const char *name, u8 bank, s32 vibesPitchShift, s32 volume, u8 priority) {

    // Load wav
    SDL_AudioSpec wav; u8 *data; s32 length;
    OMM_STRING(filename, 256, "%s/%s/%s.wav", OMM_EXE_FOLDER, OMM_SOUND_FOLDER, name);
    if (!SDL_LoadWAV(filename, &wav, &data, (u32 *) &length)) {
        sys_fatal("omm_sound_load_wav: Unable to load file %s.", filename);
    }
    if (wav.freq != OMM_AUDIO_FREQUENCY) {
        sys_fatal("omm_sound_load_wav: From file %s, audio frequency should be " OMM_AUDIO_FREQUENCY_STR " Hz, is %d.", filename, wav.freq);
    }
    if (wav.format != AUDIO_S16SYS) {
        sys_fatal("omm_sound_load_wav: From file %s, audio format is not Signed 16-bit PCM.", filename);
    }
    if (wav.channels != 1) {
        sys_fatal("omm_sound_load_wav: From file %s, audio channel count should be 1, is %d.", filename, wav.channels);
    }
    if (volume < 0 || volume > 127) {
        sys_fatal("omm_sound_load_wav: From file %s, audio volume should be between 0 and 127, is %d.", filename, volume);
    }

    // Set sound data
    OmmSoundData *sound = &sOmmSoundData[id];
    sound->input[0]     = data;
    sound->input[1]     = OMM_MEMDUP(data, length * sizeof(u8));
    sound->input[2]     = OMM_MEMDUP(data, length * sizeof(u8));
    sound->input[3]     = OMM_MEMDUP(data, length * sizeof(u8));
    sound->input[4]     = OMM_MEMDUP(data, length * sizeof(u8));
    sound->output       = OMM_MEMDUP(data, length * sizeof(u8));
    sound->length       = length;
    sound->volume       = volume;
    sound->priority     = priority;
    sound->bank         = bank;

    // Voice effects
    if (vibesPitchShift) {

        // Joy: pitch up
        omm_audio_pitch_shift(&sound->input[OMM_PEACH_VIBE_TYPE_JOY], data, length, OMM_PEACH_VIBE_SOUND_PITCH_MOD_JOY);

        // Rage: demonic voice
        u8 *rageL = OMM_MEMDUP(data, length * sizeof(u8)); omm_audio_pitch_shift(&rageL, data, length, OMM_PEACH_VIBE_SOUND_PITCH_MOD_RAGE * 0.98f);
        u8 *rageR = OMM_MEMDUP(data, length * sizeof(u8)); omm_audio_pitch_shift(&rageR, data, length, OMM_PEACH_VIBE_SOUND_PITCH_MOD_RAGE * 1.02f);
        for (s32 i = 0; i < length; i += sizeof(s16)) {
            *((s16 *) (sound->input[OMM_PEACH_VIBE_TYPE_RAGE] + i)) = (s16) omm_clamp_s((((s32) *((s16 *) (rageL + i))) + ((s32) *((s16 *) (rageR + i)))) * 0.8f, -0x8000, +0x7FFF);
        }

        // Gloom: speed down
        omm_audio_resample(&sound->input[OMM_PEACH_VIBE_TYPE_GLOOM], data, length, 1.f / OMM_PEACH_VIBE_SOUND_PITCH_MOD_GLOOM);

        // Calm: pitch slightly up
        omm_audio_pitch_shift(&sound->input[OMM_PEACH_VIBE_TYPE_CALM], data, length, OMM_PEACH_VIBE_SOUND_PITCH_MOD_CALM);
    }
}

OMM_AT_STARTUP static void omm_audio_init() {

    // "peach" bank
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_YAH,            "peach/omm_sound_peach_jump_yah",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAH_WAH_HOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_WAH,            "peach/omm_sound_peach_jump_wah",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAH_WAH_HOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_HOO,            "peach/omm_sound_peach_jump_hoo",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAH_WAH_HOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_YAHOO,          "peach/omm_sound_peach_jump_yahoo",         0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAHOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_WAHA,           "peach/omm_sound_peach_jump_waha",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAHOO_WAHA_YIPPEE));
    omm_sound_load_wav(OMM_SOUND_PEACH_JUMP_YIPPEE,         "peach/omm_sound_peach_jump_yippee",        0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAHOO_WAHA_YIPPEE));
    omm_sound_load_wav(OMM_SOUND_PEACH_LAND_HAHA,           "peach/omm_sound_peach_land_haha",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_HAHA));
    omm_sound_load_wav(OMM_SOUND_PEACH_PUNCH_YAH,           "peach/omm_sound_peach_punch_yah",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PUNCH_YAH));
    omm_sound_load_wav(OMM_SOUND_PEACH_PUNCH_WAH,           "peach/omm_sound_peach_punch_wah",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PUNCH_WAH));
    omm_sound_load_wav(OMM_SOUND_PEACH_PUNCH_HOO,           "peach/omm_sound_peach_punch_hoo",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PUNCH_HOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_GRAB_HRMM,           "peach/omm_sound_peach_grab_hrmm",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_HRMM));
    omm_sound_load_wav(OMM_SOUND_PEACH_THROW_WAH2,          "peach/omm_sound_peach_throw_wah2",         0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_WAH2));
    omm_sound_load_wav(OMM_SOUND_PEACH_GROUND_POUND_WAH,    "peach/omm_sound_peach_ground_pound_wah",   0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_GROUND_POUND_WAH));
    omm_sound_load_wav(OMM_SOUND_PEACH_DIVE_HOOHOO,         "peach/omm_sound_peach_dive_hoohoo",        0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_HOOHOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_TWIRL_BOUNCE,        "peach/omm_sound_peach_twirl_bounce",       0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_TWIRL_BOUNCE));
    omm_sound_load_wav(OMM_SOUND_PEACH_SO_LONGA_BOWSER,     "peach/omm_sound_peach_so_longa_bowser",    0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_SO_LONGA_BOWSER));
    omm_sound_load_wav(OMM_SOUND_PEACH_LEDGE_GRAB_WHOA,     "peach/omm_sound_peach_ledge_grab_whoa",    0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_WHOA));
    omm_sound_load_wav(OMM_SOUND_PEACH_LEDGE_CLIMB_EEUH,    "peach/omm_sound_peach_ledge_climb_eeuh",   0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_EEUH));
    omm_sound_load_wav(OMM_SOUND_PEACH_LEDGE_CLIMB_UH2,     "peach/omm_sound_peach_ledge_climb_uh2",    0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_UH2));
    omm_sound_load_wav(OMM_SOUND_PEACH_LEDGE_MISS_UH,       "peach/omm_sound_peach_ledge_miss_uh",      0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_UH));
    omm_sound_load_wav(OMM_SOUND_PEACH_BONK_DOH,            "peach/omm_sound_peach_bonk_doh",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_DOH));
    omm_sound_load_wav(OMM_SOUND_PEACH_BONK_OOOF,           "peach/omm_sound_peach_bonk_ooof",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_OOOF));
    omm_sound_load_wav(OMM_SOUND_PEACH_ATTACKED,            "peach/omm_sound_peach_attacked",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_ATTACKED));
    omm_sound_load_wav(OMM_SOUND_PEACH_ON_FIRE,             "peach/omm_sound_peach_on_fire",            0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_ON_FIRE));
    omm_sound_load_wav(OMM_SOUND_PEACH_DYING,               "peach/omm_sound_peach_dying",              0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_DYING));
    omm_sound_load_wav(OMM_SOUND_PEACH_DROWNING,            "peach/omm_sound_peach_drowning",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_DROWNING));
    omm_sound_load_wav(OMM_SOUND_PEACH_FALLING_WAAAOOOW,    "peach/omm_sound_peach_falling_waaaooow",   0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAH_WAH_HOO));
    omm_sound_load_wav(OMM_SOUND_PEACH_MAMA_MIA,            "peach/omm_sound_peach_mama_mia",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_MAMA_MIA));
    omm_sound_load_wav(OMM_SOUND_PEACH_LETS_A_GO,           "peach/omm_sound_peach_lets_a_go",          0, 1, 0x54, SOUND_PRIO(SOUND_MENU_STAR_SOUND_LETS_A_GO));
    omm_sound_load_wav(OMM_SOUND_PEACH_HERE_WE_GO,          "peach/omm_sound_peach_here_we_go",         0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_HERE_WE_GO));
    omm_sound_load_wav(OMM_SOUND_PEACH_PANTING1,            "peach/omm_sound_peach_panting1",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PANTING));
    omm_sound_load_wav(OMM_SOUND_PEACH_PANTING2,            "peach/omm_sound_peach_panting2",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PANTING));
    omm_sound_load_wav(OMM_SOUND_PEACH_PANTING3,            "peach/omm_sound_peach_panting3",           0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_PANTING));
    omm_sound_load_wav(OMM_SOUND_PEACH_COUGHING1,           "peach/omm_sound_peach_coughing1",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_COUGHING1));
    omm_sound_load_wav(OMM_SOUND_PEACH_COUGHING2,           "peach/omm_sound_peach_coughing2",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_COUGHING2));
    omm_sound_load_wav(OMM_SOUND_PEACH_COUGHING3,           "peach/omm_sound_peach_coughing3",          0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_COUGHING3));
    omm_sound_load_wav(OMM_SOUND_PEACH_YAWNING,             "peach/omm_sound_peach_yawning",            0, 1, 0x54, SOUND_PRIO(SOUND_MARIO_YAWNING));
    omm_sound_load_wav(OMM_SOUND_PEACH_VIBE_JOY,            "peach/omm_sound_peach_vibe_joy",           0, 0, 0x54, 0xFF);
    omm_sound_load_wav(OMM_SOUND_PEACH_VIBE_RAGE,           "peach/omm_sound_peach_vibe_rage",          0, 0, 0x54, 0xFF);
    omm_sound_load_wav(OMM_SOUND_PEACH_VIBE_GLOOM,          "peach/omm_sound_peach_vibe_gloom",         0, 0, 0x54, 0xFF);
    omm_sound_load_wav(OMM_SOUND_PEACH_VIBE_CALM,           "peach/omm_sound_peach_vibe_calm",          0, 0, 0x54, 0xFF);

    // "toad" bank
    omm_sound_load_wav(OMM_SOUND_TOAD_1,                    "toad/omm_sound_toad_1",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_2,                    "toad/omm_sound_toad_2",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_3,                    "toad/omm_sound_toad_3",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_4,                    "toad/omm_sound_toad_4",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_5,                    "toad/omm_sound_toad_5",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_6,                    "toad/omm_sound_toad_6",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_7,                    "toad/omm_sound_toad_7",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_8,                    "toad/omm_sound_toad_8",                    1, 0, 0x60, 0xFF);
    omm_sound_load_wav(OMM_SOUND_TOAD_9,                    "toad/omm_sound_toad_9",                    1, 0, 0x60, 0xFF);

    // "event" bank
    omm_sound_load_wav(OMM_SOUND_EVENT_CAPTURE,             "event/omm_sound_event_capture",            2, 0, 0x60, 0x80);
    omm_sound_load_wav(OMM_SOUND_EVENT_UNCAPTURE,           "event/omm_sound_event_uncapture",          2, 0, 0x60, 0x80);
    omm_sound_load_wav(OMM_SOUND_EVENT_LIFE_UP,             "event/omm_sound_event_life_up",            2, 0, 0x58, 0xF8);
    omm_sound_load_wav(OMM_SOUND_EVENT_SPARKLY_STAR_GET,    "event/omm_sound_event_sparkly_star_get",   2, 0, 0x74, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_MARIO,         "event/omm_sound_event_death_mario_1",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_MARIO_WATER,   "event/omm_sound_event_death_mario_2",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_MARIO_FALL,    "event/omm_sound_event_death_mario_3",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_PEACH,         "event/omm_sound_event_death_peach_1",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_PEACH_WATER,   "event/omm_sound_event_death_peach_2",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_PEACH_FALL,    "event/omm_sound_event_death_peach_3",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_LUIGI,         "event/omm_sound_event_death_luigi_1",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_LUIGI_WATER,   "event/omm_sound_event_death_luigi_2",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_LUIGI_FALL,    "event/omm_sound_event_death_luigi_3",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_WARIO,         "event/omm_sound_event_death_wario_1",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_WARIO_WATER,   "event/omm_sound_event_death_wario_2",      2, 0, 0x58, 0xFF);
    omm_sound_load_wav(OMM_SOUND_EVENT_DEATH_WARIO_FALL,    "event/omm_sound_event_death_wario_3",      2, 0, 0x58, 0xFF);

    // "effect" bank
    omm_sound_load_wav(OMM_SOUND_EFFECT_DAMAGE,             "effect/omm_sound_effect_damage",           3, 0, 0x40, 0xF0);
    omm_sound_load_wav(OMM_SOUND_EFFECT_HEAL,               "effect/omm_sound_effect_heal",             3, 0, 0x34, 0xF0);
    omm_sound_load_wav(OMM_SOUND_EFFECT_FREEZE_1,           "effect/omm_sound_effect_freeze_1",         3, 0, 0x7F, 0x00);
    omm_sound_load_wav(OMM_SOUND_EFFECT_FREEZE_2,           "effect/omm_sound_effect_freeze_2",         3, 0, 0x7F, 0x00);
    omm_sound_load_wav(OMM_SOUND_EFFECT_PROPELLER_1,        "effect/omm_sound_effect_propeller_1",      3, 0, 0x60, 0x01);
    omm_sound_load_wav(OMM_SOUND_EFFECT_PROPELLER_2,        "effect/omm_sound_effect_propeller_2",      3, 0, 0x54, 0x01);
    omm_sound_load_wav(OMM_SOUND_EFFECT_PROPELLER_3,        "effect/omm_sound_effect_propeller_3",      3, 0, 0x58, 0x01);
    omm_sound_load_wav(OMM_SOUND_EFFECT_MIDAIR_SPIN,        "effect/omm_sound_effect_midair_spin",      3, 0, 0x40, 0x02);
    omm_sound_load_wav(OMM_SOUND_EFFECT_PEACH_GLIDE,        "effect/omm_sound_effect_peach_glide",      3, 0, 0x30, 0x40);
    omm_sound_load_wav(OMM_SOUND_EFFECT_PEACH_VIBE,         "effect/omm_sound_effect_peach_vibe",       3, 0, 0x30, 0xFF);
}

//
// Character sounds
//

typedef struct {
    u8 type; // 0 = no sound, 1 = play_sound, 2 = omm_sound_play, 3 = dynos_sound_play
    union { s32 idNum; const char *idStr; };
} OmmCharacterSound;

#define NUL_SOUND           { .type = 0, .idNum = 0 }
#define N64_SOUND(sound)    { .type = 1, .idNum = sound }
#define OMM_SOUND(sound)    { .type = 2, .idNum = sound }

static const OmmCharacterSound sOmmCharacterSoundsMarioN64[] = {
    N64_SOUND(SOUND_MARIO_YAH_WAH_HOO + 0x00000),       // Jump Yah
    N64_SOUND(SOUND_MARIO_YAH_WAH_HOO + 0x10000),       // Jump Wah
    N64_SOUND(SOUND_MARIO_YAH_WAH_HOO + 0x20000),       // Jump Hoo
    N64_SOUND(SOUND_MARIO_PUNCH_YAH),                   // Punch Yah
    N64_SOUND(SOUND_MARIO_PUNCH_WAH),                   // Punch Wah
    N64_SOUND(SOUND_MARIO_PUNCH_HOO),                   // Punch Hoo
    N64_SOUND(SOUND_MARIO_HOOHOO),                      // Hoo-hoo
    N64_SOUND(SOUND_MARIO_YAHOO),                       // Yahoo
    N64_SOUND(SOUND_MARIO_YAHOO_WAHA_YIPPEE + 0x00000), // Yahoo
    N64_SOUND(SOUND_MARIO_YAHOO_WAHA_YIPPEE + 0x10000), // Yahoo
    N64_SOUND(SOUND_MARIO_YAHOO_WAHA_YIPPEE + 0x20000), // Yahoo
    N64_SOUND(SOUND_MARIO_YAHOO_WAHA_YIPPEE + 0x30000), // Waha
    N64_SOUND(SOUND_MARIO_YAHOO_WAHA_YIPPEE + 0x40000), // Yippee
    N64_SOUND(SOUND_MARIO_HAHA),                        // Ha-ha
    N64_SOUND(SOUND_MARIO_HAHA_2),                      // Ha-ha
    N64_SOUND(SOUND_MARIO_UH),                          // Grunt 1
    N64_SOUND(SOUND_MARIO_UH2),                         // Grunt 2
    N64_SOUND(SOUND_MARIO_UH2_2),                       // Grunt 2
    N64_SOUND(SOUND_MARIO_HRMM),                        // Lift
    N64_SOUND(SOUND_MARIO_WAH2),                        // Throw
    N64_SOUND(SOUND_MARIO_GROUND_POUND_WAH),            // Ground pound Wah
    N64_SOUND(SOUND_MARIO_WHOA),                        // Whoa
    N64_SOUND(SOUND_MARIO_EEUH),                        // Pull-up
    N64_SOUND(SOUND_MARIO_ATTACKED),                    // Attacked
    N64_SOUND(SOUND_MARIO_OOOF),                        // Ooof
    N64_SOUND(SOUND_MARIO_OOOF2),                       // Ooof
    N64_SOUND(SOUND_MARIO_DOH),                         // Doh
    N64_SOUND(SOUND_MARIO_HERE_WE_GO),                  // Here we go
    N64_SOUND(SOUND_MARIO_YAWNING),                     // Yawning
    N64_SOUND(SOUND_MARIO_SNORING1),                    // Snoring 1
    N64_SOUND(SOUND_MARIO_SNORING2),                    // Snoring 2
    N64_SOUND(SOUND_MARIO_SNORING3),                    // Snoring 3
    N64_SOUND(SOUND_MARIO_PANTING + 0x00000),           // Panting 1
    N64_SOUND(SOUND_MARIO_PANTING + 0x10000),           // Panting 2
    N64_SOUND(SOUND_MARIO_PANTING + 0x20000),           // Panting 3
    N64_SOUND(SOUND_MARIO_PANTING_COLD),                // Panting (cold)
    N64_SOUND(SOUND_MARIO_COUGHING1),                   // Coughing 1
    N64_SOUND(SOUND_MARIO_COUGHING2),                   // Coughing 2
    N64_SOUND(SOUND_MARIO_COUGHING3),                   // Coughing 3
    N64_SOUND(SOUND_MARIO_WAAAOOOW),                    // Falling
    N64_SOUND(SOUND_MARIO_ON_FIRE),                     // Burned
    N64_SOUND(SOUND_MARIO_DYING),                       // Dying
    N64_SOUND(SOUND_MARIO_DROWNING),                    // Drowning
    N64_SOUND(SOUND_MARIO_MAMA_MIA),                    // Mama-mia
    N64_SOUND(SOUND_MARIO_OKEY_DOKEY),                  // Okey dokey
    N64_SOUND(SOUND_MARIO_GAME_OVER),                   // Game Over
    N64_SOUND(SOUND_MARIO_HELLO),                       // Hello
    N64_SOUND(SOUND_MARIO_PRESS_START_TO_PLAY),         // Press Start to play
    N64_SOUND(SOUND_MARIO_TWIRL_BOUNCE),                // Boing
    N64_SOUND(SOUND_MARIO_SO_LONGA_BOWSER),             // So long-a Bowser
    N64_SOUND(SOUND_MARIO_IMA_TIRED),                   // I'm-a tired
 // N64_SOUND(SOUND_MENU_COIN_ITS_A_ME_MARIO),          // It's-a me Mario
    N64_SOUND(SOUND_MENU_THANK_YOU_PLAYING_MY_GAME),    // Thank you so much for playing my game
    N64_SOUND(SOUND_MENU_STAR_SOUND_OKEY_DOKEY),        // Okey dokey (+ star sound)
    N64_SOUND(SOUND_MENU_STAR_SOUND_LETS_A_GO),         // Let's-a go (+ star sound)
    N64_SOUND(SOUND_PEACH_MARIO),                       // Peach: Mario
    N64_SOUND(SOUND_PEACH_POWER_OF_THE_STARS),          // Peach: The power of the Stars...
    N64_SOUND(SOUND_PEACH_THANKS_TO_YOU),               // Peach: Thanks to you
    N64_SOUND(SOUND_PEACH_THANK_YOU_MARIO),             // Peach: Thank you Mario
    N64_SOUND(SOUND_PEACH_SOMETHING_SPECIAL),           // Peach: We have to do something special...
    N64_SOUND(SOUND_PEACH_BAKE_A_CAKE),                 // Peach: Let's bake a delicious cake
    N64_SOUND(SOUND_PEACH_FOR_MARIO),                   // Peach: For Mario
    N64_SOUND(SOUND_PEACH_MARIO2),                      // Peach: Mario!
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_MARIO),             // OMM sound: Dying
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_MARIO_WATER),       // OMM sound: Drowning
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_MARIO_FALL),        // OMM sound: Falling
};

static const OmmCharacterSound sOmmCharacterSoundsPeachOMM[] = {
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YAH),             // Jump Yah
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_WAH),             // Jump Wah
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_HOO),             // Jump Hoo
    OMM_SOUND(OMM_SOUND_PEACH_PUNCH_YAH),            // Punch Yah
    OMM_SOUND(OMM_SOUND_PEACH_PUNCH_WAH),            // Punch Wah
    OMM_SOUND(OMM_SOUND_PEACH_PUNCH_HOO),            // Punch Hoo
    OMM_SOUND(OMM_SOUND_PEACH_DIVE_HOOHOO),          // Hoo-hoo
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YAHOO),           // Yahoo
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YAHOO),           // Yahoo
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YAHOO),           // Yahoo
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YAHOO),           // Yahoo
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_WAHA),            // Waha
    OMM_SOUND(OMM_SOUND_PEACH_JUMP_YIPPEE),          // Yippee
    OMM_SOUND(OMM_SOUND_PEACH_LAND_HAHA),            // Ha-ha
    OMM_SOUND(OMM_SOUND_PEACH_LAND_HAHA),            // Ha-ha
    OMM_SOUND(OMM_SOUND_PEACH_LEDGE_MISS_UH),        // Grunt 1
    OMM_SOUND(OMM_SOUND_PEACH_LEDGE_CLIMB_UH2),      // Grunt 2
    OMM_SOUND(OMM_SOUND_PEACH_LEDGE_CLIMB_UH2),      // Grunt 2
    OMM_SOUND(OMM_SOUND_PEACH_GRAB_HRMM),            // Lift
    OMM_SOUND(OMM_SOUND_PEACH_THROW_WAH2),           // Throw
    OMM_SOUND(OMM_SOUND_PEACH_GROUND_POUND_WAH),     // Ground pound Wah
    OMM_SOUND(OMM_SOUND_PEACH_LEDGE_GRAB_WHOA),      // Whoa
    OMM_SOUND(OMM_SOUND_PEACH_LEDGE_CLIMB_EEUH),     // Pull-up
    OMM_SOUND(OMM_SOUND_PEACH_ATTACKED),             // Attacked
    OMM_SOUND(OMM_SOUND_PEACH_BONK_OOOF),            // Ooof
    OMM_SOUND(OMM_SOUND_PEACH_BONK_OOOF),            // Ooof
    OMM_SOUND(OMM_SOUND_PEACH_BONK_DOH),             // Doh
    OMM_SOUND(OMM_SOUND_PEACH_HERE_WE_GO),           // Here we go
    OMM_SOUND(OMM_SOUND_PEACH_YAWNING),              // Yawning
    NUL_SOUND,                                       // Snoring 1
    NUL_SOUND,                                       // Snoring 2
    NUL_SOUND,                                       // Snoring 3
    OMM_SOUND(OMM_SOUND_PEACH_PANTING1),             // Panting 1
    OMM_SOUND(OMM_SOUND_PEACH_PANTING2),             // Panting 2
    OMM_SOUND(OMM_SOUND_PEACH_PANTING3),             // Panting 3
    NUL_SOUND,                                       // Panting (cold)
    OMM_SOUND(OMM_SOUND_PEACH_COUGHING1),            // Coughing 1
    OMM_SOUND(OMM_SOUND_PEACH_COUGHING2),            // Coughing 2
    OMM_SOUND(OMM_SOUND_PEACH_COUGHING3),            // Coughing 3
    OMM_SOUND(OMM_SOUND_PEACH_FALLING_WAAAOOOW),     // Falling
    OMM_SOUND(OMM_SOUND_PEACH_ON_FIRE),              // Burned
    OMM_SOUND(OMM_SOUND_PEACH_DYING),                // Dying
    OMM_SOUND(OMM_SOUND_PEACH_DROWNING),             // Drowning
    OMM_SOUND(OMM_SOUND_PEACH_MAMA_MIA),             // Mama-mia
    N64_SOUND(SOUND_MENU_STAR_SOUND_OKEY_DOKEY),     // Okey dokey
    N64_SOUND(SOUND_MARIO_GAME_OVER),                // Game Over
    N64_SOUND(SOUND_MARIO_HELLO),                    // Hello
    N64_SOUND(SOUND_MARIO_PRESS_START_TO_PLAY),      // Press Start to play
    OMM_SOUND(OMM_SOUND_PEACH_TWIRL_BOUNCE),         // Boing
    OMM_SOUND(OMM_SOUND_PEACH_SO_LONGA_BOWSER),      // So long-a Bowser
    NUL_SOUND,                                       // I'm-a tired
 // N64_SOUND(SOUND_MENU_COIN_ITS_A_ME_MARIO),       // It's-a me Mario
    N64_SOUND(SOUND_MENU_THANK_YOU_PLAYING_MY_GAME), // Thank you so much for playing my game
    N64_SOUND(SOUND_MENU_STAR_SOUND_OKEY_DOKEY),     // Okey dokey (+ star sound)
    OMM_SOUND(OMM_SOUND_PEACH_LETS_A_GO),            // Let's-a go (+ star sound)
    N64_SOUND(SOUND_PEACH_MARIO),                    // Peach: Mario
    N64_SOUND(SOUND_PEACH_POWER_OF_THE_STARS),       // Peach: The power of the Stars...
    N64_SOUND(SOUND_PEACH_THANKS_TO_YOU),            // Peach: Thanks to you
    N64_SOUND(SOUND_PEACH_THANK_YOU_MARIO),          // Peach: Thank you Mario
    N64_SOUND(SOUND_PEACH_SOMETHING_SPECIAL),        // Peach: We have to do something special...
    N64_SOUND(SOUND_PEACH_BAKE_A_CAKE),              // Peach: Let's bake a delicious cake
    N64_SOUND(SOUND_PEACH_FOR_MARIO),                // Peach: For Mario
    N64_SOUND(SOUND_PEACH_MARIO2),                   // Peach: Mario!
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_PEACH),          // OMM sound: Dying
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_PEACH_WATER),    // OMM sound: Drowning
    OMM_SOUND(OMM_SOUND_EVENT_DEATH_PEACH_FALL),     // OMM sound: Falling
};

static const OmmCharacterSound *sOmmCharacterSounds[] = {
    sOmmCharacterSoundsMarioN64,
    sOmmCharacterSoundsPeachOMM,
};
static const s32 sOmmCharacterSoundsNumSounds = OMM_ARRAY_SIZE(sOmmCharacterSoundsMarioN64);
static const s32 sOmmCharacterSoundsNumBanks = OMM_ARRAY_SIZE(sOmmCharacterSounds);

static void omm_sound_process_character_sound_from_index(s32 index, f32 *pos, bool play) {

    // Find the corresponding sound for the selected character
    const OmmCharacterSound *sound = NULL;
    switch (omm_player_get_selected_index()) {
        case OMM_PLAYER_MARIO: {
            sound = &sOmmCharacterSoundsMarioN64[index];
        } break;

        case OMM_PLAYER_PEACH: {
            sound = &sOmmCharacterSoundsPeachOMM[index];
        } break;

        case OMM_PLAYER_LUIGI: {
            sound = &sOmmCharacterSoundsMarioN64[index];
        } break;

        case OMM_PLAYER_WARIO: {
            sound = &sOmmCharacterSoundsMarioN64[index];
        } break;
    }

    // Play or stop the sound if found
    if (sound) {
        if (pos == NULL) {
            pos = gGlobalSoundArgs;
        }
        switch (sound->type) {

            // No sound
            case 0: {
            } break;

            // N64 sound
            case 1: {
                if (play) {
                    play_sound(sound->idNum, pos);
                } else {
                    sound_stop(sound->idNum, pos);
                }
            } break;

            // OMM sound
            case 2: {
                if (play) {
                    omm_sound_play(sound->idNum, pos);
                } else {
                    omm_sound_stop(sound->idNum);
                }
            } break;
        }
    }
}

static bool omm_sound_process_character_sound(s32 idN64, s32 idOMM, const char *idR96, f32 *pos, bool play) {
    static bool exec = false;
    if (exec) return false;
    exec = true;
    for (s32 i = 0; i != sOmmCharacterSoundsNumBanks; ++i) {
        const OmmCharacterSound *bank = sOmmCharacterSounds[i];
        for (s32 j = 0; j != sOmmCharacterSoundsNumSounds; ++j) {
            const OmmCharacterSound *sound = &bank[j];
            if ((sound->type == 1 && idN64 != -1 && SOUND_ID(idN64) == SOUND_ID(sound->idNum)) ||
                (sound->type == 2 && idOMM != -1 && idOMM == sound->idNum) ||
                (sound->type == 3 && idR96 != NULL && strcmp(idR96, sound->idStr) == 0)) {
                omm_sound_process_character_sound_from_index(j, pos, play);
                exec = false;
                return true;
            }
        }
    }
    exec = false;
    return false;
}

bool omm_sound_play_character_sound_n64(s32 id, f32 *pos) {
    return omm_sound_process_character_sound(id, -1, NULL, pos, true);
}

bool omm_sound_play_character_sound_omm(s32 id, f32 *pos) {
    return omm_sound_process_character_sound(-1, id, NULL, pos, true);
}

bool omm_sound_play_character_sound_r96(const char *id, f32 *pos) {
    return omm_sound_process_character_sound(-1, -1, id, pos, true);
}

bool omm_sound_stop_character_sound_n64(s32 id, f32 *pos) {
    return omm_sound_process_character_sound(id, -1, NULL, pos, false);
}

bool omm_sound_stop_character_sound_omm(s32 id, f32 *pos) {
    return omm_sound_process_character_sound(-1, id, NULL, pos, false);
}

bool omm_sound_stop_character_sound_r96(const char *id, f32 *pos) {
    return omm_sound_process_character_sound(-1, -1, id, pos, false);
}
