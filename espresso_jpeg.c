#include <stdio.h>
#include "espresso_jpeg.h"


// EXAMPLE LIBRARY USAGE:
struct jpeg_info *create_jpeg_from_file(const char *path)
{
	int sections_attempted = 0, sections_loaded = 0;
	int pre_count = file_count_sections(path);

	if (pre_count <= 0)
	{
		printf("Attempted to load an invalid JPEG file!\n");
		return NULL;
	}
	
	//int read;
	FILE *fp = fopen(path, "r");
	if (!fp)
	{
		perror("fopen");
		return NULL;
	}

	struct jpeg_info *jpg = create_jpeg();
	if (!jpg)
	{
		printf("error: create_jpeg()\n");
		fclose(fp);
		return NULL;
	}

	struct jpeg_section **list = jpeg_create_list(jpg, pre_count);
	if (!list)
	{
		printf("error: jpeg_create_list IN create_jpeg_from_file()\n");
		fclose(fp);
		return NULL;
	}
	// sections are enumerated beginning at 2. a return value of 1
	// from f_h_s() is not possible as of right now now, but i 
	// wanted to reserve it to preserve convention.
	while (file_has_section(fp) >= 2)
	{
		sections_attempted++;

		unsigned int dlength;

		// leaving the default as a marker size of for now.
		// this is going to depend on how transformations are implemented. 
		struct jpeg_section *sec = section_create(0, 0, 0, 0, 0, 0, 0, 0);
		if (!sec)
		{
			printf("error: section_create()\n");
			fclose(fp);
			return jpg; // we'll return what we DO have
		}

		if ((file_read_all(fp, sec->marker, 2)) < 2)
		{
			printf("error: file_read_all(), marker\n");
			CleanIfFileError(sec, fp)
			return jpg;
		}

		section_set_length_mark(sec, 2);
		section_update_length_total(sec);

		section_set_type_from_marker(sec);
		if (!type_has_len_field(sec))
		{
			list[sections_loaded++] = sec;
			jpeg_increase_length_total(jpg, section_get_length_total(sec));
			jpeg_increase_section_count(jpg);
			continue;
		}

	
		
		if ((file_read_all(fp, sec->len, 2)) < 2)
                {
                        printf("error: file_read_all(), len\n");
                        CleanIfFileError(sec, fp)
                        return jpg;
                }
		
		section_set_length_len(sec, 2);
		section_update_length_total(sec);

		if (sec->type != STREAM)
		{	
                        // the 2 len bytes are counted in the byte slot for the protocol,
                        // so subtract it by 2 to get the length of the rest of the section.
			dlength = (convert_length(section_get_len(sec)) -2);

	        	if ((file_read_all(fp, sec->data, dlength)) < dlength)
                	{
                       	 	printf("error: file_read_all(), data\n");
                        	CleanIfFileError(sec, fp)
                        	return jpg;
                	}


		}

		else
		{
			dlength = (file_read_stream(fp, sec->data));
			if (dlength == -1)
			{
				printf("error: file_read_stream()\n");	
				CleanIfFileError(sec, fp)
				return jpg;
			}


		}

		section_set_length_data(sec, dlength);
		section_update_length_total(sec);
		list[sections_loaded++] = sec;
		jpeg_increase_length_total(jpg, section_get_length_total(sec));
		jpeg_increase_section_count(jpg);


	}
	return jpg;
}

void *jpeg_to_buf(struct jpeg_info *jpg, void *buf)
{
	int i, j;
	struct jpeg_section *section;
	for(i = 0, j = 0; i < jpg->num_sections; i++)
	{
		section = (jpg->section_list)[i];

		if (section->length_marker)
		{
			memcpy(buf+j, section->marker, section->length_marker);
			j += section->length_marker;
		}

		if (section->length_len)
		{
			memcpy(buf+j, section->len, section->length_len);
			j += section->length_len;	

		}

		if (section->length_data)
		{
			memcpy(buf+j, section->data, section->length_data);
			j += section->length_data;
		}
	}

	return buf;
}


unsigned int jpeg_increase_length_total(struct jpeg_info *ptr, unsigned int num)
{
	ptr->jpeg_length_total += num;
	return ptr->jpeg_length_total;
}

int jpeg_increase_section_count(struct jpeg_info *jpg)
{
	jpg->num_sections += 1;
	return jpg->num_sections;
}
int file_count_sections(const char *path)
{
	struct stat st_buf;
	if ((stat(path, &st_buf)) == -1)
	{
		perror("stat");
		return -1;
	}

	unsigned long size = st_buf.st_size;
	unsigned char *buf = (unsigned char *) malloc(sizeof(unsigned char) * size);
	//unsigned char *ptr = buf;
	FILE *fp = fopen(path, "r");
	int sections, times, read, i;

	read = file_read_all(fp, buf, size);
	if (read != size)
	{
		printf("error: file_read_all() <-- file_count_sections()\n");
		free(buf); 
		fclose(fp); // TODO: more checking with the file_read functions
		return -1;
	}	
	
	fclose(fp);

	if ((size % 2))
		times = --size;

	for(i = 0; i < size; i++)
	{
		if ( (buf[i] == (unsigned char) '\xff') && ((buf[i+1] > (unsigned char) '\x00')
				&& (buf[i+1] != (unsigned char) '\xff')))
		{
			sections++;
			i++;
			continue;
		} 
	
	}



	free(buf);	
	return sections;
	
}

int file_read_stream(FILE *fp, unsigned char *buf)
{
	int result, read, test;
	read = 0;
	do
	{	result = file_read_until_uch(fp, buf, MARKER_BEGIN);

		if (result == -1)
			return -1;

		read += result;
		
		test = file_has_section(fp);

		if (test == -1)
			return -1;
		
	} while (!test); 

	return read;
}


int file_read_until_uch(FILE *fp, unsigned char *buf, unsigned char ch)
{
	unsigned char c;
	int i = 0;
	while (((c = (unsigned char) fgetc(fp)) != ch))
	{
		if (feof(fp) || ferror(fp))
			return -1;
		

		buf[i++] = c;
	}
	
	if (ungetc(c, fp) != c)
		return -1;

	return i;
}


int section_update_length_total(struct jpeg_section *ptr)
{
	ptr->length_total = 0;

	if (ptr->length_marker)
		ptr->length_total += ptr->length_marker;

	if (ptr->length_len)
		ptr->length_total += ptr->length_len;

	if (ptr->length_data)
		ptr->length_total += ptr->length_data;
	
	return ptr->length_total;
}


void section_set_length_total(struct jpeg_section *ptr, unsigned long long len)
{
	ptr->length_total =  len;

}
unsigned char *section_get_len(struct jpeg_section *ptr)
{
	
	return ptr->len;
}

struct jpeg_section **jpeg_create_list(struct jpeg_info *jpg, int entries)
{

	jpg->section_list = (struct jpeg_section **) malloc(sizeof(struct jpeg_section *) * entries);
	if (!(jpg->section_list))
		return NULL;

	return jpg->section_list;
}


void section_set_type_from_marker(struct jpeg_section *ptr)
{
	ptr->type = type_from_marker(section_get_marker(ptr));
	
}


unsigned int convert_length(unsigned char *temp)
{
    return (((unsigned int) temp[0]) * 256) + ((unsigned int) temp[1]);
}


unsigned char section_get_marker(struct jpeg_section *ptr)
{
	return ptr->marker[1];
}

// returns the corresponding integer value that represents the type of jpeg section. there exist many,
// many more than are currently in here.
int type_from_marker(unsigned char marker)
{

	int type;

	switch(marker)
	{

		case (unsigned char) '\xd8':
						type = START;
						break;
		case (unsigned char) '\xd9': 
						type = END;
						break;
		case (unsigned char) '\xda': 
						type = STREAM;
						break;

					default:
						type = SEGMENT; // for now
						break;
	};

	return type;
}


// create the type of object we are using to represent a jpeg. 
struct jpeg_info *create_jpeg()
{
	struct jpeg_info *ptr = 
		(struct jpeg_info *) malloc(sizeof(struct jpeg_info));

	memset(ptr, 0, sizeof(struct jpeg_info));

	if (ptr)
		return ptr;
	else
		return NULL;
}


// check if this type of jpeg section contains a 2 byte length field. 
int type_has_len_field(struct jpeg_section *ptr)
{
	int type = ptr->type;

	switch (type)
        {
                case START:
                case END:
                                return 0;
                case STREAM:
                case HEADER:
                case EXTENSION:
                case SEGMENT:
                                return 1;

                default:
                                return 1;

        };

}

// more like "filestream_has_section"...
// checks if there is a jpeg section at the current offset. 
// return 0 on FALSE, -1 on error, and the type (a value >= 2) on TRUE 
int file_has_section(FILE *fp)
{
	unsigned char tmp[2];
	long cur_pos = ftell(fp);

	int read = file_read_all(fp, tmp, 2);

	if (read <= 0)
		return read;
	
	if ((tmp[0] == (unsigned char) '\xff')  && ((tmp[1] > (unsigned char) '\x00') 
						&& (tmp[1] != (unsigned char) '\xff')))
	{
		if ((fseek(fp, cur_pos, SEEK_SET) == -1))
		{
			perror("fseek");
			return -1;
		}
		else
			return type_from_marker(tmp[1]);
	}
	else
		return 0;
}



// wrapper for reading the amount of bytes we want, in case fread() returns less for some reason.
int file_read_all(FILE *fp, unsigned char *buf, int num)
{
	int total = 0;
	int read;
	
	do
	{
		read = fread(buf, 1, num-total, fp);
		if (read <= 0)
			return read;
		else
			total += read;

	} while (total < num);

	return total;	
}

// create an object that we use to represent a section of a jpeg file. 
struct jpeg_section *section_create(int type, unsigned char *marker, unsigned int length_marker, 
					unsigned char *len, unsigned int length_len, unsigned char *data, 
					unsigned long long length_data, unsigned long long length_total)
{
	struct jpeg_section *ptr = (struct jpeg_section *) malloc(sizeof(struct jpeg_section));

	if (!ptr)
		return NULL;
	
	memset(ptr, 0, sizeof(struct jpeg_section));

	if (!(section_set_vals(ptr, type, marker, length_marker, len, length_len, data, length_data, length_total)))
		return NULL;
	

	return ptr;
}

// set the values of an existing jpeg_section.
struct jpeg_section *section_set_vals(struct jpeg_section *ptr, int type, unsigned char *marker, unsigned int length_marker, unsigned char *len,
                                        unsigned int length_len, unsigned char *data, unsigned long long length_data, unsigned long length_total)
{

	if (!ptr)
		return NULL;

	if (type)
		ptr->type = type;

	if (marker && length_marker)
	{
		memcpy(ptr->marker, marker, length_marker);
		ptr->length_marker = length_marker;
	}

	if (len && length_len)
	{

		memcpy(ptr->len, len, length_len);
		ptr->length_len = length_len;
	}

	if (data && length_data)
	{
		memcpy(ptr->data, data, length_data);
		ptr->length_data = length_data;
	}

	if (length_total)
	{
		ptr->length_total = length_total;
	}
	else
	{
		ptr->length_total = (ptr->length_marker) + (ptr->length_len) + (ptr->length_data);

	}

	return ptr;
}

int section_set_length_len(struct jpeg_section *ptr, unsigned long len)
{
        ptr->length_len = len;
        return ptr->length_len;
}

int section_set_marker_len(struct jpeg_section *ptr, unsigned long len)
{
        ptr->length_marker = len;
        return ptr->length_marker;
}

int section_set_length_data(struct jpeg_section *ptr, unsigned long len)
{
        ptr->length_data = len;
        return ptr->length_data;
}

unsigned long long section_get_length_total(struct jpeg_section *ptr)
{
	return ptr->length_total;
}

unsigned int section_set_length_mark(struct jpeg_section *ptr, unsigned int num)
{
	ptr->length_marker = num;
	return num;
}
