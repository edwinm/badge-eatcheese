/*
 * Orientation
 * Made for WHY2025 Badge
 * By Edwin Martin 2025
 * License: MIT
 */

#include <stdio.h>

#include <unistd.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

// #include "badgevms/device.h"

#define BALL_SIZE 40
#define FOOD_SIZE 30
#define SDL_WINDOW_WIDTH           400U
#define SDL_WINDOW_HEIGHT          400U

#define GAME_WIDTH  400U
#define GAME_HEIGHT 400U

#define PI 3.14159265358979323846


typedef struct {
    // orientation_device_t *orientation;
    float x;
    float y;
    float food_x;
    float food_y;
    int score;
    int hit_wall;
    int has_lost;
    int is_eating;
    int new_cheese;
    SDL_Texture *food_texture;
} GolContext;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    GolContext  gol_ctx;
    Uint64        last_step;
} AppState;

void gol_initialize(GolContext *ctx, SDL_Renderer *renderer) {
    SDL_Surface * surf;
    printf("\n\nEngage!\n");

    ctx->x = 200.0 - BALL_SIZE / 2;
    ctx->y = 200.0 - BALL_SIZE / 2;
    ctx->food_x = 300.0;
    ctx->food_y = 300.0;
    ctx->score = 0;
    ctx->is_eating = 0;
    ctx->new_cheese = 0;
    ctx->hit_wall = 0;
    ctx->has_lost = 0;

    surf = IMG_Load("eat.png");

    if (surf == NULL) {
        printf("Failed to load image: %s\n", SDL_GetError());
    }

    ctx->food_texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);

    if (ctx->food_texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
    }

    // ctx->orientation = (orientation_device_t *)device_get("ORIENTATION0");

    // if (ctx->orientation == NULL) {
    //     printf("Well, no device found");
    //     return;
    // }
}

static SDL_AppResult handle_key_event_(GolContext *ctx, SDL_Scancode key_code) {
    switch (key_code) {
        /* Quit. */
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q: return SDL_APP_SUCCESS;
        /* Restart the game as if the program was launched. */
        // case SDL_SCANCODE_R: gol_initialize(ctx, null); break;

        default: break;
    }
    return SDL_APP_CONTINUE;
}

void drawBackground(AppState *as, GolContext *ctx) {
    SDL_FRect     r;

    SDL_SetRenderDrawColor(as->renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    if (ctx->score > 10) {
        if (ctx->has_lost == 1) {
            SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        } else {
            SDL_SetRenderDrawColor(as->renderer, 255, 128, 0, SDL_ALPHA_OPAQUE);
        }
        r.w = GAME_WIDTH - 6;
        r.h = GAME_HEIGHT - 6;
        r.x = 2;
        r.y = 2;
        SDL_RenderRect(as->renderer, &r);
    }
}

void drawCheese(AppState *as, GolContext *ctx) {
    SDL_FRect     r;

    if (ctx->new_cheese == 1) {
        ctx->food_x = (float)SDL_rand(GAME_WIDTH - FOOD_SIZE - 6) + 3;
        ctx->food_y = (float)SDL_rand(GAME_HEIGHT - FOOD_SIZE - 6) + 3;
        ctx->new_cheese = 0;
        ctx->is_eating = 0;
        ctx->score++;
    }

    SDL_SetRenderDrawColor(as->renderer, 255, 220, 0, SDL_ALPHA_OPAQUE);
    r.w = r.h = FOOD_SIZE;
    r.x = (int)ctx->food_x;
    r.y = (int)ctx->food_y;
    SDL_RenderFillRect(as->renderer, &r);
}

void drawEater(AppState *as, GolContext *ctx) {
    SDL_FRect     r;
    float dx = 0.0, dy = 0.0;

    if (ctx->has_lost == 1) {
        SDL_SetRenderDrawColor(as->renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
    } else {
        SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    }
    r.w = r.h = BALL_SIZE;

    if(ctx->is_eating == 1) {
        if (ctx->x + BALL_SIZE < ctx->food_x + FOOD_SIZE) {
            dx = 1.0;
        }
        if (ctx->x > ctx->food_x) {
            dx = -1.0;
        }
        if (ctx->y + BALL_SIZE < ctx->food_y + FOOD_SIZE) {
            dy = 1.0;
        }
        if (ctx->y > ctx->food_y) {
            dy = -1.0;
        }

        if (dx == 0.0 || dy == 0.0) {
            ctx->new_cheese = 1;
        }

    } else {
        // int degrees = ctx->orientation->_get_orientation_degrees(ctx->orientation);
        int degrees = 0; // Placeholder for orientation degrees, replace with actual logic if needed
        ctx->hit_wall = 0;

        dx = (float)SDL_sin(degrees * PI / 180.0);
        dy = (float)SDL_cos(degrees * PI / 180.0);

        if (ctx->x < 0) {
            ctx->x = 0;
            ctx->hit_wall = 1;
        }
        if (ctx->y < 0) {
            ctx->y = 0;
            ctx->hit_wall = 1;
        }
        if (ctx->x + BALL_SIZE > SDL_WINDOW_WIDTH) {
            ctx->x = SDL_WINDOW_WIDTH - BALL_SIZE;
            ctx->hit_wall = 1;
        }
        if (ctx->y + BALL_SIZE > SDL_WINDOW_HEIGHT) {
            ctx->y = SDL_WINDOW_HEIGHT - BALL_SIZE;
            ctx->hit_wall = 1;
        }
    }

    if (ctx->has_lost == 0) {
        ctx->x += dx * (5 + ctx->score / 4);
        ctx->y += dy * (5 + ctx->score / 4);
    }

    r.x = (int)ctx->x;
    r.y = (int)ctx->y;

    SDL_RenderFillRect(as->renderer, &r);
}


SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState     *as  = (AppState *)appstate;
    GolContext *ctx = &as->gol_ctx;
    Uint64 const  now = SDL_GetTicks();
    SDL_FRect     r;
    unsigned      i;
    unsigned      j;
    int           ct;

    drawBackground(as, ctx);

    drawCheese(as, ctx);

    drawEater(as, ctx);

    SDL_FRect food_rect = {
        .x = 50,
        .y = 50,
        .w = 64,
        .h = 64
    };
    SDL_RenderTexture(as->renderer, ctx->food_texture, NULL, &food_rect);


    if (ctx->x < ctx->food_x + FOOD_SIZE &&
        ctx->x + BALL_SIZE > ctx->food_x &&
        ctx->y < ctx->food_y + FOOD_SIZE &&
        ctx->y + BALL_SIZE > ctx->food_y) {
        ctx->is_eating = 1;
    }

    if (ctx->score > 10 && ctx->hit_wall == 1) {
        ctx->has_lost = 1;
        printf("You lost! Score: %d\n", ctx->score);
    }

    SDL_RenderPresent(as->renderer);
    return SDL_APP_CONTINUE;
}

static const struct {
    char const *key;
    char const *value;
} extended_metadata[] = {
    {SDL_PROP_APP_METADATA_URL_STRING, "https://badge.why2025.org/"},
    {SDL_PROP_APP_METADATA_CREATOR_STRING, "Badge team"},
    {SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "MIT"},
    {SDL_PROP_APP_METADATA_TYPE_STRING, "game"}
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    size_t i;

    if (!SDL_SetAppMetadata("Eat cheese", "1.0", "org.bitstorm.eater")) {
        return SDL_APP_FAILURE;
    }

    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }

    *appstate = as;

    // Create window first
    as->window = SDL_CreateWindow("Eat cheese", SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!as->window) {
        printf("Failed to create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Check display capabilities
    SDL_DisplayID          display      = SDL_GetDisplayForWindow(as->window);
    SDL_DisplayMode const *current_mode = SDL_GetCurrentDisplayMode(display);
    if (current_mode) {
        printf(
            "Current display mode: %dx%d @%.2fHz, format: %s",
            current_mode->w,
            current_mode->h,
            current_mode->refresh_rate,
            SDL_GetPixelFormatName(current_mode->format)
        );
    }

    // Create renderer
    as->renderer = SDL_CreateRenderer(as->window, NULL);
    if (!as->renderer) {
        printf("Failed to create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Check renderer properties
    SDL_PropertiesID props = SDL_GetRendererProperties(as->renderer);
    if (props) {
        char const *name = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "Unknown");
        printf("Renderer: %s", name);

        SDL_PixelFormat const *formats =
            (SDL_PixelFormat const *)SDL_GetPointerProperty(props, SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, NULL);
        if (formats) {
            printf("Supported texture formats:");
            for (int j = 0; formats[j] != SDL_PIXELFORMAT_UNKNOWN; j++) {
                printf("  Format %d: %s", j, SDL_GetPixelFormatName(formats[j]));
            }
        }
    }

    gol_initialize(&as->gol_ctx, as->renderer);
    as->last_step = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    GolContext *ctx = &((AppState *)appstate)->gol_ctx;
    switch (event->type) {
        case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN: return handle_key_event_(ctx, event->key.scancode);
        default: break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState *as = (AppState *)appstate;
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);
    }
}
