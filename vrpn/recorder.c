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

#include "vrpn-help.h"
#include "kuhl-util.h"
#include "vecmat.h"
#include "tdl-util.h"

int main(int argc, char* argv[])
{
	//Check if we got the proper arguments.
	if(argc < 3)
	{
		printf("Usage\n\trecorder fileName objectName\n");
		printf("\n");
		printf("This program reads data from a VRPN server and saves it to a file that can be played back later.\n");
		exit(1);
	}

	//Create a new TDL file.
	FILE *f = tdl_create(argv[1], argv[2]);
	if(f == NULL)
	{
		printf("Failed to create file: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	//Buffers for the data.
	float pos[3];
	float rotMat4[16];  // 4x4 matrix, what we vrpn_get() gives us.
	float rotMat3[9];   // a 3x3 rotation matrix, what we need to give to tdl_write()
	
	//Loop until Ctrl+C.
	while(1)
	{
		//Get the next vrpn entry
		vrpn_get(argv[2], NULL, pos, rotMat4);
		
		//Write that entry to the file
		mat3f_from_mat4f(rotMat3, rotMat4);
		tdl_write(f, pos, rotMat3);
		
		//IMPORTANT! Since there are no time stamps, this MUST be the same value as the fake
		//server that is going to be reading the file, otherwise artificial speed ups or delays
		//may occur in the final reading and output of the file.
		kuhl_limitfps(100);
	}
	
	//Close up the fd (Useless code until code to stop loop is added)
	fclose(f);
}
