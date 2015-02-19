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

 if( (pr_ctx->output == NULL) &&
     (pr_ctx->output_buffer == NULL) )
 {
  /* do nothing fast path */
  pr_ctx->n_bytes = 0;
  pr_ctx->n_characters = 0;
  return 0;
 }

 if(pr_ctx->n_bytes == 0)
  return 0;

 if(pr_ctx->current_col > pr_ctx->left_margin_width)
  width = pr_ctx->line_width -
          pr_ctx->current_col -
          pr_ctx->right_margin_width;
 else
  width = pr_ctx->line_width -
          pr_ctx->left_margin_width -
          pr_ctx->right_margin_width;

 if(pr_ctx->n_characters > width)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  fprintf(stderr, "BCA: width = %d, n_characters = %d, n_bytes = %d, current_col = %d, "
          "line buffer = \"%s\", left_margin = %d, right_margin = %d\n",
          width, pr_ctx->n_characters, pr_ctx->n_bytes, pr_ctx->current_col, pr_ctx->line_buffer,
          pr_ctx->left_margin_width, pr_ctx->right_margin_width);
  return 1;
 }

 switch(pr_ctx->justification)
 {
  case PER_LEFT_JUSTIFY:
       if(pr_ctx->current_col > pr_ctx->left_margin_width)
        space = 0;
       else if(pr_ctx->current_col > 0)
        space = pr_ctx->left_margin_width - pr_ctx->current_col;
       else
        space = pr_ctx->left_margin_width;
       break;

  case PER_CENTER_JUSTIFY:
       space = pr_ctx->left_margin_width + (width - pr_ctx->n_characters) / 2;
       break;

  case PER_RIGHT_JUSTIFY:
       if(pr_ctx->current_col > pr_ctx->left_margin_width)
        space = pr_ctx->left_margin_width + width - pr_ctx->n_characters;
       else if(pr_ctx->current_col > 0)
        space = pr_ctx->left_margin_width +
                width -
                pr_ctx->n_characters -
                pr_ctx->current_col;
       else
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
  if(pr_ctx->output_buffer_length + pr_ctx->n_bytes + 2 >= pr_ctx->output_buffer_size)
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

  pr_ctx->output_buffer[pr_ctx->output_buffer_length++] = '\n';
  memcpy(pr_ctx->output_buffer + pr_ctx->output_buffer_length,
         pr_ctx->line_buffer, pr_ctx->n_bytes);
  pr_ctx->output_buffer_length += pr_ctx->n_bytes;
  pr_ctx->output_buffer[pr_ctx->output_buffer_length] = 0;
  pr_ctx->output_buffer_lines++;
 }

 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;
 return 0;
}

int pr_enable_attribute(struct plaintext_rendering_context *pr_ctx, char *attribute)
{
 int length;
 char temp[512];

 if(pr_ctx->output != NULL)
 {
  switch(pr_ctx->output_mode)
  {
   case OUTPUT_MODE_TEXT_FILE:
        break;

   case OUTPUT_MODE_HTML_FILE:
        length = snprintf(temp, 512, "<span class=\"%s\">", attribute);
        pr_direct_to_line_buffer(pr_ctx, temp, length, 0);
        break;

   case OUTPUT_MODE_TTY:
        length = 0;
        if(strcmp(attribute, "sourcelistingnumbers") == 0)
        {
         length = snprintf(temp, 512, "%c[100;97m", 0x1B);
        } else if(strcmp(attribute, "bca_delimiter") == 0) {
         length = snprintf(temp, 512, "%c[46;37m", 0x1B);
        } else if(strcmp(attribute, "bca_escape") == 0) {
         length = snprintf(temp, 512, "%c[46;96m", 0x1B);
        } else if(strcmp(attribute, "bca_principle") == 0) {
         length = snprintf(temp, 512, "%c[46;35m", 0x1B);
        } else if(strcmp(attribute, "bca_component") == 0) {
         length = snprintf(temp, 512, "%c[46;92m", 0x1B);
        } else if(strcmp(attribute, "bca_key") == 0) {
         length = snprintf(temp, 512, "%c[46;93m", 0x1B);
        } else if(strcmp(attribute, "bca_value") == 0) {
         length = snprintf(temp, 512, "%c[46;34m", 0x1B);
        } else if(strcmp(attribute, "sourcelistingbackground") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);
        } else if(strcmp(attribute, "c_string") == 0) {
         length = snprintf(temp, 512, "%c[44;37m", 0x1B);
        } else if(strcmp(attribute, "c_escapesequence") == 0) {
         length = snprintf(temp, 512, "%c[44;30m", 0x1B);

        } else if(strcmp(attribute, "c_formatmodifier") == 0) {
         length = snprintf(temp, 512, "%c[44;30m", 0x1B);

        } else if(strcmp(attribute, "c_function") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "c_ppd") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "c_declarator") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "c_argument") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "c_keyword") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "c_operator") == 0) {
         length = snprintf(temp, 512, "%c[46;31m", 0x1B);

        } else if(strcmp(attribute, "comment") == 0) {
         length = snprintf(temp, 512, "%c[46;31m", 0x1B);

        } else if(strcmp(attribute, "keyword") == 0) {
         length = snprintf(temp, 512, "%c[46;30m", 0x1B);

        } else if(strcmp(attribute, "normal") == 0) {
         length = snprintf(temp, 512, "%c[46;31m", 0x1B);

        } else if(strcmp(attribute, "number") == 0) {
         length = snprintf(temp, 512, "%c[46;32m", 0x1B);

        } else if(strcmp(attribute, "symbol") == 0) {
         length = snprintf(temp, 512, "%c[46;33m", 0x1B);

        } else if(strcmp(attribute, "string") == 0) {
         length = snprintf(temp, 512, "%c[46;34m", 0x1B);

        } else if(strcmp(attribute, "specialchar") == 0) {
         length = snprintf(temp, 512, "%c[46;35m", 0x1B);


        } else if(strcmp(attribute, "part_title") == 0) {
        } else if(strcmp(attribute, "chapter_title") == 0) {
        } else if(strcmp(attribute, "section_title") == 0) {
        } else if(strcmp(attribute, "subsection_title") == 0) {
        } else if(strcmp(attribute, "listing_caption") == 0) {

        } else if(strncmp(attribute, "terminal-", 9) == 0) {
         if(strlen(attribute) < 15)
          return 1;

         length = 0;
         length += snprintf(temp + length, 512 - length, "%c[", 0x1B);

         switch(attribute[9]) //background
         {
          case 'N': //normal
          case 'K': //black
               length += snprintf(temp + length, 512 - length, "40");
               break;

          case 'R': //red
               length += snprintf(temp + length, 512 - length, "41");
               break;

          case 'G': //green
               length += snprintf(temp + length, 512 - length, "42");
               break;

          case 'Y': //yellow
               length += snprintf(temp + length, 512 - length, "43");
               break;

          case 'B': //blue
               length += snprintf(temp + length, 512 - length, "44");
               break;

          case 'M': //magenta
               length += snprintf(temp + length, 512 - length, "45");
               break;

          case 'C': //cyan
               length += snprintf(temp + length, 512 - length, "46");
               break;

          case 'W': //white
               length += snprintf(temp + length, 512 - length, "47");
               break;

         }

         length += snprintf(temp + length, 512 - length, ";");

         switch(attribute[10]) //foreground
         {
          case 'K': //black
               length += snprintf(temp + length, 512 - length, "30");
               break;

          case 'R': //red
               length += snprintf(temp + length, 512 - length, "31");
               break;

          case 'N': //normal
          case 'G': //green
               length += snprintf(temp + length, 512 - length, "32");
               break;

          case 'Y': //yellow
               length += snprintf(temp + length, 512 - length, "33");
               break;

          case 'B': //blue
               length += snprintf(temp + length, 512 - length, "34");
               break;

          case 'M': //magenta
               length += snprintf(temp + length, 512 - length, "35");
               break;

          case 'C': //cyan
               length += snprintf(temp + length, 512 - length, "36");
               break;

          case 'W': //white
               length += snprintf(temp + length, 512 - length, "37");
               break;

         }

         length += snprintf(temp + length, 512 - length, "m");

        } else {
         fprintf(stderr, "BCA: warning, tty unhandled attribute %s\n", attribute);
         temp[0] = length = 0;
        }
        pr_direct_to_line_buffer(pr_ctx, temp, length, 0);
        break;

  }
 }

 return 0;
}

int pr_disable_attribute(struct plaintext_rendering_context *pr_ctx)
{
 char temp[8];
 int length = 0;

 if(pr_ctx->output != NULL)
 {
  switch(pr_ctx->output_mode)
  {
   case OUTPUT_MODE_TEXT_FILE:
        break;

   case OUTPUT_MODE_HTML_FILE:
        return pr_direct_to_line_buffer(pr_ctx, "</span>", -1, 0);
        break;

   case OUTPUT_MODE_TTY:
        snprintf(temp, 8, "%c[0m", 0x1B);
        pr_direct_to_line_buffer(pr_ctx, temp, 4, 0);
        break;
  }
 }

 return 0;
}

void pr_hard_preformat_width(struct plaintext_rendering_context *pr_ctx, int spaces)
{
 int i;

 for(i=0; i < spaces; i++)
 {
  fprintf(pr_ctx->output, " ");
 }
}

int pr_page_number(struct plaintext_rendering_context *pr_ctx)
{
 char temp[128];
 int extra_preformated_width;

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

 if(plaintext_rendering_stack_push(pr_ctx->pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pr_ctx->pe_ctx))
  return 1;

 /* set page number margins */
 if( (pr_ctx->output_mode == OUTPUT_MODE_HTML_FILE) &&
     (pr_ctx->pe_ctx->even_or_odd_page == 1) )
  pr_ctx->pe_ctx->pr_ctx->justification = PER_LEFT_JUSTIFY;
 else
  pr_ctx->pe_ctx->pr_ctx->justification = PER_RIGHT_JUSTIFY;

 pr_ctx->pe_ctx->pr_ctx->left_margin_width = 0;
 pr_ctx->pe_ctx->pr_ctx->right_margin_width = 0;

 extra_preformated_width = pr_ctx->pe_ctx->pr_ctx->line_width;

 if(pr_ctx->show_page_numbers)
  if(pr_feed_generated_words(pr_ctx->pe_ctx, temp))
   return 1;

 extra_preformated_width -= pr_ctx->pe_ctx->pr_ctx->n_characters;

 /* note that pr_ctx that we were called with, is not the current
    pe_ctx->pr_ctx, because of the _stack_push() */
 if(pr_flush_line_buffer(pr_ctx->pe_ctx->pr_ctx))
  return 1;

 /* html needs at least one line preformatted width */
 if(pr_ctx->output_mode == OUTPUT_MODE_HTML_FILE)
 {
  if(pr_ctx->pe_ctx->even_or_odd_page == 1)
  {
   pr_hard_preformat_width(pr_ctx->pe_ctx->pr_ctx, extra_preformated_width);
  } else {
   if(pr_ctx->show_page_numbers == 0)
    pr_hard_preformat_width(pr_ctx->pe_ctx->pr_ctx, extra_preformated_width);
  }
 }

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

 if(pr_ctx->current_row < 1)
  return 0;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 /* finish out the current page */
 if(pr_ctx->output != NULL)
 {
  for(i=0; i<rows_left; i++)
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
  case OUTPUT_MODE_HTML_FILE:
       if(pr_ctx->output != NULL)
       {
        fprintf(pr_ctx->output,
                "    </pre>\n"
                "   </td>\n");

        if(pr_ctx->pe_ctx->even_or_odd_page == 0)
         fprintf(pr_ctx->output,
                 "  </tr>\n"
                 " </table>\n"
                 " <br>\n");
       }
       break;
 }

 pr_ctx->current_row = -1;
 pr_ctx->current_page++;

 if(pr_ctx->pe_ctx->even_or_odd_page)
 {
  pr_ctx->pe_ctx->even_or_odd_page = 0;
 } else {
  pr_ctx->pe_ctx->even_or_odd_page = 1;
 }

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
  case OUTPUT_MODE_HTML_FILE:
       if(pr_ctx->output != NULL)
       {
        if(pr_ctx->pe_ctx->even_or_odd_page == 1)
         fprintf(pr_ctx->output,
                 " <a name=\"%d\"></a>\n"
                 " <table border=1 cellpadding=10 cellspacing=0 align=center>\n"
                 "  <tr>\n",
                pr_ctx->current_page + 1);

        if(pr_ctx->pe_ctx->even_or_odd_page == 1)
         fprintf(pr_ctx->output,
                 "   <td background=\"bgl.jpg\">\n");
        else
         fprintf(pr_ctx->output,
                 "   <td background=\"bgr.jpg\">\n");

        fprintf(pr_ctx->output,
                "    <pre>");
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

int pr_render_foot_notes(struct plaintext_engine_context *pe_ctx, int limit)
{
 struct plaintext_footnote *f;
 int count, length;
 char number[16], *superscript;

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_RIGHT_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 1;
 pe_ctx->pr_ctx->right_margin_width =
  pe_ctx->pr_ctx->line_width - 4;

 pe_ctx->footnote_pr = pe_ctx->pr_ctx;

 count = 0;
 while(1)
 {
  if( (limit != 0) && (count == limit) )
   break;

  if((f = pe_ctx->footnotes_head) == NULL)
  {
   if(limit != 0)
   {
    fprintf(stderr, "BCA: pr_render_foot_notes() I should not be here\n");
    return 1;
   }
   break;
  }

  length = snprintf(number, 16, "%d", f->number);
  superscript = NULL;
  if(superscript_number(number, length, &superscript, &length))
   return 1;

  if(pr_feed_generated_words(pe_ctx, superscript))
   return 1;

  if(pr_advance_buffer(pe_ctx->pr_ctx))
   return 1;

  free(superscript);

  if(plaintext_rendering_stack_push(pe_ctx))
   return 1;

  if(plaintext_word_engine_stack_push(pe_ctx))
   return 1;

  pe_ctx->pr_ctx->justification = PER_LEFT_JUSTIFY;
  pe_ctx->pr_ctx->left_margin_width = 5;
  pe_ctx->pr_ctx->right_margin_width = 5;

  if(pr_feed_generated_words(pe_ctx, f->buffer))
   return 1;

  if(pr_advance_line(pe_ctx->pr_ctx))
   return 1;

  if(plaintext_word_engine_stack_pop(pe_ctx))
   return 1;

  if(plaintext_rendering_stack_pop(pe_ctx))
   return 1;

  pe_ctx->footnotes_head = f->next;

  if(pe_ctx->footnotes_tail == f)
   pe_ctx->footnotes_tail = NULL;

  free(f->buffer);
  free(f);
  count++;
 }

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 pe_ctx->footnote_pr = NULL;
 return 0;
}

int pr_advance_line(struct plaintext_rendering_context *pr_ctx)
{
 int rows_left, rows_consumable_by_foot_notes, n_notes;
 struct plaintext_footnote *f;

 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 if(pr_flush_line_buffer(pr_ctx))
  return 1;

 if(pr_ctx->output != NULL)
  fprintf(pr_ctx->output, "\n");

 pr_ctx->current_row++;
 pr_ctx->current_col = 0;

 rows_left = pr_ctx->page_length
           - pr_ctx->top_margin
           - pr_ctx->bottom_margin
           - pr_ctx->current_row;

 if(rows_left < 1)
  return pr_advance_page(pr_ctx);

 if(pr_ctx->pe_ctx->footnote_pr == NULL)
 {
  n_notes = 0;
  rows_consumable_by_foot_notes = 0;
  for(f = pr_ctx->pe_ctx->footnotes_head; f != NULL; f = f->next)
  {
   n_notes++;
   rows_consumable_by_foot_notes += f->n_lines;

   if(rows_left == rows_consumable_by_foot_notes)
   {
    if(pr_render_foot_notes(pr_ctx->pe_ctx, n_notes))
     return 1;
   }
  }
 }

 return 0;
}

/* Normally, either the line is explicitly advanced with advance_line()
   or advance_line() is called as a side effect of advance_word(). In
   either case, the buffer is written as a whole line. (This is part
   of handling left/right/center justification.) However, in some cases
   we want to advance what is in the line buffer with out moving to a
   new line. Generally this would be in cases where we need to output
   part of a line before going deeper into the rendering context stack.

this is the wrong aproach this this. this is only even being used for
the numbers before the foot notes.

*/
int pr_advance_buffer(struct plaintext_rendering_context *pr_ctx)
{
 int n_characters;

 if(pr_ctx->current_row == -1)
  if(pr_start_page(pr_ctx))
   return 1;

 n_characters = pr_ctx->n_characters;

 if(pr_flush_line_buffer(pr_ctx))
  return 1;

 if(pr_ctx->justification == PER_RIGHT_JUSTIFY)
  pr_ctx->current_col = pr_ctx->line_width - pr_ctx->right_margin_width;
 else
  pr_ctx->current_col += n_characters;
 return 0;
}

int pr_advance_word(struct plaintext_rendering_context *pr_ctx,
                    struct unicode_word_context *uwc)
{
 int space_this_line, rows_left, characters_left, effective_left;

 /* enough bytes left in the line buffer? */
 if(pr_ctx->n_bytes + uwc->buffer_length + 1 > PER_LINE_BUFFER_SIZE)
 {
  fprintf(stderr, "BCA: plaintext engine - pr_advance_word(): "
          "line buffer would overrun\n");
  return 1;
 }

 /* how many characters will fit on this line? */
 if(pr_ctx->current_col > pr_ctx->left_margin_width)
  effective_left = pr_ctx->current_col;
 else
  effective_left = pr_ctx->left_margin_width;

 space_this_line = pr_ctx->line_width -
                   effective_left -
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
         /* the word in uwc has been split into a two hyphenated parts for line wrap,
            first and second. Now recursively call into advance_word with the first
            part of the line (that should now fit) */
         if(pr_advance_word(pr_ctx, &(pr_ctx->pe_ctx->first)))
          return 1;

         if(pr_advance_line(pr_ctx))
          return 1;

         /* now continue on with next line with the word portion in 'second' */
         return pr_send_to_line_buffer(pr_ctx,
                                       pr_ctx->pe_ctx->second.word_buffer,
                                       pr_ctx->pe_ctx->second.buffer_length,
                                       pr_ctx->pe_ctx->second.n_characters);
         break;
   }

  } else {
   if(pr_advance_line(pr_ctx))
    return 1;
  }
 }

 /* unless this is the first word on the line, it will need a space in between words */
 if(pr_ctx->n_characters > 0)
  if(pr_send_to_line_buffer(pr_ctx, " ", 1, 1))
   return 1;

 /* add this word to the line buffer */
 return pr_send_to_line_buffer(pr_ctx,
                               uwc->word_buffer,
                               uwc->buffer_length,
                               uwc->n_characters);
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

 pe_ctx->pr_ctx->show_page_numbers = 0;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;

 if(pr_enable_attribute(pe_ctx->pr_ctx, "toc_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, "Table of Contents"))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 pr_advance_line(pe_ctx->pr_ctx);
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
if(pe_ctx->toc_cursor->parrent != NULL)
fprintf(stderr, "---%d\n", pe_ctx->toc_cursor->parrent->type);

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

  if(pr_send_to_line_buffer(pe_ctx->pr_ctx, temp, -1, -1))
   return 1;

  length =
   snprintf(temp + length, 256 - length,
            ", page %s", pe_ctx->toc_cursor->page);

  if(pr_feed_generated_words(pe_ctx, temp))
   return 1;

  pr_advance_line(pe_ctx->pr_ctx);

  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(pr_advance_page(pe_ctx->pr_ctx))
  return 1;

 return 0;
}

int pe_print_index(struct plaintext_engine_context *pe_ctx)
{
 char temp[MAX_INDEX_TERM_SIZE + 16],
      current_char[6], first_char[6];
 struct plaintext_index_entry *t, *p;

 if(pe_ctx->index == NULL)
  return 0;

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->show_page_numbers = 0;

 if(pr_advance_page(pe_ctx->pr_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;

 if(pr_enable_attribute(pe_ctx->pr_ctx, "index_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, "Index"))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 pr_advance_line(pe_ctx->pr_ctx);

 pe_ctx->pr_ctx->justification = PER_LEFT_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 1;

 pr_advance_line(pe_ctx->pr_ctx);
 current_char[0] = 0;

 for(t = pe_ctx->index; t != NULL; t = t->next_term)
 {
  //todo utf8
  first_char[0] = t->term[0];
  first_char[1] = 0;

  if(strcmp(current_char, first_char) != 0)
  {
   pr_advance_line(pe_ctx->pr_ctx);
   snprintf(current_char, 6, "%s", first_char);
   pe_ctx->pr_ctx->left_margin_width = 5;

   if(pr_enable_attribute(pe_ctx->pr_ctx, "index_character"))
    return 1;

   if(pr_feed_generated_words(pe_ctx, current_char))
    return 1;

   if(pr_disable_attribute(pe_ctx->pr_ctx))
    return 1;

   pr_advance_line(pe_ctx->pr_ctx);

   pe_ctx->pr_ctx->left_margin_width = 1;
  }

  if(pr_enable_attribute(pe_ctx->pr_ctx, "index_term"))
   return 1;

  if(pr_feed_generated_words(pe_ctx, t->term))
   return 1;

  if(pr_disable_attribute(pe_ctx->pr_ctx))
   return 1;

  p = t;
  while(p != NULL)
  {
   /* this strange because, the comma must be printed with page names that are followed by others,
      not before pluarality entries */
   if(p->next_page == NULL)
   {
    if(pr_feed_generated_words(pe_ctx, p->page))
     return 1;

    break;
   } else {
    if(strcmp(p->page, p->next_page->page) != 0)
    {
     snprintf(temp, MAX_INDEX_TERM_SIZE + 16, "%s,", p->page);
     if(pr_feed_generated_words(pe_ctx, temp))
      return 1;

     p = p->next_page;
     continue;
    } else {
     /* page listing de-dup */
     if(pr_feed_generated_words(pe_ctx, p->page))
      return 1;

     p = p->next_page;
     while(p != NULL)
     {
      if(p->next_page != NULL)
       if(strcmp(p->page, p->next_page->page) != 0)
        break;
      p = p->next_page;
     }
    }
   }
  }

  pr_advance_line(pe_ctx->pr_ctx);
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
  if(pr_send_to_line_buffer(pe_ctx->pr_ctx, " ", 1, 1))
   return 1;
 }

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

 /* free() takes place when used by word engine */
 if(unicode_word_engine_suffix(pe_ctx->uwc, superscript, length))
  return 1;

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pr_ctx = pe_ctx->pr_ctx;

 pe_ctx->footnote_pr = pr_ctx;

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

 if((f = pe_ctx->footnotes_tail) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_close() should not be here\n");
  return 1;
 }

 /* flush word */
 if(unicode_word_engine_consume_byte(pe_ctx->uwc, ' '))
  return 1;

 /* flush line */
 if(pr_flush_line_buffer(pe_ctx->pr_ctx))
  return 1;

 f->n_lines = pe_ctx->pr_ctx->output_buffer_lines;
 allocation_size = pe_ctx->pr_ctx->output_buffer_length + 1;
 if((f->buffer = (char *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_footnote_close() malloc(%d)\n", allocation_size);
  return 1;
 }
 memcpy(f->buffer, pe_ctx->pr_ctx->output_buffer, allocation_size);

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 pe_ctx->footnote_pr = NULL;

 return 0;
}

int plaintext_index_open(struct plaintext_engine_context *pe_ctx)
{
 int allocation_size;

 if(pe_ctx->index_term_buffer != NULL)
 {
  pe_ctx->index_term_buffer[pe_ctx->index_term_buffer_length] = 0;
  fprintf(stderr, "BCA: can not start index tag inside tag for term '%s'\n",
          pe_ctx->index_term_buffer);
  return 1;
 }

 if(pe_ctx->index_term_buffer == NULL)
 {
  allocation_size = MAX_INDEX_TERM_SIZE;
  if((pe_ctx->index_term_buffer = malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
   return 1;
  }
 }

 return 0;
}

int plaintext_index_close(struct plaintext_engine_context *pe_ctx)
{
 struct plaintext_index_entry *entry, *e, **prev;
 int allocation_size, code;
 char page[16];

 if(pe_ctx->index_term_buffer_length == 0)
 {
  fprintf(stderr, "BCA: plaintext_index_close(): no index term given\n");
  return 1;
 }

 /* throw away result except on second pass */
 if(pe_ctx->dctx->ctx->pass_number != 1)
 {
  free(pe_ctx->index_term_buffer);
  pe_ctx->index_term_buffer = NULL;
  pe_ctx->index_term_buffer_length = 0;
  return 0;
 }

 /* now we have the index term, allocate and store in the right place */

 allocation_size = sizeof(struct plaintext_index_entry);
 if((entry = (struct plaintext_index_entry *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: plaintext_index_open() malloc(%d) failed.\n",
          allocation_size);
  return 1;
 }
 memset(entry, 0, allocation_size);
 entry->term = pe_ctx->index_term_buffer;
 pe_ctx->index_term_buffer = NULL;
 pe_ctx->index_term_buffer_length = 0;
 snprintf(page, 16, "%d", pe_ctx->pr_ctx->current_page);
 entry->page = strdup(page);

 if(pe_ctx->index == NULL)
 {
  /* first index term */
  pe_ctx->index = entry;
  return 0;
 }

 prev = &(pe_ctx->index);
 e = pe_ctx->index;

 while(e != NULL)
 {
  code = strcmp(entry->term, e->term);
  if(code == 0)
  {
   /* previously used index term; add page reference */
   while(e->next_page != NULL)
   {
    e = e->next_page;
   }
   e->next_page = entry;
   return 0;
  }

  if(code < 0)
  {
   /* new term should be inserted at this list position */
   entry->next_term = e;
   *prev = entry;
   return 0;
  }

  if(e->next_term == NULL)
  {
   /* term comes after all previously seen index terms */
   e->next_term = entry;
   return 0;
  }

  prev = &(e->next_term);
  e = e->next_term;
 }

 return 1;
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
 pr_ctx->current_col = 0;
 pr_ctx->current_page = 0;
 pr_ctx->n_bytes = 0;
 pr_ctx->n_characters = 0;
 pr_ctx->line_buffer[0] = 0;

 pr_ctx->line_width = 0;
 pr_ctx->page_length = 0;
 pr_ctx->left_margin_width = 0;
 pr_ctx->right_margin_width = 0;
 pr_ctx->top_margin = 0;
 pr_ctx->bottom_margin = 0;
 pr_ctx->justification = PER_LEFT_JUSTIFY;
 pr_ctx->direction = PER_LEFT_TO_RIGHT;
 pr_ctx->show_page_numbers = 1;
 pr_ctx->output_mode = 0;
 pr_ctx->pad_listing_line_numbers = 0;
 pr_ctx->output_buffer = NULL;
 pr_ctx->output_buffer_length = 0;
 pr_ctx->output_buffer_size = 0;
 pr_ctx->output_buffer_lines = 0;

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
 pr_ctx->current_col = source->current_col;
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
 pr_ctx->pad_listing_line_numbers = source->pad_listing_line_numbers;

 /* buffer output is explicitly setup, so don't propagate */
 pr_ctx->output_buffer = NULL;
 pr_ctx->output_buffer_lines = 0;
 pr_ctx->output_buffer_size = 0;
 pr_ctx->output_buffer_length = 0;

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

int pr_send_to_line_buffer(struct plaintext_rendering_context *pr_ctx,
                           char *buffer, int n_bytes, int n_characters)
{
 int i, code;

 if(n_bytes == -1)
  n_bytes = strlen(buffer);

 switch(pr_ctx->output_mode)
 {
  case OUTPUT_MODE_TEXT_FILE:
  case OUTPUT_MODE_TTY:
       return pr_direct_to_line_buffer(pr_ctx, buffer, n_bytes, n_characters);
       break;

  case OUTPUT_MODE_HTML_FILE:
       i=0;
       while(i<n_bytes)
       {
        if((code = next_character(buffer + i, n_bytes - i)) == -1)
        {
         fprintf(stderr,
                 "BCA: UTF-8 problem inside pr_send_to_line_buffer(\"%s\", %d, %d), "
                 "next_character(\"%s\", %d) failed.\n",
                 buffer, n_bytes, n_characters, buffer + i, n_bytes - i);
         return 1;
        }

        if(code == 1)
        {
         switch(buffer[i])
         {
          case '<':
               if(pr_direct_to_line_buffer(pr_ctx, "&lt;", 4, 1))
                return 1;
               break;

          case '>':
               if(pr_direct_to_line_buffer(pr_ctx, "&gt;", 4, 1))
                return 1;
               break;

          case '&':
               if(pr_direct_to_line_buffer(pr_ctx, "&amp;", 4, 1))
                return 1;
               break;

          default:
               if(pr_direct_to_line_buffer(pr_ctx, buffer + i, 1, 1))
                return 1;
         }
        } else {
         if(pr_direct_to_line_buffer(pr_ctx, buffer + i, code, 1))
            return 1;
        }
        i += code;
       }
 }

 return 0;
}

int pr_direct_to_line_buffer(struct plaintext_rendering_context *pr_ctx,
                             char *buffer, int n_bytes, int n_characters)
{
 if(n_bytes == -1)
  n_bytes = strlen(buffer);

 if(pr_ctx->n_bytes + n_bytes + 1 > PER_LINE_BUFFER_SIZE)
 {
  fprintf(stderr, "BCA: pr_direct_to_line_buffer() "
          "PER_LINE_BUFFER_SIZE needs to be larger than %d. "
          "(line_buffer='%s',pr_ctx->n_bytes=%d,buffer='%s',n_bytes=%d)\n",
          pr_ctx->n_bytes + n_bytes + 1, pr_ctx->line_buffer,
          pr_ctx->n_bytes, buffer, n_bytes);
  return 1;
 }

 memcpy(pr_ctx->line_buffer + pr_ctx->n_bytes,
        buffer, n_bytes);
 pr_ctx->n_bytes += n_bytes;
 pr_ctx->line_buffer[pr_ctx->n_bytes] = 0;
 pr_ctx->n_characters += n_characters;

 return 0;
}
