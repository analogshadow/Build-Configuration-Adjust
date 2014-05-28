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

struct document_handling_context_stack_frame
{
 int (*close_function) (struct document_handling_context *, void *);
 void *data;
 int type, input_index, line_number;
};

#define DLEVEL_NONE    0
#define DLEVEL_PART    1
#define DLEVEL_CHAPTER 2
#define DLEVEL_SECTION 3
#define DLEVEL_SUB     4
#define DLEVEL_INSET   5
#define DLEVEL_LISTING 6

#define DSTACK_TYPE_TAG   100
#define DSTACK_TYPE_TABLE 101
#define DSTACK_TYPE_TR    102
#define DSTACK_TYPE_TC    103
#define DSTACK_TYPE_LIST  104
#define DSTACK_TYPE_POINT 105

struct document_handling_context
{
 struct bca_context *ctx;
 int dmode_depth;

 int table_depth, list_depth, in_tr, in_tc, in_point;

 int stack_depth;
 struct document_handling_context_stack_frame stack[64];

 int current_level;
 int implied_levels_mask[7];

 char output_buffer[1024];
 int output_buffer_length;
};

struct document_handling_context_stack_frame *
 backtrace_stack_to_last_frame_of_type(struct document_handling_context *dctx, int type)
{
 struct document_handling_context_stack_frame *frame;

 i = dctx->stack_depth;
 while(i > 0)
 {
  frame = &(dctx->stack[i]);
  if(frame->type == type)
   return frame;

  i--;
 }

 fprintf(stderr, "BCA: backtracing found no frame of type %d\n", type);
 return NULL;
}

int exit_level(struct document_handling_context *dctx)
{
 int i = dctx->current_level;
 while(i > 0)
 {
  if(dctx->start_of_level_stack_frames[i] != -1)
   break;

  i--;
 }

 dctx->current_level = i;
 return 0;
}

int enter_level(struct document_handling_context *dctx, int level_value)
{
 int i;

 if(level_value <= dctx->current_level)
 {
  fprintf(stderr, "BCA: enter_level() failing because %d <= %d\n",
          level_value, dctx->current_level);
  return 1;
 }

 for(i=dctx->current_level + 1; i<level_value; i++)
 {
  dctx->start_of_level_stack_frames[i] = -1;
 }

 dctx->start_of_level_stack_frames[level_value] = ctx->stack_depth;
 dctx->current_level = level_value;

 return 0;
}

int assert_starting_level_correctly(struct document_handling_context *dctx,
                                    int level_value, char *level_name)
{
 struct document_handling_context_stack_frame *frame;
 int logical_level;

 if(dctx->current_level < level_value)
  return 0;

 for(logical_level = dctx->current_level; logical_level > 0; logical_level--)
 {
  if(dctx->start_of_level_stack_frames[logical_level] != -1)
   break;
 }

 if(logical_level == 0)
 {
  fprintf(stderr, "BCA: something is wrong, %s, line %d\n", __FILE__, __LINE__)
  return 1;
 }

 fprintf(stderr,
         "BCA: %s, line %d: can not start %s, already inside a ",
         dctx->ctx->input_files[frame->input_index],
         frame->line_number);

 switch(logical_level)
 {
  case DLEVEL_PART:
       fprintf(stderr, "part");

  case DLEVEL_CHAPTER:
       fprintf(stderr, "chapter");

  case DLEVEL_SECTION:
       fprintf(stderr, "section");

  case DLEVEL_SUBSECTION:
       fprintf(stderr, "subsection");

  case DLEVEL_INSET:
       fprintf(stderr, "inset");

  case DLEVEL_LISTING:
       fprintf(stderr, "listing");
 }

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, logical_level)) == NULL)
  return 1;


 fprintf(stderr, " openned in %s, line %d\n.",
         dctx->ctx->input_files[frame->input_index],
         frame->line_number);

 return 1;
}

void table_error(struct document_handling_context *dctx)
{
 struct document_handling_context_stack_frame *frame;

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, DSTACK_TYPE_TABLE)) == NULL)
  return;

 fprintf(stderr,
         " inside table started in %s on line %d.\n",
         dctx->ctx->input_files[frame->input_index],
         frame->line_number);
}

void list_error(struct document_handling_context *dctx)
{
 struct document_handling_context_stack_frame *frame;

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, DSTACK_TYPE_LIST)) == NULL)
  return;

 fprintf(stderr,
         " inside list started in %s on line %d.\n",
         dctx->ctx->input_files[frame->input_index],
         frame->line_number);
}

int assert_not_in_list_or_table(struct document_handling_context *dctx,
                                char *type)
{
 if(dctx->table_depth != 0)
 {
  frpintf(stderr, "BCA: %s, line %d: %s attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number, type);

  table_error(dctx);
  return 1;
 }

 if(dctx->list_depth != 0)
 {
  frpintf(stderr, "BCA: %s, line %d: %s attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number, type);

  list_error(dctx);
  return 1;
 }

 return 0;
}

int push_close_function(struct document_handling_context *dctx,
                        int (*close_function) (struct document_handling_context *, void *),
                        void *data, int frame_type)
{
 struct document_handling_context_stack_frame *frame;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: push_close_function()\n");

 if(dctx->stack_depth > 63)
 {
  fprintf(stderr, "BCA: document stack too deep\n");
  return 1;
 }

 frame = &(dctx->stack[dctx->stack_depth]);

 frame->type = frame_type;
 frame->data = data;
 frame->close_function = close_function;
 frame->input_index = dctx->ctx->input_file_index;
 frame->line_number = dctx->ctx->line_number;

 dctx->stack_depth++;
 return 0;
}

int function_close(struct document_handling_context *dctx)
{
 struct document_handling_context_stack_frame *frame;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_close()\n");

 if(dctx->stack_depth == 0)
 {
  fprintf(stderr, "BCA: @dc()@ out of place, %s, line %d\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 frame = &(dctx->stack[dctx->stack_depth - 1]);

 if(frame->close_function(dctx, frame->data))
 {
  return 1;
 }

 frame->close_function = NULL;
 frame->data = NULL;
 frame->type = 0;
 frame->input_index = -1;
 frame->line_number = -1;

 dctx->stack_depth--;
 return 0;
}

int function_close_tag(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dtag(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtag()\n");

 return push_close_function(dctx, function_close_tag, NULL, DSTACK_TYPE_TAG);
}

int function_close_tr(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dtr(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 struct document_handling_context_tr_stack_frame *frame;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtr()\n");

 return push_close_function(dctx, function_close_tr, NULL, DSTACK_TYPE_TR);
}

int function_close_tc(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dtc(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtc()\n");

 return push_close_function(dctx, function_close_tc, NULL, DSTACK_TYPE_TC);
}

int function_close_table(struct document_handling_context *dctx, void *data)
{
 if(dctx->list_depth == 0)
 {
  frpintf(stderr, "BCA: %s, line %d: table close outside of table\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 dctx->table_depth--;
 return 0;
}

int function_dtable(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtable()\n");

 if(dctx->table_depth == 8)
 {
  frpintf(stderr, "BCA: %s, line %d: nested table depth exceeded\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 if(dctx->list_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: table attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number);

  list_error(dctx);
  return 1;
 }

 dctx->table_depth++;

 return push_close_function(dctx, function_close_table, NULL, DSTACK_TYPE_TABLE);
}

int function_close_point(struct document_handling_context *dctx, void *data)
{

 return 0;
}

int function_dpoint(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpoint()\n");

 if(assert_not_in_list_or_table(dctx, "point"))
  return 1;

 return push_close_function(dctx, function_close_point, NULL, DSTACK_TYPE_POINT);
}

int function_close_list(struct document_handling_context *dctx, void *data)
{
 if(dctx->list_depth == 0)
 {
  frpintf(stderr, "BCA: %s, line %d: listing close outside of list\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 dctx->list_depth--;
 return 0;
}

int function_dlist(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dlist()\n");

 if(dctx->list_depth == 8)
 {
  frpintf(stderr, "BCA: %s, line %d: nested list depth exceeded\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 if(dctx->table_depth != 0)
 {
  frpintf(stderr, "BCA: %s, line %d: list attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number);

  table_error(dctx);
  return 1;
 }

 dctx->list_depth++;

 return push_close_function(dctx, function_close_list, NULL, DSTACK_TYPE_LIST);
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

 return push_close_function(dctx, function_close_dmode, NULL, DLEVEL_NONE);
}

int function_close_part(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dpart(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpart()\n");

 if(assert_not_in_list_or_table(dctx, "part"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_PART, "part"))
  return 1;

 if(enter_level(dctx, DLEVEL_PART))
  return 1;

 return push_close_function(dctx, function_close_part, NULL, DLEVEL_PART);
}

int function_close_chapter(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dchapter(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dchapter()\n");

 if(assert_not_in_list_or_table(dctx, "chapter"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_CHAPTER, "chapter"))
  return 1;

 if(enter_level(dctx, DLEVEL_CHAPTER))
  return 1;

 return push_close_function(dctx, function_close_chapter, NULL, DLEVEL_CHAPTER);
}

int function_close_section(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dsection(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dsection()\n");

 if(assert_not_in_list_or_table(dctx, "section"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_SECTION, "section"))
  return 1;

 if(enter_level(dctx, DLEVEL_SECTION))
  return 1;

 return push_close_function(dctx, function_close_section, NULL, DLEVEL_SECTION);
}

int function_close_sub(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dsub(struct document_handling_context *dctx,
                  char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dsub()\n");

 if(assert_not_in_list_or_table(dctx, "subsection"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_SUBSECTION, "subsection"))
  return 1;

 if(enter_level(dctx, DLEVEL_SUBSECTION))
  return 1;

 return push_close_function(dctx, function_close_sub, NULL, DLEVEL_SUBSECTION);
}

int function_close_inset(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dinset(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dinset()\n");

 if(assert_not_in_list_or_table(dctx, "inset"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_INSET, "inset"))
  return 1;

 if(enter_level(dctx, DLEVEL_INSET))
  return 1;

 return push_close_function(dctx, function_close_inset, NULL, DLEVEL_INSET);
}

int function_close_listing(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dlisting(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dlisting()\n");

 if(assert_not_in_list_or_table(dctx, "listing"))
  return 1;

 if(assert_starting_level_correctly(dctx, DLEVEL_LISTING, "listing"))
  return 1;

 if(enter_level(dctx, DLEVEL_LISTING))
  return 1;

 return push_close_function(dctx, function_close_listing, NULL, DLEVEL_LISTING);
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

 if(strncmp(parameters[0] + 1, "part", 5) == 0)
  code = function_dpart(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "chapter", 7) == 0)
  code = function_dchapter(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "section", 7) == 0)
  code = function_dsection(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "sub", 3) == 0)
  code = function_dsub(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "inset", 5) == 0)
  code = function_dinset(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "listing", 7) == 0)
  code = function_dlisting(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "table", 5) == 0)
  code = function_dtable(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "tr", 2) == 0)
  code = function_dtr(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "tc", 2) == 0)
  code = function_dtc(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "tag", 3) == 0)
  code = function_dtag(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "list", 4) == 0)
  code = function_dlist(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "point", 5) == 0)
  code = function_dpoint(dctx, parameters, n_parameters);

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
