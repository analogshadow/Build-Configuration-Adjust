/* GPLv3

    Build Configuration Adjust, is a source configuration and Makefile
    generation tool.
    Copyright © 2015 C. Thomas Stover.
    Copyright © 2012,2013,2014 Stover Enterprises, LLC (an Alabama
    Limited Liability Corporation).
    All rights reserved.
    See https://github.com/ctstover/Build-Configuration-Adjust for more
    information.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "prototypes.h"
#endif

#ifdef HAVE_MMAP
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

int split_strings(struct bca_context *ctx, char *source, int length,
                  int *count, char ***strings)
{
 int pass = 0, allocation_size, index, n_strings = 0, substring_length, x, start;
 char *string, **array;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: split_strings(\"%s\")\n", source);
  fflush(stderr);
 }

 if(source == NULL)
 {
  *count = 0;
  *strings = NULL;
  return 0;
 }

 if(length == -1)
  length = strlen(source);

 if(length == 0)
 {
  *strings = NULL;
  *count = 0;
  return 0;
 }

 while(pass < 2)
 {
  start = -1;
  index = 0;
  x = 0;
  while(index < length)
  {

   while(index < length)
   {
    if(source[index] == ' ')
     break;

    index++;
   }

   if(pass == 0)
    n_strings++;

   if(pass == 1)
   {
    substring_length = (index - start) - 1;

    allocation_size = substring_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: split_strings(): malloc(%d) failed, %s\n",
             allocation_size, strerror(errno));
     return 1;
    }

    memcpy(string, source + start + 1, substring_length);
    string[substring_length] = 0;
    array[x++] = string;
   }

   start = index;
   index++;
  }

  if(pass == 0)
  {
   /* allocated the pointer array */
   allocation_size = n_strings * sizeof(char *);
   if((array = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: split_strings(): malloc(%d) failed, %s\n",
            allocation_size, strerror(errno));
    return 1;
   }
  }

  pass++;
 }

 *strings = array;
 *count = n_strings;
 return 0;
}

/* escapes quotes and white space */
char *escape_value(struct bca_context *ctx, char *source, int length)
{
 int allocation_size, n_quotes = 0, n_spaces = 0, i, x;
 char *new_string;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: escape_value(\"%s\")\n", source);

 if(source == NULL)
  return NULL;

 if(length == -1)
  length = strlen(source);

 for(i=0; i<length; i++)
 {
  if(source[i] == ' ')
   n_spaces++;

  if(source[i] == '"')
   n_quotes++;
 }

 if(n_spaces + n_quotes == 0)
  return source;

 allocation_size = length + 3 + n_quotes;
 if((new_string = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return NULL;
 }

 x = 0;
 if(n_spaces)
  new_string[x++] = '"';

 for(i=0; i<length; i++)
 {
  if(source[i] == '"')
  {
   new_string[x++] = '\\';
   new_string[x++] = '"';
  } else {
   new_string[x++] = source[i];
  }
 }

 if(n_spaces)
  new_string[x++] = '"';

 new_string[x] = 0;

 return new_string;
}

/* returns 1 is search is within source */
int contains_string(char *source, int source_length, char *search, int search_length)
{
 int x;

 if(source == NULL)
  return 0;

 if(search == NULL)
  return 0;

 if(source_length == -1)
  source_length = strlen(source);

 if(search_length == -1)
  search_length = strlen(search);

 x = 0;
 while( (x + search_length) < source_length)
 {
  if(strncmp(source + x, search, search_length) == 0)
   return 1;

  x++;
 }

 return 0;
}

int in_string(char *string, char *substring, int substring_length)
{
 int string_i = 0;
 if(substring_length < 0)
  substring_length = strlen(substring);

 while(string[string_i] != 0)
 {
  if(strncmp(string + string_i, substring, substring_length) == 0)
   return string_i;

  string_i++;
 }

 return -1;
}

int path_extract(const char *full_path, char **base_file_name, char **extension)
{
 int allocation_size, full_path_length, base_file_name_length, extension_length,
     index, period_position = -1, slash_position = -1;

 if(full_path == NULL)
  return 1;

 full_path_length = strlen(full_path);

 index = full_path_length;
 while(index > 0)
 {
  if(full_path[index] == '/')
  {
   index = full_path_length;
   break;
  }

  if(full_path[index] == '.')
   break;
  index--;
 }

 if((extension_length = full_path_length - index) > 0)
  period_position = index;

 if(period_position == -1)
 {
  if(extension != NULL)
   *extension = NULL;

  period_position = index;
 } else {
  if(extension != NULL)
  {
   allocation_size = extension_length + 1;
   if((*extension = (char *) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
    return 1;
   }
   memcpy(*extension, full_path + period_position, extension_length);
   (*extension)[extension_length] = 0;
  }
 }

 index = period_position;
 while(index > 0)
 {
  if(full_path[index] == '/')
  {
   slash_position = index;
   break;
  }

  index--;
 }

 base_file_name_length = period_position - (slash_position + 1);

 if(base_file_name != NULL)
 {
  allocation_size = base_file_name_length + 1;
  if((*base_file_name = (char *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
   return 1;
  }
  memcpy(*base_file_name, full_path + slash_position + 1, base_file_name_length);
  (*base_file_name)[base_file_name_length] = 0;
 }

 return 0;
}

int mmap_file(char *name, void **p_ptr, int *length_ptr, int *fd_ptr)
{
#ifdef HAVE_MMAP
 int fd;
 struct stat meta;
 void *p;
 size_t file_length;

 if((fd = open(name, O_RDONLY)) < 0)
 {
  fprintf(stderr, "BCA: open(%s) failed, %s\n", name, strerror(errno));
  return 1;
 }

 if(fstat(fd, &meta))
 {
  fprintf(stderr, "BCA: fstat() failed, %s\n", strerror(errno));
  close(fd);
  return 1;
 }

 if((file_length = meta.st_size) > (100 * 1024 * 1024))
 {
  fprintf(stderr,
          "BCA: read_file(): %s is %llu bytes. Most likely something is wrong. "
          "If you realy want to process a file greater than 100MB, please "
          "edit %s line %d\n.",
          name, (unsigned long long int) file_length, __FILE__, __LINE__);
  close(fd);
  return 1;
 }

 if(file_length == 0)
 {
  fprintf(stderr, "BCA: mmap_file(): %s is 0 bytes long.\n", name);
  close(fd);
  return 1;
 }

 if((p = mmap(NULL, file_length, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
 {
  fprintf(stderr,
          "BCA: mmap(NULL, %d, PROT_READ, MAP_SHARED, %d, 0) failed, %s\n",
          (int) file_length, fd, strerror(errno));
  return 1;
 }

 *length_ptr = (int) file_length;
 *fd_ptr = fd;
 *p_ptr = p;
 return 0;
#else
 char *c;
 int length;
 if((c = read_file(name, &length, 0)) == NULL)
  return 1;

 *p_ptr = c;
 *length_ptr = length;
 *fd_ptr = -1;
 return 0;
#endif
}

int umap_file(void *p, int length, int fd)
{
#ifdef HAVE_MMAP
 if(munmap(p, length))
 {
  fprintf(stderr, "BCA: munmap() failed, %s\n", strerror(errno));
  return 1;
 }
 close(fd);
 return 0;
#else
 if(p == NULL)
  return 1;

 if(length < 1)
  return 1;

 if(fd != -1)
  return 1;

 free(p);
 return 0;
#endif
}

char *read_file(char *name, int *length, int silent_test)
{
 int file_length, read_length;
 int allocation_size;
 char *contents;
 FILE *f;

 if((f = fopen(name, "r")) == NULL)
 {
  if(silent_test == 0)
   fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  return NULL;
 }

 if(fseek(f, 0, SEEK_END))
 {
  fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  fclose(f);
  return NULL;
 }

 if((file_length = ftell(f)) < 0)
 {
  fprintf(stderr, "BCA: ftell(%s) failed", name);
  fclose(f);
  return NULL;
 }

 if(fseek(f, 0, SEEK_SET))
 {
  fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  fclose(f);
  return NULL;
 }

 allocation_size = file_length + 1;
 if((contents = (char *)
                malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  fclose(f);
  return NULL;
 }

 if((read_length = fread(contents, 1, file_length, f)) < file_length)
 {
  fprintf(stderr, "BCA: fread() returned %d instead of %d",
            (int) read_length, (int) file_length);
  fclose(f);
  return NULL;
 }
 contents[read_length] = 0;

 fclose(f);

 *length = read_length;

 return contents;
}

int find_line(char *buffer, int buffer_length, int *start, int *end, int *line_length)
{
 int x;

 if(buffer == NULL)
 {
  *line_length = 0;
  return 0;
 }

 *start = *end + 1;
 x = *start;

 while(x < buffer_length)
 {
  if(*start > buffer_length)
   return 0;

  if(buffer[x] == 13)
  {
   if(x + 1 <= buffer_length)
   {
    if(buffer[x] == 10)
     x++;
   }
   break;
  }

  if(buffer[x] == 10)
   break;

  if(buffer[x] == 0)
   break;

  x++;
 }

 if((*line_length = x - *start) == 0)
 {
  if(buffer_length > x)
  {
   /*  fprintf(stderr, "BCA: Blank line in input file!\n"); */
   *end = x;
   return find_line(buffer, buffer_length, start, end, line_length);
  }
  return 0;
 }

 *end = x;
 return 1;
}

int add_to_string_array(char ***array, int array_size,
                        char *string, int string_length,
                        int prevent_duplicates)
{
 char **new_ptr, *new_string;
 int allocation_size, i;

 if(prevent_duplicates)
 {
  i = 0;
  while(i<array_size)
  {
   if( ( (*array)[i] == NULL) || (string == NULL) )
   {
    if((*array)[i] != NULL)
     return 1;
    if(string != NULL)
     return 1;
   } else {
    if(strcmp(string, (*array)[i]) == 0)
     return 1;
   }
   i++;
  }
 }

 if(string == NULL)
 {
  string_length = 0;
 } else {
  if(string_length == -1)
   string_length = strlen(string);
 }

 allocation_size = (array_size + 1) * sizeof(char *);

 if((new_ptr = (char **) realloc(*array, allocation_size)) == NULL)
 {
  perror("BCA: realloc()");
  return -1;
 }

 *array = new_ptr;

 if(string != NULL)
 {
  allocation_size = string_length + 1;
  if((new_string = (char *) malloc(allocation_size)) == NULL)
  {
   perror("BCA: malloc()");
   return -1;
  }
  snprintf(new_string, allocation_size, "%s", string);
  (*array)[array_size] = new_string;
 } else {
  (*array)[array_size] = NULL;
 }

 return 0;
}

int free_string_array(char **array, int n_elements)
{
 int x;

 for(x=0; x<n_elements; x++)
 {
  if(array[x] != NULL)
   free(array[x]);
 }
 free(array);

 return 0;
}

int append_array(char **source_array, int source_array_count,
                 char ***manipulate_array, int *manipulate_array_count,
                 int prevent_duplicates)
{
 int i, code;

 for(i=0; i<source_array_count; i++)
 {
  if((code =
      add_to_string_array(manipulate_array,
                          *manipulate_array_count,
                          source_array[i], -1,
                          prevent_duplicates)) == -1)
  {
   return 1;
  }
  if(code == 0)
   (*manipulate_array_count)++;
 }

 return 0;
}

int append_masked_array(char **source_array, int source_array_count,
                        char **mask_array, int mask_array_count,
                        char ***manipulate_array, int *manipulate_array_count,
                        int prevent_duplicates)
{
 int i, j, code, yes;

 for(i=0; i<source_array_count; i++)
 {
  yes = 1;
  j = 0;
  while(j < mask_array_count)
  {
   if(strcmp(source_array[i], mask_array[j]) == 0)
   {
    yes = 0;
    break;
   }
   j++;
  }

  if(yes)
  {
   if((code =
       add_to_string_array(manipulate_array,
                           *manipulate_array_count,
                           source_array[i], -1,
                           prevent_duplicates)) == -1)
   {
    return 1;
   }
   if(code == 0)
    (*manipulate_array_count)++;
  }
 }

 return 0;
}

char *join_strings(char **list, int n_elements)
{
 char *value;
 int allocation_size, i, length;

 if(n_elements == 0)
  return NULL;

 allocation_size = 1;
 for(i=0; i < n_elements; i++)
 {
  allocation_size += strlen(list[i]) + 1;
 }

 if((value = (char *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "malloc(%d) failed, %s\n",
          allocation_size, strerror(errno));
  return NULL;
 }

 length = 0;
 for(i=0; i < n_elements; i++)
 {
  length +=
  snprintf(value + length, allocation_size - length,
           "%s ", list[i]);
 }

 return value;
}

int next_character(char *buffer, int length)
{
 int expected_character_length = 0, buffer_i = 0;
 unsigned char byte;

 while(buffer_i<length)
 {
  byte = (unsigned char) buffer[buffer_i];

  if(expected_character_length == 0)
  {
   if( (byte & 128) == 0)
   {
    /* 1 byte UTF-8 character */
    expected_character_length = 1;

   } else if( (byte & 224) == 192) {
    /* 2 byte UTF-8 character */
    expected_character_length = 2;

   } else if( (byte & 240) == 224) {
    /* 3 byte UTF-8 character */
    expected_character_length = 3;

   } else if( (byte & 248) == 240) {
    /* 4 byte UTF-8 character */
    expected_character_length = 4;

   } else if( (byte & 252) == 248) {
    /* 5 byte UTF-8 character */
    expected_character_length = 5;

   } else if( (byte & 254) == 252) {
    /* 6 byte UTF-8 character */
    expected_character_length = 6;

   } else {
    fprintf(stderr,
            "BCA: next_characters(\"%s\", %d): UTF-8 encoding error: "
            "byte value out of sequence: %xh\n",
            buffer, length, byte);
    return -1;
   }

  } else {
   if( (byte & 192) != 128)
   {
    fprintf(stderr,
            "BCA: next_character(): UTF-8 encoding error:"
            "byte value out of sequence: %xh (expected byte %d of a %d byte value)\n",
             byte, buffer_i + 1, expected_character_length);
    return -1;
   }
  }

  buffer_i++;

  if(buffer_i == expected_character_length)
   return buffer_i;
 }

 return -1;
}

int count_characters(char *buffer, int length)
{
 int n_characters = 0, code, buffer_i = 0;

 while(buffer_i < length)
 {

  if((code = next_character(buffer + buffer_i, length - buffer_i)) < 0)
   return -1;

  buffer_i += code;
  n_characters++;
 }

 return n_characters;
}

int n_bytes_for_n_characters(char *buffer, int length, int n_characters)
{
 int i = 0, n_bytes = 0, c, code;

 for(c=0; c<n_characters; c++)
 {
  if((code = next_character(buffer + i, length - i)) < 0)
   return -1;

  n_bytes += code;
 }

 return n_bytes;
}

