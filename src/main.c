#include <graphics_test.h>

static
bool dice(float chance) {
    return rand() > RAND_MAX / chance;
}

static
float randf() {
    return (float)rand() / (float)RAND_MAX;
}

static
void Bounce(ecs_iter_t *it) {
    EcsPosition3 *p = ecs_column(it, EcsPosition3, 1);
    EcsVelocity3 *v = ecs_column(it, EcsVelocity3, 2);
    EcsColor *c = ecs_column(it, EcsColor, 3);

    /* Fade out colors, and move squares back into position */
    for (int i = 0; i < it->count; i ++) {
        float y = p[i].y;
        float lt = y < 0;
        p[i].y = lt * (y + it->delta_time * v[i].y);

        v[i].y += 20 * it->delta_time;

        c[i].value.r = glm_max(0.98 * c[i].value.r, 0.1);
        c[i].value.g = glm_max(0.95 * c[i].value.g, 0.12);
        c[i].value.b = glm_max(0.995 * c[i].value.b, 0.15);
    }

    /* Bounce random squares. Lower threshold to increase number of bounces */
    int threshold = RAND_MAX - RAND_MAX / 500.0;

    for (int i = 0; i < it->count; i ++) {
        if (rand() > threshold) {
            p[i].y = -0.01;
            v[i].y = -1.0 - 5 * randf();

            bool d = dice(1.1);
            c[i].value.r = d;
            c[i].value.b = 1.0;
            c[i].value.g = 1.0 - d * 0.2;
        }
    }
}

int main(int argc, char *argv[]) {
    ecs_world_t *world = ecs_init();

    ECS_IMPORT(world, FlecsComponentsTransform); /* Position, Rotation */
    ECS_IMPORT(world, FlecsComponentsGeometry); /* Rectangle, Color */
    ECS_IMPORT(world, FlecsComponentsPhysics); /* Velocity */
    ECS_IMPORT(world, FlecsComponentsGui);    /* Window, Canvas, Camera */
    ECS_IMPORT(world, FlecsSystemsTransform); /* Matrix transformation */
    ECS_IMPORT(world, FlecsSystemsSdl2);  /* Window implementation */
    ECS_IMPORT(world, FlecsSystemsSokol); /* Canvas implementation */

    /* The system that makes the squares bounce */
    ECS_SYSTEM(world, Bounce, EcsOnUpdate,
        flecs.components.transform.Position3,
        flecs.components.physics.Velocity3,
        flecs.components.geometry.Color);

    /* Setup the window, canvas and camera */
    ecs_entity_t window = ecs_set(world, 0, EcsWindow, {
        .width = 1024,
        .height = 800
    });

    ecs_entity_t camera = ecs_set(world, 0, EcsCamera, {
        .position = {0.0, -1.0, 0.0},
        .lookat = {0.0, 0.0, 5.0},
        .up = {0.0, 1.0, 0.0},
        .fov = 30
    });

    ecs_set(world, window, EcsCanvas, {
        .background_color = {0, 0, 0},
        .camera = camera
    });

    #define X_SQUARES (300.0)
    #define Z_SQUARES (800.0)
    #define SIZE (1.0)

    /* Create strip of squares */
    for (int x = 0; x < X_SQUARES; x ++) {
        for (int z = 0; z < Z_SQUARES; z ++) {
            ecs_entity_t e = 
            ecs_set(world, 0, EcsRectangle, {SIZE , SIZE });
            ecs_set(world, e, EcsColor, {0.0, 0.2, 0.0, 1.0});
            ecs_set(world, e, EcsPosition3, {x * SIZE - (X_SQUARES / 2.0) * SIZE, 0.0, z * SIZE});
            ecs_set(world, e, EcsVelocity3, {0.0, 0.0, 0.0});
            ecs_set(world, e, EcsRotation3, {GLM_PI / 2.0, 0, 0});
        }
    }   

    /* Run systems */
    const ecs_world_info_t *info = ecs_get_world_info(world);
    EcsCamera *cam = ecs_get_mut(world, camera, EcsCamera, NULL);
    float speed = 1;
    while ( ecs_progress(world, 0)) {
        /* Just move the camera in the main loop */
        cam->position[1] -= info->delta_time * (speed / 4);
        cam->position[2] += info->delta_time * speed;
        cam->lookat[2] += info->delta_time * speed;

        /* Accelerate camera movement */
        speed += info->delta_time / 5;
    }

    /* Cleanup */
    return ecs_fini(world);
}
