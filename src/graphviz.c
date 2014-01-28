/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012-2014 Stover Enterprises, LLC
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
 if(strcmp(extension, "c") == 0)
  fprintf(output, "color = blue");
  
 if(strcmp(extension, "h") == 0)
  fprintf(output, "color = blueviolet");

 if(strcmp(extension, "pc") == 0)
  fprintf(output, "color = palegreen3");

 return 0;
}

int graphviz_node_color_from_component_type(struct bca_context *ctx, FILE *output, char *type)
{
 if(strcmp(type, "BINARY") == 0)
  fprintf(output, "color = red3");
  
 if(strcmp(type, "SHAREDLIBRARY") == 0)
  fprintf(output, "color = maroon");


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

int graphviz_nodes(struct bca_context *ctx, FILE *output, 
                   char **hosts, int n_build_hosts,
                   struct component_details *cd)
{
 int x, y, z, i, yes, code, n_sources = 0, n_file_deps, n_ext_depends = 0, n_items, 
     disabled, swapped; 
 struct host_configuration *tc;
 char in[512], out[512], *extension, **sources = NULL, **extensions = NULL, 
      **ext_depends = NULL, **file_deps, *value, **list = NULL;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: graphviz_nodes()\n");

 for(x=0; x<n_build_hosts; x++)
 {
  for(y=0; y < cd->n_components; y++)
  {
   cd->project_component = cd->project_components[y];
   cd->project_component_type = cd->project_component_types[y];
   cd->host = hosts[x];

   if((tc = resolve_host_configuration(ctx, cd)) == NULL)
    return 1;

   if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
    return 1;
 
   if(engage_build_configuration_swaps_for_host(ctx, hosts[x]))
    return 1;

   disabled = 0;
   i=0; 
   while(i<ctx->n_disables)
   {
    if(strcmp(cd->project_components[y], ctx->disabled_components[i]) == 0)
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
     if(strcmp(cd->project_components[y], ctx->swapped_components[i]) == 0)
     {
      swapped = 1;
      break;
     }
     i++;
    }

    if(swapped)
    {
     if((n_items = render_project_component_output_name(ctx, 
                                                        ctx->swapped_component_hosts[i], 
                                                        cd->project_component, 2,
                                                        &list, &extensions)) < 0)
     {
      fprintf(stderr, "BCA: render_project_componet_output_names() failed\n");
      return 1;
     }
    } else {
     if((n_items = render_project_component_output_name(ctx, cd->host, 
                                                        cd->project_component, 2,
                                                        &list, &extensions)) < 0)
     {
      fprintf(stderr, "BCA: render_project_componet_output_names() failed\n");
      return 1;
     }
    }

    for(i=0; i<n_items; i++)
    {
     if(list[i][0] != 0)
     {
      if(graphviz_string_clean(ctx, list[i], -1, out, 512))
       return 1;

      fprintf(output, " %s [label = \"%s\" shape = component ", out, list[i]);

      if(graphviz_node_color_from_file_extension(ctx, output, extensions[i]))
       return 1;

      fprintf(output, "]\n");
     }
    }
 
    free_string_array(extensions, n_items);
    free_string_array(list, n_items);
    n_items = 0;
    list = NULL;
    extensions = NULL;

    /* build unique list of external dependencies */
    if(resolve_component_dependencies(ctx, cd))
     return 1;

    for(z = 0; z < cd->n_dependencies; z++)
    {
     i = 0;
     yes = 1;
     while(i < cd->n_components)
     {
      if(strcmp(cd->dependencies[z], cd->project_components[i]) == 0)
      {
       yes = 0;
       break;
      }
      i++;
     }

     if(yes)
     {
      code = add_to_string_array(&ext_depends, n_ext_depends, cd->dependencies[z], -1, 1);

      if(code == -1)
       return 1;
 
      if(code == 0)
       n_ext_depends++;
     }
    }

    free_string_array(cd->dependencies, cd->n_dependencies);
    cd->dependencies = NULL;
    cd->n_dependencies = 0;
   }

   free_host_configuration(ctx, tc);
  }
 }

 fprintf(output, "\n");

 /* add external depends nodes */
 for(i=0; i < n_ext_depends; i++)
 {
  snprintf(in, 512, "%s.pc", ext_depends[i]);

  if(graphviz_string_clean(ctx, in, -1, out, 512))
   return 1;

  fprintf(output, " %s [ label = \"%s\" ", out, in);
  fprintf(output, "shape = box3d ");
  if(graphviz_node_color_from_file_extension(ctx, output, "pc"))
   return 1;

  fprintf(output, "]\n");
 }
 free_string_array(ext_depends, n_ext_depends);
 

 /* build unique lists of sources */
 for(y=0; y < cd->n_components; y++)
 {
  disabled = 0;
  i=0; 
  while(i<ctx->n_disables)
  {
   if(strcmp(cd->project_components[y], ctx->disabled_components[i]) == 0)
   {
    disabled = 1;
    break;
   }

   i++;
  }

  if(disabled == 0)
  {
   cd->project_component_type = cd->project_component_types[y];
   cd->project_component = cd->project_components[y];

   if(resolve_component_file_dependencies(ctx, cd, y))
    return 1;

   for(x=0; x < cd->n_file_names; x++)
   {

    code = add_to_string_array(&sources, n_sources, cd->file_names[x], -1, 1);

    if(code == -1)
     return 1;
 
    if(code == 0)
    {
     if(add_to_string_array(&extensions, n_sources, cd->file_extensions[x], -1, 0) != 0)
      return 1;
     n_sources++;
    }

   }

   if(cd->n_file_names > 0)
   {
    free_string_array(cd->file_names, cd->n_file_names);
    free_string_array(cd->file_base_names, cd->n_file_names);
    free_string_array(cd->file_extensions, cd->n_file_names);
   }

   cd->n_file_names = 0;
   cd->file_names = NULL;
   cd->file_base_names = NULL;
   cd->file_extensions = NULL;

   file_deps = NULL;
   n_file_deps = 0;
   if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                          ctx->project_configuration_length, 
                          cd->project_component_types[y], 
                          cd->project_components[y], "FILE_DEPENDS")) != NULL)
   {

    if(split_strings(ctx, value, -1, &n_file_deps, &file_deps))
    {
     fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
     fclose(output);
     return 1;
    }

    for(x=0; x < n_file_deps; x++)
    {
     code = add_to_string_array(&sources, n_sources, file_deps[x], -1, 1);

     if(code == -1)
      return 1;
 
     if(code == 0)
     {
      if(path_extract(file_deps[x], NULL, &extension))
      {
       fprintf(stderr, "BCA: path_extract(%s) failed\n", file_deps[x]);
       return 1;
      }

      if(add_to_string_array(&extensions, n_sources, extension, -1, 0) != 0)
       return 1;
      n_sources++;

      free(extension);
     }
    }

    free(value);
   }

   free_string_array(file_deps, n_file_deps);
  }
 }

 for(x=0; x < n_sources; x++)
 {
  if(graphviz_string_clean(ctx, sources[x], -1, out, 512))
   return 1;

  fprintf(output, " %s [label = \"%s\" ", out, sources[x]);

  fprintf(output, "shape = note ");

  if(graphviz_node_color_from_file_extension(ctx, output, extensions[x]))
   return 1;

  fprintf(output, "]\n");
 }

 free_string_array(extensions, n_sources);
 free_string_array(sources, n_sources);
 fprintf(output, "\n\n");

 return 0;
}

int graphviz_edges(struct bca_context *ctx, FILE *output, 
                   struct component_details *cd)
{
 char temp[1024], *base_file_name, *extension, in[512], out[512],
      **list = NULL, **list_d, **extensions = NULL, *value;
 int i, x, y, yes, handled, n_items, n_items_d = 0, driver_component_index = -1, swapped;
 struct host_configuration *tc;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: graphviz_edges(host = %s)\n", cd->host);

 if((tc = resolve_host_configuration(ctx, cd)) == NULL)
 {
  return 1;
 }

 if(engage_build_configuration_disables_for_host(ctx, cd->host))
 {
  return 1;
 }

 i=0; 
 while(i<ctx->n_disables)
 {
  if(strcmp(cd->project_component, ctx->disabled_components[i]) == 0)
  {
   return 0;
  }

  i++;
 }

 if(engage_build_configuration_swaps_for_host(ctx, cd->host))
 {
  return 1;
 }

 i=0; 
 while(i<ctx->n_swaps)
 {
  if(strcmp(cd->project_component, ctx->swapped_components[i]) == 0)
  {
   return 0;
  }

  i++;
 }


 if(strcmp(cd->project_component_type, "CUSTOM") == 0)
 {
  if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                         ctx->project_configuration_length, 
                         "CUSTOM", cd->project_component, "DRIVER")) == NULL)
  {
   fprintf(stderr, "BCA: no project file record for CUSTOM.%s.DRIVER\n", cd->project_component);
   return 1;
  }

  yes = 0;
  i = 0;
  while(i < cd->n_components)
  {
   if(strcmp(cd->project_components[i], value) == 0)
   {
    if(strcmp(cd->project_component_types[i], "MACROEXPAND") == 0)
    {
     yes = 1;
    }
    break;
   }
   i++;
  }
  if(yes == 0)
  {
   fprintf(stderr, 
           "BCA: CUSTOM.%s.DRIVER = %s does not seem to be a MACROEXPAND project component\n", 
           cd->project_component, value);
   return 1;
  }
  driver_component_index = i;
  free(value);
 }

 /* now that we have the host information, we can tranlaste the .INPUTs to .FILES */
 if(cd->n_file_names == 0)
 {
  cd->file_names = NULL;
  cd->file_base_names = NULL;
  cd->file_extensions = NULL;
 }

 for(i=0; i < cd->n_inputs; i++)
 {
  /* what kind of component is this input? */
  handled = 0;
  x = 0;
  while(x < cd->n_components)
  {
   if(strcmp(cd->inputs[i], cd->project_components[x]) == 0)
   {
    handled = 1;
    break;
   }
   x++;
  }

  if(handled == 0)
  {
   /* this should have been discovered by now, but check again */
   fprintf(stderr, 
           "BCA: component %s on host %s has an unresolved .INPUT of %s.\n",
           cd->project_component, cd->host, cd->inputs[i]);
   return 1;
  }

  snprintf(temp, 1024, "%s/%s",
           tc->build_prefix, cd->project_output_names[x]);

  if(path_extract(temp, &base_file_name, &extension))
  {
   return 1;
  }

  handled = 0;

  if(driver_component_index > -1)
  {
   if(add_to_string_array(&(cd->file_names), cd->n_file_names, 
                          temp, -1, 0))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   if(add_to_string_array(&(cd->file_base_names), cd->n_file_names, 
                          base_file_name, -1, 0))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   if(add_to_string_array(&(cd->file_extensions), cd->n_file_names, 
                          extension, -1, 0))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   cd->n_file_names++;
   continue;
  }

  if(extension != NULL)
  {
   if(strcmp(extension, "c") == 0)
   {
    if(add_to_string_array(&(cd->file_names), cd->n_file_names, 
                           temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(add_to_string_array(&(cd->file_base_names), cd->n_file_names, 
                           base_file_name, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(add_to_string_array(&(cd->file_extensions), cd->n_file_names, 
                           extension, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_file_names++;
    handled = 1;
   }

   if(strcmp(extension, "h") == 0)
   {
    if(add_to_string_array(&(cd->file_deps), cd->n_file_deps, 
                          temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_file_deps++;

    handled = 1;
   }
  }

  if(handled == 0)
  {
   fprintf(stderr, "BCA: I don't know what to do with with input %s\n", cd->inputs[i]);
   return 1;
  }

 }

 /* lines from source dependencies to component output; file_deps[] ie headers*/
 if((n_items = render_project_component_output_name(ctx, cd->host, cd->project_component, 2,
                                                    &list, &extensions)) < 0)
 {
  fprintf(stderr, "BCA: render_project_component_output_name() failed. 1\n");
  return 1;
 }

 if( (n_items > 0) || (driver_component_index > -1) )
 {
  for(i=0; i < cd->n_file_deps; i++)
  {
   if(graphviz_string_clean(ctx, cd->file_deps[i], -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(driver_component_index > -1)
   {
    snprintf(temp, 1024, "%s/%s\n", tc->build_prefix,
             cd->project_output_names[driver_component_index]);
    if(graphviz_string_clean(ctx, temp, -1, out, 512))
     return 1;
   } else {
    if(graphviz_string_clean(ctx, list[0], -1, out, 512))
     return 1;
   }

   fprintf(output, "%s [", out);

   if(graphviz_node_edge_from_component_type(ctx, output, cd->project_component_type, 
                                             cd->file_deps[i]))
    return 1;

   fprintf(output, "]\n");
  }
 }

 /* lines from dependencies' .pc files to component output files
    the internal ones differ from the external ones in that internal ones are themselves 
    component output and are in specific build directories  */
 for(y=0; y < cd->n_dependencies; y++)
 {
  x = 0;
  yes = 1;
  while(x < cd->n_components)
  {
   if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
   {
    /* this is an internal dependency */
    yes = 0;
    handled = 0;

    swapped = 0;
    i=0; 
    while(i<ctx->n_swaps)
    {
     if(strcmp(cd->dependencies[y], ctx->swapped_components[i]) == 0)
     {
      swapped = 1;
      break;
     }   
     i++;
    }

    if(swapped == 0)
    {
     if((n_items_d = render_project_component_output_name(ctx, cd->host, 
                                                          cd->project_components[x], 2,
                                                          &list_d, NULL)) < 0)
     {
      fprintf(stderr, "BCA: render_project_component_output_name() failed 2\n");
      return 1;
     }
    } else {
     if((n_items_d = render_project_component_output_name(ctx,
                                                          ctx->swapped_component_hosts[i], 
                                                          cd->project_components[x], 2,
                                                          &list_d, NULL)) < 0)
     {
      fprintf(stderr, "BCA: render_project_component_output_name() failed 3\n");
      return 1;
     }
    }

    if(n_items_d == 0)
    {
     x++;
     continue;
    }

    if(n_items_d > 2)
     handled = 1;

    if(handled == 0)
     return 0;

    if(graphviz_string_clean(ctx, list_d[1], -1, out, 512))
     return 1;

    fprintf(output, " %s -> ", out);

    if(graphviz_string_clean(ctx, list[0], -1, out, 512))
     return 1;

    fprintf(output, "%s [", out);

    if(graphviz_node_edge_from_component_type(ctx, output, cd->project_component_type, list_d[1]))
     return 1;

    fprintf(output, "]\n");

    free_string_array(list_d, n_items_d);
    n_items_d = 0;
    break;
   }
   x++;
  }

  if(yes)
  {
   snprintf(in, 512, "%s.pc", cd->dependencies[y]);

   if(graphviz_string_clean(ctx, in, -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(graphviz_string_clean(ctx, list[0], -1, out, 512))
    return 1;

   fprintf(output, "%s [", out);

   if(graphviz_node_edge_from_component_type(ctx, output, cd->project_component_type, in))
    return 1;

   fprintf(output, "]\n");
  }
 }

 fprintf(output, "\n");

 /* these are the lines from the source files to the component output file */
 if( (n_items > 0) || (driver_component_index > -1) )
 {
  for(i=0; i<cd->n_file_names; i++)
  {
   if(graphviz_string_clean(ctx, cd->file_names[i], -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(driver_component_index > -1)
   {
    snprintf(temp, 1024, "%s/%s", tc->build_prefix,
             cd->project_output_names[driver_component_index]);
    if(graphviz_string_clean(ctx, temp, -1, out, 512))
     return 1;
   } else {
    if(graphviz_string_clean(ctx, list[0], -1, out, 512))
     return 1;
   }

   fprintf(output, "%s [", out);

   if(driver_component_index > -1)
   {
    fprintf(output, " color=green ");
   } else {
    if(graphviz_node_edge_from_component_type(ctx, output, cd->project_component_type, 
                                              cd->file_names[i]))
     return 1;
   }

   fprintf(output, "]\n");
  }
 }

 fprintf(output, "\n");

 /* These are the lines from shared libraries to the corrisponding .pc files */
 for(x=1; x<n_items; x++)
 {
  if(list[x][0] != 0)
  {
   if(graphviz_string_clean(ctx, list[0], -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(graphviz_string_clean(ctx, list[x], -1, out, 512))
    return 1;

   fprintf(output, "%s [", out);

   if(graphviz_node_edge_from_component_type(ctx, output, cd->project_component_type, 
                                             extensions[x]))
    return 1;

   fprintf(output, "]\n");
  }
 }

 if(strcmp(cd->project_component_type, "CUSTOM") == 0)
 {
  if(n_items_d > 0)
  {
   snprintf(temp, 1024, "%s/%s", tc->build_prefix,
            cd->project_output_names[driver_component_index]);
   if(graphviz_string_clean(ctx, temp, -1, out, 512))
    return 1;

   fprintf(output, " %s -> ", out);

   if(graphviz_string_clean(ctx, list[0], -1, out, 512))
    return 1;

   fprintf(output, "%s [ color=green ]\n", out);
  }
 }


 free_string_array(list, n_items);
 free_string_array(extensions, n_items);
 free_host_configuration(ctx, tc);

 return 0;
}


int generate_graphviz_mode(struct bca_context *ctx)
{
 char **hosts, *value, **file_deps, **include_dirs;
 int n_hosts, x, y, n_file_deps, n_include_dirs;
 FILE *output;
 struct component_details cd;

 memset(&cd, 0, sizeof(struct component_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: graphviz_output_mode()\n");

 if((ctx->build_configuration_contents = 
     read_file("./buildconfiguration/buildconfiguration", 
               &(ctx->build_configuration_length), 0)) == NULL)
 {
  fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
  return 1;
 }

 if(list_unique_principles(ctx, NULL, ctx->build_configuration_contents,
                           ctx->build_configuration_length, &hosts, &n_hosts))
 {
  fprintf(stderr, "BCA: list_build_hosts() failed\n");
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: Found the following build hosts (%d): ", n_hosts);
  for(x=0; x<n_hosts; x++)
  {
   printf("%s ", hosts[x]);
  }
  printf("\n");
 }

 if((ctx->project_configuration_contents = 
      read_file("./buildconfiguration/projectconfiguration", 
                &(ctx->project_configuration_length), 0)) == NULL)
 {
  return 1;
 }

 if(list_project_components(ctx, &cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed\n");
  return 1;
 }

 if(check_duplicate_output_names(ctx, &cd))
 {
  fclose(output);
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

 if(graphviz_nodes(ctx, output, hosts, n_hosts, &cd))
 {
  fclose(output);
  return 1;
 }

 for(x=0; x < cd.n_components; x++)
 {

  if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                         ctx->project_configuration_length, 
                         cd.project_component_types[x], 
                         cd.project_components[x], "FILE_DEPENDS")) == NULL)
  {
   if(ctx->verbose)
    printf("BCA: No file level dependencies found for %s.%s\n", 
           cd.project_component_types[x], cd.project_components[x]);

   file_deps = NULL;
   n_file_deps = 0;
  } else {

   if(split_strings(ctx, value, -1, &n_file_deps, &file_deps))
   {
    fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
    fclose(output);
    return 1;
   }
  }

  if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                         ctx->project_configuration_length, 
                         cd.project_component_types[x], 
                         cd.project_components[x], "INCLUDE_DIRS")) == NULL)
  {
   include_dirs = NULL;
   n_include_dirs = 0;
  } else {

   if(split_strings(ctx, value, -1, &n_include_dirs, &include_dirs))
   {
    fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
    fclose(output);
    return 1;
   }

  }

  if(resolve_component_file_dependencies(ctx, &cd, x))
  {
   return 1;
  }

  if(resolve_component_input_dependencies(ctx, &cd, x))
  {
   return 1;
  }

  cd.project_component = cd.project_components[x];
  cd.project_component_type = cd.project_component_types[x];
  cd.project_component_output_name = cd.project_output_names[x];
  cd.file_deps = file_deps;
  cd.n_file_deps = n_file_deps;
  cd.include_dirs = include_dirs;
  cd.n_include_dirs = n_include_dirs;

  for(y=0; y < n_hosts; y++)
  {
   cd.host = hosts[y];

   /* WITHOUTS is needed for dependency resolution below */
   if((value = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length, 
                          cd.host, cd.project_components[x], "WITHOUTS")) == NULL)
   {
    value = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length, 
                       cd.host, "ALL", "WITHOUTS");
   }

   if(value != NULL)
   {
    if(split_strings(ctx, value, -1, &(cd.n_withouts), &(cd.withouts)))
    {
     fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
     fclose(output);
     return 1;
    }
   } else {
    cd.withouts = NULL;
    cd.n_withouts = 0;
   }

   if(resolve_component_dependencies(ctx, &cd))
   {
    return 1;
   }
   if(graphviz_edges(ctx, output, &cd))
   {
    fclose(output);
    return 1;
   }

   if(cd.n_dependencies > 0)
    free_string_array(cd.dependencies, cd.n_dependencies);

  }

  free_string_array(cd.withouts, cd.n_withouts);

  /* from resolve_component_file_dependencies() */
  if(cd.n_file_names > 0)
  {
   free_string_array(cd.file_names, cd.n_file_names);
   free_string_array(cd.file_base_names, cd.n_file_names);
   free_string_array(cd.file_extensions, cd.n_file_names);
  }

  /* from resolve_component_input_dependencies() */
  if(cd.n_inputs > 0)
   free_string_array(cd.inputs, cd.n_inputs);

  free_string_array(file_deps, n_file_deps);

  fprintf(output, "\n");
 }

 fprintf(output, "}\n");
 fclose(output); 
 free_string_array(cd.project_components, cd.n_components);
 free_string_array(cd.project_component_types, cd.n_components);

 return 0;
}


