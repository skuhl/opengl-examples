#include "vrpn-help.h"
#include "msg.h"
#include "kuhl-util.h"

#define VRPN_OBJECT "DK2"
#define COUNT 1000

void filter(float *all_data, int count, int component, float filtered_data[])
{
	for(int i=0; i<count; i++)
		filtered_data[i] = all_data[i*7+component];
}

float sum(float *data, int count)
{
	float sum=0;
	for(int i=0; i<count; i++)
		sum += data[i];
	return sum;
}

float mean(float *data, int count)
{
	return sum(data, count)/count;
}

float variance(float *data, int count)
{
	float avg = mean(data, count);
	
	float squared = 0;
	for(int i=0; i<count; i++)
		squared += (data[i]-avg)*(data[i]-avg);

	return squared/count;
}

int main(void)
{
#ifdef MISSING_VRPN
	printf("This program requires VRPN.");
	exit(EXIT_FAILURE)
#else

	printf("Collecting %d samples from tracker...please wait...\n", COUNT);
	float *data = vrpn_get_raw(VRPN_OBJECT, NULL, COUNT);

	printf("First record (x,y,z, quat): \n");
	for(int i=0; i<7; i++)
		printf("%f\n", data[i]);

	float x[COUNT], y[COUNT], z[COUNT];

	filter(data, COUNT, 0, x);
	filter(data, COUNT, 1, y);
	filter(data, COUNT, 2, z);
	printf("\n--- XYZ ---\n");
	
	printf("Means: %f %f %f\n",
	       mean(x, COUNT),
	       mean(y, COUNT),
	       mean(z, COUNT));
	printf("Variance: %20.20f %20.20f %20.20f\n",
	       variance(x, COUNT),
	       variance(y, COUNT),
	       variance(z, COUNT));

	printf("  Stddev: %20.20f %20.20f %20.20f\n",
	       sqrt(variance(x, COUNT)),
	       sqrt(variance(y, COUNT)),
	       sqrt(variance(z, COUNT)));

	float q1[COUNT], q2[COUNT], q3[COUNT], q4[COUNT];
	filter(data, COUNT, 3, q1);
	filter(data, COUNT, 4, q2);
	filter(data, COUNT, 5, q3);
	filter(data, COUNT, 6, q4);

	printf("\n--- Quat ---\n");
	
	printf("Means: %f %f %f %f\n",
	       mean(q1, COUNT),
	       mean(q2, COUNT),
	       mean(q3, COUNT),
	       mean(q4, COUNT));
	
	printf("Variance: %20.20f %20.20f %20.20f %20.20f\n",
	       variance(q1, COUNT),
	       variance(q2, COUNT),
	       variance(q3, COUNT),
	       variance(q4, COUNT));
	printf("   Stddev: %20.20f %20.20f %20.20f %20.20f\n",
	       sqrt(variance(q1, COUNT)),
	       sqrt(variance(q2, COUNT)),
	       sqrt(variance(q3, COUNT)),
	       sqrt(variance(q4, COUNT)));

	return 0;
#endif
}
