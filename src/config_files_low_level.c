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

char *lookup_key(struct bca_context *ctx, char *file, int file_length,
                 char *principle_filter, char *component_filter, char *key_filter)
{
 char principle[256], component[256], key[256];
 char *value = NULL;
 int start, index, equals, in_quotes, end, value_length, offset;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: lookup_key(, %s.%s.%s)\n", principle_filter, component_filter, key_filter);

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

int lookup_value_as_list(struct bca_context *ctx, int operational_mode,
                         char *principle_filter, char *qualifier_filter, char *key_filter,
                         char ***list, int *n_elements)
{
 char *file_contents, *value;
 int file_length;

 *list = NULL;
 *n_elements = 0;

 switch(operational_mode)
 {
  case OPERATE_PROJECT_CONFIGURATION:
       file_contents = ctx->project_configuration_contents;
       file_length = ctx->project_configuration_length;
       break;

  case OPERATE_BUILD_CONFIGURATION:
       file_contents = ctx->build_configuration_contents;
       file_length = ctx->build_configuration_length;
       break;

  default:
       return 1;
 }

 if((value = lookup_key(ctx, file_contents, file_length,
                        principle_filter, qualifier_filter, key_filter)) == NULL)
  return 0;

 if(split_strings(ctx, value, -1, n_elements, list))
 {
  fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int iterate_over_values(struct bca_context *ctx, int operational_mode, void *data,
                        char *principle_filter, char *qualifier_filter, char *key_filter,
                        int (*callback) (struct bca_context *ctx,
                                         char *p, char *q, char *k, char *v, void *data) )
{
 char o_principle[256], o_qualifier[256], o_key[256];
 char *file_contents, *value;
 int file_length, end, code;

 switch(operational_mode)
 {
  case OPERATE_PROJECT_CONFIGURATION:
       file_contents = ctx->project_configuration_contents;
       file_length = ctx->project_configuration_length;
       break;

  case OPERATE_BUILD_CONFIGURATION:
       file_contents = ctx->build_configuration_contents;
       file_length = ctx->build_configuration_length;
       break;

  default:
       return 1;
 }

 end = -1;
 while(iterate_key_primitives(ctx, file_contents, file_length, &end,
                              principle_filter, qualifier_filter, key_filter,
                              o_principle, o_qualifier, o_key, NULL))
 {
  value = lookup_key(ctx, file_contents, file_length,
                     o_principle, o_qualifier, o_key);

  code = callback(ctx, o_principle, o_qualifier, o_key, value, data);

  free(value);

  if(code != 0)
   return code;
 }

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
  case OPERATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case OPERATE_BUILD_CONFIGURATION:
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
  case OPERATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case OPERATE_BUILD_CONFIGURATION:
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
  case OPERATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case OPERATE_BUILD_CONFIGURATION:
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
  case OPERATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case OPERATE_BUILD_CONFIGURATION:
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
  case OPERATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case OPERATE_BUILD_CONFIGURATION:
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

struct file_modification_set *
file_modification_set_init(void)
{
 struct file_modification_set *fms;
 int allocation_size;

 allocation_size = sizeof(struct file_modification_set);
 if((fms = (struct file_modification_set *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          allocation_size, strerror(errno));
  return NULL;
 }
 memset(fms, 0, allocation_size);

 return fms;
}

int file_modification_set_free(struct file_modification_set *fms)
{
 if(fms != NULL)
 {
  if(fms->mod_principles != NULL)
   if(free_string_array(fms->mod_principles, fms->n_modify_records))
    return 1;

  if(fms->mod_components != NULL)
   if(free_string_array(fms->mod_components, fms->n_modify_records))
    return 1;

  if(fms->mod_keys != NULL)
   if(free_string_array(fms->mod_keys, fms->n_modify_records))
    return 1;

  if(fms->mod_values != NULL)
   if(free_string_array(fms->mod_values, fms->n_modify_records))
    return 1;

  free(fms);
 }
 return 0;
}

int file_modification_set_append(struct file_modification_set *fms,
                                 char *principle, int p_length,
                                 char *qualifier, int q_length,
                                 char *key, int k_length,
                                 char *value, int v_length)
{
 if(add_to_string_array(&(fms->mod_values),
                        fms->n_modify_records,
                        value, v_length, 0))
  return 1;

 if(add_to_string_array(&(fms->mod_principles),
                        fms->n_modify_records,
                        principle, p_length, 0))
  return 1;

 if(add_to_string_array(&(fms->mod_components),
                        fms->n_modify_records,
                        qualifier, q_length, 0))
  return 1;

 if(add_to_string_array(&(fms->mod_keys),
                        fms->n_modify_records,
                        key, k_length, 0))
  return 1;

 fms->n_modify_records++;

 return 0;
}

int file_modification_set_apply(struct bca_context *ctx, FILE *output,
                                char *contents, int length,
                                struct file_modification_set *fms)
{
 return output_modifications(ctx, output, contents, length,
                             fms->n_modify_records,
                             fms->mod_principles,
                             fms->mod_components,
                             fms->mod_keys,
                             fms->mod_values);
}

int file_modification_set_print(struct file_modification_set *fms, FILE *output)
{
 int i;

 fprintf(output, "BCA: %d modification records:\n",
         fms->n_modify_records);

 for(i=0; i < fms->n_modify_records; i++)
 {
  if(fms->mod_values[i] != NULL)
   fprintf(output, "BCA: (add/edit) %s.%s.%s = %s\n",
           fms->mod_principles[i],
           fms->mod_components[i],
           fms->mod_keys[i],
           fms->mod_values[i]);
  else
   fprintf(output, "BCA: (remove) %s.%s.%s\n",
           fms->mod_principles[i],
           fms->mod_components[i],
           fms->mod_keys[i]);
 }

 return 0;
}

int load_project_config(struct bca_context *ctx, int test)
{
 if(ctx->project_configuration_contents == NULL)
 {
  if((ctx->project_configuration_contents =
      read_file("./buildconfiguration/projectconfiguration",
                &(ctx->project_configuration_length), test)) == NULL)
  {
   if(test == 0)
    fprintf(stderr, "BCA: can't open project configuration file\n");
   return 1;
  }
 }

 return 0;
}

int load_build_config(struct bca_context *ctx, int test)
{
 if(ctx->build_configuration_contents == NULL)
 {
  if((ctx->build_configuration_contents =
      read_file("./buildconfiguration/buildconfiguration",
                &(ctx->build_configuration_length), test)) == NULL)
  {
   if(test == 0)
    fprintf(stderr, "BCA: can't open build configuration file\n");
   return 1;
  }
 }

 return 0;
}

