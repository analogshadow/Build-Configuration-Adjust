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

int check_target_platform(struct bca_context *ctx, char *host_root, char **platform)
{
 FILE *test;
 char temp[1024];

 if(host_root == NULL)
 {
  if(strcmp(ctx->principle, "NATIVE") != 0)
  {
   fprintf(stderr, 
           "BCA: check_target_platform() - warning: HOST_ROOT is NULL, and this looks like cross compile.\n");
   return 0;
  }
  host_root = "";
 }

 snprintf(temp, 1024, "%s/bin/cygwin1.dll", host_root);
 test = fopen(temp, "rb");
 if(test != NULL)
 {
  fprintf(stderr, "BCA: I think host platform is cygwin1x.\n");

  *platform = "cygwin";
  fclose(test);
  return 0;
 }

 snprintf(temp, 1024, "%s/usr/lib/libSystem.B.dylib", host_root);
 test = fopen(temp, "rb");
 if(test != NULL)
 {
  fprintf(stderr, "BCA: I think host platform is Mac OSX.\n");

  *platform = "osx";
  fclose(test);
  return 0;
 }

 return 0;
}

int is_c_compiler_needed(struct bca_context *ctx, 
                         struct component_details *cd)
{
 int i, j, skip, pre_loaded, yes = 0;
 char *extension;

 if(ctx->verbose)
 {
  printf("BCA: Looking for a C source files to see C compiler is needed.\n"); 
  fflush(stdout);
 }

 for(i=0; i < cd->n_components; i++)
 {
  skip = 0;

  if(cd->project_component != NULL)
   if(strcmp(cd->project_component, cd->project_components[i]) != 0)
    skip = 1;

  if(strcmp(cd->project_component_types[i], "CAT") == 0)
   skip = 1;

  if(skip == 0)
  {
   if(cd->n_file_names > 0)
   {
    pre_loaded = 1;
   } else {
    if(resolve_component_file_dependencies(ctx, cd, i))
    {
     return -1;
    }
    pre_loaded = 0;
   }

   for(j=0; j < cd->n_file_names; j++)
   {
    if(strcmp(cd->file_extensions[j], "c") == 0)
    yes = 1;     
    
   }

   if(pre_loaded == 0)
   {
    if(cd->n_file_names > 0)
    {
     free_string_array(cd->file_names, cd->n_file_names);
     free_string_array(cd->file_base_names, cd->n_file_names);
     free_string_array(cd->file_extensions, cd->n_file_names);
    }
    cd->n_file_names = 0;
   }

   if(yes)
    return 1;
  }
 }

 for(i=0; i < cd->n_components; i++)
 {
  skip = 0;

  if(cd->project_component != NULL)
   if(strcmp(cd->project_component, cd->project_components[i]) != 0)
    skip = 1;

  if(skip == 0)
  {
   if(cd->n_inputs > 0)
   {
    pre_loaded = 1;
   } else {
    if(resolve_component_input_dependencies(ctx, cd, i))
    {
     return -1;
    }
    pre_loaded = 0;
   }

   for(j=0; j < cd->n_inputs; j++)
   {

    if(path_extract(cd->inputs[j], NULL, &extension))
    {
     return 1;
    }
   
    if(strcmp(extension, "c") == 0)
     yes = 1;

    free(extension);
   }

   if(pre_loaded == 0)
   {
    if(cd->n_inputs > 0)
    {
     free_string_array(cd->inputs, cd->n_inputs);
    }
    cd->n_inputs = 0;
   }

   if(yes)
    return 1;
  }
 }

 return 0;
}

int is_pkg_config_needed(struct bca_context *ctx, 
                         struct component_details *cd)
{
 int i, n_elements, check_all = 0;
 char **list;

 if(ctx->verbose)
 {
  printf("BCA: Looking for dependences to see if pkg-config is needed.\n");
  fflush(stdout);
 }

 if(cd->project_component == NULL)
  check_all = 1;

 for(i=0; i < cd->n_components; i++)
 {
  if(check_all)
  {
   cd->project_component = cd->project_components[i];
   cd->project_component_type = cd->project_component_types[i];
  }

  if(list_component_internal_dependencies(ctx, cd, &list, &n_elements))
   return -1;

  if(n_elements > 0)
  {
   free_string_array(list, n_elements);
   return 1;
  }

  if(list_component_external_dependencies(ctx, cd, &list, &n_elements))
   return -1;

  if(n_elements > 0)
  {
   free_string_array(list, n_elements);
   return 1;
  }

  if(list_component_opt_external_dependencies(ctx, cd, &list, &n_elements))
   return -1;

  if(n_elements > 0)
  {
   free_string_array(list, n_elements);
   return 1;
  }

  if(check_all == 0)
   return 0;
 }

 return 0;
}

int is_fpic_needed(struct bca_context *ctx,
                   struct host_configuration *tc)
{
 int test = 0, needed = 1, length;
 FILE *f;
 char command[1024], *results;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: is_fpic_needed()\n");

 if(contains_string(tc->cc, -1, "gcc", 3) > -1)
 {
  test = 1;
  needed = 1;
 }

 if(test)
 {
  if((f = fopen("./configuretestfile.c", "w")) == NULL)
  {
   fprintf(stderr, "BCA: fopen(./configuretestfile.c) failed\n");
   return -1;
  }

  fprintf(f, "int function(void) { return 0; }\n\n");
  fclose(f);

  snprintf(command, 1024, 
           "%s %s -fpic configuretestfile.c %s configuretestfile.o 1> configuretestoutput 2> configuretestoutput", 
           tc->cc, tc->cc_compile_bin_obj_flag, tc->cc_output_flag);

  if(ctx->verbose)
   printf("BCA: about to run \"%s\"\n", command); 

  system(command);

  if((results = read_file("configuretestoutput", &length, 0)) == NULL)
  {
   fprintf(stderr, "BCA: read_file(\"configuretestoutput\") failed\n");
   return -1;
  }

  if(contains_string(results, length, "-fpic ignored", -1))
   needed = 0;

  free(results);
  unlink("configuretestoutput");
  unlink("configuretestfile.c");
  unlink("configuretestfile.o");
 }

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: is_fpic_needed() = %d\n", needed);

 return needed;
}

int test_package_exist(struct bca_context *ctx, 
                       struct component_details *cd, 
                       struct host_configuration *tc,
                       char *package, int optional)
{
 char command[1024];
 int code, length;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: test_package_exist(%s, %d)\n", package, optional);

 length = 0;

 if(tc->pkg_config_path != NULL)
  length += snprintf(command + length, 1024 - length, 
                    "PKG_CONFIG_PATH=%s ", tc->pkg_config_path);

 if(tc->pkg_config_libdir != NULL)
  length += snprintf(command + length, 1024 - length, 
                    "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

 snprintf(command + length, 1024 - length, "%s %s", tc->pkg_config, package);
 

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: about to try system(\"%s\")...\n", command);

 code = system(command);

 if( (code = system(command)) == -1)
 {
  perror("BCA: system()");
  return 1;
 }

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: system() returned %d\n", WEXITSTATUS(code));

 if(WEXITSTATUS(code) == 0)
  return 0;

 if(ctx->verbose > 0)
 {
  fprintf(stderr, 
          "BCA: Could not find package '%s' for host '%s' using\n"
          "BCA: command line '%s'.\n"
          "BCA: Tweak with PKG_CONFIG_PATH, PKG_CONFIG_LIBDIR, and PKGCONFIG envrionment variables.\n",
          package, ctx->principle, command);

  fprintf(stderr,
          "BCA: It could be that %s, or its development files are not installed on this system.\n",
          package);

  fprintf(stderr,
          "BCA: It is also possible that %s itself does not use package config, but something some\n"
          "BCA: distributions may include for convience and others do not.\n", package);

  fprintf(stderr, 
          "BCA: Similarly, the author(s) of \"%s\" might intend for a package config wrapper to be\n"
          "BCA: crafted for this purpose by those performing a build. Read the project's install file as\n"
          "BCA: one may already be included with the source files for your platform.\n",
           ctx->project_name);

  if(optional == 1)
   fprintf(stderr, 
           "BCA: Since this is only needed an optional package we will continue. You can disable this\n"
           "BCA: message with --without-%s .\n", package);

  if(optional == 2)
   fprintf(stderr, 
           "BCA: Since package \"%s\" is an optional dependency for some componets of this project\n"
           "BCA: --without-%s can be used to disable the dependency for such component(s). Although in this\n"
           "BCA: project it is still required for one or more other componets.\n", package, package);

 } else {
  if(optional == 1)
  {
   fprintf(stderr, 
           "BCA: Could not find optional package '%s' for host '%s'.\n", package, ctx->principle);
  } else {
   fprintf(stderr, 
           "BCA: Could not find package '%s' for host '%s'.\n", package, ctx->principle);
  }
 }


 return 1;
}

int test_runnable(struct bca_context *ctx, char *command)
{
 int code;
 char string[1024];

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: test_runnable()\n");

 snprintf(string, 1024, "%s > /dev/null 2> /dev/null", command);

 if(ctx->verbose > 1)
  printf("BCA: running \"%s\"...\n", string);

 if( (code = system(string)) == -1)
 {
  perror("BCA: system()");
  return 1;
 }
  
 if(WEXITSTATUS(code) == 127) /* does not exit code - system(3) */
 {
  fprintf(stderr, "BCA: not able to run \"%s\". Check your path. ", string);
  return 1;
 }
  
 return 0;
}

int *expand_int_array(int *ptr, int *allocated_size, int needed_size)
{
 int *array = NULL;

 if(needed_size < *allocated_size)
  return ptr;

 *allocated_size += 128;

 if((array = (int *) realloc(ptr, *allocated_size * sizeof(int))) == NULL)
 {
  fprintf(stderr, "BCA: realloc() failed in expand_int_array()\n");
  exit(1);
 }

 return array;
}

char temp[1024];
int temp_length;

int configure(struct bca_context *ctx)
{
 char *s, **test_package_list, **list, **depends = NULL, *value, **project_disables;
 char host_prefix[512], install_prefix[512];
 int i, j, code, n_test_packages, n_elements, n_project_disables,
     n_depends = 0, x, yes;
 int n_modify_records = 0;
 int *test_package_optional_flags, test_package_optional_flags_size = 0, handled;
 char **mod_principles = NULL, **mod_components = NULL, **mod_keys = NULL, **mod_values = NULL;
 struct component_details cd;
 struct host_configuration *tc;
 char *platform = "", *host_root;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: configure()\n");

 if(ctx->verbose == 0)
  fprintf(stdout, "BCA: configure() use -v to increase verbosity\n");

 memset(&cd, 0, sizeof(struct component_details));

 cd.host = ctx->principle;

 if((ctx->build_configuration_contents = 
     read_file("./buildconfiguration/buildconfiguration", 
               &(ctx->build_configuration_length), 1)) != NULL)
 {
  if((tc = resolve_host_configuration(ctx, &cd)) == NULL)
  {
   fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
   return 1;
  }
  /* all the below reasignments of non-NULL tc members are memory leaks */
 } else {
  if((tc = (struct host_configuration *)
           malloc(sizeof(struct host_configuration))) == NULL)
  {
   fprintf(stderr, "BCA: malloc() failed\n");
   return 1;
  }
  memset(tc, 0, sizeof(struct host_configuration));

  if((tc->build_tc = (struct host_configuration *)
                      malloc(sizeof(struct host_configuration))) == NULL)
  {
   fprintf(stderr, "BCA: malloc() failed\n");
   return 1;
  }
  memset(tc->build_tc, 0, sizeof(struct host_configuration));
 }
 
 if((ctx->project_configuration_contents = 
     read_file("./buildconfiguration/projectconfiguration", 
               &(ctx->project_configuration_length), 0)) == NULL)
 {
  fprintf(stderr, "BCA: can't open project configuration file\n");
  return 1;
 }

 if(resolve_project_name(ctx))
 {
  fprintf(stderr, "BCA: can't resolve project name.\n"
                  "BCA: Make sure project file has a NONE.NONE.PROJECT_NAME record\n");
  return 1;
 } 

 if(list_project_components(ctx, &cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed.\n");
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: found (%d) project components: ", cd.n_components);
  for(i=0; i < cd.n_components; i++)
  {
   printf("%s ", cd.project_components[i]);
  }
  printf("\n");
 }

 host_root = getenv("HOST_ROOT");
 if(ctx->verbose)
  if(host_root)
   fprintf(stderr, "BCA: HOST_ROOT set via environment variable\n");


 /* project defined disables processing */
 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                      ctx->project_configuration_length, 
                      "NONE", "NONE", "DISABLES")) != NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: project file has specified some components disable by default: %s\n", value);

  if(split_strings(ctx, value, -1, &n_project_disables, &project_disables))
  {
   fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
   return 1;
  }

  /* catch the user's error first */
  for(j=0; j<ctx->n_enables; j++)
  {
   yes = 1;
   for(i=0; i<n_project_disables; i++)
   {
    if(strcmp(project_disables[i], ctx->enabled_components[j]) == 0)
    {
      yes = 0;
      break;
    }
   }

   if(yes)
   {
    fprintf(stderr, 
            "BCA: I do not have a component disabled by default to enable with --enable-%s\n",
             ctx->enabled_components[j - 1]);
    return 1;
   }
  }
 
  for(i=0; i<n_project_disables; i++)
  {
   yes = 1;
   for(j=0; j<ctx->n_enables; j++)
   {
    if(strcmp(project_disables[i], ctx->enabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
   }

   if(yes)
   {
    if(add_to_string_array(&(ctx->disabled_components), ctx->n_disables, 
                           project_disables[i], -1, 1))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed. Are you using --disable on a component "
             "that is disable by default by the project?\n");
     return 1;
    }
    ctx->n_disables++;
   }
  }

  free(value);
 }

 code = is_c_compiler_needed(ctx, &cd);
 if(code == -1)
  return 1;

 if(code == 1)
 {

  /* BUILD_CC
   in autotools "build" is the system doing the compiling, "host" is the sytem which it will run, 
   and "target" is the system for which it might generate code for.

   in bca ALL.ALL.BUILD_CC is the same as autoconf's build_CC

   ALL.component.BUILD_CC can still be set manually if need be, as that resolution patter is still 
   followed.

   There might be ways to use cpp definitions availble here to know what compiler built BCA in
   stage 0, and just use that.

  */

  /* BUILDCC */
  if((s = getenv("BUILDCC")) == NULL)
  {
   if((s = tc->build_tc->cc) == NULL)
    if((s = getenv("CC")) == NULL)
     s = "gcc";
  }

  if(test_runnable(ctx, s))
  {
   fprintf(stderr, 
           "BCA: Specify alternative build C compiler with the BUILDCC environment variable.\n");
   return 1;
  }
  
  if(ctx->verbose)
   printf("BCA: Build C Compiler = %s\n", s);

  if(tc->build_tc->cc != s)
   tc->build_tc->cc = strdup(s);

  if(tc->build_tc->cc_output_flag == NULL)
   tc->build_tc->cc_output_flag = strdup("-o");

  if(tc->build_tc->cc_compile_bin_obj_flag == NULL)
   tc->build_tc->cc_compile_bin_obj_flag = strdup("-c");

  if(tc->build_tc->binary_suffix == NULL)
   tc->build_tc->binary_suffix = strdup("");

  if(tc->build_tc->obj_suffix == NULL)
   tc->build_tc->obj_suffix = strdup(".o");

  /* host prefix */
  if(ctx->host_prefix == NULL)
  {
   host_prefix[0] = 0;
  } else {
   snprintf(host_prefix, 512, "%s-", ctx->host_prefix);
  }

  /* target platform */
  if(check_target_platform(ctx, host_root, &platform))
  {
   fprintf(stderr, "BCA: check_target_platform() hard failed\n");
   return 1;
  }

  /* C compiler */
  if((s = getenv("CC")) != NULL)
  {
   /* here we want to overwrite regardless */
   snprintf(temp, 512, "%s%s", host_prefix, s); 
  } else {
   /* here we guess only if we have to */
   if(tc->cc != NULL)
   {
    snprintf(temp, 512, "%s", tc->cc); 
   } else {
    snprintf(temp, 512, "%sgcc", host_prefix); 
   }
  }

  if(test_runnable(ctx, temp))
  {
   fprintf(stderr, 
           "BCA: Specify alternative with C compiler with the CC environment variable.\n");
   return 1;
  }
  
  if(ctx->verbose)
   printf("BCA: Host C Compiler = %s\n", temp);

  tc->cc = strdup(temp);

  /* C compiler output flag */
  if(tc->cc_output_flag == NULL)
   tc->cc_output_flag = "-o";

  /* C compiler compile object flag */
  if(tc->cc_compile_bin_obj_flag == NULL)
   tc->cc_compile_bin_obj_flag = "-c";

  /* C compiler compile shared object flag */
  if(tc->cc_compile_shared_library_obj_flag == NULL)
  {
   if((code = is_fpic_needed(ctx, tc)) < 0)
   {
    fprintf(stderr, "BCA: is_fpic_needed() failed\n");
    return 1;
   }

   if(code == 1)
    tc->cc_compile_shared_library_obj_flag = "-c -fpic";

   if(code == 0)
    tc->cc_compile_shared_library_obj_flag = strdup(tc->cc_compile_bin_obj_flag);
  }

  /* C compiler include dir flag */
  if(tc->cc_include_dir_flag == NULL)
   tc->cc_include_dir_flag = "-I";

  /* C compiler define macro flag */
  if(tc->cc_define_macro_flag == NULL)
   tc->cc_define_macro_flag = "-D";

  /* CFLAGS */
  if((s = getenv("CFLAGS")) != NULL)
  {
   /* here we want to overide regardless */
   tc->cflags = strdup(s);
  } 

 }

 /* Build prefix is where the output for this build host goes */
 if(tc->build_prefix == NULL)
 {
  if(ctx->host_prefix == NULL)
  {
   if(strcmp(ctx->principle, "NATIVE") == 0)
   {
    snprintf(temp, 512, "./native");
   } else {
    snprintf(temp, 512, ".");
   }
  } else {
   s = build_prefix_from_host_prefix(ctx);
   snprintf(temp, 512, "%s", s);
   free(s);
  }
  tc->build_prefix = strdup(temp);
 }


 /* Package Config */
 code = is_pkg_config_needed(ctx, &cd);
 if(code == -1)
  return 1;

 if(code == 1)
 {
  if((s = getenv("PKGCONFIG")) == NULL)
  {
   if((s = tc->pkg_config) == NULL)
    s = "pkg-config";
  }
  
  if(test_runnable(ctx, s))
  {
   fprintf(stderr, 
           "BCA: Specify alternative with pkg-config with the PKGCONFIG environment variable.\n");
   return 1;
  }

  if(tc->pkg_config != s)
   tc->pkg_config = strdup(s);

  /* PKG_CONFIG_PATH */
  if((s = getenv("PKG_CONFIG_PATH")) != NULL)
  {
   /* here we want to overwrite regardless */
   snprintf(temp, 512,
#ifdef HAVE_CWD
            /* "%s/%s:%s", ctx->cwd, tc->build_prefix, s); */
            "%s:%s", tc->build_prefix, s);
#else
            /* "./%s:%s", tc->build_prefix, s); */
            "%s:%s", tc->build_prefix, s);
#endif
   tc->pkg_config_path = strdup(temp);
  } else {
   if(tc->pkg_config_path == NULL)
   {
    snprintf(temp, 512,
#ifdef HAVE_CWD
             /* "%s/%s", ctx->cwd, tc->build_prefix); */
             "%s", tc->build_prefix);
#else
             /* "./%s", tc->build_prefix); */
             "%s", tc->build_prefix);
#endif
    tc->pkg_config_path = strdup(temp);
   }
  }

  /* PKG_CONFIG_LIBDIR */
  if((s = getenv("PKG_CONFIG_LIBDIR")) != NULL)
  {
   /* here we want to overwrite regardless */
   tc->pkg_config_libdir = strdup(s);
  }
 }

 /* this prefix  & suffix stuff should be moved to functions for readability */

 /* Suffix for binaries; ie .exe */
 if(tc->binary_suffix == NULL)
 {
  yes = 0;
  if(contains_string(host_prefix, -1, "mingw", 5))
   yes = 1;

  if(strcmp(platform, "cygwin") == 0)
   yes = 1;

  if(yes)
  { 
   tc->binary_suffix = ".exe";
  } else {
   tc->binary_suffix = "";
  }
 }

 /* Suffix for shared libraries; ie .dll */
 if(tc->shared_library_suffix == NULL)
 {
  yes = 0;

  if(contains_string(host_prefix, -1, "mingw", 5))
   yes = 1;

  if(strcmp(platform, "cygwin") == 0)
   yes = 1;

  if(yes)
  {
   tc->shared_library_suffix = ".dll";
  } else if (strcmp(platform, "osx") == 0) {
   tc->shared_library_suffix = ".dylib"; 
  } else {
   tc->shared_library_suffix = ".so";
  }
 }

 /* Prefix for shared libaries */
 if(tc->shared_library_prefix == NULL)
 {
  if(contains_string(host_prefix, -1, "mingw", 5))
  {
   tc->shared_library_prefix = "";
  } else {
   if(strcmp(platform, "cygwin") == 0)
   {
    tc->shared_library_prefix = "cyg";
   } else {
    tc->shared_library_prefix = "lib";
   }
  }
 }

 /* Suffix for object files */
 if(tc->obj_suffix == NULL)
  tc->obj_suffix = ".o";

 /* LDFLAGS */
 if((s = getenv("LDFLAGS")) != NULL)
 {
  /* here we want to overwrite regardless */
  tc->ldflags = strdup(s);
 }

 if(ctx->install_prefix == NULL)
 {
  if(tc->install_prefix == NULL)
  {
   if(ctx->host_prefix == NULL)
   {
    if(strcmp(platform, "cygwin") == 0)
    {
     snprintf(install_prefix, 512, "/usr");
    } else {
     snprintf(install_prefix, 512, "/usr/local");
    }
   } else {
    snprintf(install_prefix, 512, "/usr/%s", ctx->host_prefix);
   }
  } else {
   snprintf(install_prefix, 512, "%s", tc->install_prefix);
  }
 } else {
  tc->install_prefix = NULL;
  tc->install_bin_dir = NULL;
  tc->install_lib_dir = NULL;
  tc->install_include_dir = NULL;
  tc->install_pkg_config_dir = NULL;
  tc->install_lib_dir = NULL;
  tc->install_locale_data_dir = NULL;
  snprintf(install_prefix, 512, "%s", ctx->install_prefix);
 }

 /* INSTALL_PREFIX */
 if(tc->install_prefix == NULL) 
 {
  snprintf(temp, 512, "%s", install_prefix);
  tc->install_prefix = strdup(temp);
 }

 /* INSTALL_BIN_DIR */
 if(tc->install_bin_dir == NULL) 
 {
  snprintf(temp, 512, "%s/bin", install_prefix);
  tc->install_bin_dir = strdup(temp);
 }

 /* INSTALL_LIB_DIR */
 if(tc->install_lib_dir == NULL)
 {
  snprintf(temp, 512, "%s/lib", install_prefix);
  tc->install_lib_dir = strdup(temp);
 }

 /* INSTALL_INCLUDE_DIR */
 if(tc->install_include_dir == NULL)
 {
  snprintf(temp, 512, "%s/include", install_prefix);
  tc->install_include_dir = strdup(temp);
 }

 /* INSTALL_PKG_CONFIG_DIR */
 if(tc->install_pkg_config_dir == NULL)
 {
  snprintf(temp, 512, "%s/lib/pkgconfig", install_prefix);
  tc->install_pkg_config_dir = strdup(temp);
 }

 /* INSTALL_LOCALE_DATA_DIR */
 if(tc->install_locale_data_dir == NULL)
 {
  snprintf(temp, 512, "%s/share/locale", install_prefix);
  tc->install_locale_data_dir = strdup(temp);
 }

 /* DEPENDS processing. 
    Given the lists external, and optional external filtered with --withouts, created 
    a unique test list and test. 
    Then given the --withouts, the tested list, and the internal deps list create the DEPENDS key
    for each component in this build.
 */
  n_test_packages = 0;
  test_package_list = NULL;
  test_package_optional_flags = NULL;


  /* optional dependencies */
  for(i=0; i < cd.n_components; i++)
  {
   yes = 1;
   for(j=0; j < ctx->n_disables; j++)
   {
    if(strcmp(cd.project_components[i], ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
   }

   if(yes)
   {
    cd.project_component = cd.project_components[i];
    cd.project_component_type = cd.project_component_types[i];

    if(list_component_opt_external_dependencies(ctx, &cd, &list, &n_elements))
     return 1;

    for(j=0; j < n_elements; j++)
    {
     x = 0;
     yes = 1;
     while(x < ctx->n_withouts)
     {
      if(strcmp(ctx->without_strings[x], list[j]) == 0)
      {
       yes = 0;
       break;
      }
      x++;
     }

     if(yes)
     {
      if((code = add_to_string_array(&test_package_list, n_test_packages, list[j], -1, 1)) < 0)
      {
       return 1;
      }

      if(code == 0)
      {
       n_test_packages++;

       test_package_optional_flags = expand_int_array(test_package_optional_flags, 
                                                      &test_package_optional_flags_size, 
                                                      n_test_packages);

       test_package_optional_flags[n_test_packages - 1] = 1;
      }
     }
    }
   }
   free_string_array(list, n_elements);
  }

  /* non-optional externals */
  for(i=0; i < cd.n_components; i++)
  {
   yes = 1;
   for(j=0; j < ctx->n_disables; j++)
   {
    if(strcmp(cd.project_components[i], ctx->disabled_components[j]) == 0)
    {
     yes = 0;
     break;
    }
   }

   if(yes)
   {
    cd.project_component = cd.project_components[i];
    cd.project_component_type = cd.project_component_types[i];

    if(list_component_external_dependencies(ctx, &cd, &list, &n_elements))
     return 1;

    for(j=0; j < n_elements; j++)
    {
     if((code = add_to_string_array(&test_package_list, n_test_packages, list[j], -1, 1)) < 0)
     {
      return 1;
     }

     if(code == 0)
     {
      n_test_packages++;

      test_package_optional_flags = expand_int_array(test_package_optional_flags, 
                                                     &test_package_optional_flags_size, 
                                                     n_test_packages);

      test_package_optional_flags[n_test_packages - 1] = 0;
     } else {

      handled = 0;
      for(x=0; x < n_test_packages; x++)
      {
       if(strcmp(test_package_list[x], list[j]) == 0)
       {
        /* Here we change the testing of a package from optional to non-optional which effects
           the error behavior. A --without-package will still give the ability to turn it off
           for optional cases which caused this case in the first place.
        */
        test_package_optional_flags[x] = 2;
        handled = 1;
        break;
       }
      }
      if(handled == 0)
      {
       fprintf(stderr, "BCA: element that should have been found in string array was not\n");
       exit(1);
      }
     }
    }
    free_string_array(list, n_elements);
   }
  }

  /* package config exist tests */
  for(i=0; i < n_test_packages; i++)
  {
   code = test_package_exist(ctx, &cd, tc, test_package_list[i], test_package_optional_flags[i]);
   if(code != 0)
   {
    if(test_package_optional_flags[i] != 1)
    {
     return 1;
    } else {
     if(add_to_string_array(&(ctx->without_strings), ctx->n_withouts, 
                            test_package_list[i], -1, 1))
     {
      fprintf(stderr, "BCA: add_to_string_array() failed while trying to add to the without list "
                      "following a failed test for the presence of an optional package\n");
      exit(1);
     }
     ctx->n_withouts++;
    }
   }
   test_package_optional_flags[i] = code;
  }
 
  /* the last step is to generate a list for the build configuration file's 
     host.component.depends record */
  for(i=0; i < cd.n_components; i++)
  {
   /* start with the internal deps */
   cd.project_component = cd.project_components[i];
   cd.project_component_type = cd.project_component_types[i];

   if(list_component_internal_dependencies(ctx, &cd, &list, &n_elements))
    return 1;

   n_depends = 0;
   depends = NULL;
   for(j=0; j<n_elements; j++)
   {
    if((code = add_to_string_array(&depends, n_depends, list[j], -1, 1)) < 0)
    {
     return 1;
    }
    if(code == 0)
     n_depends++;  

   }
   free_string_array(list, n_elements);

   /* everything in the external deps list should have been found or we would have errored before 
      this point */
   if(list_component_external_dependencies(ctx, &cd, &list, &n_elements))
    return 1;

   for(j=0; j<n_elements; j++)
   {
    if((code = add_to_string_array(&depends, n_depends, list[j], -1, 1)) < 0)
    {
     return 1;
    }
    if(code == 0)
     n_depends++;  
   }
   free_string_array(list, n_elements);

   /* the optional deps list still has to check with the without strings since again a package
      being both optional and non-optionl for different componets of the same project is valid */
   if(list_component_opt_external_dependencies(ctx, &cd, &list, &n_elements))
    return 1;

   for(j=0; j < n_elements; j++)
   {
    x = 0;
    yes = 1;
    while(x < ctx->n_withouts)
    {
     if(strcmp(ctx->without_strings[x], list[j]) == 0)
     {
      yes = 0;
      break;
     }
     x++;
    }

    if(yes)
    {
     x = 0;
     while(x < n_test_packages)
     {
      if(strcmp(test_package_list[x], list[j]) == 0)
      {
       if(test_package_optional_flags[x] != 0) 
        yes = 0;

       break;
      }
      x++;
     }
    }
     
    if(yes)
    {
     if((code = add_to_string_array(&depends, n_depends, list[j], -1, 1)) < 0)
     {
      return 1;
     }
     if(code == 0)
      n_depends++;  
    }
   }
   free_string_array(list, n_elements);


   /* we now need a single string for this record's value */
   temp_length = 0;
   for(j=0; j < n_depends; j++)
   {
    temp_length += snprintf(temp + temp_length, 1024 - temp_length, "%s", depends[j]);
    if( (n_depends > 1) && (j < n_depends - 1) )
     temp_length += snprintf(temp + temp_length, 1024 - temp_length, " ");
   }
   free_string_array(depends, n_depends);

   if(add_to_string_array(&mod_values, n_modify_records, temp, temp_length, 0))
    return 1;

   if(add_to_string_array(&mod_principles, n_modify_records, ctx->principle, -1, 0))
    return 1;

   if(add_to_string_array(&mod_components, n_modify_records, cd.project_component, -1, 0))
    return 1;

   if(add_to_string_array(&mod_keys, n_modify_records, "DEPENDS", 7, 0))
    return 1;

  n_modify_records++;
 }

 free_string_array(test_package_list, n_test_packages);
 

 if(append_host_configuration(ctx, tc, &cd, n_modify_records, 
                              mod_principles, mod_components,
                              mod_keys, mod_values))
 {
  return 1;
 }

 return 0;
}

int append_host_configuration_helper(int *n_modify_records,
                                     char ***mod_principles,
                                     char ***mod_components,
                                     char ***mod_keys,
                                     char ***mod_values,
                                     int p_length, int q_length, int k_length, int v_length,
                                     char *principle, char *qualifier,
                                     char *key, char *value)
{
 if(value == NULL)
  return 0;

 if(add_to_string_array(mod_values, *n_modify_records, value, v_length, 0))
  return 1;

 if(add_to_string_array(mod_principles, *n_modify_records, principle, p_length, 0))
  return 1;

 if(add_to_string_array(mod_components, *n_modify_records, qualifier, q_length, 0))
  return 1;

 if(add_to_string_array(mod_keys, *n_modify_records, key, k_length, 0))
  return 1;

 (*n_modify_records)++;

 return 0;
}

int append_host_configuration(struct bca_context *ctx,
                              struct host_configuration *tc, 
                              struct component_details *cd,
                              int n_modify_records,
                              char **mod_principles,
                              char **mod_components,
                              char **mod_keys,
                              char **mod_values)
{
 char **opt_dep_list;
 int i, j, z, n_opt_deps, n_withouts, p_length, q_length;
 FILE *output;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: append_host_configuration()\n");

 /* avoid the repitition */
 p_length = strlen(ctx->principle);
 q_length = strlen(ctx->qualifier);

 if(tc->build_tc)
 {
  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 2, -1,
                                      "BUILD", "ALL", "CC", 
                                      tc->build_tc->cc))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 22, -1,
                                      "BUILD", "ALL", 
                                      "CC_SPECIFY_OUTPUT_FLAG", 
                                      tc->build_tc->cc_output_flag))
   return 1;


  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, q_length, 23, -1,
                                      "BUILD", "ALL", 
                                      "CC_COMPILE_BIN_OBJ_FLAG", 
                                      tc->build_tc->cc_compile_bin_obj_flag))
   return 1;


  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, q_length, 19, -1,
                                      "BUILD", "ALL",
                                      "CC_INCLUDE_DIR_FLAG", 
                                      tc->build_tc->cc_include_dir_flag))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, q_length, 20, -1,
                                      "BUILD", "ALL", 
                                      "CC_DEFINE_MACRO_FLAG", 
                                      tc->build_tc->cc_define_macro_flag))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, q_length, 6, -1,
                                      "BUILD", "ALL",
                                      "CFLAGS", 
                                      tc->build_tc->cflags))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 10, -1,
                                      "BUILD", "ALL", 
                                      "PKG_CONFIG", 
                                      tc->build_tc->pkg_config))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 15, -1,
                                      "BUILD", "ALL", 
                                      "PKG_CONFIG_PATH", 
                                      tc->build_tc->pkg_config_path))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 17, -1,
                                      "BUILD", "ALL", 
                                      "PKG_CONFIG_LIBDIR", 
                                      tc->build_tc->pkg_config_libdir))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 13, -1,
                                      "BUILD", "ALL", 
                                      "BINARY_SUFFIX", 
                                      tc->build_tc->binary_suffix ))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 10, -1,
                                      "BUILD", "ALL", 
                                      "OBJ_SUFFIX", 
                                      tc->build_tc->obj_suffix ))
   return 1;

  if(append_host_configuration_helper(&n_modify_records,
                                      &mod_principles, &mod_components,
                                      &mod_keys, &mod_values,
                                      5, 3, 7, -1,
                                      "BUILD", "ALL", 
                                      "LDFLAGS", 
                                      tc->build_tc->ldflags ))
   return 1;
 }


 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 2, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC", tc->cc))
  return 1;
 
 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 12, -1,
                                     ctx->principle, ctx->qualifier,
                                     "BUILD_PREFIX", tc->build_prefix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 22, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC_SPECIFY_OUTPUT_FLAG", tc->cc_output_flag))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 23, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC_COMPILE_BIN_OBJ_FLAG", tc->cc_compile_bin_obj_flag))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 34, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG", 
                                     tc->cc_compile_shared_library_obj_flag))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 19, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC_INCLUDE_DIR_FLAG", tc->cc_include_dir_flag))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 20, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CC_DEFINE_MACRO_FLAG", tc->cc_define_macro_flag))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 13, -1,
                                     ctx->principle, ctx->qualifier,
                                     "BINARY_SUFFIX", tc->binary_suffix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 21, -1,
                                     ctx->principle, ctx->qualifier,
                                     "SHARED_LIBRARY_SUFFIX", tc->shared_library_suffix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 21, -1,
                                     ctx->principle, ctx->qualifier,
                                     "SHARED_LIBRARY_PREFIX", tc->shared_library_prefix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 10, -1,
                                     ctx->principle, ctx->qualifier,
                                     "OBJ_SUFFIX", tc->obj_suffix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 10, -1,
                                     ctx->principle, ctx->qualifier,
                                     "PKG_CONFIG", tc->pkg_config))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 15, -1,
                                     ctx->principle, ctx->qualifier,
                                     "PKG_CONFIG_PATH", tc->pkg_config_path))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 17, -1,
                                     ctx->principle, ctx->qualifier,
                                     "PKG_CONFIG_LIBDIR", tc->pkg_config_libdir))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 7, -1,
                                     ctx->principle, ctx->qualifier,
                                     "CFLAGS", tc->cflags))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 7, -1,
                                     ctx->principle, ctx->qualifier,
                                     "LDFLAGS", tc->ldflags))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 15, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_PREFIX", tc->install_prefix))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 15, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_BIN_DIR", tc->install_bin_dir))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 15, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_LIB_DIR", tc->install_lib_dir))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 19, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_INCLUDE_DIR", tc->install_include_dir))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 22, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_PKG_CONFIG_DIR", tc->install_pkg_config_dir))
  return 1;

 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, q_length, 23, -1,
                                     ctx->principle, ctx->qualifier,
                                     "INSTALL_LOCALE_DATA_DIR", tc->install_locale_data_dir))
  return 1;

 for(i=0; i < cd->n_components; i++)
 {
  cd->project_component = cd->project_components[i];
  cd->project_component_type = cd->project_component_types[i];
  opt_dep_list = NULL;

  if(list_component_opt_external_dependencies(ctx, cd, &opt_dep_list, &n_opt_deps))
  {
   fprintf(stderr, "BCA: list_component_opt_external_dependencies() failed\n");
   return 1;
  }

  n_withouts = 0;
  for(j=0; j < n_opt_deps; j++)
  {
   for(z=0; z < ctx->n_withouts; z++)
   {
    if(strcmp(ctx->without_strings[z], opt_dep_list[j]) == 0)
    {
     /* first pass just counts how many */
     n_withouts++;
    }
   }
  }

  if(n_withouts > 0)
  {
   temp_length = 0;
   for(j=0; j < n_opt_deps; j++)
   {
    for(z=0; z < ctx->n_withouts; z++)
    {
     if(strcmp(ctx->without_strings[z], opt_dep_list[j]) == 0)
     {
      temp_length += snprintf(temp + temp_length, 1024 - temp_length, "%s ", ctx->without_strings[z]);
     }
    }
   }
   temp[temp_length -= 1] = 0;   

   free_string_array(opt_dep_list, n_opt_deps);  

   if(append_host_configuration_helper(&n_modify_records,
                                       &mod_principles, &mod_components,
                                       &mod_keys, &mod_values,
                                       p_length, -1, 8, temp_length,
                                       ctx->principle, cd->project_component,
                                       "WITHOUTS", temp))
    return 1;
  }

 }

 /* disable list persistance */
 /* The disable list is not loaded from the build configuration at configure time.
    The way to enable something that was previously disabled is to not disable it on
    the subsequent configure (or edit the file/value). This is in contrast to build 
    configuration values such as CFLAGS that start with the old value (if any), modify 
    with flags etc, then save the revised state.

    Remember that the --enable-* logic is only to say "regarding components that are set
    to disabled by the project configuration itself, specifically turn this one on".
    This logic, plus the --disable-* switches is were the disable_components[] come from.

    Also remember that all of this is only to effect the build configuration (including the
    test that are performed). Generating  makefiles, build plots etc, use an existing build
    configuration. 
 */

 temp_length = 0;
 for(z=0; z < ctx->n_disables; z++)
 {
  temp_length += snprintf(temp + temp_length, 1024 - temp_length, 
                          "%s", ctx->disabled_components[z]);
  if( (ctx->n_disables > 1) && (z < ctx->n_disables - 1) )
   temp_length += snprintf(temp + temp_length, 1024 - temp_length, " ");
 }
   
 if(append_host_configuration_helper(&n_modify_records,
                                     &mod_principles, &mod_components,
                                     &mod_keys, &mod_values,
                                     p_length, 3, 8, temp_length,
                                     ctx->principle, "ALL",
                                     "DISABLES", temp))
  return 1;


 if(ctx->verbose > 0)
 {
  printf("BCA: about to modify the follow %d records in the build configuration:\n", 
         n_modify_records);
  for(i=0; i<n_modify_records; i++)
  {
   printf("BCA: %s.%s.%s = %s\n",
          mod_principles[i], mod_components[i], mod_keys[i], mod_values[i]);
  }
 }

 if((output = fopen("./buildconfiguration/buildconfiguration", "w")) == NULL)
 {
  fprintf(stderr, "BCA: failed to open output file ./buildconfiguration/buildconfiguration\n");
  return 1;
 }

 if(output_modifications(ctx, output, 
                         ctx->build_configuration_contents, 
                         ctx->build_configuration_length, 
                         n_modify_records, mod_principles, mod_components,
                         mod_keys, mod_values))
 {
  fprintf(stderr, "BCA: output_modifications() failed\n");
  return 1;
 }

 fclose(output);
 return 0;
}


