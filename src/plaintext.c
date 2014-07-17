#include "prototypes.h"

struct plaintext_engine_context
{
 struct document_handling_context *dctx;
 struct unicode_word_context *uwc;
 int line_width, left_margin_width, right_margin_width;
 int current_column, current_row, current_page;
 int page_length, top_margin, bottom_margin;
 int paragraph_indent, paragraph_line_spacing;
 int show_toc;

 struct toc_element *toc_root, *toc_cursor, *index_head, *index_tail;

 FILE *output;
};

int pe_advance_line(struct plaintext_engine_context *pe_ctx);

int plaintext_add_toc_element(struct plaintext_engine_context *pe_ctx,
                              int type, char *name)
{
 struct toc_element *e, *i;

 if((e = new_toc_element(DLEVEL_PART, name)) == NULL)
  return 1;

 snprintf(e->page, 16, "%d", pe_ctx->current_page + 1);

 if(pe_ctx->toc_root == NULL)
 {
  pe_ctx->toc_cursor = pe_ctx->toc_root = e;
 } else {

  /* walk up the parantage until we find a sibling type, a paraent type, or tree root */
  i = pe_ctx->toc_cursor;
  while(i != NULL)
  {
   if(i->type > e->type)
   {
    e->parrent = i;
    i->child = e;
    pe_ctx->toc_cursor = e;
    return 0;
   } else if(i->type == e->type) {
    e->last = pe_ctx->toc_cursor;
    pe_ctx->toc_cursor->next = e;
    pe_ctx->toc_cursor = e;
    return 0;
   }

   /* parrent only of left of the row */
   while(i->last != NULL)
   {
    i = i->last;
   }
   i = i->parrent;
  }

  fprintf(stderr, "BCA: plain text engine toc error: the first (and thus topmost) "
          "toc level used was %s, yet attempt is made to use %s.\n",
          type_to_string(pe_ctx->toc_root->type), type_to_string(e->type));
  return 1;
 }

 return 0;
}

int pe_toc_cursor_advance(struct plaintext_engine_context *pe_ctx)
{
 struct toc_element *i;

 i = pe_ctx->toc_cursor;

 if(i->child != NULL)
 {
  i = i->child;
  pe_ctx->toc_cursor = i;
  return 0;
 }

 if(i->next != NULL)
 {
  i = i->next;
  pe_ctx->toc_cursor = i;
  return 0;
 }

 /* if there is no child, or next, then must find the nearest parent with a next */

 /* first go the the left of this row */
 while(i->last != NULL)
 {
  i = i->last;
 }

 i = i->parrent;
 while(i != NULL)
 {
  if(i->next != NULL)
  {
   i = i->next;
   pe_ctx->toc_cursor = i;
   return 0;
  }
  i = i->parrent;
 }

 /* indicate we are finished */
 pe_ctx->toc_cursor = NULL;
 return 0;
}

int pe_advance_page(struct plaintext_engine_context *pe_ctx)
{
 int i;
 int rows_left, page_number_length;
 char temp[128];

 rows_left = pe_ctx->page_length
           - pe_ctx->top_margin
           - pe_ctx->bottom_margin
           - pe_ctx->current_row;

 /* finish out the current line */
 if(pe_ctx->dctx->ctx->pass_number == 1)
 {
  if(pe_ctx->current_row != 0)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 /* finish out the current page */
 if(pe_ctx->dctx->ctx->pass_number == 1)
 {
  for(i=0; i<rows_left; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 /* bottom margin */
 if(pe_ctx->dctx->ctx->pass_number == 1)
 {
  for(i=0; i<pe_ctx->bottom_margin; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 /* top margin */
 if(pe_ctx->dctx->ctx->pass_number == 1)
 {
  page_number_length = snprintf(temp, 128, "%d", pe_ctx->current_page + 1);
  for(i=0; i<pe_ctx->line_width - page_number_length; i++)
  {
   fprintf(pe_ctx->output, " ");
  }
  fprintf(pe_ctx->output, "%s\n", temp);

  for(i=1; i<pe_ctx->bottom_margin; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 pe_ctx->current_row = 0;
 pe_ctx->current_column = 0;
 pe_ctx->current_page++;

 return 0;
}

int pe_advance_line(struct plaintext_engine_context *pe_ctx)
{
 int rows_left;

 rows_left = pe_ctx->page_length
           - pe_ctx->top_margin
           - pe_ctx->bottom_margin
           - pe_ctx->current_row;

 if(rows_left < 1)
  return pe_advance_page(pe_ctx);

 pe_ctx->current_row++;
 pe_ctx->current_column = 0;

 if(pe_ctx->dctx->ctx->pass_number == 1)
  fprintf(pe_ctx->output, "\n");

 return 0;
}

int pe_output(struct plaintext_engine_context *pe_ctx, char *utf8, int n_characters)
{
 int i, space_left_on_line;

 if(n_characters == -1)
  n_characters = strlen(utf8); //fix me

 space_left_on_line = pe_ctx->line_width -
                      pe_ctx->left_margin_width -
                      pe_ctx->right_margin_width -
                      pe_ctx->current_column;

 /* unless this is the first word on the line, it will need a space */
 if(pe_ctx->current_column > pe_ctx->left_margin_width)
  space_left_on_line--;

 if(n_characters > space_left_on_line)
 {
  if(pe_advance_line(pe_ctx))
   return 1;

  for(i=0; i<pe_ctx->left_margin_width; i++)
  {
   if(pe_ctx->dctx->ctx->pass_number == 1)
    fprintf(pe_ctx->output, " ");

   pe_ctx->current_column++;
  }

 } else {
  if(pe_ctx->current_column > pe_ctx->left_margin_width)
  {
   if(strcmp(utf8, " ") != 0)
   {
    if(pe_ctx->dctx->ctx->pass_number == 1)
     fprintf(pe_ctx->output, " ");

    pe_ctx->current_column++;
   }
  }
 }

 if(pe_ctx->dctx->ctx->pass_number == 1)
  fprintf(pe_ctx->output, "%s", utf8);

 pe_ctx->current_column += n_characters;
 return 0;
}

int pe_consume_word(struct unicode_word_context *uwc, void *data, int flags)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) data;

 return pe_output(pe_ctx, uwc->word_buffer, uwc->n_characters);
}

int pe_print_toc(struct plaintext_engine_context *pe_ctx)
{
 pe_ctx->toc_cursor = pe_ctx->toc_root;
 int i, type, length;
 char temp[256], *title;

 pe_advance_line(pe_ctx);

 length = snprintf(temp, 256, "Table of Contents");
 for(i=0; i < (pe_ctx->line_width - length) / 2; i++)
 {
  if(pe_output(pe_ctx, " ", 1))
   return 1;
 }

 if(pe_output(pe_ctx, temp, length))
  return 1;

 pe_advance_line(pe_ctx);
 pe_advance_line(pe_ctx);

 while(pe_ctx->toc_cursor != NULL)
 {
  type = pe_ctx->toc_cursor->type;
  title = pe_ctx->toc_cursor->name;

  switch(type)
  {
   case DLEVEL_PART:
        length = snprintf(temp, 256, " Part %d", pe_ctx->toc_cursor->count);
        if(pe_output(pe_ctx, temp, length))
         return 1;
        break;

   case DLEVEL_CHAPTER:
        break;

   case DLEVEL_SECTION:
        break;

   case DLEVEL_SUB:
        break;

   case DLEVEL_INSET:
        break;

   default:
        fprintf(stderr, "BCA: plaintext engine: unhandled toc type, %d\n", type);
  }

  if(title != NULL)
   length = snprintf(temp, 256, ": %s", title);

  length +=
   snprintf(temp + length, 256 - length,
            ", page %s", pe_ctx->toc_cursor->page);

  if(pe_output(pe_ctx, temp, length))
   return 1;

  pe_advance_line(pe_ctx);

  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 return pe_advance_page(pe_ctx);
}

int plaintext_start_document(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 int allocation_size;

 /* _start_document() is called on each loop */
 switch(dctx->ctx->pass_number)
 {
  case 0:
       if(dctx->render_engine_context != NULL)
       {
        fprintf(stderr, "BCA: plaintex_start_document(): render engine should not be initialized "
                " on start of first pass\n");
        return 1;
       }

       allocation_size = sizeof(struct plaintext_engine_context);
       if((pe_ctx = (struct plaintext_engine_context *)
                    malloc(allocation_size)) == NULL)
       {
        fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
                allocation_size, strerror(errno));
        return 1;
       }

       memset(pe_ctx, 0, allocation_size);
       pe_ctx->dctx = dctx;
       if((pe_ctx->uwc =
           unicode_word_engine_initialize(pe_ctx,
                                          pe_consume_word)) == NULL)
       {
        fprintf(stderr,
                "BCA: plaintext_start_document(): unicode_word_engine_initialize() failed.\n");
        free(pe_ctx);
        return 1;
       }

       pe_ctx->output = stdout;
       pe_ctx->line_width = 80;
       pe_ctx->page_length = 10;
       pe_ctx->top_margin = 1;
       pe_ctx->bottom_margin = 1;
       pe_ctx->paragraph_line_spacing = 1;
       pe_ctx->paragraph_indent = 4;
       pe_ctx->show_toc = 1;
       dctx->render_engine_context = pe_ctx;

       pe_ctx->current_column = 0;
       pe_ctx->current_row = 0;
       pe_ctx->current_page = 0;
       break;

  case 1:
       if((pe_ctx = (struct plaintext_engine_context *)
                     dctx->render_engine_context) == NULL)
       {
        fprintf(stderr, "BCA: plaintex_start_document(): render engine should have been initialized "
                " on start of first pass\n");
        return 1;
       }

       pe_ctx->current_column = 0;
       pe_ctx->current_row = 0;
       pe_ctx->current_page = 0;

       if(pe_ctx->show_toc == 1)
        if(pe_print_toc(pe_ctx))
         return 1;
       break;

  default:
       fprintf(stderr,
               "BCA: plaintext_start_document(): I should not have a pass %d\n",
               dctx->ctx->pass_number);
       return 1;
 }

 return 0;
}

int plaintext_finish_document(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(pe_advance_line(pe_ctx))
  return 1;

 switch(dctx->ctx->pass_number)
 {
  case 0:
       /* proceede to next pass */
       dctx->ctx->loop_inputs = 1;
       break;

  case 1:
       /* we're done. shutdown */
       unicode_word_engine_finalize(pe_ctx->uwc);
       break;

  default:
       fprintf(stderr,
               "BCA: plaintext_finish_document(): I should not have a pass %d\n",
               dctx->ctx->pass_number);
       return 1;
 }

 return 0;
}

int plaintext_paragraph_open(struct plaintext_engine_context *pe_ctx)
{
 int i;

 if(pe_ctx->paragraph_indent > pe_ctx->line_width)
 {
  fprintf(stderr, "BCA: plain text paragraph indent greater than line width!\n");
  return 1;
 }

 if(pe_advance_line(pe_ctx))
  return 1;

 for(i=0; i<pe_ctx->paragraph_indent; i++)
 {
  if(pe_ctx->dctx->ctx->pass_number == 1)
   fprintf(pe_ctx->output, " ");
  pe_ctx->current_column++;
 }

 return 0;
}

int plaintext_paragraph_close(struct plaintext_engine_context *pe_ctx)
{
 int i;

 for(i=0; i<pe_ctx->paragraph_line_spacing; i++)
 {
  pe_advance_line(pe_ctx);
 }

 return 0;
}


int plaintext_consume_text(struct document_handling_context *dctx,
                           char *text, int length)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;
 int i;

 for(i=0; i<length; i++)
 {
  if(unicode_word_engine_consume_byte(pe_ctx->uwc, (unsigned char) text[i]))
  {
   fprintf(stderr, "BCA: plaintext_consume_text(): "
           "unicode_word_engine_consume_byte() failed\n");
   return 1;
  }
 }

 return 0;
}

int plaintext_open_point(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_point(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_list(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_list(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_listing(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_listing(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_inset(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_inset(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_subsection(struct document_handling_context *dctx,
                              char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_subsection(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_section(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_section(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_chapter(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_chapter(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}


int plaintext_open_part(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *part_name = NULL;

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(n_parameters == 2)
  part_name = parameters[1];

 if(dctx->ctx->pass_number == 0)
 {
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_PART, part_name))
   return 1;
 }


 return 0;
}

int plaintext_close_part(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_table(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_table(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tr(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tr(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tc(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tc(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tag(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *tag_name;

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
  return plaintext_paragraph_open(pe_ctx);

 fprintf(stderr,
         "BCA: plaintext_open_tag(): warning, plain text engine does "
         "not support tag '%s'.\n",
         tag_name);

 return 0;
}

int plaintext_close_tag(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 char *tag_name;

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
  return plaintext_paragraph_close(pe_ctx);

 return 0;
}

int activate_document_engine_plaintext(struct document_handling_context *dctx)
{
 dctx->start_document = plaintext_start_document;
 dctx->finish_document = plaintext_finish_document;
 dctx->consume_text = plaintext_consume_text;
 dctx->open_point = plaintext_open_point;
 dctx->close_point = plaintext_close_point;
 dctx->open_list = plaintext_open_list;
 dctx->close_list = plaintext_close_list;
 dctx->open_listing = plaintext_open_listing;
 dctx->close_listing = plaintext_close_listing;
 dctx->open_inset = plaintext_open_inset;
 dctx->close_inset = plaintext_close_inset;
 dctx->open_subsection = plaintext_open_subsection;
 dctx->close_subsection = plaintext_close_subsection;
 dctx->open_section = plaintext_open_section;
 dctx->close_section = plaintext_close_section;
 dctx->open_chapter = plaintext_open_chapter;
 dctx->close_chapter = plaintext_close_chapter;
 dctx->open_part = plaintext_open_part;
 dctx->close_part = plaintext_close_part;
 dctx->open_table = plaintext_open_table;
 dctx->close_table = plaintext_close_table;
 dctx->open_tr = plaintext_open_tr;
 dctx->close_tr = plaintext_close_tr;
 dctx->open_tc = plaintext_open_tc;
 dctx->close_tc = plaintext_close_tc;
 dctx->open_tag = plaintext_open_tag;
 dctx->close_tag = plaintext_close_tag;

 return 0;
}
