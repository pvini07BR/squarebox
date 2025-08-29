#include "block_registry.h"
#include "block_models.h"
#include "defines.h"

#include <stdlib.h>
#include <stdio.h>

static BlockRegistry* blockRegistry = NULL;

void block_registry_init() {
    blockRegistry = calloc(BLOCK_COUNT, sizeof(BlockRegistry));
    if (blockRegistry == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for the block registry.\n");
        return;
    };

    blockRegistry[BLOCK_AIR] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_DIRT] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 1, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 2, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 3, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 4, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOOD_LOG] = (BlockRegistry){
        .variant_count = 2,
        .variants = { 
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 1 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_LEAVES] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 6, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_GLASS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 7, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_LAMP] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 8, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 15,
    };

    blockRegistry[BLOCK_CHEST] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 9, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .trait = BLOCK_TRAIT_CONTAINER,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_DIRT_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 2, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &blockRegistry[idx];
}

BlockVariant br_get_block_variant(size_t reg_idx, size_t variant_idx) {
    BlockRegistry* br = br_get_block_registry(reg_idx);
    if (br == NULL) return (BlockVariant){ 0, 0, 0, 0, 0 };
    if (variant_idx >= br->variant_count) return (BlockVariant){ 0, 0, 0, 0, 0 };
    return br->variants[variant_idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
