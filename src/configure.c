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

char temp[1024];
int temp_length;

int detect_platform(struct bca_context *ctx, 
                    char *host_root, 
                    char **platform)
{
 FILE *test;

 if(host_root == NULL)
 {
  if( (strcmp(ctx->principle, "NATIVE") != 0) &&
      (ctx->host_prefix != NULL) )
  {
   fprintf(stderr, 
           "BCA: detect_platform() - warning: HOST_ROOT is NULL, "
           "and this looks like cross compile.\n");
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

int is_file_of_type_used(struct bca_context *ctx, 
                         struct component_details *cd,
                         char *type_extension)
{
 int i, j, skip, pre_loaded, yes = 0;
 char *extension;

 if(ctx->verbose > 1)
 {
  printf("BCA: Looking for a project file for an enabled component with extension '%s'.\n",
         type_extension); 
  fflush(stdout);
 }

//this needs to be commented. has some mysteries. needs to considered disables

 /* first consider the FILES of a component */
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

   j=0;
   while(j < cd->n_file_names)
   {
    if(strcmp(cd->file_extensions[j], type_extension) == 0)
    {
     yes = 1;     
     break;
    }
 
    j++;
   }

   if(yes)
   {
    if(ctx->verbose)
     fprintf(stderr, "BCA: File %s satisfies condition to find a *.%s file.\n",
             cd->file_names[j], type_extension);
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

 /* now consider the INPUT */
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
   
    if(strcmp(extension, type_extension) == 0)
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

int is_c_compiler_needed(struct bca_context *ctx, 
                         struct component_details *cd)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for a C source files to see if C compiler is needed.\n"); 
  fflush(stdout);
 }

 return is_file_of_type_used(ctx, cd, "c");
}

int is_cxx_compiler_needed(struct bca_context *ctx, 
                           struct component_details *cd)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for a C++ source files to see if C++ compiler is needed.\n"); 
  fflush(stdout);
 }

 return is_file_of_type_used(ctx, cd, "cpp");
}

int is_erlang_compiler_needed(struct bca_context *ctx, 
                              struct component_details *cd)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for an Erlang source files to see if Erlang compiler is needed.\n"); 
  fflush(stdout);
 }

 return is_file_of_type_used(ctx, cd, "erl");
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
           "%s %s -fpic configuretestfile.c %s configuretestfile.o 1> configuretestoutput "
           "2> configuretestoutput", 
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
          "BCA: Tweak with PKG_CONFIG_PATH, PKG_CONFIG_LIBDIR, and "
          "PKGCONFIG envrionment variables.\n",
          package, ctx->principle, command);

  fprintf(stderr,
          "BCA: It could be that %s, or its development files are not installed "
          "on this system.\n",
          package);

  fprintf(stderr,
          "BCA: It is also possible that %s itself does not use package config, "
          "but something some\n"
          "BCA: distributions may include for convience and others do not.\n", package);

  fprintf(stderr, 
          "BCA: Similarly, the author(s) of \"%s\" might intend for a package config wrapper "
          "to be\n"
          "BCA: crafted for this purpose by those performing a build. Read the project's "
          "install file as\n"
          "BCA: one may already be included with the source files for your platform.\n",
           ctx->project_name);

  if(optional == 1)
   fprintf(stderr, 
           "BCA: Since this is only needed an optional package we will continue. "
           "You can disable this\n"
           "BCA: message with --without-%s .\n", package);

  if(optional == 2)
   fprintf(stderr, 
           "BCA: Since package \"%s\" is an optional dependency for some componets of "
           "this project\n"
           "BCA: --without-%s can be used to disable the dependency for such component(s). "
           "Although in this\n"
           "BCA: project it is still required for one or more other componets.\n",
           package, package);

 } else {
  if(optional == 1)
  {
   fprintf(stderr, 
           "BCA: Could not find optional package '%s' for host '%s'.\n",
           package, ctx->principle);
  } else {
   fprintf(stderr, 
           "BCA: Could not find package '%s' for host '%s'.\n", 
           package, ctx->principle);
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

int host_cc_configuration(struct bca_context *ctx, 
                          struct host_configuration *tc,
                          struct component_details *cd)
{
 char *s;
 char host_prefix[512];

 /* host prefix */
 if(ctx->host_prefix == NULL)
 {
  host_prefix[0] = 0;
 } else {
  snprintf(host_prefix, 512, "%s-", ctx->host_prefix);
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
  printf("BCA: Host %s C Compiler = %s\n", ctx->principle, temp);

 if(tc->cc != NULL)
  free(tc->cc);

 tc->cc = strdup(temp);
 return 0;
}

int host_cxx_configuration(struct bca_context *ctx, 
                           struct host_configuration *tc,
                           struct component_details *cd)
{
 char *s;
 char host_prefix[512];

 /* host prefix */
 if(ctx->host_prefix == NULL)
 {
  host_prefix[0] = 0;
 } else {
  snprintf(host_prefix, 512, "%s-", ctx->host_prefix);
 }

 /* C++ compiler */
 if((s = getenv("CXX")) != NULL)
 {
  /* here we want to overwrite regardless */
  snprintf(temp, 512, "%s%s", host_prefix, s); 
 } else {
  /* here we guess only if we have to */
  if(tc->cc != NULL)
  {
   snprintf(temp, 512, "%s", tc->cxx); 
  } else {
   snprintf(temp, 512, "%sg++", host_prefix); 
  }
 }

 if(test_runnable(ctx, temp))
 {
  fprintf(stderr, 
          "BCA: Specify alternative with C++ compiler with the CXX environment variable.\n");
  return 1;
 }
  
 if(ctx->verbose)
  printf("BCA: Host %s C++ Compiler = %s\n", ctx->principle, temp);

 if(tc->cxx != NULL)
  free(tc->cxx);

 tc->cxx = strdup(temp);
 return 0;
}

int c_family_configuration(struct bca_context *ctx, 
                           struct host_configuration *tc,
                           struct component_details *cd)
{
 int code, need_cc, need_cxx; 
 char *s;

 if((need_cxx = is_cxx_compiler_needed(ctx, cd)) == -1)
  return 1;

 if((need_cc = is_c_compiler_needed(ctx, cd)) == -1)
  return 1;

 if(need_cc)
  if(host_cc_configuration(ctx, tc, cd))
   return 1;

 if(need_cxx)
  if(host_cxx_configuration(ctx, tc, cd))
   return 1;

 if(need_cxx || need_cc )
 {
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

 return 0;
}

int pkg_config_tests(struct bca_context *ctx, 
                     struct host_configuration *tc,
                     struct component_details *cd)
{
 char *s;
 int code;

 if((code = is_pkg_config_needed(ctx, cd)) == -1)
  return 1;

 if(code == 1)
 {
  if((s = getenv("PKG_CONFIG")) == NULL)
  {
   if((s = tc->pkg_config) == NULL)
    s = "pkg-config";
  }
  
  if(test_runnable(ctx, s))
  {
   fprintf(stderr, 
           "BCA: Specify alternative with pkg-config with the PKG_CONFIG "
           "environment variable.\n");
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

int process_dependencies(struct bca_context *ctx, 
                         struct host_configuration *tc,
                         struct component_details *cd,
                         char ***mod_principles,
                         char ***mod_components,
                         char ***mod_keys, 
                         char ***mod_values,
                         int *n_modify_records)
{
 /* Given the lists of external, and optional external filtered with --withouts, created 
    a unique test list and test. Then given the --withouts, the tested list, and the
    internal deps list create the DEPENDS key for each component in this build.
 */
 int n_test_packages = 0, test_package_optional_flags_size = 0, 
     n_elements, n_depends, n_opt_deps, n_withouts, x, i, j, yes, code, handled, p_length, 
     *test_package_optional_flags = NULL;
 char **test_package_list = NULL, **depends = NULL, **list = NULL, **opt_dep_list = NULL;

 /* optional dependencies */
 for(i=0; i < cd->n_components; i++)
 {
  yes = 1;
  for(j=0; j < ctx->n_disables; j++)
  {
   if(strcmp(cd->project_components[i], ctx->disabled_components[j]) == 0)
   {
    yes = 0;
    break;
   }
  }

  /* if the component is not disabled, pull a list of its optional dependencies */
  if(yes)
  {
   cd->project_component = cd->project_components[i];
   cd->project_component_type = cd->project_component_types[i];

   if(list_component_opt_external_dependencies(ctx, cd, &list, &n_elements))
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

    /* if the optional dependency was not turned off with --without, add it to the
       list of packages to test for */
    if(yes)
    {
     if((code = add_to_string_array(&test_package_list, n_test_packages, list[j], -1, 1)) < 0)
     {
      return 1;
     }

     if(code == 0)
     {
      n_test_packages++;

      /* we keep an array along side the list of packages to test for that 
         holds if the package was an optional dependency. This way we don't
         test for the same package twice if it is in both catagories. The
         optional flag then deturmins if we can proceed if the package was
         not found. */
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
 for(i=0; i < cd->n_components; i++)
 {
  yes = 1;
  for(j=0; j < ctx->n_disables; j++)
  {
   if(strcmp(cd->project_components[i], ctx->disabled_components[j]) == 0)
   {
    yes = 0;
    break;
   }
  }

  /* if the component is not disabled, pull a list of its required dependencies */
  if(yes)
  {
   cd->project_component = cd->project_components[i];
   cd->project_component_type = cd->project_component_types[i];

   if(list_component_external_dependencies(ctx, cd, &list, &n_elements))
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
  code = test_package_exist(ctx, cd, tc, test_package_list[i], test_package_optional_flags[i]);
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
    host.component.depends record. This is how the set of dependencies
    persists from the configure phase to the makefile generation phase. */
 for(i=0; i < cd->n_components; i++)
 {
  /* start with the internal deps */
  cd->project_component = cd->project_components[i];
  cd->project_component_type = cd->project_component_types[i];

  if(list_component_internal_dependencies(ctx, cd, &list, &n_elements))
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
  if(list_component_external_dependencies(ctx, cd, &list, &n_elements))
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
  if(list_component_opt_external_dependencies(ctx, cd, &list, &n_elements))
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

  /* the actual modifying of the buildconfiguration file is deffered 
     until the last step (for several reasons) with the 4 string arrays
     of p.c.k = v "to modify" strings.   
   */ 
  if(add_to_string_array(mod_principles, *n_modify_records, ctx->principle, -1, 0))
   return 1;

  if(add_to_string_array(mod_components, *n_modify_records, cd->project_component, -1, 0))
   return 1;

  if(add_to_string_array(mod_keys, *n_modify_records, "DEPENDS", 7, 0))
   return 1;

  if(add_to_string_array(mod_values, *n_modify_records, temp, temp_length, 0))
   return 1;

  (*n_modify_records)++;
 }

 free_string_array(test_package_list, n_test_packages);

 /* WITHOUTS persistance */
 p_length = strlen(ctx->principle);

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
   for(x=0; x < ctx->n_withouts; x++)
   {
    if(strcmp(ctx->without_strings[x], opt_dep_list[j]) == 0)
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
    for(x=0; x < ctx->n_withouts; x++)
    {
     if(strcmp(ctx->without_strings[x], opt_dep_list[j]) == 0)
     {
      temp_length += snprintf(temp + temp_length, 1024 - temp_length, "%s ", 
                              ctx->without_strings[x]);
     }
    }
   }
   temp[temp_length -= 1] = 0;   

   free_string_array(opt_dep_list, n_opt_deps);  

   if(append_host_configuration_helper(n_modify_records,
                                       mod_principles, mod_components,
                                       mod_keys, mod_values,
                                       p_length, -1, 8, temp_length,
                                       ctx->principle, cd->project_component,
                                       "WITHOUTS", temp))
    return 1;
  }

 }

 return 0;
}

int derive_file_suffixes(struct bca_context *ctx, 
                         struct host_configuration *tc,
                         struct component_details *cd,
                         char *platform)
{
 char host_prefix[512], *s;
 int yes;
 
 /* host prefix */
 if(ctx->host_prefix == NULL)
 {
  host_prefix[0] = 0;
 } else {
  snprintf(host_prefix, 512, "%s-", ctx->host_prefix);
 }

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

 return 0;
}
int derive_install_paths(struct bca_context *ctx, 
                         struct host_configuration *tc,
                         struct component_details *cd,
                         char *platform)
{
 char install_prefix[512];

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

 return 0;
}

int swap_checks(struct bca_context *ctx)
{
 int x, y, disabled_by_default;
 char *value, **project_disables;
 int j, n_project_disables;

 /* check that no swaped components are disabled;
    this will mean that components disabled by default will
    require --enable-X --swap-X otherhost, but this seems
    to make sense.
  */

 for(x=0; x<ctx->n_swaps; x++)
 {

  if(strcmp(ctx->swapped_component_hosts[x],
            ctx->principle) == 0)
  {
   fprintf(stderr, 
           "BCA: swap for component %s can not point to the same host\n",
           ctx->swapped_components[x]);
   return 1;
  }

  for(y=0; y<ctx->n_disables; y++)
  {
   if(strcmp(ctx->swapped_components[x], ctx->disabled_components[y]) == 0)
   {
    disabled_by_default = 0;

    if((value = lookup_key(ctx, ctx->project_configuration_contents,
                           ctx->project_configuration_length, 
                           "NONE", "NONE", "DISABLES")) != NULL)
    {
     if(split_strings(ctx, value, -1, &n_project_disables, &project_disables))
     {
      fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
      return 1;
     }
    
     j = 0;
     while(j < n_project_disables)
     {
      if(strcmp(ctx->swapped_components[x], project_disables[j]) == 0)
      {
       disabled_by_default = 1;
       break;
      }
      j++;
     }
    }

    if(disabled_by_default)
    {
     fprintf(stderr,
             "BCA: Swaped compent \"%s\" is disabled by default. "
             "This requires both --enable-%s and --swap-%s\n.",
             ctx->swapped_components[x], ctx->swapped_components[x], 
             ctx->swapped_components[x]);
    } else {
     fprintf(stderr, 
             "BCA: --disable-%s and --swap-%s are mutally exclusive.\n",
             ctx->swapped_components[x], ctx->swapped_components[x]);
    }

    return 1;
   }
  }
 } 

 return 0;
}

int disables_and_enables(struct bca_context *ctx, 
                         struct host_configuration *tc,
                         struct component_details *cd,
                         int *n_modify_records,
                         char ***mod_principles,
                         char ***mod_components,
                         char ***mod_keys,
                         char ***mod_values)
{
 char *value, **project_disables;
 int i, j, n_project_disables, yes, p_length;

 /* hack warning: 
    list_project_components() takes the disabled list into account,
    here want the list of all components period */
 n_project_disables = ctx->n_disables;
 project_disables = ctx->disabled_components;
 ctx->n_disables = 0;
 ctx->disabled_components = NULL;
 if(list_project_components(ctx, cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed.\n");
  return 1;
 }
 ctx->n_disables = n_project_disables;
 ctx->disabled_components = project_disables;
 n_project_disables = 0;
 project_disables = NULL;

 if(ctx->verbose)
 {
  printf("BCA: found (%d) project components: ", cd->n_components);
  for(i=0; i < cd->n_components; i++)
  {
   printf("%s ", cd->project_components[i]);
  }
  printf("\n");
 }

 /* 1) Start with disable by default components - the NONE.NONE.DISABLES. */
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
 }

 /* 2) If there are any disables on the command line, add those to our
       working list (project_disables) while verifying that those are
       actually project components (catch user errors). */
 for(i=0; i<ctx->n_disables; i++)
 {
  yes = 0;
  j = 0;
  while(j < cd->n_components)
  {
   if(strcmp(ctx->disabled_components[i], cd->project_components[j]) == 0)
   {
    yes = 1;
    break;
   }
   j++;
  }
  
  if(yes == 0)
  {
   fprintf(stderr,
           "BCA: disabled component '%s' is not a component of this project\n",
           ctx->disabled_components[i]);
   return 1;
  }

  if(add_to_string_array(&project_disables, n_project_disables, 
                         ctx->disabled_components[i], -1, 1) == -1)
  {
   /* this could safely return 1 for the duplicates */
   fprintf(stderr, "BCA: add_to_string_array() failed. This should not have happend!\n");
   return 1;
  }  
  n_project_disables++;
 }

 if(ctx->n_enables == 0)
 {
  if(n_project_disables > 0)
  {
   /* 3) if there are no enables, then we can swap out our working set
         for the set that will be use */
   free_string_array(ctx->disabled_components, ctx->n_disables);
   ctx->disabled_components = project_disables;
   ctx->n_disables = n_project_disables;
  }
  /* 4) if there are no enables, and no default disables, then then the
     only disables will be what ever (if any) specified on the command 
     line and already in use */
 } else {

  /* 5) we must consider the list of enables. enables just get taken
        away from the set of disables */
  free_string_array(ctx->disabled_components, ctx->n_disables);
  ctx->disabled_components = NULL;
  ctx->n_disables = 0; 

  /* user error check on the enable list */
  for(i=0; i < ctx->n_enables; i++)
  {
   yes = 0;
   j=0;
   while(j < n_project_disables)
   {
    if(strcmp(ctx->enabled_components[i], project_disables[j]) == 0)
    {
     yes = 1;
     break;
    }
    j++;
   }

   if(yes == 0)
   {
    fprintf(stderr, 
            "BCA: I do not have a disabled component named %s to enable.\n",
            ctx->enabled_components[i]);
     return 1;
   }
  }

  for(i=0; i < n_project_disables; i++)
  {
   yes = 1;

   j=0;
   while(j < ctx->n_enables)
   {
    if(strcmp(ctx->enabled_components[j], project_disables[i]) == 0)
    {
     yes = 0;
     break;
    }
    j++;
   }

   /* those disables not in the enable list will get placed in the new
      effective set */
   if(yes == 1)
   {
    if(add_to_string_array(&(ctx->disabled_components), ctx->n_disables, 
                           project_disables[i], -1, 1) != 0)
    {
     fprintf(stderr, "BCA: add_to_string_array() failed. This should not have happend!\n");
      return 1;
    }
    ctx->n_disables++;
   }
  }

  /* working set no longer needed */
  free_string_array(project_disables, n_project_disables);
  project_disables = NULL;
  n_project_disables = 0;
 }

 /* DISABLED persistance */
 /* The disable list is not loaded from the build configuration at configure time.
    The way to enable something that was previously disabled is to not disable it on
    the subsequent configure (or edit the file/value). This is in contrast to build 
    configuration values such as CFLAGS that start with the old value (if any), modify 
    with flags etc, then save the revised state.

    Also remember that all of this is only to effect the build configuration (including the
    test that are performed). Generating  makefiles, build plots etc, use an existing build
    configuration. 
 */

 temp_length = 0;
 for(i=0; i < ctx->n_disables; i++)
 {
  temp_length += snprintf(temp + temp_length, 1024 - temp_length, 
                          "%s", ctx->disabled_components[i]);
  if( (ctx->n_disables > 1) && (i < ctx->n_disables - 1) )
   temp_length += snprintf(temp + temp_length, 1024 - temp_length, " ");
 }

 p_length = strlen(ctx->principle);

 if(append_host_configuration_helper(n_modify_records,
                                     mod_principles, mod_components,
                                     mod_keys, mod_values,
                                     p_length, 3, 8, temp_length,
                                     ctx->principle, "ALL",
                                     "DISABLES", temp))
  return 1;

 
 return 0;
}

int persist_host_swap_configuration(struct bca_context *ctx,
                                    struct host_configuration *tc, 
                                    struct component_details *cd,
                                    int *n_modify_records,
                                    char ***mod_principles,
                                    char ***mod_components,
                                    char ***mod_keys,
                                    char ***mod_values)
{
 char o_principle[256], o_component[256], o_key[256], *o_value;
 int *handled, i, allocation_size, end, yes, p_length;

 allocation_size = sizeof(int) * (ctx->n_swaps + 1);
 if((handled = (int *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }
 memset(handled, 0, allocation_size);

 p_length = strlen(ctx->principle);
 end = -1;
                          
 while(iterate_key_primitives(ctx, ctx->build_configuration_contents,
                              ctx->build_configuration_length, &end,
                              ctx->principle, NULL, "SWAP", 
                              o_principle, o_component, o_key, NULL))
 {
  o_value = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       o_principle, o_component, o_key);

  yes = 1;
  i = 0;
  while(i < ctx->n_swaps)
  {
   if(strcmp(o_component, ctx->swapped_components[i]) == 0)
   {
    yes = 0;
    {
     handled[i] = 1;
     if(strcmp(o_key, ctx->swapped_component_hosts[i]) == 0)
     {
      /* case 1: we are retaining a swap value */
     } else {
      /* case 2: update a swap value */
      if(append_host_configuration_helper(n_modify_records,
                                          mod_principles,
                                          mod_components,
                                          mod_keys,
                                          mod_values,
                                          p_length, -1, 4, -1,
                                          o_principle, o_component, "SWAP",
                                          ctx->swapped_component_hosts[i]))
       return 1;
     }
     break;
    }
   }
   i++;
  }

  if(yes)
  {
   /* case 3: remove a swap */
   if(append_host_configuration_helper(n_modify_records,
                                       mod_principles,
                                       mod_components,
                                       mod_keys,
                                       mod_values,
                                       p_length, -1, 4, -1,
                                       o_principle, o_component, "SWAP", NULL))
   return 1;
  }

  free(o_value);
 }

 for(i=0; i < ctx->n_swaps; i++)
 {
  if(handled[i] == 0)
  {
   /* case 4: add a swap value */
   if(append_host_configuration_helper(n_modify_records,
                                       mod_principles,
                                       mod_components,
                                       mod_keys,
                                       mod_values,
                                       p_length, -1, 4, -1,
                                       ctx->principle, 
                                       ctx->swapped_components[i], "SWAP",
                                       ctx->swapped_component_hosts[i]))
    return 1;
  }
 }

 free(handled);
 return 0;
}

int append_host_configuration(struct bca_context *ctx,
                              struct host_configuration *tc, 
                              struct component_details *cd,
                              int *n_modify_records,
                              char ***mod_principles,
                              char ***mod_components,
                              char ***mod_keys,
                              char ***mod_values)
{
 int i, p_length, q_length;
 FILE *output;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: append_host_configuration()\n");

 char *host_updates[45] = 

 { "CC", tc->cc, 
   "BUILD_PREFIX", tc->build_prefix,
   "CC_SPECIFY_OUTPUT_FLAG", tc->cc_output_flag,
   "CC_COMPILE_BIN_OBJ_FLAG", tc->cc_compile_bin_obj_flag,
   "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG", tc->cc_compile_shared_library_obj_flag,
   "CC_INCLUDE_DIR_FLAG", tc->cc_include_dir_flag,
   "CC_DEFINE_MACRO_FLAG", tc->cc_define_macro_flag,
   "BINARY_SUFFIX", tc->binary_suffix,
   "SHARED_LIBRARY_SUFFIX", tc->shared_library_suffix,
   "SHARED_LIBRARY_PREFIX", tc->shared_library_prefix,
   "OBJ_SUFFIX", tc->obj_suffix,
   "PKG_CONFIG", tc->pkg_config,
   "PKG_CONFIG_PATH", tc->pkg_config_path,
   "PKG_CONFIG_LIBDIR", tc->pkg_config_libdir,
   "CFLAGS", tc->cflags,
   "LDFLAGS", tc->ldflags,
   "INSTALL_PREFIX", tc->install_prefix,
   "INSTALL_BIN_DIR", tc->install_bin_dir,
   "INSTALL_LIB_DIR", tc->install_lib_dir,
   "INSTALL_INCLUDE_DIR", tc->install_include_dir,
   "INSTALL_PKG_CONFIG_DIR", tc->install_pkg_config_dir,
   "INSTALL_LOCALE_DATA_DIR", tc->install_locale_data_dir } ;


 p_length = strlen(ctx->principle);
 q_length = strlen(ctx->qualifier);

 for(i=0; i < 44; i += 2)
 {
  if(append_host_configuration_helper(n_modify_records,
                                      mod_principles, mod_components,
                                      mod_keys, mod_values,
                                      p_length, q_length, -1, -1,
                                      ctx->principle, ctx->qualifier,
                                      host_updates[i], host_updates[i + 1]))
  return 1;
 }

 if(ctx->verbose > 0)
 {
  printf("BCA: about to modify the follow %d records in the build configuration:\n", 
         *n_modify_records);
  for(i=0; i<*n_modify_records; i++)
  {
   printf("BCA: %s.%s.%s = %s\n",
          (*mod_principles)[i], (*mod_components)[i], (*mod_keys)[i], (*mod_values)[i]);
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
                         *n_modify_records, *mod_principles, *mod_components,
                         *mod_keys, *mod_values))
 {
  fprintf(stderr, "BCA: output_modifications() failed\n");
  return 1;
 }

 fclose(output);
 return 0;
}

int configure(struct bca_context *ctx)
{
 char *s;
 int n_modify_records = 0;
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
 }
 
 if(ctx->project_configuration_contents == NULL)
 {
  if((ctx->project_configuration_contents = 
      read_file("./buildconfiguration/projectconfiguration", 
                &(ctx->project_configuration_length), 0)) == NULL)
  {
   fprintf(stderr, "BCA: can't open project configuration file\n");
   return 1;
  }
 }

 if(resolve_project_name(ctx))
 {
  fprintf(stderr, "BCA: can't resolve project name.\n"
                  "BCA: Make sure project file has a NONE.NONE.PROJECT_NAME record\n");
  return 1;
 } 

 host_root = getenv("HOST_ROOT");
 if(ctx->verbose)
  if(host_root)
   fprintf(stderr, "BCA: HOST_ROOT set via environment variable\n");

 /* target platform */
 if(detect_platform(ctx, host_root, &platform))
 {
  fprintf(stderr, "BCA: detect_platform() hard failed\n");
  return 1;
 }

 /* Build prefix is where the output for this build host goes */
 if(ctx->build_prefix != NULL)
 {
  if(tc->build_prefix != NULL)
   free(tc->build_prefix);
  snprintf(temp, 512, "./%s", ctx->build_prefix);
  tc->build_prefix = strdup(temp);
 } else {
  if(tc->build_prefix == NULL)
  {
   if(ctx->host_prefix == NULL)
   {
    if(strcmp(ctx->principle, "NATIVE") == 0)
    {
     snprintf(temp, 512, "./native");
    } else {
     snprintf(temp, 512, "./%s", ctx->principle);
    }
   } else {
    s = build_prefix_from_host_prefix(ctx);
    snprintf(temp, 512, "%s", s);
    free(s);
   }
   tc->build_prefix = strdup(temp);
  }
 }

 if(derive_file_suffixes(ctx, tc, &cd, platform))
  return 1;

 if(derive_install_paths(ctx, tc, &cd, platform))
  return 1;

 /* we need to find out what is enabled next so as to be able
    to skip configure logic for parts not enabled */
 if(disables_and_enables(ctx, tc, &cd,
                         &n_modify_records,
                         &mod_principles,
                         &mod_components,
                         &mod_keys,
                         &mod_values))
  return 1;

 if(swap_checks(ctx))
  return 1;

 if(c_family_configuration(ctx, tc, &cd))
  return 1;

 if(pkg_config_tests(ctx, tc, &cd))
  return 1;

 if(process_dependencies(ctx, tc, &cd,
                         &mod_principles,
                         &mod_components,
                         &mod_keys, 
                         &mod_values,
                         &n_modify_records))
 return 1;
 
 if(persist_host_swap_configuration(ctx, tc, &cd, 
                                    &n_modify_records,
                                    &mod_principles,
                                    &mod_components,
                                    &mod_keys,
                                    &mod_values))
  return 1;

 if(append_host_configuration(ctx, tc, &cd, &n_modify_records, 
                              &mod_principles, &mod_components,
                              &mod_keys, &mod_values))
 {
  return 1;
 }

 return 0;
}


