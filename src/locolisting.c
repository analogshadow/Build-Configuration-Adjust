#include "plaintext.h"

#define SCREEN_COLOR_NORMAL  255
#define SCREEN_COLOR_BLACK   256
#define SCREEN_COLOR_RED     257
#define SCREEN_COLOR_GREEN   258
#define SCREEN_COLOR_YELLOW  259
#define SCREEN_COLOR_BLUE    260
#define SCREEN_COLOR_MAGENTA 261
#define SCREEN_COLOR_CYAN    262
#define SCREEN_COLOR_WHITE   263

#define LL_DONE      799
#define LL_LISTING   800
#define LL_OBJECT    801
#define LL_ATTRIBUTE 802

#define SOURCE_CODE 340
#define SCREEN_LOG  341

struct source_listing_object
{
 int line_break, n_bytes, n_chars;
 char text[1024];
 char attribute[1024];
};

struct screen_log_object
{
 int line_break, n_bytes, n_chars;
 int foreground, background, bold;
 char text[1024];
};

struct locolisting_operation_context
{
 struct plaintext_engine_context *pe_ctx;
 struct plaintext_rendering_context *pr_ctx;
 struct listing_entry *listing_entry;
 struct document_handling_context *dctx;
 struct loco_listing *ld;
 char *file_name;
 char *source;
 char *data;
 char *argument;
 int data_length;
 int source_length;
 int first_pass;
 int source_line_number;
 int argument_length;
 int argument_n_chars;
 int line_width;
 int cursor;
};

int color_to_code(struct locolisting_operation_context *octx, char *string)
{
 if(strcmp(string, "termfg-normal") == 0)
  return SCREEN_COLOR_NORMAL;
 else if(strcmp(string, "termfg-black") == 0)
  return SCREEN_COLOR_BLACK;
 else if(strcmp(string, "termfg-red") == 0)
  return SCREEN_COLOR_RED;
 else if(strcmp(string, "termfg-green") == 0)
  return SCREEN_COLOR_GREEN;
 else if(strcmp(string, "termfg-yellow") == 0)
  return SCREEN_COLOR_YELLOW;
 else if(strcmp(string, "termfg-blue") == 0)
  return SCREEN_COLOR_BLUE;
 else if(strcmp(string, "termfg-magenta") == 0)
  return SCREEN_COLOR_MAGENTA;
 else if(strcmp(string, "termfg-cyan") == 0)
  return SCREEN_COLOR_CYAN;
 else if(strcmp(string, "termfg-white") == 0)
  return SCREEN_COLOR_WHITE;
 else {
  fprintf(stderr, "BCA: locolisting file %s, line %d "
          "unknown terminal color in locolisting file \"%s\".\n",
          octx->file_name, octx->source_line_number, string);
  return -1;
 }
}

/* Un-escape locolisting escaping of strings */
int locolisting_escape_processing(struct locolisting_operation_context *octx,
                                  char *buffer, int length)
{
 int i = 0, in_quotes = 0, started = 0;

 while(i < length)
 {
  if(buffer[i] == '\"') {
   if(in_quotes == 0)
   {
    in_quotes = 1;
    memmove(buffer + i, buffer + i + 1, length - i - 1);
   } else {
    buffer[i] = 0;
    break;
   }
  } else if(buffer[i] == ' ') {
   if(!((started == 0) || (in_quotes == 1)))
   {
    fprintf(stderr,
            "BCA: locolisting format file %s line %d unquoted white space in argument '%s'\n",
            octx->file_name, octx->source_line_number, buffer);
    return -1;
   }
   i++;
  } else if(buffer[i] == '\\') {
   memmove(buffer + i, buffer + i + 1, length - i - 1);
   started = 1;
   i++;
  } else {
   started = 1;
   i++;
  }
 }

 return i;
}

int send_to_line_buffer(struct locolisting_operation_context *octx,
                        char *buffer, int n_bytes, int n_chars,
                        char *attribute, char *fill_attribute)
{
 int width, width_bytes, need_width, need_line_wrap, handled_bytes, handled_chars, s;

 /* line width for current rendering context */
 width = octx->pr_ctx->line_width -
         octx->pr_ctx->left_margin_width -
         octx->pr_ctx->right_margin_width;

 if(octx->pr_ctx->n_characters + n_chars > width)
 {
  need_width = width - octx->pr_ctx->n_characters - 1;
  handled_bytes = 0;
  handled_chars = 0;
  need_line_wrap = 1;

  while(need_line_wrap)
  {

   if((width_bytes =
       n_bytes_for_n_characters(buffer + handled_bytes,
                                n_bytes - handled_bytes,
                                need_width)) < 0)
    return 1;

   if(pr_enable_attribute(octx->pr_ctx, attribute))
    return 1;

   if(pr_send_to_line_buffer(octx->pr_ctx,
                             buffer + handled_bytes,
                             width_bytes, need_width))
    return 1;

   if(pr_disable_attribute(octx->pr_ctx))
    return 1;

   if(pr_enable_attribute(octx->pr_ctx, fill_attribute))
    return 1;

   if(pr_send_to_line_buffer(octx->pr_ctx, "→", 3, 1))
    return 1;

   if(pr_disable_attribute(octx->pr_ctx))
    return 1;

   if(pr_advance_line(octx->pr_ctx))
    return 1;

   if(octx->ld->line_numbers)
   {
    if(pr_enable_attribute(octx->pr_ctx, "sourcelistingnumbers"))
     return 1;

    for(s = 0; s < (octx->ld->n_chars_needed_for_line_numbers); s++)
    {
     if(pr_send_to_line_buffer(octx->pr_ctx, " ", 1, 1))
      return 1;
    }

    if(pr_disable_attribute(octx->pr_ctx))
     return 1;
   }

   handled_bytes += width_bytes;
   handled_chars += need_width;
   need_width = n_chars - handled_chars;
   if(need_width < width - octx->ld->n_chars_needed_for_line_numbers)
    need_line_wrap = 0;
   else
    need_width = width - 1 - octx->ld->n_chars_needed_for_line_numbers;
  }

  if(need_width > 0)
  {
   if((width_bytes =
       n_bytes_for_n_characters(buffer + handled_bytes,
                                n_bytes - handled_bytes,
                                need_width)) < 0)
    return 1;

   if(pr_enable_attribute(octx->pr_ctx, attribute))
    return 1;

   if(pr_send_to_line_buffer(octx->pr_ctx,
                             buffer + handled_bytes,
                             width_bytes, need_width))
    return 1;

   if(pr_disable_attribute(octx->pr_ctx))
    return 1;

  }

 } else {

  if(pr_enable_attribute(octx->pr_ctx, attribute))
   return 1;

  if(pr_send_to_line_buffer(octx->pr_ctx, buffer, n_bytes, n_chars))
    return 1;

   if(pr_disable_attribute(octx->pr_ctx))
    return 1;

 }

 return 0;
}

int render_locolisting_caption(struct locolisting_operation_context *octx)
{
 char temp[64];

 if(pr_enable_attribute(octx->pr_ctx, "listing_caption"))
  return 1;

 snprintf(temp, 64, "Listing %d:", octx->listing_entry->listing_number);

 if(pr_feed_generated_words(octx->pe_ctx, temp))
  return 1;

 if(octx->ld->caption != NULL)
 {
  if(pr_feed_generated_words(octx->pe_ctx, octx->ld->caption))
   return 1;
 } else {
  if(pr_feed_generated_words(octx->pe_ctx, octx->listing_entry->caption))
   return 1;
 }

 if(pr_disable_attribute(octx->pe_ctx->pr_ctx))
  return 1;

 return pr_advance_line(octx->pe_ctx->pr_ctx);
}

int render_locolisting_line_numbers(struct locolisting_operation_context *octx)
{
 int n_chars, s;
 char buffer[32];

 if(octx->ld->line_numbers)
 {
  if(octx->pr_ctx->pad_listing_line_numbers)
   n_chars = snprintf(buffer, 32, "%d ", octx->ld->line);
  else
   n_chars = snprintf(buffer, 32, "%d", octx->ld->line);

  /* right justify */
  for(s = 0; s < (octx->ld->n_chars_needed_for_line_numbers - n_chars); s++)
  {
   memmove(buffer + s + 1, buffer + s, n_chars++);
           buffer[s] = ' ';
  }

  if(pr_enable_attribute(octx->pr_ctx, "sourcelistingnumbers"))
   return 1;

  if(pr_send_to_line_buffer(octx->pr_ctx, buffer, n_chars, n_chars))
   return 1;

  if(pr_disable_attribute(octx->pr_ctx))
   return 1;

 }

 return 0;
}

int send_source_listing_object(struct locolisting_operation_context *octx,
                               struct source_listing_object *object,
                               int last_line)
{
 char *fill_attribute = "sourcelistingbackground";
 int s;

// fprintf(stderr, "object = [linebreak=%d, n_bytes=%d, n_chars=%d, text='%s', attribute='%s']\n",
//         object->line_break, object->n_bytes, object->n_chars, object->text, object->attribute);

 if(object->attribute[0] == 0)
  snprintf(object->attribute, 1024, "%s", fill_attribute);

 if(object->line_break)
 {
  if(pr_enable_attribute(octx->pr_ctx, fill_attribute))
   return 1;

  for(s = octx->pr_ctx->n_characters;
      s < (octx->listing_entry->render_width - octx->ld->n_chars_needed_for_line_numbers);
      s++)
  {
   if(pr_send_to_line_buffer(octx->pr_ctx, " ", 1, 1))
    return 1;
  }

  if(pr_disable_attribute(octx->pr_ctx))
   return 1;

  if(pr_advance_line(octx->pr_ctx))
   return 1;

  octx->ld->line++;
  octx->listing_entry->render_height++;
 }

 if(object->line_break)
 {
  if(render_locolisting_line_numbers(octx))
   return 1;
 }

 if(object->n_bytes > 0)
 {
  if(send_to_line_buffer(octx,
                         object->text, object->n_bytes, object->n_chars,
                         object->attribute, fill_attribute))
   return 1;
 }

 if(last_line)
 {
  if(pr_enable_attribute(octx->pr_ctx, "sourcelistingbackground"))
   return 1;

  for(s=octx->pr_ctx->n_characters;
      s < (octx->listing_entry->render_width - octx->ld->n_chars_needed_for_line_numbers);
      s++)
  {
   if(pr_send_to_line_buffer(octx->pr_ctx, " ", 1, 1))
    return 1;
  }

  if(pr_disable_attribute(octx->pr_ctx))
   return 1;

  if(pr_advance_line(octx->pr_ctx))
   return 1;

  octx->ld->line++;
  octx->listing_entry->render_height++;
 }

 return 0;
}

int send_screen_log_object(struct locolisting_operation_context *octx,
                           struct screen_log_object *object,
                           int last_line)
{
 int s;
 char attribute[256], fill_attribute[256];

// fprintf(stderr, "object = [linebreak=%d, n_bytes=%d, n_chars=%d, text='%s', fg=%d bg=%d]\n",
//         object->line_break, object->n_bytes, object->n_chars, object->text,
//         object->foreground, object->background);

 snprintf(attribute, 256, "terminal-NN----");
 snprintf(fill_attribute, 256, "terminal-NN----");

 switch(object->foreground)
 {
  case SCREEN_COLOR_NORMAL:
       attribute[10] = 'N';
       break;

  case SCREEN_COLOR_BLACK:
       attribute[10] = 'K';
       break;

  case SCREEN_COLOR_RED:
       attribute[10] = 'R';
       break;

  case SCREEN_COLOR_GREEN:
       attribute[10] = 'G';
       break;

  case SCREEN_COLOR_YELLOW:
       attribute[10] = 'Y';
       break;

  case SCREEN_COLOR_BLUE:
       attribute[10] = 'B';
       break;

  case SCREEN_COLOR_MAGENTA:
       attribute[10] = 'M';
       break;

  case SCREEN_COLOR_CYAN:
       attribute[10] = 'C';
       break;

  case SCREEN_COLOR_WHITE:
       attribute[10] = 'W';
       break;

  default:
       fprintf(stderr, "BCA: send_screen_log_object() object->foreground = %d\n",
               object->foreground);
       return 1;
 }

 if(object->line_break)
 {
  if(pr_enable_attribute(octx->pr_ctx, fill_attribute))
   return 1;

  for(s = octx->pr_ctx->n_characters; s < octx->listing_entry->render_width; s++)
  {
   if(pr_send_to_line_buffer(octx->pr_ctx, " ", 1, 1))
    return 1;
  }

  if(pr_disable_attribute(octx->pr_ctx))
   return 1;

  if(pr_advance_line(octx->pr_ctx))
   return 1;

  octx->ld->line++;
  octx->listing_entry->render_height++;
 }

 if(send_to_line_buffer(octx, object->text, object->n_bytes, object->n_chars,
                        attribute, fill_attribute))
  return 1;

 if(last_line)
 {
  if(pr_enable_attribute(octx->pr_ctx, fill_attribute))
   return 1;

  for(s = octx->pr_ctx->n_characters; s < octx->listing_entry->render_width; s++)
  {
   if(pr_send_to_line_buffer(octx->pr_ctx, " ", 1, 1))
    return 1;
  }

  if(pr_disable_attribute(octx->pr_ctx))
   return 1;

  if(pr_advance_line(octx->pr_ctx))
   return 1;

  octx->ld->line++;
  octx->listing_entry->render_height++;
 }

 return 0;
}


/* scan lines thar are in the form declarative:data[,arg];
   file_name and source_line_number are for error messages
   declarative, data, and argument are filled inon success
   cursor is the stateful position in the file (contained in
   source) which starts at -1
*/
int next_ll_declarative(struct locolisting_operation_context *octx,
                        int *declarative, int data_size, int argument_size)
{
 int line_length, i, start, end, col, comma, length, p_start, code;

 octx->argument_length = 0;
 octx->data_length = 0;
 *declarative = -1;
 end = octx->cursor;

 while((code = find_line(octx->source, octx->source_length,
                         &start, &end, &line_length)))
 {
  octx->source_line_number++;
  i = start;
  octx->data[0] = 0;
  octx->argument[0] = 0;

  /* skip leading whitespace */
  while(octx->source[i] == ' ')
  {
   i++;
   if(i == end)
    break;
  }

  /* ignore blank lines */
  if(i == end)
   continue;

  break;
 }

 if(code == 0)
  return LL_DONE;

 p_start = i;

 /* look for a ; */
 while(end > i)
 {
  if(octx->source[end] == ';')
   break;
  end--;
 }
 if(end == i)
 {
  fprintf(stderr,
          "BCA: locolisting format file %s line %d: "
          "expected terminating ';' on non-blank line\n",
          octx->file_name, octx->source_line_number);
  return 1;
 }

 col = i;
 /* look for a : */
 while(col < end)
 {
  if(octx->source[col] == ':')
   break;
  col++;
 }

 if(col == end)
 {
  fprintf(stderr,
          "BCA: locolisting format file %s line %d: expected ':' on non-blank line\n",
          octx->file_name, octx->source_line_number);
  return 1;
 }

 /* declarative */
 if(strncmp(octx->source + p_start, "listing", 7) == 0)
 {
  *declarative = LL_LISTING;
 } else if(strncmp(octx->source + p_start, "object", 6) == 0) {
  *declarative = LL_OBJECT;
 } else if(strncmp(octx->source + p_start, "attribute", 9) == 0) {
  *declarative = LL_ATTRIBUTE;
 } else {
  octx->source[col] = 0;
  fprintf(stderr,
          "BCA: locolisting format file %s line %d: expected listing, object, or attribute "
          "instead of \"%s\".\n",
          octx->file_name, octx->source_line_number, octx->source + p_start);
  return 1;
 }

 /* look for a , */
 comma = col + 1;
 while(comma < end)
 {
  if(octx->source[comma] == ',')
   break;
  comma++;
 }
 if(comma == end)
 {
  comma = -1;
  length = end - col - 2;
 } else {
  length = comma - col - 2;
 }
 length++;

 /* data */
 if(length > data_size)
 {
  octx->source[col + 1 + length] = 0;
  fprintf(stderr,
         "BCA: locolisting format file %s line %d: data too long (%d > %d) \"%s\".\n",
         octx->file_name, octx->source_line_number, length, data_size, octx->source + col + 1);
  return 1;
 }

 memcpy(octx->data, octx->source + col + 1, length);
 octx->data[length] = 0;
 octx->data_length = length;

 /* argument */
 if(comma != -1)
 {
  length = end - comma - 1;

  if(length > argument_size)
  {
   octx->source[end] = 0;
   fprintf(stderr,
          "BCA: locolisting format file %s line %d: data too long (%d > %d) \"%s\".\n",
          octx->file_name, octx->source_line_number, length, data_size, octx->source + comma + 1);
   return 1;
  }
  memcpy(octx->argument, octx->source + comma + 1, length);
  octx->argument[length] = 0;

  if((length =
      locolisting_escape_processing(octx, octx->argument, length)) < 0)
   return 1;

  octx->argument_length = length;

  if((octx->argument_n_chars = count_characters(octx->argument, length)) < 0)
   return 1;
 }

 octx->cursor = end;
 return 0;
}

int handle_loco_source_listing_object(struct locolisting_operation_context *octx,
                                      struct source_listing_object *object)
{

 if(strcmp(octx->data, "text") == 0)
 {

  if(octx->first_pass)
  {
   /* all we are trying to do on the first pass is the source dimensions */
   octx->line_width += octx->argument_n_chars;
  } else {
   if(object->text[0] != 0)
    if(send_source_listing_object(octx, object, 0))
     return 1;

   snprintf(object->text, 1024, "%s", octx->argument);
   object->line_break = 0;
  }
  object->attribute[0] = 0;
  object->n_bytes = octx->argument_length;
  object->n_chars = octx->argument_n_chars;
 } else if(strcmp(octx->data, "linebreak") == 0) {
  if(octx->first_pass)
  {
   if(octx->line_width > octx->ld->width)
    octx->ld->width = octx->line_width;

   octx->line_width = octx->argument_n_chars;
   octx->ld->height++;
  } else {

   if(send_source_listing_object(octx, object, 0))
    return 1;

   snprintf(object->text, 1024, "%s", octx->argument);
  }
  object->line_break = 1;
  object->attribute[0] = 0;
  object->n_bytes = octx->argument_length;
  object->n_chars = octx->argument_n_chars;
 }

 return 0;
}

int handle_loco_source_listing_attribute(struct locolisting_operation_context *octx,
                                         struct source_listing_object *object)
{

 if(strcmp(octx->data, "syntaxhighlight") == 0)
 {
  if(object->attribute[0] != 0)
  {
   fprintf(stderr,
           "BCA: loco listing file %s, line %d: source listing object attribute \"syntaxhighlight\" "
           "already set to \"%s\"\n",
           octx->file_name, octx->source_line_number, object->attribute);
   return 1;
  }

  if(octx->argument_length == 0)
  {
   fprintf(stderr,
           "BCA: loco listing file %s, line %d: source listing object attribute \"syntaxhighlight\" "
           "expects an argument\n",
           octx->file_name, octx->source_line_number);
   return 1;
  }

  snprintf(object->attribute, 1024, "%s", octx->argument);
 } else {
  fprintf(stderr,
          "BCA: loco listing file %s, line %d: unknown source listing object attribute \"%s\"\n",
          octx->file_name, octx->source_line_number, octx->data);
  return 1;
 }

 return 0;
}

int handle_loco_screen_log_object(struct locolisting_operation_context *octx,
                                  struct screen_log_object *object)
{
 if(strcmp(octx->data, "text") == 0)
 {

  if(octx->first_pass)
  {
   /* all we are trying to do on the first pass is the source dimensions */
   octx->line_width += octx->argument_n_chars;
  } else {
   if(object->text[0] != 0)
    if(send_screen_log_object(octx, object, 0))
     return 1;

   snprintf(object->text, 1024, "%s", octx->argument);
   object->line_break = 0;
  }
  object->n_bytes = octx->argument_length;
  object->n_chars = octx->argument_n_chars;
 } else if(strcmp(octx->data, "linebreak") == 0) {
  if(octx->first_pass)
  {
   if(octx->line_width > octx->ld->width)
    octx->ld->width = octx->line_width;

   octx->line_width = octx->argument_n_chars;
   octx->ld->height++;
  } else {

   if(send_screen_log_object(octx, object, 0))
    return 1;

   snprintf(object->text, 1024, "%s", octx->argument);
  }
  object->line_break = 1;
  object->n_bytes = octx->argument_length;
  object->n_chars = octx->argument_n_chars;
 }

 return 0;
}

int handle_loco_screen_log_attribute(struct locolisting_operation_context *octx,
                                     struct screen_log_object *object)
{

 if(strcmp(octx->data, "terminalforeground") == 0)
 {
  if((object->foreground = color_to_code(octx, octx->argument)) == -1)
   return 1;

 } else if(strcmp(octx->data, "terminalbackground") == 0) {
  if((object->background = color_to_code(octx, octx->argument)) == -1)

   return 1;

 } else {
  fprintf(stderr,
          "BCA: loco listing file %s, line %d: unknown screen log listing object attribute \"%s\"\n",
          octx->file_name, octx->source_line_number, octx->data);
  return 1;
 }

 return 0;
}

int handle_locolisting(struct document_handling_context *dctx,
                       struct plaintext_engine_context *pe_ctx,
                       struct listing_entry *listing_entry,
                       char *file_name)
{
 int allocation_size, declarative_in_scope, code, declarative, listing_type;
 char data[1024], argument[1024];
 struct locolisting_operation_context op_context, *octx = &op_context;
 struct source_listing_object source_listing_object;
 struct screen_log_object screen_log_object;

 octx->dctx = dctx;
 octx->pe_ctx = pe_ctx;
 octx->pr_ctx = pe_ctx->pr_ctx;
 octx->listing_entry = listing_entry;
 octx->ld = NULL;
 octx->file_name = file_name;
 octx->data = data;
 octx->data_length = 0;
 octx->argument = argument;
 octx->argument_length = 0;
 octx->argument_n_chars = 0;
 octx->line_width = 0;
 octx->source_line_number = 1;
 octx->cursor = -1;
 octx->source = NULL;
 octx->source_length = 0;
 octx->first_pass = 0;

 source_listing_object.attribute[0] = 0;
 source_listing_object.text[0] = 0;
 screen_log_object.text[0] = 0;
 screen_log_object.foreground = -1;
 screen_log_object.background = -1;

 if(listing_entry->listing_type_specific == NULL)
 {
  octx->first_pass = 1;
  allocation_size = sizeof(struct loco_listing);
  if((octx->ld = (struct loco_listing *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
           allocation_size, strerror(errno));
   return 1;
  }
  memset(octx->ld, 0, allocation_size);

  if((octx->ld->file_name = strdup(file_name)) == NULL)
  {
   fprintf(stderr, "BCA: strdup(%s) failed, %s\n",
           file_name, strerror(errno));
   return 1;
  }

  octx->ld->line_numbers_start = 1;

  listing_entry->listing_type_specific = octx->ld;
 } else {
  octx->ld = (struct loco_listing *) listing_entry->listing_type_specific;

  if(strcmp(octx->ld->file_name, file_name) != 0)
  {
   fprintf(stderr,
           "BCA: expected listing cursor locolisting file name to be %s not %s!\n",
           file_name, octx->ld->file_name);
   return 1;
  }

  octx->ld->line = octx->ld->line_numbers_start;
 }

 if((octx->source = read_file(file_name, &(octx->source_length), 0)) == NULL)
  return 1;

 declarative_in_scope = -1;

 if(!octx->first_pass)
 {
  if(render_locolisting_caption(octx))
   return 1;

  if(render_locolisting_line_numbers(octx))
   return 1;
 }

 while((code = next_ll_declarative(octx, &declarative, 1024, 1024)) == 0)
 {
//  fprintf(stderr, "fp = %d line = %d code = %d, declarative = %d, data = '%s', argument = '%s'\n",
//          octx->first_pass, octx->source_line_number, code, declarative, data, argument);

  if(octx->data_length < 1)
  {
   fprintf(stderr,
           "BCA: locolisting format file %s line %d: missing data element.\n",
           file_name, octx->source_line_number);
   return 1;
  }

  switch(declarative)
  {
   case LL_LISTING:
        declarative_in_scope = 1;
        allocation_size = octx->data_length;
        if(octx->first_pass)
        {
         if((octx->ld->caption = (char *) malloc(allocation_size)) == NULL)
         {
          fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
                  allocation_size, strerror(errno));
          return 1;
         }
         memcpy(octx->ld->caption, data, octx->data_length);
         octx->ld->caption[octx->data_length] = 0;
        } else {
         if(strncmp(octx->ld->caption, data, octx->data_length) != 0)
         {
          fprintf(stderr,
                  "BCA: expected listing cursor's locolisting supplied caption to "
                  "be \"%s\" not \"%s\"!\n",
                  octx->ld->caption, data);
          return 1;
         }
        }
        break;

   case LL_OBJECT:
        declarative_in_scope = 2;
        switch(listing_type)
        {
         case SOURCE_CODE:
              if(handle_loco_source_listing_object(octx, &source_listing_object))
               return 1;
              break;

         case SCREEN_LOG:
              if(handle_loco_screen_log_object(octx, &screen_log_object))
               return 1;
              break;
        }
        break;

   case LL_ATTRIBUTE:
        switch(declarative_in_scope)
        {
         case 1:
              if(strncmp(data, "listingtype", octx->data_length) == 0)
              {
               if(strncmp(argument, "sourcecode", octx->argument_length) == 0)
               {
                listing_type = SOURCE_CODE;
               } else if(strncmp(argument, "terminalsession", octx->argument_length) == 0) {
                listing_type = SCREEN_LOG;
               } else {
                fprintf(stderr,
                        "BCA: locolisting format file %s line %d: unknown value \"%s\" "
                        " for listingtype attribute\n",
                        file_name, octx->source_line_number, argument);
                return 1;
               }
              } else if(strncmp(data, "linenumbers", octx->data_length) == 0) {
               if(strncmp(argument, "yes", 3) == 0)
               {
                octx->ld->line_numbers = 1;
               } else if(strncmp(argument, "no", 2) == 0) {
                octx->ld->line_numbers = 0;
               } else {
                fprintf(stderr,
                        "BCA: locolisting format file %s line %d: linenumbers attribute "
                        "expects yes or no, not '%s'\n",
                        file_name, octx->source_line_number, argument);
                return 1;
               }
              } else {
               fprintf(stderr,
                       "BCA: locolisting format file %s line %d: attribute type \"%s\" "
                       "unrecognized for declarative in scope of \"listing\".\n",
                       file_name, octx->source_line_number, data);
               return 1;
              }
              break;

         case 2:
              switch(listing_type)
              {
               case SOURCE_CODE:
                    if(handle_loco_source_listing_attribute(octx, &source_listing_object))
                     return 1;
                    break;

               case SCREEN_LOG:
                    if(handle_loco_screen_log_attribute(octx, &screen_log_object))
                     return 1;
                    break;
              }
              break;
        }
        break;
  }
 }

 free(octx->source);

 if(code != LL_DONE)
  return 1;

 if(octx->first_pass)
 {
  octx->ld->height++;

  if(octx->ld->line_numbers)
  {
   if(octx->ld->line_numbers_start + octx->ld->height > 9999)
   {
    fprintf(stderr, "BCA: locolisting file %s uses greater than 9999 line numbers.\n",
            file_name);
    return 1;
   } else if(octx->ld->line_numbers_start + octx->ld->height > 999) {
    octx->ld->n_chars_needed_for_line_numbers = 4;
   } else if(octx->ld->line_numbers_start + octx->ld->height > 99) {
    octx->ld->n_chars_needed_for_line_numbers = 3;
   } else if(octx->ld->line_numbers_start + octx->ld->height > 9) {
    octx->ld->n_chars_needed_for_line_numbers = 2;
   } else {
    octx->ld->n_chars_needed_for_line_numbers = 1;
   }
   if(octx->pr_ctx->pad_listing_line_numbers)
    octx->ld->n_chars_needed_for_line_numbers++;
  }

  /* now that we know what the caption will be (ie - will it line wrap?),
     we can render it out of place here, just to get the line counts in */
  if(render_locolisting_caption(octx))
   return 1;

 } else {

  if(source_listing_object.text[0] != 0)
  {
   if(send_source_listing_object(octx, &source_listing_object, 1))
    return 1;
  } else if(screen_log_object.text[0] != 0) {
   if(send_screen_log_object(octx, &screen_log_object, 1))
    return 1;
  }

  if(pr_advance_line(pe_ctx->pr_ctx))
   return 1;

 }

 return 0;
}


