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

char *type_to_string(int type)
{
 char error[64];

 switch(type)
 {
  case DLEVEL_NONE:
       return "none/doc";

  case DLEVEL_PART:
       return "part";

  case DLEVEL_CHAPTER:
       return "chapter";

  case DLEVEL_SECTION:
       return "section";

  case DLEVEL_SUB:
       return "subsection";

  case DLEVEL_INSET:
       return "inset";

  case DLEVEL_LISTING:
       return "listing";

  case DSTACK_TYPE_TAG:
       return "tag";

  case DSTACK_TYPE_TABLE:
       return "table";

  case DSTACK_TYPE_TR:
       return "tablerow";

  case DSTACK_TYPE_TC:
       return "tablecell";

  case DSTACK_TYPE_LIST:
       return "list";

  case DSTACK_TYPE_POINT:
       return "point";

 }

 snprintf(error, 64, "<failure, type_to_string(%d)>", type);
 return strdup(error);
}

int type_string_to_id(char *string)
{
 if(strcmp(string, "part") == 0)
 {
  return DLEVEL_PART;
 } else if(strcmp(string, "chapter") == 0) {
  return DLEVEL_CHAPTER;
 } else if(strcmp(string, "section") == 0) {
  return DLEVEL_SECTION;
 } else if(strcmp(string, "sub") == 0) {
  return DLEVEL_SUB;
 } else if(strcmp(string, "inset") == 0) {
  return DLEVEL_INSET;
 } else if(strcmp(string, "listing") == 0) {
  return DLEVEL_LISTING;
 } else if(strcmp(string, "tag") == 0) {
  return DSTACK_TYPE_TAG;
 } else if(strcmp(string, "table") == 0) {
  return DSTACK_TYPE_TABLE;
 } else if(strcmp(string, "tr") == 0) {
  return DSTACK_TYPE_TR;
 } else if(strcmp(string, "tc") == 0) {
  return DSTACK_TYPE_TC;
 } else if(strcmp(string, "list") == 0) {
  return DSTACK_TYPE_LIST;
 } else if(strcmp(string, "point") == 0) {
  return DSTACK_TYPE_POINT;
 } else if(strcmp(string, "doc") == 0) {
  return DLEVEL_NONE;
 }

 return -1;
}

int end_of_input_tests(struct document_handling_context *dctx)
{
 int i;
 struct document_handling_context_stack_frame *frame;

 if(dctx->stack_depth == 0)
  return 0;

 fprintf(stderr,
         "BCA: reached end of input before expected closings for the following: (depth %d) ",
         dctx->stack_depth);

 i = dctx->stack_depth;
 while(i > 0)
 {
  frame = &(dctx->stack[i]);

  fprintf(stderr, "(%s in %s, line %d) ",
          type_to_string(frame->type),
          dctx->ctx->input_files[frame->input_file_index],
          frame->line_number);

  i--;
 }

 fprintf(stderr, "\n");

 return 1;
}

int backtrace_stack_test(struct document_handling_context *dctx,
                         int need_to_find, int before_finding)
{
 int i;
 struct document_handling_context_stack_frame *frame;

 i = dctx->stack_depth;
 while(i > 0)
 {
  frame = &(dctx->stack[i]);
  if(frame->type == need_to_find)
   return 0;

  if(frame->type == before_finding)
  {
   fprintf(stderr,
           "BCA: %s, line %d: openning of type %s in %s, line %d would need to be "
           "followed by an openning of type %s before here in this context.\n",
           dctx->ctx->input_files[dctx->ctx->input_file_index],
           dctx->ctx->line_number,
           type_to_string(before_finding),
           dctx->ctx->input_files[frame->input_file_index],
           frame->line_number,
           type_to_string(need_to_find));
   return 1;
  }

  i--;
 }

 fprintf(stderr,
         "BCA: %s, line %d: expected to find openning of type %s before here.\n",
         dctx->ctx->input_files[dctx->ctx->input_file_index],
         dctx->ctx->line_number,
         type_to_string(need_to_find));

 return 1;
}

struct document_handling_context_stack_frame *
 backtrace_stack_to_last_frame_of_type(struct document_handling_context *dctx, int type)
{
 int i;
 struct document_handling_context_stack_frame *frame;

 i = dctx->stack_depth;
 while(i > 0)
 {
  frame = &(dctx->stack[i]);
  if(frame->type == type)
   return frame;

  i--;
 }

 fprintf(stderr, "BCA: backtracing found no frame of type %s\n", type_to_string(type));
 return NULL;
}

int exit_level(struct document_handling_context *dctx)
{
 int i = dctx->current_level;
 while(i > 0)
 {
  if(dctx->implied_levels_mask[i] != 1)
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
  dctx->implied_levels_mask[i] = 1;
 }

 dctx->implied_levels_mask[level_value] = 0;
 dctx->current_level = level_value;

 return 0;
}

int test_starting_level_correctly(struct document_handling_context *dctx, int level_value)
{
 struct document_handling_context_stack_frame *frame;

 if(dctx->current_level < level_value)
  return 0;

 fprintf(stderr,
         "BCA: %s, line %d: can not start %s, already inside a %s",
         dctx->ctx->input_files[dctx->ctx->input_file_index],
         dctx->ctx->line_number,
         type_to_string(level_value),
         type_to_string(dctx->current_level));

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, dctx->current_level)) == NULL)
  return 1;

 fprintf(stderr, " openned in %s, line %d\n.",
         dctx->ctx->input_files[frame->input_file_index],
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
         dctx->ctx->input_files[frame->input_file_index],
         frame->line_number);
}

void list_error(struct document_handling_context *dctx)
{
 struct document_handling_context_stack_frame *frame;

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, DSTACK_TYPE_LIST)) == NULL)
  return;

 fprintf(stderr,
         " inside list started in %s on line %d.\n",
         dctx->ctx->input_files[frame->input_file_index],
         frame->line_number);
}

int test_not_in_list_or_table(struct document_handling_context *dctx,
                                char *type)
{
 if(dctx->table_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: %s attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number, type);

  table_error(dctx);
  return 1;
 }

 if(dctx->list_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: %s attempted ",
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
  fprintf(stderr, "BCA: push_close_function(%d, , , %d, %d)\n",
          frame_type, dctx->ctx->input_file_index, dctx->ctx->line_number);

 if(dctx->stack_depth > 63)
 {
  fprintf(stderr, "BCA: document stack too deep\n");
  return 1;
 }

 frame = &(dctx->stack[dctx->stack_depth]);

 frame->type = frame_type;
 frame->data = data;
 frame->close_function = close_function;
 frame->input_file_index = dctx->ctx->input_file_index;
 frame->line_number = dctx->ctx->line_number;

 dctx->stack_depth++;

 return 0;
}

int function_close(struct document_handling_context *dctx, int type)
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

 if(frame->type != type)
 {
  fprintf(stderr, "BCA: %s, line %d dc() was expecting to be closing opening %s in %s on %d.\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number,
          type_to_string(type),
          dctx->ctx->input_files[frame->input_file_index],
          frame->line_number);

  return 1;
 }

 if(frame->close_function(dctx, frame->data))
 {
  return 1;
 }

 frame->close_function = NULL;
 frame->data = NULL;
 frame->type = 0;
 frame->input_file_index = -1;
 frame->line_number = -1;

 dctx->stack_depth--;
 return 0;
}

int function_close_tag(struct document_handling_context *dctx, void *data)
{
 if(dctx->close_tag != NULL)
  if(dctx->close_tag(dctx))
   return 1;

 dctx->tag_depth--;
 return 0;
}

int function_dtag(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 int tag_length;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtag()\n");

 if(n_parameters != 2)
 {
  fprintf(stderr, "BCA: %s, %d: tag() expectes 1 parameter, not %d\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number, n_parameters - 1);
  return 1;
 }

 if(dctx->tag_depth > 30)
 {
  fprintf(stderr, "BCA: tag depth too deep\n");
  return 1;
 }

 tag_length = strlen(parameters[1]) + 1;
 if(dctx->tag_buffer_length + tag_length >= 1024)
 {
  fprintf(stderr, "BCA: tag buffer full\n");
  return 1;
 }

 dctx->tags[dctx->tag_depth] = dctx->tag_buffer + dctx->tag_buffer_length;
 memcpy(dctx->tags[dctx->tag_depth], parameters[1], tag_length);
 dctx->tag_buffer_length += tag_length;
 dctx->tag_depth++;

 if(dctx->open_tag != NULL)
  if(dctx->open_tag(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_tag, NULL, DSTACK_TYPE_TAG);
}

int function_close_tr(struct document_handling_context *dctx, void *data)
{
 if(dctx->close_tr)
  if(dctx->close_tr(dctx))
   return 1;

 return 0;
}

int function_dtr(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtr()\n");

 if(backtrace_stack_test(dctx, DSTACK_TYPE_TABLE, DSTACK_TYPE_TR))
  return 1;

 if(dctx->open_tr != NULL)
  if(dctx->open_tr(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_tr, NULL, DSTACK_TYPE_TR);
}

int function_close_tc(struct document_handling_context *dctx, void *data)
{
 if(dctx->close_tc != NULL)
  if(dctx->close_tc(dctx))
   return 1;

 return 0;
}

int function_dtc(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtc()\n");

 if(backtrace_stack_test(dctx, DSTACK_TYPE_TR, DSTACK_TYPE_TC))
  return 1;

 if(dctx->open_tc != NULL)
  if(dctx->open_tc(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_tc, NULL, DSTACK_TYPE_TC);
}

int function_close_table(struct document_handling_context *dctx, void *data)
{
 if(dctx->list_depth == 0)
 {
  fprintf(stderr, "BCA: %s, line %d: table close outside of table\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 dctx->table_depth--;

 if(dctx->close_table != NULL)
  if(dctx->close_table(dctx))
   return 1;

 return 0;
}

int function_dtable(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtable()\n");

 if(dctx->table_depth == 8)
 {
  fprintf(stderr, "BCA: %s, line %d: nested table depth exceeded\n",
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

 if(dctx->open_table != NULL)
  if(dctx->open_table(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_table, NULL, DSTACK_TYPE_TABLE);
}

int function_close_point(struct document_handling_context *dctx, void *data)
{
 if(dctx->close_point != NULL)
  if(dctx->close_point(dctx))
   return 1;

 return 0;
}

int function_dpoint(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpoint()\n");

 if(backtrace_stack_test(dctx, DSTACK_TYPE_LIST, DSTACK_TYPE_POINT))
  return 1;

 if(dctx->open_point != NULL)
  if(dctx->open_point(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_point, NULL, DSTACK_TYPE_POINT);
}

int function_close_list(struct document_handling_context *dctx, void *data)
{
 if(dctx->list_depth == 0)
 {
  fprintf(stderr, "BCA: %s, line %d: listing close outside of list\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 dctx->list_depth--;

 if(dctx->close_list != NULL)
  if(dctx->close_list(dctx))
   return 1;

 return 0;
}

int function_dlist(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dlist()\n");

 if(dctx->list_depth == 8)
 {
  fprintf(stderr, "BCA: %s, line %d: nested list depth exceeded\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  return 1;
 }

 if(dctx->table_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: list attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number);

  table_error(dctx);
  return 1;
 }

 dctx->list_depth++;

 if(dctx->open_list != NULL)
  if(dctx->open_list(dctx, parameters, n_parameters))
   return 1;

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

 dctx->dmode_depth++;

 return push_close_function(dctx, function_close_dmode, NULL, DLEVEL_NONE);
}

int function_close_part(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_part != NULL)
  if(dctx->close_part(dctx))
   return 1;

 return 0;
}

int function_dpart(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpart()\n");

 if(test_not_in_list_or_table(dctx, "part"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_PART))
  return 1;

 if(enter_level(dctx, DLEVEL_PART))
  return 1;

 if(dctx->open_part != NULL)
  if(dctx->open_part(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_part, NULL, DLEVEL_PART);
}

int function_close_chapter(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_chapter != NULL)
  if(dctx->close_chapter(dctx))
   return 1;

 return 0;
}

int function_dchapter(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dchapter()\n");

 if(test_not_in_list_or_table(dctx, "chapter"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_CHAPTER))
  return 1;

 if(enter_level(dctx, DLEVEL_CHAPTER))
  return 1;

 if(dctx->open_chapter != NULL)
  if(dctx->open_chapter(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_chapter, NULL, DLEVEL_CHAPTER);
}

int function_close_section(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_section != NULL)
  if(dctx->close_section(dctx))
   return 1;

 return 0;
}

int function_dsection(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dsection()\n");

 if(test_not_in_list_or_table(dctx, "section"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_SECTION))
  return 1;

 if(enter_level(dctx, DLEVEL_SECTION))
  return 1;

 if(dctx->open_section != NULL)
  if(dctx->open_section(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_section, NULL, DLEVEL_SECTION);
}

int function_close_sub(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_subsection != NULL)
  if(dctx->close_subsection(dctx))
   return 1;

 return 0;
}

int function_dsub(struct document_handling_context *dctx,
                  char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dsub()\n");

 if(test_not_in_list_or_table(dctx, "subsection"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_SUB))
  return 1;

 if(enter_level(dctx, DLEVEL_SUB))
  return 1;

 if(dctx->open_subsection != NULL)
  if(dctx->open_subsection(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_sub, NULL, DLEVEL_SUB);
}

int function_close_inset(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_inset != NULL)
  if(dctx->close_inset(dctx))
   return 1;

 return 0;
}

int function_dinset(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dinset()\n");

 if(test_not_in_list_or_table(dctx, "inset"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_INSET))
  return 1;

 if(enter_level(dctx, DLEVEL_INSET))
  return 1;

 if(dctx->open_inset != NULL)
  if(dctx->open_inset(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_inset, NULL, DLEVEL_INSET);
}

int function_close_listing(struct document_handling_context *dctx, void *data)
{
 if(exit_level(dctx))
  return 1;

 if(dctx->close_listing != NULL)
  if(dctx->close_listing(dctx))
   return 1;

 return 0;
}

int function_dlisting(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dlisting()\n");

 if(test_not_in_list_or_table(dctx, "listing"))
  return 1;

 if(test_starting_level_correctly(dctx, DLEVEL_LISTING))
  return 1;

 if(enter_level(dctx, DLEVEL_LISTING))
  return 1;

 if(dctx->open_listing != NULL)
  if(dctx->open_listing(dctx, parameters, n_parameters))
   return 1;

 return push_close_function(dctx, function_close_listing, NULL, DLEVEL_LISTING);
}

char *handle_document_functions(struct bca_context *ctx, char *key)
{
 char **parameters;
 int n_parameters, code = -5, type;
 struct document_handling_context *dctx;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: handle_document_functions(%s)\n", key);

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

 if(strncmp(key + 1, "c(", 2) == 0)
 {
  if(n_parameters != 2)
  {
   fprintf(stderr, "BCA: dc() needs a type\n");
   return NULL;
  }
  if((type = type_string_to_id(parameters[1])) == -1)
  {
   fprintf(stderr, "BCA: %s, line %d: unknown type %s\n",
           current_file_name(dctx->ctx),
           dctx->ctx->line_number,
           parameters[1]);
   return NULL;
  }
  code = function_close(dctx, type);
 }

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

int handle_output(struct bca_context *ctx,
                  struct document_handling_context *dctx,
                  char *string, int length)
{
 if(length == -1)
  length = strlen(string);

 if(length == 0)
  return 0;

 if(dctx->consume_text != NULL)
  return dctx->consume_text(dctx, string, length);

 return 0;
}

int add_to_output_buffer(struct bca_context *ctx,
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

 return 0;
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
   if(add_to_output_buffer(ctx, dctx, &c, 1))
    return 1;
  } else {

   dctx->output_buffer[dctx->output_buffer_length] = 0;
   if(handle_output(ctx, dctx, dctx->output_buffer, dctx->output_buffer_length))
    return 1;
   dctx->output_buffer_length = 0;

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
    if(add_to_output_buffer(ctx, dctx, &c, 1))
     return 1;
   } else {

    key[index] = 0;

    if((value = resolve_string_replace_key(ctx, key)) == NULL)
    {
     fprintf(stderr,
             "BCA: string_replace(): could not resolve key \"%s\": %s, line %d\n",
             key, current_file_name(ctx), ctx->line_number);
     return 1;
    }

    if(handle_output(ctx, dctx, value, -1))
     return 1;

    free(value);
   }
  }
 }

 dctx->output_buffer[dctx->output_buffer_length] = 0;
 if(handle_output(ctx, dctx, dctx->output_buffer, dctx->output_buffer_length))
  return 1;
 dctx->output_buffer_length = 0;
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

 if(activate_document_engine_plaintext(dctx))
 {
  fprintf(stderr,
          "BCA: document_mode(): activate_document_engine_plaintext() failed\n");
  return 1;
 }

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

  if(dctx->start_document != NULL)
   if(dctx->start_document(dctx))
    return 1;

  for(i=0; i<ctx->n_input_files; i++)
  {
   ctx->input_file_index = i;
   ctx->line_number = 1;

   if(process_file(ctx, dctx, contents_array[i], length_array[i]))
    return 1;
  }
  if(end_of_input_tests(dctx))
   return 1;

  if(dctx->finish_document != NULL)
   if(dctx->finish_document(dctx))
    return 1;

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
