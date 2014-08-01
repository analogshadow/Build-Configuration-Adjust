#ifndef _plaintext_h_
#define _plaintext_h_

#include "prototypes.h"

struct plaintext_engine_context;

struct plaintext_rendering_context
{
 int line_width, left_margin_width, right_margin_width;
 int current_column, current_row, current_page;
 int page_length, top_margin, bottom_margin;
 FILE *output;
 struct plaintext_engine_context *pe_ctx;
};

struct plaintext_engine_context
{
 struct document_handling_context *dctx;
 struct unicode_word_context *uwc;

 struct plaintext_rendering_context *effective_rendering_context;

 int paragraph_indent, paragraph_line_spacing;

 int show_toc;
 struct toc_element *toc_root, *toc_cursor, *index_head, *index_tail;
};


/* render context */
struct plaintext_rendering_context *
plaintext_rendering_context_new(struct plaintext_engine_context *pe_ctx,
                                FILE *output);

struct plaintext_rendering_context *
plaintext_rendering_context_copy(struct plaintext_rendering_context *source);

int plaintext_rendering_context_finalize(struct plaintext_rendering_context *pr_ctx);

int pr_advance_line(struct plaintext_rendering_context *pr_ctx);

int pr_advance_page(struct plaintext_rendering_context *pr_ctx);

int pr_advance_column(struct plaintext_rendering_context *pr_ctx, int n_cols);

int pr_toc_cursor_advance(struct plaintext_engine_context *pe_ctx);

int pr_advance_word(struct plaintext_rendering_context *pr_ctx,
                    struct unicode_word_context *uwc);

int pr_right_justified_page_output(struct plaintext_rendering_context *pr_ctx,
                                   char *temp, int temp_length);

/* toc data structures */
int plaintext_add_toc_element(struct plaintext_engine_context *pe_ctx,
                              int type, char *name);



#endif
