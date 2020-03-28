#include <glm/glm.hpp>

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

extern double fov;
extern colour3 background_colour;

void choose_scene(char const *fn);

bool trace(glm::vec3 rayOrigin, glm::vec3 rayDir, colour3& colour, int bouncesLeft, float refrIndex = 1);
bool trace(const point3 & rayOrigin, const point3 &screenPoint, colour3 &colour, bool pick);