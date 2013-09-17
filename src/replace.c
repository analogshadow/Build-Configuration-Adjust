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

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "prototypes.h"
#endif

char *resolve_string_replace_key(struct bca_context *ctx, char *key)
{
 char *value, a[256], b[256], c[256], **list = NULL, **withouts = NULL;
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
                      ctx->principle, a, "MACROS");

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
    if(ctx->verbose)
     printf("BCA: Could not find %s.%s.WITHOUTS\n", ctx->principle, a);
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

  length = 0;
  for(i=0; i<n_items; i++)
  {
   length += snprintf(value + length, allocation_size - length,
                      "#define %s\n", list[i]);
  }

  for(i=0; i < n_withouts; i++)
  {
   length += snprintf(value + length, allocation_size - length,
                      "#define WITHOUT_%s\n", withouts[i]);
  }
  length += snprintf(value + length, allocation_size - length, "\n");

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
   fprintf(stderr, "BCA: replace key error: index out of range in \"%s\".\n", key);
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
     fprintf(stderr, "BCA: string_replace(): could not resolve key \"%s\"\n", key);
     return 1;
    }

    fprintf(stdout, "%s", value);
    free(value);
   }

  }

 }

 return 0;
}
