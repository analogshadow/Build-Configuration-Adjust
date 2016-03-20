/* Begin config_files.c -------------------------------------- (sfd organizer) */

/* GPLv3

    Build Configuration Adjust, is a source configuration and Makefile
    generation tool.
    Copyright © 2012,2013,2014,2015,2016 C. Thomas Stover.
    All rights reserved. See
    https://github.com/analogshadow/Build-Configuration-Adjust for more
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

#include <glob.h>

int list_component_internal_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_component_internal_dependencies(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No internal dependencies found for component \"%s\".\n",
          cd->component_name);

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

int list_component_opt_internal_dependencies(struct bca_context *ctx,
                                             struct component_details *cd,
                                             char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_component_opt_external_dependencies(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "OPT_INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No optional internal dependencies found for component \"%s\".\n",
          cd->component_name);

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
  fprintf(stderr, "BCA: list_component_external_dependencies(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No external dependencies found for component \"%s\".\n",
          cd->component_name);

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
  fprintf(stderr, "BCA: list_component_opt_external_dependencies(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "OPT_EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No optional external dependencies found for component \"%s\".\n",
          cd->component_name);

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

int check_duplicate_output_names(struct bca_context *ctx, struct project_details *pd)
{
 char **output_names = NULL;
 int n_output_names = 0, x;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: check_duplicate_output_names()\n");

 for(x=0; x<pd->n_components; x++)
 {
  if(add_to_string_array(&output_names, n_output_names,
                         pd->component_output_names[x], -1, 1))
  {
   fprintf(stderr,
           "BCA: The component output name \"%s\" is used more than once.\n",
           pd->component_output_names[x]);

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

 char *component_keys[22] = { "PROJECT_NAME", "NAME", "MAJOR", "MINOR",
                              "AUTHOR", "EMAIL", "URL",
                              "FILES", "INPUT", "DRIVER", "INCLUDE_DIRS", "FILE_DEPENDS",
                              "INT_DEPENDS", "OPT_INT_DEPENDS", "EXT_DEPENDS", "OPT_EXT_DEPENDS",
                              "LIB_HEADERS", "DESCRIPTION", "PACKAGE_NAME",
                              "DISABLES", "WITHOUTS" };

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
  while(i<21)
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
   pd->project_components
   pd->project_ouput_names
   pd->project_component_types
   pd->n_components
*/
int list_project_components(struct bca_context *ctx,
                            struct project_details *pd)
{
 char **list = NULL, *name, *source, **source_files;
 int n_elements = 0, x, offset, n_source_files;
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

 pd->n_components = 0;
 pd->component_names = NULL;

 for(x=0; x<n_elements; x++)
 {
  if(strcmp(list[x], "ALL") == 0)
  {
   fprintf(stderr, "BCA: \"ALL\" should not be used as component name in the project configuration\n");
   return 1;
  }

  if(strcmp(list[x], "NONE") == 0)
   continue;

  if(add_to_string_array(&(pd->component_names),
                         pd->n_components,
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
   return 1;
  }

  if(add_to_string_array(&(pd->component_output_names),
                         pd->n_components, name, -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }

  free(name);
  pd->n_components++;
 }

 free_string_array(list, n_elements);
 return 0;
}

int list_build_hosts(struct bca_context *ctx,
                     struct build_details *bd)
{
 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: list_build_hosts()\n");

 if(list_unique_principles(ctx, NULL,
                           ctx->build_configuration_contents,
                           ctx->build_configuration_length,
                           &(bd->hosts), &(bd->n_hosts)))
 {
  fprintf(stderr, "BCA: list_build_hosts(): list_unique_principles() failed\n");
  return 1;
 }

 return 0;
}

int list_unique_opt_int_depends(struct bca_context *ctx,
                                char ***list_ptr,
                                int *n_elements_ptr,
                                int factor_disables)
{
 int offset, n_compound_values, i, j, n_opt_int_depends, code, yes;
 char principle[256], qualifier[256], key[256], *value,
      **compound_values, **opt_int_depends;

 n_opt_int_depends = 0;
 opt_int_depends = NULL;
 offset = -1;
 while(iterate_key_primitives(ctx,
                              ctx->project_configuration_contents,
                              ctx->project_configuration_length,
                              &offset,
                              NULL, NULL, "OPT_INT_DEPENDS",
                              principle, qualifier, key, NULL))
 {
  yes = 1;

  if(factor_disables)
  {
   j = 0;
   while(j<ctx->n_disables)
   {
    if(strcmp(qualifier, ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }
  }

  if(yes == 0)
   continue;

  if((value = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         principle, qualifier, key)) == NULL)
  {
   return 1;
  }

  if(split_strings(ctx, value, -1, &n_compound_values, &compound_values))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return 1;
  }

  for(i=0; i<n_compound_values; i++)
  {
   yes = 1;

   if(factor_disables)
   {
    j = 0;
    while(j<ctx->n_disables)
    {
     if(strcmp(compound_values[i], ctx->disabled_components[j]) == 0)
     {
      yes = 0;
      break;
     }
     j++;
    }
   }

   if(yes)
   {
    if((code = add_to_string_array(&opt_int_depends, n_opt_int_depends,
                                   compound_values[i], -1, 1)) == -1)
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(code == 0)
     n_opt_int_depends++;
   }

   free_string_array(compound_values, n_compound_values);
  }

  free(value);
 }

 *list_ptr = opt_int_depends;
 *n_elements_ptr = n_opt_int_depends;
 return 0;
}

int list_unique_opt_ext_depends(struct bca_context *ctx,
                                char ***list_ptr,
                                int *n_elements_ptr,
                                int factor_disables)
{
 int offset, n_compound_values, i, j, n_opt_ext_depends, code, yes;
 char principle[256], qualifier[256], key[256], *value,
      **compound_values, **opt_ext_depends;

 n_opt_ext_depends = 0;
 opt_ext_depends = NULL;
 offset = -1;
 while(iterate_key_primitives(ctx,
                              ctx->project_configuration_contents,
                              ctx->project_configuration_length,
                              &offset,
                              NULL, NULL, "OPT_EXT_DEPENDS",
                              principle, qualifier, key, NULL))
 {
  yes = 1;

  if(factor_disables)
  {
   j = 0;
   while(j<ctx->n_disables)
   {
    if(strcmp(qualifier, ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }
  }

  if(yes == 0)
   continue;

  if((value = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         principle, qualifier, key)) == NULL)
  {
   return 1;
  }

  if(split_strings(ctx, value, -1, &n_compound_values, &compound_values))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return 1;
  }

  for(i=0; i<n_compound_values; i++)
  {
   if((code = add_to_string_array(&opt_ext_depends, n_opt_ext_depends,
                                  compound_values[i], -1, 1)) == -1)
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   if(code == 0)
   {
    n_opt_ext_depends++;
   }

  }

  free_string_array(compound_values, n_compound_values);
  free(value);
 }

 *list_ptr = opt_ext_depends;
 *n_elements_ptr = n_opt_ext_depends;
 return 0;
}

int list_unique_ext_depends(struct bca_context *ctx,
                            char ***list_ptr,
                            int *n_elements_ptr,
                            int factor_disables)
{
 int offset, n_compound_values, i, j, n_ext_depends, code, yes;
 char principle[256], qualifier[256], key[256], *value,
      **compound_values, **ext_depends;

 n_ext_depends = 0;
 ext_depends = NULL;
 offset = -1;
 while(iterate_key_primitives(ctx,
                              ctx->project_configuration_contents,
                              ctx->project_configuration_length,
                              &offset,
                              NULL, NULL, "EXT_DEPENDS",
                              principle, qualifier, key, NULL))
 {
  yes = 1;

  if(factor_disables)
  {
   j = 0;
   while(j<ctx->n_disables)
   {
    if(strcmp(qualifier, ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }
  }

  if(yes == 0)
   continue;

  if((value = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         principle, qualifier, key)) == NULL)
  {
   return 1;
  }

  if(split_strings(ctx, value, -1, &n_compound_values, &compound_values))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return 1;
  }

  for(i=0; i<n_compound_values; i++)
  {
   if((code = add_to_string_array(&ext_depends, n_ext_depends,
                                  compound_values[i], -1, 1)) == -1)
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   if(code == 0)
   {
    n_ext_depends++;
   }

  }

  free_string_array(compound_values, n_compound_values);
  free(value);
 }

 *list_ptr = ext_depends;
 *n_elements_ptr = n_ext_depends;
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
resolve_host_configuration(struct bca_context *ctx, char *host, char *component)
{
 int allocation_size, i;
 struct host_configuration *tc;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: resolve_host_configuration(%s)\n", host);
  fflush(stderr);
 }

 allocation_size = sizeof(struct host_configuration);
 if((tc = (struct host_configuration *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }
 memset(tc, 0, allocation_size);

 char **host_resolve_vars[30] =
 {
  &(tc->build_prefix),
  &(tc->cc),
  &(tc->cxx),
  &(tc->cc_output_flag),
  &(tc->cc_compile_bin_obj_flag),
  &(tc->cc_compile_shared_library_obj_flag),
  &(tc->cc_include_dir_flag),
  &(tc->cc_define_macro_flag),
  &(tc->cflags),
  &(tc->ccflags),
  &(tc->cxxflags),
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

 char *host_resolve_keys[30] =
 {
  "BUILD_PREFIX",
  "CC",
  "CXX",
  "CC_SPECIFY_OUTPUT_FLAG",
  "CC_COMPILE_BIN_OBJ_FLAG",
  "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG",
  "CC_INCLUDE_DIR_FLAG",
  "CC_DEFINE_MACRO_FLAG",
  "CFLAGS",
  "CCFLAGS",
  "CXXFLAGS",
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

 for(i=0; i<29; i++)
 {
  *(host_resolve_vars[i]) = resolve_build_host_variable(ctx, host, component,
                                                        host_resolve_keys[i]);
 }

 if(ctx->verbose > 2)
 {
  for(i=0; i<26; i++)
  {
   printf("BCA: %s.%s.%s resolves to %s\n",
          host, component,  host_resolve_keys[i], *(host_resolve_vars[i]));
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

  if(tc->cxx != NULL)
   free(tc->cxx);

  if(tc->cflags != NULL)
   free(tc->cflags);

  if(tc->ccflags != NULL)
   free(tc->ccflags);

  if(tc->cxxflags != NULL)
   free(tc->cxxflags);

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
                     cd->host, cd->component_name, "DEPENDS");

 if(value == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: No DEPENDS key found for component \"%s\" "
           "on host \"%s\", implying dependencies.\n",
            cd->component_name, cd->host);

  cd->dependencies = NULL;
  cd->n_dependencies = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, &(cd->n_dependencies), &(cd->dependencies)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 if(ctx->verbose > 1)
 {
  printf("BCA: Found the following dependencies for component \"%s\" on host \"%s\" (%d): ",
         cd->component_name, cd->host, cd->n_dependencies);

  for(i=0; i < cd->n_dependencies; i++)
  {
   printf("%s ", cd->dependencies[i]);
  }
  printf("\n");
 }

 free(value);
 return 0;
}

int resolve_component_extra_file_dependencies(struct bca_context *ctx,
                                              struct component_details *cd)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_extra_file_dependencies(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "FILE_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No optional file level dependencies found for component \"%s\".\n",
          cd->component_name);

  cd->extra_file_deps = NULL;
  cd->n_extra_file_deps = 0;
 } else {

  if(split_strings(ctx, value, -1,
                   &(cd->n_extra_file_deps),
                   &(cd->extra_file_deps) ))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return 1;
  }
 }

 free(value);

 if(strcmp(cd->component_type, "SHAREDLIBRARY") != 0)
  return 0;

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "LIB_HEADERS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No Library headers found for library component \"%s\".\n",
          cd->component_name);

  cd->lib_headers = NULL;
  cd->n_lib_headers = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1,
                  &(cd->n_lib_headers),
                  &(cd->lib_headers) ))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int resolve_component_include_directories(struct bca_context *ctx,
                                          struct component_details *cd)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_include_directories(%s)\n",
          cd->component_name);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "INCLUDE_DIRS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No optional file level dependencies found for component \"%s\".\n",
          cd->component_name);

  cd->n_include_dirs = 0;
  cd->include_dirs = NULL;
  return 0;
 }

 if(split_strings(ctx, value, -1,
                  &(cd->n_include_dirs),
                  &(cd->include_dirs) ))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 free(value);
 return 0;
}

int resolve_component_source_files(struct bca_context *ctx,
                                   struct component_details *cd)
{
 char *base_file_name, *extension;
 int z, allocation_size;
 glob_t glob_data;
 int n_entries = 0;
 char **list = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_source_files(%s)\n",
          cd->component_name);

 if(lookup_value_as_list(ctx, OPERATE_PROJECT_CONFIGURATION,
                         cd->component_type,
                         cd->component_name,
                         "FILES",
                         &(list), &(n_entries)))
  return 1;

 /* attempt globbing */
 memset(&glob_data, 0, sizeof(glob_t));
 for(z=0; z < n_entries; z++)
 {
  if(glob(list[z], GLOB_APPEND, NULL, &glob_data))
  {
   fprintf(stderr, "BCA: glob(%s) failed\n", list[z]);
   return 1;
  }
 }
 for(z=0; z < glob_data.gl_pathc; z++)
 {
  if(add_to_string_array(&(cd->source_file_names),
                         cd->n_source_files,
                         glob_data.gl_pathv[z], -1, 0))
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }
  cd->n_source_files++;
 }
 globfree(&glob_data);
 free_string_array(list, n_entries);

 if(ctx->verbose > 1)
 {
  printf("BCA: Found the following source files for component \"%s\" (%d): ",
         cd->component_name, cd->n_source_files);

  for(z=0; z < cd->n_source_files; z++)
  {
   printf("%s ", cd->source_file_names[z]);
  }
  printf("\n");
 }

 allocation_size = cd->n_source_files * sizeof(char *);
 if((cd->source_file_base_names = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 if((cd->source_file_extensions = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 for(z=0; z < cd->n_source_files; z++)
 {
  if(path_extract(cd->source_file_names[z], &base_file_name, &extension))
  {
   return 1;
  }
  cd->source_file_base_names[z] = base_file_name;
  cd->source_file_extensions[z] = extension;
 }

 return 0;
}

int lookup_component_inputs(struct bca_context *ctx,
                            char *component_type, char *component_name,
                            char ***components_ptr,  char ***output_names_ptr,
                            int *n_inputs_ptr)
{
 char **components, **output_names, *value;
 int n_inputs, i;

 components = NULL;
 output_names = NULL;
 n_inputs = 0;

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        component_type, component_name, "INPUT")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_inputs, &components))
  {
   fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
   return 1;
  }

  free(value);

  for(i=0; i<n_inputs; i++)
  {
   value = lookup_key(ctx,
                      ctx->project_configuration_contents,
                      ctx->project_configuration_length,
                      NULL, components[i], "NAME");

   if(value == NULL)
   {
    fprintf(stderr,
            "BCA: component \"%s\" lists \"%s\" as an INPUT, yet project has no "
            "record \"*.%s.NAME = *\".\n",
            component_name, components[i], components[i]);
    return 1;
   }

   if(add_to_string_array(&output_names, i, value, -1, 0) != 0)
    return 1;

   free(value);
  }

 }

 if(components_ptr != NULL)
  *components_ptr = components;
 else
  free_string_array(components, n_inputs);

 if(output_names_ptr != NULL)
  *output_names_ptr = output_names;
 else
  free_string_array(output_names, n_inputs);

 if(n_inputs_ptr != NULL)
  *n_inputs_ptr = n_inputs;

 return 0;
}

int resolve_component_input_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         struct project_details *pd)
{
 char *value, **list = NULL;
 int z, i, n_elements = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_input_dependencies()\n");

 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->component_type,
                        cd->component_name,
                        "INPUT")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: Could not find %s.%s.INPUT\n",
          cd->component_type, cd->component_name);

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

  if(strcmp(cd->component_name, list[z]) == 0)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project "
           "component %s apears to list itself as an INPUT element\n",
           cd->component_name);

   free_string_array(list, n_elements);
   return 1;
  }

  i = 0;
  while(i< pd->n_components)
  {
   if(strcmp(list[z], pd->component_names[i]) == 0)
    break;
   i++;
  }

  if(i  == pd->n_components)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project component %s list INPUT element"
           " %s that is not itself a project component. Perhapes the element belongs in "
           "a FILES record?\n",
           cd->component_name, list[z]);
   free_string_array(list, n_elements);
   return 1;
  }

  if(add_to_string_array(&(cd->inputs), cd->n_inputs, pd->component_names[i], -1, 1))
  {
   fprintf(stderr, "BCA: resolve_component_input_dependencies(): add_to_string_array()\n");
   return 1;
  }
  cd->n_inputs++;
 }

 free_string_array(list, n_elements);

 return 0;
}

int resolve_component_version(struct bca_context *ctx,
                              struct component_details *cd)

{
 int generated = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_version()\n");

 if((cd->major = lookup_key(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            cd->component_type, cd->component_name,
                            "MAJOR")) == NULL)
 {
  if((cd->major = lookup_key(ctx,
                             ctx->project_configuration_contents,
                             ctx->project_configuration_length,
                             "NONE", "NONE", "MAJOR")) == NULL)
  {
   cd->major = strdup("0");
   generated = 1;
  }
 }

 if((cd->minor = lookup_key(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            cd->component_type, cd->component_name,
                            "MINOR")) == NULL)
 {
  if((cd->minor = lookup_key(ctx,
                             ctx->project_configuration_contents,
                             ctx->project_configuration_length,
                             "NONE", "NONE", "MINOR")) == NULL)
  {
   cd->minor = strdup("0");
   generated = 1;
  }
 }

 if(generated)
 {
  if(ctx->verbose)
     printf("BCA: Component \"%s\" version string artificialy set to %s.%s\n",
            cd->component_name, cd->major, cd->minor);
 } else {
  if(ctx->verbose > 1)
     printf("BCA: Component \"%s\" version string set to %s.%s\n",
            cd->component_name, cd->major, cd->minor);
 }

 return 0;
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
 {
  fprintf(stderr, "BCA: Project name not set!\n");
  return 1;
 }

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

int list_of_component_internal_dependencies(struct bca_context *ctx, char *component,
                                            char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_of_component_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        NULL, component, "INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No optional internal dependencies found for component \"%s\".\n",
          component);

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

int list_of_component_opt_internal_dependencies(struct bca_context *ctx, char *component,
                                                char ***list, int *n_elements,
                                                int factor_disables)
{
 int i, j, n_deps, yes, code;
 char *value = NULL, **deps;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_of_component_opt_internal_dependencies(%s)\n", component);

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        NULL, component, "OPT_INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No internal dependencies found for component \"%s\".\n",
          component);

  *list = NULL;
  *n_elements = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, n_elements, list))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }


 if(factor_disables)
 {
  deps = NULL;
  n_deps = 0;

  for(i=0; i<*n_elements; i++)
  {
   yes = 1;
   j=0;
   while(j<ctx->n_disables)
   {
    if(strcmp((*list)[i], ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }

   if(yes)
   {
    if((code = add_to_string_array(&deps, n_deps, (*list)[i], -1, 1)) == -1)
     return 1;

    if(code == 0)
     n_deps++;
   }
  }

  free_string_array(*list, *n_elements);
  *list = deps;
  *n_elements = n_deps;
 }

 free(value);
 return 0;
}

int list_of_component_external_dependencies(struct bca_context *ctx, char *component,
                                            char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_of_component_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        NULL, component, "EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No external dependencies found for component \"%s\".\n",
          component);

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

int list_of_component_opt_external_dependencies(struct bca_context *ctx, char *component,
                                                char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_of_component_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        NULL, component, "OPT_EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose > 1)
   printf("BCA: No optional external dependencies found for component \"%s\".\n",
          component);

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

int list_of_project_components(struct bca_context *ctx,
                               char ***component_names_ptr,
                               char ***component_types_ptr,
                               int *n_components_ptr,
                               int factor_disables)
{
 char **component_names, **component_types;
 int n_components, end, code, yes, j;
 char o_principle[256], o_qualifier[256], o_key[256];

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: list_of_project_components()\n");

 component_names = NULL;
 component_types = NULL;
 n_components = 0;

 end = -1;
 while((code = iterate_key_primitives(ctx, ctx->project_configuration_contents,
                                      ctx->project_configuration_length, &end,
                                      NULL, NULL, "NAME",
                                      o_principle, o_qualifier, o_key, NULL)) == 1)
 {
  yes = 1;
  if(factor_disables)
  {
   j = 0;
   while(j < ctx->n_disables)
   {
    if(strcmp(o_qualifier, ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }
  }

  if(yes)
  {
   if((code = add_to_string_array(&component_names, n_components,
                                  o_qualifier, -1, 1)) == -1)
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   if(code == 0)
   {

    if(add_to_string_array(&component_types, n_components,
                           o_principle, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }
    n_components++;
   }
  }
 }

 if(code == -1)
  return 1;

 if(component_names_ptr != NULL)
  *component_names_ptr = component_names;
 else
  free_string_array(component_names, n_components);

 if(component_types_ptr != NULL)
  *component_types_ptr = component_types;
 else
  free_string_array(component_types, n_components);

 if(n_components_ptr != NULL)
  *n_components_ptr = n_components;

 return 0;
}

int component_details_resolve_all(struct bca_context *ctx,
                                  struct component_details *cd,
                                  struct project_details *pd)
{
 if(resolve_component_source_files(ctx, cd))
  return 1;

 if(resolve_component_dependencies(ctx, cd))
  return 1;

 if(resolve_component_input_dependencies(ctx, cd, pd))
  return 1;

 if(resolve_component_extra_file_dependencies(ctx, cd))
  return 1;

 if(resolve_component_include_directories(ctx, cd))
  return 1;

 if(resolve_component_version(ctx, cd))
  return 1;

 return 0;
}

int component_details_cleanup(struct component_details *cd)
{

 if(cd->n_inputs != 0)
 {
  free_string_array(cd->inputs, cd->n_inputs);
  cd->inputs = NULL;
  cd->n_inputs = 0;
 }

 if(cd->n_source_files != 0)
 {
  free_string_array(cd->source_file_names, cd->n_source_files);
  free_string_array(cd->source_file_base_names, cd->n_source_files);
  free_string_array(cd->source_file_extensions, cd->n_source_files);
  cd->source_file_names = NULL;
  cd->source_file_base_names = NULL;
  cd->source_file_extensions = NULL;
  cd->n_source_files = 0;
 }

 if(cd->n_extra_file_deps != 0)
 {
  free_string_array(cd->extra_file_deps, cd->n_extra_file_deps);
  cd->extra_file_deps = NULL;
  cd->n_extra_file_deps = 0;
 }

 if(cd->n_include_dirs != 0)
 {
  free_string_array(cd->include_dirs, cd->n_include_dirs);
  cd->include_dirs = NULL;
  cd->n_include_dirs = 0;
 }

 if(cd->major != NULL)
 {
  free(cd->major);
  cd->major = NULL;
 }

 if(cd->minor != NULL)
 {
  free(cd->minor);
  cd->minor = NULL;
 }

 return 0;
}

#ifndef IN_SINGLE_FILE_DISTRIBUTION
int config_file_to_loco_listing(struct bca_context *ctx,
                                char *in_file_name,
                                char *out_file_name)
{
 char principle[256], component[256], key[256];
 char *contents, *value;
 int contents_length, offset = -1, equals_pos, contains_spaces, i,
     value_length, first;
 FILE *output;

 if(ctx->verbose > 1)
   fprintf(stderr, "BCA: config_file_to_loco_listing()\n");

 if((output = fopen(out_file_name, "w")) == NULL)
 {
  fprintf(stderr, "BCA: fopen(%s, w) failed, %s\n",
          out_file_name, strerror(errno));
  return 1;
 }

 if((contents = read_file(in_file_name, &contents_length, 0)) == NULL)
 {
  return 1;
 }

 fprintf(output, "listing:%s;\n", in_file_name);
 fprintf(output, " attribute:listingtype,sourcecode;\n");
 fprintf(output, " attribute:linenumbers,yes;\n");
 first = 1;

 while(iterate_key_primitives(ctx, contents, contents_length, &offset,
                              NULL, NULL, NULL,
                              principle, component, key, &equals_pos))
 {
  if((value = lookup_key(ctx, contents, contents_length,
                         principle, component, key)) == NULL)
  {
   fprintf(stderr, "BCA: problem in file %s with %s.%s.%s\n",
           in_file_name, principle, component, key);
   fclose(output);
   free(contents);
   return 1;
  }

  value_length = 0;
  i = 0;
  contains_spaces = 0;
  while(value[i] != 0)
  {
   if(value[i] == ' ')
    contains_spaces = 1;

   i++;
   value_length = i;
  }

  if(first)
  {
   fprintf(output, " object:text,");
   first = 0;
  } else {
   fprintf(output, " object:linebreak,");
  }

  fprintf(output, "\"%s\";\n", principle);
  fprintf(output, "  attribute:syntaxhighlight,bca_principle;\n");

  fprintf(output, " object:text,\".\";\n");
  fprintf(output, "  attribute:syntaxhighlight,bca_delimiter;\n");

  fprintf(output, " object:text,\"%s\";\n", component);
  fprintf(output, "  attribute:syntaxhighlight,bca_component;\n");

  fprintf(output, " object:text,\".\";\n");
  fprintf(output, "  attribute:syntaxhighlight,bca_delimiter;\n");

  fprintf(output, " object:text,\"%s\";\n", key);
  fprintf(output, "  attribute:syntaxhighlight,bca_key;\n");

  fprintf(output, " object:text,\" = \";\n");
  fprintf(output, "  attribute:syntaxhighlight,bca_delimiter;\n");

  if(contains_spaces)
  {
   fprintf(output, " object:text,\"\\\"\";\n");
   fprintf(output, "  attribute:syntaxhighlight,bca_delimiter;\n");
  }

  fprintf(output, " object:text,\"");
  for(i=0; i<value_length; i++)
  {
   if(value[i] == '\"')
   {
    fprintf(output, "\"\";\n");
    fprintf(output, "  attribute:syntaxhighlight,bca_value;\n");
    fprintf(output, " object:text,\"\\\"\";\n");
    fprintf(output, "  attribute:syntaxhighlight,bca_escape;\n");
    fprintf(output, " object:text,\"");
   } else {
    fprintf(output, "%c", value[i]);
   }
  }
  fprintf(output, "\";\n");
  fprintf(output, "  attribute:syntaxhighlight,bca_value;\n");

  if(contains_spaces)
  {
   fprintf(output, " object:text,\"\\\"\";\n");
   fprintf(output, "  attribute:syntaxhighlight,bca_delimiter;\n");
  }

  free(value);
 }

 fclose(output);
 free(contents);
 return 0;
}
#endif
/* End config_files.c ---------------------------------------- (sfd organizer) */
