#include "espresso_jpeg.h"

int file_offset_reset(FILE *fp)
{
	if ( (fseek(fp, 0, SEEK_SET)) != 0)
	{
		perror("fseek");
		exit(-1);
	}

	return 0;
}

int transform_and_call(struct espresso_state *state, (void *f)(void))
{

	int i;
	for(i = 0 ; i < (state->jpg->num_sections); i++)
	{
		struct jpeg_section *ptr = (state->jpg->section_list)[i];
	
		unsigned short *sh = &(ptr->marker);
		if (ptr->type == SEGMENT)
		{
			while ((*sh) < 65535)
			{
				
			}

		}
	}
		

}



