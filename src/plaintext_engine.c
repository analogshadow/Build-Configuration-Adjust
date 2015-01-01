#include "plaintext.h"

int plaintext_consume_text(struct document_handling_context *dctx,
                          char *text, int length)
{
 struct plaintext_engine_context *pe_ctx;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;
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
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_point(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_list(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_list(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int argument_escape_processing(char *file_name, int line_number,
                               char *buffer, int length)
{
 int i = 0, in_quotes = 0;

 while(i < length)
 {
  if(in_quotes)
  {
   if(buffer[i] == '\\')
   {
    memmove(buffer + i, buffer + i + 1, length - i - 1);
    i++;
   } else {
    if(buffer[i] == '\"')
    {
     buffer[i] = 0;
     break;
    }
    i++;
   }

  } else {
   if(buffer[i] == '\"')
   {
    in_quotes = 1;
   } else if(buffer[i] == ' ') {
    i++;
   } else {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d unquoted text argument '%s'\n",
            file_name, line_number, buffer);
    return -1;
   }

   memmove(buffer + i, buffer + i + 1, length - i - 1);
  }
 }

 return i;
}

int send_to_line_buffer(struct plaintext_rendering_context *pr_ctx,
                        struct loco_listing *ld,
                        char *buffer, int n_bytes, int n_chars)
{
 int width;

 /* line width for current rendering context */
 width = pr_ctx->line_width -
         pr_ctx->left_margin_width -
         pr_ctx->right_margin_width;

 if(pr_ctx->n_characters + n_chars > width)
 {
  fprintf(stderr, "BCA: listing line wrap - finish me: %s %d\n", __FILE__, __LINE__);
  return 1;
/*
  how do you split attributes?
  also we will need to add space for the line numbers
  if(pr_advance_line(pe_ctx->pr_ctx))
   return 1;

  ld->render_height++;
*/
 } else {
  if(pr_ctx->n_bytes + n_bytes + 1 > PER_LINE_BUFFER_SIZE)
  {
   fprintf(stderr, "BCA: fix me: %s %d\n", __FILE__, __LINE__);
   return 1;
  }

  memcpy(pr_ctx->line_buffer + pr_ctx->n_bytes,
         buffer, n_bytes);
  pr_ctx->line_buffer[pr_ctx->n_bytes += n_bytes] = 0;
  pr_ctx->n_characters += n_chars;
 }

 return 0;
}

int handle_locolisting(struct document_handling_context *dctx,
                       struct plaintext_engine_context *pe_ctx,
                       struct listing_entry *listing_entry,
                       char *file_name)
{
 struct loco_listing *ld;
 char *source, data[32], argument[1024];
 int source_length, start, end, line_length, i, comma, col, line,
     handled, allocation_size, declarative_in_scope, argument_length,
     n_chars, line_width;

 if(listing_entry->listing_type_specific == NULL)
 {
  /* first pass */
  allocation_size = sizeof(struct loco_listing);
  if((ld = (struct loco_listing *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
           allocation_size, strerror(errno));
   return 1;
  }
  memset(ld, 0, allocation_size);

  if((ld->file_name = strdup(file_name)) == NULL)
  {
   fprintf(stderr, "BCA: strdup(%s) failed, %s\n",
           file_name, strerror(errno));
   return 1;
  }

  ld->line_numbers_start = 1;

  listing_entry->listing_type_specific = ld;
 } else {
  /* not first pass */
  ld = (struct loco_listing *) listing_entry->listing_type_specific;

  if(strcmp(ld->file_name, file_name) != 0)
  {
   fprintf(stderr,
           "BCA: expected listing cursor locolisting file name to be %s not %s!\n",
           file_name, ld->file_name);
   return 1;
  }

  ld->line = ld->line_numbers_start;
 }

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 if((source = read_file(file_name, &source_length, 0)) == NULL)
  return 1;


 declarative_in_scope = -1;
 line = 0;
 line_width = 0;
 end = -1;

 if(1)
 {
  n_chars = snprintf(data, 32, "%d ", ld->line);
  if(send_to_line_buffer(pe_ctx->pr_ctx, ld, data, n_chars, n_chars))
    return 1;
 }

 while(find_line(source, source_length, &start, &end, &line_length))
 {
  line++;
  i = start;
  data[0] = 0;

  /* lines are in the form declarative:data[,arg]; */

  /* skip leading whitespace */
  while(source[i] == ' ')
  {
   i++;
   if(i == end)
    break;
  }

  /* ignore blank lines */
  if(i == end)
   continue;

  /* look for a ; */
  while(end > i)
  {
   if(source[end] == ';')
    break;
   end--;
  }
  if(end == i)
  {
   fprintf(stderr,
           "BCA: locolisting format file %s line %d: "
           "expected terminating ';' on non-blank line\n",
           file_name, line);
   return 1;
  }

  col = i;
  /* look for a : */
  while(col < end)
  {
   if(source[col] == ':')
    break;
   col++;
  }

  if(col == end)
  {
   fprintf(stderr,
           "BCA: locolisting format file %s line %d: expected ':' on non-blank line\n",
           file_name, line);
   return 1;
  }

  /* look for a , */
  comma = col + 1;
  while(comma < end)
  {
   if(source[comma] == ',')
    break;
   comma++;
  }
  if(comma == end)
   comma = -1;

  /* match declarative */
  handled = 0;

  if(strncmp(source + i, "listing", 7) == 0)
  {
   handled = 1;
   declarative_in_scope = 1;
   allocation_size = (end - col);

   if(ld->caption == NULL)
   {

    /* first pass */
    if((ld->caption = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
             allocation_size, strerror(errno));
     return 1;
    }
    memcpy(ld->caption, source + col + 1, allocation_size - 1);
    ld->caption[allocation_size] = 0;

   } else {

    /* not first pass */
    if(strncmp(ld->caption, source + col + 1, allocation_size - 1) != 0)
    {
     source[col + 1 + allocation_size - 1] = 0;
     fprintf(stderr,
             "BCA: expected listing cursor's locolisting supplied caption to "
             "be \"%s\" not \"%s\"!\n",
             ld->caption, source + col + 1);
     return 1;
    }

   }
  } else {
   if(comma == -1)
   {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d: expected declarative:data,argument;\n",
            file_name, line);
    return 1;
   }
   if((allocation_size = comma - col - 1) > 31)
   {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d: data too long (%d) in format "
            "declarative:data,argument;\n",
            file_name, line, allocation_size);
    return 1;
   }
   memcpy(data, source + col + 1, allocation_size);
   data[allocation_size] = 0;
  }

  if(strncmp(source + i, "object", 6) == 0)
  {
   handled = 1;
   declarative_in_scope = 2;

   if((argument_length = end - comma - 1) > 31)
   {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d: argument too long (%d) in format "
            "declarative:data,argument;\n",
            file_name, line, allocation_size);
    return 1;
   }
   memcpy(argument, source + comma + 1, argument_length);
   argument[argument_length] = 0;

   if((argument_length =
       argument_escape_processing(file_name, line, argument, argument_length)) < 0)
    return 1;

   if((n_chars = count_characters(argument, argument_length)) < 0)
    return 1;

   if(strcmp(data, "text") == 0)
   {
    if(send_to_line_buffer(pe_ctx->pr_ctx, ld, argument, argument_length, n_chars))
     return 1;

    /* independently, track the width and height of the listing as it is in the
       listing file */
    line_width += n_chars;

   } else if(strcmp(data, "linebreak") == 0) {
    ld->height++;
    listing_entry->render_height++;
    ld->line++;

    if(line_width > ld->width)
     ld->width = line_width;

    line_width = n_chars;

    if(pr_advance_line(pe_ctx->pr_ctx))
     return 1;

    if(1)
    {
     n_chars = snprintf(data, 32, "%d ", ld->line);
     if(send_to_line_buffer(pe_ctx->pr_ctx, ld, data, n_chars, n_chars))
      return 1;
    }

    if(send_to_line_buffer(pe_ctx->pr_ctx, ld, argument, argument_length, n_chars))
     return 1;

   } else {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d: object type \"%s\" unrecognized\n",
            file_name, line, data);
    return 1;
   }
  }

  if(strncmp(source + i, "attribute", 9) == 0)
  {
   handled = 1;
   switch(declarative_in_scope)
   {
    case -1:
         fprintf(stderr,
                 "BCA: locolisting format file %s line %d: attribute outside "
                 "scope of a declarative\n",
                 file_name, line);
         return 1;
         break;

    case 1:
         if(strcmp(data, "listingtype") == 0)
         {
          if(strncmp(source + comma + 1, "sourcecode", 10) == 0)
          {

          } else {
           source[end] = 0;
           fprintf(stderr,
                   "BCA: locolisting format file %s line %d: unknown value \"%s\" "
                   " for listingtype attribute\n",
                   file_name, line, source + comma + 1);
            return 1;
          }

         } else if(strcmp(data, "linenumbers") == 0) {
          if(strncmp(source + comma + 1, "yes", 3) == 0)
          {
           ld->line_numbers = 1;
          } else if(strncmp(source + comma + 1, "no", 2) == 0) {
           ld->line_numbers = 0;
          } else {
           source[end] = 0;
           fprintf(stderr,
                   "BCA: locolisting format file %s line %d: linenumbers attribute "
                   "expects yes or no, not '%s'\n",
                   file_name, line, source + comma + 1);
            return 1;
          }

         } else {
          fprintf(stderr,
                  "BCA: locolisting format file %s line %d: attribute type \"%s\" "
                  "unrecognized for declarative in scope of \"listing\".\n",
                  file_name, line, data);
           return 1;
         }
         break;

    case 2:
         if(strcmp(data, "syntaxhighlight") == 0)
         {

         } else {
          fprintf(stderr,
                  "BCA: locolisting format file %s line %d: attribute type \"%s\" "
                  "unrecognized for declarative in scope of \"object\".\n",
                  file_name, line, data);
           return 1;
         }
         break;
   }
  }

  if(handled == 0)
  {
   source[col] = 0;
   fprintf(stderr,
           "BCA: locolisting format file %s line %d: unknown type declaritive \"%s\"\n",
           file_name, line, source + i);
   return 1;
  }

 }
 ld->height++;
 listing_entry->render_height++;

 if(pr_advance_line(pe_ctx->pr_ctx))
  return 1;

 free(source);
 return 0;
}

int plaintext_open_listing(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 struct listing_entry *listing_entry;
 char *caption, *file_name;
 int handled, allocation_size;
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(n_parameters > 1)
  caption = parameters[1];
 else
  caption = "";

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

   return handle_locolisting(dctx, pe_ctx, listing_entry, file_name);
  }

  if(handled == 0)
  {
   fprintf(stderr, "BCA: unknown listing mode '%s'\n", parameters[2]);
   return 1;
  }
 }

 return 0;
}

int plaintext_close_listing(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_inset(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_inset(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_subsection(struct document_handling_context *dctx,
                              char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *subsection_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(n_parameters == 2)
  subsection_name = parameters[1];

 if(dctx->ctx->pass_number == 0)
 {
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_SUB, subsection_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_SUB)
  {
   fprintf(stderr, "BCA: toc logic fail\n");
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

 if(pr_feed_generated_words(pe_ctx, temp))
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
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_section(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *section_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(n_parameters == 2)
  section_name = parameters[1];

 if(dctx->ctx->pass_number == 0)
 {
  if(plaintext_add_toc_element(pe_ctx, DLEVEL_SECTION, section_name))
   return 1;
 } else {
  if(pe_ctx->toc_cursor->type != DLEVEL_SECTION)
  {
   fprintf(stderr, "BCA: toc logic fail\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Section %d: %s", pe_ctx->toc_cursor->count, section_name);

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->justification = PER_CENTER_JUSTIFY;
 pe_ctx->pr_ctx->left_margin_width = 10;
 pe_ctx->pr_ctx->right_margin_width = 10;

 if(pr_feed_generated_words(pe_ctx, temp))
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
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_chapter(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *chapter_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

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
   fprintf(stderr, "BCA: toc logic fail\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Chapter %d: %s", pe_ctx->toc_cursor->count, chapter_name);

 if(pr_feed_generated_words(pe_ctx, temp))
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
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(pr_render_foot_notes(pe_ctx, 0))
  return 1;

 return 0;
}

int plaintext_open_part(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *pe_ctx;
 char *part_name = NULL, temp[256];

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 if(n_parameters == 2)
  part_name = parameters[1];

 if(plaintext_rendering_stack_push(pe_ctx))
  return 1;

 if(plaintext_word_engine_stack_push(pe_ctx))
  return 1;

 pe_ctx->pr_ctx->show_page_numbers = 0;
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
   fprintf(stderr, "BCA: toc logic fail\n");
   return 1;
  }
 }

 snprintf(temp, 256, "Part %d: %s", pe_ctx->toc_cursor->count, part_name);

 if(pr_feed_generated_words(pe_ctx, temp))
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
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_table(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_table(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tr(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tr(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tc(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tc(struct document_handling_context *dctx)
{
// struct plaintext_engine_context *pe_ctx;
// pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

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

 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;
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

       if((pe_ctx->hc = hyphenation_engine_initialize("en_US")) == NULL)
       {
        fprintf(stderr, "BCA: Warning: hyphenation engine not available\n");
       }

       /* hard code some defaults for now */
       pe_ctx->paragraph_line_spacing = 1;
       pe_ctx->paragraph_indent = 4;
       pe_ctx->show_toc = 1;
       pe_ctx->show_index = 1;
       pe_ctx->even_or_odd_page = 1;

       dctx->render_engine_context = pe_ctx;

       if((pe_ctx->pr_ctx = plaintext_rendering_context_new(pe_ctx, NULL)) == NULL)
       {
        return 1;
       }
       break;

  case 1:
       if((pe_ctx = (struct plaintext_engine_context *)
                     dctx->render_engine_context) == NULL)
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
       pe_ctx->listings_cursor = pe_ctx->listings_head;

       switch(pe_ctx->pr_ctx->output_mode)
       {
        case PER_OUTPUT_MODE_HTML_FILE:
        if(pe_ctx->pr_ctx->output != NULL)
        {
         fprintf(pe_ctx->pr_ctx->output,
                 "<html>\n"
                 " <head>\n"
                 "  <meta charset=\"UTF-8\">\n"
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
 pe_ctx = (struct plaintext_engine_context *) dctx->render_engine_context;

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
        case PER_OUTPUT_MODE_HTML_FILE:
        if(pe_ctx->pr_ctx->output != NULL)
        {
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


