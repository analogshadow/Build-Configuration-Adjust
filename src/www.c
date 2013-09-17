/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2011,2012,2013 Stover Enterprises, LLC
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

#ifndef WITHOUT_MONGOOSE

#include <stdio.h>
#include <string.h>
#include "prototypes.h"
#include "mongoose.h"

void mgi_project_mode(struct mg_connection *conn,
                      const struct mg_request_info *request_info,
                      struct bca_context *bca_ctx)
{
 int i;
 struct component_details cd;

 if(list_project_components(bca_ctx, &cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed\n");
  return;
 }

   mg_printf(conn, 
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<html>\n"
             " <head>\n"
             "  <title>%s - BCA Mongoose Interface</title>\n", 
             bca_ctx->project_name);

   mg_printf(conn,
             " </head>\n"
             " <body>\n"
             "  <table align=center width=\"90%%\" height=\"90%%\">\n"
             "   <tr>\n"
             "    <td valign=top>\n"
             "     <h1>%s</h1>\n",
             bca_ctx->project_name);

   mg_printf(conn,
             "     <h3>Project Manipulation</h3>\n"
             "     <hr width=90%%>\n"
             "     <table border=1 cellspacing=2>\n"
             "<tr><td><b>Component Name</b></td><td><b>Component Type</b></td></tr>\n");


 for(i=0; i < cd.n_components; i++)
 {
  mg_printf(conn, 
            " <tr>\n"
            "  <td>%s</td>\n" 
            "  <td>%s</td>\n"
            " </tr>\n",
            cd.project_components[i],
            cd.project_component_types[i]); 
 }

  mg_printf(conn, " </table>\n");


   mg_printf(conn,
             "    </td>\n"
             "   </tr>\n"
             "   <tr>\n"
             "    <td align=right>\n"
             "     <p>BCA version x. Compiled on y.</p>\n"
             "    </td>\n"
             "   </tr>\n"
             "  </table>\n"
             " </body>\n"
             "</html>\n");




 free_string_array(cd.project_components, cd.n_components);
 free_string_array(cd.project_component_types, cd.n_components);
}

static void *callback(enum mg_event event,
                      struct mg_connection *conn,
                      const struct mg_request_info *request_info)
{
 int handled;
 struct bca_context *bca_ctx;

 if (event == MG_NEW_REQUEST)
 {
  handled = 0;

  bca_ctx = (struct bca_context *) request_info->user_data;

  if(strcmp(request_info->uri, "/") == 0)
  {
   mg_printf(conn, 
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<html>\n"
             " <head>\n"
             "  <title>%s - BCA Mongoose Interface</title>\n", 
             bca_ctx->project_name);

   mg_printf(conn,
             " </head>\n"
             " <body>\n"
             "  <table align=center width=\"90%%\" height=\"90%%\">\n"
             "   <tr>\n"
             "    <td valign=top>\n"
             "     <h1>%s</h1>\n",
             bca_ctx->project_name);

   mg_printf(conn,
             "     <h3><a href="">Build Configuration Adjust</a> Mongoose Interface</h3>\n"
             "     <hr width=90%%>\n"
             "     <h2>Actions</h2>\n"
             "     <ul>\n"
             "      <li><a href=\"./project\">Manipulate project</a></li>\n"
             "      <li><a href=\"\">Configure project for build</a></li>\n"
             "      <li><a href=\"\">Build project</a></li>\n"
             "      <li>Other</li>\n"
             "      <ul>\n"
             "       <li><a href=\"\">Graphviz Plots</a></li>\n"
             "      </ul>\n"
             "     </ul>\n"
             "    </td>\n"
             "   </tr>\n"
             "   <tr>\n"
             "    <td align=right>\n"
             "     <p>BCA version x. Compiled on y.</p>\n"
             "    </td>\n"
             "   </tr>\n"
             "  </table>\n"
             " </body>\n"
             "</html>\n");

   handled = 1;
  }

  if(strncmp(request_info->uri, "/project", 8) == 0)
  {
   mgi_project_mode(conn, request_info, bca_ctx);
   handled = 1;
  }

  if(handled == 0)
  {
   mg_printf(conn, 
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<html><h1>BCA: Unknown URI \"%s\"</h1></html>\n", request_info->uri);
  }

  return "";

 } else {
  return NULL;
 }
}

int mongoose_interface(struct bca_context *bca_ctx)
{
 struct mg_context *mongoose_ctx;
 const char *options[] = {"listening_ports", "127.0.0.1:8080", NULL};

 if(bca_ctx->verbose > 2)
  fprintf(stderr, "BCA: mongoose_interface()\n");

 bca_ctx->build_configuration_contents = 
     read_file("./buildconfiguration/buildconfiguration", 
               &(bca_ctx->build_configuration_length), 0);

 if((bca_ctx->project_configuration_contents = 
      read_file("./buildconfiguration/projectconfiguration", 
                &(bca_ctx->project_configuration_length), 0)) == NULL)
 {
  return 1;
 }

 resolve_project_name(bca_ctx);

 mongoose_ctx = mg_start(&callback, bca_ctx, options);

 fprintf(stderr, "BCA: mongoose http interface should be active on 127.0.0.1:8080\n");
 fprintf(stderr, "BCA: press any key to stop.\n");
 getchar();  // Wait until user hits "enter"
 mg_stop(mongoose_ctx);
 return 0;
}

#endif

