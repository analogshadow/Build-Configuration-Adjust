/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2011,2012,2013,2014 Stover Enterprises, LLC
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

int line_number = 1;

int parse_function_parameters(char *string, char ***array, int *array_length)
{
 int length, i, mark = -1;

 *array = NULL;
 *array_length = 0;
 length = strlen(string);

 if(string[length - 1] != ')')
 {
  fprintf(stderr, "BCA: string '%s' should end with a ')'\n", string);
  return 1;
 }
 length -= 1;

 i = 0;
 while(i< length)
 {
  switch(string[i])
  {
   case '(':
        if(mark != -1)
        {
         fprintf(stderr, "BCA: string '%s' should not have more than one '('\n", string);
         free_string_array(*array, *array_length);
         return 1;
        }
        if(add_to_string_array(array, *array_length,
                               string, i, 0))
        {
         fprintf(stderr, "BCA: add_to_string_array() failed\n");
         return 1;
        }
        (*array_length)++;
        mark = i + 1;
        break;

  case ',':
        if(add_to_string_array(array, *array_length,
                               string + mark, i - mark, 0))
        {
         fprintf(stderr, "BCA: add_to_string_array() failed\n");
         free_string_array(*array, *array_length);
         return 1;
        }
        (*array_length)++;

       mark = i + 1;
       break;

  }

  i++;
 }

 if(i - mark > 0)
 {
  if(add_to_string_array(array, *array_length,
                         string + mark, i - mark, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   free_string_array(*array, *array_length);
   return 1;
  }
  (*array_length)++;
 }

 return 0;
}

char *check_function(struct bca_context *ctx, char *key)
{
 char **parameters, *contents, *p, *q, *k, *c, *result;
 int n_parameters, code, length, i;

 if(parse_function_parameters(key, &parameters, &n_parameters))
 {
  fprintf(stderr, "BCA: parse_function_parameters(%s) failed\n", key);
  return NULL;
 }

 if(ctx->verbose > 1)
 {
  fprintf(stderr, "BCA: trying CHECK(");
  for(i=1; i<n_parameters; i++)
  {
   fprintf(stderr, "%s", parameters[i]);
   if(i + 1 < n_parameters)
    fprintf(stderr, ",");
  }
  fprintf(stderr, ")\n");
 }
  /* 0) CHECK
    1) BUILD | PROJECT
    2) CURRENT | HOSTNAME
    3) COMPONENT | ALL
    4) KEY
    5) CHECK-VALUE
 */

 if(n_parameters != 6)
 {
  fprintf(stderr, "BCA: CHECK() macro function expects 5 parameters, not %d\n", n_parameters - 1);
  free_string_array(parameters, n_parameters);
  return NULL;
 }

 if(strcmp(parameters[1], "BUILD") == 0)
 {

  if(ctx->build_configuration_contents == NULL)
  {
   if((ctx->build_configuration_contents =
       read_file("./buildconfiguration/buildconfiguration",
                 &(ctx->build_configuration_length), 0)) == NULL)
   {
    fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
    free_string_array(parameters, n_parameters);
    return NULL;
   }
  }

  contents = ctx->build_configuration_contents;
  length = ctx->build_configuration_length;

 } else if(strcmp(parameters[1], "PROJECT") == 0) {

  if(ctx->project_configuration_contents == NULL)
  {
   if((ctx->project_configuration_contents =
        read_file("./buildconfiguration/projectconfiguration",
                  &(ctx->project_configuration_length), 0)) == NULL)
   {
    free_string_array(parameters, n_parameters);
    return NULL;
   }
  }

  contents = ctx->project_configuration_contents;
  length = ctx->project_configuration_length;

 } else {
  fprintf(stderr, "BCA: CHECK() first parameters should be BUILD or PROJECT\n");
  free_string_array(parameters, n_parameters);
  return NULL;
 }

 if(strcmp(parameters[2], "CURRENT") == 0)
 {
  p = ctx->principle;
 } else {
  p = parameters[2];
 }
 q = parameters[3];
 k = parameters[4];
 c = parameters[5];

 if((code = check_value_inline(ctx, contents, length,
                               p, q, k, c)) < 0)
 {
  fprintf(stderr, "BCA: check_function(%s): check_value_inline() failed\n", key);
  return NULL;
 }

 free_string_array(parameters, n_parameters);
 if((result = malloc(3)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(3) failed\n");
  return NULL;
 }

 snprintf(result, 3, "%d", code);
 return result;
}

char *file_to_C_source_function(struct bca_context *ctx, char *key)
{
 char **parameters, *contents;
 int n_parameters, i;

 if(parse_function_parameters(key, &parameters, &n_parameters))
 {
  fprintf(stderr, "BCA: parse_function_parameters(%s) failed\n", key);
  return NULL;
 }

 if(ctx->verbose > 1)
 {
  fprintf(stderr, "BCA: trying FILE_TO_C_SOURCE(");
  for(i=1; i<n_parameters; i++)
  {
   fprintf(stderr, "%s", parameters[i]);
   if(i + 1 < n_parameters)
    fprintf(stderr, ",");
  }
  fprintf(stderr, ")\n");
 }

 if(n_parameters != 2)
 {
  fprintf(stderr, "BCA: FILE_TO_C_SOURCE() needs a file name\n");
  free_string_array(parameters, n_parameters);
  return NULL;
 }

 if(file_to_C_source(ctx, parameters[1]) != 0)
 {
  fprintf(stderr, "BCA: file_to_C_source() failed\n");
  return NULL;
 }

 return strdup("");
}

char *resolve_string_replace_key(struct bca_context *ctx, char *key)
{
 char *value, a[256], b[256], c[256], **list = NULL, **withouts = NULL, *without_macro;
 int mode = 0, n_dots = 0, dots[2], length, i, n_items = 0, n_withouts = 0,
     edition, allocation_size;

/*package config, project & build config, targets, withouts, disables, versions,
  paths, build name, date, tests?
*/

 if(ctx->verbose > 1)
 {
  fprintf(stderr, "BCA: resolve_string_replace_key(%s)\n", key);
 }

 if(strncmp(key, "ENV.", 4) == 0)
 {
  if((value = getenv(key + 4)) == NULL)
  {
   return NULL;
  }
  return strdup(value);
 }

 if(key[0] == 'd')
 {
#ifndef IN_SINGLE_FILE_DISTRIBUTION
  return handle_document_functions(ctx, key);
#else
  fprintf(stderr,
          "BCA: macro key startint with 'd' is likely a document handling function. "
          "Document processing macros are not in the single file distribution.\n");
  return NULL;
#endif
 }

 if(strncmp(key, "CHECK(", 6) == 0)
 {
  return check_function(ctx, key);
 }

 if(strncmp(key, "FILE_TO_C_SOURCE(", 17) == 0)
 {
  return file_to_C_source_function(ctx, key);
 }

 if(strncmp(key, "BCA.BUILDIR", 11) == 0)
 {
#ifdef HAVE_CWD
  return strdup(ctx->cwd);
#else
  fprintf(stderr, "BCA: fixme, I've been built without cwd() and need that for @BCA.BUILDDIR@\n");
  return NULL;
#endif
 }

 if(strncmp(key, "BCA.MACROS.", 11) == 0)
 {
  length = strlen(key + 11);

  memcpy(a, key + 11, length);
  a[length] = 0;

  if(ctx->build_configuration_contents == NULL)
  {
   if((ctx->build_configuration_contents =
       read_file("./buildconfiguration/buildconfiguration",
                 &(ctx->build_configuration_length), 0)) == NULL)
   {
    fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
    return NULL;
   }
  }

  value = lookup_key(ctx, ctx->build_configuration_contents,
                     ctx->build_configuration_length,
                     ctx->principle, a, "MACROS");

  if(value == NULL)
  {
   if(strcmp(b, "ALL") == 0)
   {
    if(ctx->verbose > 1)
     fprintf(stderr, "BCA: no %s.%s.%s\n", ctx->principle, a, "MACROS");
   }

   value = lookup_key(ctx, ctx->build_configuration_contents,
                      ctx->build_configuration_length,
                      ctx->principle, "ALL", "MACROS");

   if(value == NULL)
   {
    if(ctx->verbose > 1)
     fprintf(stderr, "BCA: no %s.%s.%s\n", ctx->principle, "ALL", "MACROS");
   }
  }

  if(value != NULL)
  {
   if(split_strings(ctx, value, -1, &n_items, &list))
   {
    fprintf(stderr, "BCA: split_strings() failed\n");
    return NULL;
   }
   free(value);
   value = NULL;
  }

  if((value = lookup_key(ctx, ctx->build_configuration_contents,
                         ctx->build_configuration_length,
                         ctx->principle, a, "WITHOUTS")) == NULL)
  {
   if((value = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          ctx->principle, "ALL", "WITHOUTS")) == NULL)
   {
    fprintf(stderr,
            "BCA: WARNING: Could not find %s.[%s|ALL].WITHOUTS as needed for @%s@. Are you sure you mean component name \"%s\"?\n",
            ctx->principle, a, key, a);
   }
  }

  if(value != NULL)
  {
   if(split_strings(ctx, value, -1, &n_withouts, &withouts))
   {
    fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
    return NULL;
   }
  } else {
   withouts = NULL;
   n_withouts = 0;
  }

  free(value);

  allocation_size = 1;
  for(i=0; i<n_items; i++)
  {
   allocation_size += strlen(list[i]) + 12;
  }

  for(i=0; i < n_withouts; i++)
  {
   allocation_size += strlen(withouts[i]) + 20;
  }

  if((value = (char *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
   return NULL;
  }
  value[0] = 0;

  length = 0;
  for(i=0; i<n_items; i++)
  {
   length += snprintf(value + length, allocation_size - length,
                      "#define %s\n", list[i]);
  }

  for(i=0; i < n_withouts; i++)
  {
   if((without_macro = without_string_to_without_macro(ctx, withouts[i])) != NULL)
   {
    length += snprintf(value + length, allocation_size - length,
                       "#define WITHOUT_%s\n", without_macro);
    free(without_macro);
    without_macro = NULL;
   } else {
    fprintf(stderr, "BCA: without_string_to_without_macro(%s) failed\n", withouts[i]);
   }
  }

  free_string_array(withouts, n_withouts);
  free_string_array(list, n_items);
  return value;
 }

 if(strncmp(key, "BCA.OUTPUTNAME.", 15) == 0)
  mode = 15;

 if(strncmp(key, "BCA.BUILDOUTPUTNAME.", 20) == 0)
  mode = 20;

 if(strncmp(key, "BCA.INSTALLEDNAME.", 18) == 0)
  mode = 18;

 if(strncmp(key, "BCA.EFFECTIVEOUTPUTNAME.", 24) == 0)
  mode = 24;

 if(mode > 0)
 {
  length = strlen(key + mode);
  i = mode;
  dots[0] = dots[1] = -1;
  while(i < mode + length)
  {
   if(key[i] == '[')
    dots[0] = i;

   if(key[i] == ']')
    dots[1] = i;

   i++;
  }

  if( (dots[0] < mode + 1) ||
      (dots[1] < mode + 3) ||
      (length + (mode - 1) != dots[1]) ||
      (dots[1] - (dots[0] + 1) < 1) ||
      (dots[1] > 255 + mode) )
  {
   fprintf(stderr, "BCA: replace key error: problem with format"
           " BCA.OUTPUTNAME.component[i] in \"%s\".\n", key);
   return NULL;
  }

  length = dots[1] - (dots[0] + 1);
  if(length > 2)
   return NULL;
  memcpy(b, key + dots[0] + 1, length);
  b[length] = 0;

  i = -1;
  sscanf(b, "%d", &i);
  if(i < 0)
  {
   fprintf(stderr, "BCA: replace key error: problem with index i of format"
           " BCA.OUTPUTNAME.component[i] in \"%s\".\n", key);
   return NULL;
  }

  length = dots[0] - mode;
  memcpy(a, key + mode, length);
  a[length] = 0;

  switch(mode)
  {
   case 15:
        edition = 1;
        break;

   case 18:
        edition = 3;
        break;

   case 20:
        edition = 2;
        break;

   case 24:
        edition = 4;
        break;
  }

  n_items = render_project_component_output_name(ctx, ctx->principle, a, edition, &list, NULL);
  if(n_items < 1)
  {
   fprintf(stderr, "BCA: replace key note: render_project_component_output_name() yielded "
           "no result for project component \"%s\" on host \"%s\".\n", a, ctx->principle);
   return strdup("");
  }

  if(i < n_items)
  {
   value = strdup(list[i]);
   free_string_array(list, n_items);
   return value;
  } else {
   fprintf(stderr, "BCA: replace key error: index %d out of range in \"%s\".\n",
           i, key);
   return NULL;
  }
 }

 if(strncmp(key, "BCA.PROJECT.", 12) == 0)
  mode = 12;

 if(strncmp(key, "BCA.BUILD.", 10) == 0)
  mode = 10;

 if(mode > 0)
 {
  length = strlen(key + mode);
  i = mode;
  while(i < mode + length)
  {
   if(key[i] == '.')
   {
    if(n_dots == 2)
    {
     fprintf(stderr, "BCA: replace key error: problem with format"
             " BCA.PROJECT|BUILD.principle.qualifier.key in \"%s\" too many periods.\n", key);
     return NULL;
    }
    dots[n_dots++] = i;
   }
   i++;
  }

  if( (n_dots != 2) ||
      (dots[0] - mode < 1) ||
      (dots[1] - (dots[0] + 1) < 1) ||
      ((mode + length) - dots[1] < 1) )
  {
   fprintf(stderr, "BCA: replace key error: problem with format"
           " BCA.PROJECT|BUILD.principle.qualifier.key in \"%s\"\n", key);
   return NULL;
  }

  i = dots[0] - mode;
  if(i > 255)
   return NULL;

  memcpy(a, key + mode, i);
  a[i] = 0;

  i = dots[1] - (dots[0] + 1);
  if(i > 255)
   return NULL;

  memcpy(b, key + dots[0] + 1, i);
  b[i] = 0;

  i = (mode + length + 1) - dots[1];
  if(i > 255)
   return NULL;

  memcpy(c, key + dots[1] + 1, i);
  c[i] = 0;

  if(mode == 10)
  {
   if(ctx->build_configuration_contents == NULL)
   {
    if((ctx->build_configuration_contents =
        read_file("./buildconfiguration/buildconfiguration",
                  &(ctx->build_configuration_length), 0)) == NULL)
    {
     fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
     return NULL;
    }
   }

   if(strcmp(a, "CURRENT") == 0)
    snprintf(a , 256, "%s", ctx->principle);

   if((value = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length, a, b, c)) != NULL)
   {
    return value;
   }

   if(strcmp(b, "ALL") != 0)
   {
    return lookup_key(ctx, ctx->build_configuration_contents,
                      ctx->build_configuration_length, a, "ALL", c);
   }

   return NULL;
  }

  if(mode == 12)
  {
   if(ctx->project_configuration_contents == NULL)
   {
    if((ctx->project_configuration_contents =
         read_file("./buildconfiguration/projectconfiguration",
                   &(ctx->project_configuration_length), 0)) == NULL)
    {
     return NULL;
    }
   }

   if((value = lookup_key(ctx, ctx->project_configuration_contents,
                          ctx->project_configuration_length, a, b, c)) != NULL)
   {
    return value;
   }

   if( ( (strcmp(c, "MAJOR") == 0) || (strcmp(c, "MINOR") == 0) ) &&
        (strcmp(b, "NONE") != 0) )
   {
    return lookup_key(ctx, ctx->project_configuration_contents,
                      ctx->project_configuration_length, "NONE", "NONE", c);
   }

   return NULL;
  }

 }

 return NULL;
}

int string_replace(struct bca_context *ctx)
{
 char c, key[256], *value;
 int index;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: string_replace()\n");

 while(!feof(stdin))
 {
  fscanf(stdin, "%c", &c);
  if(c != '@')
  {
   fprintf(stdout, "%c", c);
  } else {

   index = 0;
   while(!feof(stdin))
   {
    fscanf(stdin, "%c", &c);

    if(c == '\n')
     line_number++;

    if(c != '@')
    {
     if(index > 255)
     {
      fprintf(stderr, "BCA:, string_replace(): key is too long\n");
      return 1;
     }
     key[index++] = c;
    } else {
     break;
    }
   }

   if(index == 0)
   {
    /* @@ escapes out @ */
    fprintf(stdout, "@");
   } else {

    key[index] = 0;

    if((value = resolve_string_replace_key(ctx, key)) == NULL)
    {
     fprintf(stderr,
             "BCA: string_replace(): could not resolve key \"%s\", line %d\n",
             key, line_number);
     return 1;
    }

    fprintf(stdout, "%s", value);
    free(value);
   }

  }

 }

 return 0;
}
