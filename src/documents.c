/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2014 Stover Enterprises, LLC
    (an Alabama Limited Liability Corporation), All rights reserved.
    See http://bca.stoverenterprises.com for more information.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "prototypes.h"

extern int line_number;

struct document_handling_context
{
 struct bca_context *ctx;
 int close_stack_depth;
 int (*close_functions_stack[32]) (struct document_handling_context *, void *);
 void *close_functions_data[32];
};

int push_close_function(struct document_handling_context *dctx,
                        int (*close_function) (struct document_handling_context *,
                                               void *),
                        void *data)
{
 int i;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: push_close_function()\n");

 if((i = dctx->close_stack_depth) > 31)
 {
  fprintf(stderr, "BCA: document stack too deep\n");
  return 1;
 }

 dctx->close_functions_data[i] = data;
 dctx->close_functions_stack[i] = close_function;
 dctx->close_stack_depth++;

 return 0;
}

int function_close(struct document_handling_context *dctx)
{
 int i;

 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_close()\n");

 if((i = dctx->close_stack_depth) == 0)
 {
  fprintf(stderr, "BCA: @dc()@ out of place, line %d\n", line_number);
  return 1;
 }

 if(dctx->close_functions_stack[i-1](dctx, dctx->close_functions_data[i-1]))
 {
  return 1;
 }

 dctx->close_stack_depth--;
 return 0;
}

int function_close_point(struct document_handling_context *dctx, void *data)
{
 fprintf(stdout, "</li>");
 return 0;
}

int function_dpoint(struct document_handling_context *dctx,
                    char **parameters, int n_parameters)
{
 if(dctx->ctx->verbose > 2)
  fprintf(stderr, "BCA: function_dpoint()\n");

 if(n_parameters != 1)
  return 1;

 fprintf(stdout, "<li>");

 return push_close_function(dctx, function_close_point, NULL);
}

char *handle_document_functions(struct bca_context *ctx, char *key)
{
 char **parameters;
 int n_parameters, allocation_size, code = -5;
 struct document_handling_context *dctx;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: handle_document_functions()\n");

 if(parse_function_parameters(key, &parameters, &n_parameters))
  return NULL;

 if((dctx = ctx->dctx) == NULL)
 {
  allocation_size = sizeof(struct document_handling_context);
  if((dctx = (struct document_handling_context *)
             malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
   return NULL;
  }
  memset(dctx, 0, allocation_size);
  dctx->ctx = ctx;
  ctx->dctx = dctx;
 }


 if(strncmp(parameters[0] + 1, "point", 5) == 0)
  code = function_dpoint(dctx, parameters, n_parameters);

 if(strncmp(parameters[0] + 1, "c", 1) == 0)
  code = function_close(dctx);

 if(code == -5)
 {
  fprintf(stderr,
         "BCA: handle_document_functions(): I can't handle function: %s, line %d\n",
         parameters[0], line_number);
  return NULL;
 }

 if(code == 0)
  return strdup("");

 return NULL;
}




