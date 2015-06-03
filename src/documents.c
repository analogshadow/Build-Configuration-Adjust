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

#include "prototypes.h"

char *type_to_string(int type)
{
 char error[64];

 switch(type)
 {
  case DLEVEL_NONE:
       return "none";

  case DLEVEL_DOCUMENT:
       return "doc";

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
  return DLEVEL_DOCUMENT;
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
  frame = &(dctx->stack[i - 1]);

  fprintf(stderr, "(%s in %s, line %d) ",
          type_to_string(frame->type),
          dctx->ctx->input_files[frame->input_file_index],
          frame->line_number);

  i--;
 }

 fprintf(stderr, "\n");

 return 1;
}

int document_frame_dump(struct document_handling_context *dctx)
{
 int i;
 struct document_handling_context_stack_frame *frame;

 fprintf(stderr, "BCA: Document Control Frame Backtrace:\n");

 for(i=dctx->stack_depth - 1; i>-1; i--)
 {
  frame = &(dctx->stack[i]);
  fprintf(stderr, "BCA: %d) %s:%d %s\n",
          i,
          dctx->ctx->input_files[frame->input_file_index],
          frame->line_number,
          type_to_string(frame->type));
 }

 return 0;
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

   document_frame_dump(dctx);
   return 1;
  }

  i--;
 }

 fprintf(stderr,
         "BCA: %s, line %d: expected to find openning of type %s before here.\n",
         dctx->ctx->input_files[dctx->ctx->input_file_index],
         dctx->ctx->line_number,
         type_to_string(need_to_find));

 document_frame_dump(dctx);
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
 int i;

 if(dctx->current_level == 0)
 {
  fprintf(stderr, "BCA: exit_level()y already at level 0!\n");
  return 1;
 }

 i = dctx->current_level - 1;

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
         "BCA: %s, line %d: can not start %s, already inside a %s\n",
         dctx->ctx->input_files[dctx->ctx->input_file_index],
         dctx->ctx->line_number,
         type_to_string(level_value),
         type_to_string(dctx->current_level));

 if((frame = backtrace_stack_to_last_frame_of_type(dctx, dctx->current_level)) == NULL)
  return 1;

 fprintf(stderr, " openned in %s, line %d\n.",
         dctx->ctx->input_files[frame->input_file_index],
         frame->line_number);

 document_frame_dump(dctx);
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
  document_frame_dump(dctx);
  return 1;
 }

 if(dctx->list_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: %s attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number, type);

  list_error(dctx);
  document_frame_dump(dctx);
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
  fprintf(stderr, "BCA: push_close_function(%d/%s, , , %d, %d)\n",
          frame_type, type_to_string(frame_type),
          dctx->ctx->input_file_index, dctx->ctx->line_number);

 if(dctx->stack_depth > 63)
 {
  fprintf(stderr, "BCA: document stack too deep\n");
  document_frame_dump(dctx);
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
  fprintf(stderr, "BCA: function_close(%d)\n", type);

 if(dctx->stack_depth == 0)
 {
  fprintf(stderr, "BCA: @dc()@ out of place, %s, line %d\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  document_frame_dump(dctx);
  return 1;
 }

 frame = &(dctx->stack[dctx->stack_depth - 1]);

 if(frame->type != type)
 {
  fprintf(stderr,
          "BCA: %s, line %d @dc()@ was expected to be closing a %s, "
          "not a %s opened in %s on line %d.\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number,
          type_to_string(type), type_to_string(frame->type),
          dctx->ctx->input_files[frame->input_file_index],
          frame->line_number);

  document_frame_dump(dctx);
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

 if(dctx->tag_datas[dctx->tag_depth] != NULL)
 {
  free(dctx->tag_datas[dctx->tag_depth]);
  dctx->tag_datas[dctx->tag_depth] = NULL;
 }

 return 0;
}

int function_dtag(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 int tag_length, i;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dtag()\n");

 if(n_parameters < 2 || n_parameters > 3)
 {
  fprintf(stderr, "BCA: %s, %d: tag() expectes 1 or 2 parameters, not %d\n",
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

 for(i = dctx->tag_depth - 1; i > -1; i--)
 {
  if(strcmp(dctx->tags[i], parameters[1]) == 0)
  {
   fprintf(stderr, "BCA: function_dtag(): %s, %d already inside of tag %s\n",
           current_file_name(dctx->ctx), dctx->ctx->line_number, parameters[1]);

   return 1;
  }
 }

 if(n_parameters == 2)
  dctx->tag_datas[dctx->tag_depth] = NULL;

 if(n_parameters == 3)
  dctx->tag_datas[dctx->tag_depth] = strdup(parameters[2]);

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
  document_frame_dump(dctx);
  return 1;
 }

 if(dctx->list_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: table attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number);

  list_error(dctx);
  document_frame_dump(dctx);
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
  fprintf(stderr, "BCA: %s, line %d: list close outside of list\n",
          current_file_name(dctx->ctx), dctx->ctx->line_number);
  document_frame_dump(dctx);
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
  document_frame_dump(dctx);
  return 1;
 }

 if(dctx->table_depth != 0)
 {
  fprintf(stderr, "BCA: %s, line %d: list attempted ",
          current_file_name(dctx->ctx), dctx->ctx->line_number);

  table_error(dctx);
  document_frame_dump(dctx);
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

 if(exit_level(dctx))
  return 1;

 return 0;
}

int function_dmode(struct document_handling_context *dctx,
                   char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dmode()\n");

 if(n_parameters != 1)
  return 1;

 if(enter_level(dctx, DLEVEL_DOCUMENT))
  return 1;

 dctx->dmode_depth++;

 return push_close_function(dctx, function_close_dmode, NULL, DLEVEL_DOCUMENT);
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

 if(strncmp(parameters[0] + 1, "doc", 3) == 0)
  code = function_dmode(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "part", 5) == 0)
  code = function_dpart(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "chapter", 7) == 0)
  code = function_dchapter(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "section", 7) == 0)
  code = function_dsection(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "sub", 3) == 0)
  code = function_dsub(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "inset", 5) == 0)
  code = function_dinset(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "listing", 7) == 0)
  code = function_dlisting(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "table", 5) == 0)
  code = function_dtable(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "tr", 2) == 0)
  code = function_dtr(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "tc", 2) == 0)
  code = function_dtc(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "tag", 3) == 0)
  code = function_dtag(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "list", 4) == 0)
  code = function_dlist(dctx, parameters, n_parameters);
 else if(strncmp(parameters[0] + 1, "point", 5) == 0)
  code = function_dpoint(dctx, parameters, n_parameters);
 else if(strncmp(key + 1, "c(", 2) == 0)
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

int handle_input(struct bca_context *ctx,
                 struct document_handling_context *dctx,
                 char *string, int length)
{
 if(length == -1)
  length = strlen(string);

 if(length == 0)
  return 0;

 if(dctx->consume_text != NULL)
 {
  return dctx->consume_text(dctx, string, length);
 } else {
  fprintf(stderr, "BCA: consume_text is NULL\n");
  return 1;
 }
 return 0;
}

int add_to_input_buffer(struct bca_context *ctx,
                        struct document_handling_context *dctx,
                        char *string, int length)
{
 int i;

 if(dctx->input_buffer_length + length > 1023)
 {
  fprintf(stderr, "BCA: documents.c add_to_input_buffer() fix me\n"
          "%d \"%s\" + %d \"%s\"\n",
          dctx->input_buffer_length,
          dctx->input_buffer,
          length, string);
  return 1;
 } else {
  for(i=0; i<length; i++)
  {
   dctx->input_buffer[dctx->input_buffer_length + i] = string[i];
  }
  dctx->input_buffer_length += length;
 }

 return 0;
}

int process_file(struct bca_context *ctx,
                 struct document_handling_context *dctx,
                 char *contents, int length)
{
 char c, key[256], *value;
 int offset = 0, index;

 dctx->input_buffer_length = 0;

 while(offset != length)
 {
  c = next_byte(contents, length, &offset);

  if(c == '\n')
   ctx->line_number++;

  if(c != '@')
  {
   if(add_to_input_buffer(ctx, dctx, &c, 1))
   {
    fprintf(stderr, "BCA: fail\n");
    return 1;
   }

   dctx->input_buffer[dctx->input_buffer_length] = 0;
   if(handle_input(ctx, dctx, dctx->input_buffer, dctx->input_buffer_length))
    return 1;
   dctx->input_buffer_length = 0;


  } else {

   dctx->input_buffer[dctx->input_buffer_length] = 0;
   if(handle_input(ctx, dctx, dctx->input_buffer, dctx->input_buffer_length))
    return 1;
   dctx->input_buffer_length = 0;

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
    if(add_to_input_buffer(ctx, dctx, &c, 1))
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

    if(handle_input(ctx, dctx, value, -1))
     return 1;

    free(value);
   }
  }
 }

 dctx->input_buffer[dctx->input_buffer_length] = 0;
 if(handle_input(ctx, dctx, dctx->input_buffer, dctx->input_buffer_length))
  return 1;
 dctx->input_buffer_length = 0;
 return 0;
}

int document_mode(struct bca_context *ctx)
{
 int i, allocation_size, *length_array, *fd_array;
 char **contents_array, *parameters[5];
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

 if((dctx->document_configuration_contents =
     read_file("./buildconfiguration/documentconfiguration",
               &(dctx->document_configuration_length), 0)) == NULL)
 {
  fprintf(stderr, "BCA: WARNING: document mode configuration file not found. "
          "Try --stubdocumentconfiguration.\n");
  return 1;
 }

 if(strcmp(ctx->engine_name, "plaintext") == 0) {
  if(activate_document_engine_plaintext(dctx))
  {
   fprintf(stderr,
           "BCA: document_mode(): activate_document_engine_plaintext() failed\n");
   return 1;
  }
 } else if(strcmp(ctx->engine_name, "passthrough") == 0) {
  if(activate_document_engine_passthrough(dctx))
  {
   fprintf(stderr,
           "BCA: document_mode(): activate_document_engine_passthrough() failed\n");
   return 1;
  }
 } else if(strcmp(ctx->engine_name, "cairo") == 0) {
  fprintf(stderr, "BCA: cairo documentation engine not implimented yet.\n");
  return 1;
 } else {
  fprintf(stderr, "BCA: unknown document mode engine \"%s\"\n", ctx->engine_name);
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

 /* engines that need multiple passes, must reset this back to 1 before
    finishing a pass. */
 ctx->loop_inputs = 1;

 /* special case handling for direct loco listing rendering */
 if(ctx->n_input_files == 1)
 {
  if((i = strlen(ctx->input_files[0])) > 4)
  {
   if(strncmp(ctx->input_files[0] + i - 5, ".loco", 5) == 0)
   {
    if(ctx->verbose > 0)
     fprintf(stderr, "BCA: using direct listing render mode\n");

    while(ctx->loop_inputs)
    {
     ctx->loop_inputs = 0;

     if(dctx->start_document != NULL)
      if(dctx->start_document(dctx))
       return 1;

     ctx->input_file_index = 0;
     ctx->line_number = 1;

     parameters[0] = "dlisting";
     parameters[1] = ""; /* caption */
     parameters[2] = "locolisting";
     parameters[3] = ctx->input_files[0];

     if(function_dlisting(dctx, parameters, 4))
      return 1;

     if(function_close(dctx, DLEVEL_LISTING))
      return 1;

     if(dctx->finish_document != NULL)
      if(dctx->finish_document(dctx))
       return 1;

     ctx->pass_number++;
    }
   }
  }
 }

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

struct toc_element *
new_toc_element(int type, char *name)
{
 struct toc_element *e;
 int allocation_size, length = 0;

 if(name != NULL)
  length = strlen(name) + 1;

 allocation_size = sizeof(struct toc_element)
                 + length;

 if((e = (struct toc_element *)
         malloc(allocation_size)) == NULL)
 {
  fprintf(stderr,
          "BCA: new_toc_element(): malloc(%d) failed.\n", allocation_size);
  return NULL;
 }

 e->type = type;
 e->name = ((char *) e) + sizeof(struct toc_element);
 snprintf(e->name, length, "%s", name);
 e->count = 0;
 e->page[0] = 0;
 e->last = NULL;
 e->next = NULL;
 e->child = NULL;
 e->parrent = NULL;

 return e;
}

int stub_document_configuration_file(struct bca_context *ctx)
{
 FILE *configuration = NULL;
 int n_records = 0, contents_length = 0;
 char *principles[80], *components[80], *keys[80], *values[80];
 char *contents;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "line_width";
 values[n_records] = "80";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "line_width";
 values[n_records] = "100";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "page_length";
 values[n_records] = "50";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "page_length";
 values[n_records] = "60";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "top_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "top_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "bottom_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "top_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "bottom_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "bottom_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "left_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "left_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "left_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "right_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "right_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "right_margin";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "show_page_numbers";
 values[n_records] = "yes";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "show_page_numbers";
 values[n_records] = "yes";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "show_page_numbers";
 values[n_records] = "no";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "paragraph_line_spacing";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "paragraph_line_spacing";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "paragraph_line_spacing";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "paragraph_indent";
 values[n_records] = "4";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "paragraph_indent";
 values[n_records] = "4";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "paragraph_indent";
 values[n_records] = "4";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "show_toc";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "show_toc";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "show_toc";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "show_index";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "show_index";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "show_index";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "text";
 keys[n_records] = "pad_listing_line_numbers";
 values[n_records] = "1";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "html";
 keys[n_records] = "pad_listing_line_numbers";
 values[n_records] = "0";
 n_records++;

 principles[n_records] = "plaintext";
 components[n_records] = "tty";
 keys[n_records] = "pad_listing_line_numbers";
 values[n_records] = "1";
 n_records++;


 contents =
 read_file("./buildconfiguration/documentconfiguration",
           &contents_length, 1);

 if((configuration = fopen("./buildconfiguration/documentconfiguration", "wb")) == NULL)
 {
  fprintf(stderr,
          "BCA: fopen(./buildconfiguration/documentconfiguration, w) failed, %s\n",
          strerror(errno));
  return 1;
 }

 if(output_modifications(ctx, configuration, contents, contents_length,
                         n_records, principles, components, keys, values))
 {
  fprintf(stderr, "BCA: output_modifications() failed\n");
  return 1;
 }

 fclose(configuration);
 return 0;
}

int conf_lookup_int(struct document_handling_context *dctx,
                    char *key, int *value, int default_value)
{
 char *v;

 if(dctx->document_configuration_contents == NULL)
 {
  return 1;
 }

 v = lookup_key(dctx->ctx,
                dctx->document_configuration_contents,
                dctx->document_configuration_length,
                dctx->ctx->engine_name,
                dctx->ctx->output_type,
                key);

 if(v == NULL)
 {
  v = lookup_key(dctx->ctx,
                 dctx->document_configuration_contents,
                 dctx->document_configuration_length,
                 dctx->ctx->engine_name, "ALL", key);
 }

 if(v == NULL)
 {
  fprintf(stderr,
          "BCA: WARNING Can not find %s.%s.%s in document configuration. "
          "Going with the default %d\n",
           dctx->ctx->engine_name, dctx->ctx->output_type,
           key, default_value);

  *value = default_value;
 } else {
  sscanf(v, "%d", value);
  free(v);
 }

 return 0;
}

int conf_lookup_string(struct document_handling_context *dctx,
                       char *key, char **value, char *default_value)
{
 char *v;

 if(dctx->document_configuration_contents == NULL)
 {
  return 1;
 }

 v = lookup_key(dctx->ctx,
                dctx->document_configuration_contents,
                dctx->document_configuration_length,
                dctx->ctx->engine_name,
                dctx->ctx->output_type,
                key);

 if(v == NULL)
 {
  v = lookup_key(dctx->ctx,
                 dctx->document_configuration_contents,
                 dctx->document_configuration_length,
                 dctx->ctx->engine_name, "ALL", key);
 }

 if(v == NULL)
 {
  fprintf(stderr,
          "BCA: WARNING Can not find %s.%s.%s in document configuration. "
          "Going with the default \"%s\"\n",
           dctx->ctx->engine_name, dctx->ctx->output_type,
           key, default_value);

  *value = default_value;
 } else {
  *value = v;
 }

 return 0;
}
