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

struct project_details *
resolve_project_details(struct bca_context *ctx)
{
 struct project_details *pd;
 char **list = NULL, *name, *source, **source_files, *major, *minor;
 int n_elements = 0, x, offset, n_source_files, allocation_size;
 char principle[256];

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: resolve_project_details()\n");

 allocation_size = sizeof(struct project_details);
 if((pd = (struct project_details *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed, %s\n",
          allocation_size, strerror(errno));
  return NULL;
 }
 memset(pd, 0, allocation_size);

 if((pd->project_name =
     lookup_key(ctx, ctx->project_configuration_contents, ctx->project_configuration_length,
                "NONE", "NONE", "PROJECT_NAME")) == NULL)
 {
  fprintf(stderr, "BCA: Project name not set!\n");
  free_project_details(pd);
  return NULL;
 }

 if(list_unique_qualifiers(ctx,
                           ctx->project_configuration_contents,
                           ctx->project_configuration_length,
                           &list, &n_elements))
 {
  fprintf(stderr, "BCA: list_unique_principles() failed.\n");
  free_project_details(pd);
  return NULL;
 }

 pd->n_components = 0;
 pd->component_names = NULL;

 for(x=0; x<n_elements; x++)
 {
  if(strcmp(list[x], "ALL") == 0)
  {
   fprintf(stderr, "BCA: \"ALL\" should not be used as component name in the project configuration\n");
   free_project_details(pd);
   return NULL;
  }

  if(strcmp(list[x], "NONE") == 0)
   continue;

  if(add_to_string_array(&(pd->component_names),
                         pd->n_components,
                         list[x], -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   free_project_details(pd);
   return NULL;
  }

  offset = -1;
  if(iterate_key_primitives(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            &offset, NULL, list[x], NULL,
                            principle, NULL, NULL, NULL) == 0)
  {
   fprintf(stderr, "BCA: iterate_key_primitives(*, %s, *) failed \n", list[x]);
   free_project_details(pd);
   return NULL;
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
    free_project_details(pd);
    return NULL;
   }

   if(split_strings(ctx, source, -1, &n_source_files, &source_files))
   {
    fprintf(stderr, "BCA: split_strings() failed on '%s'\n", source);
    free_project_details(pd);
    return NULL;
   }
   if(free_string_array(source_files, n_source_files))
   {
    free_project_details(pd);
    return NULL;
   }

   if(n_source_files > 1)
   {
    fprintf(stderr,
            "BCA: BEAM component %s should only have one input file, not \"%s\"\n",
            list[x], source);
    free_project_details(pd);
    return NULL;
   }
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

  if(add_to_string_array(&(pd->component_types),
                         pd->n_components, principle, -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   free_project_details(pd);
   return NULL;
  }

  if(add_to_string_array(&(pd->component_output_names),
                         pd->n_components, name, -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   free_project_details(pd);
   return NULL;
  }

  free(name);
  pd->n_components++;
 }

 if(free_string_array(list, n_elements))
 {
  free_project_details(pd);
  return NULL;
 }

 if((pd->version_string = lookup_key(ctx,
                                     ctx->build_configuration_contents,
                                     ctx->build_configuration_length,
                                     "ALL", "ALL", "VERSION")) == NULL)
 {
  if((major = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         "NONE", "NONE", "MAJOR")) == NULL)
  {
   major = strdup("0");
  }

  if((minor = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         "NONE", "NONE", "MINOR")) == NULL)
  {
   minor = strdup("0");
  }

  allocation_size = strlen(major) + strlen(minor) + 2;
  if((pd->version_string = malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed, %s.\n",
           allocation_size, strerror(errno));
   free_project_details(pd);
   return NULL;
  }

  snprintf(pd->version_string, allocation_size, "%s.%s", major, minor);
  free(major);
  free(minor);
 }

 return pd;
}

int free_project_details(struct project_details *pd)
{
 if(pd != NULL)
 {
  if(pd->project_name != NULL)
   free(pd->project_name);

  if(pd->version_string != NULL)
   free(pd->version_string);

  if(pd->n_components > 0)
  {
   free_string_array(pd->component_names, pd->n_components);
   free_string_array(pd->component_types, pd->n_components);
   free_string_array(pd->component_output_names, pd->n_components);
  }

  free(pd);
 }

 return 0;
}
