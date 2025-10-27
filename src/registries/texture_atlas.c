#include "registries/texture_atlas.h"

#include "thirdparty/stb_image.h"

#include <raymath.h>

static AtlasEntry entries[ATLAS_COUNT];
static Texture2D generatedAtlas;
static Material material;

void texture_atlas_init() {
    entries[ATLAS_GRASS_BLOCK] = (AtlasEntry){
        .fileName = "grass_block.png",
        .variantCount = 1
    };

    entries[ATLAS_DIRT] = (AtlasEntry){
        .fileName = "dirt.png",
        .variantCount = 1
    };

    entries[ATLAS_SAND] = (AtlasEntry){
        .fileName = "sand.png",
        .variantCount = 1
    };

    entries[ATLAS_STONE] = (AtlasEntry){
        .fileName = "stone.png",
        .variantCount = 1
    };

    entries[ATLAS_COBBLESTONE] = (AtlasEntry){
        .fileName = "cobblestone.png",
        .variantCount = 1
    };

    entries[ATLAS_WOODEN_PLANKS] = (AtlasEntry){
        .fileName = "wooden_planks.png",
        .variantCount = 1
    };

    entries[ATLAS_WOOD_LOG] = (AtlasEntry){
        .fileName = "wood_log.png",
        .variantCount = 2
    };

    entries[ATLAS_LEAVES] = (AtlasEntry){
        .fileName = "leaves.png",
        .variantCount = 1
    };

    entries[ATLAS_GLASS] = (AtlasEntry){
        .fileName = "glass.png",
        .variantCount = 1
    };

    entries[ATLAS_LAMP] = (AtlasEntry){
        .fileName = "lamp.png",
        .variantCount = 1
    };

    entries[ATLAS_CHEST] = (AtlasEntry){
        .fileName = "chest.png",
        .variantCount = 1
    };

    entries[ATLAS_BOUNCY_BLOCK] = (AtlasEntry){
        .fileName = "bouncy_block.png",
        .variantCount = 1
    };

    entries[ATLAS_ICE] = (AtlasEntry){
        .fileName = "ice.png",
        .variantCount = 1
    };

    entries[ATLAS_WOOL] = (AtlasEntry){
        .fileName = "wool.png",
        .variantCount = 1
    };

    entries[ATLAS_ORANGE_WOOL] = (AtlasEntry){
        .fileName = "orange_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_MAGENTA_WOOL] = (AtlasEntry){
        .fileName = "magenta_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_LIGHT_BLUE_WOOL] = (AtlasEntry){
        .fileName = "light_blue_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_YELLOW_WOOL] = (AtlasEntry){
        .fileName = "yellow_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_LIME_WOOL] = (AtlasEntry){
        .fileName = "lime_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_PINK_WOOL] = (AtlasEntry){
        .fileName = "pink_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_GRAY_WOOL] = (AtlasEntry){
        .fileName = "gray_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_LIGHT_GRAY_WOOL] = (AtlasEntry){
        .fileName = "light_gray_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_CYAN_WOOL] = (AtlasEntry){
        .fileName = "cyan_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_PURPLE_WOOL] = (AtlasEntry){
        .fileName = "purple_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_BLUE_WOOL] = (AtlasEntry){
        .fileName = "blue_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_BROWN_WOOL] = (AtlasEntry){
        .fileName = "brown_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_GREEN_WOOL] = (AtlasEntry){
        .fileName = "green_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_RED_WOOL] = (AtlasEntry){
        .fileName = "red_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_BLACK_WOOL] = (AtlasEntry){
        .fileName = "black_wool.png",
        .variantCount = 1
    };

    entries[ATLAS_GRASS] = (AtlasEntry){
        .fileName = "grass.png",
        .variantCount = 1
    };

    entries[ATLAS_FLOWER] = (AtlasEntry){
        .fileName = "flower.png",
        .variantCount = 1
    };

    entries[ATLAS_PEBBLES] = (AtlasEntry){
        .fileName = "pebbles.png",
        .variantCount = 1
    };

    entries[ATLAS_WOODEN_FENCE] = (AtlasEntry){
        .fileName = "wooden_fence.png",
        .variantCount = 8
    };

    entries[ATLAS_LADDERS] = (AtlasEntry){
        .fileName = "ladders.png",
        .variantCount = 1
    };

    entries[ATLAS_FRAME] = (AtlasEntry){
        .fileName = "frame.png",
        .variantCount = 1
    };

    entries[ATLAS_SLAB_FRAME] = (AtlasEntry){
        .fileName = "slab_frame.png",
        .variantCount = 1
    };

    entries[ATLAS_STAIRS_FRAME] = (AtlasEntry){
        .fileName = "stairs_frame.png",
        .variantCount = 1
    };

    entries[ATLAS_NUB_FRAME] = (AtlasEntry){
        .fileName = "nub_frame.png",
        .variantCount = 1
    };

    entries[ATLAS_TRAPDOOR] = (AtlasEntry){
        .fileName = "trapdoor.png",
        .variantCount = 1
    };

    entries[ATLAS_SIGN] = (AtlasEntry){
        .fileName = "sign.png",
        .variantCount = 2
    };

    entries[ATLAS_TORCH] = (AtlasEntry){
        .fileName = "torch.png",
        .variantCount = 1
    };

    entries[ATLAS_WATER_BUCKET] = (AtlasEntry){
        .fileName = "water_bucket.png",
        .variantCount = 1
    };

    entries[ATLAS_POWER_WIRE] = (AtlasEntry){
        .fileName = "power_wire.png",
        .variantCount = 16
    };

    Image generated = GenImageColor(1, TILE_SIZE, (Color) { 0, 0, 0, 0 });
    Rectangle cursor = { 0, 0, 0, 0 };
    uint8_t offset = 0;

    for (int i = 0; i < ATLAS_COUNT; i++) {
        entries[i].offset = offset;

        Image img = LoadImage(TextFormat(ASSETS_PATH "blocks/%s", entries[i].fileName));
        cursor.width = img.width;
        cursor.height = img.height;

        ImageResizeCanvas(&generated, generated.width + img.width, generated.height, 0, 0, (Color) { 0, 0, 0, 0 });

        ImageDraw(&generated, img, (Rectangle) { 0, 0, img.width, img.height }, cursor, WHITE);

        cursor.x += img.width;
        offset += entries[i].variantCount;

        UnloadImage(img);
    }

    ImageResizeCanvas(&generated, generated.width - 1, generated.height, 0, 0, (Color) { 0, 0, 0, 0 });

    generatedAtlas = LoadTextureFromImage(generated);
    SetTextureWrap(generatedAtlas, TEXTURE_WRAP_CLAMP);

    material = LoadMaterialDefault();
    LoadMaterialDefault();
    SetMaterialTexture(&material, MATERIAL_MAP_ALBEDO, generatedAtlas);

    UnloadImage(generated);
}

Texture2D texture_atlas_get() { return generatedAtlas; }

Rectangle texture_atlas_get_rect(TextureAtlasEnum atlas, uint8_t variantIdx) {
    return (Rectangle) {
        .x = (entries[atlas].offset + variantIdx) * TILE_SIZE,
        .y = 0.0f,
        .width = TILE_SIZE,
        .height = TILE_SIZE
    };
}

Vector2 texture_atlas_get_uv(TextureAtlasEnum atlas, uint8_t variantIdx, Vector2 relativePoint, bool flipH, bool flipV) {
    float atlas_unit = 1.0f / generatedAtlas.width;
    float unit = atlas_unit * TILE_SIZE;
    int idx = entries[atlas].offset + variantIdx;
    float p = idx * unit;

    if (flipH) relativePoint.x = 1.0f - (relativePoint.x + 0.00001f);
    if (flipV) relativePoint.y = 1.0f - relativePoint.y;

    return (Vector2) {
        .x = Lerp(p + (atlas_unit * 0.01f), p + unit, relativePoint.x),
        .y = Lerp(0.0f, 1.0f, relativePoint.y)
    };
}

Material texture_atlas_get_material()
{
    return material;
}

void texture_atlas_free()
{
    UnloadMaterial(material);
    UnloadTexture(generatedAtlas);
}