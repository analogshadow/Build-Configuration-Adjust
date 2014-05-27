/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2014 Stover Enterprises, LLC
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

#include "prototypes.h"

struct document_handling_context
{
 struct bca_context *ctx;
 int close_stack_depth, dmode_depth;
 int (*close_functions_stack[32]) (struct document_handling_context *, void *);
 void *close_functions_data[32];

 char output_buffer[1024];
 int output_buffer_length;
};

int push_close_function(struct document_handling_context *dctx,
                        int (*close_function) (struct document_handling_context *,
                                               void *),
                        void *data)
{
 int i;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: push_close_function()\n");

 if((i = dctx->close_stack_depth) > 31)
 {
  fprintf(stderr, "BCA: document stack too deep\n");
  return 1;
 }

 dctx->close_functions_data[i] = data;
 dctx->close_functions_stack[i] = close_function;
 dctx->close_stack_depth++;

 return 0;
}

int function_close(struct document_handling_context *dctx)
{
 int i;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_close()\n");

 if((i = dctx->close_stack_depth) == 0)
 {
  fprintf(stderr, "BCA: @dc()@ out of place, %s, line %d\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 if(dctx->close_functions_stack[i-1](dctx, dctx->close_functions_data[i-1]))
 {
  return 1;
 }

 dctx->close_stack_depth--;
 return 0;
}

int function_close_point(struct document_handling_context *dctx, void *data)
{
 fprintf(stdout, "</li>");
 return 0;
}

int function_dpoint(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpoint()\n");

 if(n_parameters != 1)
  return 1;

 fprintf(stdout, "<li>");

 return push_close_function(dctx, function_close_point, NULL);
}

int function_close_dmode(struct document_handling_context *dctx, void *data)
{
 dctx->dmode_depth--;
 return 0;
}

int function_dmode(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dmode()\n");

 if(n_parameters != 1)
  return 1;

 if(dctx->ctx->pass_number == 0)
 {
  dctx->ctx->loop_inputs = 1;
 }

 dctx->dmode_depth++;

 return push_close_function(dctx, function_close_dmode, NULL);
}

int function_close_part(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dpart(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpart()\n");

 return push_close_function(dctx, function_close_part, NULL);
}

int function_close_chapter(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dchapter(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dchapter()\n");

 return push_close_function(dctx, function_close_chapter, NULL);
}

int function_close_section(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dsection(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dsection()\n");

 return push_close_function(dctx, function_close_section, NULL);
}

int function_close_subsection(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dsubsection(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpart()\n");

 return push_close_function(dctx, function_close_part, NULL);
}



char *handle_document_functions(struct bca_context *ctx, char *key)
{
 char **parameters;
 int n_parameters, code = -5;
 struct document_handling_context *dctx;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: handle_document_functions()\n");

 if((dctx = ctx->dctx) == NULL)
 {
  fprintf(stderr, "BCA: use --document for files with document macros\n");
  return NULL;
 }

 if(parse_function_parameters(key, &parameters, &n_parameters))
  return NULL;

 if(strncmp(parameters[0] + 1, "mode", 4) == 0)
  code = function_dmode(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "point", 5) == 0)
  code = function_dpoint(dctx, parameters, n_parameters);

/* parts, chapters, sections, subsectons, insets, listings */

 if(strncmp(parameters[0] + 1, "c", 1) == 0)
  code = function_close(dctx);

 if(code == -5)
 {
  fprintf(stderr,
         "BCA: handle_document_functions(): I can't handle function: %s, %s, line %d\n",
         parameters[0], current_file_name(dctx->ctx), dctx->ctx->line_number);
  return NULL;
 }

 if(code == 0)
  return strdup("");

 return NULL;
}

char next_byte(char *contents, int length, int *offset)
{
 char c;

 c = contents[*offset];
 (*offset)++;
 return c;
}

void handle_output(struct bca_context *ctx,
                   struct document_handling_context *dctx,
                   char *string, int length)
{
 if(length == -1)
  length = strlen(string);

 if(length == 0)
  return;

 fprintf(stdout, "%s", string);
}

void add_to_output_buffer(struct bca_context *ctx,
                          struct document_handling_context *dctx,
                          char *string, int length)
{
 int i;

 if(dctx->output_buffer_length + length > 1023)
 {
  fprintf(stderr, "fix me\n");
 } else {
  for(i=0; i<length; i++)
  {
   dctx->output_buffer[dctx->output_buffer_length + i] = string[i];
  }
  dctx->output_buffer_length += length;
 }
}

int process_file(struct bca_context *ctx,
                 struct document_handling_context *dctx,
                 char *contents, int length)
{
 char c, key[256], *value;
 int offset = 0, index;

 dctx->output_buffer_length = 0;

 while(offset != length)
 {
  c = next_byte(contents, length, &offset);
  if(c != '@')
  {
   add_to_output_buffer(ctx, dctx, &c, 1);
  } else {

   if(ctx->pass_number == 1)
   {
    dctx->output_buffer[dctx->output_buffer_length] = 0;
    handle_output(ctx, dctx, dctx->output_buffer, dctx->output_buffer_length);
    dctx->output_buffer_length = 0;
   }

   index = 0;
   while(offset != length)
   {
    c = next_byte(contents, length, &offset);

    if(c == '\n')
     ctx->line_number++;

    if(c != '@')
    {
     if(index > 255)
     {
      fprintf(stderr, "BCA:, string_replace(): key is too long\n");
      return 1;
     }
     key[index++] = c;
    } else {
     break;
    }
   }

   if(index == 0)
   {
    /* @@ escapes out @ */
    add_to_output_buffer(ctx, dctx, &c, 1);
   } else {

    key[index] = 0;

    if((value = resolve_string_replace_key(ctx, key)) == NULL)
    {
     fprintf(stderr,
             "BCA: string_replace(): could not resolve key \"%s\": %s, line %d\n",
             key, current_file_name(ctx), ctx->line_number);
     return 1;
    }

    if(ctx->pass_number == 1)
     handle_output(ctx, dctx, value, -1);

    free(value);
   }
  }
 }

 if(ctx->pass_number == 1)
 {
  dctx->output_buffer[dctx->output_buffer_length] = 0;
  handle_output(ctx, dctx, dctx->output_buffer, dctx->output_buffer_length);
  dctx->output_buffer_length = 0;
 }

 return 0;
}

int document_mode(struct bca_context *ctx)
{
 int i, allocation_size, *length_array, *fd_array;
 char **contents_array;
 struct document_handling_context *dctx;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: document_mode()\n");

 allocation_size = sizeof(struct document_handling_context);
 if((dctx = (struct document_handling_context *)
            malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }
 memset(dctx, 0, allocation_size);
 dctx->ctx = ctx;
 ctx->dctx = dctx;

 allocation_size = ctx->n_input_files * sizeof(int);
 if((length_array = (int *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          (int) allocation_size, strerror(errno));
  return 1;
 }

 if((fd_array = (int *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          (int) allocation_size, strerror(errno));
  return 1;
 }

 allocation_size = ctx->n_input_files * sizeof(char *);
 if((contents_array = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          allocation_size, strerror(errno));
  return 1;
 }

 for(i=0; i<ctx->n_input_files; i++)
 {
  if(mmap_file(ctx->input_files[i],
               (void **) &(contents_array[i]),
               &(length_array[i]),
               &(fd_array[i])))
  {
   fprintf(stderr, "BCA: mmap_file() failed\n");
   return 1;
  }
 }

 ctx->loop_inputs = 1;
 while(ctx->loop_inputs)
 {
  ctx->loop_inputs = 0;
  for(i=0; i<ctx->n_input_files; i++)
  {
   ctx->input_file_index = i;
   ctx->line_number = 1;

   if(process_file(ctx, dctx, contents_array[i], length_array[i]))
    return 1;
  }
  ctx->pass_number++;
 }

 for(i=0; i<ctx->n_input_files; i++)
 {
  if(umap_file(contents_array[i], length_array[i], fd_array[i]) != 0)
  {
   fprintf(stderr, "BCA: umap_file() failed\n");
   return 1;
  }
 }

 free(contents_array);
 free(length_array);
 free(fd_array);

 return 0;
}
