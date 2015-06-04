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

#define MAKE_PASS_ALL                0
#define MAKE_PASS_HOST_TARGETS       1
#define MAKE_PASS_HELP               2
#define MAKE_PASS_CLEAN_RULES        3
#define MAKE_PASS_CLEAN_RULES_2      4
#define MAKE_PASS_INSTALL_RULES      5
#define MAKE_PASS_INSTALL_RULES_2    6
#define MAKE_PASS_INSTALL_RULES_3    7
#define MAKE_PASS_UNINSTALL_RULES    8
#define MAKE_PASS_UNINSTALL_RULES_2  9
#define MAKE_PASS_UNINSTALL_RULES_3  10
#define MAKE_PASS_TARBALL_RULES      11
#define MAKE_PASS_BUILD_RULES        12
#define N_MAKE_PASSES                13

int build_host_component_file_rule_cflags(struct bca_context *ctx, FILE *output,
                                          struct component_details *cd,
                                          struct host_configuration *tc);

int decide_cxx_runtime_requirement(struct bca_context *ctx,
                                   struct component_details *cd,
                                   struct project_details *pd,
                                   struct host_configuration *tc);

int generate_host_component_target_dependencies(struct bca_context *ctx,
                                                struct component_details *cd,
                                                struct project_details *pd,
                                                char *output_file_name,
                                                FILE *output);

char *component_swap_host(struct bca_context *ctx, char *component)
{
 int i;

 i = 0;
 while(i < ctx->n_swaps)
 {
  if(strcmp(ctx->swapped_components[i], component) == 0)
   return ctx->swapped_component_hosts[i];
  i++;
 }

 return NULL;
}

/*  each component might have multiple dependencies,
    each one of those might be a internal dep, and also
    be swapped component. If so, then for each of those
    the directory for that host needs to be added to the
    PKG_CONFIG_PATH *after* the build dir for the current
    host (otherwise conflicting .pc files may get out of
    order).
*/
int component_pkg_config_path(struct bca_context *ctx,
                              struct component_details *cd,
                              struct host_configuration *tc,
                              FILE *output)
{
 int y, n_paths = 0, list_length = 0;
 char **list = NULL, *swapped_host, *value;

 if(tc->pkg_config_path != NULL)
 {
  fprintf(output, "PKG_CONFIG_PATH=");
  fprintf(output, "%s", tc->pkg_config_path);
  n_paths++;
 }

 for(y=0; y < cd->n_dependencies; y++)
 {
  swapped_host = component_swap_host(ctx, cd->dependencies[y]);

  if(swapped_host != NULL)
  {
   if((value = lookup_key(ctx,
                          ctx->build_configuration_contents,
                          ctx->build_configuration_length,
                          swapped_host, "ALL", "BUILD_PREFIX")) == NULL)
   {
    fprintf(stderr,
            "BCA: can find the build prefix for component \"%s\" on host "
            "\"%s\" which is swapped from host \"%s\"\n",
            cd->dependencies[y], swapped_host, cd->host);
    return 1;
   }

   /* prevent duplicate dirs */
   if(add_to_string_array(&list, list_length,
                          value, -1, 1) == 0)
   {
    if(n_paths == 0)
     fprintf(output, "PKG_CONFIG_PATH=");

    if(n_paths > 0)
     fprintf(output, ":");

    fprintf(output, "%s", value);
    n_paths++;

    list_length++;
   }

   free(value);
  }
 }

 if(n_paths)
  fprintf(output, " ");

 free_string_array(list, list_length);

 return 0;
}

int generate_host_component_pkg_config_file(struct bca_context *ctx,
                                            struct component_details *cd,
                                            struct project_details *pd,
                                            struct host_configuration *tc,
                                            char *pkg_file_name,
                                            char **output_file_names,
                                            int n_output_file_names,
                                            FILE *output,
                                            int installed_version)
{
 int x, i, yes, need_cxx_runtime = 0;
 struct component_details cd_d;
 char *build_prefix, *package_name = NULL, *package_description = NULL, *link_name;

 if((need_cxx_runtime = decide_cxx_runtime_requirement(ctx, cd, pd, tc)) == -1)
 {
  fprintf(stderr, "BCA: decide_cxx_runtime_requirement() failed\n");
  return 1;
 }

/*
   Idea / question:
    reconcille package config's --variable and --define-variable with BCA variables
*/

 if((package_name = lookup_key(ctx,
                               ctx->project_configuration_contents,
                               ctx->project_configuration_length,
                               cd->component_type,
                               cd->component_name,
                               "PACKAGE_NAME")) == NULL)
 {
  package_name = cd->component_output_name;
 }

 package_description = lookup_key(ctx,
                                  ctx->project_configuration_contents,
                                  ctx->project_configuration_length,
                                  cd->component_type,
                                  cd->component_name,
                                  "DESCRIPTION");

 if(installed_version)
 {
  fprintf(output, "\techo \"prefix=%s\" > %s\n",
          tc->install_prefix, pkg_file_name);

 } else {
  build_prefix = tc->build_prefix;
  if(strncmp(build_prefix, "./", 2) == 0)
   build_prefix += 2;

  fprintf(output, "\techo \"prefix=%s/%s\" > %s\n",
#ifdef HAVE_CWD
          ctx->cwd,
#else
          "`pwd`",
#endif
          build_prefix, pkg_file_name);
 }

 fprintf(output, "\techo 'exec_prefix=$${prefix}' >> %s\n", pkg_file_name);

 if(installed_version)
 {
  /* need to make sure this matches the installation dir. or maybe this
     should just be the installation dir? */
  fprintf(output, "\techo 'libdir=$${prefix}/lib' >> %s\n", pkg_file_name);
 } else {
  fprintf(output, "\techo 'libdir=$${exec_prefix}' >> %s\n", pkg_file_name);
 }

 if(cd->n_include_dirs > 0)
 {
  if(strncmp(cd->include_dirs[0], "./", 2) == 0)
  {
   fprintf(output, "\techo \"includedir=%s/%s\" >> %s\n",
#ifdef HAVE_CWD
           ctx->cwd,
#else
           "`pwd`",
#endif
           cd->include_dirs[0] + 2, pkg_file_name);
  } else {
   fprintf(output, "\techo 'includedir=%s' >> %s\n", cd->include_dirs[0], pkg_file_name);
  }
 }
 fprintf(output, "\techo 'Name: %s' >> %s\n",
         package_name, pkg_file_name);

 if(package_description == NULL)
  fprintf(output, "\techo 'Description: (set me with SHAREDLIBRARY.%s.DESCRIPTION)' >> %s\n",
          cd->component_name, pkg_file_name);
 else
  fprintf(output, "\techo 'Description: %s' >> %s\n",
          package_description, pkg_file_name);

 fprintf(output, "\techo 'Version: %s.%s' >> %s\n",
         cd->major, cd->minor, pkg_file_name);

 fprintf(output, "\techo 'Requires: ");

 for(i=0; i < cd->n_dependencies; i++)
 {
  yes = 0;
  x = 0;
  while(x < pd->n_components)
  {
   if(strcmp(cd->dependencies[i], pd->component_names[x]) == 0)
   {
    cd_d.component_type = "SHAREDLIBRARY";
    cd_d.component_name = pd->component_names[x];
    resolve_component_version(ctx, &cd_d);

    fprintf(output, "%s-%s ", pd->component_output_names[x], cd_d.major);
    yes = 1;
    break;
   }
   x++;
  }

  if(yes == 0)
   fprintf(output, "%s ", cd->dependencies[i]);
 }

 fprintf(output, "' >> %s\n", pkg_file_name);

 if(installed_version)
 {
  if((link_name = lib_file_name_to_link_name(output_file_names[0])) == NULL)
  {
   fprintf(stderr, "BCA: lib_file_name_to_link_name(%s) failed\n", output_file_names[0]);
   return 1;
  }

  fprintf(output, "\techo 'Libs: -L$${libdir} -l%s", link_name);
  free(link_name);
 } else {
  fprintf(output, "\techo 'Libs: $${libdir}/%s", output_file_names[0]);
 }

 if(need_cxx_runtime)
 {
  fprintf(output, " -lstdc++"); //this needs to be in the build configuration (per host)
 }

 if(tc->ldflags != NULL)
  fprintf(output, " %s", tc->ldflags);

 fprintf(output, "' >> %s\n", pkg_file_name);

 fprintf(output, "\techo 'Cflags:");

 if(cd->n_include_dirs > 0)
  fprintf(output, " -I$${includedir} ");

 if(build_host_component_file_rule_cflags(ctx, output, cd, tc))
  return 1;

 fprintf(output, "' >> %s\n", pkg_file_name);

 /* add unversioned symlink */
 if(installed_version)
 {
  fprintf(output, "\tcd %s; ln -sf %s %s.pc\n",
          tc->install_pkg_config_dir, output_file_names[1],
          cd->component_output_name);

 } else {
  fprintf(output, "\tcd %s; ln -sf %s %s.pc\n",
          tc->build_prefix, output_file_names[1],
          cd->component_output_name);
 }

 if(package_description != NULL)
  free(package_description);

 if(package_name != NULL)
  if(package_name != cd->component_output_name)
   free(package_name);

 fprintf(output, "\n");
 return 0;
}


/* both source files that first go to object files before linking (ie .c),
   and sources that compile and link in one step (ie .cs) will us these
   "compile flags".
 */
int build_host_component_file_rule_cflags(struct bca_context *ctx, FILE *output,
                                          struct component_details *cd,
                                          struct host_configuration *tc)
{
 int i;
 char *string;

 if(tc->cflags != NULL)
 {
  fprintf(output, "%s ", tc->cflags);
 }

 if(is_project_using_config_h(ctx))
  return 0;

 for(i=0; i < cd->n_withouts; i++)
 {
  fprintf(output, "%sWITHOUT_", tc->cc_define_macro_flag);

  if((string = without_string_to_without_macro(ctx, cd->withouts[i])) == NULL)
  {
   fprintf(stderr, "without_string_to_without_macro() failed\n");
   return 1;
  }
  fprintf(output, "%s ", string);
  free(string);
 }

 return 0;
}

int object_from_c_file(struct bca_context *ctx,
                       struct component_details *cd,
                       struct project_details *pd,
                       struct host_configuration *tc,
                       char *source_file_base_name,
                       char *source_file_name,
                       char *output_file_name,
                       FILE *output)
{
 int x, y, yes;
 char temp[1024];
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

 snprintf(temp, 1024, "%s/obj/%s-%s%s",
          tc->build_prefix, cd->component_name,
          source_file_base_name, tc->obj_suffix);

 fprintf(output, "%s : %s", temp, source_file_name);

 if(count_host_component_target_dependencies(ctx, cd) > 0)
  fprintf(output, " $(%s-FILE_DEPENDENCIES)", output_file_name);

 fprintf(output, "\n");

 fprintf(output, "\t%s ", tc->cc);

 if(build_host_component_file_rule_cflags(ctx, output, cd, tc))
  return 1;

 /* handle dependencies (internal and extrnal) */
 if(cd->n_dependencies > 0)
 {
  fprintf(output, "`");

  if(component_pkg_config_path(ctx, cd, tc, output))
   return 1;

  if(tc->pkg_config_libdir != NULL)
   fprintf(output, "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

  fprintf(output, "%s --cflags ", tc->pkg_config);

  for(y=0; y < cd->n_dependencies; y++)
  {
   yes = 0;
   x = 0;
   while(x < pd->n_components)
   {
    if(strcmp(cd->dependencies[y], pd->component_names[x]) == 0)
    {
     cd_d.component_type = "SHAREDLIBRARY";
     cd_d.component_name = pd->component_output_names[x];

     resolve_component_version(ctx, &cd_d);

     fprintf(output, "%s-%s ", pd->component_output_names[x], cd_d.major);
     yes = 1;
     break;
    }
    x++;
   }

   if(yes == 0)
    fprintf(output, "%s ", cd->dependencies[y]);
  }

  fprintf(output, "` ");
 }

 for(y=0; y< cd->n_include_dirs; y++)
 {
  fprintf(output, "%s %s ", tc->cc_include_dir_flag, cd->include_dirs[y]);
 }

 if(strcmp(cd->component_type, "BINARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_bin_obj_flag);

 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_shared_library_obj_flag);

 if(tc->cxxflags != NULL)
  fprintf(output, "%s ", tc->ccflags);

 fprintf(output, "%s ", source_file_name);

 fprintf(output, "%s ", tc->cc_output_flag);

 fprintf(output, "%s\n\n", temp);

 return 0;
}

int object_from_cxx_file(struct bca_context *ctx,
                         struct component_details *cd,
                         struct project_details *pd,
                         struct host_configuration *tc,
                         char *source_file_base_name,
                         char *source_file_name,
                         char *output_file_name,
                         FILE *output)
{
 int x, y, yes;
 char temp[1024];
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

 snprintf(temp, 1024, "%s/obj/%s-%s%s",
          tc->build_prefix, cd->component_name,
          source_file_base_name, tc->obj_suffix);

 fprintf(output, "%s : %s", temp, source_file_name);

 if(count_host_component_target_dependencies(ctx, cd) > 0)
  fprintf(output, " $(%s-FILE_DEPENDENCIES)", output_file_name);

 fprintf(output, "\n");

 fprintf(output, "\t%s ", tc->cxx);

 if(build_host_component_file_rule_cflags(ctx, output, cd, tc))
  return 1;

 /* handle dependencies (internal and extrnal) */
 if(cd->n_dependencies > 0)
 {
  fprintf(output, "`");

  if(component_pkg_config_path(ctx, cd, tc, output))
   return 1;

  if(tc->pkg_config_libdir != NULL)
   fprintf(output, "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

  fprintf(output, "%s --cflags ", tc->pkg_config);

  for(y=0; y < cd->n_dependencies; y++)
  {
   yes = 0;
   x = 0;
   while(x < pd->n_components)
   {
    if(strcmp(cd->dependencies[y], pd->component_names[x]) == 0)
    {
     cd_d.component_type = "SHAREDLIBRARY";
     cd_d.component_name = pd->component_names[x];
     resolve_component_version(ctx, &cd_d);

     fprintf(output, "%s-%s ", pd->component_output_names[x], cd_d.major);
     yes = 1;
     break;
    }
    x++;
   }

   if(yes == 0)
    fprintf(output, "%s ", cd->dependencies[y]);
  }

  fprintf(output, "` ");
 }

 for(y=0; y< cd->n_include_dirs; y++)
 {
  fprintf(output, "%s %s ", tc->cc_include_dir_flag, cd->include_dirs[y]);
 }

 if(strcmp(cd->component_type, "BINARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_bin_obj_flag);

 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_shared_library_obj_flag);

 if(tc->cxxflags != NULL)
  fprintf(output, "%s ", tc->cxxflags);

 fprintf(output, "%s ", source_file_name);

 fprintf(output, "%s ", tc->cc_output_flag);

 fprintf(output, "%s\n\n", temp);

 return 0;
}

int decide_cxx_runtime_requirement(struct bca_context *ctx,
                                   struct component_details *cd,
                                   struct project_details *pd,
                                   struct host_configuration *tc)
{
 int i, x, handled;
 char *extension;

 /* This is hacky as "all get out". For now just see if component had any files
    that were C++ sources.
 */

 for(i=0; i < cd->n_source_files; i++)
 {
  if( (strcmp(cd->source_file_extensions[i], "cc") == 0) ||
      (strcmp(cd->source_file_extensions[i], "cxx") == 0) ||
      (strcmp(cd->source_file_extensions[i], "cpp") == 0) )
  {
   return 1;
  }
 }

 for(i=0; i < cd->n_inputs; i++)
 {
  x = 0;

  /* this test is done in multiple places when the opertunity comes up */
  while(x < pd->n_components)
  {
   if(strcmp(cd->inputs[i], pd->component_names[x]) == 0)
   {
    handled = 1;
    break;
   }
   x++;
  }

  if(handled == 0)
  {
   fprintf(stderr,
           "BCA: component %s on host %s has an unresolved .INPUT of %s.\n",
           cd->component_name, cd->host, cd->inputs[i]);
   return 1;
  }

  if(path_extract(pd->component_output_names[x], NULL, &extension))
  {
   return 1;
  }

  if( (strcmp(extension, "cc") == 0) ||
      (strcmp(extension, "cxx") == 0) ||
      (strcmp(extension, "cpp") == 0) )
  {
   free(extension);
   return 1;
  }

  free(extension);
 }

 return 0;
}

/* .INPUT is a list of other components from which to dynamically translate
   the output file name of into elements on the .FILES list.

   Note that there is no inheritance for things like: dependencies, .FILE_DEPS,
   and .INCLUDE_DIRS. Those must and can be added to the component(s)
   using other components as an input.

   The primary use case is for MACROEXPAND, CAT, and CUSTOM component types
   to create source files for other types. See the test examples.enerateddeps
   for an example.

   Chaining of multiple layers can also be done. ie a conCATenation of MACROEXPANDS.
   (that needs a test).

   The other allowed use is a CUSTOM component may have a BINARY compoent as
   an INPUT. The idea is the the .DRIVER script would make use of said BINARY.
 */
int derive_file_dependencies_from_inputs(struct bca_context *ctx,
                                         struct host_configuration *tc,
                                         struct component_details *cd,
                                         struct project_details *pd)
{
 int i, x, y, handled, valid_input, process_file_name, add_temp_to_files,
     original_n_files, swapped_i;
 char temp[1024], *base_file_name, *extension, *ptr;
 struct component_details output_names_cd;

 original_n_files = cd->n_source_files;

 for(i=0; i < cd->n_inputs; i++)
 {
  valid_input = 0;

  /* what kind of component is this input? */
  handled = 0;
  x = 0;
  while(x < pd->n_components)
  {
   if(strcmp(cd->inputs[i], pd->component_names[x]) == 0)
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
           cd->component_name, cd->host, cd->inputs[i]);
   return 1;
  }

  valid_input = 0;
  process_file_name = 0;

  if(strcmp(pd->component_types[x], "CUSTOM") == 0)
  {
   valid_input = 1;
   process_file_name = 1;
  }

  if(strcmp(pd->component_types[x], "CAT") == 0)
  {
   valid_input = 1;
   process_file_name = 1;
  }

  if(strcmp(pd->component_types[x], "MACROEXPAND") == 0)
  {
   valid_input = 1;
   process_file_name = 1;
  }

  if(strcmp(cd->component_type, "CUSTOM") == 0)
  {
   if(strcmp(pd->component_types[x], "BINARY") == 0)
   {
    valid_input = 1;

    swapped_i = -1;
    y = 0;
    while(y < ctx->n_swaps)
    {
     if(strcmp(pd->component_names[x], ctx->swapped_components[y]) == 0)
     {
      swapped_i = -1;
      break;
     }
     y++;
    }

    memset(&output_names_cd, 0, sizeof(struct component_details));

    if(swapped_i == -1)
     output_names_cd.host = cd->host;
    else
     output_names_cd.host = ctx->swapped_component_hosts[swapped_i];

    output_names_cd.component_type = pd->component_types[x];
    output_names_cd.component_name = pd->component_names[x];
    output_names_cd.component_output_name = pd->component_output_names[x];

    if(render_project_component_output_names(ctx,
                                             &output_names_cd,
                                             RENDER_BUILD_OUTPUT_NAME))
    {
     fprintf(stderr, "BCA: render_project_component_output_names() failed 1\n");
     return 1;
    }

    if(add_to_string_array(&(cd->source_file_names), cd->n_source_files,
                           output_names_cd.rendered_names[0], -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(path_extract(output_names_cd.rendered_names[0], &base_file_name, &extension))
    {
     fprintf(stderr, "BCA: path_extract(%s) failed\n", temp);
     return 1;
    }

    if(add_to_string_array(&(cd->source_file_base_names), cd->n_source_files,
                           base_file_name, 0, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(add_to_string_array(&(cd->source_file_extensions), cd->n_source_files,
                           extension, 0, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_source_files++;

    free_rendered_names(&output_names_cd);
    free(base_file_name);
    free(extension);
   }
  }

  if(valid_input == 0)
  {
   fprintf(stderr,
           "BCA: I don't know what to do with input component %s of type %s "
           "for component %s of type %s on host %s.\n",
           cd->inputs[i], pd->component_types[x],
           cd->component_name, cd->component_type, cd->host);
   return 1;
  }

  if(process_file_name)
  {
   handled = 0;
   add_temp_to_files = 0;

   snprintf(temp, 1024, "%s/%s",
            tc->build_prefix, pd->component_output_names[x]);

   if(path_extract(temp, &base_file_name, &extension))
   {
    fprintf(stderr, "BCA: path_extract(%s) failed\n", temp);
    return 1;
   }

   /* this needs to be updated everytime new file types are added, this really
      should be an array that is shared everywhere.
      We could skip this check since later when the component target rules get
      generated, a file type with an unknown corse of action will be reported,
      but we wont know that it was from an .INPUT and we wont get to deal with
      header files specially here.
   */
   if(strcmp(extension, ".c") == 0)
   {
    handled = 1;
    add_temp_to_files = 1;
   }

   if( (strcmp(extension, ".cpp") == 0) ||
       (strcmp(extension, ".cc") == 0) )
   {
    handled = 1;
    add_temp_to_files = 1;
   }

   /*  If the input is a header file, then add it to the .FILE_DEPS since
       it that there is some code path for creating the correct makefile dependency.
       We need to also add the directory to the list of include paths.
    */
   if(strcmp(extension, ".h") == 0)
   {
    if(add_to_string_array(&(cd->extra_file_deps), cd->n_extra_file_deps,
                           temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_extra_file_deps++;

    y = strlen(temp);
    while(y > 0)
    {
     if(temp[y] == '/')
      break;

     temp[y--] = 0;
    }

    if(add_to_string_array(&(cd->include_dirs), cd->n_include_dirs, temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_include_dirs++;
    handled = 1;
   }

   /* the text processing types can of course work with any file extension */
   if((handled == 0) &&
      (strcmp(cd->component_type, "CAT") == 0) )
   {
    handled = 1;
    add_temp_to_files = 1;
   }

   if((handled == 0) &&
      (strcmp(cd->component_type, "MACROEXPAND") == 0) )
   {
    handled = 1;
    add_temp_to_files = 1;
   }

   /* along the same lines CUSTOM types could be doing anything */
   if((handled == 0) &&
      (strcmp(cd->component_type, "CUSTOM") == 0) )
   {
    handled = 1;
    add_temp_to_files = 1;
   }

   if(add_temp_to_files)
   {
    /* Do the actual adding of the ouptut name to .FILES. At this point,
       base names and extensions have already been expanded for the other
       .FILES, so these need to be added here. */
    if(add_to_string_array(&(cd->source_file_names), cd->n_source_files,
                           temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(add_to_string_array(&(cd->source_file_base_names), cd->n_source_files,
                           base_file_name, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    if(add_to_string_array(&(cd->source_file_extensions), cd->n_source_files,
                           extension, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_source_files++;
   }

   if(handled == 0)
   {
    fprintf(stderr,
            "BCA: I don't know what to do with input component %s of type %s "
           "for component %s of type %s on host %s, that is yielding an "
           "output file with the extension '%s'.\n",
           cd->inputs[i], pd->component_types[x],
           cd->component_name, cd->component_type,
           cd->host, extension);
    return 1;
   }

   free(base_file_name);
   free(extension);
  }
 }

 /* hack to put .INPUTS before .FILES (matters for CAT)*/
 if(original_n_files == 0)
  return 0;

 for(y=0; y<cd->n_source_files - original_n_files; y++)
 {
  ptr = cd->source_file_names[original_n_files + y];
  for(x=original_n_files - 1; x>-1; x--)
  {
   cd->source_file_names[x + y + 1] = cd->source_file_names[x + y];
  }
  cd->source_file_names[y] = ptr;

  ptr = cd->source_file_base_names[original_n_files + y];
  for(x=original_n_files - 1; x>-1; x--)
  {
   cd->source_file_base_names[x + y + 1] = cd->source_file_base_names[x + y];
  }
  cd->source_file_base_names[y] = ptr;

  ptr = cd->source_file_extensions[original_n_files + y];
  for(x=original_n_files - 1; x>-1; x--)
  {
   cd->source_file_extensions[x + y + 1] = cd->source_file_extensions[x + y];
  }
  cd->source_file_extensions[y] = ptr;
 }

 return 0;
}

int build_rule_component_pythonmodule(struct bca_context *ctx,
                                      struct component_details *cd,
                                      struct host_configuration *tc,
                                      char *output_file_name,
                                      FILE *output)
{
 int i;

 for(i=0; i<cd->n_source_files; i++)
 {

  if(strcmp(cd->source_file_extensions[i], "c") == 0)
  {
   fprintf(output,
           "# finish me: should define howto build %s as part of python "
           "module %s for host %s\n",
           cd->source_file_names[i], cd->component_name, cd->host);
  } else {
   fprintf(stderr, "I don't know how to build a python module from file \"%s\".\n",
           cd->source_file_names[i]);
   return 1;
  }
 }

 return 0;
}

int build_rule_component_erlangbeam(struct bca_context *ctx,
                                    struct component_details *cd,
                                    struct host_configuration *tc,
                                    char *output_file_name,
                                    FILE *output)
{
 if(cd->n_source_files != 1)
 {
  fprintf(stderr,
          "BCA: I should have exactly one input file for BEAM component %s.\n",
          cd->component_name);
  return 1;
 }

 fprintf(output, "%s : ",  output_file_name);

 if(count_host_component_target_dependencies(ctx, cd) > 0)
 {
  fprintf(output, "$(%s-FILE_DEPENDENCIES) ", output_file_name);
 }

 fprintf(output, "%s\n", cd->source_file_names[0]);


 fprintf(output, "\t%s ", tc->erlc);

 if(tc->erlc_flags != NULL)
  fprintf(output, "%s ", tc->erlc_flags);

 fprintf(output, "%s %s %s\n\n",
         tc->erlc_output_dir_flag, tc->build_prefix, cd->source_file_names[0]);

 return 0;
}

int build_rule_component_custom(struct bca_context *ctx,
                                struct component_details *cd,
                                struct project_details *pd,
                                struct host_configuration *tc,
                                char *output_file_name,
                                FILE *output)
{
 char *value;
 int yes, i, j, driver_component;

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        "CUSTOM",
                        cd->component_name,
                        "DRIVER")) == NULL)
 {
  fprintf(stderr, "BCA: no project file record for CUSTOM.%s.DRIVER\n", cd->component_name);
  return 1;
 }

 yes = 0;
 i = 0;
 while(i < pd->n_components)
 {
  if(strcmp(pd->component_names[i], value) == 0)
  {
   if(strcmp(pd->component_types[i], "MACROEXPAND") == 0)
   {
    yes = 1;
   }
   break;
  }
  i++;
 }
 driver_component = i;
 if(yes == 0)
 {
  fprintf(stderr,
          "BCA: CUSTOM.%s.DRIVER = %s does not seem to be a MACROEXPAND project component\n",
          cd->component_name, value);
  return 1;
 }
 free(value);

 if( (cd->source_file_names == NULL) && (cd->n_inputs == 0) )
 {
  fprintf(stderr, "BCA: CUSTOM component \"%s\", does not seem to have .INPUT or .FILES\n",
          cd->component_name);
  return 1;
 }

 fprintf(output, "%s : ", output_file_name);
 if(count_host_component_target_dependencies(ctx, cd) > 0)
 {
  fprintf(output, "$(%s-FILE_DEPENDENCIES) ", output_file_name);
 }

 for(i=0; i<cd->n_source_files; i++)
 {
  fprintf(output, "%s ", cd->source_file_names[i]);
 }

 for(i=0; i<cd->n_inputs; i++)
 {
  j=0;
  while(j < pd->n_components)
  {
   if(strcmp(pd->component_names[j], cd->inputs[i]) == 0)
    break;
   j++;
  }

  fprintf(output, "%s/%s ", tc->build_prefix, pd->component_output_names[j]);
 }

 fprintf(output, "%s/%s\n", tc->build_prefix, pd->component_output_names[driver_component]);

 fprintf(output, "\tchmod +x %s/%s\n",
         tc->build_prefix, pd->component_output_names[driver_component]);

 fprintf(output, "\t%s/%s ",
         tc->build_prefix, pd->component_output_names[driver_component]);

 for(j=0; j<cd->n_source_files; j++)
 {
  fprintf(output, "%s ", cd->source_file_names[j]);
 }

 fprintf(output, "%s\n\n", output_file_name);
 return 0;
}

int build_rule_component_macroexpand(struct bca_context *ctx,
                                     struct component_details *cd,
                                     char *output_file_name,
                                     FILE *output)
{
 int i;

 fprintf(output, "%s : ", output_file_name);
 if(count_host_component_target_dependencies(ctx, cd) > 0)
  fprintf(output, "$(%s-FILE_DEPENDENCIES) ", output_file_name);

 for(i=0; i<cd->n_source_files; i++)
 {
  fprintf(output, "%s ", cd->source_file_names[i]);
 }

 if(strcmp(cd->component_name, "config_h") == 0)
 {
  fprintf(output, "./buildconfiguration/buildconfiguration");
 }

 fprintf(output, "\n\t./bca --host %s --replacestrings < ", cd->host);

 if(cd->n_source_files < 1)
 {
  fprintf(stderr,
          "BCA: MACROEXPAND component %s has no source files. Did you use .INPUT or .FILES?\n",
          cd->component_name);
  return 1;
 }

 for(i=0; i<cd->n_source_files; i++)
 {
  fprintf(output, "%s ", cd->source_file_names[i]);
 }

 fprintf(output, "> %s\n\n", output_file_name);

 return 0;
}

int build_rule_component_concatenate(struct bca_context *ctx,
                                     struct component_details *cd,
                                     char *output_file_name,
                                     FILE *output)
{
 int i;

 fprintf(output, "%s : ", output_file_name);
 if(count_host_component_target_dependencies(ctx, cd) > 0)
  fprintf(output, "$(%s-FILE_DEPENDENCIES) ", output_file_name);

 for(i=0; i<cd->n_source_files; i++)
 {
  fprintf(output, "%s ", cd->source_file_names[i]);
 }

 fprintf(output, "\n\t./bca --concatenate ");

 for(i=0; i<cd->n_source_files; i++)
 {
  fprintf(output, "%s ", cd->source_file_names[i]);
 }

 fprintf(output, "> %s\n\n", output_file_name);
 return 0;
}

int build_rule_component_bins_and_libs(struct bca_context *ctx,
                                       struct component_details *cd,
                                       struct project_details *pd,
                                       struct host_configuration *tc,
                                       FILE *output)
{
 int i, x, y, handled, yes, swapped;
 struct component_details output_names, cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));
 memset(&output_names, 0, sizeof(struct component_details));

 output_names.host = cd->host;
 output_names.component_type = cd->component_type;
 output_names.component_name = cd->component_name;
 output_names.component_output_name = cd->component_output_name;

 if(render_project_component_output_names(ctx, &output_names, RENDER_OUTPUT_NAME))
 {
  fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
  return 1;
 }

 /* first build targets for each object file */
 for(i=0; i<cd->n_source_files; i++)
 {
  handled = 0;

  if(strcmp(cd->source_file_extensions[i], ".c") == 0)
  {
   if(object_from_c_file(ctx, cd, pd, tc,
                         cd->source_file_base_names[i],
                         cd->source_file_names[i],
                         cd->rendered_names[0],
                         output))
   {
    fprintf(stderr,
            "BCA: object_from_c_file(%s.%s.%s) failed\n",
            cd->component_type, cd->host, cd->component_name);
    return 1;
   }

   handled = 1;
  }

  if( (strcmp(cd->source_file_extensions[i], ".cc") == 0) ||
      (strcmp(cd->source_file_extensions[i], ".cxx") == 0) ||
      (strcmp(cd->source_file_extensions[i], ".cpp") == 0) )
  {
   if(object_from_cxx_file(ctx, cd, pd, tc,
                           cd->source_file_base_names[i],
                           cd->source_file_names[i],
                           cd->rendered_names[0],
                           output))
   {
    fprintf(stderr,
            "BCA: object_from_cxx_file(%s.%s.%s) failed\n",
            cd->component_type, cd->host, cd->component_name);
    return 1;
   }

   handled = 1;
  }

  if(handled == 0)
  {
   fprintf(stderr,
           "BCA: I don't know how to build a project component of type %s with a file "
           "of type '%s'. (%s)\n",
            cd->component_type, cd->source_file_extensions[i], cd->source_file_names[i]);
   return 1;
  }
 }

 /* make a variable to refer to the set of all the above objects */
 fprintf(output, "%s-OBJECTS = ", cd->rendered_names[0]);

 for(i=0; i < cd->n_source_files; i++)
 {
  fprintf(output, "%s/obj/%s-%s",
          tc->build_prefix, cd->component_name, cd->source_file_base_names[i]);

  fprintf(output, "%s ", tc->obj_suffix);
 }
 fprintf(output, "\n\n");

 /* now make the component depend on its objects */
 fprintf(output, "%s : $(%s-OBJECTS) ", cd->rendered_names[0], cd->rendered_names[0]);

 /* the component's target also depends on its internal dependencies targets */
 for(y=0; y < cd->n_dependencies; y++)
 {
  x = 0;
  while(x < pd->n_components)
  {
   if(strcmp(cd->dependencies[y], pd->component_names[x]) == 0)
   {
    /* internel dep */
    swapped = 0;
    i = 0;
    while(i<ctx->n_swaps)
    {
     if(strcmp(pd->component_names[x], ctx->swapped_components[i]) == 0)
     {
      swapped = 1;
      break;
     }
     i++;
    }

    if(swapped == 0)
    {
     cd_d.host = cd->host;
    } else {
     cd_d.host = ctx->swapped_component_hosts[i];
    }
    cd_d.component_type = pd->component_types[x];
    cd_d.component_name = pd->component_names[x];
    cd_d.component_output_name = pd->component_output_names[x];

    if(render_project_component_output_names(ctx, &cd_d, RENDER_BUILD_OUTPUT_NAME))
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
      return 1;
     }

    fprintf(output, "%s ", cd_d.rendered_names[1]);

    free_rendered_names(&cd_d);
    break;
   }
   x++;
  }
 }

 /* make libraries depend on .pc files (or should it be the other way arround?) */
 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {
  fprintf(output, "%s", cd->rendered_names[1]);
 }

 /* target action is to link the above objects, etc */
 fprintf(output, "\n\t%s", tc->cc);

 if(strcmp(component_type_file_extension(ctx, tc, cd->component_type,
                                         cd->component_output_name), ".dylib") == 0)
 {
  fprintf(output, " -dynamiclib ");
 } else if (strcmp(cd->component_type, "SHAREDLIBRARY") == 0) {
  fprintf(output, " -shared ");
 }

 fprintf(output, " $(%s-OBJECTS) %s",
         cd->rendered_names[0], tc->cc_output_flag);

 /* figure out what the name of the output file should be,
    different for libraries and binaries */
 yes = 1;
 if(cd->n_rendered_names > 3)
 {
  if(cd->rendered_names[3][0] != 0)
  {
   yes = 0;
   fprintf(output, " %s", cd->rendered_names[3]);
  }
 }

 if(yes == 1)
  fprintf(output, " %s", cd->rendered_names[0]);

 /* add shared libary switches */
 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {
  if(contains_string(tc->cc, -1, "mingw", -1))
  {
   if(cd->n_rendered_names < 3)
   {
    fprintf(stderr, "BCA: there should have been an import name here\n");
    return 1;
   }
   fprintf(output, " -Wl,--out-implib,%s", cd->rendered_names[2]);
  } else {
   if(strcmp(component_type_file_extension(ctx, tc, cd->component_type,
                                           cd->component_output_name), ".dll") == 0)
   {
    /* cygwin */
    if(cd->n_rendered_names < 3)
    {
     fprintf(stderr, "BCA: there should have been an import name here\n");
     return 1;
    }
    fprintf(output, " -Wl,--out-implib,%s",
            cd->rendered_names[2]);
   }
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->component_type,
                                          cd->component_output_name), ".dylib") == 0)
  {
   fprintf(output, " -compatibility_version %s.%s -install_name %s",
           cd->major, cd->minor, cd->rendered_names[0]);
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->component_type,
                                          cd->component_output_name), ".so") == 0)
  {
   fprintf(output, " -Wl,-soname,%s",
           output_names.rendered_names[0]);
  }

 }
 fprintf(output, " ");

 /* both libraries and binaries may have LDFLAGS */
 if(tc->ldflags != NULL)
  fprintf(output, " %s", tc->ldflags);

 /* both libaries and binaries may have pkg-config dependencies */
 if(cd->n_dependencies > 0)
 {
  fprintf(output, "`");

  if(component_pkg_config_path(ctx, cd, tc, output))
   return 1;

  if(tc->pkg_config_libdir != NULL)
   fprintf(output, "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

  fprintf(output, "%s --libs ", tc->pkg_config);

  for(y=0; y < cd->n_dependencies; y++)
  {
   yes = 0;
   x = 0;
   while(x < pd->n_components)
   {
    if(strcmp(cd->dependencies[y], pd->component_names[x]) == 0)
    {
     cd_d.component_type = "SHAREDLIBRARY";
     cd_d.component_name = pd->component_names[x];
     resolve_component_version(ctx, &cd_d);

     fprintf(output, "%s-%s ", pd->component_output_names[x], cd_d.major);
     yes = 1;
     break;
    }
    x++;
   }

   if(yes == 0)
    fprintf(output, "%s ", cd->dependencies[y]);
  }

  fprintf(output, "`");
 }
 fprintf(output, "\n");

 /* soft links for linux style .so file versioning for the left-in-build-directory versions*/
 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {
  if(strcmp(component_type_file_extension(ctx, tc, cd->component_type,
                                          cd->component_output_name), ".dll") != 0)
  {
   fprintf(output, "\tcd %s; ln -sf %s %s\n",
           tc->build_prefix, output_names.rendered_names[3], output_names.rendered_names[0]);
  }
 }
 fprintf(output, "\n");

 /* lastly the .pc file if needed */
 if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0)
 {

  fprintf(output, "%s : Makefile\n",
          cd->rendered_names[1]);

  if(generate_host_component_pkg_config_file(ctx, cd, pd, tc,
                                             cd->rendered_names[1],
                                             output_names.rendered_names,
                                             output_names.n_rendered_names,
                                             output, 0))
  {
   fprintf(stderr,
           "BCA: generate_host_component_pkg_config_file(%s.%s.%s) failed\n",
           cd->component_type, cd->host, cd->component_name);
   return 1;
  }
 }

 if(free_rendered_names(&output_names))
  return 1;

 return 0;
}

int generate_host_component_target_dependencies(struct bca_context *ctx,
                                                struct component_details *cd,
                                                struct project_details *pd,
                                                char *output_file_name,
                                                FILE *output)
{
 int i, x, y, swapped;
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

 /* any component type may have a FILE_DEPENDS key */
 fprintf(output, "# dependencies and build rules for %s-%s\n",
         cd->host, cd->component_name);

 fprintf(output, "%s-FILE_DEPENDENCIES = ", output_file_name);

 for(i=0; i < cd->n_extra_file_deps; i++)
 {
  fprintf(output, "%s ", cd->extra_file_deps[i]);
 }

 for(i=0; i < cd->n_lib_headers; i++)
 {
  fprintf(output, "%s ", cd->lib_headers[i]);
 }

 /* look through the list of build configuration dependecies, and for the ones that are
    from the internal deps list from the project configurtation, add the .pc
    file as file dependency.
 */

 for(y=0; y < cd->n_dependencies; y++)
 {
  x = 0;
  while(x < pd->n_components)
  {
   if(strcmp(cd->dependencies[y], pd->component_names[x]) == 0)
   {
    swapped = 0;
    i = 0;
    while(i<ctx->n_swaps)
    {
     if(strcmp(pd->component_names[x], ctx->swapped_components[i]) == 0)
     {
      swapped = 1;
      break;
     }
     i++;
    }

    if(strcmp(pd->component_types[x], "SHAREDLIBRARY"))
    {
     fprintf(stderr,
             "BCA: project component \"%s\" has an internal dependency on component \"%s\" which "
             "has an unknown dependency relationship. i.e. type \"%s\" is not a library\n",
             cd->component_name, pd->component_names[x], pd->component_types[x]);
     return 1;
    }

    if(swapped == 0)
    {
     cd_d.host = cd->host;
    } else {
     cd_d.host = ctx->swapped_component_hosts[i];
    }

    cd_d.component_type = pd->component_types[x];
    cd_d.component_name = pd->component_names[x];
    cd_d.component_output_name = pd->component_output_names[x];

    if(render_project_component_output_names(ctx, &cd_d, RENDER_BUILD_OUTPUT_NAME))
    {
     fprintf(stderr, "BCA: render_project_component_ouput_names() failed on internal dep\n");
     return 1;
    }

    fprintf(output, "%s ", cd_d.rendered_names[0]);

    free_rendered_names(&cd_d);
    break;
   }
   x++;
  }
 }

 fprintf(output, "\n\n");
 return 0;
}

int count_host_component_target_dependencies(struct bca_context *ctx,
                                             struct component_details *cd)
{
 int count = 0;

 count += cd->n_extra_file_deps;
 count += cd->n_lib_headers;
 count += cd->n_dependencies;

 return count;
}

int make_component_build_rules(struct bca_context *ctx,
                               struct build_details *bd,
                               struct project_details *pd,
                               struct component_details *cd,
                               struct host_configuration *tc,
                               FILE *output)
{

 if(count_host_component_target_dependencies(ctx, cd) > 0)
 {
  if(generate_host_component_target_dependencies(ctx, cd, pd,
                                                 cd->rendered_names[0],
                                                 output))
  {
   fprintf(stderr,
           "BCA: generate_host_component_target_dependencies(%s.%s) failed\n",
           cd->host, cd->component_name);
   return 1;
  }
 }

 if(strcmp(cd->component_type, "CAT") == 0)
 {
  if(build_rule_component_concatenate(ctx, cd, cd->rendered_names[0], output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_concatentate_custom(%s.%s) failed\n",
           cd->host, cd->component_name);
   return 1;
  }

  return 0;
 }

 if(strcmp(cd->component_type, "MACROEXPAND") == 0)
 {
  if(build_rule_component_macroexpand(ctx, cd, cd->rendered_names[0], output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_macroexpand(%s.%s) failed\n",
           cd->host, cd->component_name);
   return 1;
  }

  return 0;
 }

 if(strcmp(cd->component_type, "CUSTOM") == 0)
 {
  if(build_rule_component_custom(ctx, cd, pd, tc, cd->rendered_names[0], output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_custom(%s.%s) failed\n",
           cd->host, cd->component_name);

   return 1;
  }

  return 0;
 }

 if(strcmp(cd->component_type, "PYTHONMODULE") == 0)
 {
  if(build_rule_component_pythonmodule(ctx, cd, tc, cd->rendered_names[0], output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_pythonmodule(%s.%s) failed\n",
           cd->host, cd->component_name);

   return 1;
  }

  return 0;
 }

 if(strcmp(cd->component_type, "BEAM") == 0)
 {
  if(build_rule_component_erlangbeam(ctx, cd, tc, cd->rendered_names[0], output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_erlangbeam(%s.%s) failed\n",
           cd->host, cd->component_name);

   return 1;
  }

  return 0;
 }

 if( (strcmp(cd->component_type, "BINARY") == 0) ||
     (strcmp(cd->component_type, "SHAREDLIBRARY") == 0) )
 {
  if(build_rule_component_bins_and_libs(ctx, cd, pd, tc, output))
  {
   fprintf(stderr,
           "BCA: build_rule_component_bins_and_libs(%s.%s.%s) failed\n",
           cd->component_type, cd->host, cd->component_name);
   return 1;
  }

  return 0;
 }

 fprintf(stderr,
         "BCA: I don't know what to do with component type %s. "
         "I should not have made it here.\n ",
         cd->component_type);

 return 1;
}

int fresh_config_depends_check(struct bca_context *ctx, struct component_details *cd)
{
 char **list = NULL, *value, **default_disables;
 int n_elements = 0, x, i, j, clear, n_default_disables;

 if(list_component_internal_dependencies(ctx, cd, &list, &n_elements))
 {
  fprintf(stderr, "BCA: list_component_internal_dependencies() failed\n");
  return 1;
 }

 if(n_elements)
 {
  for(i = 0; i < n_elements; i++)
  {
   clear = 0;
   for(x = 0; x < cd->n_dependencies; x++)
   {
    if(strcmp(list[i], cd->dependencies[x]) == 0)
    {
     clear = 1;
     break;
    }
   }

   if(clear == 0)
   {
    fprintf(stderr,
            "BCA: WARNING - In the project configuration, component \"%s\" has an internal "
            "dependency on project component \"%s\", but this is not reflected in the build "
            "configuration. Are you using a fresh configure?\n",
            cd->component_name, list[i]);
   }

   for(x = 0; x < ctx->n_disables; x++)
   {
    if(strcmp(list[i], ctx->disabled_components[x]) == 0)
    {
     fprintf(stderr,
             "BCA: Can not generate makefile because component \"%s\" has an internal "
             "dependency on component \"%s\", which has been disabled.\n",
             cd->component_name, list[i]);

     if((value = lookup_key(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            "NONE", "NONE", "DISABLES")) != NULL)
     {
      if(split_strings(ctx, value, -1, &n_default_disables, &default_disables))
      {
       fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
       return 1;
      }

      j = 0;
      while(j < n_default_disables)
      {
       if(strcmp(list[i], default_disables[j]) == 0)
       {
        fprintf(stderr,
                "BCA: Also, since \"%s\" is in the project's disable by default list "
                "(\"NONE.NONE.DISABLES\"), is it meant to be listed in:\n"
                "BCA: \"%s.%s.OPT_INT_DEPENDS\" instead of \"%s.%s.INT_DEPENDS\"?\n",
                list[i], cd->component_type, cd->component_name,
                cd->component_type, cd->component_name);
       }
       j++;
      }
      free(value);
     }

     return 1;
    }
   }
  }

  free_string_array(list, n_elements);
  list = NULL;
  n_elements = 0;
 }

 if(list_component_external_dependencies(ctx, cd, &list, &n_elements))
 {
  fprintf(stderr, "BCA: list_component_external_dependencies() failed\n");
  return 1;
 }

 if(n_elements)
 {
  for(i = 0; i < n_elements; i++)
  {
   clear = 0;
   for(x = 0; x < cd->n_dependencies; x++)
   {
    if(strcmp(list[i], cd->dependencies[x]) == 0)
    {
     clear = 1;
     break;
    }
   }

   if(clear == 0)
   {
    fprintf(stderr,
            "BCA: WARNING - In the project configuration, component \"%s\" has non-optional "
            "external dependency on package \"%s\", but this is not reflected in the build "
            "configuration. Are you using a fresh configure?\n",
            cd->component_name, list[i]);
   }
  }
  free_string_array(list, n_elements);
  list = NULL;
  n_elements = 0;
 }

 return 0;
}

int component_type_sanity_check(char *type)
{
 int x;

 char *valid_component_types[7] =
 {
  "BINARY",
  "SHAREDLIBRARY",
  "CAT",
  "MACROEXPAND",
  "PYTHONMODULE",
  "BEAM",
  "CUSTOM",
 };

 for(x=0; x<7; x++)
 {
  if(strcmp(type, valid_component_types[x]) == 0)
   return 0;
 }

 fprintf(stderr,
         "BCA: I don't know how to generate gmakefile rules for type type of \"%s\"\n",
         type);

 return 1;
}

char *generate_tar_name(struct bca_context *ctx, struct project_details *pd)
{
 int length = strlen(pd->project_name), i;
 char *t;

 if((t = malloc(length + 1)) == NULL)
  return NULL;

 for(i=0; i<length; i++)
 {
  if(pd->project_name[i] == ' ')
   t[i] = '_';
  else if(!isalnum(pd->project_name[i]))
   t[i] = '_';
  else if(isupper(pd->project_name[i]))
   t[i] = tolower(pd->project_name[i]);
  else
   t[i] = pd->project_name[i];
 }
 t[length] = 0;

 return t;
}

int make_tarball_target(struct bca_context *ctx,
                        struct project_details *pd,
                        FILE *output)
{
#ifndef IN_SINGLE_FILE_DISTRIBUTION
 int x, y, z, n_files, n_strings;
 char temp[512], subdir[512], **files, **strings, *tar_name;
 struct component_details cd;
 FILE *f;

 memset(&cd, 0, sizeof(struct component_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: make_tarball_target()\n");

 if((tar_name = generate_tar_name(ctx, pd)) == NULL)
 {
  fprintf(stderr, "BCA: generate_tar_name() failed\n");
  return 1;
 }

 fprintf(output, "#source distribution tarball creation\n");

 files = NULL;
 n_files = 0;

 if(add_to_string_array(&files, n_files,
                        "./buildconfiguration/projectconfiguration", -1, 0))
 {
  fprintf(stderr, "BCA: add_to_string_array() failed\n");
  return 1;
 }
 n_files++;

 for(x=0; x < pd->n_components; x++)
 {
  cd.component_name = pd->component_names[x];
  cd.component_type = pd->component_types[x];
  cd.component_output_name = pd->component_output_names[x];

  if(resolve_component_source_files(ctx, &cd))
   return 1;

  if(resolve_component_extra_file_dependencies(ctx, &cd))
   return 1;

  for(y=0; y<cd.n_source_files; y++)
  {
   if(add_to_string_array(&files, n_files, cd.source_file_names[y], -1, 1) == 0)
    n_files++;
  }

  for(y=0; y<cd.n_extra_file_deps; y++)
  {
   if(add_to_string_array(&files, n_files, cd.extra_file_deps[y], -1, 1) == 0)
    n_files++;
  }

  if(component_details_cleanup(&cd))
   return 1;
 }

 fprintf(output, "tar : ");
 for(x=0; x<n_files; x++)
 {
  fprintf(output, "%s ", files[x]);
 }
 fprintf(output, "\n");

 snprintf(temp, 512, "%s.%s", tar_name, pd->version_string);

 /* mkdir lines */
 strings = NULL;
 n_strings = 0;
 for(x=0; x<n_files; x++)
 {
  z = strlen(files[x]);
  if(z > 512)
  {
   fprintf(stderr, "BCA: file name %s too long\n", files[x]);
   return 1;
  }

  while(z > 0)
  {
   if(files[x][z] == '/')
   {
    z++;
    break;
   }
   z--;
  }
  if(z > 2)
  {
   memcpy(subdir, files[x] + 1, z - 1);
   subdir[z - 1] = 0;

   if(add_to_string_array(&strings, n_strings, subdir, z, 1) == 0)
    n_strings++;
  }
 }

 for(x=0; x<n_strings; x++)
 {
  fprintf(output, "\tmkdir -p ./%s%s\n", temp, strings[x]);
 }
 free_string_array(strings, n_strings);
 strings = NULL;
 n_strings = 0;

 /*cp lines */
 for(x=0; x<n_files; x++)
 {
  z = strlen(files[x]);

  while(z > 0)
  {
   if(files[x][z] == '/')
   {
    z++;
    break;
   }
   z--;
  }

  if(z > 0)
  {
   memcpy(subdir, files[x] + 1, z - 1);
   subdir[z - 1] = 0;
   fprintf(output, "\tcp %s ./%s%s\n", files[x], temp, subdir);
  } else {
   fprintf(output, "\tcp %s ./%s/\n", files[x], temp);
  }
 }

 fprintf(output, "\t./bca --output-configure > ./%s/configure\n", temp);
 fprintf(output, "\tchmod +x ./%s/configure\n", temp);
 fprintf(output,
         "\t./bca --output-buildconfigurationadjust.c > ./%s/buildconfiguration/buildconfigurationadjust.c\n",
         temp);

 if((f = fopen("./configure-autoconf", "r")) != NULL)
 {
  fprintf(output, "\tcp configure-autoconf ./%s/\n", temp);
  fprintf(output, "\tchmod +x ./%s/configure\n", temp);
  fclose(f);
 }

 if((f = fopen("./configure-extra", "r")) != NULL)
 {
  fprintf(output, "\tcp configure-extra ./%s/\n", temp);
  fprintf(output, "\tchmod +x ./%s/configure\n", temp);
  fclose(f);
 }

 fprintf(output, "\ttar -pczf %s.tar.gz ./%s\n", temp, temp);

 free_string_array(files, n_files);
 fprintf(output, "\n\n");

 free(tar_name);

#else
 fprintf(output, "#source distribution tarball creation support is not in single file distribution\n\n\n");
#endif
 return 0;
}

int generate_library_component_install_rule(struct bca_context *ctx, FILE *output,
                                            int uninstall_version,
                                            struct host_configuration *tc,
                                            struct component_details *cd,
                                            struct component_details *build_names,
                                            struct project_details *pd)
{
 int n_lib_headers, i, j;
 char **lib_headers;
 struct component_details output_names;
 char temp[1024];

 if(tc->install_include_dir == NULL)
 {
  fprintf(stderr, "BCA: I need a value for %s.[%s/ALL].INSTALL_INCLUDE_DIR\n",
          cd->host, cd->component_name);
   return 1;
 }

 n_lib_headers = 0;
 lib_headers = NULL;
 if(lookup_value_as_list(ctx, OPERATE_PROJECT_CONFIGURATION,
                         cd->component_type,
                         cd->component_name,
                         "LIB_HEADERS",
                         &lib_headers, &n_lib_headers))
  return 1;

 if(uninstall_version)
 {
  for(i=0; i < cd->n_rendered_names; i++)
  {
   if(cd->rendered_names[i][0] != 0)
    fprintf(output, "\trm %s\n", cd->rendered_names[i]);
  }

  for(i=0; i < n_lib_headers; i++)
  {
   j = strlen(lib_headers[i]);
   while(j > 0)
   {
    if(lib_headers[i][j] == '/')
    {
     j++;
     break;
    }
    j--;
   }
   snprintf(temp, 1024, "%s/%s", tc->install_include_dir, lib_headers[i] + j);
   fprintf(output, "\trm %s\n", temp);
  }

  return 0;
 }

 if(resolve_component_version(ctx, cd))
 {
  fprintf(stderr, "BCA: resolve_component_version() failed\n");
  return 1;
 }

 memset(&output_names, 0, sizeof(struct component_details));

 output_names.host = cd->host;
 output_names.component_type = cd->component_type;
 output_names.component_name = cd->component_name;

 if(render_project_component_output_names(ctx, &output_names, RENDER_OUTPUT_NAME))
 {
  fprintf(stderr, "BCA: generate_host_component_install_rules(): "
          "render_project_component_ouput_names() failed\n");
  return 1;
 }

 if(cd->n_rendered_names > 3)
 {
  fprintf(output, "\tinstall %s %s\n",
          build_names->rendered_names[3],
          tc->install_lib_dir);

  if(strcmp(build_names->rendered_extensions[0], ".dll") != 0)
  {
   fprintf(output, "\tcd %s; ln -sf %s %s\n",
           tc->install_lib_dir,
           output_names.rendered_names[3],
           output_names.rendered_names[0]);
  }
 }

 if(generate_host_component_pkg_config_file(ctx, cd, pd, tc,
                                            cd->rendered_names[1],
                                            output_names.rendered_names,
                                            output_names.n_rendered_names,
                                            output, 1))
 {
  fprintf(stderr, "BCA: generate_host_component_pkg_config_file() failed\n");
  return 1;
 }

 for(i=0; i<n_lib_headers; i++)
 {
  fprintf(output, "\tinstall %s %s\n", lib_headers[i], tc->install_include_dir);
 }

 if(free_rendered_names(&output_names))
  return 1;

 if(free_string_array(lib_headers, n_lib_headers))
  return 1;

 return 0;
}

int makefile_component_pass(struct bca_context *ctx,
                            struct project_details *pd,
                            struct build_details *bd,
                            FILE *output,
                            int pass, int host_i, int component_i,
                            int *unique_list_length, char ***unique_list)
{
 int swapped, i;
 struct host_configuration *tc;
 struct component_details cd, install_cd;
 char temp[512], *install_directory;

 if(pass == MAKE_PASS_BUILD_RULES)
  if(component_type_sanity_check(pd->component_types[component_i]))
   return 1;

 if(engage_build_configuration_disables_for_host(ctx, bd->hosts[host_i]))
 {
  fprintf(stderr,
          "BCA: engage_build_configuration_disables_for_host(%s) failed\n",
          bd->hosts[host_i]);
  return 1;
 }

 if(engage_build_configuration_swaps_for_host(ctx, bd->hosts[host_i]))
 {
  fprintf(stderr,
          "BCA: engage_build_configuration_swaps_for_host(%s) failed\n",
           bd->hosts[host_i]);
  return 1;
 }

 i = 0;
 while(i < ctx->n_disables)
 {
  if(strcmp(ctx->disabled_components[i], pd->component_names[component_i]) == 0)
   return 0;

  i++;
 }

 swapped = -1;
 i = 0;
 while(i<ctx->n_swaps)
 {
  if(strcmp(pd->component_names[component_i], ctx->swapped_components[i]) == 0)
  {
   swapped = i;
   break;
  }
  i++;
 }

 memset(&cd, 0, sizeof(struct component_details));

 if(swapped == -1)
 {
  cd.host = bd->hosts[host_i];
 } else {
  switch(pass)
  {
   case MAKE_PASS_INSTALL_RULES:
   case MAKE_PASS_UNINSTALL_RULES:
        return 0;
        break;
  }
  cd.host = ctx->swapped_component_hosts[swapped];
 }

 if((tc = resolve_host_configuration(ctx, cd.host,
                                     pd->component_names[component_i])) == NULL)
 {
  fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
  return 1;
 }

 cd.component_type = pd->component_types[component_i];
 cd.component_name = pd->component_names[component_i];
 cd.component_output_name = pd->component_output_names[component_i];

 if(render_project_component_output_names(ctx, &cd, RENDER_BUILD_OUTPUT_NAME)) //needs to pass tc
 {
  fprintf(stderr, "BCA: render_project_component_ouput_names() failed\n");
   return 1;
 }

 memset(&install_cd, 0, sizeof(struct component_details));
 switch(pass)
 {
  case MAKE_PASS_INSTALL_RULES:
  case MAKE_PASS_UNINSTALL_RULES:
       install_cd.host = cd.host;
       install_cd.component_type = cd.component_type;
       install_cd.component_name = cd.component_name;
       install_cd.component_output_name = cd.component_output_name;
       if(render_project_component_output_names(ctx, &install_cd, RENDER_INSTALL_OUTPUT_NAME)) //needs to pass tc
       {
        fprintf(stderr, "BCA: render_project_component_ouput_names() failed\n");
        return 1;
       }
       break;
 }

 switch(pass)
 {
  case MAKE_PASS_CLEAN_RULES:
       snprintf(temp, 512, "%s/obj/*", tc->build_prefix);
       if(add_to_string_array(unique_list, *unique_list_length, temp, -1, 1) == 0)
       {
        fprintf(output, "%s ", temp);
        (*unique_list_length)++;
       }

       if(strcmp(pd->component_types[component_i], "SHAREDLIBRARY") == 0)
       {
        /* the .pc rendered by render_project_component_output_names() will be the versioned one,
          this covers the unversioned symlink */
        fprintf(output, "%s/%s.pc ", tc->build_prefix,
                pd->component_output_names[component_i]);
       }

       for(i=0; i < cd.n_rendered_names; i++)
       {
        if(cd.rendered_names[i][0] != 0)
         fprintf(output, "%s ", cd.rendered_names[i]);
       }
       break;

  case MAKE_PASS_INSTALL_RULES:
       if(resolve_component_installation_path(ctx, cd.host,
                                              cd.component_type,
                                              cd.component_name,
                                              &install_directory))
       {
        fprintf(stderr,
                "BCA: resolve_component_installation_path(%s, %s, %s) failed\n",
                cd.host, cd.component_type, cd.component_name);
        return 1;
       }

       fprintf(output, "%s-%s-install : %s\n",
               cd.host, cd.component_name, cd.rendered_names[0]);
       if( (strcmp(cd.component_type, "BINARY") == 0)      ||
           (strcmp(cd.component_type, "CAT") == 0)         ||
           (strcmp(cd.component_type, "MACROEXPAND") == 0) ||
           (strcmp(cd.component_type, "CUSTOM") == 0) )
       {
        fprintf(output, "\tinstall %s %s\n",
                cd.rendered_names[0], install_directory);
       } else if(strcmp(cd.component_type, "SHAREDLIBRARY") == 0) {
        if(generate_library_component_install_rule(ctx, output, 0,
                                                   tc, &cd, &cd, pd))
         return 1;
       }
       break;

  case MAKE_PASS_INSTALL_RULES_2:
       fprintf(output, "%s-%s-install ",
               cd.host, cd.component_name);
       break;

  case MAKE_PASS_UNINSTALL_RULES:
       fprintf(output, "%s-%s-uninstall :\n", cd.host, cd.component_name);
       if( (strcmp(cd.component_type, "BINARY") == 0)      ||
           (strcmp(cd.component_type, "CAT") == 0)         ||
           (strcmp(cd.component_type, "MACROEXPAND") == 0) ||
           (strcmp(cd.component_type, "CUSTOM") == 0) )
       {
        for(i=0; i < install_cd.n_rendered_names; i++)
        {
         if(install_cd.rendered_names[i][0] != 0)
          fprintf(output, "\trm %s\n", install_cd.rendered_names[0]);
        }
       } else if(strcmp(cd.component_type, "SHAREDLIBRARY") == 0) {
        if(generate_library_component_install_rule(ctx, output, 1,
                                                   tc, &cd, &cd, pd))
         return 1;
       }
       break;

  case MAKE_PASS_UNINSTALL_RULES_2:
       fprintf(output, "%s-%s-uninstall ",
               cd.host, cd.component_name);
       break;

  case MAKE_PASS_BUILD_RULES:
       if(component_details_resolve_all(ctx, &cd, pd))
        return 1;

       if(derive_file_dependencies_from_inputs(ctx, tc, &cd, pd))
        return 1;

       if(fresh_config_depends_check(ctx, &cd))
        return 1;

       if(make_component_build_rules(ctx, bd, pd, &cd, tc, output))
        return 1;
       break;

  case MAKE_PASS_HOST_TARGETS:
       fprintf(output, "%s ", cd.rendered_names[0]);
       break;
 }

 if(free_rendered_names(&install_cd))
  return 1;

 if(free_host_configuration(ctx, tc))
  return 1;

 if(free_rendered_names(&cd))
  return 1;

 if(component_details_cleanup(&cd))
  return 1;

 return 0;
}

int makefile_mode_pass(struct bca_context *ctx,
                       struct project_details *pd,
                       struct build_details *bd,
                       FILE *output, int pass)
{
 int host_i, component_i;
 char **unique_list = NULL;
 int unique_list_length = 0;

 /* pre-looping over hosts */
 switch(pass)
 {
  case MAKE_PASS_ALL:
       fprintf(output, "# by default build everything\n");
       fprintf(output, "all : ");
       break;

  case MAKE_PASS_HOST_TARGETS:
       fprintf(output, "# top level host definitions\n");
       break;

  case MAKE_PASS_HELP:
       fprintf(output, "# output for \"Make help\"\n");
       fprintf(output, "help :\n");
       fprintf(output, "\t@echo \"Project: %s\"\n", pd->project_name);
       fprintf(output, "\t@echo \"Primary targets in this Makefile:\"\n");
       fprintf(output, "\t@echo \" clean - removes all files created by the non-install targets "
               "(this is not a reconfigure)\"\n");
       fprintf(output, "\t@echo \" install \"\n");
       fprintf(output, "\t@echo \" uninstall \"\n");
       fprintf(output, "\t@echo \" help - print this message\"\n");
#ifndef IN_SINGLE_FILE_DISTRIBUTION
       fprintf(output, "\t@echo \" tar - create source tarball\"\n");
#endif
       fprintf(output, "\t@echo \" all - default target (builds all of the below)\"\n");
       break;

  case MAKE_PASS_CLEAN_RULES:
       fprintf(output, "# clean host and top level definitions\n");
       break;

  case MAKE_PASS_CLEAN_RULES_2:
       fprintf(output, "clean :\n\trm -f ");
       break;

  case MAKE_PASS_INSTALL_RULES_2:
       fprintf(output, "# install host and top level definitions\n");
       break;

  case MAKE_PASS_INSTALL_RULES_3:
       fprintf(output, "install : ");
       break;

  case MAKE_PASS_UNINSTALL_RULES_2:
       fprintf(output, "# uninstall host and top level definitions\n");
       break;

  case MAKE_PASS_UNINSTALL_RULES_3:
       fprintf(output, "uninstall : ");
       break;

  case MAKE_PASS_TARBALL_RULES:
       /* tar is different in that per-build-host concepts don't apply */
       if(make_tarball_target(ctx, pd, output))
        return 1;

       return 0;
       break;
 }

 /* looping over hosts */
 switch(pass)
 {
  case MAKE_PASS_ALL:
  case MAKE_PASS_HOST_TARGETS:
  case MAKE_PASS_HELP:
  case MAKE_PASS_CLEAN_RULES:
  case MAKE_PASS_CLEAN_RULES_2:
  case MAKE_PASS_INSTALL_RULES:
  case MAKE_PASS_INSTALL_RULES_2:
  case MAKE_PASS_INSTALL_RULES_3:
  case MAKE_PASS_UNINSTALL_RULES:
  case MAKE_PASS_UNINSTALL_RULES_2:
  case MAKE_PASS_UNINSTALL_RULES_3:
  case MAKE_PASS_BUILD_RULES:
       /* pre-looping over components */
       for(host_i = 0; host_i < bd->n_hosts; host_i++)
       {
        switch(pass)
        {
         case MAKE_PASS_ALL:
              fprintf(output, "%s ", bd->hosts[host_i]);
              break;

         case MAKE_PASS_HELP:
              fprintf(output, "\t@echo \" %s\"\n", bd->hosts[host_i]);
              break;

         case MAKE_PASS_CLEAN_RULES:
              fprintf(output, "%s-clean-targets = ", bd->hosts[host_i]);
              unique_list_length = 0;
              unique_list = NULL;
              break;

         case MAKE_PASS_CLEAN_RULES_2:
              fprintf(output, "$(%s-clean-targets) ", bd->hosts[host_i]);
              break;

         case MAKE_PASS_INSTALL_RULES_2:
              fprintf(output, "%s-install-targets = ", bd->hosts[host_i]);
              break;

         case MAKE_PASS_INSTALL_RULES_3:
              fprintf(output, "$(%s-install-targets) ", bd->hosts[host_i]);
              break;

         case MAKE_PASS_UNINSTALL_RULES_2:
              fprintf(output, "%s-uninstall-targets = ", bd->hosts[host_i]);
              break;

         case MAKE_PASS_UNINSTALL_RULES_3:
              fprintf(output, "$(%s-uninstall-targets) ", bd->hosts[host_i]);
              break;
        }
       }

       /* looping over components */
       switch(pass)
       {
        case MAKE_PASS_HOST_TARGETS:
        case MAKE_PASS_CLEAN_RULES:
        case MAKE_PASS_INSTALL_RULES:
        case MAKE_PASS_INSTALL_RULES_2:
        case MAKE_PASS_UNINSTALL_RULES:
        case MAKE_PASS_UNINSTALL_RULES_2:
        case MAKE_PASS_BUILD_RULES:
             for(host_i = 0; host_i < bd->n_hosts; host_i++)
             {

              if(pass == MAKE_PASS_HOST_TARGETS)
               fprintf(output, "%s : ", bd->hosts[host_i]);

              for(component_i = 0; component_i < pd->n_components; component_i++)
              {
               if(makefile_component_pass(ctx, pd, bd, output, pass, host_i, component_i,
                                          &unique_list_length, &unique_list))
                return 1;
              }

              if(pass == MAKE_PASS_HOST_TARGETS)
               fprintf(output, "\n\n");

             }
             break;
       }

       /* post-looping over components */
       switch(pass)
       {
        case MAKE_PASS_UNINSTALL_RULES:
        case MAKE_PASS_INSTALL_RULES:
             fprintf(output, "\n");
             break;

        case MAKE_PASS_CLEAN_RULES:
        case MAKE_PASS_INSTALL_RULES_2:
        case MAKE_PASS_UNINSTALL_RULES_2:
             fprintf(output, "\n\n");
             break;

       }
       break;
 }

 /* post-looping over hosts */
 switch(pass)
 {
  case MAKE_PASS_ALL:
  case MAKE_PASS_CLEAN_RULES_2:
  case MAKE_PASS_INSTALL_RULES_3:
  case MAKE_PASS_UNINSTALL_RULES_3:
       fprintf(output, "\n\n\n");
       break;

  case MAKE_PASS_HELP:
       fprintf(output,
               "\t@echo \"This Makefile was generated with Build Configuration Adjust "
               "version %s\"\n\n\n", BCA_VERSION);
       break;
 }

 return 0;
}

int generate_makefile_mode(struct bca_context *ctx)
{
 struct project_details *pd;
 struct build_details bd;
 FILE *output;
 int pass;

 memset(&bd, 0, sizeof(struct build_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_makefile_mode()\n");

 if(load_project_config(ctx, 0))
  return 1;

 if(load_build_config(ctx, 0))
  return 1;

 if(list_build_hosts(ctx, &bd))
 {
  fprintf(stderr, "BCA: list_build_hosts() failed\n");
  return 1;
 }

 if((pd = resolve_project_details(ctx)) == NULL)
 {
  fprintf(stderr, "BCA: resolve_project_details() failed\n");
  return 1;
 }

 if(check_duplicate_output_names(ctx, pd))
 {
  fprintf(stderr, "BCA: check_duplicate_output_names() failed\n");
  return 1;
 }

 if((output = fopen("Makefile", "w")) == NULL)
 {
  perror("BCA: fopen(\"Makefile\")");
  return 1;
 }

 fprintf(output,
         "# This Makefile for GNU Make was generated by Build Configuration Adjust\n"
         "# See http://bca.stoverenterprises.com for more information\n\n");

 for(pass=0; pass<N_MAKE_PASSES; pass++)
 {
  if(makefile_mode_pass(ctx, pd, &bd, output, pass))
   return 1;
 }

 fclose(output);
 free_project_details(pd);
 free_string_array(bd.hosts, bd.n_hosts);

 return 0;
}
