#include "viewmat.h"
#include "orient-sensor.h"

class camcontrolOrientSensor : public camcontrol
{
private:
	OrientSensorState orientsense;
	float position[3];

public:
	camcontrolOrientSensor(const float pos[3]);
	~camcontrolOrientSensor();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
