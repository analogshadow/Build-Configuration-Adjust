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


int gmake_clean_rules(struct bca_context *ctx, FILE *output, 
                      char **hosts, int n_build_hosts,
                      struct component_details *cd)
{
 int x, y, array_length = 0, n_names, i, swapped;
 struct host_configuration *tc;
 char temp[512], **array = NULL, **names;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: gmake_clean_rules()\n");

 fprintf(output, "# cleaning rules\n");

 for(x=0; x<n_build_hosts; x++)
 {
  fprintf(output, "%s-clean-targets = ", hosts[x]); 

  if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
  {
   fprintf(stderr, "BCA: engage_build_configuration_disables_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(engage_build_configuration_swaps_for_host(ctx, hosts[x]))
  {
  fprintf(stderr, "BCA: engage_build_configuration_swaps_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(list_project_components(ctx, cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   return 1;
  }

  for(y=0; y < cd->n_components; y++)
  {
   swapped = 0;
   i = 0;
   while(i<ctx->n_swaps)
   {
    if(strcmp(cd->project_components[y], ctx->swapped_components[i]) == 0)
    {
     swapped = 1;
     break;
    }       
    i++;
   }

   if(swapped == 0)
   {
    cd->project_component = cd->project_components[y];
    cd->project_component_type = cd->project_component_types[y];
    cd->host = hosts[x];

    if((tc = resolve_host_configuration(ctx, cd)) == NULL)
    {
     fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
     return 1;
    }

    snprintf(temp, 512, "%s/obj/*", tc->build_prefix);

    if(add_to_string_array(&array, array_length, temp, -1, 1) == 0)
    {
     fprintf(output, "%s ", temp);
     array_length++;
    }

    if((n_names = 
        render_project_component_output_name(ctx, hosts[x], 
                                             cd->project_components[y], 2, &names, NULL)) < 0)
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
     return 1;
    }

    if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0)
    {
     /* the .pc rendered by render_project_component_output_name() will be the versioned one,
        this covers the unversioned symlink */
     fprintf(output, "%s/%s.pc ", tc->build_prefix, cd->project_output_names[y]);
    }

    for(i=0; i<n_names; i++)
    {
     if(names[i][0] != 0)
      fprintf(output, "%s ", names[i]);
    }
    free_string_array(names, n_names);

    free_host_configuration(ctx, tc);
   }

  }

  fprintf(output, "\n\n");
 }

 fprintf(output, "clean :\n\trm -f ");
 for(x=0; x<n_build_hosts; x++)
 {
  fprintf(output, "$(%s-clean-targets) ", hosts[x]); 
 }
 fprintf(output, "\n\n");

 free_string_array(array, array_length);

 return 0;
}

int gmake_help(struct bca_context *ctx, FILE *output, 
                      char **hosts, int n_build_hosts,
                      struct component_details *cd)
{
 int x;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: gmake_help()\n");

 fprintf(output, "# output for \"Make help\"\n");
 fprintf(output, "help :\n");
 fprintf(output, "\t@echo \"Project: %s\"\n", ctx->project_name);
 fprintf(output, "\t@echo \"Primary targets in this Makefile:\"\n");

 fprintf(output, "\t@echo \" clean - removes all files created by the non-install targets "
                 "(this is not a reconfigure)\"\n");
 fprintf(output, "\t@echo \" install \"\n");
 fprintf(output, "\t@echo \" uninstall \"\n");
 fprintf(output, "\t@echo \" help - print this message\"\n");
 fprintf(output, "\t@echo \" tar - create source tarball\"\n");
 fprintf(output, "\t@echo \" all - default target (builds all of the below)\"\n");

 for(x=0; x<n_build_hosts; x++)
 {
  fprintf(output, "\t@echo \" %s\"\n", hosts[x]); 
 }

 fprintf(output, "\t@echo \"This Makefile was generated with Build Configuration Adjust "
         "version X\"\n\n");

 return 0;
}

int generate_gmake_host_components(struct bca_context *ctx, FILE *output, 
                                   char **hosts, int n_hosts,
                                   struct component_details *cd)
{
 int x, y, z, n_names, yes, i, swapped;
 char **names;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_gmake_host_components()\n");
 
 fprintf(output, "# here we define which components get built for each host\n");
 for(x=0; x<n_hosts; x++)
 {
  fprintf(output, "%s : ", hosts[x]);

  if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
  {
   fprintf(stderr, "BCA: engage_build_configuration_disables_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(engage_build_configuration_swaps_for_host(ctx, hosts[x]))
  {
   fprintf(stderr, "BCA: engage_build_configuration_swaps_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(list_project_components(ctx, cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   return 1;
  }

  for(y=0; y < cd->n_components; y++)
  {
   yes = 1;

   if(strcmp(cd->project_component_types[y], "PYTHONMODULE") == 0)
    yes = 0;

   swapped = 0;
   i = 0;
   while(i<ctx->n_swaps)
   {
    if(strcmp(cd->project_components[y], ctx->swapped_components[i]) == 0)
    {
     swapped = 1;
     break;
    }       
    i++;
   }

   if(yes)
   {
    if(swapped == 0)
    {
     if((n_names = 
         render_project_component_output_name(ctx, hosts[x], 
                                              cd->project_components[y], 
                                              2, &names, NULL)) < 1)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
      return 1;
     }
    } else {
     if((n_names = 
         render_project_component_output_name(ctx, ctx->swapped_component_hosts[i], 
                                              cd->project_components[y], 
                                              2, &names, NULL)) < 1)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
      return 1;
     }
    }

    for(z=0; z<n_names; z++)
    {
     fprintf(output, "%s ", names[z]);
    }
    free_string_array(names, n_names);
   } 
  }

  fprintf(output, "\n");
 }

 return 0;
}

int gmake_host_component_file_rule_cflags(struct bca_context *ctx, FILE *output, 
                                          struct component_details *cd,
                                          struct host_configuration *tc)
{
 int i;
 char *string;

 if(tc->cflags != NULL)
 {
  fprintf(output, "%s ", tc->cflags);
 }

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

int derive_file_dependencies_from_inputs(struct bca_context *ctx,
                                         struct host_configuration *tc,
                                         struct component_details *cd)
{
 int i, x, handled;
 char temp[1024], *base_file_name, *extension;

 /* tranlaste the .INPUTs to .FILES */


//swaps logic needed here

 if(cd->n_file_names == 0)
 {
  cd->file_names = NULL;
  cd->file_base_names = NULL;
  cd->file_extensions = NULL;
 }

 for(i=0; i < cd->n_inputs; i++)
 {
  snprintf(temp, 1024, "%s/%s",
           tc->build_prefix, cd->inputs[i]);

  if(path_extract(temp, &base_file_name, &extension))
  {
   return 1;
  }

  handled = 0;

  if( (strcmp(cd->project_component_type, "CUSTOM") == 0) ||
      (strcmp(cd->project_component_type, "MACROEXPAND") == 0) ||
      (strcmp(cd->project_component_type, "CAT") == 0) )
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

  if(strcmp(extension, "py") == 0)
  {
   handled = 1;
  }

  if( (handled == 0) && (strcmp(extension, "c") == 0) )
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

  if( (handled == 0) && (strcmp(extension, "h") == 0) )
  {
   if(add_to_string_array(&(cd->file_deps), cd->n_file_deps, 
                          temp, -1, 0))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   cd->n_file_deps++;

   x = strlen(temp);
   while(x > 0)
   {
    if(temp[x] == '/')
     break;
    
    temp[x--] = 0;
   }

   if(add_to_string_array(&(cd->include_dirs), cd->n_include_dirs, 
                          temp, -1, 0))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return 1;
   }

   cd->n_include_dirs++;
   handled = 1;
  }

  if(handled == 0)
  {
   fprintf(stderr, 
           "BCA: I don't know what to do with with input %s\n", cd->inputs[i]);
   return 1;
  }

  free(base_file_name);
  free(extension);
 }

 return 0;
}

int generate_gmake_host_component_pythonmodule(struct bca_context *ctx,  
                                               struct component_details *cd,
                                               struct host_configuration *tc,
                                               char *output_file_name,
                                               FILE *output)
{
 int i;

 for(i=0; i<cd->n_file_names; i++)
 {
 
  if(strcmp(cd->file_extensions[i], "c") == 0)
  {
   fprintf(output, 
           "# finish me: should define howto build %s as part of python "
           "module %s for host %s\n", 
           cd->file_names[i], cd->project_component, cd->host);
  } else {
   fprintf(stderr, "I don't know how to build a python module from file \"%s\".\n",
           cd->file_names[i]);
   return 1;
  }
 }

 return 0;
}

int generate_gmake_host_component_custom(struct bca_context *ctx,  
                                         struct component_details *cd,
                                         struct host_configuration *tc,
                                         char *output_file_name,
                                         FILE *output)
{
 char *value;
 int yes, i;

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents, 
                        ctx->project_configuration_length, 
                        "CUSTOM", 
                        cd->project_component, 
                        "DRIVER")) == NULL)
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
 yes = i;
 free(value);

 fprintf(output, "%s : $(%s-FILE_DEPENDENCIES) ",
         output_file_name, output_file_name);

 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 if(cd->file_names == NULL)
 {
  fprintf(stderr, "BCA: CUSTOM component \"%s\", does not seem to have .INPUT or .FILES\n", 
          cd->project_component);
  return 1;
 }

 fprintf(output, "%s/%s\n", tc->build_prefix, cd->project_output_names[yes]);

 fprintf(output, "\tchmod +x %s/%s\n", tc->build_prefix, cd->project_output_names[yes]);
 fprintf(output, "\t%s/%s %s %s\n", 
         tc->build_prefix, cd->project_output_names[yes],
         cd->file_names[0], output_file_name);

 fprintf(output, "\n");

 return 0;
}

int generate_gmake_host_component_macroexpand(struct bca_context *ctx,  
                                              struct component_details *cd,
                                              char *output_file_name,
                                              FILE *output)
{
 int i;

 fprintf(output, "%s : $(%s-FILE_DEPENDENCIES) ",
         output_file_name, output_file_name);
 
 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 fprintf(output, "\n\t./bca --host %s --replacestrings < ", cd->host);

 if(cd->n_file_names < 1)
 {
  fprintf(stderr, 
          "BCA: MACROEXPAND component %s has no source files. Did you use .INPUT or .FILES?\n",
          cd->project_component);
  return 1;
 }

 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 fprintf(output, "> %s\n\n", output_file_name);

 return 0;
}

int generate_gmake_host_component_concatenate(struct bca_context *ctx,  
                                              struct component_details *cd,
                                              char *output_file_name,
                                              FILE *output)
{
 int i;

 fprintf(output, "%s : $(%s-FILE_DEPENDENCIES) ",
         output_file_name, output_file_name);
 
 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 fprintf(output, "\n\t./bca --concatenate ");

 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 fprintf(output, "> %s\n\n", output_file_name);
 return 0;
}

int generate_host_component_target_dependencies(struct bca_context *ctx,  
                                                struct component_details *cd,
                                                char *output_file_name,
                                                FILE *output)
{
 int i, x, y, n_names;
 char **names = NULL;

 /* any component type may have a FILE_DEPENDS key */
 fprintf(output, "\n# dependencies and build rules for %s-%s\n",
         cd->host, cd->project_component);

 fprintf(output, "%s-FILE_DEPENDENCIES = ", output_file_name);

 for(i=0; i < cd->n_file_deps; i++)
 {
  fprintf(output, "%s ", cd->file_deps[i]);
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
  while(x < cd->n_components)
  {
   if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
   {
    if(strcmp(cd->project_component_types[x], "SHAREDLIBRARY"))
    {
     fprintf(stderr, 
             "BCA: project component \"%s\" has an internal dependency on component \"%s\" which "
             "has an unknown dependency relationship. i.e. type \"%s\" is not a library\n",
             cd->project_component, cd->project_components[x], cd->project_component_types[x]);
     return 1;
    }

    if((n_names = 
        render_project_component_output_name(ctx, cd->host, cd->project_components[x],
                                             2, &names, NULL)) < 2)
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
     return 1;
    }
    fprintf(output, "%s ", names[1]);

    free_string_array(names, n_names);
    break;
   }
   x++;
  }
 }

 fprintf(output, "\n\n");
 return 0;
}

int object_from_c_file(struct bca_context *ctx,  
                       struct component_details *cd,
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
          tc->build_prefix, cd->project_component, 
          source_file_base_name, tc->obj_suffix);

 fprintf(output, 
         "%s : %s $(%s-FILE_DEPENDENCIES)\n",
         temp, source_file_name, output_file_name);

 fprintf(output, "\t%s ", tc->cc);

 if(gmake_host_component_file_rule_cflags(ctx, output, cd, tc))
  return 1;

 /* handle dependencies (internal and extrnal) */
 if(cd->n_dependencies > 0)
 {
  fprintf(output, "`");

  if(tc->pkg_config_path != NULL)
   fprintf(output, "PKG_CONFIG_PATH=%s ", tc->pkg_config_path);

  if(tc->pkg_config_libdir != NULL)
   fprintf(output, "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

  fprintf(output, "%s --cflags ", tc->pkg_config);

  for(y=0; y < cd->n_dependencies; y++)
  {
   yes = 0;
   x = 0;
   while(x < cd->n_components)
   {
    if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
    {
     resolve_component_version(ctx, ctx->project_configuration_contents, 
                               ctx->project_configuration_length, &cd_d,
                               "SHAREDLIBRARY", cd->project_components[x]);
     fprintf(output, "%s-%s ", cd->project_output_names[x], cd_d.major);
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

 if(strcmp(cd->project_component_type, "BINARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_bin_obj_flag);

 if(strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0)
  fprintf(output, "%s ", tc->cc_compile_shared_library_obj_flag);

 fprintf(output, "%s ", source_file_name);

 fprintf(output, "%s ", tc->cc_output_flag);

 fprintf(output, "%s\n\n", temp);

 return 0;
}

int generate_host_component_pkg_config_file(struct bca_context *ctx,  
                                            struct component_details *cd,
                                            struct host_configuration *tc,
                                            char **output_file_names,
                                            int n_output_file_nmes,
                                            FILE *output)
{
 int x, i, yes;
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

/* 
   Idea / question:
    reconcille package config's --variable and --define-variable with BCA variables
*/
  fprintf(output, "%s : %s\n",
          output_file_names[1], output_file_names[0]);

  fprintf(output, "\trm -f %s\n", output_file_names[1]);

  fprintf(output, "\techo \"prefix=%s/%s\" >> %s\n", 
#ifdef HAVE_CWD
          ctx->cwd, 
#else
          "`pwd`",
#endif
          tc->build_prefix, output_file_names[1]);

  fprintf(output, "\techo 'exec_prefix=$${prefix}' >> %s\n", output_file_names[1]);
  fprintf(output, "\techo 'libdir=$${exec_prefix}' >> %s\n", output_file_names[1]);
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
            cd->include_dirs[0] + 2, output_file_names[1]);
   } else {
    fprintf(output, "\techo 'includedir=%s' >> %s\n", cd->include_dirs[0], output_file_names[1]);
   }
  }
  fprintf(output, "\techo 'Name: %s' >> %s\n", cd->project_component, output_file_names[1]);
  fprintf(output, "\techo 'Description: %s' >> %s\n", 
          cd->project_component, output_file_names[1]);
  fprintf(output, "\techo 'Version: %s.%s' >> %s\n", 
          cd->major, cd->minor, output_file_names[1]);
  fprintf(output, "\techo 'Requires: ");
  for(i=0; i < cd->n_dependencies; i++)
  {
   yes = 0;
   x = 0;
   while(x < cd->n_components)
   {
    if(strcmp(cd->dependencies[i], cd->project_components[x]) == 0)
    {
     resolve_component_version(ctx, ctx->project_configuration_contents, 
                               ctx->project_configuration_length, &cd_d,
                                "SHAREDLIBRARY", cd->project_components[x]);

     fprintf(output, "%s-%s ", cd->project_output_names[x], cd_d.major);
     yes = 1;
     break;
    }
    x++;
   }

   if(yes == 0)
    fprintf(output, "%s ", cd->dependencies[i]);
  }

  fprintf(output, "' >> %s\n", output_file_names[1]);

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".so") == 0)
  {
   fprintf(output, "\techo 'Libs: $${libdir}/%s", output_file_names[0]);
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".dylib") == 0)
  {
   fprintf(output, "\techo 'Libs: $${libdir}/%s", output_file_names[0]);
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".dll") == 0)
  {
   fprintf(output, "\techo 'Libs: -L$${libdir} -l%s-%s.%s", 
           cd->project_component_output_name, cd->major, cd->minor);
  }

  if(tc->ldflags != NULL)
   fprintf(output, " %s", tc->ldflags);

  fprintf(output, "' >> %s\n", output_file_names[1]);

  fprintf(output, "\techo 'Cflags:");

  if(cd->n_include_dirs > 0)
   fprintf(output, " -I$${includedir} ");

  if(gmake_host_component_file_rule_cflags(ctx, output, cd, tc))
   return 1;

  fprintf(output, "' >> %s\n", output_file_names[1]);
  
  /* add unversioned symlink */
  fprintf(output, "\tcd %s; ln -sf %s-%s.pc %s.pc", 
          tc->build_prefix, cd->project_component_output_name, 
          cd->major, cd->project_component_output_name);

 return 0;
}

int generate_gmake_host_component_bins_and_libs(struct bca_context *ctx,  
                                                struct component_details *cd,
                                                struct host_configuration *tc,
                                                char **output_file_names,
                                                int n_output_file_nmes,
                                                FILE *output)
{
 char **names_d;
 int i, x, y, handled, yes, n_names_d;
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

 /* first build targets for each object file */
 for(i=0; i<cd->n_file_names; i++)
 {
  handled = 0;

  if(strcmp(cd->file_extensions[i], "c") == 0)
  {
   if(object_from_c_file(ctx, cd, tc, 
                         cd->file_base_names[i],
                         cd->file_names[i],
                         output_file_names[0],
                         output))
   {
    fprintf(stderr, 
            "BCA: object_from_c_file(%s.%s.%s) failed\n", 
            cd->project_component_type, cd->host, cd->project_component);
    return 1;
   }

   handled = 1;
  }

  if(handled == 0)
  {
   fprintf(stderr,
           "BCA: I don't know how to build a project component of type %s with a file "
           "of type '%s'. (%s)\n",
            cd->project_component_type, cd->file_extensions[i], cd->file_names[i]);
   return 1;
  }
 }

 /* make a variable to refer to the set of all the above objects */
 fprintf(output, "%s-OBJECTS = ", output_file_names[0]);

 for(i=0; i < cd->n_file_names; i++)
 {
  fprintf(output, "%s/obj/%s-%s", 
          tc->build_prefix, cd->project_component, cd->file_base_names[i]);

  fprintf(output, "%s ", tc->obj_suffix);
 }
 fprintf(output, "\n\n");

 /* now make the component depend on its objects */
 fprintf(output, "%s : $(%s-OBJECTS) ", output_file_names[0], output_file_names[0]);

 /* the component's target also depends on its internal dependencies targets */

//this needs swap support
 for(y=0; y < cd->n_dependencies; y++)
 {
  x = 0;
  while(x < cd->n_components)
  {
   if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
   {
    if((n_names_d = 
        render_project_component_output_name(ctx, cd->host, cd->project_components[x],
                                             2, &names_d, NULL)) < 2)
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
     return 1;
    }

    fprintf(output, "%s ", names_d[1]);

    free_string_array(names_d, n_names_d);

    break;
   }
   x++;
  }
 }

 /* target action is to link the above objects, etc */
 fprintf(output, "\n\t%s", tc->cc);

 if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                         cd->project_component_output_name), ".dylib") == 0)
 {
  fprintf(output, " -dynamiclib ");
 } else if (strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0) {
  fprintf(output, " -shared ");
 }

 fprintf(output, " $(%s-OBJECTS) %s",
         output_file_names[0], tc->cc_output_flag);

 /* figure out what the name of the output file should be,
    different for libraries and binaries */
 yes = 1;
 if(n_output_file_nmes > 3)
 {
  if(output_file_names[3][0] != 0)
  {
   yes = 0;
   fprintf(output, " %s", output_file_names[3]);
  }
 }

 if(yes == 1)
  fprintf(output, " %s", output_file_names[0]);

 /* add shared libary switches */
 if(strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0)
 {
  if(contains_string(tc->cc, -1, "mingw", -1)) 
  {
   if(n_output_file_nmes < 3)
   {
    fprintf(stderr, "BCA: there should have been an import name here\n");
    return 1;
   }
   fprintf(output, " -Wl,--out-implib,%s", output_file_names[2]);
  } else {
   if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type, 
                                           cd->project_component_output_name), ".dll") == 0)
   {
    /* cygwin */
    if(n_output_file_nmes < 3)
    {
     fprintf(stderr, "BCA: there should have been an import name here\n");
     return 1;
    }
    fprintf(output, " -Wl,--out-implib,%s",
            output_file_names[2]);
   }
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".dylib") == 0)
  {
   fprintf(output, " -compatibility_version %s.%s -install_name %s", 
           cd->major, cd->minor, output_file_names[0]);
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".so") == 0)
  {
   fprintf(output, " -Wl,-soname,%s", 
           output_file_names[0]);
  }

 } else {

  fprintf(output, " ");
 }

 /* both libraries and binaries may have LDFLAGS */
 if(tc->ldflags != NULL)
  fprintf(output, " %s", tc->ldflags);

 /* both libaries and binaries may have pkg-config dependencies */
 if(cd->n_dependencies > 0)
 {
  fprintf(output, " `");

  if(tc->pkg_config_path != NULL)
   fprintf(output, "PKG_CONFIG_PATH=%s ", tc->pkg_config_path);

  if(tc->pkg_config_libdir != NULL)
   fprintf(output, "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

  fprintf(output, "%s --libs ", tc->pkg_config);

  for(y=0; y < cd->n_dependencies; y++)
  {
   yes = 0;
   x = 0;
   while(x < cd->n_components)
   {
    if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
    {
     resolve_component_version(ctx, ctx->project_configuration_contents, 
                               ctx->project_configuration_length, &cd_d,
                               "SHAREDLIBRARY", cd->project_components[x]);

     fprintf(output, "%s-%s ", cd->project_output_names[x], cd_d.major);
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
 if(strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0)
 {
  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".dll") != 0)
  {
   fprintf(output, "\tcd %s; ln -sf %s %s\n", 
           tc->build_prefix, output_file_names[3], output_file_names[0]);
  }
 }
 fprintf(output, "\n");

 /* lastly the .pc file if needed */
 if(strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0)
 {
  if(generate_host_component_pkg_config_file(ctx, cd, tc,
                                             output_file_names,
                                             n_output_file_nmes,
                                             output))
  {
   fprintf(stderr, 
           "BCA: generate_host_component_pkg_config_file(%s.%s.%s) failed\n", 
           cd->project_component_type, cd->host, cd->project_component);
   return 1;
  }
 }

 return 0;
}

int generate_gmake_host_component_file_rules(struct bca_context *ctx, FILE *output, 
                                             struct component_details *cd)
{
 char **names, **output_names;
 int handled, n_names, n_output_names;
 struct host_configuration *tc;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_gmake_host_component_file_rules()\n");

 if((tc = resolve_host_configuration(ctx, cd)) == NULL)
 {
  fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
  return 1;
 }

 if(derive_file_dependencies_from_inputs(ctx, tc, cd))
 {
  fprintf(stderr, "BCA: derive_file_dependencies_from_inputs(%s.%s) failed\n",
          cd->host, cd->project_component);
  return 1;
 }

 if((n_output_names = 
     render_project_component_output_name(ctx, cd->host, cd->project_component,
                                          1, &output_names, NULL)) < 0)
 {
  fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
  return 1;
 }

 if((n_names = 
     render_project_component_output_name(ctx, cd->host, cd->project_component,
                                          2, &names, NULL)) < 0)
 {
  fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
  return 1;
 }

 if(generate_host_component_target_dependencies(ctx, cd, names[0], output))
 {
  fprintf(stderr, 
          "BCA: generate_gmake_host_component_concatentate_custom(%s.%s) failed\n", 
          cd->host, cd->project_component);
  return 1;
 }

 handled = 0;

 if(strcmp(cd->project_component_type, "CAT") == 0)
 {
  if(generate_gmake_host_component_concatenate(ctx, cd, names[0], output))
  {
   fprintf(stderr, 
           "BCA: generate_gmake_host_component_concatentate_custom(%s.%s) failed\n", 
           cd->host, cd->project_component);
   return 1;
  }
 
  handled = 1;
 }

 if(strcmp(cd->project_component_type, "MACROEXPAND") == 0)
 {
  if(generate_gmake_host_component_macroexpand(ctx, cd, names[0], output))
  {
   fprintf(stderr, 
           "BCA: generate_gmake_host_component_macroexpand(%s.%s) failed\n", 
           cd->host, cd->project_component);
   return 1;
  }

  handled = 1;
 }

 if(strcmp(cd->project_component_type, "CUSTOM") == 0)
 {
  if(generate_gmake_host_component_custom(ctx, cd, tc, names[0], output))
  {
   fprintf(stderr, 
           "BCA: generate_gmake_host_component_custom(%s.%s) failed\n", 
           cd->host, cd->project_component);

   return 1;
  }

  handled = 1;
 }

 if(strcmp(cd->project_component_type, "PYTHONMODULE") == 0)
 {
  if(generate_gmake_host_component_pythonmodule(ctx, cd, tc, names[0], output))
  {
   fprintf(stderr, 
           "BCA: generate_gmake_host_component_pythonmodule(%s.%s) failed\n", 
           cd->host, cd->project_component);

   return 1;
  }

  handled = 1;
 }

 if( (strcmp(cd->project_component_type, "BINARY") == 0) ||
     (strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0) )
 {
  if(generate_gmake_host_component_bins_and_libs(ctx, cd, tc, names, n_names, output))
  {
   fprintf(stderr, 
           "BCA: generate_gmake_host_component_bin_and_libs(%s.%s.%s) failed\n", 
           cd->project_component_type, cd->host, cd->project_component);
   return 1;
  }

  handled = 1;
 }

 fprintf(output, "\n");

 free_string_array(output_names, n_output_names);
 free_string_array(names, n_names);
 free_host_configuration(ctx, tc);

 if(handled == 0)
 {
  fprintf(stderr, 
          "BCA: I don't know what to do with component type %s. "
          "I should not have made it here.\n ",
          cd->project_component_type);
              
  return 1;
 }

 return 0;
}

int fresh_config_depends_check(struct bca_context *ctx, struct component_details *cd)
{
 char **list = NULL;
 int n_elements = 0, x, i, clear;

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
            cd->project_component, list[i]);
   }

   for(x = 0; x < ctx->n_disables; x++)
   {
    if(strcmp(list[i], ctx->disabled_components[x]) == 0)
    {
     fprintf(stderr, 
             "BCA: can not generate makefile because component \"%s\" has an internal "
             "dependency on component \"%s\", which as been disabled.\n",
             cd->project_component, list[i]);

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
            cd->project_component, list[i]);
   }
  }
  free_string_array(list, n_elements);
  list = NULL;
  n_elements = 0;
 }

 return 0;
}

int generate_gmakefile_mode(struct bca_context *ctx)
{
 char **hosts, *value, **file_deps, **include_dirs, **lib_headers;
 int n_hosts, host_i, component_i, i, handled, n_file_deps, n_include_dirs, n_lib_headers;
 FILE *output;
 struct component_details cd;

 memset(&cd, 0, sizeof(struct component_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_gmakefile_mode()\n");

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
  for(host_i=0; host_i<n_hosts; host_i++)
  {
   printf("%s ", hosts[host_i]);
  }
  printf("\n");
 }

 if((ctx->project_configuration_contents = 
      read_file("./buildconfiguration/projectconfiguration", 
                &(ctx->project_configuration_length), 0)) == NULL)
 {
  return 1;
 }

 resolve_project_name(ctx);
 check_project_component_types(ctx);

 if((output = fopen("Makefile.bca", "w")) == NULL)
 {
  perror("BCA: fopen(\"Makefile.bca\")");
  return 1;
 }

 fprintf(output, 
         "# This Makefile for GNU Make was generated by Build Configuration Adjust\n"
         "# See http://bca.stoverenterprises.com for more information\n\n");

 fprintf(output, "# by default build everything\n");
 fprintf(output, "all : ");
 for(host_i=0; host_i<n_hosts; host_i++)
 {
  fprintf(output, "%s ", hosts[host_i]);
 }
 fprintf(output, "\n\n");

 if(gmake_help(ctx, output, hosts, n_hosts, &cd))
 {
  fclose(output);
  return 1;
 }

 if(gmake_clean_rules(ctx, output, hosts, n_hosts, &cd))
 {
  fclose(output);
  return 1;
 }

 if(generate_gmake_install_rules(ctx, output, hosts, n_hosts, &cd)) 
 {
  fclose(output);
  return 1;
 }

 if(generate_gmake_host_components(ctx, output, hosts, n_hosts, &cd))
 {
  fclose(output);
  return 1;
 }

 for(host_i=0; host_i<n_hosts; host_i++)
 {
  cd.host = hosts[host_i];

  if(engage_build_configuration_disables_for_host(ctx, hosts[host_i]))
  {
   fclose(output);
   return 1;
  }

  if(engage_build_configuration_swaps_for_host(ctx, hosts[host_i]))
  {
   fclose(output);
   return 1;
  }

  if(list_project_components(ctx, &cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   fclose(output);
   return 1;
  }

  if(ctx->verbose)
  {
   printf("BCA: Found the following project components (%d): ", cd.n_components);
   for(component_i=0; component_i<cd.n_components; component_i++)
   {
    printf("%s ", cd.project_components[component_i]);
   }
   printf("\n");
  }

  for(component_i=0; component_i<cd.n_components; component_i++)
  {

   /* component type sanity check ------------------- */
   handled = 0;

   if(strcmp(cd.project_component_types[component_i], "BINARY") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "BUILDBINARY") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "SHAREDLIBRARY") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "STATICLIBRARY") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "CAT") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "MACROEXPAND") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "PYTHONMODULE") == 0)
    handled = 1;

   if(strcmp(cd.project_component_types[component_i], "CUSTOM") == 0)
    handled = 1;

   if(handled == 0)
   {
    fprintf(stderr, 
            "BCA: I don't know how to generate gmakefile rules for type type of \"%s\"\n", 
            cd.project_component_types[component_i]);
    fclose(output);
    return 1;
   }
  
   /* FILE_DEPENDS --------------------- */
   if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                          ctx->project_configuration_length, 
                          cd.project_component_types[component_i], 
                          cd.project_components[component_i], "FILE_DEPENDS")) == NULL)
   {
    if(ctx->verbose)
     printf("BCA: No file level dependencies found for %s.%s\n", 
            cd.project_component_types[component_i], cd.project_components[component_i]);

    file_deps = NULL;
    n_file_deps = 0;
   } else {

    if(split_strings(ctx, value, -1, &n_file_deps, &file_deps))
    {
     fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
     fclose(output);
     return 1;
    }

    if(ctx->verbose)
    {
     printf("BCA: Found the following file level dependencies for component \"%s\" (%d): ", 
            cd.project_components[component_i], n_file_deps);

     for(i=0; i<n_file_deps; i++)
     {
      printf("%s ", file_deps[i]);
     }
     printf("\n");
    }

   }

   /* LIB_HEADERS --------------------------- */
   if(strcmp(cd.project_component_types[component_i], "SHAREDLIBRARY") == 0)
   {
    if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                           ctx->project_configuration_length, 
                           cd.project_component_types[component_i], 
                           cd.project_components[component_i], "LIB_HEADERS")) == NULL)
    {
     if(ctx->verbose)
      printf("BCA: No lib headers found for %s.%s\n", 
             cd.project_component_types[component_i], cd.project_components[component_i]);

     lib_headers = NULL;
     n_lib_headers = 0;
    } else {

     if(split_strings(ctx, value, -1, &n_lib_headers, &lib_headers))
     {
      fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
      fclose(output);
      return 1;
     }

     if(ctx->verbose)
     {
      printf("BCA: Found the following library headers for component \"%s\" (%d): ", 
             cd.project_components[component_i], n_lib_headers);
      for(i=0; i<n_lib_headers; i++)
      {
       printf("%s ", lib_headers[i]);
      }
      printf("\n");
     }
    }
   } else {
    lib_headers = NULL;
    n_lib_headers = 0;
   }

   /* INCLUDE_DIRS ---------------------- */ 
   include_dirs = NULL;
   n_include_dirs = 0;
   if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                          ctx->project_configuration_length, 
                          cd.project_component_types[component_i], 
                          cd.project_components[component_i], "INCLUDE_DIRS")) == NULL)
   {
    if(ctx->verbose)
     printf("BCA: No project level include directories for %s.%s\n", 
            cd.project_component_types[component_i], cd.project_components[component_i]);
   } else {

    if(split_strings(ctx, value, -1, &n_include_dirs, &include_dirs))
    {
     fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
     fclose(output);
     return 1;
    }

    if(ctx->verbose)
    {
     printf("BCA: Found the following project level include directories for component "
            "\"%s\" (%d): ", cd.project_components[component_i], n_include_dirs);
     for(i=0; i<n_include_dirs; i++)
     {
      printf("%s ", include_dirs[i]);
     }
     printf("\n");
    }
   }


   /* WITHOUTS --------------------------------- */
   if((value = lookup_key(ctx, ctx->build_configuration_contents,
                          ctx->build_configuration_length, 
                          hosts[host_i], 
                          cd.project_components[component_i], 
                          "WITHOUTS")) == NULL)
   {
    if((value = lookup_key(ctx, ctx->build_configuration_contents,
                           ctx->build_configuration_length, 
                           hosts[host_i], "ALL", "WITHOUTS")) == NULL)
    {
     if(ctx->verbose)
      printf("BCA: Could not find %s.%s.WITHOUTS\n", 
             cd.project_component_types[component_i], cd.project_components[component_i]);
    }
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

   if(ctx->verbose)
   {
    printf("BCA: Found the following withouts for component \"%s\" (%d): ", 
           cd.project_components[component_i], cd.n_withouts);
    for(i=0; i<cd.n_withouts; i++)
    {
     printf("%s ", cd.withouts[i]);
    }
    printf("\n");
   }

   /* dependencies -------------------------------------- */
   cd.project_component = cd.project_components[component_i];
   cd.project_component_type = cd.project_component_types[component_i];
   cd.project_component_output_name = cd.project_output_names[component_i];
   cd.host = hosts[host_i];

   /* when the files within a component have dependencies */
   if(resolve_component_file_dependencies(ctx, &cd, component_i))
   {
    return 1;
   }

   /* when components need the output of another component as a dependency */
   if(resolve_component_input_dependencies(ctx, &cd, component_i))
   {
    return 1;
   }
  
   /* when components need other components, internal or external */
   if(resolve_component_dependencies(ctx, &cd))
   {
    return 1;
   }

   /* save pain */
   if(fresh_config_depends_check(ctx, &cd))
    return 1;

   /* get ready to process this component ----------- */
   cd.file_deps = file_deps;
   cd.n_file_deps = n_file_deps;
   cd.lib_headers = lib_headers;
   cd.n_lib_headers = n_lib_headers;
   cd.include_dirs = include_dirs;
   cd.n_include_dirs = n_include_dirs;

   if(resolve_component_version(ctx, ctx->project_configuration_contents, 
                                ctx->project_configuration_length, 
                                &cd, cd.project_component_types[component_i], 
                                cd.project_components[component_i]))
   {
    return 1;
   }

   /* finally do something  ----------- */
   if(generate_gmake_host_component_file_rules(ctx, output, &cd))
   {
    fclose(output);
    return 1;
   }

   /* now clean up for the next loop ----------- */
   if(cd.n_dependencies > 0)
   {
    free_string_array(cd.dependencies, cd.n_dependencies);
    cd.dependencies = NULL;
    cd.n_dependencies = 0;
   }

   if(cd.n_withouts > 0)
   {
    free_string_array(cd.withouts, cd.n_withouts);
    cd.withouts = NULL;
    cd.n_withouts = 0;
   }

   if(cd.n_include_dirs > 0)
   {
    free_string_array(cd.include_dirs, cd.n_include_dirs);
    cd.include_dirs = NULL;
    cd.n_include_dirs = 0;
   }

   if(cd.n_file_names > 0)
   {
    free_string_array(cd.file_names, cd.n_file_names);
    free_string_array(cd.file_base_names, cd.n_file_names);
    free_string_array(cd.file_extensions, cd.n_file_names);
    cd.file_names = NULL;
    cd.file_base_names = NULL;
    cd.file_extensions = NULL;
    cd.n_file_names = 0;
   }

   if(cd.n_inputs > 0)
   {
    free_string_array(cd.inputs, cd.n_inputs);
    cd.inputs = NULL;
    cd.n_inputs = 0;
   }

   if(cd.n_file_deps)
   {
    free_string_array(cd.file_deps, cd.n_file_deps);
    cd.file_deps = NULL;
    cd.n_file_deps = 0;
   }

   if(cd.n_lib_headers)
   {
    free_string_array(cd.lib_headers, cd.n_lib_headers);
    cd.lib_headers = NULL;
    cd.n_lib_headers = 0;
   }

   free(cd.minor);
   cd.minor = NULL;

   free(cd.major);   
   cd.major = NULL;
  }

  free_string_array(cd.project_components, cd.n_components);
  free_string_array(cd.project_component_types, cd.n_components);
  cd.project_components = NULL;
  cd.project_component_types = NULL;
  cd.n_components = 0;

  /* add extra newline between hosts */
  fprintf(output, "\n");
 }


 fclose(output);
 return 0;
}


int generate_gmake_install_rules(struct bca_context *ctx, FILE *output, 
                                 char **hosts, int n_build_hosts,
                                 struct component_details *cd)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_gmake_install_rules()\n");

 int x, y, n_build_names, n_install_names, n_output_names, i, 
     length, index, yes;
 struct host_configuration *tc;
 char temp[512], **extensions = NULL, **build_names, 
      **install_names, *value, **output_names;
 struct component_details cd_d;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: gmake_install_rules()\n");

 return 0;

 memset(&cd_d, 0, sizeof(struct component_details));

 fprintf(output, "install : \n");

 for(x=0; x<n_build_hosts; x++)
 {

  if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
  {
   fprintf(stderr, "BCA: engage_build_configuration_disables_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(list_project_components(ctx, cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   return 1;
  }

  for(y=0; y < cd->n_components; y++)
  {
   cd->project_component = cd->project_components[y];
   cd->project_component_type = cd->project_component_types[y];
   cd->host = hosts[x];

   if((tc = resolve_host_configuration(ctx, cd)) == NULL)
   {
    fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
    return 1;
   }

   if(resolve_component_dependencies(ctx, cd))
   {
    return 1;
   }

   if((n_output_names = 
       render_project_component_output_name(ctx, hosts[x], 
                                            cd->project_components[y], 1, 
                                            &output_names, NULL)) < 0)
   {
    fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
    return 1;
   }

   if((n_build_names = 
       render_project_component_output_name(ctx, hosts[x], 
                                            cd->project_components[y], 2, 
                                            &build_names, NULL)) < 0)
   {
    fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
    return 1;
   }

   n_install_names = render_project_component_output_name(ctx, hosts[x], 
                                                          cd->project_components[y], 3, 
                                                          &install_names, &extensions);
   for(i=0; i<n_install_names; i++)
   {
    yes = 1;
    
    if(install_names[i][0] == 0)
     yes = 0;

    if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0) 
    {
     if(i == 1)
      yes = 0;

     if(strcmp(extensions[0], ".so") == 0)
      if(i == 0)
       yes = 0;
    }

    if(yes)
    {
     if(strcmp(cd->project_component_types[y], "BINARY") == 0) 
      fprintf(output, "\tinstall --group root --owner root --mode=rwxr-xr-x %s %s\n", 
              build_names[i], install_names[i]);

     if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0) 
      fprintf(output, "\tinstall --group root --owner root --mode=rwxr-xr-x %s %s\n", 
              build_names[i], install_names[i]);
    }
   }

   if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0)
   {
    if(strcmp(extensions[0], ".so") == 0)
    {
     fprintf(output, "\tcd %s; ln -s %s %s\n", 
             tc->install_lib_dir, install_names[3], install_names[0]);
    }

    if(resolve_component_version(ctx, ctx->project_configuration_contents,
                                 ctx->project_configuration_length,
                                 cd, cd->project_component_types[y],
                                 cd->project_components[y]))
    {
     fprintf(stderr, "BCA: resolve_component_version() failed\n");
     return 1;
    }

    fprintf(output, "\trm -f %s\n", install_names[1]);

    /* derive prefix */
    length = strlen(tc->install_lib_dir);
    index = length;
    while(index > -1)
    {
     if(tc->install_lib_dir[index] == '/')
     {
      index++;
      break;
     }
     index--;
    }
    memcpy(temp, tc->install_lib_dir, index);
    temp[index] = 0;

    fprintf(output, "\techo \"prefix=%s\" >> %s\n", temp, install_names[1]);
    fprintf(output, "\techo 'exec_prefix=$${prefix}' >> %s\n", install_names[1]);
    fprintf(output, "\techo 'libdir=$${exec_prefix}' >> %s\n", install_names[1]);
    fprintf(output, "\techo 'includedir=%s/%s-%s' >> %s\n", 
            tc->install_include_dir, cd->project_output_names[y], cd->major, install_names[1]);
    fprintf(output, "\techo 'Name: %s' >> %s\n", cd->project_component, install_names[1]);
    fprintf(output, "\techo 'Description: %s' >> %s\n", cd->project_component, install_names[1]);
    fprintf(output, "\techo 'Version: %s.%s' >> %s\n", cd->major, cd->minor, install_names[1]);
    fprintf(output, "\techo 'Requires: ");
    for(i=0; i < cd->n_dependencies; i++)
    {
     yes = 0;
     index = 0;
     while(index < cd->n_components)
     {
      if(strcmp(cd->dependencies[i], cd->project_components[index]) == 0)
      {
       resolve_component_version(ctx, ctx->project_configuration_contents, 
                                 ctx->project_configuration_length, &cd_d,
                                 "SHAREDLIBRARY", cd->project_components[index]);

       fprintf(output, "%s-%s", cd->project_output_names[index], cd_d.major);
       yes = 1;
       break;
      }
      index++;
     }

     if(yes == 0)
      fprintf(output, "%s ", cd->dependencies[i]);
    }

    fprintf(output, "' >> %s\n", install_names[1]);

    if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                            cd->project_component_output_name), ".so") == 0)
    {
     fprintf(output, "\techo 'Libs: $${libdir}/%s", output_names[0]);
    }

    if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                            cd->project_component_output_name), ".dylib") == 0)
    {
     fprintf(output, "\techo 'Libs: $${libdir}/%s", output_names[0]);
    }

    if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                            cd->project_component_output_name), ".dll") == 0)
    {
     fprintf(output, "\techo 'Libs: -L$${libdir} -l%s-%s.%s", 
             cd->project_output_names[y], cd->major, cd->minor);
    }

    if(tc->ldflags != NULL)
     fprintf(output, " %s", tc->ldflags);

    fprintf(output, "' >> %s\n", install_names[1]);

    fprintf(output, "\techo 'Cflags: ");

    if(cd->n_include_dirs > 0)
     fprintf(output, " -I$${includedir} ");

    if(gmake_host_component_file_rule_cflags(ctx, output, cd, tc))
     return 1;

    fprintf(output, "' >> %s\n", install_names[1]);
    fprintf(output, "\tchown root:root %s\n", install_names[1]);
    fprintf(output, "\tchmod 755 %s\n", install_names[1]);

    if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                           ctx->project_configuration_length, 
                           cd->project_component_types[y], 
                           cd->project_components[y], "LIB_HEADERS")) != NULL)
    {

     if(split_strings(ctx, value, -1, &(cd->n_lib_headers), &(cd->lib_headers)))
     {
      fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
      fclose(output);
      return 1;
     }

     for(i=0; i<cd->n_lib_headers; i++)
     {
      length = strlen(cd->lib_headers[i]);
      index = length;
      while(index > -1)
      {
       if(cd->lib_headers[i][index] == '/')
       {
        index++;
        break;
       }
       index--;
      }
      fprintf(output, "\tinstall --group root --owner root %s %s/%s-%s/%s\n", 
              cd->lib_headers[i], tc->install_include_dir, cd->project_output_names[y], 
              cd->major, cd->lib_headers[i] + index);
     }
    }

    free_string_array(cd->lib_headers, cd->n_lib_headers);
    cd->n_lib_headers = 0;
    cd->lib_headers = NULL;
   }

   if(strcmp(cd->project_component_types[y], "PYTHONMODULE") == 0)
   {
    snprintf(temp, 512, "%s/%s-setup.py", 
             tc->build_prefix, cd->project_output_names[y]);

    fprintf(output, "\trm -f %s\n", temp);
    fprintf(output, "\techo \"from distutils.core import setup\" >> %s\n", temp);

    fprintf(output, "#\t%s %s install\n", tc->python, temp);
   }

   if(cd->n_dependencies > 0)
   {
    free_string_array(cd->dependencies, cd->n_dependencies);
    cd->dependencies = NULL;
    cd->n_dependencies = 0;
   }

   free_string_array(output_names, n_output_names);
   free_string_array(build_names, n_build_names);
   free_string_array(install_names, n_install_names);
   free_string_array(extensions, n_install_names);
   free_host_configuration(ctx, tc);
   extensions = NULL;
   build_names = NULL;
   install_names = NULL;
   n_build_names = n_install_names = 0;
  }

  fprintf(output, "\n\n");
 }



 fprintf(output, "uninstall : \n");
 for(x=0; x<n_build_hosts; x++)
 {
  if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
  {
   fprintf(stderr, "BCA: engage_build_configuration_disables_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(list_project_components(ctx, cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   return 1;
  }

  for(y=0; y < cd->n_components; y++)
  {
   cd->project_component = cd->project_components[y];
   cd->project_component_type = cd->project_component_types[y];
   cd->host = hosts[x];

   if(strcmp(cd->project_component_types[y], "BUILDBINARY") == 0)
    continue;

   if((tc = resolve_host_configuration(ctx, cd)) == NULL)
   {
    fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
     return 1;
   }

   if((n_build_names = 
       render_project_component_output_name(ctx, hosts[x], 
                                            cd->project_components[y], 2, 
                                            &build_names, NULL)) < 0)
   {
    fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
    return 1;
   }

   if((n_install_names = 
       render_project_component_output_name(ctx, hosts[x], 
                                            cd->project_components[y], 3, 
                                            &install_names, NULL)) != n_build_names)
   {
    fprintf(stderr, "BCA: render_project_component_ouput_name() failed. "
            "install names != build names\n"); 
    return 1;
   }

   for(i=0; i<n_build_names; i++)
   {
    yes = 1;
    
    if(install_names[i][0] == 0)
     yes = 0;

    if(yes)
     fprintf(output, "\trm -f %s\n", install_names[i]);
   }

   if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0)
   {
    if(resolve_component_version(ctx, ctx->project_configuration_contents,
                                 ctx->project_configuration_length,
                                 cd, cd->project_component_types[y],
                                 cd->project_components[y]))
    {
     fprintf(stderr, "BCA: resolve_component_version() failed\n");
     return 1;
    }

    if((value = lookup_key(ctx, ctx->project_configuration_contents, 
                           ctx->project_configuration_length, 
                           cd->project_component_types[y], 
                           cd->project_components[y], "LIB_HEADERS")) != NULL)
    {

     if(split_strings(ctx, value, -1, &(cd->n_lib_headers), &(cd->lib_headers)))
     {
      fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
      fclose(output);
      return 1;
     }

     for(i=0; i<cd->n_lib_headers; i++)
     {
      length = strlen(cd->lib_headers[i]);
      index = length;
      while(index > -1)
      {
       if(cd->lib_headers[i][index] == '/')
       {
        index++;
        break;
       }
       index--;
      }
      fprintf(output, "\trm -f %s\\%s\n",  
              tc->install_include_dir, cd->lib_headers[i] + index);
     }
    }

    free_string_array(cd->lib_headers, cd->n_lib_headers);
    cd->n_lib_headers = 0;
    cd->lib_headers = NULL;
   }
   free_string_array(build_names, n_build_names);
   free_string_array(install_names, n_install_names);
   free_host_configuration(ctx, tc);
  }

  fprintf(output, "\n\n");
 }

 return 0;
}



