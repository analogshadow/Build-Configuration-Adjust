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
   ----------------------
    5) CHECK-VALUE
 */

 if( (n_parameters < 5) || (n_parameters > 6) )
 {
  fprintf(stderr, "BCA: CHECK() macro function expects 4 or 5 parameters, not %d\n", n_parameters - 1);
  free_string_array(parameters, n_parameters);
  return NULL;
 }

 if(strcmp(parameters[1], "BUILD") == 0)
 {

  if(load_build_config(ctx, 0))
   return NULL;

  contents = ctx->build_configuration_contents;
  length = ctx->build_configuration_length;

 } else if(strcmp(parameters[1], "PROJECT") == 0) {

  if(load_project_config(ctx, 0))
   return NULL;

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


 if(n_parameters == 6)
 {
  c = parameters[5];

  if((code = check_value_inline(ctx, contents, length,
                                p, q, k, c)) < 0)
  {
   fprintf(stderr, "BCA: check_function(%s): check_value_inline() failed\n", key);
   return NULL;
  }
 } else {

  if((result = lookup_key(ctx, contents, length, p, q, k)) == NULL)
  {
   code = 0;
  } else {
   free(result);
   result = NULL;
   code = 1;
  }

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

char *lookupor_function(struct bca_context *ctx, char *key)
{
 char **parameters, *contents, *p, *q, *k, *result;
 int n_parameters, length, i;

 if(parse_function_parameters(key, &parameters, &n_parameters))
 {
  fprintf(stderr, "BCA: parse_function_parameters(%s) failed\n", key);
  return NULL;
 }

 if(ctx->verbose > 1)
 {
  fprintf(stderr, "BCA: trying LKUPOR(");
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
    5) return value if not found
 */

 if(n_parameters != 6)
 {
  fprintf(stderr, "BCA: LKUPOR() macro function expects 5 parameters, not %d\n", n_parameters - 1);
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
  fprintf(stderr, "BCA: FILL() first parameters should be BUILD or PROJECT\n");
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

 if((result = lookup_key(ctx, contents, length, p, q, k)) == NULL)
 {
  result = strdup(parameters[5]);
 }

 free_string_array(parameters, n_parameters);
 return result;
}


char *file_to_C_source_function(struct bca_context *ctx, char *key)
{
 char **parameters;
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

char *handle_bca_config_h_macros(struct bca_context *ctx)
{
 char *result, *value, *without_macro,
      **opt_int_deps, **withouts, **disables, **macros;
 int n_opt_int_deps, n_withouts, n_disables, n_macros, i, j, yes, code,
     allocation_size, length;

 /* we are going to need details from both project and build config files
    so load them now if not already */

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

 if(ctx->project_configuration_contents == NULL)
 {
  if((ctx->project_configuration_contents =
       read_file("./buildconfiguration/projectconfiguration",
                 &(ctx->project_configuration_length), 0)) == NULL)
  {
   fprintf(stderr, "BCA: could not read ./buildconfiguration/projectconfiguration\n");
   return NULL;
  }
 }

 /* gather needed lists */
 macros = NULL;
 n_macros = 0;
 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        ctx->principle, "ALL", "MACROS")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_macros, &macros))
  {
   fprintf(stderr, "BCA: split_strings() failed\n");
   return NULL;
  }

  free(value);
 }

 withouts = NULL;
 n_withouts = 0;
 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        ctx->principle, "ALL", "WITHOUTS")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_withouts, &withouts))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return NULL;
  }
  free(value);
 }

 disables = NULL;
 n_disables = 0;
 if((value = lookup_key(ctx, ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        ctx->principle, "ALL", "DISABLES")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_disables, &disables))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return NULL;
  }

  free(value);
 }

 opt_int_deps = NULL;
 n_opt_int_deps = 0;

 if(list_unique_opt_int_depends(ctx, &opt_int_deps, &n_opt_int_deps, 0))
  return NULL;

 /* DISABLES that are also OPT_INT_DEPENDS in the project also get WITHOUT_x CPP macros */
 for(j=0; j<n_disables; j++)
 {
  yes = 0;
  i = 0;
  while(i<n_opt_int_deps)
  {
   if(strcmp(disables[j], opt_int_deps[i]) == 0)
   {
    yes = 1;
    break;
   }
   i++;
  }

  if(yes == 1)
  {
   if((code = add_to_string_array(&withouts, n_withouts, disables[j], -1, 1)) < 0)
   {
    return NULL;
   }
   if(code == 0)
    n_withouts++;
  }
 }

 /* allocate result buffer */
 allocation_size = 1;
 for(i=0; i<n_macros; i++)
 {
  allocation_size += strlen(macros[i]) + 12;
 }

 for(i=0; i < n_withouts; i++)
 {
  allocation_size += strlen(withouts[i]) + 20;
 }

 if((result = (char *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }
 result[0] = 0;

 /* render output */
 length = 0;
 for(i=0; i<n_macros; i++)
 {
  length += snprintf(result + length, allocation_size - length,
                     "#define %s\n", macros[i]);
 }

 for(i=0; i < n_withouts; i++)
 {
  if((without_macro = without_string_to_without_macro(ctx, withouts[i])) != NULL)
  {
   length += snprintf(result + length, allocation_size - length,
                      "#define WITHOUT_%s\n", without_macro);
   free(without_macro);
   without_macro = NULL;
  } else {
   fprintf(stderr, "BCA: without_string_to_without_macro(%s) failed\n", withouts[i]);
  }
 }

 /* cleanup */
 free_string_array(macros, n_macros);
 free_string_array(withouts, n_withouts);
 free_string_array(disables, n_disables);
 free_string_array(opt_int_deps, n_opt_int_deps);

 return result;
}

char *resolve_string_replace_key(struct bca_context *ctx,
                                 char *key)
{
 char *value, a[256], b[256], c[256];
 int mode = 0, n_dots = 0, dots[2], length, i, edition, x, component_i;
 struct component_details cd;
 struct project_details *pd = NULL;

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

 if(strncmp(key, "CHECK(", 6) == 0)
 {
  return check_function(ctx, key);
 }

 if(strncmp(key, "LKUPOR(", 7) == 0)
 {
  return lookupor_function(ctx, key);
 }

 if(strncmp(key, "FILE_TO_C_SOURCE(", 17) == 0)
 {
  return file_to_C_source_function(ctx, key);
 }

 if(strncmp(key, "BCA.BUILDDIR", 12) == 0)
 {
#ifdef HAVE_CWD
  return strdup(ctx->cwd);
#else
  fprintf(stderr, "BCA: fixme, I've been built without cwd() and need that for @BCA.BUILDDIR@\n");
  return NULL;
#endif
 }

 if(strncmp(key, "BCA.MACROS.CONFIG_H", 19) == 0)
 {
  return handle_bca_config_h_macros(ctx);
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
        edition = RENDER_OUTPUT_NAME;
        break;

   case 18:
        edition = RENDER_INSTALL_OUTPUT_NAME;
        break;

   case 20:
        edition = RENDER_BUILD_OUTPUT_NAME;
        break;

   case 24:
        edition = RENDER_EFFECTIVE_OUTPUT_NAME;
        break;
  }

  if(load_build_config(ctx, 0))
   return NULL;

  if(load_project_config(ctx, 0))
   return NULL;

  if((pd = resolve_project_details(ctx)) == NULL)
   return NULL;

  component_i = -1;
  x = 0;
  while(x < pd->n_components)
  {
   if(strcmp(a, pd->component_names[x]) == 0)
   {
    component_i = x;
    break;
   }
   x++;
  }

  if(component_i < 0)
  {
   fprintf(stderr, "BCA: can't find component named '%s'\n", a);
   return NULL;
  }

  memset(&cd, 0, sizeof(struct component_details));
  cd.host = ctx->principle;
  cd.component_type = pd->component_types[component_i];
  cd.component_name = pd->component_names[component_i];
  cd.component_output_name = pd->component_output_names[component_i];

  if(render_project_component_output_names(ctx, &cd, NULL, edition))
  {
   fprintf(stderr, "BCA: replace key note: render_project_component_output_names() yielded "
           "no result for project component \"%s\" on host \"%s\".\n", a, ctx->principle);
   return strdup("");
  }

  if(free_project_details(pd))
   return NULL;

  if(i < cd.n_rendered_names)
  {
   value = strdup(cd.rendered_names[i]);
   free_rendered_names(&cd);
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
     ctx->line_number++;

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
             "BCA: string_replace(): could not resolve key \"%s\": %s, line %d\n",
             key, current_file_name(ctx), ctx->line_number);
     return 1;
    }

    fprintf(stdout, "%s", value);
    free(value);
   }
  }
 }

 return 0;
}

char *current_file_name(struct bca_context *ctx)
{
 if(ctx->n_input_files == 0)
  return "stdin";

 return ctx->input_files[ctx->input_file_index];
}
