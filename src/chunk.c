#include "chunk.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include "light_queue.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

bool wallAmbientOcclusion = true;
bool smoothLighting = true;

#define BLOCK_IS_SOLID_DARK(i) \
    (!(registries[i]->transparent) && (registries[i]->lightLevel <= 0))

Color get_light_color(uint8_t lightValue) {
    unsigned char value = (unsigned char)(((float)lightValue / 15.0f) * 255.0f);
    return (Color) {
        .r = 0,
        .g = 0,
        .b = 0,
        .a = 255 - value
    };
}

unsigned int posmod(int v, int m) {
    int r = v % m;
    return (unsigned int)(r < 0 ? r + m : r);
}

void set_quad_positions(float* positions, uint8_t bx, uint8_t by) {
    float x = bx * TILE_SIZE;
    float y = by * TILE_SIZE;

    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 3;

    float x0 = x;
    float y0 = y;
    float x1 = x + TILE_SIZE;
    float y1 = y + TILE_SIZE;

    float verts[6][3] = {
        {x0, y0, 0.0f},
        {x1, y0, 0.0f},
        {x1, y1, 0.0f},
        {x0, y0, 0.0f},
        {x1, y1, 0.0f},
        {x0, y1, 0.0f},
    };

    for (size_t v = 0; v < 6; v++) {
        positions[voffset + v * 3 + 0] = verts[v][0];
        positions[voffset + v * 3 + 1] = verts[v][1];
        positions[voffset + v * 3 + 2] = verts[v][2];
    }
}

void set_quad_uvs(float* uvs, uint8_t bx, uint8_t by,
    float u0, float v0, float u1, float v1) {
    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 2;

    float verts[6][2] = {
        {u0, v0},
        {u1, v0},
        {u1, v1},
        {u0, v0},
        {u1, v1},
        {u0, v1},
    };

    for (size_t v = 0; v < 6; v++) {
        uvs[voffset + v * 2 + 0] = verts[v][0];
        uvs[voffset + v * 2 + 1] = verts[v][1];
    }
}

void set_quad_colors(unsigned char* colors, uint8_t bx, uint8_t by,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 4;

    uint32_t verts[6][4] = {
        {r, g, b, a},
        {r, g, b, a},
        {r, g, b, a},
        {r, g, b, a},
        {r, g, b, a},
        {r, g, b, a},
    };

    for (size_t v = 0; v < 6; v++) {
        colors[voffset + v * 4 + 0] = verts[v][0];
        colors[voffset + v * 4 + 1] = verts[v][1];
        colors[voffset + v * 4 + 2] = verts[v][2];
        colors[voffset + v * 4 + 3] = verts[v][3];
    }
}

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue) {
    if (!chunk) return;
    if (newLightValue < 1 || newLightValue > 15) return;
    if (startPoint.x < 0 || startPoint.x >= CHUNK_WIDTH) return;
    if (startPoint.y < 0 || startPoint.y >= CHUNK_WIDTH) return;

    uint8_t current = chunk_get_light(chunk, startPoint);
    if (current >= newLightValue) return;

    chunk_set_light(chunk, startPoint, newLightValue);

    uint8_t decayAmount = 1;

    BlockRegistry* br = block_registry_get_block_registry(chunk_get_block(chunk, startPoint, false));
    if (!br->transparent) decayAmount = 4;

    Vector2i neighbors[] = {
        { -1, 0 }, { 1, 0 }, { 0, 1 }, { 0, -1 }
    };

    for (int i = 0; i < 4; i++) {
        Chunk* nextChunk = chunk;

        Vector2i neighPos = {
            .x = startPoint.x + neighbors[i].x,
            .y = startPoint.y + neighbors[i].y
        };

        if (neighPos.x < 0) {
            nextChunk = chunk->neighbors.left;
            neighPos.x = posmod(neighPos.x, CHUNK_WIDTH);
        }
        else if (neighPos.x >= CHUNK_WIDTH) {
            nextChunk = chunk->neighbors.right;
            neighPos.x = posmod(neighPos.x, CHUNK_WIDTH);
        }
        if (neighPos.y < 0) {
            nextChunk = chunk->neighbors.up;
            neighPos.y = posmod(neighPos.y, CHUNK_WIDTH);
        }
        else if (neighPos.y >= CHUNK_WIDTH) {
            nextChunk = chunk->neighbors.down;
            neighPos.y = posmod(neighPos.y, CHUNK_WIDTH);
        }

        if (nextChunk) chunk_fill_light(nextChunk, (Vector2u) { neighPos.x, neighPos.y }, newLightValue - decayAmount);
    }
}

void chunk_calc_lighting(Chunk* chunk) {
    for (int i = 0; i < CHUNK_AREA; i++) chunk->light[i] = 0;
    
    for (int i = 0; i < CHUNK_AREA; i++) {
        if (chunk->blocks[i] == 0 && chunk->walls[i] == 0) {
            chunk_fill_light(chunk, (Vector2u) { (i % CHUNK_WIDTH), (i / CHUNK_WIDTH) }, 15);
        }
    }
}

void chunk_set_block(Chunk* chunk, Vector2u position, uint8_t blockValue, bool isWall) {
    if (!chunk) return;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;

    if (!isWall)
        chunk->blocks[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
    else
        chunk->walls[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
}

uint8_t chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
    if (!chunk) return 0;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return 0;
    if (!isWall)
        return chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
    else
        return chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
}

void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value) {
    if (!chunk) return;
    if (position.x < 0 || position.x >= CHUNK_WIDTH || position.y < 0 || position.y >= CHUNK_WIDTH) return;

    chunk->light[position.x + (position.y * CHUNK_WIDTH)] = value;
}

uint8_t chunk_get_light(Chunk* chunk, Vector2u position) {
    if (!chunk) return 0;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return 0;
    return chunk->light[position.x + (position.y * CHUNK_WIDTH)];
}

void chunk_init_meshes(Chunk* chunk)
{
    if (chunk == NULL) return;
    if (chunk->initializedMeshes == true) return;

    chunk->wallMesh = (Mesh){ 0 };
    chunk->blockMesh = (Mesh){ 0 };

    chunk->wallMesh.triangleCount = chunk->blockMesh.triangleCount = CHUNK_AREA * 2;
    chunk->wallMesh.vertexCount = chunk->wallMesh.triangleCount * 3;
    chunk->blockMesh.vertexCount = chunk->blockMesh.triangleCount * 3;

    chunk->wallMesh.vertices = (float*)MemAlloc(CHUNK_VERTEX_COUNT * 3 * sizeof(float));
    chunk->blockMesh.vertices = (float*)MemAlloc(CHUNK_VERTEX_COUNT * 3 * sizeof(float));

    chunk->wallMesh.texcoords = (float*)MemAlloc(CHUNK_VERTEX_COUNT * 2 * sizeof(float));
    chunk->blockMesh.texcoords = (float*)MemAlloc(CHUNK_VERTEX_COUNT * 2 * sizeof(float));

    chunk->wallMesh.colors = (unsigned char*)MemAlloc(CHUNK_VERTEX_COUNT * 4 * sizeof(unsigned char));
    chunk->blockMesh.colors = (unsigned char*)MemAlloc(CHUNK_VERTEX_COUNT * 4 * sizeof(unsigned char));

    UploadMesh(&chunk->wallMesh, true);
    UploadMesh(&chunk->blockMesh, true);

    chunk->initializedMeshes = true;
}

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    chunk->seed = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);
    srand(chunk->seed);
    for (int i = 0; i < CHUNK_AREA; i++) {
        // World generation
        Vector2i globalBlockPos = {
            chunk->position.x * CHUNK_WIDTH + (i % CHUNK_WIDTH),
            chunk->position.y * CHUNK_WIDTH + (i / CHUNK_WIDTH)
        };

        int sin = (int)floorf(sinf((float)globalBlockPos.x / CHUNK_WIDTH) * CHUNK_WIDTH);

        if (globalBlockPos.y == sin) {
            chunk->blocks[i] = 1;
            chunk->walls[i] = 1;
        } else if (globalBlockPos.y > sin && globalBlockPos.y <= 50) {
            chunk->blocks[i] = 2;
            chunk->walls[i] = 2;
        } else if (globalBlockPos.y > 50) {
            chunk->blocks[i] = 3;
            chunk->walls[i] = 3;
        }
        else {
            chunk->blocks[i] = 0;
            chunk->walls[i] = 0;
        }
        chunk->light[i] = 0;
    }
}

void chunk_genmesh(Chunk* chunk) {
    if (chunk == NULL) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        float uv_unit = (1.0f / 8.0f);

        if (chunk->walls[i] > 0) {
            set_quad_positions(chunk->wallMesh.vertices, x, y);
            set_quad_uvs(chunk->wallMesh.texcoords, x, y, uv_unit * (chunk->walls[i] - 1), 0.0f, (uv_unit * (chunk->walls[i] - 1)) + uv_unit, 1.0f);
            set_quad_colors(chunk->wallMesh.colors, x, y, 128, 128, 128, 255);
        }

        if (chunk->blocks[i] > 0) {
            set_quad_positions(chunk->blockMesh.vertices, x, y);
            set_quad_uvs(chunk->blockMesh.texcoords, x, y, uv_unit * (chunk->blocks[i]-1), 0.0f, (uv_unit * (chunk->blocks[i] - 1)) + uv_unit, 1.0f);
            set_quad_colors(chunk->blockMesh.colors, x, y, 255, 255, 255, 255);
        }
    }

    UpdateMeshBuffer(chunk->wallMesh, 0, chunk->wallMesh.vertices, chunk->wallMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->wallMesh, 1, chunk->wallMesh.texcoords, chunk->wallMesh.vertexCount * 2 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->wallMesh, 3, chunk->wallMesh.colors, chunk->wallMesh.vertexCount * 4 * sizeof(unsigned char), 0);

    UpdateMeshBuffer(chunk->blockMesh, 0, chunk->blockMesh.vertices, chunk->blockMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->blockMesh, 1, chunk->blockMesh.texcoords, chunk->blockMesh.vertexCount * 2 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->blockMesh, 3, chunk->blockMesh.colors, chunk->blockMesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

void chunk_draw(Chunk* chunk, Material* material) {
    if (!chunk) return;

    rlPushMatrix();

    rlTranslatef(
        chunk->position.x * CHUNK_WIDTH * TILE_SIZE,
        chunk->position.y * CHUNK_WIDTH * TILE_SIZE,
        0.0f
    );

    DrawMesh(chunk->wallMesh, *material, MatrixIdentity());
    DrawMesh(chunk->blockMesh, *material, MatrixIdentity());

    /*

    for (int j = 0; j < CHUNK_AREA; j++) {
        const int x = j % CHUNK_WIDTH;
        const int y = (j / CHUNK_WIDTH) % CHUNK_WIDTH;

        const Rectangle blockRect = {
            .x = x * TILE_SIZE,
            .y = y * TILE_SIZE,
            .width = TILE_SIZE,
            .height = TILE_SIZE
        };

        if (chunk->walls[j] > 0) {
            const unsigned int seed = chunk_get_block_seed(chunk, (Vector2u) { x, y }, true);
            BlockRegistry* brg = block_registry_get_block_registry(chunk->walls[j]);
            Rectangle blockTextRect = block_registry_get_block_texture_rect(
                chunk->walls[j],
                brg->flipH && (seed & 1) ? true : false,
                brg->flipV && (seed & 2) ? true : false
            );

            DrawTexturePro(
                *block_registry_get_block_atlas(),
                blockTextRect,
                blockRect,
                Vector2Zero(),
                0.0f,
                GRAY
            );

            if (brg->lightLevel <= 0 && wallAmbientOcclusion) {
                // Wall "ambient occlusion"
                uint8_t neighbors[8] = {
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x,     y - 1 }, false),   // Up
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1, y     }, false),   // Right
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x,     y + 1 }, false),   // Down
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1, y     }, false),   // Left

                    chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1,  y - 1 }, false),  // Up left
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1,  y - 1 }, false),  // Up right
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1,  y + 1 }, false),  // Down left
                    chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1,  y + 1 }, false),  // Down right
                };

                BlockRegistry* registries[8];
                for (int i = 0; i < 8; i++)
                    registries[i] = block_registry_get_block_registry(neighbors[i]);

                const Color fadeColor = { 0, 0, 0, 128 };

                Color topLeft = { 0, 0, 0, 0 };
                Color bottomLeft = { 0, 0, 0, 0 };
                Color topRight = { 0, 0, 0, 0 };
                Color bottomRight = { 0, 0, 0, 0 };

                if (BLOCK_IS_SOLID_DARK(0)) topLeft = topRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(1)) topRight = bottomRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(2)) bottomLeft = bottomRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(3)) topLeft = bottomLeft = fadeColor;

                if (BLOCK_IS_SOLID_DARK(4)) topLeft = fadeColor;
                if (BLOCK_IS_SOLID_DARK(5)) topRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(6)) bottomLeft = fadeColor;
                if (BLOCK_IS_SOLID_DARK(7)) bottomRight = fadeColor;

                DrawRectangleGradientEx(
                    blockRect,
                    topLeft,
                    bottomLeft,
                    bottomRight,
                    topRight
                );
            }
        }

        if (chunk->blocks[j] > 0) {
            unsigned int seed = chunk_get_block_seed(chunk, (Vector2u) { x, y }, false);
            BlockRegistry* brg = block_registry_get_block_registry(chunk->blocks[j]);
            Rectangle blockTextRect = block_registry_get_block_texture_rect(
                chunk->blocks[j],
                brg->flipH && (seed & 1) ? true : false,
                brg->flipV && (seed & 2) ? true : false
            );

            DrawTexturePro(
                *block_registry_get_block_atlas(),
                blockTextRect,
                blockRect,
                Vector2Zero(),
                0.0f,
                WHITE
            );
        }

        // Drawing light
        if (chunk->blocks[j] > 0 || chunk->walls[j] > 0) {
            if (!smoothLighting) {
                // Without smooth lighting, just draw some simple squares
                DrawRectangle(
                    x * TILE_SIZE,
                    y * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE,
                    get_light_color(chunk->light[j])
                );
            } else {
                // Smooth lighting gets neighboring light values
                uint8_t neighbors[8] = {
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x,     y - 1 }),   // Up
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x + 1, y     }),   // Right
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x,     y + 1 }),   // Down
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x - 1, y     }),   // Left
                              
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x - 1, y - 1 }),   // Up left
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x + 1, y - 1 }),   // Up right
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x - 1, y + 1 }),   // Down left
                    chunk_get_light_extrapolating(chunk, (Vector2i) { x + 1, y + 1 }),   // Down right
                };

                float topLeftAverage = (
                    (float)chunk->light[j] +
                    (float)neighbors[3] +
                    (float)neighbors[4] +
                    (float)neighbors[0]
                ) / 4.0f;
                Color topLeft = get_light_color(topLeftAverage);

                float bottomLeftAverage = (
                    (float)chunk->light[j] +
                    (float)neighbors[3] +
                    (float)neighbors[6] +
                    (float)neighbors[2]
                ) / 4.0f;
                Color bottomLeft = get_light_color(bottomLeftAverage);

                float topRightAverage = (
                    (float)chunk->light[j] +
                    (float)neighbors[1] +
                    (float)neighbors[5] +
                    (float)neighbors[0]
                ) / 4.0f;
                Color topRight = get_light_color(topRightAverage);

                float bottomRightAverage = (
                    (float)chunk->light[j] +
                    (float)neighbors[1] +
                    (float)neighbors[7] +
                    (float)neighbors[2]
                ) / 4.0f;
                Color bottomRight = get_light_color(bottomRightAverage);

                DrawRectangleGradientEx(
                    blockRect,
                    topLeft,
                    bottomLeft,
                    bottomRight,
                    topRight
                );
            }
        }
    }
    */

    rlPopMatrix();
}

void chunk_free_meshes(Chunk* chunk)
{
    if (!chunk) return;

    UnloadMesh(chunk->wallMesh);
    UnloadMesh(chunk->blockMesh);
}

unsigned int chunk_get_block_seed(Chunk* chunk, Vector2u position, bool isWall) {
    unsigned int h = chunk->seed;
    h ^= position.x * 374761393u;
    h ^= position.y * 668265263u;
    h ^= (unsigned int)isWall * 1442695040888963407ull;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h;
}

uint8_t chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall) {
    if (!chunk) return 0;

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        if (!isWall)
            return chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
        else
            return chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
    }
    else {
        Chunk* neighbor = NULL;

        if (position.x < 0 && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upLeft;
        else if (position.x >= CHUNK_WIDTH && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upRight;
        else if (position.x < 0 && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downLeft;
        else if (position.x >= CHUNK_WIDTH && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downRight;
        else if (position.x < 0) neighbor = (Chunk*)chunk->neighbors.left;
        else if (position.x >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.right;
        else if (position.y < 0) neighbor = (Chunk*)chunk->neighbors.up;
        else if (position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.down;

        if (neighbor == NULL) return 0;

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        if (!isWall)
            return neighbor->blocks[relPos.x + (relPos.y * CHUNK_WIDTH)];
        else
            return neighbor->walls[relPos.x + (relPos.y * CHUNK_WIDTH)];
    }
}

uint8_t chunk_get_light_extrapolating(Chunk* chunk, Vector2i position)
{
    if (!chunk) return 0;

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        return chunk->light[position.x + (position.y * CHUNK_WIDTH)];
    }
    else {
        Chunk* neighbor = NULL;

        if (position.x < 0 && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upLeft;
        else if (position.x >= CHUNK_WIDTH && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upRight;
        else if (position.x < 0 && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downLeft;
        else if (position.x >= CHUNK_WIDTH && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downRight;
        else if (position.x < 0) neighbor = (Chunk*)chunk->neighbors.left;
        else if (position.x >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.right;
        else if (position.y < 0) neighbor = (Chunk*)chunk->neighbors.up;
        else if (position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.down;

        if (neighbor == NULL) return 0;

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        return neighbor->light[relPos.x + (relPos.y * CHUNK_WIDTH)];
    }
}
