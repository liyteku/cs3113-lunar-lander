/**
* Author: Lewis Zhao
* Assignment: Lunar Lander!
* Date due: 10/30/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include <cstdlib>

// Enums
enum MissionStatus { FLYING, ACCOMPLISHED, FAILED };

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 60;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

constexpr float GRAVITY = 60.0f;
constexpr float UP_ACCELERATION = 150.0f;
constexpr float SIDE_ACCELERATION = 120.0f;
constexpr float SLOWDOWN = 1.5f;
constexpr Vector2 SHIP_START = { ORIGIN.x, 60.0f };
constexpr Vector2 SHIP_SIZE = { 80.0f, 71.0f };

constexpr float MAX_FUEL = 100.0f;
constexpr float FUEL_BURN = 20.0f;
constexpr Vector2 FUEL_BAR_POSITION = { 20.0f, 20.0f };
constexpr Vector2 FUEL_BAR_SIZE = { 200.0f, 20.0f };

constexpr int PLANET_COUNT = 5;
constexpr int MOVING_PLANET = 4;
constexpr int PLANET_SPEED = 120;
constexpr float PLANET_SIZE = 100.0f;
constexpr float MOVING_MIN_X = 250.0f;
constexpr float MOVING_MAX_X = 750.0f;
const char *const PLANET_FILES[PLANET_COUNT] = {
    "assets/game/timber_hearth.png",
    "assets/game/brittle_hollow.png",
    "assets/game/giants_deep.png",
    "assets/game/dark_bramble.png",
    "assets/game/attlerock.png"
};
constexpr Vector2 PLANET_POSITIONS[PLANET_COUNT] = {
    { 130.0f, 530.0f },
    { 380.0f, 530.0f },
    { 630.0f, 530.0f },
    { 880.0f, 530.0f },
    { ORIGIN.x, 330.0f }
};

constexpr int STAR_COUNT = 150;
const Color BACKGROUND_COLOR = ColorFromHex("#000000");
const Color STAR_COLOR = ColorFromHex("#ffffff");

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f,
          gTimeAccumulator = 0.0f;

MissionStatus gMissionStatus = FLYING;

float gFuel = MAX_FUEL;
bool gIsUsingFuel = false;
int gTargetPlanet = 0;

Vector2 gStars[STAR_COUNT];

Entity *gShip = nullptr;
Entity *gPlanets = nullptr;
Entity *gFuelBar = nullptr;
Entity *gAccomplishedMessage = nullptr;
Entity *gFailedMessage = nullptr;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

Entity *createMessage(const char *filepath) {
    Entity *message = new Entity(ORIGIN, { 0.0f, 0.0f }, filepath);
    Texture2D texture = message->getTexture();
    message->setScale({ static_cast<float>(texture.width), static_cast<float>(texture.height) });
    return message;
}

// Function Definitions
void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Outer Wilds Lander");

    gShip = new Entity(SHIP_START, SHIP_SIZE, "assets/game/ship.png");
    gShip->setAcceleration({ 0.0f, GRAVITY });
    gPlanets = new Entity[PLANET_COUNT];
    for (int i = 0; i < PLANET_COUNT; i++) {
        gPlanets[i].setTexture(PLANET_FILES[i]);
        gPlanets[i].setScale({ PLANET_SIZE, PLANET_SIZE });
        gPlanets[i].setColliderDimensions({ PLANET_SIZE, PLANET_SIZE });
        gPlanets[i].setPosition(PLANET_POSITIONS[i]);
    }
    gPlanets[MOVING_PLANET].setSpeed(PLANET_SPEED);
    gPlanets[MOVING_PLANET].moveRight();

    gFuelBar = new Entity(
        { FUEL_BAR_POSITION.x + FUEL_BAR_SIZE.x / 2.0f, FUEL_BAR_POSITION.y + FUEL_BAR_SIZE.y / 2.0f },
        FUEL_BAR_SIZE,
        "assets/game/fuel_fill.png"
    );
    gAccomplishedMessage = createMessage("assets/game/mission_accomplished.png");
    gFailedMessage = createMessage("assets/game/mission_failed.png");

    // random target planet and star positions
    srand(time(nullptr));
    gTargetPlanet = rand() % PLANET_COUNT;
    for (int i = 0; i < STAR_COUNT; i++) {
        gStars[i] = { static_cast<float>(rand() % SCREEN_WIDTH), static_cast<float>(rand() % SCREEN_HEIGHT) };
    }

    SetTargetFPS(FPS);
}

void processInput() {
    if (WindowShouldClose()) {
        gAppStatus = TERMINATED;
    }

    if (gMissionStatus != FLYING) {
        return;
    }

    Vector2 acceleration = { 0.0f, GRAVITY };
    gIsUsingFuel = false;

    if (gFuel > 0.0f) {
        if (IsKeyDown(KEY_A)) {
            acceleration.x = -SIDE_ACCELERATION;
            gIsUsingFuel = true;
        } else if (IsKeyDown(KEY_D)) {
            acceleration.x = SIDE_ACCELERATION;
            gIsUsingFuel = true;
        }
        if (IsKeyDown(KEY_W)) {
            acceleration.y -= UP_ACCELERATION;
            gIsUsingFuel = true;
        }
    }
    if (acceleration.x == 0.0f) {
        acceleration.x = -gShip->getVelocity().x * SLOWDOWN;
    }

    gShip->setAcceleration(acceleration);
}

void update() {
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (gMissionStatus != FLYING) {
        return;
    }

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP) {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        float previousShipBottom = gShip->getPosition().y + gShip->getColliderDimensions().y / 2.0f;

        gShip->update(FIXED_TIMESTEP, nullptr, 0);

        Vector2 movingPosition = gPlanets[MOVING_PLANET].getPosition();

        if (movingPosition.x > MOVING_MAX_X) {
            gPlanets[MOVING_PLANET].moveLeft();
        }
        if (movingPosition.x < MOVING_MIN_X) {
            gPlanets[MOVING_PLANET].moveRight();
        }
        for (int i = 0; i < PLANET_COUNT; i++) {
            gPlanets[i].update(FIXED_TIMESTEP, nullptr, 0);
        }

        // fuel
        if (gIsUsingFuel) {
            gFuel -= FUEL_BURN * FIXED_TIMESTEP;

            if (gFuel < 0.0f) {
                gFuel = 0.0f;
            }
        }

        // landing on the target planet wins, touching any other planet loses
        for (int i = 0; i < PLANET_COUNT; i++) {
            if (gShip->isColliding(&gPlanets[i])) {
                float planetTop = gPlanets[i].getPosition().y - gPlanets[i].getColliderDimensions().y / 2.0f;
                bool cameFromAbove = previousShipBottom <= planetTop;

                if (i == gTargetPlanet && cameFromAbove) {
                    gMissionStatus = ACCOMPLISHED;
                } else {
                    gMissionStatus = FAILED;
                }
            }
        }

        // leaving the screen also loses
        Vector2 shipPosition = gShip->getPosition();

        if (shipPosition.x < 0.0f || shipPosition.x > SCREEN_WIDTH ||
            shipPosition.y < 0.0f || shipPosition.y > SCREEN_HEIGHT) {
            gMissionStatus = FAILED;
        }

        deltaTime -= FIXED_TIMESTEP;

        if (gMissionStatus != FLYING) {
            break;
        }
    }

    gTimeAccumulator = deltaTime;

    float fuelWidth = FUEL_BAR_SIZE.x * gFuel / MAX_FUEL;
    gFuelBar->setScale({ fuelWidth, FUEL_BAR_SIZE.y });
    gFuelBar->setPosition({ FUEL_BAR_POSITION.x + fuelWidth / 2.0f, FUEL_BAR_POSITION.y + FUEL_BAR_SIZE.y / 2.0f });
}

void render() {
    BeginDrawing();

    ClearBackground(BACKGROUND_COLOR);

    for (int i = 0; i < STAR_COUNT; i++) {
        DrawPoly(gStars[i], 4, 1.5f, 45.0f, STAR_COLOR);
    }

    for (int i = 0; i < PLANET_COUNT; i++) {
        gPlanets[i].render();
    }

    // a green box marks the target planet
    gPlanets[gTargetPlanet].displayCollider();

    gShip->render();

    // Fuel UI
    gFuelBar->render();
    DrawRectangleLines(FUEL_BAR_POSITION.x, FUEL_BAR_POSITION.y, FUEL_BAR_SIZE.x, FUEL_BAR_SIZE.y, WHITE);

    if (gMissionStatus == ACCOMPLISHED) {
        gAccomplishedMessage->render();
    } else if (gMissionStatus == FAILED) {
        gFailedMessage->render();
    }

    EndDrawing();
}

void shutdown() {
    delete gShip;
    delete[] gPlanets;
    delete gFuelBar;
    delete gAccomplishedMessage;
    delete gFailedMessage;

    CloseWindow();
}

int main(void) {
    initialise();

    while (gAppStatus == RUNNING) {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
