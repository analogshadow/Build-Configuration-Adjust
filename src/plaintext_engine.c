#include "plaintext.h"

#include <sys/ioctl.h>

int plaintext_consume_text(struct document_handling_context *dctx,
                          char *text, int length)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;
 int i;

 for(i=0; i<length; i++)
 {
  /*  This takes us from a stream of bytes, and gives us a sequence of
      words, ignoring white space, and taking into consideration multi-byte
      UTF-8 characters. Calls here either result in failure, or invokations
      of the pe_consume_word() callback below with a word, the number of
      characters, and the number of bytes.
      (One reason it is done this way is because the word engine is intended
      to be used by all the different rendering engines.)
   */
  if(unicode_word_engine_consume_byte(pe_ctx->uwc, (unsigned char) text[i]))
  {
   fprintf(stderr, "BCA: plaintext_consume_text(): "
           "unicode_word_engine_consume_byte() failed\n");
   return 1;
  }
 }

 return 0;
}

int pe_consume_word(struct unicode_word_context *uwc, void *data, int flags)
{
 struct plaintext_engine_context *pe_ctx;
 struct plaintext_rendering_context *pr_ctx;

 pe_ctx = (struct plaintext_engine_context *) data;
 pr_ctx = pe_ctx->pr_ctx;

 if(pe_ctx->index_term_buffer != NULL)
 {
  /* inside an index tag - unlike the footnote logic, where the output path is redirected
     to an alternative rendering context, index terms just need to be collected to a buffer.
     Thus, we add a this conditional here to bypass feeding to a rendering context. */

  if(pe_ctx->index_term_buffer_length + uwc->buffer_length > MAX_INDEX_TERM_SIZE)
  {
   pe_ctx->index_term_buffer[pe_ctx->index_term_buffer_length] = 0;
   fprintf(stderr, "BCA: index term too long %s + %s...\n",
           pe_ctx->index_term_buffer, (char *) uwc->word_buffer);
   return 1;
  }

  memcpy(pe_ctx->index_term_buffer + pe_ctx->index_term_buffer_length,
         uwc->word_buffer,
         uwc->buffer_length);

  pe_ctx->index_term_buffer[pe_ctx->index_term_buffer_length += uwc->buffer_length] = 0;
  return 0;
 }

 /* we are inside some pass of the document loop, and within some
    hierchial element, and maybe within a tag stack. In all cases,
    here we simply feed a word to the current rendering context.
    The content may actually be writing text output, or it may be
    just be performing formating calculations.
 */
 return pr_advance_word(pr_ctx, uwc);
}

int plaintext_open_point(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_point(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_list(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_list(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_listing(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 struct listing_entry *listing_entry;
 struct loco_listing *ld = NULL;
 char *caption, *file_name;
 int handled, allocation_size, render_line_width, i;
 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(n_parameters > 1)
  caption = parameters[1];
 else
  caption = "";

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;

 /* first pass allocates entries, subsequent passes find the entry */
 if(dctx->ctx->pass_number == 0)
 {
  allocation_size = sizeof(struct listing_entry);
  if((listing_entry = (struct listing_entry *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
           allocation_size, strerror(errno));
   return 1;
  }
  memset(listing_entry, 0, allocation_size);
  if((listing_entry->caption = strdup(caption)) == NULL)
  {
   fprintf(stderr, "BCA: strdup(%s) failed, %s\n",
           caption, strerror(errno));
   return 1;
  }
  if(pe_ctx->listings_head == NULL)
  {
   /* first one */
   listing_entry->listing_number = 1;
   pe_ctx->listings_cursor = pe_ctx->listings_head = listing_entry;
  } else {
   /* not the first one */
   listing_entry->listing_number = pe_ctx->listings_cursor->listing_number + 1;
   pe_ctx->listings_cursor->next = listing_entry;
   pe_ctx->listings_cursor = listing_entry;
  }
 } else {

  if(pe_ctx->listings_cursor == NULL)
   pe_ctx->listings_cursor = pe_ctx->listings_head;
  else
   pe_ctx->listings_cursor = pe_ctx->listings_cursor->next;

  /* we should simply be the next entry in the listings list, so just verify */
  if(pe_ctx->listings_cursor == NULL)
  {
   fprintf(stderr, "BCA: expected non-null listings cursor on non-first pass!\n");
   return 1;
  }

  if(strcmp(pe_ctx->listings_cursor->caption, caption) != 0)
  {
   fprintf(stderr,
           "BCA: expected caption of listings cursor (\"%s\") to match \"%s\"!\n",
           pe_ctx->listings_cursor->caption, caption);
   return 1;
  }

  listing_entry = pe_ctx->listings_cursor;
 }

 if(n_parameters > 2)
 {
  handled = 0;

  if(strcmp(parameters[2], "locolisting") == 0)
  {
   handled = 1;
   if(n_parameters == 3)
   {
    fprintf(stderr, "BCA: listing mode 'locolisting' expects file name parameter\n");
    return 1;
   }

   file_name = parameters[3];

   if(dctx->ctx->pass_number == 0)
   {
    listing_entry->listing_type = LISTING_TYPE_LOCOLISTING;
   } else {
    if(listing_entry->listing_type != LISTING_TYPE_LOCOLISTING)
    {
     fprintf(stderr, "BCA: expected listings cursor to be of type %d not %d!\n",
             LISTING_TYPE_LOCOLISTING, listing_entry->listing_type);
     return 1;
    }
   }

   if(dctx->ctx->pass_number == 0)
   {
    if(handle_locolisting(dctx, pe_ctx, listing_entry, file_name))
     return 1;

    ld = (struct loco_listing *) listing_entry->listing_type_specific;
    render_line_width = pe_ctx->pr_ctx->line_width -
                        pe_ctx->pr_ctx->left_margin_width -
                        pe_ctx->pr_ctx->right_margin_width;

    /* now that we know the source's width, height, and line number info,
       we can either re-run handle_locolisting() to find render width and
       height, or if it will fit - just use the info from the source */
    if(ld->n_chars_needed_for_line_numbers + ld->width > render_line_width)
    {
     listing_entry->render_width = render_line_width;
     if(handle_locolisting(dctx, pe_ctx, listing_entry, file_name))
      return 1;
    } else {
     listing_entry->render_width = ld->width +
                                   ld->n_chars_needed_for_line_numbers;
     listing_entry->render_height = ld->height;
     for(i=0; i<listing_entry->render_height; i++)
     {
      if(pr_advance_line(pe_ctx->pr_ctx))
       return 1;
     }

    }

   } else {
    if(handle_locolisting(dctx, pe_ctx, listing_entry, file_name))
     return 1;
   }
  }

  if(handled == 0)
  {
   fprintf(stderr, "BCA: unknown listing mode '%s'\n", parameters[2]);
   return 1;
  }
 }

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 return 0;
}

int plaintext_close_listing(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_inset(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_inset(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_subsection(struct document_handling_context *dctx,
                              char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *subsection_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  subsection_name = parameters[1];

 if(dctx->ctx->pass_number == 0)
 {
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_SUB, subsection_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_SUB)
  {
   fprintf(stderr, "BCA: toc logic fail; cursor is not a subsection\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Subsection %d: %s", pe_ctx->toc_cursor->count, subsection_name);

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_LEFT_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 3;
 pe_ctx->pr_ctx->right_margin_width = 10;

 if(pr_ensure_minimum_rows_left(pe_ctx->pr_ctx, 3))
  return 1;

 if(pr_enable_attribute(pe_ctx->pr_ctx, "subsection_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, temp))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(dctx->ctx->pass_number != 0)
 {
  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 return 0;
}

int plaintext_close_subsection(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_section(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *section_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  section_name = parameters[1];

 if(dctx->ctx->pass_number == 0)
 {
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_SECTION, section_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_SECTION)
  {
   fprintf(stderr, "BCA: toc logic fail; cursor is not a section\n");
   return 1;
  }
 }

//this seems broken
// if(pr_ensure_minimum_rows_left(pe_ctx->pr_ctx, 3))
//  return 1;

 snprintf(temp, 256, "Section %d: %s", pe_ctx->toc_cursor->count, section_name);

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 10;
 pe_ctx->pr_ctx->right_margin_width = 10;

 if(pr_enable_attribute(pe_ctx->pr_ctx, "section_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, temp))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(dctx->ctx->pass_number != 0)
 {
  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 return 0;
}

int plaintext_close_section(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_chapter(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *chapter_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  chapter_name = parameters[1];

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->show_page_numbers = 1;
 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 10;
 pe_ctx->pr_ctx->right_margin_width = 10;

 if(pr_third_way_down(pe_ctx->pr_ctx))
  return 1;

 if(dctx->ctx->pass_number == 0)
 {
  /* this is done after the likely page advance, so the correct
     page number is picked up*/
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_CHAPTER, chapter_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_CHAPTER)
  {
   fprintf(stderr, "BCA: toc logic fail; cursor is not a chapter\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Chapter %d: %s", pe_ctx->toc_cursor->count, chapter_name);

 if(pr_enable_attribute(pe_ctx->pr_ctx, "chapter_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, temp))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(dctx->ctx->pass_number != 0)
 {
  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 return 0;
}

int plaintext_close_chapter(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(pe_ctx->footnotes_head != NULL)
 {
  if(pr_render_foot_notes(pe_ctx, 0))
   return 1;
 }

 return 0;
}

int plaintext_open_part(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *part_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  part_name = parameters[1];

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

// pe_ctx->pr_ctx->show_page_numbers = 0;
 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 10;
 pe_ctx->pr_ctx->right_margin_width = 10;

 if(pr_center_row(pe_ctx->pr_ctx))
  return 1;

 if(dctx->ctx->pass_number == 0)
 {
  /* this is done after the likely page advance, so the correct
     page number is picked up*/
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_PART, part_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_PART)
  {
   fprintf(stderr, "BCA: toc logic fail; cursor is not a part\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Part %d: %s", pe_ctx->toc_cursor->count, part_name);

 if(pr_enable_attribute(pe_ctx->pr_ctx, "part_title"))
  return 1;

 if(pr_feed_generated_words(pe_ctx, temp))
  return 1;

 if(pr_disable_attribute(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_page(pe_ctx->pr_ctx))
  return 1;

 if(plaintext_word_engine_stack_pop(pe_ctx))
  return 1;

 if(plaintext_rendering_stack_pop(pe_ctx))
  return 1;

 if(dctx->ctx->pass_number != 0)
 {
  if(pe_toc_cursor_advance(pe_ctx))
   return 1;
 }

 return 0;
}

int plaintext_close_part(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_table(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_table(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_tr(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_tr(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_tc(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_close_tc(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 return 0;
}

int plaintext_open_tag(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *tag_name;

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
  return plaintext_paragraph_open(pe_ctx);

 if(strcmp(tag_name, "f") == 0)
  return plaintext_footnote_open(pe_ctx);

 if(strcmp(tag_name, "i") == 0)
  return plaintext_index_open(pe_ctx);

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

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
  return plaintext_paragraph_close(pe_ctx);

 if(strcmp(tag_name, "f") == 0)
  return plaintext_footnote_close(pe_ctx);

 if(strcmp(tag_name, "i") == 0)
  return plaintext_index_close(pe_ctx);

 return 0;
}

int plaintext_start_document(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *pe_ctx;

 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 /* _start_document() is called on each loop */
 switch(dctx->ctx->pass_number)
 {
  case 0:
       break;

  case 1:
       if((pe_ctx = (struct plaintext_engine_context *)
                     dctx->document_engine_context) == NULL)
       {
        fprintf(stderr, "BCA: plaintex_start_document(): render engine should have been initialized "
                " on start of first pass\n");
        return 1;
       }

       pe_ctx->pr_ctx->output = stdout;
       pe_ctx->pr_ctx->current_row = -1;
       pe_ctx->pr_ctx->current_page = 0;
       pe_ctx->even_or_odd_page = 1;
       pe_ctx->n_footnotes = 0;
       pe_ctx->listings_cursor = NULL;

       switch(pe_ctx->pr_ctx->output_mode)
       {
        case OUTPUT_MODE_HTML_FILE:
        if(pe_ctx->pr_ctx->output != NULL)
        {
         fprintf(pe_ctx->pr_ctx->output,
                 "<html>\n"
                 " <head>\n"
                 "  <meta charset=\"UTF-8\">\n"
                 "  <link rel=\"stylesheet\" type=\"text/css\" href=\"bcadoc.css\">\n"
                 " </head>\n"
                 " <body background=\"bg.jpg\">\n");
        }
        break;
       }

       if(pe_ctx->show_toc == 1)
        if(pe_print_toc(pe_ctx))
         return 1;

       pe_ctx->toc_cursor = pe_ctx->toc_root;
       pe_ctx->pr_ctx->current_page = 0;
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
 pe_ctx = (struct plaintext_engine_context *) dctx->document_engine_context;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if(pr_advance_page(pe_ctx->pr_ctx))
  return 1;

 switch(dctx->ctx->pass_number)
 {
  case 0: /* proceede to next pass */
       dctx->ctx->loop_inputs = 1;
       break;

  case 1: /* we're done. finish and shutdown */
       if(pe_ctx->show_index == 1)
        if(pe_print_index(pe_ctx))
         return 1;

       switch(pe_ctx->pr_ctx->output_mode)
       {
        case OUTPUT_MODE_HTML_FILE:
        if(pe_ctx->pr_ctx->output != NULL)
        {
         if(pe_ctx->even_or_odd_page == 0) //gets swapped one last time above
         {
          pe_ctx->pr_ctx->show_page_numbers = 0;

          if(pr_advance_line(pe_ctx->pr_ctx))
           return 1;

          if(pr_advance_page(pe_ctx->pr_ctx))
           return 1;

         }

         fprintf(pe_ctx->pr_ctx->output,
                 " </body>\n"
                 "</html>\n");
        }
        break;
       }

       hyphenation_engine_finalize(pe_ctx->hc);
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

int activate_document_engine_plaintext(struct document_handling_context *dctx)
{
 int allocation_size;
 struct plaintext_engine_context *pe_ctx;
 struct plaintext_rendering_context *pr_ctx;
 struct winsize tty_size;

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

 if(dctx->document_engine_context != NULL)
 {
  fprintf(stderr,
          "BCA: activate_document_engine_plaintext(): render engine should not be initialized.\n");
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

 if((pe_ctx->hc = hyphenation_engine_initialize("en_US")) == NULL)
 {
  fprintf(stderr,
          "BCA: Warning: hyphenation engine not available for language \"%s\"\n",
          "en_US");
 }

 if((pr_ctx = plaintext_rendering_context_new(pe_ctx, NULL)) == NULL)
 {
  return 1;
 }

 pe_ctx->pr_ctx = pr_ctx;

 if(strcmp(dctx->ctx->output_type, "text") == 0)
 {
  pr_ctx->output_mode = OUTPUT_MODE_TEXT_FILE;

  if(conf_lookup_int(dctx, "line_width",
                     &(pr_ctx->line_width), 80))
   return 1;

  if(conf_lookup_int(dctx, "page_length",
                     &(pr_ctx->page_length), 50))
   return 1;

  if(conf_lookup_int(dctx, "top_margin",
                     &(pr_ctx->top_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "bottom_margin",
                     &(pr_ctx->bottom_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "left_margin",
                     &(pr_ctx->left_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "right_margin",
                     &(pr_ctx->right_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_line_spacing",
                     &(pe_ctx->paragraph_line_spacing), 1))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_indent",
                     &(pe_ctx->paragraph_indent), 4))
   return 1;

  if(conf_lookup_int(dctx, "show_toc",
                     &(pe_ctx->show_toc), 1))
   return 1;

  if(conf_lookup_int(dctx, "show_index",
                     &(pe_ctx->show_index), 1))
   return 1;

  if(conf_lookup_int(dctx, "pad_listing_line_numbers",
                     &(pr_ctx->pad_listing_line_numbers), 1))
   return 1;

 } else if(strcmp(dctx->ctx->output_type, "html") == 0) {

  pr_ctx->output_mode = OUTPUT_MODE_HTML_FILE;

  if(conf_lookup_int(dctx, "line_width",
                     &(pr_ctx->line_width), 80))
   return 1;

  if(conf_lookup_int(dctx, "page_length",
                     &(pr_ctx->page_length), 50))
   return 1;

  if(conf_lookup_int(dctx, "top_margin",
                     &(pr_ctx->top_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "bottom_margin",
                     &(pr_ctx->bottom_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "left_margin",
                     &(pr_ctx->left_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "right_margin",
                     &(pr_ctx->right_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_line_spacing",
                     &(pe_ctx->paragraph_line_spacing), 1))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_indent",
                     &(pe_ctx->paragraph_indent), 4))
   return 1;

  if(conf_lookup_int(dctx, "show_toc",
                     &(pe_ctx->show_toc), 1))
   return 1;

  if(conf_lookup_int(dctx, "show_index",
                     &(pe_ctx->show_index), 1))
   return 1;

  if(conf_lookup_int(dctx, "pad_listing_line_numbers",
                     &(pe_ctx->pr_ctx->pad_listing_line_numbers), 0))
   return 1;

 } else if(strcmp(dctx->ctx->output_type, "tty") == 0) {

  pr_ctx->output_mode = OUTPUT_MODE_TTY;

  if(ioctl(0, TIOCGWINSZ, &tty_size))
  {
   fprintf(stderr, "BCA: ioctl() failed, \"%s\"\n", strerror(errno));
   return 1;
  }
  pr_ctx->line_width = tty_size.ws_col;
  pr_ctx->page_length = tty_size.ws_row;

  if(conf_lookup_int(dctx, "top_margin",
                     &(pr_ctx->top_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "bottom_margin",
                     &(pr_ctx->bottom_margin), 1))
   return 1;

  if(conf_lookup_int(dctx, "left_margin",
                     &(pr_ctx->left_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "right_margin",
                     &(pr_ctx->right_margin_width), 0))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_line_spacing",
                     &(pe_ctx->paragraph_line_spacing), 1))
   return 1;

  if(conf_lookup_int(dctx, "paragraph_indent",
                     &(pe_ctx->paragraph_indent), 4))
   return 1;

  if(conf_lookup_int(dctx, "show_toc",
                     &(pe_ctx->show_toc), 1))
   return 1;

  if(conf_lookup_int(dctx, "show_index",
                     &(pe_ctx->show_index), 1))
   return 1;

  if(conf_lookup_int(dctx, "pad_listing_line_numbers",
                     &(pr_ctx->pad_listing_line_numbers), 1))
   return 1;


 } else {
  fprintf(stderr,
          "BCA: plaintext engine - unknown output type \"%s\", try text, html, or tty\n",
          dctx->ctx->output_type);
  return 1;
 }

 pe_ctx->even_or_odd_page = 1;

 dctx->document_engine_context = pe_ctx;
 return 0;
}

int plaintext_word_engine_stack_push(struct plaintext_engine_context *pe_ctx)
{
 struct unicode_word_context *uwc;

 if(pe_ctx->word_engine_stack_depth > 14)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 if((uwc =
     unicode_word_engine_initialize(pe_ctx, pe_consume_word)) == NULL)
 {
  return 1;
 }

 pe_ctx->uwc_stack[pe_ctx->word_engine_stack_depth++] = pe_ctx->uwc;
 pe_ctx->uwc = uwc;

 return 0;
}

int plaintext_word_engine_stack_pop(struct plaintext_engine_context *pe_ctx)
{
 if(pe_ctx->word_engine_stack_depth < 1)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 if(unicode_word_engine_finalize(pe_ctx->uwc))
  return 1;

 pe_ctx->uwc =
  pe_ctx->uwc_stack[--pe_ctx->word_engine_stack_depth];

 return 0;
}

int plaintext_rendering_stack_push(struct plaintext_engine_context *pe_ctx)
{
 struct plaintext_rendering_context *pr_ctx;

 if(pe_ctx->rendering_context_stack_depth > 14)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 if((pr_ctx = plaintext_rendering_context_copy(pe_ctx->pr_ctx)) == NULL)
 {
  return 1;
 }

 pe_ctx->pr_ctx_stack[pe_ctx->rendering_context_stack_depth++] = pe_ctx->pr_ctx;
 pe_ctx->pr_ctx = pr_ctx;
 return 0;
}

int plaintext_rendering_stack_pop(struct plaintext_engine_context *pe_ctx)
{
 struct plaintext_rendering_context *pr_ctx;

 if(pe_ctx->rendering_context_stack_depth < 1)
 {
  fprintf(stderr, "BCA: should not be here: %s %d\n", __FILE__, __LINE__);
  return 1;
 }

 pr_ctx = pe_ctx->pr_ctx;

 pe_ctx->pr_ctx =
  pe_ctx->pr_ctx_stack[--pe_ctx->rendering_context_stack_depth];

 pe_ctx->pr_ctx->current_row = pr_ctx->current_row;
 pe_ctx->pr_ctx->current_page = pr_ctx->current_page;
 pe_ctx->pr_ctx->current_col = pr_ctx->current_col;

 if(plaintext_rendering_context_finalize(pr_ctx))
  return 1;

 return 0;
}
