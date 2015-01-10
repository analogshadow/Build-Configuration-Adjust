#include "plaintext.h"

struct passthrough_engine_context
{
 /* convenience circular reference */
 struct document_handling_context *dctx;
 FILE *output;
 int output_mode;
};

int passthrough_consume_text(struct document_handling_context *dctx,
                          char *text, int length)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "%s", text); //needs to escape
       break;
 }

 return 0;
}

int passthrough_open_point(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<li>");
       break;
 }

 return 0;
}

int passthrough_close_point(struct document_handling_context *dctx)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "</li>");
       break;
 }

 return 0;
}

int passthrough_open_list(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<ul>");
       break;
 }

 return 0;
}

int passthrough_close_list(struct document_handling_context *dctx)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "</ul>");
       break;
 }

 return 0;
}

int passthrough_open_listing(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 return 0;
}

int passthrough_close_listing(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_inset(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_close_inset(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_subsection(struct document_handling_context *dctx,
                              char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 char *subsection_name = NULL;

 if(n_parameters == 2)
  subsection_name = parameters[1];

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<h3>Subsection: %s</h3>", subsection_name);
       break;
 }

 return 0;
}

int passthrough_close_subsection(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_section(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 char *section_name = NULL;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  section_name = parameters[1];

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<h2>Section: %s</h2>", section_name);
       break;
 }

 return 0;
}

int passthrough_close_section(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_chapter(struct document_handling_context *dctx,
                           char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 char *chapter_name = NULL;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  chapter_name = parameters[1];

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<h1>Chapter: %s</h1>", chapter_name);
       break;
 }

 return 0;
}

int passthrough_close_chapter(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;


 return 0;
}

int passthrough_open_part(struct document_handling_context *dctx,
                        char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 char *part_name = NULL;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 if(n_parameters == 2)
  part_name = parameters[1];

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output,
               "<br>\n<br>\n"
               "<h1>Part: %s</h1>\n"
               "<br>\n<br>\n",
               part_name);
       break;
 }

 return 0;
}

int passthrough_close_part(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_table(struct document_handling_context *dctx,
                         char **parameters, int n_parameters)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_close_table(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_tr(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_close_tr(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_tc(struct document_handling_context *dctx,
                      char **parameters, int n_parameters)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_close_tc(struct document_handling_context *dctx)
{
// struct passthrough_engine_context *pt_ctx;
// pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 return 0;
}

int passthrough_open_tag(struct document_handling_context *dctx,
                       char **parameters, int n_parameters)
{
 struct passthrough_engine_context *pt_ctx;
 char *tag_name;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "<p>");
       break;
 }

 if(strcmp(tag_name, "f") == 0)
  return 0;

 if(strcmp(tag_name, "i") == 0)
  return 0;

 return 0;
}

int passthrough_close_tag(struct document_handling_context *dctx)
{
 struct passthrough_engine_context *pt_ctx;
 char *tag_name;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;
 tag_name = dctx->tags[dctx->tag_depth - 1];

 if(strcmp(tag_name, "p") == 0)
 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output, "</p>");
       break;
 }

 if(strcmp(tag_name, "f") == 0)
  return 0;

 if(strcmp(tag_name, "i") == 0)
  return 0;

 return 0;
}

int passthrough_start_document(struct document_handling_context *dctx)
{
 struct passthrough_engine_context *pt_ctx;

 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output,
               "<html>\n"
               " <head>\n"
               "  <meta charset=\"UTF-8\">\n"
               "  <link rel=\"stylesheet\" type=\"text/css\" href=\"bcadoc.css\">\n"
               " </head>\n"
               " <body>\n");
       break;
 }

 return 0;
}

int passthrough_finish_document(struct document_handling_context *dctx)
{
 struct passthrough_engine_context *pt_ctx;
 pt_ctx = (struct passthrough_engine_context *) dctx->document_engine_context;

 switch(pt_ctx->output_mode)
 {
  case OUTPUT_MODE_HTML_FILE:
       fprintf(pt_ctx->output,
               " </body>\n"
               "</html>\n");
       break;
 }

 return 0;
}


int activate_document_engine_passthrough(struct document_handling_context *dctx)
{
 int allocation_size;
 struct passthrough_engine_context *pt_ctx;

 dctx->start_document = passthrough_start_document;
 dctx->finish_document = passthrough_finish_document;
 dctx->consume_text = passthrough_consume_text;
 dctx->open_point = passthrough_open_point;
 dctx->close_point = passthrough_close_point;
 dctx->open_list = passthrough_open_list;
 dctx->close_list = passthrough_close_list;
 dctx->open_listing = passthrough_open_listing;
 dctx->close_listing = passthrough_close_listing;
 dctx->open_inset = passthrough_open_inset;
 dctx->close_inset = passthrough_close_inset;
 dctx->open_subsection = passthrough_open_subsection;
 dctx->close_subsection = passthrough_close_subsection;
 dctx->open_section = passthrough_open_section;
 dctx->close_section = passthrough_close_section;
 dctx->open_chapter = passthrough_open_chapter;
 dctx->close_chapter = passthrough_close_chapter;
 dctx->open_part = passthrough_open_part;
 dctx->close_part = passthrough_close_part;
 dctx->open_table = passthrough_open_table;
 dctx->close_table = passthrough_close_table;
 dctx->open_tr = passthrough_open_tr;
 dctx->close_tr = passthrough_close_tr;
 dctx->open_tc = passthrough_open_tc;
 dctx->close_tc = passthrough_close_tc;
 dctx->open_tag = passthrough_open_tag;
 dctx->close_tag = passthrough_close_tag;

 if(dctx->document_engine_context != NULL)
 {
  fprintf(stderr,
          "BCA: activate_document_engine_passthrough(): render engine should not be initialized.\n");
  return 1;
 }

 allocation_size = sizeof(struct passthrough_engine_context);
 if((pt_ctx = (struct passthrough_engine_context *)
              malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          allocation_size, strerror(errno));
  return 1;
 }

 memset(pt_ctx, 0, allocation_size);
 pt_ctx->dctx = dctx;
 pt_ctx->output = stdout;

 if(strcmp(dctx->ctx->output_type, "html") == 0)
 {

  pt_ctx->output_mode = OUTPUT_MODE_HTML_FILE;

 } else {
  fprintf(stderr,
          "BCA: passthrough engine - unknown output type \"%s\", try html\n",
          dctx->ctx->output_type);
  return 1;
 }

 dctx->document_engine_context = pt_ctx;
 return 0;
}

