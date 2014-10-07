#include "plaintext.h"

int pr_feed_generated_words(struct plaintext_engine_context *pe_ctx, char *buffer)
{
 int i;

 for(i=0; buffer[i] != 0; i++)
 {
  if(unicode_word_engine_consume_byte(pe_ctx->uwc, (unsigned char) buffer[i]))
   return 1;
 }

 /* flush the last word out */
 if(unicode_word_engine_consume_byte(pe_ctx->uwc, (unsigned char) ' '))
  return 1;

 return 0;
}

int pr_flush_line_buffer(struct plaintext_rendering_context *pr_ctx)
{
 int i, space, width, allocation_size;
 char *realloc_ptr;

if(pr_ctx->pe_ctx->footnote_pr != NULL)
fprintf(stderr, "_flush_line_buffer(%llx)\n", pr_ctx);

 if( (pr_ctx->output == NULL) &&
     (pr_ctx->output_buffer == NULL) )
 {
  pr_ctx->n_bytes = 0;
  pr_ctx->n_characters = 0;
  return 0;
 }

 if(pr_ctx->n_bytes == 0)
  return 0;

 width = pr_ctx->line_width -
         pr_ctx->left_margin_width -
         pr_ctx->right_margin_width;

 if(pr_ctx->n_characters > width)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 switch(pr_ctx->justification)
 {
  case PER_LEFT_JUSTIFY:
       space = pr_ctx->left_margin_width;
       break;

  case PER_CENTER_JUSTIFY:
       space = pr_ctx->left_margin_width + (width - pr_ctx->n_characters) / 2;
       break;

  case PER_RIGHT_JUSTIFY:
       space = pr_ctx->left_margin_width + width - pr_ctx->n_characters;
       break;

  default:
       fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
       return 1;
 }

 if(pr_ctx->output != NULL)
 {
  for(i=0; i<space; i++)
  {
   fprintf(pr_ctx->output, " ");
  }
 }

 if(pr_ctx->output != NULL)
 {
  fprintf(pr_ctx->output, "%s", pr_ctx->line_buffer);
 } else {
  if(pr_ctx->output_buffer_length + pr_ctx->n_bytes + 1 >= pr_ctx->output_buffer_size)
  {
   allocation_size = pr_ctx->output_buffer_size + PER_LINE_BUFFER_SIZE;
   if((realloc_ptr = realloc(pr_ctx->output_buffer, allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: pr_flush_line_buffer() realloc(%d) failed\n", allocation_size);
    return 1;
   }
   pr_ctx->output_buffer = realloc_ptr;
   pr_ctx->output_buffer_size = allocation_size;
  }

  memcpy(pr_ctx->output_buffer + pr_ctx->output_buffer_length,
         pr_ctx->line_buffer, pr_ctx->n_bytes);
  pr_ctx->output_buffer_length += pr_ctx->n_bytes;
  pr_ctx->output_buffer[pr_ctx->output_buffer_length] = 0;
fprintf(stderr, "a1: '%s'\n", pr_ctx->output_buffer);
 }

 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;

 return 0;
}

int pr_hard_full_line(struct plaintext_rendering_context *pr_ctx)
{
 int i;

 for(i=0; i<pr_ctx->line_width; i++)
 {
  fprintf(pr_ctx->output, " ");
 }

 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;
 return 0;
}

int pr_page_number(struct plaintext_rendering_context *pr_ctx)
{
 char temp[128];

 if( (pr_ctx->n_characters > 0) && (pr_ctx->current_row != -1) )
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 //this will grow in complexity, page names, etc;
 snprintf(temp, 128, "%d", pr_ctx->current_page + 1);
 //this rendered temp needs to be keept somewhere for use by toc and index structures;
 //remember to change the anchor tags when this happens

 if(pr_ctx->output == NULL)
  return 0;

 if(pr_ctx->show_page_numbers == 0)
 {
  switch(pr_ctx->output_mode)
  {
   case PER_OUTPUT_MODE_TEXT_FILE:
        return 0;
        break;

   case PER_OUTPUT_MODE_HTML_FILE:
        /* the page number is the only full width line that can be certain,
           so even no turned off, we still need a guaranteed full line for
           the widths to match up */
        return pr_hard_full_line(pr_ctx);
        break;
  }
 }

 if(plaintext_rendering_stack_push(pr_ctx->pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pr_ctx->pe_ctx))
  return 1;

 /* set page number margins */
 pr_ctx->pe_ctx->pr_ctx->justification = PER_RIGHT_JUSTIFY;
 pr_ctx->pe_ctx->pr_ctx->left_margin_width = 10; /* could page number be multi line? */
 pr_ctx->pe_ctx->pr_ctx->right_margin_width = 0;

 if(pr_feed_generated_words(pr_ctx->pe_ctx, temp))
  return 1;

 /* note that pr_ctx that we were called with, is not the current
    pe_ctx->pr_ctx, because of the _stack_push() */
 if(pr_flush_line_buffer(pr_ctx->pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pr_ctx->pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pr_ctx->pe_ctx))
  return 1;

 return 0;
}

int pr_advance_page(struct plaintext_rendering_context *pr_ctx)
{
 int i;
 int rows_left;

 if(pr_ctx->current_row == 0)
  return 0;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 /* finish out the current page */
 if(pr_ctx->output != NULL)
 {
  fprintf(pr_ctx->output, "\n");
  for(i=1; i<rows_left; i++)
  {
   fprintf(pr_ctx->output, "\n");
  }
 }

 /* bottom margin */
 if(pr_ctx->output != NULL)
 {
  for(i=0; i<pr_ctx->bottom_margin; i++)
  {
   fprintf(pr_ctx->output, "\n");
  }
 }

 switch(pr_ctx->output_mode)
 {
  case PER_OUTPUT_MODE_HTML_FILE:
       if(pr_ctx->output != NULL)
       {
        fprintf(pr_ctx->output,
                "    </pre>\n"
                "   </td>\n"
                "  </tr>\n"
                " </table>\n");
       }
       break;
 }

 pr_ctx->current_row = -1;
 pr_ctx->current_page++;

 return 0;
}

int pr_start_page(struct plaintext_rendering_context *pr_ctx)
{
 int i;

 if(pr_ctx->current_row != -1)
 {
  fprintf(stderr, "BCA: pr_start_page() page already started\n");
  return 1;
 }

 switch(pr_ctx->output_mode)
 {
  case PER_OUTPUT_MODE_HTML_FILE:
       if(pr_ctx->output != NULL)
       {
        fprintf(pr_ctx->output,
                " <a name=\"%d\"></a>\n"
                " <table border=1 cellpadding=10>\n"
                "  <tr>\n"
                "   <td>\n"
                "    <pre>",
                pr_ctx->current_page + 1);
       }
       break;
 }

 /* top margin */
 if(pr_page_number(pr_ctx))
  return 1;

 if(pr_ctx->output != NULL)
 {
  fprintf(pr_ctx->output, "\n"); //unless not doing page numbers?

  for(i=1; i<pr_ctx->top_margin; i++)
  {
   fprintf(pr_ctx->output, "\n");
  }
 }

 pr_ctx->current_row = 0;
 return 0;
}

int pr_third_way_down(struct plaintext_rendering_context *pr_ctx)
{
 int i, count;

 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 if(pr_flush_line_buffer(pr_ctx))
  return 1;

 if(pr_advance_page(pr_ctx))
  return 1;

 /* this needs to be conditional, since in the case of the page being
    blank to start with, advance_page doesn't do anything */
 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 count = ((pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin) / 3) - 1;

 for(i=0; i<count; i++)
 {
  pr_ctx->current_row++;

  if(pr_ctx->output != NULL)
   fprintf(pr_ctx->output, "\n");
 }

 return 0;
}

int pr_center_row(struct plaintext_rendering_context *pr_ctx)
{
 int i, count;

 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 if(pr_flush_line_buffer(pr_ctx))
  return 1;

 if(pr_advance_page(pr_ctx))
  return 1;

 /* this needs to be conditional, since in the case of the page being
    blank to start with, advance_page doesn't do anything */
 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 count = ((pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin) / 2) - 1;

 for(i=0; i<count; i++)
 {
  pr_ctx->current_row++;

  if(pr_ctx->output != NULL)
   fprintf(pr_ctx->output, "\n");
 }

 return 0;
}

int pr_ensure_minimum_rows_left(struct plaintext_rendering_context *pr_ctx, 
                                int minimum)
{
 int rows_left;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 if(minimum < rows_left)
  return pr_advance_page(pr_ctx);

 return 0;
}

int pr_advance_line(struct plaintext_rendering_context *pr_ctx)
{
 int rows_left;

 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 if(pr_flush_line_buffer(pr_ctx))
  return 1;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 if(rows_left < 1)
  return pr_advance_page(pr_ctx);

 pr_ctx->current_row++;

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "\n");

 return 0;
}

int pr_advance_word(struct plaintext_rendering_context *pr_ctx,
                    struct unicode_word_context *uwc)
{
 int space_this_line, rows_left, characters_left;

 /* enough bytes left in the line buffer? */
 if(pr_ctx->n_bytes + uwc->buffer_length + 1 > PER_LINE_BUFFER_SIZE)
 {
  fprintf(stderr, "BCA: plaintext engine -  pr_advance_word(): "
          "line buffer would overrun\n");
  return 1;
 }

 /* how many characters will fit on this line? */
 space_this_line = pr_ctx->line_width -
                   pr_ctx->left_margin_width -
                   pr_ctx->right_margin_width;

 /* need to line wrap? */
 if(pr_ctx->n_characters + uwc->n_characters + 1 > space_this_line)
 {

  if(pr_ctx->current_row == -1)
  {
   rows_left = 2;
  } else {
   rows_left = pr_ctx->page_length
             - pr_ctx->top_margin
             - pr_ctx->bottom_margin
             - pr_ctx->current_row;
  }

  characters_left = space_this_line - pr_ctx->n_characters - 1;

  if( (pr_ctx->pe_ctx->hc != NULL) &&
      (rows_left > 1) &&
      (characters_left > 2) )
  {

   switch(hyphenation_engine_attempt(pr_ctx->pe_ctx->hc,
                                     characters_left,
                                     uwc,
                                     &(pr_ctx->pe_ctx->first),
                                     &(pr_ctx->pe_ctx->second)))
   {
    case -1:
         fprintf(stderr, "BCA: pr_advace_word(): hyphentation_engine_attempt() failed\n");
         return 1;
         break;

    case 0:
/*
         fprintf(stderr, "n %d '%s'\n",
                 characters_left,
                 uwc->word_buffer);
*/
         if(pr_advance_line(pr_ctx))
          return 1;
         break;

    case 1:
/*
         fprintf(stderr, "y %d '%s' -> '%s' & '%s'\n",
                 characters_left,
                 uwc->word_buffer,
                 pr_ctx->pe_ctx->first.word_buffer,
                 pr_ctx->pe_ctx->second.word_buffer);
*/
         /* the word in uwc has been split into a two hyphenated parsts for line wrap,
            first and second. Now recursively call into advance_word with the first
            part of the line (that should now fit) */
         if(pr_advance_word(pr_ctx, &(pr_ctx->pe_ctx->first)))
          return 1;

         if(pr_advance_line(pr_ctx))
          return 1;

         /* now contine on with next line with the word portion in 'second' */
         memcpy(pr_ctx->line_buffer + pr_ctx->n_bytes,
                pr_ctx->pe_ctx->second.word_buffer,
                pr_ctx->pe_ctx->second.buffer_length + 1);

         pr_ctx->n_characters += pr_ctx->pe_ctx->second.n_characters;
         pr_ctx->n_bytes += pr_ctx->pe_ctx->second.buffer_length;
         pr_ctx->line_buffer[pr_ctx->n_bytes] = 0;
         return 0;
         break;
   }

  } else {
   if(pr_advance_line(pr_ctx))
    return 1;
  }
 }

 /* unless this is the first word on the line, it will need a space in between words */
 if(pr_ctx->n_bytes > 0)
 {
  pr_ctx->line_buffer[pr_ctx->n_bytes++] = ' ';
  pr_ctx->n_characters++;
 }

 /* add this word to the line buffer */
 memcpy(pr_ctx->line_buffer + pr_ctx->n_bytes,
        uwc->word_buffer, uwc->buffer_length + 1);

 pr_ctx->n_characters += uwc->n_characters;
 pr_ctx->n_bytes += uwc->buffer_length;

 pr_ctx->line_buffer[pr_ctx->n_bytes] = 0;

 return 0;
}

int plaintext_add_toc_element(struct plaintext_engine_context *pe_ctx,
                              int type, char *name)
{
 struct toc_element *e, *i;

 if((e = new_toc_element(type, name)) == NULL)
  return 1;

 snprintf(e->page, 16, "%d", pe_ctx->pr_ctx->current_page + 1);

 if(pe_ctx->toc_root == NULL)
 {
  pe_ctx->toc_cursor = pe_ctx->toc_root = e;
  e->count = 1;
 } else {

  /* walk up the parantage until we find a sibling type, a paraent type, or tree root */
  i = pe_ctx->toc_cursor;
  while(i != NULL)
  {
   if(i->type < e->type)
   {
    e->parrent = i;
    e->count = 1;
    i->child = e;
    pe_ctx->toc_cursor = e;
    return 0;
   } else if(i->type == e->type) {
    e->last = pe_ctx->toc_cursor;
    e->count = pe_ctx->toc_cursor->count + 1;
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

int pe_print_toc(struct plaintext_engine_context *pe_ctx)
{
 int type, length;
 char temp[256], *title;

 if((pe_ctx->toc_cursor = pe_ctx->toc_root) == NULL)
 return 0;

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;

 if(pr_feed_generated_words(pe_ctx, "Table of Contents"))
  return 1;

 pr_advance_line(pe_ctx->pr_ctx);

 pe_ctx->pr_ctx->show_page_numbers = 0;
 pe_ctx->pr_ctx->justification = PER_LEFT_JUSTIFY;
 pe_ctx->pr_ctx->right_margin_width = 5;

 pr_advance_line(pe_ctx->pr_ctx);

 while(pe_ctx->toc_cursor != NULL)
 {
  type = pe_ctx->toc_cursor->type;
  title = pe_ctx->toc_cursor->name;

  pe_ctx->pr_ctx->left_margin_width = 1 + type;

  switch(type)
  {
   case DLEVEL_PART:
        length = snprintf(temp, 256, "Part %d", pe_ctx->toc_cursor->count);
        break;

   case DLEVEL_CHAPTER:
        length = snprintf(temp, 256, "Chapter %d", pe_ctx->toc_cursor->count);
        break;

   case DLEVEL_SECTION:
//dectect if using chapters and then do %d.%d
        length = snprintf(temp, 256, "Section %d", pe_ctx->toc_cursor->count);
        break;

   case DLEVEL_SUB:
        length = snprintf(temp, 256, "Subsection %d", pe_ctx->toc_cursor->count);
        break;

   case DLEVEL_INSET:
        length = snprintf(temp, 256, "Inset %d", pe_ctx->toc_cursor->count);
        break;

   default:
        fprintf(stderr, "BCA: plaintext engine: unhandled toc type, %d\n", type);
  }

  if(title != NULL)
   length += snprintf(temp + length, 256, ": %s", title);

  length +=
   snprintf(temp + length, 256 - length,
            ", page %s", pe_ctx->toc_cursor->page);

  if(pr_feed_generated_words(pe_ctx, temp))
   return 1;

  pr_advance_line(pe_ctx->pr_ctx);

  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 if(pr_advance_page(pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 return 0;
}

int plaintext_paragraph_open(struct plaintext_engine_context *pe_ctx)
{
 int i;

 if(pe_ctx->paragraph_indent > pe_ctx->pr_ctx->line_width)
 {
  fprintf(stderr, "BCA: plain text paragraph indent greater than line width!\n");
  return 1;
 }

 if(pe_ctx->pr_ctx->n_bytes > 0)
 {
  fprintf(stderr,
          "BCA: plain text engine: paragraph should start on a blank line "
         "not \"%s\"\n", pe_ctx->pr_ctx->line_buffer);
  return 1;
 }

 for(i=0; i<pe_ctx->paragraph_indent; i++)
 {
  pe_ctx->pr_ctx->line_buffer[pe_ctx->pr_ctx->n_bytes++] = ' ';
  pe_ctx->pr_ctx->n_characters++;
 }
 pe_ctx->pr_ctx->line_buffer[pe_ctx->pr_ctx->n_bytes] = 0;

 return 0;
}

int plaintext_paragraph_close(struct plaintext_engine_context *pe_ctx)
{
 int i;

 /* finish the last line if any */
 if(pe_ctx->pr_ctx->n_bytes > 0)
  if(pr_advance_line(pe_ctx->pr_ctx))
   return 1;

 for(i=0; i<pe_ctx->paragraph_line_spacing; i++)
 {
  if(pr_advance_line(pe_ctx->pr_ctx))
   return 1;
 }

 return 0;
}

int superscript_number(char *number, int length, char **superscript_ptr, int *out_length)
{
 int i, allocation_size, j = 0;
 char *superscript;

 if(length == -1)
  length = strlen(number);

 if(*superscript_ptr != NULL)
 {
  superscript = *superscript_ptr;
  allocation_size = *out_length;
 } else {
  allocation_size = (length * 4) + 1;
  if((superscript = (char *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: superscript_number() malloc(%d) failed: %s\n",
           allocation_size, strerror(errno));
   return 1;
  }
 }

 for(i=0; i<length; i++)
 {
  switch(number[i])
  {
   case '0':
        /* not the same as the degree symbol '°' */
        j += snprintf(superscript + j, allocation_size - j, "⁰");
        break;

   case '1':
        j += snprintf(superscript + j, allocation_size - j, "¹");
        break;

   case '2':
        j += snprintf(superscript + j, allocation_size - j, "²");
        break;

   case '3':
        j += snprintf(superscript + j, allocation_size - j, "³");
        break;

   case '4':
        j += snprintf(superscript + j, allocation_size - j, "⁴");
        break;

   case '5':
        j += snprintf(superscript + j, allocation_size - j, "⁵");
        break;

   case '6':
        j += snprintf(superscript + j, allocation_size - j, "⁶");
        break;

   case '7':
        j += snprintf(superscript + j, allocation_size - j, "⁷");
        break;

   case '8':
        j += snprintf(superscript + j, allocation_size - j, "⁸");
        break;

   case '9':
        j += snprintf(superscript + j, allocation_size - j, "⁹");
        break;

   default:
        j += snprintf(superscript + j, allocation_size - j, "%c", number[i]);
  }
 }

 *superscript_ptr = superscript;
 *out_length = j;
 return 0;
}

int plaintext_footnote_open(struct plaintext_engine_context *pe_ctx)
{
 struct plaintext_footnote *f;
 struct plaintext_rendering_context *pr_ctx;
 int allocation_size, length;
 char number[16], *superscript;

 if(pe_ctx->footnote_pr != NULL)
 {
  fprintf(stderr,
          "BCA: plaintext_footnote_open() foot note starting inside another foot note\n");
  return 1;
 }

 allocation_size = sizeof(struct plaintext_footnote);
 if((f = (struct plaintext_footnote *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_open() malloc(%d) failed.\n",
          allocation_size);
  return 1;
 }
 memset(f, 0, allocation_size);

 f->number = ++pe_ctx->n_footnotes;

 if(pe_ctx->footnotes_head == NULL)
  pe_ctx->footnotes_head = f;

 if(pe_ctx->footnotes_tail == NULL)
 {
  pe_ctx->footnotes_tail = f;
 } else {
  pe_ctx->footnotes_tail->next = f;
  pe_ctx->footnotes_tail = f;
 }

 length = snprintf(number, 16, "%d", f->number);
 superscript = NULL;
 if(superscript_number(number, length, &superscript, &length))
  return 1;

fprintf(stderr, "-- '%s'\n", superscript);
//free() takes place when used by word engine

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pr_ctx = pe_ctx->pr_ctx;

 pe_ctx->footnote_pr = pr_ctx;
 fprintf(stderr, "= %llx\n", pe_ctx->pr_ctx);


 pr_ctx->justification = PER_LEFT_JUSTIFY;
 pr_ctx->left_margin_width += 2;
 pr_ctx->right_margin_width += 2;

 /* this rendering context will write to ->output_buffer */
 pr_ctx->output = NULL;
 allocation_size = PER_LINE_BUFFER_SIZE;
 if((pr_ctx->output_buffer = (char *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_open() malloc(%d) failed.\n",
          allocation_size);
  free(f);
  return 1;
 }
 pr_ctx->output_buffer[0] = 0;
 pr_ctx->output_buffer_size = allocation_size;

 return 0;
}

int plaintext_footnote_close(struct plaintext_engine_context *pe_ctx)
{
 struct plaintext_footnote *f;
 int allocation_size;

fprintf(stderr, "_footnote_close()\n");

 if((f = pe_ctx->footnotes_tail) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_close() should not be here\n");
  return 1;
 }

 if(pr_flush_line_buffer(pe_ctx->pr_ctx))
  return 1;

fprintf(stderr, "0b0 : '%s'\n", pe_ctx->pr_ctx->output_buffer);

 f->n_lines = pe_ctx->pr_ctx->output_buffer_lines;
 allocation_size = pe_ctx->pr_ctx->output_buffer_length + 1;
 if((f->buffer = (char *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_close() malloc(%d)\n", allocation_size);
  return 1;
 }
 memcpy(f->buffer, pe_ctx->pr_ctx->output_buffer, allocation_size);

fprintf(stderr, "0b1 pe_ctx->pr_ctx = %llx\n", pe_ctx->pr_ctx);

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

fprintf(stderr, "0b2 pe_ctx->pr_ctx = %llx\n", pe_ctx->pr_ctx);

 pe_ctx->footnote_pr = NULL;

fprintf(stderr, "got here: '%s'\n", f->buffer);
 return 0;
}

struct plaintext_rendering_context *
plaintext_rendering_context_new(struct plaintext_engine_context *pe_ctx,
                                FILE *output)
{
 struct plaintext_rendering_context *pr_ctx;
 int allocation_size;

 allocation_size = (sizeof(struct plaintext_rendering_context));
 if((pr_ctx = (struct plaintext_rendering_context *)
              malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_rendering_context_new() malloc(%d) failed.\n",
          allocation_size);
  return NULL;
 }

 pr_ctx->pe_ctx = pe_ctx;
 pr_ctx->output = output;
 pr_ctx->current_row = -1;
 pr_ctx->current_page = 0;
 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;
 pr_ctx->line_buffer[0] = 0;

 pr_ctx->line_width = 70;
 pr_ctx->page_length = 20;
 pr_ctx->left_margin_width = 0;
 pr_ctx->right_margin_width = 0;
 pr_ctx->top_margin = 1;
 pr_ctx->bottom_margin = 1;
 pr_ctx->justification = PER_LEFT_JUSTIFY;
 pr_ctx->direction = PER_LEFT_TO_RIGHT;
 pr_ctx->show_page_numbers = 1;

 pr_ctx->output_mode = PER_OUTPUT_MODE_TEXT_FILE;
// pr_ctx->output_mode = PER_OUTPUT_MODE_HTML_FILE;

 pr_ctx->output_buffer = NULL;
 pr_ctx->output_buffer_length = 0;
 pr_ctx->output_buffer_size = 0;
 return pr_ctx;
}

struct plaintext_rendering_context *
plaintext_rendering_context_copy(struct plaintext_rendering_context *source)
{
 struct plaintext_rendering_context *pr_ctx;
 int allocation_size;

 allocation_size = (sizeof(struct plaintext_rendering_context));
 if((pr_ctx = (struct plaintext_rendering_context *)
              malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_rendering_context_new() malloc(%d) failed.\n",
          allocation_size);
  return NULL;
 }

 pr_ctx->pe_ctx = source->pe_ctx;
 pr_ctx->output = source->output;
 pr_ctx->output_mode = source->output_mode;
 pr_ctx->current_row = source->current_row;
 pr_ctx->current_page = source->current_page;
 pr_ctx->show_page_numbers = source->show_page_numbers;
 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;
 pr_ctx->line_buffer[0] = 0;

 pr_ctx->line_width = source->line_width;
 pr_ctx->page_length = source->page_length;
 pr_ctx->left_margin_width = source->left_margin_width;
 pr_ctx->right_margin_width = source->right_margin_width;
 pr_ctx->top_margin = source->top_margin;
 pr_ctx->bottom_margin = source->bottom_margin;
 pr_ctx->justification = source->justification;
 pr_ctx->direction = source->direction;

 /* buffer output is explicitly setup, so don't propagate */
 pr_ctx->output_buffer = NULL;
 pr_ctx->output_buffer_length = 0;
 pr_ctx->output_buffer_size = 0;

 return pr_ctx;
}

int plaintext_rendering_context_finalize(struct plaintext_rendering_context *pr_ctx)
{
 if(pr_ctx)
 {
  if(pr_ctx->output_buffer != NULL)
   free(pr_ctx->output_buffer);

  free(pr_ctx);
 }

 return 0;
}

