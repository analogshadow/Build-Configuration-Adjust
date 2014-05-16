/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2011,2012,2013,2014 Stover Enterprises, LLC
    (an Alabama Limited Liability Corporation), All rights reserved.
    See http://bca.stoverenterprises.com for more information.

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

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: escape_value()\n");

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

 if((extension_length = full_path_length - (index + 1)) > 0)
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
   memcpy(*extension, full_path + period_position + 1, extension_length);
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


