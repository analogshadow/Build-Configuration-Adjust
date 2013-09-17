/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012,2013 Stover Enterprises, LLC
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
      /* escape out " marks in values with \" */
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
  fprintf(stderr, "BCA: malloc(%d) failed\n", (sizeof(int) * (n_records + 1)));
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
     fprintf(stderr, "BCA: left of '=' not in the format TARGET.COMPONENT.KEY on the line \"%s\"\n",
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

  memcpy(principle, file + start, principle_length);
  principle[principle_length] = 0;

  if(component_filter[0] != '*')
  {
   if(component_length != component_filter_length)
    continue;
   if(strncmp(component_filter, file + periods[0] + 1, component_length) != 0)
    continue;
  }

  memcpy(component, file + periods[0] + 1, component_length);
  component[component_length] = 0;

  if(key_filter[0] != '*')
  {
   if(key_length != key_filter_length) 
    continue;
   if(strncmp(key_filter, file + periods[1] + 1, key_length) != 0)
    continue; 
  }

  memcpy(key, file + periods[1] + 1, key_length);
  key[key_length] = 0;

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

int check_project_component_types(struct bca_context *ctx)
{
 int handled, offset = -1;
 char *string, type[256], component[256], key[256];

 while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                              ctx->project_configuration_length, &offset,
                              NULL, NULL, NULL,
                              type, component, key, NULL))
 {
  handled = 0;

  if(strcmp(type, "NONE") == 0)
   handled = 1;

  if(strcmp(type, "BINARY") == 0)
   handled = 1;

  if(strcmp(type, "BUILDBINARY") == 0)
   handled = 1;

  if(strcmp(type, "SHAREDLIBRARY") == 0)
   handled = 1;

  if(strcmp(type, "STATICLIBRARY") == 0)
   handled = 1;

  if(strcmp(type, "CAT") == 0)
   handled = 1;

  if(strcmp(type, "MACROEXPAND") == 0)
   handled = 1;

  if(strcmp(type, "PYTHONMODULE") == 0)
   handled = 1;

  if(strcmp(type, "CUSTOM") == 0)
   handled = 1;

  if(handled == 0)
  {
   fprintf(stderr, "BCA: WARNING - Are you sure about a project component type of \"%s\"?\n", type);
  }

  handled = 0;

  if(strcmp(key, "PROJECT_NAME") == 0)
   handled = 1;

  if(strcmp(key, "NAME") == 0)
   handled = 1;

  if(strcmp(key, "MAJOR") == 0)
   handled = 1;

  if(strcmp(key, "MINOR") == 0)
   handled = 1;

  if(strcmp(key, "AUTHOR") == 0)
   handled = 1;

  if(strcmp(key, "EMAIL") == 0)
   handled = 1;

  if(strcmp(key, "URL") == 0)
   handled = 1;

  if(strcmp(key, "FILES") == 0)
   handled = 1;

  if(strcmp(key, "INPUT") == 0)
   handled = 1;

  if(strcmp(key, "DRIVER") == 0)
   handled = 1;

  if(strcmp(key, "INCLUDE_DIRS") == 0)
   handled = 1;

  if(strcmp(key, "FILE_DEPENDS") == 0)
   handled = 1;

  if(strcmp(key, "INT_DEPENDS") == 0)
   handled = 1;

  if(strcmp(key, "EXT_DEPENDS") == 0)
   handled = 1;

  if(strcmp(key, "OPT_EXT_DEPENDS") == 0)
   handled = 1;

  if(strcmp(key, "LIB_HEADERS") == 0)
   handled = 1;

  if(strcmp(key, "DISABLES") == 0)
   handled = 1;

  if(handled == 0)
  {
   fprintf(stderr, "BCA: WARNING - Are you sure about a project component key of \"%s\"?\n", key);
  }

 } 

 return 0;
}

int list_project_components(struct bca_context *ctx, 
                            struct component_details *cd)
{
 int pass = 0, allocation_size, string_length, offset, i, disabled;
 char *string, type[256], component[256], key[256];

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_project_components()\n");

 while(pass < 2)
 {
  offset = -1;
  cd->n_components = 0;
  while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                               ctx->project_configuration_length, &offset,
                               NULL, NULL, "NAME",
                               type, component, key, NULL))
  {
   i = 0;
   disabled = 0;
   while(i < ctx->n_disables)
   {
    if(strcmp(component, ctx->disabled_components[i]) == 0)
    {
     disabled = 1;
     break;
    }
    i++;
   }

   if(pass == 0)
   {
    /* validate and count */
    if(disabled == 0)
     cd->n_components++;
   }

   if( (pass == 1) && (disabled == 0) )
   {
    string_length = strlen(component);
    allocation_size = string_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: malloc(%d) failed 1\n", allocation_size);
//free array
     return 1;
    }

    snprintf(string, allocation_size, "%s", component);
    cd->project_components[cd->n_components] = string;


    string_length = strlen(type);
    allocation_size = string_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: malloc(%d) failed 2\n", allocation_size);
//free array
     return 1;
    }

    snprintf(string, allocation_size, "%s", type);
    cd->project_component_types[cd->n_components] = string;


    if((cd->project_output_names[cd->n_components] = 
        lookup_key(ctx, ctx->project_configuration_contents,
                   ctx->project_configuration_length, type, component, "NAME")) == NULL)
    {
     fprintf(stderr, "BCA: lookup_key() failed 3\n");
     return 1;
    }

    cd->n_components++;
   }

  }

  if(pass == 0)
  {
   if(cd->n_components == 0)
   {
    cd->project_components = NULL;
    cd->project_component_types = NULL;
    cd->project_output_names = NULL;
    return 0;
   }

   /* allocate array of pointers */
   allocation_size = cd->n_components * sizeof(char *);
   if((cd->project_components = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 4\n", allocation_size);
    return 1;
   }

   if((cd->project_component_types = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 5\n", allocation_size);
    free(cd->project_components);
    return 1;
   }

   if((cd->project_output_names = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 6\n", allocation_size);
    free(cd->project_components);
    free(cd->project_component_types);
    return 1;
   }

  }

  pass++;
 }

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

  if(strcmp(principle, "BUILD") == 0)
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


char *resolve_build_host_variable(struct bca_context *ctx, 
                                  char *host, 
                                  char *project_component,
                                  char *key, char *default_value)
{
 char *value;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: resolve_build_host_variable()\n");

 if(project_component == NULL)
  project_component = "ALL";

 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length, 
                        host, project_component, key)) == NULL)
 {
  if((value = 
      lookup_key(ctx, ctx->build_configuration_contents, ctx->build_configuration_length, 
                 host, "ALL", key)) == NULL)
  {
   if(project_component != NULL)
    if(default_value != NULL)
     fprintf(stderr, 
             "BCA: Can not resolve component %s's (or ALL's) %s for host %s. "
             "Defaulting to \"%s\".\n", 
             project_component, key, host, default_value);

   if(default_value != NULL)
    value = strdup(default_value);
   else
    value = NULL;
  }
 } 

 return value;
}

struct host_configuration *
resolve_host_build_configuration(struct bca_context *ctx, struct component_details *cd)
{
 int allocation_size;
 struct host_configuration *tc;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: resolve_host_build_configuration()\n");
  fflush(stderr);
 }

 allocation_size = sizeof(struct host_configuration);
 if((tc = (struct host_configuration *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }
 memset(tc, 0, allocation_size);

 tc->cc = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component, "CC", NULL);

 tc->cc_output_flag = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "CC_SPECIFY_OUTPUT_FLAG", NULL);

 tc->cc_compile_bin_obj_flag = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "CC_COMPILE_BIN_OBJ_FLAG", NULL);

 tc->cc_include_dir_flag = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "CC_INCLUDE_DIR_FLAG", NULL);

 tc->cc_define_macro_flag = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "CC_DEFINE_MACRO_FLAG", NULL);

 tc->cflags = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "CFLAGS", NULL);

 tc->pkg_config = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "PKG_CONFIG", NULL);

 tc->pkg_config_path = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "PKG_CONFIG_PATH", NULL);

 tc->pkg_config_libdir = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "PKG_CONFIG_LIBDIR", NULL);

 tc->binary_suffix = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "BINARY_SUFFIX", NULL);

 tc->obj_suffix = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "OBJ_SUFFIX", NULL);

 tc->ldflags = 
  resolve_build_host_variable(ctx, "BUILD", cd->project_component,
                              "LDFLAGS", NULL);

 return tc;
}

struct host_configuration *
resolve_host_configuration(struct bca_context *ctx, struct component_details *cd)
{
 int allocation_size;
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

 tc->build_prefix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "BUILD_PREFIX", NULL);

 tc->build_tc = resolve_host_build_configuration(ctx, cd);

 tc->cc = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC", NULL);

 tc->cc_output_flag = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC_SPECIFY_OUTPUT_FLAG", NULL);

 tc->cc_compile_bin_obj_flag = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC_COMPILE_BIN_OBJ_FLAG", NULL);

 tc->cc_compile_shared_library_obj_flag = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG", NULL);

 tc->cc_include_dir_flag = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC_INCLUDE_DIR_FLAG", NULL);

 tc->cc_define_macro_flag = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CC_DEFINE_MACRO_FLAG", NULL);

 tc->cflags = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "CFLAGS", NULL);

 tc->pkg_config = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "PKG_CONFIG", NULL);

 tc->pkg_config_path = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "PKG_CONFIG_PATH", NULL);

 tc->pkg_config_libdir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "PKG_CONFIG_LIBDIR", NULL);

 tc->binary_suffix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "BINARY_SUFFIX", NULL);

 tc->shared_library_suffix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "SHARED_LIBRARY_SUFFIX", NULL);

 tc->shared_library_prefix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "SHARED_LIBRARY_PREFIX", NULL);

 tc->obj_suffix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "OBJ_SUFFIX", NULL);

 tc->ldflags = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "LDFLAGS", NULL);

 tc->install_prefix = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_PREFIX", NULL);

 tc->install_bin_dir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_BIN_DIR", NULL);

 tc->install_lib_dir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_LIB_DIR", NULL);

 tc->install_include_dir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_INCLUDE_DIR", NULL);

 tc->install_pkg_config_dir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_PKG_CONFIG_DIR", NULL);

 tc->install_locale_data_dir = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "INSTALL_LOCALE_DATA_DIR", NULL);

 tc->python = 
  resolve_build_host_variable(ctx, cd->host, cd->project_component,
                              "PYTHON", NULL);

 if(ctx->verbose > 1)
 {
  printf("BCA: %s.%s.CC resolves to %s\n", 
         cd->host, cd->project_component, tc->cc);
  printf("BCA: %s.%s.BUILD_PREFIX resolves to %s\n", 
         cd->host, cd->project_component, tc->build_prefix);
  printf("BCA: %s.%s.CC_SPECIFY_OUTPUT_FLAG resolves to %s\n", 
         cd->host, cd->project_component, tc->cc_output_flag);
  printf("BCA: %s.%s.CC_COMPILE_BIN_OBJ_FLAG resolves to %s\n", 
         cd->host, cd->project_component, tc->cc_compile_bin_obj_flag);
  printf("BCA: %s.%s.CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG resolves to %s\n", 
         cd->host, cd->project_component, tc->cc_compile_shared_library_obj_flag);
  printf("BCA: %s.%s.CC_INCLUDE_DIR_FLAG resolves to %s\n", 
         cd->host, cd->project_component, tc->cc_include_dir_flag);
  printf("BCA: %s.%s.CC_DEFINE_MACRO_FLAG resolves to %s\n", 
         cd->host, cd->project_component, tc->cc_define_macro_flag);
  printf("BCA: %s.%s.BINARY_SUFFIX resolves to %s\n", 
         cd->host, cd->project_component, tc->binary_suffix);
  printf("BCA: %s.%s.OBJ_SUFFIX resolves to %s\n", 
         cd->host, cd->project_component, tc->obj_suffix);
  printf("BCA: %s.%s.SHARED_LIBRARY_SUFFIX resolves to %s\n", 
         cd->host, cd->project_component, tc->shared_library_suffix);
  printf("BCA: %s.%s.SHARED_LIBRARY_PREFIX resolves to %s\n", 
         cd->host, cd->project_component, tc->shared_library_prefix);
  printf("BCA: %s.%s.PKG_CONFIG resolves to %s\n", 
         cd->host, cd->project_component, tc->pkg_config);
  printf("BCA: %s.%s.PKG_CONFIG_PATH resolves to %s\n", 
         cd->host, cd->project_component, tc->pkg_config_path);
  printf("BCA: %s.%s.PKG_CFLAGS to %s\n", 
         cd->host, cd->project_component, tc->cflags);
  printf("BCA: %s.%s.PKG_LDFLAGS to %s\n", 
         cd->host, cd->project_component, tc->ldflags);
  printf("BCA: %s.%s.INSTALL_BIN_DIR to %s\n", 
         cd->host, cd->project_component, tc->install_bin_dir);
  printf("BCA: %s.%s.INSTALL_LIB_DIR to %s\n", 
         cd->host, cd->project_component, tc->install_lib_dir);
  printf("BCA: %s.%s.INSTALL_INCLUDE_DIR to %s\n", 
         cd->host, cd->project_component, tc->install_include_dir);
  printf("BCA: %s.%s.INSTALL_PKG_CONFIG_DIR to %s\n", 
         cd->host, cd->project_component, tc->install_pkg_config_dir);
  printf("BCA: %s.%s.PYTHON to %s\n", 
         cd->host, cd->project_component, tc->python);

  fflush(stdout);
 }

 return tc;
}

int free_host_configuration(struct bca_context *ctx, struct host_configuration *tc)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: fee_host_configuration()\n");

 if(tc != NULL) 
 {
  if(tc->build_tc != NULL)
   free_host_configuration(ctx, tc->build_tc);

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

  if(tc->cc_specify_output_flag != NULL)
   free(tc->cc_specify_output_flag);
 
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
           "BCA: No DEPENDS key found for component \"%s\" on host \"%s\", implying dependencies.\n", 
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
           "BCA: resolve_component_input_dependencies(): project component %s apears to list itself as an INPUT element\n",
           cd->project_components[component_index]);

   free_string_array(list, n_elements);
   return 1;
  }

  if(i  == cd->n_components)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project component %s list INPUT element"
           " %s that is not itself a project component. Perhapes the element belongs in a FILES record?\n",
           cd->project_components[component_index], list[z]);
   free_string_array(list, n_elements);
   return 1;
  }

  if(add_to_string_array(&(cd->inputs), cd->n_inputs, cd->project_output_names[i], -1, 1))
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
          "BCA: should not happen: substring not in .ALL. value being to be copied and modified\n");
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

int check_value(struct bca_context *ctx)
{
 char *contents, *file, **values, *value, *q;
 int length, n_values, i, value_length;

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
 
 q = ctx->qualifier;
 if((value = lookup_key(ctx, contents, length, ctx->principle, 
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
           ctx->principle, ctx->qualifier, ctx->search_value_key, file);

  if(strcmp(ctx->qualifier, "ALL") != 0)
  {
   if((value = lookup_key(ctx, contents, length, ctx->principle, 
                          "ALL", ctx->search_value_key)) == NULL)
   {
    if(ctx->verbose)
     fprintf(stderr,
             "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
             ctx->principle, "ALL", ctx->search_value_key, file);
 
    return 0;
   }
   q = "ALL";
  } else {
   return 0;
  }
 }

 value_length = strlen(value);

 if(split_strings(ctx, value, value_length, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 for(i=0; i<n_values; i++)
 {
  if(strcmp(values[i], ctx->new_value_string) == 0)
  {
   if(ctx->verbose)
    fprintf(stderr, 
            "BCA: '%s' found in %s.%s.%s = '%s'. Returning 2.\n", 
            ctx->new_value_string, ctx->principle, q, ctx->search_value_key, value);

   free_string_array(values, n_values);
   free(value);
   free(contents);
   return 2;
  }
 }

 if(ctx->verbose)
  fprintf(stderr, "BCA: '%s' not found in %s.%s.%s. Returning 3\n", 
          ctx->new_value_string, ctx->principle, ctx->qualifier, ctx->search_value_key);

 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 3;
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

int resolve_component_installation_path(struct bca_context *ctx, char *component_type, 
                                        char *component, char **path)
{
 char *avalue, *bvalue;
 char temp[1024];
 int offset;

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

 if(avalue != NULL)
 {
  if(strcmp(avalue, "NONE") == 0)
  {
   *path = NULL;
  }

  if(strncmp(avalue, "${PREFIX}", 9) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_PREFIX")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_PREFIX");
   }
   offset = 9;
  }

  if(strncmp(avalue, "${BIN_DIR}", 10) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_BIN_DIR")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_BIN_DIR");
   }
   offset = 10;
  }

  if(strncmp(avalue, "${LIB_DIR}", 10) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_LIB_DIR")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_LIB_DIR");
   }
   offset = 10;
  }

  if(strncmp(avalue, "${INCLUDE_DIR}", 14) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_INCLUDE_DIR")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_INCLUDE_DIR");
   }
   offset = 14;
  }

  if(strncmp(avalue, "${PKG_CONFIG_DIR}", 17) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_PKG_CONFIG_DIR")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_PKG_CONFIG_DIR");
   }
   offset = 17;
  }

  if(strncmp(avalue, "${LOCALE_DATA_DIR}", 18) == 0)
  {
   if((bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                           ctx->build_configuration_length, 
                           ctx->principle, component, "INSTALL_LOCALE_DATA_DIR")) == NULL)
   {
    bvalue = lookup_key(ctx, ctx->build_configuration_contents, 
                        ctx->build_configuration_length, 
                        ctx->principle, "ALL", "INSTALL_LOCALE_DATA_DIR");
   }
   offset = 18;
  }

  if(bvalue != NULL)
  {
   snprintf(temp, 1024, "%s%s", bvalue, avalue + offset);
   *path = strdup(temp);
   free(bvalue);
  } else {
   fprintf(stderr, 
           "BCA: resolve_component_installation_path(): component %s INSTALL_PATH %s failed.\n",
           component, avalue);
   return 1;
  }

  free(avalue);
  return 0;
 }

 return 1;
}

