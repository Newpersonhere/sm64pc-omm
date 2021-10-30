#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// OMM Array: Dynamically allocated array
//

// 0x00: size (4 bytes)
// 0x04: count (4 bytes)
// 0x08: capacity (4 bytes)
// 0x0C: start of data

OmmArray __omm_array_new(s32 size) {
    if (size > 0) {
        OmmArray arr = OMM_MEMNEW(u8, (3 * sizeof(s32)) + (8 * size));
        omm_array_size(arr) = size;
        omm_array_count(arr) = 0;
        omm_array_capacity(arr) = 8;
        return arr;
    }
    return NULL;
}

void __omm_array_add(OmmArray *parr, const void *item) {
    if (parr && *parr) {
        if (omm_array_count(*parr) == omm_array_capacity(*parr)) {
            s32 size = omm_array_size(*parr);
            s32 count = omm_array_count(*parr);
            s32 capacity = omm_array_capacity(*parr);
            OmmArray arr = OMM_MEMNEW(u8, (3 * sizeof(s32)) + (capacity * 2 * size));
            OMM_MEMCPY(arr, *parr, (3 * sizeof(s32)) + (count * size));
            omm_array_capacity(arr) *= 2;
            OMM_MEMDEL(*parr);
            *parr = arr;
        }
        OMM_MEMCPY(omm_array_at(*parr, omm_array_count(*parr)), item, omm_array_size(*parr));
        omm_array_count(*parr)++;
    }
}

void __omm_array_remove(OmmArray arr, s32 index) {
    if (arr) {
        if (index == -1) {
            omm_array_count(arr) = 0;
        } else {
            OMM_MEMMOV(omm_array_at(arr, index), omm_array_at(arr, index + 1), (omm_array_count(arr) - index - 1) * omm_array_size(arr));
            omm_array_count(arr)--;
        }
    }
}

s32 __omm_array_find(const OmmArray arr, const void *item) {
    if (arr) {
        const u8 *cur = omm_array_at(arr, 0);
        const u8 *end = omm_array_at(arr, omm_array_count(arr));
        for (s32 i = 0; cur < end; ++i, cur += omm_array_size(arr)) {
            if (OMM_MEMCMP(cur, item, omm_array_size(arr))) {
                return i;
            }
        }
    }
    return -1;
}

//
// Memory pools
// Circular buffers allocated on heap once and for all
// Objects can be resized if not enough space
//

typedef struct {
    void *owner;
    void *data;
    s32 size;
} OmmMemoryObject;

typedef struct {
    OmmMemoryObject *objects;
    s32 capacity;
    s32 current;
} OmmMemoryPool;

void *gOmmMemoryPoolStrings;
void *gOmmMemoryPoolGeoData;

static OmmMemoryObject *omm_memory_get_free_slot(OmmMemoryPool *omp) {
    OmmMemoryObject *obj = NULL;
    while (!obj) {
        obj = &omp->objects[omp->current];

        // Check if the owner's oGeoData points to the block of memory
        // If that's the case, it means the block is in use, so skip it
        if (omp == gOmmMemoryPoolGeoData) {
            struct Object *o = (struct Object *) obj->owner;
            if (o && o->oGeoData && o->oGeoData == obj->data) {
                obj = NULL;
            }
        }
        omp->current = (omp->current + 1) % (omp->capacity);
    }
    return obj;
}

void *omm_memory_new(void *pool, s32 size, void *caller) {
    void *ptr = NULL;
    if (pool) {
        OmmMemoryPool *omp = (OmmMemoryPool *) pool;
        OmmMemoryObject *obj = omm_memory_get_free_slot(omp);
        if (obj->size < size) {
            OMM_MEMDEL(obj->data);
            obj->data = OMM_MEMNEW(u8, size);
            obj->size = size;
        } else {
            OMM_MEMSET(obj->data, 0, size);
        }
        obj->owner = caller;
        ptr = obj->data;
    } else {
        ptr = OMM_MEMNEW(u8, size);
    }
    return ptr;
}

static void omm_memory_init_pool(void **pool, s32 capacity) {
    *pool = OMM_MEMNEW(OmmMemoryPool, 1);
    OmmMemoryPool *omp = (OmmMemoryPool *) *pool;
    omp->objects = (OmmMemoryObject *) OMM_MEMNEW(OmmMemoryObject, capacity);
    omp->capacity = capacity;
    omp->current = 0;
}

OMM_AT_STARTUP static void omm_memory_init_pools() {
    omm_memory_init_pool(&gOmmMemoryPoolStrings, 32);
    omm_memory_init_pool(&gOmmMemoryPoolGeoData, 256);
}

// DEBUG DISPLAY FOR COUNTING THE USED SLOTS OF GEO DATA POOL
/*OMM_ROUTINE_UPDATE(aaa) {
    s32 usedCount = 0;
    OmmMemoryPool *omp = (OmmMemoryPool *) gOmmMemoryPoolGeoData;
    for (s32 i = 0; i != omp->capacity; ++i) {
        OmmMemoryObject *obj = &omp->objects[i];
        if (omp == gOmmMemoryPoolGeoData) {
            struct Object *o = (struct Object *) obj->owner;
            if (o && o->oGeoData && o->oGeoData == obj->data) {
                usedCount++;
            }
        }
    }
    OMM_PRINT_TEXT(-60, 24, "%d", omp->current);
    OMM_PRINT_TEXT(-60, 4, "%d I %d", usedCount, omp->capacity);
}*/