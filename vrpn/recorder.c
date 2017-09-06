/*
 * This is a simple program that will read from the VRPN
 * server set in the home directory and print the entries 
 * it reads to the specified file.
 *
 * @author John Thomas
 * @LastModified June 2015
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h> // gettimeofday
#include <time.h> // localtime

#include "vrpn-help.h"
#include "kuhl-util.h"
#include "vecmat.h"
#include "tdl-util.h"

int main(int argc, char* argv[])
{
	//Check if we got the proper arguments.
	if(argc < 3)
	{
		printf("Usage\n\trecorder serverHost objName1 [ objName2 ... ]\n");
		printf("\n");
		printf("This program reads data from a VRPN server and saves it to a file that can be played back later.\n");
		exit(EXIT_FAILURE);
	}

	/* Get the current time for a timestamp to be included in filename */
	struct timeval tv;
	if(gettimeofday(&tv, NULL) < 0)
	{
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}
	time_t nowtime = tv.tv_sec;
	struct tm *nowtm = localtime(&nowtime);  // statically allocated
	char timestamp[1024]; // construct a string without microseconds
	strftime(timestamp, 1024, "%Y%m%d-%H%M%S", nowtm);

	int objectsToRecord = argc - 2;
	FILE **outputFiles = malloc(sizeof(FILE*)*objectsToRecord);
	
	for(int i=0; i<objectsToRecord; i++) // for each object to record
	{
		char filename[2048];
		snprintf(filename, 2048, "%s-%s.tdl", argv[i+2], timestamp);

		//Create a new TDL file.
		printf("Output file: %s\n", filename);
		outputFiles[i] = tdl_create(filename, argv[i+2]);
		if(outputFiles[i] == NULL)
		{
			printf("Failed to create file: %s\n", filename);
			exit(EXIT_FAILURE);
		}
		else
			printf("Storing data from object '%s' in file '%s\n", argv[i+2], filename);
	}


	//Loop until Ctrl+C.
	while(1)
	{
		//Buffers for the data.
		float pos[3];
		float rotMat4[16];  // 4x4 matrix, what we vrpn_get() gives us.
		float rotMat3[9];   // a 3x3 rotation matrix, what we need to give to tdl_write()

		for(int i=0; i<objectsToRecord; i++)
		{
			//Get the next vrpn entry
			vrpn_get(argv[i+2], argv[1], pos, rotMat4);
			
			//Write that entry to the file
			mat3f_from_mat4f(rotMat3, rotMat4);
			tdl_write(outputFiles[i], pos, rotMat3);
		}
		
		//IMPORTANT! Since there are no time stamps, this MUST be the same value as the fake
		//server that is going to be reading the file, otherwise artificial speed ups or delays
		//may occur in the final reading and output of the file.
		kuhl_limitfps(100);
	}
	

	//Close the fd (Useless code until code to stop loop is added)
	for(int i=0; i<objectsToRecord; i++)
		fclose(outputFiles[i]);
	         
}
