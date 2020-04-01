#include <glm/glm.hpp>

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

extern double fov;
extern colour3 background_colour;

void choose_scene(char const *fn);
void jsonImport();

glm::vec3 trace(glm::vec3 rayOrigin, glm::vec3 rayDir, int bouncesLeft, bool inside);
bool trace(const point3& rayOrigin, const point3& screenPoint, glm::vec3& colour, bool pick);

