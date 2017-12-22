#pragma once
#include <vector>


class Voxel {

public:
	std::vector<Voxel*> childPointers;

	int validMask = 0;	
	int leafMask = 0;

	// info section				/* info section encompasses a directory of the available attachments
	//							   as well as a pointer to the first child descriptor */	

	// parent pointer?  		/* In addition to traversing the voxel hierarchy, we must also be able to
	//								tell which block a given voxel resides in*/

	// shading attributes: color, normal, id:type



	// Constructors___________________




	// Functions______________________

	/*If the voxel contains any non - leaf children, we store an unsigned
	15 - bit child pointer in order to reference their data*/

};