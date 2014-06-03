#include "prototypes.h"

struct plaintext_engine_context
{
 int max_line_width;
 int current_line_width;
};

int plaintext_start_document(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 int allocation_size;

 if(dctx->render_engine_context == NULL)
 {
  allocation_size = sizeof(struct plaintext_engine_context);
  if((ectx = (struct plaintext_engine_context *)
             malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
           allocation_size, strerror(errno));
   return 1;
  }
  dctx->render_engine_context = ectx;
 }

 return 0;
}

int plaintext_finish_document(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_consume_text(struct document_handling_context *dctx,
                           char *text, int length)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 printf("%s", text);

 return 0;
}

int plaintext_open_point(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_point(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_list(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_list(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_listing(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_listing(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_inset(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_inset(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_subsection(struct document_handling_context *dctx,
                              char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_subsection(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_section(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_section(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_chapter(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_chapter(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_part(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_part(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_table(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_table(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tr(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tr(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tc(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tc(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_open_tag(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

 return 0;
}

int plaintext_close_tag(struct document_handling_context *dctx)
{
 struct plaintext_engine_context *ectx;
 ectx = (struct plaintext_engine_context *) dctx->render_engine_context;

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
 dctx->close_tag = plaintext_open_tag;

 return 0;
}
