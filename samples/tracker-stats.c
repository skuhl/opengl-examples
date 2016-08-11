#include <stdlib.h>
#include "libkuhl.h"

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

int main(int argc, char *argv[])
{
#ifdef MISSING_VRPN
	printf("This program requires VRPN.");
	exit(EXIT_FAILURE);
#else

	if(argc != 3)
	{
		printf("Usage: %s vrpnObjectName numRecords\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int numRecords = 0;
	if(sscanf(argv[2], "%d", &numRecords) != 1)
	{
		msg(MSG_FATAL, "Error parsing numRecords parameter.\n");
		exit(EXIT_FAILURE);
	}

	const char *vrpnObject = argv[1];

	
	msg(MSG_BLUE, "Collecting %d samples from tracker...please wait...\n", numRecords);
	float *data = vrpn_get_raw(vrpnObject, NULL, numRecords);
	if(data == NULL)
	{
		msg(MSG_FATAL, "Failed to collect data.");
		exit(EXIT_FAILURE);
	}

	printf("First record (x,y,z, quat): \n");
	for(int i=0; i<7; i++)
		printf("%f\n", data[i]);

	float x[numRecords], y[numRecords], z[numRecords];

	filter(data, numRecords, 0, x);
	filter(data, numRecords, 1, y);
	filter(data, numRecords, 2, z);
	msg(MSG_BLUE, "--- XYZ ---\n");
	
	msg(MSG_INFO, "Means: %f %f %f\n",
	    mean(x, numRecords),
	    mean(y, numRecords),
	    mean(z, numRecords));
	msg(MSG_INFO, "Variance: %20.20f %20.20f %20.20f\n",
	    variance(x, numRecords),
	    variance(y, numRecords),
	    variance(z, numRecords));

	msg(MSG_INFO, "  Stddev: %20.20f %20.20f %20.20f\n",
	    sqrt(variance(x, numRecords)),
	    sqrt(variance(y, numRecords)),
	    sqrt(variance(z, numRecords)));

	float q1[numRecords], q2[numRecords], q3[numRecords], q4[numRecords];
	filter(data, numRecords, 3, q1);
	filter(data, numRecords, 4, q2);
	filter(data, numRecords, 5, q3);
	filter(data, numRecords, 6, q4);

	msg(MSG_BLUE, "--- Quat ---\n");
	
	msg(MSG_INFO, "Means: %f %f %f %f\n",
	    mean(q1, numRecords),
	    mean(q2, numRecords),
	    mean(q3, numRecords),
	    mean(q4, numRecords));
	
	msg(MSG_INFO, "Variance: %20.20f %20.20f %20.20f %20.20f\n",
	    variance(q1, numRecords),
	    variance(q2, numRecords),
	    variance(q3, numRecords),
	    variance(q4, numRecords));
	msg(MSG_INFO, "   Stddev: %20.20f %20.20f %20.20f %20.20f\n",
	    sqrt(variance(q1, numRecords)),
	    sqrt(variance(q2, numRecords)),
	    sqrt(variance(q3, numRecords)),
	    sqrt(variance(q4, numRecords)));

	free(data);
	
	return 0;
#endif
}
