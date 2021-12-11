#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#if defined(RAPI_RT64)
#define NOT_COMPATIBLE
#define MOD_NAME "Super Mario 64 RT"
#elif defined(CHEATER) && !defined(R96A)
#define NOT_COMPATIBLE
#define MOD_NAME "CHEATER"
#elif defined(ACT_SPINDASH) || defined(ACT_TRANSFORM)
#define NOT_COMPATIBLE
#define MOD_NAME "Sonic Edition"
#elif defined(ANGLE_QUEUE_SIZE) || defined(ACT_LEDGE_PARKOUR) || defined(ACT_WATER_GROUND_POUND_STROKE)
#define NOT_COMPATIBLE
#define MOD_NAME "Extended Moveset"
#else
#define MOD_NAME "(Unknown)"
#endif

#if defined(_WIN32) && IS_64_BIT
#include "pc/gfx/gfx_window_manager_api.h"
#include "pc/gfx/gfx_dxgi.h"
#include "pc/gfx/gfx_sdl.h"
#include <windows.h>

static struct {
    u32 code;
    const char *error;
    const char *message;
} sOmmCrashHandlerErrors[] = {
    { EXCEPTION_ACCESS_VIOLATION,       "Segmentation Fault",       "The game tried to %s at address 0x%016llX." },
    { EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  "Array Out Of Bounds",      "The game tried to access an element out of the array bounds." },
    { EXCEPTION_DATATYPE_MISALIGNMENT,  "Data Misalignment",        "The game tried to access misaligned data." },
    { EXCEPTION_BREAKPOINT,             "Breakpoint",               "The game reached a breakpoint." },
    { EXCEPTION_FLT_DENORMAL_OPERAND,   "Float Denormal Operand",   "The game tried to perform a floating point operation with a denormal operand." },
    { EXCEPTION_FLT_DIVIDE_BY_ZERO,     "Float Division By Zero",   "The game tried to divide a floating point number by zero." },
    { EXCEPTION_FLT_INEXACT_RESULT,     "Float Inexact Result",     "The game couldn't represent the result of a floating point operation as a decimal fraction." },
    { EXCEPTION_FLT_INVALID_OPERATION,  "Float Invalid Operation",  "The game tried to perform an invalid floating point operation." },
    { EXCEPTION_FLT_OVERFLOW,           "Float Overflow",           "An overflow occurred with a floating point number." },
    { EXCEPTION_FLT_STACK_CHECK,        "Float Stack Overflow",     "The game performed a floating point operation resulting in a stack overflow." },
    { EXCEPTION_FLT_UNDERFLOW,          "Float Underflow",          "An underflow occurred with a floating point number." },
    { EXCEPTION_ILLEGAL_INSTRUCTION,    "Illegal Instruction",      "The game tried to execute an invalid instruction." },
    { EXCEPTION_IN_PAGE_ERROR,          "Page Error",               "The game tried to %s at address 0x%016llX." },
    { EXCEPTION_INT_DIVIDE_BY_ZERO,     "Integer Division By Zero", "The game tried to divide an integer by zero." },
    { EXCEPTION_INT_OVERFLOW,           "Integer Overflow",         "An overflow occurred with an integer." },
    { EXCEPTION_PRIV_INSTRUCTION,       "Instruction Not Allowed",  "The game tried to execute an invalid instruction." },
    { EXCEPTION_STACK_OVERFLOW,         "Stack Overflow",           "The game performed an operation resulting in a stack overflow." },
    { 0,                                "Unknown Exception",        "An unknown exception occurred." },
};

typedef struct {
    s32 x, y;
    u8 r, g, b;
    char s[128];
} OmmCrashHandlerText;
static OmmCrashHandlerText sOmmCrashHandlerText[128];

#define omm_crash_handler_set_text(_x_, _y_, _r_, _g_, _b_, _fmt_, ...) \
{                                                                       \
    if (_x_ == -1) {                                                    \
        pText->x = ((pText - 1)->x + strlen((pText - 1)->s) * 4);       \
    } else {                                                            \
        pText->x = _x_;                                                 \
    }                                                                   \
    pText->y = _y_;                                                     \
    pText->r = _r_;                                                     \
    pText->g = _g_;                                                     \
    pText->b = _b_;                                                     \
    snprintf(pText->s, 128, _fmt_, __VA_ARGS__);                        \
    pText++;                                                            \
}

static void omm_crash_handler_produce_one_frame() {

    // Start frame
    gfx_start_frame();
    load_gfx_memory_pool();
    init_scene_rendering();

    // Clear screen
    create_dl_translation_matrix(MENU_MTX_PUSH, GFX_DIMENSIONS_FROM_LEFT_EDGE(0), 240.f, 0.f);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, (GFX_DIMENSIONS_ASPECT_RATIO * SCREEN_HEIGHT) / 130.f, 3.f, 1.f);
    gDPSetEnvColor(gDisplayListHead++, 0x00, 0x00, 0x00, 0xFF);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Print background
    static const char *sCrashTex = OMM_GFX "omm_crash.bin";
    s32 w = SCREEN_HEIGHT / 2;
    s32 x = GFX_DIMENSIONS_FROM_RIGHT_EDGE(24 + w);
    s32 y = 12;
    omm_render_create_dl_ortho_matrix();
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTextureBlock(gDisplayListHead++, sCrashTex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
    gSPTextureRectangle(gDisplayListHead++, (x) << 2, (SCREEN_HEIGHT - w - y) << 2, (x + w) << 2, (SCREEN_HEIGHT - y) << 2, G_TX_RENDERTILE, 0, 0, (0x4000 / w), (0x4000 / w));
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);

    // Print text
    static Vtx sVertices[0x10000];
    Vtx *vp = sVertices;
    for (OmmCrashHandlerText *text = sOmmCrashHandlerText; text->s[0] != 0; ++text) {
        s32 x = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(text->x);
        s32 y = SCREEN_HEIGHT - 8 - text->y;
        omm_render_create_dl_ortho_matrix();
        gDPPipeSync(gDisplayListHead++);
        gDPSetEnvColor(gDisplayListHead++, text->r, text->g, text->b, 0xFF);
        gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
        gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT);
        gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPLoadSync(gDisplayListHead++);
        gDPLoadTextureBlock(gDisplayListHead++, OMM_TEXTURE_MENU_FONT, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1024, 1024, 0, G_TX_WRAP, G_TX_WRAP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
        for (char *c = text->s; *c != 0; ++c) {
            if (*c > 0x20 && *c < 0x7F) {
                s32 u = ((*c - 0x20) % 20) * 48 * 32;
                s32 v = ((*c - 0x20) / 20) * 80 * 32;
                vp[0] = (Vtx) { { { x,     y,     0 }, 0, { u,             v + (80 * 32) }, { text->r, text->g, text->b, 0xFF } } };
                vp[1] = (Vtx) { { { x + 4, y,     0 }, 0, { u + (48 * 32), v + (80 * 32) }, { text->r, text->g, text->b, 0xFF } } };
                vp[2] = (Vtx) { { { x + 4, y + 7, 0 }, 0, { u + (48 * 32), v             }, { text->r, text->g, text->b, 0xFF } } };
                vp[3] = (Vtx) { { { x,     y + 7, 0 }, 0, { u,             v             }, { text->r, text->g, text->b, 0xFF } } };
                gSPVertex(gDisplayListHead++, vp, 4, 0);
                gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 0, 2, 3, 0);
                vp += 4;
            }
            x += 4;
        }
        gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    }

    // Render frame
    end_master_display_list();
    alloc_display_list(0);
    display_and_vsync();
    gfx_end_frame();
}

static LONG omm_crash_handler(EXCEPTION_POINTERS *ExceptionInfo) {
    OMM_MEMSET(sOmmCrashHandlerText, 0, sizeof(sOmmCrashHandlerText));
    OmmCrashHandlerText *pText = &sOmmCrashHandlerText[0];

    // Exception address, code, type and info
    if (ExceptionInfo && ExceptionInfo->ExceptionRecord) {
        PEXCEPTION_RECORD er = ExceptionInfo->ExceptionRecord;
        omm_crash_handler_set_text( 8, 4, 0xFF, 0x00, 0x00, "%s", "Exception occurred at address ");
        omm_crash_handler_set_text(-1, 4, 0xFF, 0xFF, 0x00, "0x%016llX", (uintptr_t) er->ExceptionAddress);
        omm_crash_handler_set_text(-1, 4, 0xFF, 0x00, 0x00, "%s", " with error code ");
        omm_crash_handler_set_text(-1, 4, 0xFF, 0x00, 0xFF, "0x%08X", (u32) er->ExceptionCode);
        omm_crash_handler_set_text(-1, 4, 0xFF, 0x00, 0x00, "%s", ":");
        for (s32 i = 0; i != OMM_ARRAY_SIZE(sOmmCrashHandlerErrors); ++i) {
            if (sOmmCrashHandlerErrors[i].code == (u32) er->ExceptionCode || sOmmCrashHandlerErrors[i].code == 0) {
                omm_crash_handler_set_text( 8, 12, 0xFF, 0x00, 0x00, "%s", sOmmCrashHandlerErrors[i].error);
                omm_crash_handler_set_text(-1, 12, 0xFF, 0xFF, 0xFF, "%s", " - ");
                if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || er->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
                    omm_crash_handler_set_text(-1, 12, 0xFF, 0xFF, 0xFF, sOmmCrashHandlerErrors[i].message, (er->ExceptionInformation[0] ? "write" : "read"), (uintptr_t) er->ExceptionInformation[1]);
                } else {
                    omm_crash_handler_set_text(-1, 12, 0xFF, 0xFF, 0xFF, "%s", sOmmCrashHandlerErrors[i].message);
                }
                break;
            }
        }
    } else {
        omm_crash_handler_set_text(8,  4, 0xFF, 0x00, 0x00, "%s", "An unknown exception occurred somewhere in the game's code.");
        omm_crash_handler_set_text(8, 12, 0x80, 0x80, 0x80, "%s", "Unable to retrieve the exception info.");
    }

    // Exception report
    omm_crash_handler_set_text(8, 28, 0xFF, 0x80, 0x00, "%s", "Exception report:");
    if (ExceptionInfo) {
        if (OMM_CHEATS_ENABLED || !OMM_MEMCMP(&Cheats, OMM_MEMNEW(Cheats, 1), sizeof(Cheats))) {
            omm_crash_handler_set_text(8, 36, 0xFF, 0x80, 0x00, "%s", "No code conflict detected, but some Cheats were enabled when the exception occurred.");
        } else {
            omm_crash_handler_set_text(8, 36, 0xFF, 0x80, 0x00, "%s", "No code conflict detected. Please report this crash with a consistent way to reproduce it.");
        }
    } else {
        omm_crash_handler_set_text(8, 36, 0xFF, 0x80, 0x00, "%s", "Odyssey Mario's Moveset v" STRINGIFY(OMM_VERSION) " is not compatible with " MOD_NAME ".");
    }

    // Registers
    if (ExceptionInfo && ExceptionInfo->ContextRecord) {
        PCONTEXT cr = ExceptionInfo->ContextRecord;
        omm_crash_handler_set_text( 8, 52, 0xFF, 0xFF, 0xFF,   "%s", "Registers:");
        omm_crash_handler_set_text( 8, 60, 0xFF, 0xFF, 0xFF,   "RSP: 0x%016llX", cr->Rsp);
        omm_crash_handler_set_text(-1, 60, 0xFF, 0xFF, 0xFF, "  RBP: 0x%016llX", cr->Rbp);
        omm_crash_handler_set_text(-1, 60, 0xFF, 0xFF, 0xFF, "  RIP: 0x%016llX", cr->Rip);
        omm_crash_handler_set_text( 8, 68, 0xFF, 0xFF, 0xFF,   "RAX: 0x%016llX", cr->Rax);
        omm_crash_handler_set_text(-1, 68, 0xFF, 0xFF, 0xFF, "  RBX: 0x%016llX", cr->Rbx);
        omm_crash_handler_set_text(-1, 68, 0xFF, 0xFF, 0xFF, "  RCX: 0x%016llX", cr->Rcx);
        omm_crash_handler_set_text(-1, 68, 0xFF, 0xFF, 0xFF, "  RDX: 0x%016llX", cr->Rdx);
        omm_crash_handler_set_text( 8, 76, 0xFF, 0xFF, 0xFF,   "R08: 0x%016llX", cr->R8);
        omm_crash_handler_set_text(-1, 76, 0xFF, 0xFF, 0xFF, "  R09: 0x%016llX", cr->R9);
        omm_crash_handler_set_text(-1, 76, 0xFF, 0xFF, 0xFF, "  R10: 0x%016llX", cr->R10);
        omm_crash_handler_set_text(-1, 76, 0xFF, 0xFF, 0xFF, "  R11: 0x%016llX", cr->R11);
        omm_crash_handler_set_text( 8, 84, 0xFF, 0xFF, 0xFF,   "R12: 0x%016llX", cr->R12);
        omm_crash_handler_set_text(-1, 84, 0xFF, 0xFF, 0xFF, "  R13: 0x%016llX", cr->R13);
        omm_crash_handler_set_text(-1, 84, 0xFF, 0xFF, 0xFF, "  R14: 0x%016llX", cr->R14);
        omm_crash_handler_set_text(-1, 84, 0xFF, 0xFF, 0xFF, "  R15: 0x%016llX", cr->R15);
        omm_crash_handler_set_text( 8, 92, 0xFF, 0xFF, 0xFF,   "RSI: 0x%016llX", cr->Rsi);
        omm_crash_handler_set_text(-1, 92, 0xFF, 0xFF, 0xFF, "  RDI: 0x%016llX", cr->Rdi);
    } else {
        omm_crash_handler_set_text(8, 52, 0xFF, 0xFF, 0xFF, "%s", "Registers:");
        omm_crash_handler_set_text(8, 60, 0x80, 0x80, 0x80, "%s", "Unable to access the registers.");
    }

    // Stack trace
    omm_crash_handler_set_text(8, 108, 0xFF, 0xFF, 0xFF, "%s", "Stack trace:");
    if (ExceptionInfo && ExceptionInfo->ContextRecord) {
        static const char sGlobalFunctionIdentifier[] = "(sec1)(fl0x00)(ty20)(scl2)(nx0)0x";
        static const char sStaticFunctionIdentifier[] = "(sec1)(fl0x00)(ty20)(scl3)(nx0)0x";
        typedef struct Symbol { uintptr_t offset; char name[128]; struct Symbol *next; } Symbol;
        Symbol *symbols = NULL;
        Symbol *symbol0 = NULL;

        // Load symbols
        OMM_STRING(filename, 256, "%s/%s/omm.map", OMM_EXE_FOLDER, OMM_RES_FOLDER);
        FILE *f = fopen(filename, "r");
        if (f) {
            char buffer[1024];
            while (fgets(buffer, 1024, f)) {

                // Remove spaces
                char bufferNoSpace[1024] = { 0 };
                for (char *p0 = buffer, *p1 = bufferNoSpace; *p0 != 0; ++p0) {
                    if (*p0 > 0x20) {
                        *(p1++) = *p0;
                    }
                }

                // Try to find indentifiers
                char *id0 = strstr(bufferNoSpace, sGlobalFunctionIdentifier);
                char *id1 = strstr(bufferNoSpace, sStaticFunctionIdentifier);
                if (id0 || id1) {
                    char *addr = (char *) max((uintptr_t) id0, (uintptr_t) id1) + sizeof(sGlobalFunctionIdentifier) - 1;
                    char *name = addr + 16;

                    // New symbol
                    Symbol *newSymbol = OMM_MEMNEW(Symbol, 1);
                    snprintf(newSymbol->name, 128, "%s", name); *name = 0;
                    sscanf(addr, "%016llX", &newSymbol->offset);
                    newSymbol->next = NULL;

                    // Store symbol
                    if (symbols == NULL) {
                        symbols = newSymbol;
                    } else {
                        for (Symbol *symbol = symbols;; symbol = symbol->next) {
                            if (symbol->next == NULL) {
                                symbol->next = newSymbol;
                                break;
                            }
                            if (symbol->next->offset > newSymbol->offset) {
                                newSymbol->next = symbol->next;
                                symbol->next = newSymbol;
                                break;
                            }
                        }
                    }

                    // Reference
                    if (OMM_MEMCMP(newSymbol->name, "set_mario_action", sizeof("set_mario_action"))) {
                        symbol0 = newSymbol;
                    }
                }
            }
            fclose(f);
        }
        uintptr_t addr0 = (symbol0 ? ((uintptr_t) set_mario_action - symbol0->offset) : 0);

        // Unwind and print call stack
        void *stack[64];
        s32 frames = CaptureStackBackTrace(0, 64, stack, NULL);
        for (s32 i = 1, j = 0; i < frames && j < 15; ++i) {
            s32 y = 116 + j * 8;
            omm_crash_handler_set_text( 8, y, 0xFF, 0xFF, 0x00, "0x%016llX", (uintptr_t) stack[i]);
            omm_crash_handler_set_text(-1, y, 0xFF, 0xFF, 0xFF, "%s", ": ");
            for (Symbol *symbol = symbols;; symbol = symbol->next) {
                if (symbol == NULL || symbol->next == NULL) {
                    if (j != 0) {
                        omm_crash_handler_set_text(-1, y, 0x00, 0xFF, 0xFF, "%s", "????");
                        ++j;
                    }
                    break;
                } else {
                    uintptr_t offset = (uintptr_t) stack[i] - addr0;
                    if (symbol->next->offset > offset) {
                        omm_crash_handler_set_text(-1, y, 0x00, 0xFF, 0xFF, "%s", symbol->name);
                        omm_crash_handler_set_text(-1, y, 0xFF, 0xFF, 0xFF, " + 0x%llX", offset - symbol->offset);
                        ++j;
                        break;
                    }
                }
            }
        }
    } else {
        omm_crash_handler_set_text(8, 116, 0x80, 0x80, 0x80, "%s", "Unable to unwind the call stack.");
    }

    // Game over sound effect
#if defined(R96A)
    dynos_music_stop();
    dynos_jingle_stop();
    dynos_sound_stop(1);
#endif
    if (SDL_WasInit(SDL_INIT_AUDIO) || SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
        SDL_AudioSpec want, have;
        want.freq = 32000;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 0x200;
        want.callback = NULL;
        want.userdata = NULL;
        s32 device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
        if (device) {
            SDL_PauseAudioDevice(device, 0);
            OMM_STRING(filename, 256, "%s/%s/omm_crash.bin", OMM_EXE_FOLDER, OMM_GFX_FOLDER);
            FILE *f = fopen(filename, "rb");
            if (f) {
                u8 data[0x60000];
                fseek(f, 0x20000, SEEK_SET);
                if (fread(data, 1, 0x60000, f) == 0x60000) {
                    fclose(f);
                    SDL_ClearQueuedAudio(device);
                    if (SDL_QueueAudio(device, data, 0x60000) == 0) {
                        gMarioObject = NULL;
                        gObjectLists = NULL;
                        gLoadedGraphNodes = NULL;
                    }
                }
            }
        }
    }

    // Main loop
    while (true) {
#if defined(WAPI_SDL1) || defined(WAPI_SDL2)
        gfx_sdl.main_loop(omm_crash_handler_produce_one_frame);
#elif defined(WAPI_DXGI)
        gfx_dxgi.main_loop(omm_crash_handler_produce_one_frame);
#endif
    }
    exit(0);
}

OMM_AT_STARTUP static void omm_init_crash_handler() {
    SetUnhandledExceptionFilter(omm_crash_handler);
}

#else

static void omm_crash_handler(UNUSED void *p) {
    exit(0);
}

#endif

OMM_ROUTINE_UPDATE(omm_update_crash_handler) {
#if defined(NOT_COMPATIBLE)
    if (gMarioObject || obj_get_first_with_behavior(bhvMenuButtonManager)) {
        omm_crash_handler(NULL);
    }
#endif
}
