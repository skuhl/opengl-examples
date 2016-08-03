#include "viewmat.h"

class camcontrolMouse : public camcontrol
{
public:
	camcontrolMouse(const float pos[3], const float look[3], const float up[3]);
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
