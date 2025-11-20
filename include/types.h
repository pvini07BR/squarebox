#ifndef TYPES_H
#define TYPES_H

#define TILE_SIZE 32

#define CHUNK_WIDTH 16
#define CHUNK_AREA CHUNK_WIDTH*CHUNK_WIDTH

#define GAMEPAD_STICK_DEADZONE 0.1f

#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

typedef enum {
	BLOCK_MODEL_QUAD,
	BLOCK_MODEL_SLAB,
	BLOCK_MODEL_STAIRS,
	BLOCK_MODEL_NUB,
	BLOCK_MODEL_TORCH,
	BLOCK_MODEL_TORCH_WALL_RIGHT,
	BLOCK_MODEL_TORCH_WALL_LEFT,
	BLOCK_MODEL_COUNT
} BlockModelEnum;

typedef enum {
	ATLAS_GRASS_BLOCK,
	ATLAS_DIRT,
	ATLAS_SAND,
	ATLAS_STONE,
	ATLAS_COBBLESTONE,
	ATLAS_WOODEN_PLANKS,
	ATLAS_WOOD_LOG,
	ATLAS_LEAVES,
	ATLAS_GLASS,
	ATLAS_LAMP,
	ATLAS_CHEST,
	ATLAS_BOUNCY_BLOCK,
	ATLAS_ICE,
	ATLAS_WOOL,
	ATLAS_ORANGE_WOOL,
	ATLAS_MAGENTA_WOOL,
	ATLAS_LIGHT_BLUE_WOOL,
	ATLAS_YELLOW_WOOL,
	ATLAS_LIME_WOOL,
	ATLAS_PINK_WOOL,
	ATLAS_GRAY_WOOL,
	ATLAS_LIGHT_GRAY_WOOL,
	ATLAS_CYAN_WOOL,
	ATLAS_PURPLE_WOOL,
	ATLAS_BLUE_WOOL,
	ATLAS_BROWN_WOOL,
	ATLAS_GREEN_WOOL,
	ATLAS_RED_WOOL,
	ATLAS_BLACK_WOOL,
	ATLAS_GRASS,
	ATLAS_FLOWER,
	ATLAS_PEBBLES,
	ATLAS_WOODEN_FENCE,
	ATLAS_LADDERS,
	ATLAS_FRAME,
	ATLAS_SLAB_FRAME,
	ATLAS_STAIRS_FRAME,
	ATLAS_NUB_FRAME,
	ATLAS_TRAPDOOR,
	ATLAS_SIGN,
	ATLAS_TORCH,
	ATLAS_WATER_BUCKET,
	ATLAS_POWER_WIRE,
	ATLAS_BATTERY,
	ATLAS_POWER_REPEATER,
	ATLAS_POWERED_LAMP,
	ATLAS_COUNT
} TextureAtlasEnum;

typedef enum {
	CHUNK_LAYER_BACKGROUND,
	CHUNK_LAYER_FOREGROUND,
	CHUNK_LAYER_COUNT
} ChunkLayerEnum;

typedef enum {
	BLOCK_COLLIDER_QUAD,
	BLOCK_COLLIDER_SLAB,
	BLOCK_COLLIDER_STAIRS,
	BLOCK_COLLIDER_NUB,
	BLOCK_COLLIDER_TRAPDOOR,
	BLOCK_COLLIDER_COUNT
} BlockColliderEnum;

typedef struct {
	unsigned int x;
	unsigned int y;
} Vector2u;

typedef struct {
	int x;
	int y;
} Vector2i;

typedef struct {
	// ID Number in the Block Registry.
	uint8_t id;
	// Member variable used for block states.
	// Its usage depends on what block it is.
	// it can be used as a simple number, or use bitwise operations
	// to achieve multiple states.
	uint8_t state;
	// Pointer to a external data. Set NULL if the block
	// does not have an associated external data.
	void* data;
} BlockInstance;

typedef struct {
	// Index in the atlas texture to use.
	// See texture_atlas.c.
	TextureAtlasEnum atlas_idx;
	// Variant of the specified atlas texture.
	// set to 1 if there is only one variant.
	uint8_t atlas_variant;
	// Index of the model in the block models array.
	// see block_models.c.
	BlockModelEnum model_idx;
	// Index of the collider in the block colliders array.
	// see block_colliders.c.
	BlockColliderEnum collider_idx;
	// Determines what rotation the model should have in this block state.
	// goes from 0 to 3, representing rotations in a anti-clockwise order
	// by 90 degrees each.
	uint8_t rotation;
	// Locks the UV and does not let the texture rotate with the model.
	bool uv_lock;
	// Color tint that will be applied to the block.
	Color tint;
} BlockVariant;

typedef struct {
	Font font;
	const char* str;
	Vector2 bounds;
	float fontSize;
	float spacing;
} Label;

typedef enum {
	LABEL_ALIGN_BEGIN,
	LABEL_ALIGN_CENTER,
	LABEL_ALIGN_END
} LabelAlignment;

Label create_label(const char* str, float fontSize, float spacing, Font font);
void draw_label(Label* label, Vector2 pos, float rotation, float scale, Color color, LabelAlignment horizontal_alignment, LabelAlignment vertical_alignment);

#endif