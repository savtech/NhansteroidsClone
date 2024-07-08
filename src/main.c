#include "types.h"
#include "raylib.h"
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SHIP_SPEED 10
#define SHIP_TRIANGLE_SIZE 10
#define MAX_ASTEROID_COUNT 32
#define ASTEROID_SPAWN_RATE 2
#define SMALL_ASTEROID_MIN_RADIUS 8
#define SMALL_ASTEROID_MAX_RADIUS 32
#define SMALL_ASTEROID_MIN_SPEED 2
#define SMALL_ASTEROID_MAX_SPEED 10
#define LARGE_ASTEROID_MIN_RADIUS 32
#define LARGE_ASTEROID_MAX_RADIUS 48
#define LARGE_ASTEROID_MIN_SPEED 1
#define LARGE_ASTEROID_MAX_SPEED 4
#define MAX_LASER_COUNT 8
#define LASER_SPEED 20

typedef enum {
    ENTITY_STATE_INACTIVE,
    ENTITY_STATE_ACTIVE
} EntityState;

typedef enum {
    ASTEROID_TYPE_SMALL,
    ASTEROID_TYPE_LARGE,
    MAX_ASTEROID_TYPES
} AsteroidType;

typedef struct {
    EntityState state;
    AsteroidType type; 
    u32 radius;
    Vector2 position;
    u32 speed;
} Asteroid;

typedef struct {
    EntityState state;
    Vector2 position;
} Laser;

typedef struct {
    Vector2 vertices[3];
    Vector2 position;
} Ship;

typedef struct {
    Ship ship;
    Asteroid asteroids[MAX_ASTEROID_COUNT];
    u32 laser_index;
    Laser lasers[MAX_LASER_COUNT];
} GameContext;

GameContext game;

void ship_init(Ship* ship);
void ship_update(Ship* ship);
void ship_render(Ship* ship);
void asteroids_spawn(Asteroid* asteroids, u32 count);
void asteroids_update(Asteroid* asteroids);
void asteroids_render(Asteroid* asteroids);
void lasers_init(Laser* lasers);
void lasers_update(Laser* lasers);
void lasers_render(Laser* lasers);
void update();
void render();
void check_collisions();

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Nhansteroids Clone");
    SetTargetFPS(60);

    ship_init(&game.ship);
    asteroids_spawn(game.asteroids, MAX_ASTEROID_COUNT);
    lasers_init(game.lasers);

    while(!WindowShouldClose()) {
        update();
        render();
    }

    return 0;
}

void ship_init(Ship* ship) {
    //Our ship's starting position, the center of the screen
    ship->position = (Vector2){WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

    //We can derive the vertices of the ship's triangle representation from the starting position.
    //
    //       0
    //      / \
    //     /   \
    //    /  C  \
    //   /       \
    //  1---------2
    //
    // The triangle above shows how our vertices are laid out in memory (Raylib draws vertices in counter-clockwise order). "C" represents our ship's actual position, the center of the triangle.

    ship->vertices[0] = (Vector2){ship->position.x, ship->position.y - SHIP_TRIANGLE_SIZE};
    ship->vertices[1] = (Vector2){ship->position.x - SHIP_TRIANGLE_SIZE, ship->position.y + SHIP_TRIANGLE_SIZE};
    ship->vertices[2] = (Vector2){ship->position.x + SHIP_TRIANGLE_SIZE, ship->position.y + SHIP_TRIANGLE_SIZE};
}

void ship_update(Ship* ship) {
    u32 updated = 0;
    if(IsKeyDown(KEY_A)) {
        ship->position.x -= SHIP_SPEED;
        updated = 1;
    } 
    if(IsKeyDown(KEY_D)) {
        ship->position.x += SHIP_SPEED;
        updated = 1;
    }
    // if(IsKeyDown(KEY_W)) {
    //     ship->position.y -= SHIP_SPEED;
    //     updated = 1;
    // }
    // if(IsKeyDown(KEY_S)) {
    //     ship->position.y += SHIP_SPEED;
    //     updated = 1;
    // }

    if(updated == 1) {
        ship->vertices[0] = (Vector2){ship->position.x, ship->position.y - SHIP_TRIANGLE_SIZE};
        ship->vertices[1] = (Vector2){ship->position.x - SHIP_TRIANGLE_SIZE, ship->position.y + SHIP_TRIANGLE_SIZE};
        ship->vertices[2] = (Vector2){ship->position.x + SHIP_TRIANGLE_SIZE, ship->position.y + SHIP_TRIANGLE_SIZE};
    }

    if(IsKeyPressed(KEY_SPACE)) {
        game.lasers[game.laser_index].state = ENTITY_STATE_ACTIVE;
        game.lasers[game.laser_index].position = ship->vertices[0];
        ++game.laser_index;
        //printf("Laser fired.\n");
    }
}

void ship_render(Ship* ship) {
    DrawTriangle(ship->vertices[0], ship->vertices[1], ship->vertices[2], RED);
}

void asteroids_spawn(Asteroid* asteroids, u32 count) {
    for(u32 i = 0; i < count; ++i) {
        asteroids[i].type = GetRandomValue(0, MAX_ASTEROID_TYPES - 1);
        switch(asteroids[i].type) {
            case ASTEROID_TYPE_SMALL: {
                asteroids[i].radius = GetRandomValue(SMALL_ASTEROID_MIN_RADIUS, SMALL_ASTEROID_MAX_RADIUS);
                asteroids[i].speed = GetRandomValue(SMALL_ASTEROID_MIN_SPEED, SMALL_ASTEROID_MAX_SPEED);
            } break;
            case ASTEROID_TYPE_LARGE: {
                asteroids[i].radius = GetRandomValue(LARGE_ASTEROID_MIN_RADIUS, LARGE_ASTEROID_MAX_RADIUS);
                asteroids[i].speed = GetRandomValue(LARGE_ASTEROID_MIN_SPEED, LARGE_ASTEROID_MAX_SPEED);
            } break;
        }
        asteroids[i].position = (Vector2){GetRandomValue(0, WINDOW_WIDTH), GetRandomValue(0 - asteroids[i].radius, -(WINDOW_HEIGHT / 2))};
        asteroids[i].state = ENTITY_STATE_ACTIVE;
    }
}

void asteroids_update(Asteroid* asteroids) {
    for(u32 i = 0; i < MAX_ASTEROID_COUNT; ++i) {
        switch(asteroids[i].state) {
            case ENTITY_STATE_ACTIVE: {
                asteroids[i].position.y += asteroids[i].speed;
                if(asteroids[i].position.y > WINDOW_HEIGHT + asteroids[i].radius) {
                    asteroids[i].state = ENTITY_STATE_INACTIVE;
                }
            } break;
            case ENTITY_STATE_INACTIVE: {
                asteroids_spawn(&asteroids[i], 1);
            } break;
        }
    }
}

void asteroids_render(Asteroid* asteroids) {
    for(u32 i = 0; i < MAX_ASTEROID_COUNT; ++i) {
        DrawCircle(asteroids[i].position.x, asteroids[i].position.y, asteroids[i].radius, BLUE);
    }
}

void lasers_init(Laser* lasers) {
    for(u32 i = 0; i < MAX_LASER_COUNT; ++i) {
        lasers[i].state = ENTITY_STATE_INACTIVE;
    }
}

void lasers_update(Laser* lasers) {

    //Update the y coordinate of each active laser. "Active" lasers are lasers that we have fired, but have not yet collided with any asteroids or gone off-screen
    for(u32 i = 0; i < MAX_LASER_COUNT; ++i) {
        if(lasers[i].state == ENTITY_STATE_ACTIVE) {
            lasers[i].position.y -= LASER_SPEED;
        }
    }
}

void lasers_render(Laser* lasers) {
    for(u32 i = 0; i < MAX_LASER_COUNT; ++i) {
        if(lasers[i].state == ENTITY_STATE_ACTIVE) {
            DrawLine(lasers[i].position.x, lasers[i].position.y, lasers[i].position.x, lasers[i].position.y - 10, GREEN);
        }
    }
}

void update() {
    ship_update(&game.ship);
    asteroids_update(game.asteroids);
    lasers_update(game.lasers);

    check_collisions();
    render();
}

void render() {
    BeginDrawing();
    ClearBackground(BLACK);

    //Draw our ship
    ship_render(&game.ship);
    asteroids_render(game.asteroids);
    lasers_render(game.lasers);

    EndDrawing();
}

void check_collisions() {
    for(u32 i = 0; i < MAX_LASER_COUNT; ++i) {
        if(game.lasers[i].state == ENTITY_STATE_ACTIVE) {
            for(u32 j = 0; j < MAX_ASTEROID_COUNT; ++j) {
                if(CheckCollisionPointCircle(game.lasers[i].position, game.asteroids[j].position, game.asteroids[j].radius)) {
                    game.lasers[i].state == ENTITY_STATE_INACTIVE;
                    game.asteroids[j].state == ENTITY_STATE_INACTIVE;
                    asteroids_spawn(&game.asteroids[j], 1);
                }
            }
        }
    }
}