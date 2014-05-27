/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012,2013,2014 Stover Enterprises, LLC
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

char *lookup_key(struct bca_context *ctx, char *file, int file_length,
                 char *principle_filter, char *component_filter, char *key_filter)
{
 char principle[256], component[256], key[256];
 char *value = NULL;
 int start, index, equals, in_quotes, end, value_length, offset;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: lookup_key()\n");

 offset = -1;
 if(iterate_key_primitives(ctx, file, file_length, &offset,
                           principle_filter, component_filter, key_filter,
                           principle, component, key, &equals) == 0)
  return NULL;

 end = offset;
 index = equals + 1;
 in_quotes = 0;
 start = -1;
 while(index < offset)
 {
  if(start == -1)
  {
   if(file[index] != ' ')
    start = index;
  }

  if(start > -1)
  {
   if(in_quotes == 0)
   {
    if(file[index] == '"')
    {
     in_quotes = 1;
     if(index + 1 < end)
     {
      start = ++index;
      continue;
     } else {
      file[end] = 0;
      fprintf(stderr, "BCA: quotation error (1) in value of '%s'\n", file + equals + 1);
      exit(1);
      return NULL;
     }
    }
   }

   if(in_quotes == 1)
   {
    if(file[index] == '"')
    {
     if( (start != index - 1) && (file[index-1] == '\\') )
     {
      /* escape out double quotation marks in values with backslash */
      index++;
      continue;
     }

     end = index++;
     in_quotes = 0;
     break;
    }
   }

  }
  index++;
 }

 if(in_quotes == 1)
 {
  fprintf(stderr, "BCA: quotation error (3) in value of '%s'\n", file + equals + 1);
  exit(1);
  return NULL;
 }

 if(start < 0)
  value_length = 0;
 else
  value_length = end - start;

 if((value = malloc(value_length + 1)) == NULL)
 {
  perror("BCA: malloc");
  exit(1);
  return NULL;
 }

 memcpy(value, file + start, value_length);
 value[value_length] = 0;

 /* escape out " marks in values with \" */
 index=0;
 while(index < (value_length - 1))
 {
  if( (value[index] == '\\') &&
      (value[index+1] == '"') )
  {
   memmove(value + index, value + index + 1, --value_length - index );
   value[value_length] = 0;
  } else {
   index++;
  }
 }

 return value;
}

int output_modifications(struct bca_context *ctx, FILE *output,
                         char *contents, int length, int n_records,
                         char **principle, char **component, char **key, char **value)
{
 char o_principle[256], o_component[256], o_key[256], *o_value, *output_value, *temp;
 int end = -1, action, *handled, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: output_modification()\n");

 if((handled = (int *) malloc(sizeof(int) * (n_records + 1))) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%u) failed\n",
          (unsigned int) (sizeof(int) * (n_records + 1)));
  return 1;
 }

 for(i=0; i<n_records; i++)
 {
  handled[i] = 0;
 }

 while(iterate_key_primitives(ctx, contents, length, &end,
                              NULL, NULL, NULL,
                              o_principle, o_component, o_key, NULL))
 {
  o_value = lookup_key(ctx, contents, length, o_principle, o_component, o_key);

  action = 1;

  i = 0;
  while(i < n_records)
  {
   if(strcmp(o_principle, principle[i]) == 0)
   {
    if(strcmp(o_component, component[i]) == 0)
    {
     if(strcmp(o_key, key[i]) == 0)
     {
      handled[i] = 1;
      action = 2;
      if(value[i] == NULL)
       action = 3;

      break;
     }
    }
   }
   i++;
  }

  switch(action)
  {
   case 1:  /* copy original */
        fprintf(output, "%s.%s.%s = ", o_principle, o_component, o_key);
        temp = o_value;

        if((output_value = escape_value(ctx, temp, -1)) == NULL)
        {
         /* bail */
         free(handled);
         return 1;
        }
        fprintf(output, "%s\n", output_value);

        if(ctx->verbose > 3)
         fprintf(stderr, "BCA: keeping record not in modify array %s.%s.%s = %s\n",
                 o_principle, o_component, o_key, output_value);

        if(output_value != temp)
         free(output_value);
        break;

   case 2:  /* replace with updated value case */
        fprintf(output, "%s.%s.%s = ", principle[i], component[i], key[i]);
        temp = value[i];

        if((output_value = escape_value(ctx, temp, -1)) == NULL)
        {
         /* bail */
         free(handled);
         return 1;
        }
        fprintf(output, "%s\n", output_value);

        if(ctx->verbose > 3)
         fprintf(stderr, "BCA: updating record %s.%s.%s = %s\n",
                 principle[i], component[i], key[i], output_value);

        if(output_value != temp)
         free(output_value);
        break;

   case 3:  /* leave out this record */
        if(ctx->verbose > 3)
         fprintf(stderr, "BCA: dropping set-NULL value record %s.%s.%s\n",
                 principle[i], component[i], key[i]);
        break;

  }

  free(o_value);
 }

 /* append cases */
 for(i=0; i<n_records; i++)
 {
  if( (handled[i] == 0) && (value[i] != NULL) )
  {
   if((output_value = escape_value(ctx, value[i], -1)) == NULL)
   {
    /* bail */
    free(handled);
    return 1;
   }

   if(ctx->verbose > 3)
    fprintf(stderr, "BCA: appending record %s.%s.%s = %s\n",
            principle[i], component[i], key[i], output_value);

   fprintf(output, "%s.%s.%s = %s\n", principle[i], component[i], key[i], output_value);

   if(output_value != value[i])
    free(output_value);
  }
 }

 free(handled);
 return 0;
}

int output_modification(struct bca_context *ctx, FILE *output,
                        char *contents, int length,
                        char *principle, char *component, char *key, char *value)
{
 return output_modifications(ctx, output, contents, length, 1,
                             &principle, &component, &key, &value);
}


int modify_file(struct bca_context *ctx, char *filename,
                char *principle, char *component, char *key, char *value)
{
 FILE *output;
 char *contents;
 int length;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: modify_file()\n");

 if((contents = read_file(filename, &length, 0)) == NULL)
 {
  fprintf(stderr, "BCA: modify_file(): read_file() failed\n");
  return 1;
 }

 if((output = fopen(filename, "w")) == NULL)
 {
  perror("BCA: modify_file()\n");
  return 1;
 }

 if(output_modification(ctx, output, contents, length, principle, component, key, value))
 {
  fprintf(stderr, "BCA: modify_file(): output_modifications() failed\n");
  fclose(output);
  return 1;
 }

 fclose(output);
 free(contents);

 return 0;
}

int iterate_key_primitives(struct bca_context *ctx, char *file, int file_length, int *offset,
                           char *principle_filter, char *component_filter, char *key_filter,
                           char principle[256], char component[256], char key[256],
                           int *equals_pos)
{
 int start, end, line_length = 0, n_periods, periods[2], index, equals = -1, no,
     principle_length, component_length, key_length, principle_filter_length,
     component_filter_length, key_filter_length;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: iterate_key_primitives()\n");

 if(principle_filter == NULL)
 {
  principle_filter = "*";
  principle_filter_length = 1;
 } else
  principle_filter_length = strlen(principle_filter);

 if(component_filter == NULL)
 {
  component_filter = "*";
  component_filter_length = 1;
 } else
  component_filter_length = strlen(component_filter);

 if(key_filter == NULL)
 {
  key_filter = "*";
  key_filter_length = 1;
 } else
  key_filter_length = strlen(key_filter);

 end = *offset;
 while(find_line(file, file_length, &start, &end, &line_length))
 {
  *offset = end;
  index = start;
  n_periods = 0;
  while(index < end)
  {
   if(file[index] == '.')
   {
    if(n_periods > 1)
    {
     file[end] = 0; /* destructive but we are aborting anyway */
     fprintf(stderr, "BCA: more than 2 periods on the left of '=' on the line \"%s\"\n",
             file + start);
     exit(1);

    }

    periods[n_periods++] = index;
   }

   if(file[index] == '=')
   {
    equals = index;
    no = 0;

    if(n_periods != 2)
     no = 1;

    if((principle_length = (periods[0] - start)) < 1)
     no = 1;

    if(principle_length > 255)
     no = 1;

    if((component_length = (periods[1] - (periods[0] + 1))) < 1)
     no = 1;

    if(component_length > 255)
     no = 1;

    if((key_length = (equals - (periods[1] + 1))) < 1)
     no = 1;

    while(key_length > 1)
    {
     if(file[periods[1] + key_length] == ' ') {
      key_length--;
     } else
      break;
    }

    if(key_length > 255)
     no = 1;

    if(no)
    {
     file[end] = 0;
     fprintf(stderr,
             "BCA: left of '=' not in the format PRINCIPLE.QUALIFIER.KEY on the line \"%s\"\n",
             file + start);
     exit(1);
    }

    index++;
    break;
   }

   index++;
  }

  if(equals == -1)
  {
   file[end] = 0;
   fprintf(stderr, "BCA: no '=' on the line \"%s\"\n", file + start);
   exit(1);
  }

  if(principle_filter[0] != '*')
  {
   if(principle_length != principle_filter_length)
    continue;

   if(strncmp(principle_filter, file + start, principle_length) != 0)
    continue;
  }

  if(principle != NULL)
  {
   memcpy(principle, file + start, principle_length);
   principle[principle_length] = 0;
  }

  if(component_filter[0] != '*')
  {
   if(component_length != component_filter_length)
    continue;
   if(strncmp(component_filter, file + periods[0] + 1, component_length) != 0)
    continue;
  }

  if(component != NULL)
  {
   memcpy(component, file + periods[0] + 1, component_length);
   component[component_length] = 0;
  }

  if(key_filter[0] != '*')
  {
   if(key_length != key_filter_length)
    continue;
   if(strncmp(key_filter, file + periods[1] + 1, key_length) != 0)
    continue;
  }

  if(key != NULL)
  {
   memcpy(key, file + periods[1] + 1, key_length);
   key[key_length] = 0;
  }

  if(equals_pos != NULL)
   *equals_pos = equals;

  return 1;
 }

 return 0;
}

int list_component_internal_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: list_component_internal_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No internal dependencies found for component \"%s\".\n",
          cd->project_component);

  *list = NULL;
  *n_elements = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, n_elements, list))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int list_component_external_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_component_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No external dependencies found for component \"%s\".\n",
          cd->project_component);

  *list = NULL;
  *n_elements = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, n_elements, list))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int list_component_opt_external_dependencies(struct bca_context *ctx,
                                             struct component_details *cd,
                                             char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_component_opt_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "OPT_EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No optional external dependencies found for component \"%s\".\n",
          cd->project_component);

  *list = NULL;
  *n_elements = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, n_elements, list))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int check_duplicate_output_names(struct bca_context *ctx, struct component_details *cd)
{
 char **output_names = NULL;
 int n_output_names = 0, x;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: check_duplicate_output_names()\n");

 for(x=0; x<cd->n_components; x++)
 {
  if(add_to_string_array(&output_names, n_output_names,
                         cd->project_output_names[x], -1, 1))
  {
   fprintf(stderr,
           "BCA: The component output name \"%s\" is used more than once.\n",
           cd->project_output_names[x]);

   return 1;
  }
  n_output_names++;
 }

 free_string_array(output_names, n_output_names);
 return 0;
}

int engage_build_configuration_disables_for_host(struct bca_context *ctx, char *host)
{
 char *value;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: engage_build_configuration_disables_for_host(%s)\n", host);

 value = lookup_key(ctx, ctx->build_configuration_contents,
                    ctx->build_configuration_length,
                    host, "ALL", "DISABLES");

 if(ctx->disabled_components != NULL)
 {
  free_string_array(ctx->disabled_components, ctx->n_disables);
  ctx->n_disables = 0;
  ctx->disabled_components = NULL;
 }

 if(value == NULL)
  return 0;

 if(split_strings(ctx, value, -1, &(ctx->n_disables), &(ctx->disabled_components)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int engage_build_configuration_swaps_for_host(struct bca_context *ctx, char *host)
{
 char *value, **hosts = NULL, *disables;
 char principle[256], component[256], key[256];
 int n_hosts = 0, i, ok, end = -1;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: engage_build_configuration_swaps_for_host(%s)\n", host);

 if(ctx->swapped_components != NULL)
 {
  free_string_array(ctx->swapped_components, ctx->n_swaps);
  free_string_array(ctx->swapped_component_hosts, ctx->n_swaps);
  ctx->n_swaps = 0;
  ctx->swapped_components = NULL;
  ctx->swapped_component_hosts = NULL;
 }

 if(list_unique_principles(ctx, NULL,
                           ctx->build_configuration_contents,
                           ctx->build_configuration_length,
                           &hosts, &n_hosts))
 {
  fprintf(stderr, "BCA: list_unique_principles() failed.\n");
 }

 while(iterate_key_primitives(ctx, ctx->build_configuration_contents,
                              ctx->build_configuration_length, &end,
                              host, NULL, "SWAP",
                              principle, component, key, NULL))
 {
  value = lookup_key(ctx, ctx->build_configuration_contents,
                     ctx->build_configuration_length,
                     principle, component, key);

  if(strcmp(value, host) == 0)
  {
   fprintf(stderr,
           "BCA: Component %s on host %s swaps back to the same host.\n",
           component, host);
   free(value);
   free_string_array(hosts, n_hosts);
   return 1;
  }

  ok = 0;
  i = 0;
  while(i<n_hosts)
  {
   if(strcmp(value, hosts[i]) == 0)
   {
    if(strcmp(hosts[i], host) != 0)
    {
     ok = 1;
     break;
    }
   }
   i++;
  }
  if(ok == 0)
  {
   fprintf(stderr,
           "BCA: Component \"%s\" on host \"%s\" swaps to unconfigured host \"%s\".\n",
           component, host, value);
   free(value);
   free_string_array(hosts, n_hosts);
   return 1;
  }

  if((disables = lookup_key(ctx,
                            ctx->build_configuration_contents,
                            ctx->build_configuration_length,
                            hosts[i], component, "DISABLES")) == NULL)
  {
   if(contains_string(disables, -1, component, -1))
   {
    fprintf(stderr,
            "BCA: Component \"%s\" on host \"%s\" swaps to host \"%s\", "
            "on which it is disabled.\n",
            component, host, hosts[i]);
    free(disables);
    free(value);
    free_string_array(hosts, n_hosts);
    return 1;
   }

   free(disables);
  }

  if(add_to_string_array(&(ctx->swapped_components),
                         ctx->n_swaps,
                         component, -1, 1))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  if(add_to_string_array(&(ctx->swapped_component_hosts),
                         ctx->n_swaps, value, -1, 1))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  ctx->n_swaps++;
  free(value);
 }

 free_string_array(hosts, n_hosts);
 return 0;
}

int check_project_component_types(struct bca_context *ctx)
{
 int handled, offset = -1, i;
 char type[256], component[256], key[256];

 char *component_types[10] = { "NONE", "BINARY", "SHAREDLIBRARY", "STATICLIBRARY", "CAT",
                              "MACROEXPAND", "PYTHONMODULE", "CUSTOM", "BEAM" };

 char *component_keys[20] = { "PROJECT_NAME", "NAME", "MAJOR", "MINOR", "AUTHOR", "EMAIL",
                              "URL", "FILES", "INPUT", "DRIVER", "INCLUDE_DIRS",
                              "FILE_DEPENDS", "INT_DEPENDS", "EXT_DEPENDS", "OPT_EXT_DEPENDS",
                              "LIB_HEADERS", "DISABLES", "DESCRIPTION", "PACKAGE_NAME" };

 while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                              ctx->project_configuration_length, &offset,
                              NULL, NULL, NULL,
                              type, component, key, NULL))
 {
  handled = 0;
  i=0;
  while(i<9)
  {
   if(strcmp(type, component_types[i]) == 0)
   {
    handled = 1;
    break;
   }
   i++;
  }

  if(handled == 0)
   fprintf(stderr,
           "BCA: WARNING - Are you sure about a project component type of \"%s\"?\n",
           type);

  handled = 0;
  i=0;
  while(i<19)
  {
   if(strcmp(key, component_keys[i]) == 0)
   {
    handled = 1;
    break;
   }
   i++;
  }

  if(handled == 0)
   fprintf(stderr,
           "BCA: WARNING - Are you sure about a project component key of \"%s\"?\n",
           key);

 }

 return 0;
}

/* does some error checking and fills in
   cd->project_components
   cd->project_ouput_names
   cd->project_component_types
   cd->n_components
*/
int list_project_components(struct bca_context *ctx,
                            struct component_details *cd)
{
 char **list = NULL, *name, *source, *base_file_name, **source_files;
 int n_elements = 0, x, i, disabled, allocation_size, offset, n_source_files;
 char principle[256];

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: list_project_components()\n");

 if(list_unique_qualifiers(ctx,
                           ctx->project_configuration_contents,
                           ctx->project_configuration_length,
                           &list, &n_elements))
 {
  fprintf(stderr, "BCA: list_unique_principles() failed.\n");
  return 1;
 }

 cd->n_components = 0;
 cd->project_components = NULL;

 for(x=0; x<n_elements; x++)
 {
  disabled = 0;
  for(i=0; i < ctx->n_disables; i++)
  {
   if(strcmp(list[x], ctx->disabled_components[i]) == 0)
   {
    disabled = 1;
    break;
   }
  }

  if(disabled)
   continue;

  if(strcmp(list[x], "ALL") == 0)
   continue;

  if(strcmp(list[x], "NONE") == 0)
   continue;

  if(add_to_string_array(&(cd->project_components),
                         cd->n_components,
                         list[x], -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  offset = -1;
  if(iterate_key_primitives(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            &offset, NULL, list[x], NULL,
                            principle, NULL, NULL, NULL) == 0)
  {
   fprintf(stderr, "BCA: iterate_key_primitives(*, %s, *) failed \n", list[x]);
   return 1;
  }

  name = NULL;
  if(strcmp(principle, "BEAM") == 0)
  {
   if((source = lookup_key(ctx,
                           ctx->project_configuration_contents,
                           ctx->project_configuration_length,
                           principle, list[x], "FILES")) == NULL)
   {
    fprintf(stderr, "BCA: lookup_key(%s, %s, FILES) failed\n", principle, list[x]);
    return 1;
   }

   if(split_strings(ctx, source, -1, &n_source_files, &source_files))
   {
    fprintf(stderr, "BCA: split_strings() failed on '%s'\n", source);
    return 1;
   }
   free_string_array(source_files, n_source_files);

   if(n_source_files > 1)
   {
    fprintf(stderr,
            "BCA: BEAM component %s should only have one input file, not \"%s\"\n",
            list[x], source);
    return 1;
   }

   if(path_extract(source, &base_file_name, NULL))
   {
    fprintf(stderr, "BCA: path_extract(%s) failed\n", list[x]);
    return 1;
   }

   allocation_size = strlen(base_file_name) + 6;
   name = malloc(allocation_size);
   snprintf(name, allocation_size, "%s", base_file_name);
   free(base_file_name);
   free(source);
  }

  if(name == NULL)
  {
   if((name = lookup_key(ctx, ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         principle, list[x], "NAME")) == NULL)
   {
    fprintf(stderr, "BCA: warning: no .NAME for component %s of %s.\n",
            list[x], principle);
    name = strdup(list[x]);
   }
  }

  if(add_to_string_array(&(cd->project_component_types),
                         cd->n_components, principle, -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  if(add_to_string_array(&(cd->project_output_names),
                         cd->n_components, name, -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  free(name);
  cd->n_components++;
 }

 free_string_array(list, n_elements);
 return 0;
}


int list_unique_principles(struct bca_context *ctx, char *search_qualifier,
                           char *contents, int length,
                           char ***principle_list, int *n_principles)
{
 char principle[256], qualifier[256], key[256], **principles = NULL, **new_ptr, *string;
 int offset, matched, x, allocation_size;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_unique_principles()\n");

 *n_principles = 0;

 offset = -1;
 while(iterate_key_primitives(ctx, contents, length, &offset,
                              NULL, search_qualifier, NULL,
                              principle, qualifier, key, NULL))
 {
  matched = 0;
  x = 0;

  if(strcmp(principle, "ALL") == 0)
   matched = 1;

  if(strcmp(principle, "NONE") == 0)
   matched = 1;

  if(matched == 0)
  {
   while(x < *n_principles)
   {
    if(strcmp(principles[x], principle) == 0)
    {
     matched = 1;
     break;
    }

    x++;
   }
  }

  if(matched == 0)
  {
   (*n_principles)++;
   allocation_size = *n_principles * sizeof(char *);

   if((new_ptr = (char **) realloc(principles, allocation_size)) == NULL)
   {
    perror("BCA: list_unique_principles(): realloc()");
    return 1;
   }

   principles = new_ptr;

   allocation_size = strlen(principle) + 1;
   if((string = (char *) malloc(allocation_size)) == NULL)
   {
    perror("BCA: list_unique_principles(): malloc()");
    return 1;
   }

   snprintf(string, allocation_size, "%s", principle);
   principles[*n_principles - 1] = string;
  }

 }

 *principle_list = principles;
 return 0;
}

int list_unique_qualifiers(struct bca_context *ctx,
                           char *contents, int length,
                           char ***list, int *n_elements)
{
 char principle[256], qualifier[256], key[256];
 int offset, code;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_unique_qualifiers()\n");

 *list = NULL;
 *n_elements = 0;

 offset = -1;
 while(iterate_key_primitives(ctx, contents, length, &offset,
                              NULL, NULL, NULL,
                              principle, qualifier, key, NULL))
 {
  if(strcmp(qualifier, "ALL") == 0)
   continue;

  if(strcmp(qualifier, "NONE") == 0)
   continue;

  if((code = add_to_string_array(list, *n_elements,
                                    qualifier, -1, 1)) == -1)
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  if(code == 0)
  {
   (*n_elements)++;
  }
 }

 return 0;
}

char *resolve_build_host_variable(struct bca_context *ctx,
                                  char *host,
                                  char *project_component,
                                  char *key)
{
 char *value = NULL;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: resolve_build_host_variable()\n");

 if(project_component == NULL)
  project_component = "ALL";

 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        host, project_component, key)) == NULL)
 {
  value = lookup_key(ctx, ctx->build_configuration_contents, ctx->build_configuration_length,
                     host, "ALL", key);
 }

 return value;
}

struct host_configuration *
resolve_host_configuration(struct bca_context *ctx, struct component_details *cd)
{
 int allocation_size, i;
 struct host_configuration *tc;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: resolve_host_configuration()\n");
  fflush(stderr);
 }

 allocation_size = sizeof(struct host_configuration);
 if((tc = (struct host_configuration *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }
 memset(tc, 0, allocation_size);

 char **host_resolve_vars[27] =
 {
  &(tc->build_prefix),
  &(tc->cc),
  &(tc->cc_output_flag),
  &(tc->cc_compile_bin_obj_flag),
  &(tc->cc_compile_shared_library_obj_flag),
  &(tc->cc_include_dir_flag),
  &(tc->cc_define_macro_flag),
  &(tc->cflags),
  &(tc->pkg_config),
  &(tc->pkg_config_path),
  &(tc->pkg_config_libdir),
  &(tc->binary_suffix),
  &(tc->shared_library_suffix),
  &(tc->shared_library_prefix),
  &(tc->obj_suffix),
  &(tc->ldflags),
  &(tc->install_prefix),
  &(tc->install_bin_dir),
  &(tc->install_lib_dir),
  &(tc->install_include_dir),
  &(tc->install_pkg_config_dir),
  &(tc->install_locale_data_dir),
  &(tc->python),
  &(tc->erlc),
  &(tc->erlc_flags),
  &(tc->erlc_output_dir_flag)
 };

 char *host_resolve_keys[27] =
 {
  "BUILD_PREFIX",
  "CC",
  "CC_SPECIFY_OUTPUT_FLAG",
  "CC_COMPILE_BIN_OBJ_FLAG",
  "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG",
  "CC_INCLUDE_DIR_FLAG",
  "CC_DEFINE_MACRO_FLAG",
  "CFLAGS",
  "PKG_CONFIG",
  "PKG_CONFIG_PATH",
  "PKG_CONFIG_LIBDIR",
  "BINARY_SUFFIX",
  "SHARED_LIBRARY_SUFFIX",
  "SHARED_LIBRARY_PREFIX",
  "OBJ_SUFFIX",
  "LDFLAGS",
  "INSTALL_PREFIX",
  "INSTALL_BIN_DIR",
  "INSTALL_LIB_DIR",
  "INSTALL_INCLUDE_DIR",
  "INSTALL_PKG_CONFIG_DIR",
  "INSTALL_LOCALE_DATA_DIR",
  "PYTHON",
  "ERLC",
  "ERLCFLAGS",
  "ERLC_OUTPUT_DIR_FLAG"
 };

 for(i=0; i<26; i++)
 {
  *(host_resolve_vars[i]) = resolve_build_host_variable(ctx, cd->host,
                                                        cd->project_component,
                                                        host_resolve_keys[i]);
 }

 if(ctx->verbose > 2)
 {
  for(i=0; i<26; i++)
  {
   printf("BCA: %s.%s.%s resolves to %s\n",
          cd->host, cd->project_component,  host_resolve_keys[i], *(host_resolve_vars[i]));
  }
 }

 return tc;
}

int free_host_configuration(struct bca_context *ctx, struct host_configuration *tc)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: fee_host_configuration()\n");

 if(tc != NULL)
 {
  if(tc->cc != NULL)
   free(tc->cc);

  if(tc->cflags != NULL)
   free(tc->cflags);

  if(tc->ldflags != NULL)
   free(tc->ldflags);

  if(tc->build_prefix != NULL)
   free(tc->build_prefix);

  if(tc->cc_output_flag != NULL)
   free(tc->cc_output_flag);

  if(tc->cc_compile_bin_obj_flag != NULL)
   free(tc->cc_compile_bin_obj_flag);

  if(tc->binary_suffix != NULL)
   free(tc->binary_suffix);

  if(tc->shared_library_suffix != NULL)
   free(tc->shared_library_suffix);

  if(tc->shared_library_prefix != NULL)
   free(tc->shared_library_prefix);

  if(tc->cc_compile_shared_library_obj_flag != NULL)
   free(tc->cc_compile_shared_library_obj_flag);

  if(tc->obj_suffix != NULL)
   free(tc->obj_suffix);

  if(tc->cc_include_dir_flag != NULL)
   free(tc->cc_include_dir_flag);

  if(tc->cc_define_macro_flag != NULL)
   free(tc->cc_define_macro_flag);

  if(tc->pkg_config != NULL)
   free(tc->pkg_config);

  if(tc->pkg_config_path != NULL)
   free(tc->pkg_config_path);

  if(tc->pkg_config_libdir != NULL)
   free(tc->pkg_config_libdir);

  if(tc->install_bin_dir != NULL)
   free(tc->install_bin_dir);

  if(tc->install_lib_dir != NULL)
   free(tc->install_lib_dir);

  if(tc->install_include_dir != NULL)
   free(tc->install_include_dir);

  if(tc->install_pkg_config_dir != NULL)
   free(tc->install_pkg_config_dir);

  if(tc->install_locale_data_dir != NULL)
   free(tc->install_locale_data_dir);

  if(tc->python != NULL)
   free(tc->python);

  if(tc->erlc != NULL)
   free(tc->erlc);

  if(tc->erlc_flags != NULL)
   free(tc->erlc_flags);

  if(tc->erlc_output_dir_flag != NULL)
   free(tc->erlc_output_dir_flag);

  free(tc);
 }
 return 0;
}

int resolve_component_dependencies(struct bca_context *ctx,
                                   struct component_details *cd)
{
 char *value = NULL;
 int i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_dependencies()\n");

 if(ctx->build_configuration_contents != NULL)
  value = lookup_key(ctx,
                     ctx->build_configuration_contents,
                     ctx->build_configuration_length,
                     cd->host, cd->project_component, "DEPENDS");

 if(value == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: No DEPENDS key found for component \"%s\" "
           "on host \"%s\", implying dependencies.\n",
            cd->project_component, cd->host);

  cd->dependencies = NULL;
  cd->n_dependencies = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, &(cd->n_dependencies), &(cd->dependencies)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: Found the following dependencies for component \"%s\" on host \"%s\" (%d): ",
         cd->project_component, cd->host, cd->n_dependencies);

  for(i=0; i < cd->n_dependencies; i++)
  {
   printf("%s ", cd->dependencies[i]);
  }
  printf("\n");
 }

 free(value);
 return 0;
}

int resolve_component_file_dependencies(struct bca_context *ctx,
                                        struct component_details *cd,
                                        int component_index)
{
 char *value, *base_file_name, *extension;
 int z, allocation_size;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_file_dependencies()\n");

 if(component_index >= cd->n_components)
 {
  fprintf(stderr,
          "BCA: resolve_component_file_dependencies(): invalid component_index %d\n",
          component_index);
  return 1;
 }

 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_types[component_index],
                        cd->project_components[component_index],
                        "FILES")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: Could not find %s.%s.FILES\n",
          cd->project_component_types[component_index],
          cd->project_components[component_index]);

  cd->n_file_names = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1,
                  &(cd->n_file_names),
                  &(cd->file_names)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: Found the following files for component \"%s\" (%d): ",
         cd->project_components[component_index], cd->n_file_names);

  for(z=0; z < cd->n_file_names; z++)
  {
   printf("%s ", cd->file_names[z]);
  }
  printf("\n");
  fflush(stdout);
 }

 allocation_size = cd->n_file_names * sizeof(char *);
 if((cd->file_base_names = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 if((cd->file_extensions = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 for(z=0; z < cd->n_file_names; z++)
 {
  if(path_extract(cd->file_names[z], &base_file_name, &extension))
  {
   return 1;
  }
  cd->file_base_names[z] = base_file_name;
  cd->file_extensions[z] = extension;
 }

 return 0;
}

int resolve_component_input_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         int component_index)
{
 char *value, **list = NULL;
 int z, i, n_elements = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_input_dependencies()\n");

 if(component_index >= cd->n_components)
 {
  fprintf(stderr,
          "BCA: resolve_component_file_dependencies(): invalid component_index %d\n",
          component_index);
  return 1;
 }

 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_types[component_index],
                        cd->project_components[component_index],
                        "INPUT")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: Could not find %s.%s.INPUT\n",
          cd->project_component_types[component_index],
          cd->project_components[component_index]);

  cd->n_inputs = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, &n_elements, &list))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 cd->n_inputs = 0;
 cd->inputs = NULL;
 for(z=0; z < n_elements; z++)
 {
  /* match this to a project component */
  i = 0;
  while(i< cd->n_components)
  {
   if(strcmp(list[z], cd->project_components[i]) == 0)
    break;
   i++;
  }

  if(i == component_index)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project "
           "component %s apears to list itself as an INPUT element\n",
           cd->project_components[component_index]);

   free_string_array(list, n_elements);
   return 1;
  }

  if(i  == cd->n_components)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project component %s list INPUT element"
           " %s that is not itself a project component. Perhapes the element belongs in "
           "a FILES record?\n",
           cd->project_components[component_index], list[z]);
   free_string_array(list, n_elements);
   return 1;
  }

  if(add_to_string_array(&(cd->inputs), cd->n_inputs, cd->project_components[i], -1, 1))
  {
   fprintf(stderr, "BCA: resolve_component_file_dependencies(): add_to_string_array()\n");
   return 1;
  }
  cd->n_inputs++;
 }

 free_string_array(list, n_elements);

 return 0;
}

int resolve_component_version(struct bca_context *ctx,
                              char *contents, int contents_length,
                              struct component_details *cd,
                              char *component_type,
                              char *project_component)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_version()\n");

 if((cd->major = lookup_key(ctx, contents, contents_length,
                            component_type, project_component, "MAJOR")) == NULL)
 {
  if((cd->major = lookup_key(ctx, contents, contents_length,
                             "NONE", "NONE", "MAJOR")) == NULL)
  {
   cd->major = strdup("0");
  }
 }

 if((cd->minor = lookup_key(ctx, contents, contents_length,
                            component_type, project_component, "MINOR")) == NULL)
 {
  if((cd->minor = lookup_key(ctx, contents, contents_length,
                             "NONE", "NONE", "MINOR")) == NULL)
  {
   cd->minor = strdup("0");
  }
 }


 if(ctx->verbose)
    printf("BCA: Component \"%s\" version string set to %s.%s\n",
           project_component, cd->major, cd->minor);

 return 0;
}

int smart_pull_value(struct bca_context *ctx)
{
 char *new_value, **values, *file, *contents, *value, *q;
 int code, length, nv_length, allocation_size, n_values, i, handled;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: smart_pull_value()\n");

 if((code = check_value(ctx)) == 1)
 {
  fprintf(stderr, "BCA: check_value() failed.\n");
  return 1;
 }

 if(code != 2)
 {
  if(ctx->verbose)
   printf("BCA: value not in effective value\n");
  return 0;
 }

 /* at this point the value is present either the exact or the .ALL. variant */

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 q = ctx->qualifier;
 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  if(strcmp(ctx->qualifier, "ALL") != 0)
  {
   q = "ALL";
   value = lookup_key(ctx, contents, length, ctx->principle, "ALL", ctx->search_value_key);
  }

  if(value == NULL)
  {
   fprintf(stderr,
           "BCA: This should not happen!  No exact or .ALL. variant present"
           " after positive check_value().\n");
   return 1;
  }
 }

 if( (strcmp(q, "ALL") != 0) ||
     (strcmp(ctx->qualifier, "ALL") == 0) )
 {
  /* at this point, either the value was in an exact record so we do a regular pull_value(), or
     the intended modification for a .ALL. record itself and we do the same thing.  */

/* todo: if the exact record will now hold the same thing as the ALL record, remove the exact record ?*/

  free(value);
  free(contents);
  return pull_value(ctx);
 }

 /* here the value is present in a .ALL. record, but we want to remove it for the exact record
    case. so we copy and modify the .ALL. record and store it as an exact record */

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 allocation_size += 2;
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 new_value[nv_length = 0] = 0;
 for(i=0; i < n_values; i++)
 {
  if(strcmp(values[i], ctx->new_value_string) == 0)
  {
   handled = 1;
  } else {
   nv_length +=
   snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
  }
 }

 if(handled == 0)
 {
  fprintf(stderr,
          "BCA: should not happen: substring not in .ALL. value being to be copied "
          "and modified\n");
  return 1;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int smart_add_value(struct bca_context *ctx)
{
 char *new_value, **values, *file, *contents, *value;
 int code, length, allocation_size, n_values, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: smart_add_value()\n");

 if((code = check_value(ctx)) == 1)
 {
  fprintf(stderr, "BCA: check_value() failed.\n");
  return 1;
 }

 if(code == 2)
 {
  if(ctx->verbose)
   printf("BCA: value already in effective value\n");
  return 0;
 }

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  if(strcmp(ctx->qualifier, "ALL") != 0)
   value = lookup_key(ctx, contents, length, ctx->principle, "ALL", ctx->search_value_key);
 }

 if(value != NULL)
 {

  allocation_size = strlen(value);

  if(split_strings(ctx, value, allocation_size, &n_values, &values))
  {
   fprintf(stderr, "BCA: split_string() failed on %s\n", value);
   return 1;
  }

  length = strlen(ctx->new_value_string);

  if(add_to_string_array(&values, n_values,
                         ctx->new_value_string, length, 1) != 0)
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }
  n_values++;

  allocation_size += (n_values + length + 2);
  if((new_value = malloc(allocation_size)) == NULL)
  {
   perror("BCA: malloc()");
   return 1;
  }

  length = 0;
  for(i=0; i < n_values; i++)
  {
   length +=
   snprintf(new_value + length, allocation_size - length,
            "%s ", values[i]);
  }

 } else {
  new_value = ctx->new_value_string;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(contents);
 return 0;
}

int add_value(struct bca_context *ctx)
{
 char *contents, *file, **values, *new_value, *value;
 int length, n_values, allocation_size, nv_length, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: add_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  fprintf(stderr,
          "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
          ctx->principle, ctx->qualifier, ctx->search_value_key, file);
  return 1;
 }

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 nv_length = strlen(ctx->new_value_string);

 if(add_to_string_array(&values, n_values,
                        ctx->new_value_string, nv_length, 1) != 0)
 {
  fprintf(stderr, "BCA: add_to_string_array() failed\n");
  return 1;
 }
 n_values++;

 allocation_size += (n_values + nv_length + 2);
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 nv_length = 0;
 for(i=0; i < n_values; i++)
 {
  nv_length +=
  snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
 }


 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int pull_value(struct bca_context *ctx)
{
 char *contents, *file, **values, *new_value, *value;
 int length, n_values, allocation_size, nv_length, i, handled = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: pull_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  fprintf(stderr,
          "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
          ctx->principle, ctx->qualifier, ctx->search_value_key, file);
  return 1;
 }

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 allocation_size += 2;
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 new_value[nv_length = 0] = 0;
 for(i=0; i < n_values; i++)
 {
  if(strcmp(values[i], ctx->new_value_string) == 0)
  {
   handled = 1;
  } else {
   nv_length +=
   snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
  }
 }

 if(handled == 0)
 {
  fprintf(stderr, "BCA: pull_value(): substring '%s' no in original value\n",
          ctx->new_value_string);
  return 1;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int check_value_inline(struct bca_context *ctx,
                       char *contents, int length,
                       char *principle, char *qualifier,
                       char *key, char *check_value)
{
 char *value, **values = NULL;
 int i, value_length, n_values = 0, checked = 0;

 if((value = lookup_key(ctx, contents, length,
                        principle, qualifier, key)) == NULL)
 {
  if(strcmp(qualifier, "ALL") != 0)
   value = lookup_key(ctx, contents, length,
                      principle, "ALL", key);
 }

 if(value == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: lookup_key() failed for %s.%s.%s\n",
           ctx->principle, ctx->qualifier, ctx->search_value_key);

  return 0;
 }

 value_length = strlen(value);

 if(split_strings(ctx, value, value_length, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return -1;
 }

 i=0;
 while(i<n_values)
 {
  if(strcmp(values[i], check_value) == 0)
  {
   checked = 1;
   break;
  }
  i++;
 }

 free_string_array(values, n_values);
 free(value);

 return checked;
}

int check_value(struct bca_context *ctx)
{
 char *contents, *file;
 int code, length;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: check_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 code = check_value_inline(ctx, contents, length,
                           ctx->principle, ctx->qualifier, ctx->search_value_key,
                           ctx->new_value_string);

 switch(code)
 {
  case -1:
       code = 1;
       break;

  case 0:
       if(ctx->verbose)
        fprintf(stderr, "BCA: '%s' not found in %s.[%s/ALL].%s. Returning 3\n",
                ctx->new_value_string, ctx->principle, ctx->qualifier, ctx->search_value_key);

       code = 3;
       break;

  case 1:
       if(ctx->verbose)
        fprintf(stderr,
                "BCA: '%s' found in %s.[%s/ALL].%s. Returning 2.\n",
                ctx->new_value_string, ctx->principle, ctx->qualifier, ctx->search_value_key);
       code = 2;
       break;
 }

 free(contents);
 return code;
}

int resolve_project_name(struct bca_context *ctx)
{
 if(ctx->project_configuration_contents == NULL)
  fprintf(stderr, "BCA: resolve_project_name(): project configuration contents not availale\n");

 if(ctx->project_name != NULL)
  free(ctx->project_name);

 if((ctx->project_name =
     lookup_key(ctx, ctx->project_configuration_contents, ctx->project_configuration_length,
                "NONE", "NONE", "PROJECT_NAME")) == NULL)
  return 1;

 return 0;
}

int resolve_effective_path_mode(struct bca_context *ctx)
{
 char *value;
 int result, handled = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_effective_path_mode()\n");

 /* default to LOCAL */
 result = EFFECTIVE_PATHS_LOCAL;

 if(ctx->build_configuration_contents == NULL)
 {
  if((ctx->build_configuration_contents =
      read_file("./buildconfiguration/buildconfiguration",
                &(ctx->build_configuration_length), 1)) == NULL)
  {
   return -1;
  }
 }

 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        "ALL", "ALL", "EFFECTIVE_PATHS")) != NULL)
 {
  if(strcmp(value, "LOCAL") == 0)
  {
   handled = 1;
   result = EFFECTIVE_PATHS_LOCAL;
  }

  if(strcmp(value, "INSTALL") == 0)
  {
   handled = 1;
   result = EFFECTIVE_PATHS_INSTALL;
  }

  if(handled == 0)
  {
   fprintf(stderr,
           "BCA: invalid value for build configuration value ALL.ALL.EFFECTIVE_PATHS, \"%s\"\n",
           value);
   result = -1;
  }
  free(value);
 }

 return result;
}

int resolve_component_installation_path(struct bca_context *ctx,
                                        char *host,
                                        char *component_type,
                                        char *component,
                                        char **path)
{
 char *avalue, *bvalue = NULL;
 char temp[1024];
 int offset = 0;

 if(ctx->project_configuration_contents == NULL)
 {
  if((ctx->project_configuration_contents =
      read_file("./buildconfiguration/projectconfiguration",
                &(ctx->project_configuration_length), 1)) == NULL)
  {
   return 1;
  }
 }

 if(ctx->build_configuration_contents == NULL)
 {
  if((ctx->build_configuration_contents =
      read_file("./buildconfiguration/buildconfiguration",
                &(ctx->build_configuration_length), 1)) == NULL)
  {
   return 1;
  }
 }

 if((avalue = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        component_type, component, "INSTALL_PATH")) == NULL)
 {
  avalue = lookup_key(ctx, ctx->project_configuration_contents,
                      ctx->project_configuration_length,
                      component_type, "ALL", "INSTALL_PATH");
 }

 if(avalue == NULL)
 {
  /* if component install path was not given fallback to defaults based
     on component type */
  if(strcmp(component_type, "BINARY") == 0)
  {
   avalue = strdup("${BIN_DIR}");
  } else if(strcmp(component_type, "SHAREDLIBRARY") == 0) {
   avalue = strdup("${LIB_DIR}");
  } else {
   avalue = strdup("NONE");
  }
 }

 /* now resolve the build configuration specific install location vars */
 if(strcmp(avalue, "NONE") == 0)
 {
  *path = NULL;
 }

 if(strncmp(avalue, "${PREFIX}", 9) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_PREFIX")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_PREFIX");
  }
  offset = 9;
 }

 if(strncmp(avalue, "${BIN_DIR}", 10) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_BIN_DIR")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_BIN_DIR");
  }
  offset = 10;
 }

 if(strncmp(avalue, "${LIB_DIR}", 10) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_LIB_DIR")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_LIB_DIR");
  }
  offset = 10;
 }

 if(strncmp(avalue, "${INCLUDE_DIR}", 14) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_INCLUDE_DIR")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_INCLUDE_DIR");
  }
  offset = 14;
 }

 if(strncmp(avalue, "${PKG_CONFIG_DIR}", 17) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_PKG_CONFIG_DIR")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_PKG_CONFIG_DIR");
  }
  offset = 17;
 }

 if(strncmp(avalue, "${LOCALE_DATA_DIR}", 18) == 0)
 {
  if((bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          host, component, "INSTALL_LOCALE_DATA_DIR")) == NULL)
  {
   bvalue = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       host, "ALL", "INSTALL_LOCALE_DATA_DIR");
  }
  offset = 18;
 }

 if(bvalue == NULL)
 {
  if(strcmp(avalue, "NONE") != 0)
  {
   fprintf(stderr,
           "BCA: could not resolve %s for host %s\n",
           avalue, host);
   return 1;
  }

  return 0;
 }

 snprintf(temp, 1024, "%s%s", bvalue, avalue + offset);
 *path = strdup(temp);
 free(bvalue);
 free(avalue);
 return 0;
}

int is_project_using_config_h(struct bca_context *ctx)
{
 char *value = NULL;
 int config_h = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: is_project_using_config_h()\n");

 value = lookup_key(ctx,
                    ctx->project_configuration_contents,
                    ctx->project_configuration_length,
                    "*", "config_h", "NAME");

 if(value != NULL)
 {
  config_h = 1;
  free(value);
 }

 return config_h;
}

