#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "convert.h"

void help(void);

int main(int argc, char **argv)
{
 int current_arg = 1, handled, mode = 0, length, i;
 struct locolisting_context context, *ctx = &context;
 char *filename = NULL;

 memset(ctx, 0, sizeof(struct locolisting_context));
 ctx->line_numbers = -1;
 
 if(argc < 2)
 {
  help();
  return 1;
 }

 while(current_arg < argc)
 {
  handled = 0;

  if(strcmp(argv[current_arg], "--help") == 0)
  {
   help();
   return 0;
  }

  if( (strcmp(argv[current_arg], "--verbose") == 0) ||
      (strcmp(argv[current_arg], "-v") == 0) )
  {
   handled = 1;
   ctx->verbose++;
  }

  if(strcmp(argv[current_arg], "--conversiontype") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--conversiontype expects a type\n");
    return 1;
   }

   handled = 1;
   ctx->conversion_type = argv[++current_arg];
  }

  if(strcmp(argv[current_arg], "--title") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--title expects a name\n");
    return 1;
   }

   handled = 1;
   ctx->title = argv[++current_arg];
  }

  if(strcmp(argv[current_arg], "--width") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--width expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->width));
  }

  if(strcmp(argv[current_arg], "--height") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--height expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->height));
  }

  if(strcmp(argv[current_arg], "--lcrop") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--lcrop expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->l_crop));
  }

  if(strcmp(argv[current_arg], "--rcrop") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--rcrop expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->r_crop));
  }

  if(strcmp(argv[current_arg], "--tcrop") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--tcrop expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->t_crop));
  }

  if(strcmp(argv[current_arg], "--bcrop") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--bcrop expects a number\n");
    return 1;
   }

   handled = 1;
   sscanf(argv[++current_arg], "%d", &(ctx->b_crop));
  }

  if(strcmp(argv[current_arg], "--linenumbers") == 0)
  {
   if(current_arg + 1 >= argc)
   {
    fprintf(stderr, "--linenumbers expects yes or no\n");
    return 1;
   }

   current_arg++;

   if(strcmp(argv[current_arg], "yes") == 0)
   {
    ctx->line_numbers = 1;
    handled = 1;
   }

   if(strcmp(argv[current_arg], "no") == 0)
   {
    ctx->line_numbers = 0;
    handled = 1;
   }


   if(handled == 0)
   {
    fprintf(stderr, "--linenumbers expects yes or no\n");
    return 1;
   }
  }

  if( (handled == 0) &&
      (current_arg == argc - 1) )
  {
   filename = argv[current_arg];
   handled = 1;
  }

  if(handled == 0)
  {
   fprintf(stderr, "unkown or out of place parameter, \"%s\"\n", argv[current_arg]);
   return 1;
  }

  current_arg++;
 }

 if(filename == NULL)
 {
  help();
  return 1;
 }

 if(ctx->conversion_type == NULL)
 {
  length = strlen(filename);
  i = length - 1;
  while(i > 0)
  {
   if(filename[i] == '.')
    break;
   i--;
  }

  if(strncmp(filename, "Makefile", 8) == 0)
   ctx->conversion_type = "Makefile";

  if(i>-1)
  {
   if( (strcmp(filename + i, ".c") == 0) ||
       (strcmp(filename + i, ".C") == 0) ||
       (strcmp(filename + i, ".h") == 0) ||
       (strcmp(filename + i, ".H") == 0) )
    ctx->conversion_type = "C";

  }

  if(ctx->conversion_type == NULL)
  {
   fprintf(stderr, "could not derive conversion type from input filename, going with \"plaintext\"\n");
   ctx->conversion_type = "plaintext";
  }
 }

 if(ctx->verbose)
  fprintf(stderr, "conversion type is %s\n", ctx->conversion_type);

 if(strcmp(ctx->conversion_type, "C") == 0)
 { 
  return convert_c_source(ctx, filename);
 }

 if(strcmp(ctx->conversion_type, "screenlog") == 0)
 { 
  return convert_screen_log(ctx, filename);
 }

 if(strcmp(ctx->conversion_type, "plaintext") == 0)
 { 
  return convert_plaintext(ctx, filename);
 }

 fprintf(stderr, "unkown conversion type: %s\n", ctx->conversion_type); 
 return 1;
}

int read_line(struct locolisting_context *ctx) 
{
 char c, *p;
 int allocation_size, bytes_read;

 ctx->line_length = 0;

 while(!feof(ctx->current_file))
 {
  bytes_read = fread(&c, 1, 1, ctx->current_file);

  if(bytes_read != 1)
  {
   if(feof(ctx->current_file))
    return 1;

   fprintf(stderr, "fread() error\n");
   return -1;
  }

  if(ctx->line_length == ctx->line_buffer_size)
  {
   allocation_size = ctx->line_buffer_size + 512;

   if(allocation_size > 4096)
   {
    fprintf(stderr, "%s line %d is too long\n",
            ctx->current_filename, ctx->line_number);
    return -1;
   }
 
   if((p = (char *) realloc(ctx->line_buffer, allocation_size)) == NULL)
   {
    fprintf(stderr, "realloc(%d) failed\n", allocation_size);
    return -1;
   }

   ctx->line_buffer = p;
   ctx->line_buffer_size = allocation_size;
  }

  if(c == '\n')
  {
   ctx->line_buffer[ctx->line_length] = 0;
   break;
  }

  ctx->line_buffer[ctx->line_length++] = c;
 }

 return feof(ctx->current_file);
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
   if(strcmp(string, (*array)[i]) == 0)
    return 1;
   i++;
  }
 }

 if(string_length == -1)
  string_length = strlen(string);

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

void help(void)
{
 fprintf(stderr, 
         "usage:\n"
         " locolisting [options] inputfilename\n"
         " where options are:\n"
         "  --verbose\n"
         "  --conversiontype TYPE\n"
         "  --linenumbers yes|no\n"
         "  --title TITLE\n"
         "  --width X\n"
         "  --height X\n"
         "  --lcrop|--rcrop|--hcrop|--bcrop X\n"
         "\n");
}

int convert_plaintext(struct locolisting_context *ctx, char *filename)
{
 char buffer[2];
 int line_width;

 if((ctx->current_file = fopen(filename, "r")) == NULL)
 {
  fprintf(stderr, "fopen(%s, r) failed\n", filename);
  return 1;
 }

 if(ctx->title == NULL)
  ctx->title = filename;

 printf("listing:\"%s\";\n", ctx->title);
 printf(" attribute:listingtype,plaintext;\n");

 if(ctx->line_numbers == 1)
  printf(" attribute:linenumbers,yes;\n");

 if(ctx->line_numbers == 0)
  printf(" attribute:linenumbers,no;\n");

 printf(" object:text,\"");

 line_width = 0;
 while(!feof(ctx->current_file))
 {
  if(fread(buffer, 1, 1, ctx->current_file) != 1)
   break;
  buffer[1] = 0;

  if(buffer[0] == '\n')
  {
   printf("\";\n"
          "  object:linebreak,\"");
   line_width = 0;
   continue;
  }

  if(line_width == ctx->width)
  {
   line_width = 0;
   printf("\";\n"
          "  object:linewrap,\"");
  }

  if(buffer[0] == '\\')
  {
   printf("\\\\");
   line_width++;
   continue;
  }

  if(buffer[0] == '"')
  {
   printf("\\\"");
   line_width++;
   continue;
  }

  printf("%s", buffer);
  line_width++;
 }
 printf("\";\n");

 fclose(ctx->current_file);
 return 0;
}

