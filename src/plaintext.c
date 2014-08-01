#include "plaintext.h"

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

struct right_justify_context
{
 struct plaintext_rendering_context *pr_ctx;
 int n_words, pass;
 int word_sizes[64];
};

int right_justify_consume_word(struct unicode_word_context *uwc,
                               void *data, int flags)
{
 struct right_justify_context *rjc = (struct right_justify_context *) data;

 if(rjc->n_words > 63)
 {
  fprintf(stderr, "BCA: plaintext engine: right_justify_consume_word(): too many words in buffer \"%s\"\n",
          rjc->word_buffer);
  return 1;
 }

 rjc->word_sizes[rjc->n_words] = uwc->n_characters;
 rjc->n_words++;
 return 0;
}

int pr_right_justified_page_output(struct plaintext_rendering_context *pr_ctx,
                                   char *buffer, int length)
{
 struct right_justify_context rjc;
 struct unicode_word_context *uwc;
 int i, total_chars = 0;

 rjc->pr_ctx = pr_ctx;
 rjc->pass = 0;
 rjc->n_words = 0;

 if((uwc = unicode_word_engine_initialize(&rjc, right_justify_consume_word)) == NULL)
  return 1;

 /* first pass finds word sizes */
 for(i=0; i<length; i++)
 {
  if(unicode_word_engine_consume_byte(uwc, (unsigned char) buffer[i])
  {
   return 1;
  }
 }

 /* will this fit on one line */
 for(i=0; i<rjc->n_words; i++)
 {
  total_chars += rjc->word_sizes[i] + 1;
 }
 if(toal_chars < pr->line_width)
 {

 }

 /* second pass does the output */
 pr_ctx->pass++;
 if(pr_ctx->output != NULL)
 {

  for(i=0; i<length; i++)
  {
   if(unicode_word_engine_consume_byte(uwc, (unsigned char) buffer[i])
   {
    return 1;
   }
  }
 }

 return unicode_word_engine_finalize(uwc);
}



int pr_advance_page(struct plaintext_rendering_context *pr_ctx)
{
 int i;
 int rows_left, page_number_length;
 char temp[128];

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 /* finish out the current page */
 if(pr_ctx->output != NULL)
 {
  for(i=0; i<rows_left; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 /* bottom margin */
 if(pr_ctx->output != NULL)
 {
  for(i=0; i<pe_ctx->bottom_margin; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 /* top margin */
 if(pr_ctx->output != NULL)
 {
  page_number_length = snprintf(temp, 128, "%d", pe_ctx->current_page + 1);
  for(i=0; i<pe_ctx->line_width - page_number_length; i++)
  {
   fprintf(pe_ctx->output, " ");
  }
  fprintf(pe_ctx->output, "%s\n", temp);

  if(pr_right_justified_page_output(pr_ctx, temp, temp_length))
   return 1;

  for(i=1; i<pe_ctx->top_margin; i++)
  {
   fprintf(pe_ctx->output, "\n");
  }
 }

 pr_ctx->current_row = 0;
 pr_ctx->current_column = 0;
 pr_ctx->current_page++;

 return 0;
}

int pr_advance_line(struct plaintext_rendering_context *pr_ctx)
{
 int rows_left;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 if(rows_left < 1)
  return pe_advance_page(pr_ctx);

 pr_ctx->current_row++;
 pr_ctx->current_column = 0;

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "\n");

 return 0;
}

int pr_advance_word(struct plaintext_rendering_context *pr_ctx,
                    struct unicode_word_context *uwc)
{
 int i, space_left_on_line;

 space_left_on_line = pr_ctx->line_width -
                      pr_ctx->left_margin_width -
                      pr_ctx->right_margin_width -
                      pr_ctx->current_column;

 /* unless this is the first word on the line, it will need a space */
 if(pr_ctx->current_column > pr_ctx->left_margin_width)
  space_left_on_line--;

 if(n_characters > space_left_on_line)
 {
  /* if we added word hyphenation / line wrapping to the word engine,
     here is where it would be invoked */

  /* we don't just print a newline, since we may need to move to the
     next page, etc*/
  if(pr_advance_line(pr_ctx))
   return 1;

  /* left margin logic */
  for(i=0; i<pr_ctx->left_margin_width; i++)
  {
   if(pr_ctx->output != NULL)
    fprintf(pe_ctx->output, " ");

   pr_ctx->current_column++;
  }

 }

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "%s", utf8);

 pr_ctx->current_column += n_characters;
 return 0;
}

int pe_advance_column(struct plaintext_rendering_context *pr_ctx, int n_cols)
{
 int i, space_left_on_line;

 space_left_on_line = pr_ctx->line_width -
                      pr_ctx->left_margin_width -
                      pr_ctx->right_margin_width -
                      pr_ctx->current_column;

 if(n_cols > space_left_on_line)
  return 1;

 if(pr_ctx->output != NULL)
 {
  for(i=0; i<n_cols; i++)
  {
   fprintf(pr_ctx->ouput, " ");
  }
 }

 pr_ctx->current_column += n_characters;
 return 0;
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

int plaintext_paragraph_open(struct plaintext_engine_context *pe_ctx)
{
 int i;

 if(pe_ctx->paragraph_indent > pe_ctx->effective_render_context->line_width)
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






















struct plaintext_rendering_context *
plaintext_rendering_context_new(struct plaintext_engine_context *pe_ctx,
                                FILE *output)
{
 struct plaintext_rendering_context *pr_ctx;
 int allocation_size;

 allocation_size = sizeof(struct plaintext_rendering_context);
 if((pr_ctx = (struct plaintext_rendering_context *)
              malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }

 pr_ctx->output = ouput;
 pr_ctx->line_width = 80;
 pr_ctx->page_length = 10;
 pr_ctx->top_margin = 1;
 pr_ctx->bottom_margin = 1;

 pr_ctx->plaintext_engine_context = pe_ctx;

 pe_ctx->current_column = 0;
 pe_ctx->current_row = 0;
 pe_ctx->current_page = 0;

 return pr_ctx;
}

int plaintext_rendering_context_finalize(struct plaintext_rendering_context *pr_ctx)
{
 if(pr_ctx == NULL)
  return 0;

 free(pr_ctx);
 return 0;
}

struct plaintext_rendering_context *
plaintext_rendering_context_copy(struct plaintext_rendering_context *source)
{
 struct plaintext_rendering_context *pr_ctx;
 int allocation_size;

 allocation_size = sizeof(struct plaintext_rendering_context);
 if((pr_ctx = (struct plaintext_rendering_context *)
              malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }

 pr_ctx->output = source->ouput;
 pr_ctx->line_width = source->line_width;
 pr_ctx->page_length = source->page_length;
 pr_ctx->top_margin = source->top_margin;
 pr_ctx->bottom_margin = source->bottom_margin;
 pr_ctx->plaintext_engine_context = source->plaintext_engine_context;
 pe_ctx->current_column = source->current_column;
 pe_ctx->current_row = source->current_row;
 pe_ctx->current_page = source->current_page;

 return pr_ctx;
}

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

int pe_advance_page(struct plaintext_rendering_context *pr_ctx)
{
 int i;
 int rows_left, page_number_length;
 char temp[128];

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 /* finish out the current line */
 if(pr_ctx->output != NULL)
 {
  if(pr_ctx->current_row != 0)
  {
   fprintf(pr_ctx->output, "\n");
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

int pe_advance_line(struct plaintext_rendering_context *pr_ctx)
{
 int rows_left;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 if(rows_left < 1)
  return pe_advance_page(pr_ctx);

 pr_ctx->current_row++;
 pr_ctx->current_column = 0;

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "\n");

 return 0;
}

int pe_advance_word(struct plaintext_rendering_context *pr_ctx, char *utf8, int n_characters)
{
 int i, space_left_on_line;

 if(n_characters == -1)
  n_characters = strlen(utf8); //fix me

 space_left_on_line = pr_ctx->line_width -
                      pr_ctx->left_margin_width -
                      pr_ctx->right_margin_width -
                      pr_ctx->current_column;

 /* unless this is the first word on the line, it will need a space */
 if(pr_ctx->current_column > pr_ctx->left_margin_width)
  space_left_on_line--;

 if(n_characters > space_left_on_line)
 {
  if(pe_advance_line(pr_ctx))
   return 1;

  for(i=0; i<pr_ctx->left_margin_width; i++)
  {
   if(pr_ctx->output != NULL)
    fprintf(pe_ctx->output, " ");

   pr_ctx->current_column++;
  }

 } else {
  if(pr_ctx->current_column > pr_ctx->left_margin_width)
  {
   if(strcmp(utf8, " ") != 0)
   {
    if(pr_ctx->output != NULL)
     fprintf(pr_ctx->output, " ");

    pr_ctx->current_column++;
   }
  }
 }

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "%s", utf8);

 pr_ctx->current_column += n_characters;
 return 0;
}

int pe_advance_column(struct plaintext_rendering_context *pr_ctx, int n_cols)
{
 int i, space_left_on_line;

 space_left_on_line = pr_ctx->line_width -
                      pr_ctx->left_margin_width -
                      pr_ctx->right_margin_width -
                      pr_ctx->current_column;

 if(n_cols > space_left_on_line)
  return 1;

 if(pr_ctx->output != NULL)
 {
  for(i=0; i<n_cols; i++)
  {
   fprintf(pr_ctx->ouput, " ");
  }
 }

 pr_ctx->current_column += n_characters;
 return 0;
}

int pe_consume_word(struct unicode_word_context *uwc, void *data, int flags)
{
 struct plaintext_engine_context *pe_ctx;
 struct plaintext_rendering_context *pr_ctx;

 pe_ctx = (struct plaintext_engine_context *) data;
 pr_ctx = pe_ctx->effective_rendering_context;

 return pe_advance_word(pr_ctx, uwc->word_buffer, uwc->n_characters);
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


int plaintext_paragraph_open(struct plaintext_engine_context *pe_ctx)
{
 int i;

 if(pe_ctx->paragraph_indent > pe_ctx->effective_render_context->line_width)
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

