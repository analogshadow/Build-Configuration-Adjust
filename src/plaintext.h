#ifndef _plaintext_h_
#define _plaintext_h_

#include "prototypes.h"

struct plaintext_engine_context;

#define PER_LEFT_JUSTIFY   2
#define PER_CENTER_JUSTIFY 3
#define PER_RIGHT_JUSTIFY  4
#define PER_LEFT_TO_RIGHT  10
#define PER_RIGHT_TO_LEFT  11
#define PER_LINE_BUFFER_SIZE 2048
#define MAX_INDEX_TERM_SIZE 256

#define OUTPUT_MODE_TEXT_FILE 800
#define OUTPUT_MODE_HTML_FILE 801

struct plaintext_footnote
{
 char *buffer;
 int buffer_length;
 int n_lines;
 int number;
 struct plaintext_footnote *next;
};

struct plaintext_index_entry
{
 char *term;
 char *page;
 struct plaintext_index_entry *next_term, *next_page;
};

#define LISTING_TYPE_LOCOLISTING 500

struct listing_entry
{
 int listing_number, listing_type;

 /* width and height as it will apear in the rendering context used */
 int render_width, render_height;

 /* caption given by the listing parameters */
 char *caption;

 void *listing_type_specific;
 struct listing_entry *next;
};

struct loco_listing
{
 /* width and height as it appears in the listing */
 int width, height;

 /* whether or not to use line numbers, which line the listing starts at,
    and a line counter used by the rendering */
 int line_numbers, line_numbers_start, line,
     n_chars_needed_for_line_numbers;

 /* used for assertions in multi-pass processing */
 char *file_name;

 /* caption given in locolisting file */
 char *caption;
};


struct plaintext_rendering_context
{
 int line_width, left_margin_width, right_margin_width;
 int current_col, current_row, current_page;
 int page_length, top_margin, bottom_margin;
 int justification, direction, show_page_numbers;
 int pad_listing_line_numbers;
 FILE *output;
 int output_mode;
 struct plaintext_engine_context *pe_ctx;
 int n_bytes, n_characters;
 char line_buffer[PER_LINE_BUFFER_SIZE];

 char *output_buffer;
 int output_buffer_length, output_buffer_size, output_buffer_lines;
};

struct plaintext_engine_context
{
 /* convenience circular reference */
 struct document_handling_context *dctx;

 /* word engine */
 struct unicode_word_context *uwc, *uwc_stack[16];

 /* hyphenation engine */
 struct hyphenation_context *hc;
 struct unicode_word_context first, second;

 /* rendering engine */
 struct plaintext_rendering_context *pr_ctx, *pr_ctx_stack[16];
 int rendering_context_stack_depth, word_engine_stack_depth;

 /* non-rendering context specific preferences */
 int paragraph_indent, paragraph_line_spacing;

 /* table of contents */
 int show_toc;
 struct toc_element *toc_root, *toc_cursor, *index_head, *index_tail;

 /* footnotes */
 int n_footnotes;
 struct plaintext_footnote *footnotes_head, *footnotes_tail;
 struct plaintext_rendering_context *footnote_pr;

 /* index */
 int show_index;
 char *index_term_buffer;
 int index_term_buffer_length;
 struct plaintext_index_entry *index;

 /* listings */
 struct listing_entry *listings_head, *listings_cursor;

 /* lefthand righthand page logic */
 int even_or_odd_page;
};

int pr_feed_generated_words(struct plaintext_engine_context *pe_ctx, char *buffer);

int plaintext_consume_text(struct document_handling_context *dctx,
                           char *text, int length);

int plaintext_rendering_stack_push(struct plaintext_engine_context *pe_ctx);

int plaintext_rendering_stack_pop(struct plaintext_engine_context *pe_ctx);

int plaintext_word_engine_stack_push(struct plaintext_engine_context *pe_ctx);

int plaintext_word_engine_stack_pop(struct plaintext_engine_context *pe_ctx);

int plaintext_paragraph_open(struct plaintext_engine_context *pe_ctx);

int plaintext_paragraph_close(struct plaintext_engine_context *pe_ctx);

int plaintext_footnote_open(struct plaintext_engine_context *pe_ctx);

int plaintext_footnote_close(struct plaintext_engine_context *pe_ctx);

int plaintext_index_open(struct plaintext_engine_context *pe_ctx);

int plaintext_index_close(struct plaintext_engine_context *pe_ctx);

int pe_print_toc(struct plaintext_engine_context *pe_ctx);

int pe_print_index(struct plaintext_engine_context *pe_ctx);

int pe_toc_cursor_advance(struct plaintext_engine_context *pe_ctx);

/* render context */
struct plaintext_rendering_context *
plaintext_rendering_context_new(struct plaintext_engine_context *pe_ctx,
                                FILE *output);

struct plaintext_rendering_context *
plaintext_rendering_context_copy(struct plaintext_rendering_context *source);

int plaintext_rendering_context_finalize(struct plaintext_rendering_context *pr_ctx);

/* input files feed bytes into a word engine, which callback into pe_consume_word() */
int pe_consume_word(struct unicode_word_context *uwc, void *data, int flags);

/* pe_consume_word() then calls pr_advance_word() */
int pr_advance_word(struct plaintext_rendering_context *pr_ctx,
                    struct unicode_word_context *uwc);

/* or generated text feeds words in a word engine with this */
int pr_feed_generated_words(struct plaintext_engine_context *pe_ctx, char *buffer);

/* call pr_flush_line_buffer() and move to the next line */
int pr_advance_line(struct plaintext_rendering_context *pr_ctx);

/* without necessarily flushing the line, move in the render context by the amount
   already in the line buffer. current hack for using multiple render contexts
   on the same line */
int pr_advance_buffer(struct plaintext_rendering_context *pr_ctx);

/* lowest level internal functions */
int pr_send_to_line_buffer(struct plaintext_rendering_context *pr_ctx,
                           char *buffer, int n_bytes, int n_characters);

int pr_direct_to_line_buffer(struct plaintext_rendering_context *pr_ctx,
                             char *buffer, int n_bytes, int n_characters);

/*     Output is line oriented, meaning is collected in the "line buffer" (which
   tracks the number of bytes and the number of charactrs separately), then sent to
   output with the current justification setting (left/right/center). At the lowest
   level, pe_send_to_line_buffer() and pe_direct_to_line_buffer()  append bytes
   into the line buffer for overflows on the number of bytes. The difference between
   the two functions is pe_send_to_line_buffer() also performs escaping characters
   required for the output type (example: html needs > changed to &gt; ). This level
   is only used internally, with functions such as as attribute realization needing
   un-escapped output.

      One level up is another internal function pr_flush_line_buffer(), which
  actually does the justified output. It will fail with error if can not fit the
  number of characters in the line buffer on the current rendering context line, as
  this should have been prevented by the higher level functions such as
  pr_advance_line().

     Bytes are feed into the word engine either by input files, or by generated
  output. The word engine verifies UTF-8 sequencing, and finds word bounderies. The
  words are delivered by callback to pe_consume_word() which then calls
  pr_advance_word(). pr_advance_word() in turn calls pr_advance_line() as needed.
  Also pr_advance_line() is called just move to the next effective line.

*/

/* move to the next page, unless current page already blank */
int pr_advance_page(struct plaintext_rendering_context *pr_ctx);

int pr_enable_attribute(struct plaintext_rendering_context *pr_ctx, char *attribute);

int pr_disable_attribute(struct plaintext_rendering_context *pr_ctx);

int pr_ensure_minimum_rows_left(struct plaintext_rendering_context *pr_ctx,
                                int minimum);

int pr_center_row(struct plaintext_rendering_context *pr_ctx);

int pr_third_way_down(struct plaintext_rendering_context *pr_ctx);

int pr_render_foot_notes(struct plaintext_engine_context *pe_ctx, int limit);

int pr_toc_cursor_advance(struct plaintext_engine_context *pe_ctx);


/* toc data structures */
int plaintext_add_toc_element(struct plaintext_engine_context *pe_ctx,
                              int type, char *name);

int superscript_number(char *number, int length, char **superscript_ptr, int *out_length);

/* locolisting.c */
int handle_locolisting(struct document_handling_context *dctx,
                       struct plaintext_engine_context *pe_ctx,
                       struct listing_entry *listing_entry,
                       char *file_name);

#endif
