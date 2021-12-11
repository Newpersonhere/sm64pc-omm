#ifndef OMM_MEMORY_H
#define OMM_MEMORY_H

#include "data/omm/omm_includes.h"

//
// OMM Array: Dynamically allocated array
//

typedef void *OmmArray;

OmmArray __omm_array_new(s32 size);
void __omm_array_add(OmmArray *parr, const void *item);
void __omm_array_remove(OmmArray arr, s32 index);
s32 __omm_array_find(const OmmArray arr, const void *item);

#define omm_array_new(type)                                 __omm_array_new(sizeof(type))
#define omm_array_init(arr, type)                           { if (!arr) { arr = omm_array_new(type); } }
#define omm_array_delete(arr)                               OMM_MEMDEL(arr)
#define omm_array_size(arr)                                 (((s32 *) arr)[0])
#define omm_array_count(arr)                                (((s32 *) arr)[1])
#define omm_array_capacity(arr)                             (((s32 *) arr)[2])
#define omm_array_data(arr)                                 (((u8 *) arr) + (3 * sizeof(s32)))
#define omm_array_at(arr, index)                            (((u8 *) arr) + (3 * sizeof(s32)) + ((index) * omm_array_size(arr)))
#define omm_array_add(arr, item)                            __omm_array_add(&(arr), &(item))
#define omm_array_remove(arr, index)                        __omm_array_remove(arr, index)
#define omm_array_clear(arr)                                __omm_array_remove(arr, -1)
#define omm_array_find(arr, item)                           __omm_array_find(arr, &(item))
#define omm_array_get(arr, type, index)                    (*((type *) omm_array_at(arr, index)))
#define omm_array_getp(arr, type, index)                     ((type *) omm_array_at(arr, index))
#define omm_array_set(arr, item, index)                     OMM_MEMCPY(omm_array_at(arr, index), &(item), omm_array_size(arr))
#define omm_array_add_inplace(arr, type, ...)               { type __inplace__ = __VA_ARGS__; omm_array_add(arr, __inplace__); }
#define omm_array_set_inplace(arr, index, type, ...)        { type __inplace__ = __VA_ARGS__; omm_array_set(arr, __inplace__, index); }
#define omm_array_grow(arr, count, type, ...)               { omm_array_init(arr, type); while (omm_array_count(arr) < (count)) { omm_array_add_inplace(arr, type, __VA_ARGS__); } }
#define omm_array_for_each(arr, type, item)                 s32 index_##item = (arr ? 0 : -1); for (type *item = omm_array_getp(arr, type, 0); index_##item != -1 && index_##item != omm_array_count(arr); ++index_##item, item++)

//
// Memory pools
//

extern void *gOmmMemoryPoolStrings;
extern void *gOmmMemoryPoolGeoData;
void *omm_memory_new(void *pool, s32 size, void *caller);

#endif // OMM_MEMORY_H