/* GPLv3

    Build Configuration Adjust, is a source configuration and Makefile
    generation tool.
    Copyright © 2015 C. Thomas Stover.
    Copyright © 2012,2013,2014 Stover Enterprises, LLC (an Alabama
    Limited Liability Corporation).
    All rights reserved.
    See https://github.com/ctstover/Build-Configuration-Adjust for more
    information.

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

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "prototypes.h"
#endif

int graphviz_string_clean(struct bca_context *ctx,
                          char *input, int length,
                          char *output, int size)
{
 int x, clean;

 if(length == -1)
  length = strlen(input);

 if(length > size)
 {
  fprintf(stderr, "BCA: graphviz_string_clean(): length > size\n");
  return 1;
 }

 for(x=0; x<length; x++)
 {
  clean = 1;

  if(input[x] < 48)
   clean = 0;

  if( (input[x] > 57) && (input[x] < 65) )
   clean = 0;

  if( (input[x] > 90) && (input[x] < 97) )
   clean = 0;

  if(input[x] > 122)
   clean = 0;

  if(clean)
   output[x] = input[x];
  else
   output[x] = '_';
 }
 output[x] = 0;

 return 0;
}

int graphviz_node_color_from_file_extension(struct bca_context *ctx, FILE *output, char *extension)
{
 if(extension == NULL)
  return 0;

 if(strcmp(extension, ".c") == 0)
  fprintf(output, "color = blue");

 if(strcmp(extension, ".h") == 0)
  fprintf(output, "color = blueviolet");

 if(strcmp(extension, ".pc") == 0)
  fprintf(output, "color = palegreen3");

 if(strcmp(extension, ".sh") == 0)
  fprintf(output, "color = yellow1");

 return 0;
}

int graphviz_node_edge_from_component_type(struct bca_context *ctx, FILE *output,
                                           char *type, char *source)
{
 int source_length = 0;

 if(source != NULL)
  source_length = strlen(source);

 if(source_length > 3)
 {
  if(strcmp(source + source_length - 3, ".pc") == 0)
  {
   if( (strcmp(type, "BINARY") == 0) ||
       (strcmp(type, "SHAREDLIBRARY") == 0) )
   {
    fprintf(output, "color = salmon2 style = dashed");
    return 0;
   }
  }
 }

 if(strcmp(type, "BINARY") == 0)
  fprintf(output, "color = red");

 if(strcmp(type, "SHAREDLIBRARY") == 0)
  fprintf(output, "color = red");

 if(strcmp(type, "CAT") == 0)
  fprintf(output, "color = burlywood4");

 if(strcmp(type, "MACROEXPAND") == 0)
  fprintf(output, "color = darkslateblue");

 return 0;
}

int generic_component_nodes(struct bca_context *ctx,
                            struct component_details *cd,
                            struct host_configuration *tc,
                            FILE *output)
{
 char out[512];

 if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
  return 1;

 fprintf(output, " %s [label = \"%s\" shape = component ",
         out, cd->rendered_names[0]);

 if(graphviz_node_color_from_file_extension(ctx, output,
                                            cd->rendered_extensions[0]))
  return 1;

 fprintf(output, "]\n");

 return 0;
}

int library_component_nodes(struct bca_context *ctx,
                            struct component_details *cd,
                            struct host_configuration *tc,
                            FILE *output)
{
 char out[512];
 int i;

 /* first the .pc file gv node */
 if(graphviz_string_clean(ctx, cd->rendered_names[1], -1, out, 512))
  return 1;

 fprintf(output, " %s [label = \"%s\" shape = component ",
         out, cd->rendered_names[1]);

 if(graphviz_node_color_from_file_extension(ctx, output,
                                            cd->rendered_extensions[1]))
  return 1;

 fprintf(output, "]\n");

 /* now the others */
 if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
   return 1;

 fprintf(output, " %s [label = \"", out);

 i = 0;
 while(i<cd->n_rendered_names)
 {
  if( (i != 1) &&
      (cd->rendered_names[i][0] != 0) )
  {
   if(i != 0)
    fprintf(output, "\\n");

   fprintf(output, "%s", cd->rendered_names[i]);
  }
  i++;
 }

 fprintf(output, "\" shape = component ");

 if(graphviz_node_color_from_file_extension(ctx, output,
                                            cd->rendered_extensions[0]))
  return 1;

 fprintf(output, "]\n");
 return 0;
}

int graphviz_mode_component_nodes(struct bca_context *ctx,
                                  struct component_details *cd,
                                  struct host_configuration *tc,
                                  FILE *output)
{
 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {
  if(library_component_nodes(ctx, cd, tc, output))
   return 1;
 } else {
  if(generic_component_nodes(ctx, cd, tc, output))
   return 1;
 }

 return 0;
}

int graphviz_mode_sources_nodes(struct bca_context *ctx,
                                char **source_files,
                                char **source_file_extensions,
                                int n_source_files,
                                FILE *output)
{
 char out[512];
 int source_i;

 for(source_i = 0; source_i < n_source_files; source_i++)
 {
  if(graphviz_string_clean(ctx, source_files[source_i], -1, out, 512))
   return 1;

  fprintf(output, " %s [label = \"%s\" ", out, source_files[source_i]);

  fprintf(output, "shape = note ");

  if(graphviz_node_color_from_file_extension(ctx, output,
                                             source_file_extensions[source_i]))
   return 1;

  fprintf(output, "]\n");
 }

 return 0;
}

int edge_from_internal_dependency(struct bca_context *ctx,
                                  struct host_configuration *tc,
                                  struct component_details *cd,
                                  struct project_details *pd,
                                  int internal_dep,
                                  FILE *output, char *out)
{
 int swapped, i;
 struct component_details dep_cd;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: edge_from_internal_dependency(%s -> %s)\n",
          pd->component_names[internal_dep], cd->component_name);

 memset(&dep_cd, 0, sizeof(struct component_details));

 swapped = -1;
 i=0;
 while(i < ctx->n_swaps)
 {
  if(strcmp(pd->component_names[internal_dep], ctx->swapped_components[i]) == 0)
  {
   swapped = i;
   break;
  }
  i++;
 }

 dep_cd.component_name = pd->component_names[internal_dep];
 dep_cd.component_type = pd->component_types[internal_dep];
 dep_cd.component_output_name = pd->component_output_names[internal_dep];

 if(swapped == -1)
  dep_cd.host = cd->host;
 else
  dep_cd.host = ctx->swapped_component_hosts[swapped];

 if(resolve_component_version(ctx, cd))
  return 1;

 if(render_project_component_output_names(ctx, &dep_cd, RENDER_BUILD_OUTPUT_NAME))
  return 1;

 if(dep_cd.n_rendered_names > 2)
 {
  if(graphviz_string_clean(ctx, dep_cd.rendered_names[1], -1, out, 512))
   return 1;

  fprintf(output, " %s -> ", out);

  if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
   return 1;

  fprintf(output, "%s [", out);

  if(graphviz_node_edge_from_component_type(ctx, output,
                                            cd->component_type,
                                            dep_cd.rendered_names[1]))
   return 1;

  fprintf(output, "]\n");
 }

 if(free_rendered_names(&dep_cd))
  return 1;

 if(component_details_cleanup(cd))
  return 1;

 return 0;
}

int graphviz_mode_component_edges(struct bca_context *ctx,
                                  struct component_details *cd,
                                  struct project_details *pd,
                                  struct host_configuration *tc,
                                  FILE *output)
{
 int i, j, internal_dep;
 char out[512], temp[512];

 for(i=0; i < cd->n_source_files; i++)
 {
  if(graphviz_string_clean(ctx, cd->source_file_names[i], -1, out, 512))
   return 1;

  fprintf(output, " %s -> ", out);

  if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
   return 1;

  fprintf(output, "%s [", out);

  if(graphviz_node_edge_from_component_type(ctx, output, cd->component_type,
                                            cd->source_file_names[i]))
   return 1;

  fprintf(output, "]\n");
 }

 for(i=0; i < cd->n_dependencies; i++)
 {
  internal_dep = -1;
  j = 0;
  while(j < pd->n_components)
  {
   if(strcmp(cd->dependencies[i], pd->component_names[j]) == 0)
   {
    internal_dep = j;
    break;
   }
   j++;
  }

  if(internal_dep > -1)
  {
   if(edge_from_internal_dependency(ctx, tc, cd, pd, internal_dep, output, out))
    return 1;

  } else {
   snprintf(temp, 512, "%s.pc", cd->dependencies[i]);

   if(graphviz_string_clean(ctx, temp, -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
    return 1;

   fprintf(output, "%s [", out);

   if(graphviz_node_edge_from_component_type(ctx, output, cd->component_type, temp))
    return 1;

   fprintf(output, "]\n");
  }
 }

 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {
  if(cd->n_rendered_names > 2)
  {
   if(graphviz_string_clean(ctx, cd->rendered_names[0], -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(graphviz_string_clean(ctx, cd->rendered_names[1], -1, out, 512))
    return 1;

   fprintf(output, "%s [", out);

   if(graphviz_node_edge_from_component_type(ctx, output, "", ""))
    return 1;

   fprintf(output, "]\n");
  }
 }

 return 0;
}

int graphviz_mode_pass(struct bca_context *ctx,
                       struct project_details *pd,
                       struct build_details *bd,
                       FILE *output, int pass)
{
 int host_i, component_i, disabled, swapped, i, yes;
 struct component_details cd;
 struct host_configuration *tc;

 int n_unique_source_files = 0;
 char **unique_source_files = NULL, **unique_source_file_extensions = NULL;

 memset(&cd, 0, sizeof(struct component_details));

 for(host_i = 0; host_i < bd->n_hosts; host_i++)
 {
  if(engage_build_configuration_disables_for_host(ctx, bd->hosts[host_i]))
   return 1;

  if(engage_build_configuration_swaps_for_host(ctx, bd->hosts[host_i]))
   return 1;

  for(component_i = 0; component_i < pd->n_components; component_i++)
  {

   disabled = 0;
   i=0;
   while(i<ctx->n_disables)
   {
    if(strcmp(pd->component_names[component_i], ctx->disabled_components[i]) == 0)
    {
     disabled = 1;
     break;
    }

    i++;
   }

   if(disabled == 0)
   {
    swapped = 0;
    i = 0;
    while(i < ctx->n_swaps)
    {
     if(strcmp(pd->component_names[component_i], ctx->swapped_components[i]) == 0)
     {
      swapped = 1;
      break;
     }
     i++;
    }

    yes = 0;
    if(swapped)
    {
     cd.host = ctx->swapped_component_hosts[i];
     if(pass == 1)
      yes = 1;
    } else {
     cd.host = bd->hosts[host_i];
     yes = 1;
    }

    if(yes)
    {
     cd.component_name = pd->component_names[component_i];
     cd.component_type = pd->component_types[component_i];
     cd.component_output_name = pd->component_output_names[component_i];

     if(component_details_resolve_all(ctx, &cd, pd))
      return 1;

     if((tc = resolve_host_configuration(ctx, cd.host, cd.component_name)) == NULL)
      return 1;

     if(render_project_component_output_names(ctx, &cd, RENDER_BUILD_OUTPUT_NAME))
     {
      fprintf(stderr, "BCA: render_project_componet_output_names() failed\n");
      return 1;
     }

     switch(pass)
     {
      case 0:
           if(graphviz_mode_component_nodes(ctx, &cd, tc, output))
            return 1;

           /* the same source files may be used in multiple components; plus multiple
              hosts may be involved; thus for nodes - first assemble a single unique
              list then render each only once below */
           for(i=0; i < cd.n_source_files; i++)
           {
            switch(add_to_string_array(&unique_source_files, n_unique_source_files,
                                       cd.source_file_names[i], -1, 1))
            {
             case -1:
                  return 1;
                  break;

             case 0:
                  if(add_to_string_array(&unique_source_file_extensions, n_unique_source_files,
                                         cd.source_file_extensions[i], -1, 0))
                   return 1;
                  n_unique_source_files++;
                  break;
            }
           }
           //add extra_file_deps to this?
           break;

      case 1:
           if(graphviz_mode_component_edges(ctx, &cd, pd, tc, output))
            return 1;
           break;
     }

     if(free_rendered_names(&cd))
      return 1;

     if(free_host_configuration(ctx, tc))
      return 1;

     if(component_details_cleanup(&cd))
      return 1;
    }
   }
  }
 }

 if(n_unique_source_files > 0)
 {
  if(graphviz_mode_sources_nodes(ctx,
                                 unique_source_files,
                                 unique_source_file_extensions,
                                 n_unique_source_files,
                                 output))
   return 1;

  free_string_array(unique_source_files, n_unique_source_files);
  free_string_array(unique_source_file_extensions, n_unique_source_files);
 }

 return 0;
}

int generate_graphviz_mode(struct bca_context *ctx)
{
 struct project_details pd;
 struct build_details bd;
 FILE *output;
 int pass;

 memset(&pd, 0, sizeof(struct project_details));
 memset(&bd, 0, sizeof(struct build_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: graphviz_output_mode()\n");

 if(load_project_config(ctx, 0))
  return 1;

 if(load_build_config(ctx, 0))
  return 1;

 if(list_build_hosts(ctx, &bd))
 {
  fprintf(stderr, "BCA: list_build_hosts() failed\n");
  return 1;
 }

 if(list_project_components(ctx, &pd))
 {
  fprintf(stderr, "BCA: list_project_components() failed\n");
  return 1;
 }

 if(check_duplicate_output_names(ctx, &pd))
 {
  fprintf(stderr, "BCA: check_duplicate_output_names() failed\n");
  return 1;
 }

 if((output = fopen("bcaproject.dot", "w")) == NULL)
 {
  perror("BCA: fopen(\"bcaproject.dot\")");
  return 1;
 }

 fprintf(output,
         "/* This dot file for graphviz was generated by Build Configuration Adjust\n"
         " * See http://bca.stoverenterprises.com for more information\n"
         " */\n\n");

 fprintf(output,
         "digraph G {\n"
         " rankdir=LR;\n");

 for(pass=0; pass<2; pass++)
 {
  fprintf(output, "\n");

  if(graphviz_mode_pass(ctx, &pd, &bd, output, pass))
   return 1;
 }

 fprintf(output, "}\n");
 fclose(output);

 free_string_array(pd.component_names, pd.n_components);
 free_string_array(pd.component_types, pd.n_components);
 free_string_array(pd.component_output_names, pd.n_components);
 free_string_array(bd.hosts, bd.n_hosts);

 return 0;
}

