#include "viewmat.h"

class camcontrolVrpn : public camcontrol
{
private:
	char *object;
	char *hostname;
public:
	camcontrolVrpn(const char *object, const char *hostname);
	~camcontrolVrpn();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
