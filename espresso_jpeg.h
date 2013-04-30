#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_BUFSZ 65535
#define MAX_MARKLEN 2
#define MAX_LENLEN 2
#define MARKER_BEGIN '\xff'

enum section_types { HEADER = 2, SEGMENT = 3, END = 4, EXTENSION = 5, START = 6, STREAM = 7};

// makeup of the jpeg "file" used for fuzzing.
struct jpeg_info
{

	unsigned int num_sections;

	// combined length of data in all sections
	unsigned long long jpeg_length_total;

	// whole list
	struct jpeg_section **section_list;

	// sub-lists for quick fuzz transformations
	struct jpeg_section **header_list;
	struct jpeg_section **stream_list;
	struct jpeg_section **segment_list;
};

// our representation of a jpeg file segment.
struct jpeg_section
{
	int type;

	unsigned char marker[MAX_MARKLEN];
	unsigned int length_marker;

	unsigned char len[MAX_LENLEN];	
	unsigned int length_len;
	

	unsigned char data[MAX_BUFSZ];
	unsigned long long length_data;


	unsigned long long length_total;
};


struct jpeg_section *section_create(int type, unsigned char *marker, unsigned int length_marker, unsigned char *len,
					unsigned int length_len, unsigned char *data, unsigned long long length_data, unsigned long long length_total);

struct jpeg_info *jpeg_create();


int has_len(int type);


// checks if there is a jpeg section beginning at the current position of fp. 
int file_has_section(FILE *fp);


// make sure we get full reads from files.
int file_read_all(FILE *fp, unsigned char *buf, int num);


int file_read_marker(FILE *fp, struct jpeg_section *ptr);


int file_read_section (FILE *fp);


struct jpeg_section *section_set_vals(struct jpeg_section *ptr, int type, unsigned char *marker, unsigned int length_marker, unsigned char *len,
                                        unsigned int length_len, unsigned char *data, unsigned long long length_data, unsigned long length_total);


void section_set_type_from_marker(struct jpeg_section *ptr);

int type_has_len_field(struct jpeg_section *ptr);

unsigned int convert_length(unsigned char *temp);

unsigned char *section_get_len(struct jpeg_section *ptr);

int file_read_stream(FILE *fp, unsigned char *buf);

int section_set_length_data(struct jpeg_section *ptr, unsigned long len);

int section_update_length_total(struct jpeg_section *ptr);

int file_read_until_uch(FILE *fp, unsigned char *buf, unsigned char ch);

unsigned char *section_get_len(struct jpeg_section *ptr); 

struct jpeg_section **jpeg_create_list(struct jpeg_info *jpg, int entries);

int type_from_marker(unsigned char marker);

struct jpeg_info *create_jpeg();

unsigned char section_get_marker(struct jpeg_section *ptr);
int jpeg_increase_section_count(struct jpeg_info *jpg);
int file_count_sections(const char *path);

int section_set_length_len(struct jpeg_section *ptr, unsigned long len);
unsigned long long section_get_length_total(struct jpeg_section *ptr);
unsigned int jpeg_increase_length_total(struct jpeg_info *ptr, unsigned int num);

struct jpeg_info *create_jpeg_from_file(const char *path);
unsigned int section_set_length_mark(struct jpeg_section *ptr, unsigned int num);
void *jpeg_to_buf(struct jpeg_info *jpg, void *buf);


// *** MACROS ***
#define CleanIfFileError(x, y) free(x); fclose(y);
#define AddChAndUpdate(x, y) (x->data)[length_data] = y; (x->length_total)++; (x->length_data)++;
