#ifndef CONSTANTS
#define CONSTANTS

// Screen dimension constants
#define ScreenWidth 640
#define ScreenHeight 480
#define hfov (0.73f*ScreenHeight)  // Affects the horizontal field of vision
#define vfov (.2f*ScreenHeight)    // Affects the vertical field of vision

// Player attributes
#define EyeHeight  6    // Camera height from floor when standing
#define DuckHeight 2.5  // And when crouching
#define HeadMargin 1    // How much room there is above camera before the head hits the ceiling
#define KneeHeight 2    // How tall obstacles the player can simply walk over without jumping

// Map related
#define MapName "map-test.txt" // Map-test.txt = single room, map-clear.txt = multiple rooms

#endif
