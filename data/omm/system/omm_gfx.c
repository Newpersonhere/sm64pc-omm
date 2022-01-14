#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// STBI implementation
//

#if OMM_STBI_IMPL && !OMM_CODE_DYNOS
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#endif

//
// Texture load
//

struct OmmTexture {
    const char *id;
    const uint8_t *data;
    int width;
    int height;
    struct OmmTexture *next;
};

static struct OmmTexture *omm_gfx_get_texture(const char *id) {
    static struct OmmTexture *sOmmTextures = NULL;

    // Null-check
    if (!id) {
        return NULL;
    }

    // Check if the texture is already loaded
    for (struct OmmTexture *tex = sOmmTextures; tex != NULL; tex = tex->next) {
        if (strcmp(tex->id, id) == 0) {
            return tex;
        }
    }

    // Check if the id is valid
    if (!OMM_MEMCMP(id, OMM_GFX, sizeof(OMM_GFX) - 1)) {
        return NULL;
    }

    // Open file
    bool isBin = strstr(id, ".bin") != NULL;
    OMM_STRING(filename, 256, "%s/%s/%s%s", OMM_EXE_FOLDER, OMM_GFX_FOLDER, id + sizeof(OMM_GFX) - 1, (isBin ? "" : ".png"));
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }

    // Load from file
    int width, height;
    uint8_t *data = stbi_load_from_file(f, &width, &height, NULL, 4);
    fclose(f);
    if (!data) {
        return NULL;
    }

    // Set new texture data
    struct OmmTexture *tex = OMM_MEMNEW(struct OmmTexture, 1);
    tex->id = OMM_MEMDUP(id, strlen(id) + 1);
    tex->data = data;
    tex->width = width;
    tex->height = height;
    tex->next = sOmmTextures;
    sOmmTextures = tex;
    return tex;
}

//
// Texture import
//

static bool __omm_gfx_import_texture(struct GfxRenderingAPI *rapi, void *cache, int tile, void **node, const void *addr, int fmt, int siz) {
    struct OmmTexture *tex = omm_gfx_get_texture((const char *) addr);
    if (tex) {
        if (!((bool (*)(int, void **, const void *, int, int)) cache)(tile, node, tex->id, fmt, siz)) {
            rapi->upload_texture(tex->data, tex->width, tex->height);
        }
        return true;
    }
    return false;
}

bool omm_gfx_import_texture(struct GfxRenderingAPI *rapi, void *cache, int tile, void **node, const void *addr, int fmt, int siz) {
#if defined(EXTERNAL_DATA) || OMM_GAME_IS_R96A
    static bool precached = false;
    if (!precached && configPrecacheRes) {
#undef OMM_TEXTURE_
#define OMM_TEXTURE_(id, name) __omm_gfx_import_texture(rapi, cache, tile, node, OMM_TEXTURE_##id, fmt, siz);
#include "data/omm/omm_defines_textures.inl"
#undef OMM_TEXTURE_
        precached = true;
    }
#endif
    return __omm_gfx_import_texture(rapi, cache, tile, node, addr, fmt, siz);
}
