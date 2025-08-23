#include "chunk.h"
#include "defines.h"
#include "block_registry.h"
#include "texture_atlas.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <string.h>

#define FNL_IMPL
#include <thirdparty/FastNoiseLite.h>

int seed = 0;
bool wallAmbientOcclusion = true;
bool smoothLighting = true;
unsigned int wallBrightness = 128;
unsigned int wallAOvalue = 64;

#define BLOCK_IS_SOLID_DARK(i) \
    (!(registries[i]->transparent) && (registries[i]->lightLevel <= 0))

unsigned int posmod(int v, int m) {
    int r = v % m;
    return (unsigned int)(r < 0 ? r + m : r);
}

void set_quad_positions(float* positions, uint8_t bx, uint8_t by, bool flipTriangles) {
    float x = bx * TILE_SIZE;
    float y = by * TILE_SIZE;

    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 3;

    float x0 = x;
    float y0 = y;
    float x1 = x + TILE_SIZE;
    float y1 = y + TILE_SIZE;

    float verts[6][3];

    if (flipTriangles) {
        float invertedVerts[6][3] = {
            {x0, y0, 0.0f},
            {x1, y0, 0.0f},
            {x0, y1, 0.0f},
            {x0, y1, 0.0f},
            {x1, y1, 0.0f},
            {x1, y0, 0.0f},
        };
        memcpy(verts, invertedVerts, sizeof(verts));
    }
    else {
        float normalVerts[6][3] = {
            {x0, y0, 0.0f},
            {x1, y0, 0.0f},
            {x1, y1, 0.0f},
            {x0, y0, 0.0f},
            {x1, y1, 0.0f},
            {x0, y1, 0.0f},
        };
        memcpy(verts, normalVerts, sizeof(verts));
    }

    for (size_t v = 0; v < 6; v++) {
        positions[voffset + v * 3 + 0] = verts[v][0];
        positions[voffset + v * 3 + 1] = verts[v][1];
        positions[voffset + v * 3 + 2] = verts[v][2];
    }
}

void set_quad_uvs(float* uvs, uint8_t bx, uint8_t by, Rectangle rect, bool flipTriangles, uint8_t rotation) {
    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 2;

    float u0 = rect.x;
    float v0 = rect.y;
    float u1 = rect.x + rect.width;
    float v1 = rect.y + rect.height;

    float cornerUVs[4][2];

    switch (rotation) {
    case 1: // 90°
        cornerUVs[0][0] = u1; cornerUVs[0][1] = v0;
        cornerUVs[1][0] = u1; cornerUVs[1][1] = v1;
        cornerUVs[2][0] = u0; cornerUVs[2][1] = v1;
        cornerUVs[3][0] = u0; cornerUVs[3][1] = v0;
        break;

    case 2: // 180°
        cornerUVs[0][0] = u1; cornerUVs[0][1] = v1;
        cornerUVs[1][0] = u0; cornerUVs[1][1] = v1;
        cornerUVs[2][0] = u0; cornerUVs[2][1] = v0;
        cornerUVs[3][0] = u1; cornerUVs[3][1] = v0;
        break;

    case 3: // 270°
        cornerUVs[0][0] = u0; cornerUVs[0][1] = v1;
        cornerUVs[1][0] = u0; cornerUVs[1][1] = v0;
        cornerUVs[2][0] = u1; cornerUVs[2][1] = v0;
        cornerUVs[3][0] = u1; cornerUVs[3][1] = v1;
        break;

    case 0:
    default:
        cornerUVs[0][0] = u0; cornerUVs[0][1] = v0;
        cornerUVs[1][0] = u1; cornerUVs[1][1] = v0;
        cornerUVs[2][0] = u1; cornerUVs[2][1] = v1;
        cornerUVs[3][0] = u0; cornerUVs[3][1] = v1;
        break;
    }

    float verts[6][2];

    if (flipTriangles) {
        float invertedVerts[6][2] = {
            {cornerUVs[0][0], cornerUVs[0][1]},
            {cornerUVs[1][0], cornerUVs[1][1]},
            {cornerUVs[3][0], cornerUVs[3][1]},
            {cornerUVs[3][0], cornerUVs[3][1]},
            {cornerUVs[2][0], cornerUVs[2][1]},
            {cornerUVs[1][0], cornerUVs[1][1]},
        };
        memcpy(verts, invertedVerts, sizeof(verts));
    }
    else {
        float normalVerts[6][2] = {
            {cornerUVs[0][0], cornerUVs[0][1]},
            {cornerUVs[1][0], cornerUVs[1][1]},
            {cornerUVs[2][0], cornerUVs[2][1]},
            {cornerUVs[0][0], cornerUVs[0][1]},
            {cornerUVs[2][0], cornerUVs[2][1]},
            {cornerUVs[3][0], cornerUVs[3][1]},
        };
        memcpy(verts, normalVerts, sizeof(verts));
    }

    for (size_t v = 0; v < 6; v++) {
        uvs[voffset + v * 2 + 0] = verts[v][0];
        uvs[voffset + v * 2 + 1] = verts[v][1];
    }
}

void set_quad_colors(unsigned char* colors, uint8_t bx, uint8_t by, Color corners[4], bool flipTriangles) {
    size_t i = bx + (by * CHUNK_WIDTH);
    size_t voffset = i * 6 * 4;

    int indices[6];

    if (flipTriangles) {
        int invertedIndices[6] = { 0, 1, 3, 3, 2, 1 };
        memcpy(indices, invertedIndices, sizeof(indices));
    }
    else {
        int normalIndices[6] = { 0, 1, 2, 0, 2, 3 };
        memcpy(indices, normalIndices, sizeof(indices));
    }

    for (size_t v = 0; v < 6; v++) {
        Color c = corners[indices[v]];
        colors[voffset + v * 4 + 0] = c.r;
        colors[voffset + v * 4 + 1] = c.g;
        colors[voffset + v * 4 + 2] = c.b;
        colors[voffset + v * 4 + 3] = 255;
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

    BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, startPoint, false).id);
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

void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, bool isWall) {
    if (!chunk) return;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;

    if (!isWall)
        chunk->blocks[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
    else
        chunk->walls[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
}

BlockInstance chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
    if (!chunk) return (BlockInstance){ 0, 0 };
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return (BlockInstance) { 0, 0 };
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

void build_quad(Chunk* chunk, BlockInstance* blocks, Mesh* mesh, bool isWall, uint8_t x, uint8_t y, uint8_t brightness) {
    int i = x + (y * CHUNK_WIDTH);
    if (blocks[i].id <= 0) return;
    
    // Start by brightness value
    uint8_t cornerValues[4] = { brightness, brightness, brightness, brightness };

    // Flipping the block texture when requested
    unsigned int h = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);
    h ^= x * 374761393u;
    h ^= y * 668265263u;
    h ^= (unsigned int)isWall * 1442695040888963407ull;
    h = (h ^ (h >> 13)) * 1274126177u;

    BlockRegistry* brg = br_get_block_registry(blocks[i].id);

    bool flipH = brg->flipH && (h & 1) ? true : false;
    bool flipV = brg->flipV && (h & 2) ? true : false;

    bool flipTriangles = false;

    if (!smoothLighting) {
        uint8_t lightValue = (uint8_t)((chunk->light[i] / 15.0f) * 255.0f);
        uint8_t reduction = 255 - lightValue;

        for (int i = 0; i < 4; i++) {
            if (cornerValues[i] > reduction) cornerValues[i] -= reduction;
            else cornerValues[i] = 0;
        }
    } else {
        uint8_t neighbors[8];
        chunk_get_light_neighbors_with_corners(chunk, (Vector2u) { x, y }, neighbors);

        // 0 = Top Left
        // 1 = Top Right
        // 2 = Bottom Right
        // 3 = Bottom Left

        int cornerNeighbors[4][3] = {
            {NEIGHBOR_LEFT, NEIGHBOR_TOP_LEFT, NEIGHBOR_TOP},           // Top Left
            {NEIGHBOR_RIGHT, NEIGHBOR_TOP_RIGHT, NEIGHBOR_TOP},         // Top Right
            {NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM_RIGHT, NEIGHBOR_BOTTOM},   // Bottom Right
            {NEIGHBOR_LEFT, NEIGHBOR_BOTTOM_LEFT, NEIGHBOR_BOTTOM}      // Bottom Left
        };

        for (int corner = 0; corner < 4; corner++) {
            float lightSum = (float)chunk->light[i];
            for (int n = 0; n < 3; n++) {
                lightSum += (float)neighbors[cornerNeighbors[corner][n]];
            }
            float average = lightSum / 4.0f;

            uint8_t lightValue = (uint8_t)((average / 15.0f) * 255.0f);

            uint8_t reduction = 255 - lightValue;
            if (cornerValues[corner] > reduction) cornerValues[corner] -= reduction;
            else cornerValues[corner] = 0;
        }
    }

    // Wall "ambient occulsion" for walls only
    if (wallAmbientOcclusion && isWall) {
        BlockInstance neighbors[8];
        chunk_get_block_neighbors_with_corners(chunk, (Vector2u) { x, y }, false, neighbors);

        BlockRegistry* registries[8];
        for (int i = 0; i < 8; i++) registries[i] = br_get_block_registry(neighbors[i].id);

        struct {
            int corners[2];
            bool flipTri;
        } aoRules[8] = {
            {{0, 1}, false},    // Top
            {{1, 2}, false},    // Right
            {{2, 3}, false},    // Bottom
            {{0, 3}, false},    // Left

            {{0, -1}, true},    // Top Left
            {{1, -1}, false},   // Top Right
            {{2, -1}, true},    // Bottom Right
            {{3, -1}, false}    // Bottom Left
        };

        for (int dir = 0; dir < 8; dir++) {
            if (BLOCK_IS_SOLID_DARK(dir)) {
                for (int c = 0; c < 2; c++) {
                    int corner = aoRules[dir].corners[c];
                    if (corner >= 0) {
                        cornerValues[corner] = fminf(cornerValues[corner], wallAOvalue);
                    }
                }

                flipTriangles = aoRules[dir].flipTri;
            }
        }
    }

    Color colors[4];
    for (int i = 0; i < 4; i++) {
        colors[i] = (Color){
            .r = cornerValues[i],
            .g = cornerValues[i],
            .b = cornerValues[i],
            .a = 255
        };
    }

    // Block state rendering
    uint8_t uvRotation = 0;

    if (brg->flag == BLOCK_FLAG_LOG_LIKE) {
        uvRotation = blocks[i].state;
    }

    set_quad_positions(mesh->vertices, x, y, flipTriangles);
    set_quad_uvs(mesh->texcoords, x, y, texture_atlas_get_uv(brg->atlas_idx, flipH, flipV), flipTriangles, uvRotation);
    set_quad_colors(mesh->colors, x, y, colors, flipTriangles);
}

void chunk_genmesh(Chunk* chunk) {
    if (chunk == NULL) return;

    for (int i = 0; i < (CHUNK_VERTEX_COUNT * 3); i++) {
        chunk->wallMesh.vertices[i] = 0.0f;
        chunk->blockMesh.vertices[i] = 0.0f;
    }

    for (int i = 0; i < CHUNK_AREA; i++) {
        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        build_quad(chunk, chunk->walls, &chunk->wallMesh, true, x, y, wallBrightness);
        build_quad(chunk, chunk->blocks, &chunk->blockMesh, false, x, y, 255);
    }

    UpdateMeshBuffer(chunk->wallMesh, 0, chunk->wallMesh.vertices, chunk->wallMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->wallMesh, 1, chunk->wallMesh.texcoords, chunk->wallMesh.vertexCount * 2 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->wallMesh, 3, chunk->wallMesh.colors, chunk->wallMesh.vertexCount * 4 * sizeof(unsigned char), 0);

    UpdateMeshBuffer(chunk->blockMesh, 0, chunk->blockMesh.vertices, chunk->blockMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->blockMesh, 1, chunk->blockMesh.texcoords, chunk->blockMesh.vertexCount * 2 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->blockMesh, 3, chunk->blockMesh.colors, chunk->blockMesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

void chunk_free_meshes(Chunk* chunk)
{
    if (!chunk) return;

    UnloadMesh(chunk->wallMesh);
    UnloadMesh(chunk->blockMesh);
}

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    fnl_state noise = fnlCreateState();
    noise.seed = seed;
    noise.frequency = -0.025f;
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.fractal_type = FNL_FRACTAL_FBM;

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        float value = fnlGetNoise2D(&noise, (chunk->position.x * CHUNK_WIDTH) + x, 0.0f);
        value /= 2.0f;
        value += 0.5;
        value *= (CHUNK_WIDTH * 2);
        value = round(value);

        for (int y = 0; y < CHUNK_WIDTH; y++) {
            int i = x + (y * CHUNK_WIDTH);
            Vector2i globalBlockPos = {
                chunk->position.x * CHUNK_WIDTH + (i % CHUNK_WIDTH),
                chunk->position.y * CHUNK_WIDTH + (i / CHUNK_WIDTH)
            };

            if (globalBlockPos.y == value) {
                chunk->blocks[i] = (BlockInstance){ 1, 0 };
                chunk->walls[i] = (BlockInstance){ 1, 0 };
            }
            else if (globalBlockPos.y > value && globalBlockPos.y <= (value + 16)) {
                chunk->blocks[i] = (BlockInstance){ 2, 0 };
                chunk->walls[i] = (BlockInstance){ 2, 0 };
            } 
            else if (globalBlockPos.y > (value + 16)) {
                chunk->blocks[i] = (BlockInstance){ 3, 0 };
                chunk->walls[i] = (BlockInstance){ 3, 0 };
            }
            else {
                chunk->blocks[i] = (BlockInstance){ 0, 0 };
                chunk->walls[i] = (BlockInstance){ 0, 0 };
            }

            chunk->light[i] = 0;
        }
    }
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

    rlPopMatrix();
}

BlockInstance chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall) {
    if (!chunk) return (BlockInstance) { 0, 0 };

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

        if (neighbor == NULL) return (BlockInstance) { 0, 0 };

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

void chunk_get_block_neighbors(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, isWall);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, isWall);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, isWall);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, isWall);
}

void chunk_get_block_neighbors_with_corners(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, isWall);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, isWall);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, isWall);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, isWall);

    output[NEIGHBOR_TOP_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y - 1 }, isWall);
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y - 1 }, isWall);
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y + 1 }, isWall);
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y + 1 }, isWall);
}

void chunk_get_light_neighbors(Chunk* chunk, Vector2u position, uint8_t output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 });
    output[NEIGHBOR_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y });
    output[NEIGHBOR_BOTTOM] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 });
    output[NEIGHBOR_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y });
}

void chunk_get_light_neighbors_with_corners(Chunk* chunk, Vector2u position, uint8_t output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 });
    output[NEIGHBOR_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y });
    output[NEIGHBOR_BOTTOM] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 });
    output[NEIGHBOR_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y });

    output[NEIGHBOR_TOP_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y - 1 });
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y - 1 });
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y + 1 });
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y + 1 });
}
