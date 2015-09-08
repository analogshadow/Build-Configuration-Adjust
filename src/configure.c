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
#include <unistd.h>

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

int assemble_list_of_used_source_files(struct bca_context *ctx,
                                       char ***file_list_ptr,
                                       char ***extensions_list_ptr,
                                       int *count_ptr)
{
 char **names = NULL, **extensions = NULL, **component_names, **files,
      **component_types, *extension, **inputs, **inputs_out_names;
 int i, j, n_components, n_files, count = 0, n_inputs;

 if(ctx->verbose > 0)
 {
  printf("BCA: Gathering list of source files in use...\n");
 }

 names = NULL;
 extensions = NULL;
 count = 0;

 if(list_of_project_components(ctx,
                               &component_names,
                               &component_types,
                               &n_components, 0))
 {
  return 1;
 }

 /* iterate over non-disabled components */
 for(i=0; i < n_components; i++)
 {
  /* CAT components don't count here */
  if(strcmp(component_types[i], "CAT") == 0)
   continue;

  files = NULL;
  n_files = 0;

  /* first consider the .FILES of a component */
  if(lookup_value_as_list(ctx, OPERATE_PROJECT_CONFIGURATION,
                          component_types[i], component_names[i], "FILES",
                          &files, &n_files))
  {
   return 1;
  }

  for(j=0; j<n_files; j++)
  {
   /* duplicates may arise, since files can be used for more than one
      component, here that must be kept since different extensions are possible */
   if(add_to_string_array(&names, count, files[j], -1, 0))
    return 1;

   if(path_extract(files[j], NULL, &extension))
    return 1;

   if(add_to_string_array(&extensions, count, extension, -1, 0))
    return 1;

   free(extension);
   count++;
  }

  free_string_array(files, n_files);

  /* now consider the .INPUT */
  inputs = NULL;
  inputs_out_names = NULL;
  n_inputs = 0;

  if(lookup_component_inputs(ctx, component_types[i], component_names[i],
                             &inputs, &inputs_out_names, &n_inputs))
   return 1;

  for(j=0; j < n_inputs; j++)
  {
   if(path_extract(inputs_out_names[j], NULL, &extension))
    return 1;

   if(add_to_string_array(&names, count, inputs_out_names[j], -1, 0))
    return 1;

   if(add_to_string_array(&extensions, count, extension, -1, 0))
    return 1;

   count++;

   free(extension);
  }

  if(n_inputs > 0)
  {
   free_string_array(inputs, n_inputs);
   free_string_array(inputs_out_names, n_inputs);
  }

 }

 *file_list_ptr = names;
 *extensions_list_ptr = extensions;
 *count_ptr = count;

 return 0;
}

int is_file_of_type_used(struct bca_context *ctx,
                         char **files, char **extensions, int count,
                         char *type_extension)
{
 int i;

 if(ctx->verbose > 1)
 {
  printf("BCA: Looking for a project source file for an enabled component with extension '%s'...\n",
         type_extension);
 }

 for(i=0; i<count; i++)
 {
  if(extensions[i] != NULL)
  {
   if(strcmp(extensions[i], type_extension) == 0)
   {
    if(ctx->verbose)
     fprintf(stderr,
             "BCA: File %s satisfies condition to find a *.%s file.\n",
             files[i], type_extension);
    return 1;
   }
  }
 }

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: No files matching *.%s found.\n", type_extension);

 return 0;
}

int is_c_compiler_needed(struct bca_context *ctx,
                         char **files, char **extensions, int count)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for a C source files to see if C compiler is needed...\n");
  fflush(stdout);
 }

 return is_file_of_type_used(ctx, files, extensions, count, ".c");
}

int is_cxx_compiler_needed(struct bca_context *ctx,
                           char **files, char **extensions, int count)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for a C++ source files to see if C++ compiler is needed...\n");
  fflush(stdout);
 }

 if(is_file_of_type_used(ctx, files, extensions, count, ".cc"))
  return 1;
 else if(is_file_of_type_used(ctx, files, extensions, count, ".cpp"))
  return 1;
 else if(is_file_of_type_used(ctx, files, extensions, count, ".cxx"))
  return 1;
 else
 return 0;
}

int is_erlang_compiler_needed(struct bca_context *ctx,
                              char **files, char **extensions, int count)
{
 if(ctx->verbose)
 {
  printf("BCA: Looking for an Erlang source files to see if Erlang compiler is needed...\n");
  fflush(stdout);
 }

 return is_file_of_type_used(ctx, files, extensions, count, "erl");
}

int is_pkg_config_needed(struct bca_context *ctx)
{
 char *value;

 if(ctx->verbose)
 {
  printf("BCA: Looking for dependences to see if pkg-config is needed...\n");
 }

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        NULL, NULL, "INT_DEPENDS")) == NULL)
 {
  if((value = lookup_key(ctx,
                         ctx->project_configuration_contents,
                         ctx->project_configuration_length,
                         NULL, NULL, "EXT_DEPENDS")) == NULL)
  {
   if((value = lookup_key(ctx,
                          ctx->project_configuration_contents,
                          ctx->project_configuration_length,
                          NULL, NULL, "OPT_EXT_DEPENDS")) == NULL)
   {
    if((value = lookup_key(ctx,
                           ctx->project_configuration_contents,
                           ctx->project_configuration_length,
                           NULL, NULL, "INT_EXT_DEPENDS")) == NULL)
    {
     return 0;
    }
   }
  }
 }

 free(value);
 return 1;
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

int test_package_exist_helper(char *command,
                              struct bca_context *ctx,
                              struct host_configuration *tc,
                              char *package)
{
 int code, length;

 length = 0;

 if(tc->pkg_config_path != NULL)
  length += snprintf(command + length, 1024 - length,
                    "PKG_CONFIG_PATH=%s ", tc->pkg_config_path);

 if(tc->pkg_config_libdir != NULL)
  length += snprintf(command + length, 1024 - length,
                    "PKG_CONFIG_LIBDIR=%s ", tc->pkg_config_libdir);

 if(tc->pkg_config == NULL)
 {
  fprintf(stderr, "BCA: I should have a path to a pkg_config binary executable by now.\n");
  return -1;
 }

 snprintf(command + length, 1024 - length, "%s %s", tc->pkg_config, package);

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: about to try system(\"%s\")...\n", command);

 code = system(command);

 if( (code = system(command)) == -1)
 {
  fprintf(stderr, "BCA: system(%s) failed with error code %d, %s\n",
          command, WEXITSTATUS(code), strerror(errno));
  return 1;
 }

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: system() returned %d\n", WEXITSTATUS(code));

 if(WEXITSTATUS(code) == 0)
  return 0;

 return 1;
}

int test_package_exist(struct bca_context *ctx,
                       struct host_configuration *tc,
                       char *package, char *host)
{
 char command[1024];
 int code, length;
 FILE *test;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: test_package_exist(%s)\n", package);

 if((code = test_package_exist_helper(command, ctx, tc, package)) == 0)
  return 0;

 if(code < 0)
  return -1;

 length = snprintf(command, 1024, "./buildconfiguration/generate-pkg-config-%s",
                   package);

 if((test = fopen(command, "r")) != NULL)
 {
  fclose(test);
  snprintf(command + length, 1024 - length,
           "%s", host);

  if(ctx->verbose > 1)
   fprintf(stderr, "BCA: attempting \"%s\"...\n", command);

  if( (code = system(command)) > -1)
  {
   if(WEXITSTATUS(code) == 0)
   {
    if((code = test_package_exist_helper(command, ctx, tc, package)) == 0)
     return 0;
    if(code < 0)
     return 1;

   } else {
    fprintf(stderr, "BCA: %s not successful.\n", command);
   }
  }
 }

 if(ctx->verbose > 0)
 {
  fprintf(stderr,
          "BCA: Could not find package '%s' for host '%s' using\n"
          "BCA: command line '%s'.\n"
          "BCA: Tweak with PKG_CONFIG_PATH, PKG_CONFIG_LIBDIR, and "
          "PKGCONFIG envrionment variables.\n",
          package, host, command);

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

int host_cc_configuration(struct bca_context *ctx,
                          struct host_configuration *tc)
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
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
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
          "BCA: Specify alternative C compiler with the CC environment variable.\n");
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
                           struct host_configuration *tc)
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
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
 {
  /* here we want to overwrite regardless */
  snprintf(temp, 512, "%s%s", host_prefix, s);
 } else {
  /* here we guess only if we have to */
  if(tc->cxx != NULL)
  {
   snprintf(temp, 512, "%s", tc->cxx);
  } else {
   snprintf(temp, 512, "%sg++", host_prefix);
  }
 }

 if(test_runnable(ctx, temp))
 {
  fprintf(stderr,
          "BCA: Specify alternative C++ compiler with the CXX environment variable.\n");
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
                           char *host,
                           char **files, char **extensions, int count)
{
 int code, need_cc, need_cxx;
 char *s;

 if((need_cxx = is_cxx_compiler_needed(ctx, files, extensions, count)) == -1)
  return 1;

 if((need_cc = is_c_compiler_needed(ctx, files, extensions, count)) == -1)
  return 1;

 if(need_cc)
  if(host_cc_configuration(ctx, tc))
   return 1;

 if(need_cxx)
  if(host_cxx_configuration(ctx, tc))
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
   if(s[0] == 0)
    s = NULL;
  if(s != NULL)
  {
   /* here we want to overide regardless */
   tc->cflags = strdup(s);
  }

  /* CCFLAGS */
  if((s = getenv("CCFLAGS")) != NULL)
   if(s[0] == 0)
    s = NULL;
  if(s != NULL)
  {
   /* here we want to overide regardless */
   tc->ccflags = strdup(s);
  }

  /* CXXFLAGS */
  if((s = getenv("CXXFLAGS")) != NULL)
   if(s[0] == 0)
    s = NULL;
  if(s != NULL)
  {
   /* here we want to overide regardless */
   tc->cxxflags = strdup(s);
  }

 }

 return 0;
}

int host_erlc_configuration(struct bca_context *ctx,
                            struct host_configuration *tc,
                            char *host)
{
 char *s;
 char host_prefix[512];

 /* Normally we think of cross compilation for compilers that output
    machine code. We also think of host prefixes (as in foo-bar-baz-gcc)
    as identifying one of multiple compilers. Different versions of
    the same compiler are generally suffixes (ie gcc-4.4). Typically,
    multiple versions of erlang are installed in different paths,
    and we would select which one with "ERLC=/full/path/of/erlc ./configure".
    Sense the logic is already in place in bca, for consistancy erlc
    can also leverage the host prefix. For example,
    "ERLC=abc123 ./configure --host=custombranch" would attempt
    to use an erlc binary named custombranch-abc123.

    If there was an erlang compiler that created non-portable output,
    being used to cross compile, this would be the model. Also as expected,
    build_prefix would based on host_prefix unless otherwise specified.
    (When neither option is specified, build_prefix defaults to "native".)
 */

 /* host prefix */
 if(ctx->host_prefix == NULL)
 {
  host_prefix[0] = 0;
 } else {
  snprintf(host_prefix, 512, "%s-", ctx->host_prefix);
 }

 /* erlang compiler */
 if((s = getenv("ERLC")) != NULL)
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
 {
  /* here we want to overwrite regardless */
  snprintf(temp, 512, "%s%s", host_prefix, s);
 } else {
  /* here we guess only if we have to */
  if(tc->erlc != NULL)
  {
   snprintf(temp, 512, "%s", tc->erlc);
  } else {
   snprintf(temp, 512, "%serlc", host_prefix);
  }
 }

 if(test_runnable(ctx, temp))
 {
  fprintf(stderr,
          "BCA: Specify alternative Erlang compiler with the ERLC environment variable.\n");
  return 1;
 }

 tc->erlc = strdup(temp);
 return 0;
}


int erlang_family_configuration(struct bca_context *ctx,
                                struct host_configuration *tc,
                                char *host,
                                char **files, char **extensions, int count)
{
 int need_erlc;
 char *s;

 if((need_erlc = is_erlang_compiler_needed(ctx, files, extensions, count)) == -1)
  return 1;

 if(need_erlc == 0)
  return 0;

 if(host_erlc_configuration(ctx, tc, host))
  return 1;

 /* erlang compiler output directory flag */
 if(tc->erlc_output_dir_flag == NULL)
  tc->erlc_output_dir_flag = "-o";

 /* ERLCFLAGS */
 if((s = getenv("ERLCFLAGS")) != NULL)
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
 {
  /* here we want to overide regardless */
  tc->erlc_flags = strdup(s);
 }

 return 0;
}


int pkg_config_tests(struct bca_context *ctx,
                     struct host_configuration *tc)
{
 char *s;
 int code;

 if((code = is_pkg_config_needed(ctx)) == -1)
  return 1;

 if(code == 1)
 {
  if((s = getenv("PKG_CONFIG")) != NULL)
   if(s[0] == 0)
    s = NULL;
  if(s == NULL)
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
   if(s[0] == 0)
    s = NULL;

  if(s != NULL)
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
   if(s[0] == 0)
    s = NULL;
  if(s != NULL)
  {
   /* here we want to overwrite regardless */
   tc->pkg_config_libdir = strdup(s);
  }
 }

 return 0;
}

int compute_effective_withouts_for_host(struct bca_context *ctx, char *host,
                                        char ***new_withouts,
                                        int *n_new_withouts)
{
 char *value, **withouts_from_bc, **default_withouts;
 int n_withouts_from_bc, n_default_withouts;

 default_withouts = NULL;
 n_default_withouts = 0;
 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        "NONE", "NONE", "WITHOUTS")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_default_withouts, &default_withouts))
  {
   fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
   return 1;
  }
  free(value);
 }

 withouts_from_bc = NULL;
 n_withouts_from_bc = 0;
 if((value = lookup_key(ctx,
                        ctx->build_configuration_contents,
                        ctx->build_configuration_length,
                        host, "NONE", "WITHOUTS")) != NULL)
 {
  if(split_strings(ctx, value, -1, &n_withouts_from_bc, &withouts_from_bc))
  {
   fprintf(stderr, "BCA: split_string() on '%s' failed\n", value);
   return 1;
  }
  free(value);
 }

 *new_withouts = NULL;
 *n_new_withouts = 0;

 if(append_masked_array(default_withouts, n_default_withouts,
                        ctx->with_strings, ctx->n_withs,
                        new_withouts, n_new_withouts, 1))
  return 1;

 if(append_masked_array(withouts_from_bc, n_withouts_from_bc,
                        ctx->with_strings, ctx->n_withs,
                        new_withouts, n_new_withouts, 1))
  return 1;

 if(append_masked_array(ctx->without_strings, ctx->n_withouts,
                        ctx->with_strings, ctx->n_withs,
                        new_withouts, n_new_withouts, 1))
  return 1;

 return 0;
}

int process_dependencies(struct bca_context *ctx, struct host_configuration *tc,
                         char *host, struct file_modification_set *fms)
{
 char **withouts, **opt_ext_depends, **ext_depends, **opt_int_depends,
      **int_depends, **test_packages, **components, **depends, *value;
 int n_withouts, n_opt_ext_depends, n_ext_depends, n_opt_int_depends,
     n_int_depends, n_test_packages, n_components, n_depends, i, j, yes, code;

 if(compute_effective_withouts_for_host(ctx, host, &withouts, &n_withouts))
 {
  fprintf(stderr, "BCA: compute_effective_withouts_for_host(%s) failed\n",
          host);
  return 1;
 }

 /* check to see if we have withouts that are likely typos etc (just for UX) */
 if(list_unique_opt_ext_depends(ctx, &opt_ext_depends, &n_opt_ext_depends, 0))
  return 1;

 for(i=0; i < n_withouts; i++)
 {
  yes = 0;
  j=0;
  while(j<n_opt_ext_depends)
  {
   if(strcmp(withouts[i], opt_ext_depends[j]) == 0)
   {
    yes = 1;
    break;
   }
   j++;
  }

  if(yes == 0)
  {
   fprintf(stderr,
           "BCA: without value \"%s\" is not a optional external dependency in the project\n",
           withouts[i]);
   return 1;
  }
 }

 free_string_array(opt_ext_depends, n_opt_ext_depends);

 /* construct the list of needed dependencies */
 if(list_unique_opt_ext_depends(ctx, &opt_ext_depends, &n_opt_ext_depends, 1))
  return 1;

 if(list_unique_ext_depends(ctx, &ext_depends, &n_ext_depends, 1))
  return 1;

 test_packages = NULL;
 n_test_packages = 0;

 if(append_masked_array(opt_ext_depends, n_opt_ext_depends,
                        withouts, n_withouts,
                        &test_packages, &n_test_packages, 1))
  return 1;

 if(append_masked_array(ext_depends, n_ext_depends,
                        withouts, n_withouts,
                        &test_packages, &n_test_packages, 1))
  return 1;

 /* ux help */


 /* package config exist tests */
 for(i=0; i < n_test_packages; i++)
 {
  code = test_package_exist(ctx, tc, test_packages[i], host);
  if(code < 0)
   return 1;

  if(code == 1)
  {
   yes = 0;

   while(j<n_ext_depends)
   {
    if(strcmp(test_packages[i], ext_depends[j]) == 0)
    {
     yes = 1;
     break;
    }
    j++;
   }

   fprintf(stderr,
           "BCA: Could not find package '%s' for host '%s'.\n",
           test_packages[i], ctx->principle);

   if(yes == 0)
   {
    fprintf(stderr,
            "BCA: Note that since package '%s' is optional in this project you can try to run "
            "configure with the parameter \"--without-%s\".\n",
            test_packages[i], test_packages[i]);
   }
   return 1;
  }
 }

 free_string_array(opt_ext_depends, n_opt_ext_depends);
 free_string_array(ext_depends, n_ext_depends);

 /* .DEPENDS keys in the build configuration */
 if(list_of_project_components(ctx, &components, NULL, &n_components, 1))
  return 1;

 for(i=0; i<n_components; i++)
 {
  depends = NULL;
  n_depends = 0;
  opt_ext_depends = NULL;
  n_opt_ext_depends = 0;
  ext_depends = NULL;
  n_ext_depends = 0;
  int_depends = NULL;
  n_int_depends = 0;
  opt_int_depends = NULL;
  n_opt_int_depends = 0;


  if(list_of_component_internal_dependencies(ctx, components[i],
                                             &int_depends, &n_int_depends))
   return 1;

  if(append_array(int_depends, n_int_depends, &depends, &n_depends, 1))
   return 1;

  free_string_array(int_depends, n_int_depends);

  if(list_of_component_opt_internal_dependencies(ctx, components[i],
                                                 &opt_int_depends, &n_opt_int_depends, 1))
   return 1;

  if(append_array(opt_int_depends, n_opt_int_depends, &depends, &n_depends, 1))
   return 1;

  free_string_array(opt_int_depends, n_opt_int_depends);

  if(list_of_component_external_dependencies(ctx, components[i],
                                             &ext_depends, &n_ext_depends))
   return 1;

  if(append_array(ext_depends, n_ext_depends, &depends, &n_depends, 1))
   return 1;

  free_string_array(ext_depends, n_ext_depends);

  if(list_of_component_opt_external_dependencies(ctx, components[i],
                                                 &opt_ext_depends, &n_opt_ext_depends))
   return 1;

  if(append_masked_array(opt_ext_depends, n_opt_ext_depends,
                         withouts, n_withouts,
                         &depends, &n_depends, 1))
   return 1;

  free_string_array(opt_ext_depends, n_opt_ext_depends);

  value = join_strings(depends, n_depends);

  free_string_array(depends, n_depends);

  if(file_modification_set_append(fms,
                                  host, -1,
                                  components[i], -1,
                                  "DEPENDS", 7,
                                  value, -1))
   return 1;
 }

 /* WITHOUTS persistance */
 if((value = join_strings(withouts, n_withouts)) != NULL)
 {
  free_string_array(withouts, n_withouts);
 }

 if(file_modification_set_append(fms,
                                 host, -1,
                                 "ALL", 3,
                                 "WITHOUTS", 8,
                                 value, -1))
  return 1;

 return 0;
}


int derive_file_suffixes(struct bca_context *ctx,
                         struct host_configuration *tc,
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
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
 {
  /* here we want to overwrite regardless */
  tc->ldflags = strdup(s);
 }

 return 0;
}

int find_multiarch_libpath(struct bca_context *ctx,
                           struct host_configuration *tc,
                           char **path)
{
 char command[1024], *results;
 int length, code;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: find_multiarch_libpath()\n");

 *path = NULL;

 snprintf(command, 1024, "dpkg-architecture");

 if(test_runnable(ctx, command) != 0)
  return 0;

 snprintf(command, 1024, "dpkg-architecture -qDEB_HOST_MULTIARCH > configuretestoutput");

 if(ctx->verbose)
  printf("BCA: about to run \"%s\"\n", command);

 code = system(command);
 if(WEXITSTATUS(code) != 0)
 {
  unlink("configuretestoutput");
  return 0;
 }

 if((results = read_file("configuretestoutput", &length, 0)) == NULL)
 {
  fprintf(stderr, "BCA: read_file(\"configuretestoutput\") failed\n");
  return 0;
 }

 unlink("configuretestoutput");

 if(results[length - 1] == '\n')
  results[--length] = 0;

 *path = results;
 return 0;
}

/* Attempts to find if library paths are in the form of "lib32"
   or "lib64", instead of just "lib" as can be found on Linux
   multi-arch distros for instance. This is done by compiling a
   hello world, running ldd on the output, then simple looking
   through the output. 
   Returns:

   -1 error
   0  inconclusive
   1  lib64
   2  lib32
*/
int find_multilib_suffix(struct bca_context *ctx,
                         struct host_configuration *tc)
{
 int code, length;
 FILE *f;
 char command[1024], *results;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: find_multilib_suffix()\n");

 if((f = fopen("./configuretestfile.c", "w")) == NULL)
 {
  fprintf(stderr, "BCA: fopen(./configuretestfile.c) failed\n");
  return -1;
 }

 fprintf(f,
         "#include <stdio.h>\n"
         "int main(void)\n"
         "{\n"
         " printf(\"hello world\");\n"
         " return 0;\n"
         " }\n\n");
 fclose(f);

 snprintf(command, 1024,
          "%s configuretestfile.c %s configuretestfile 1> configuretestoutput "
          "2> configuretestoutput",
          tc->cc, tc->cc_output_flag);


 if(ctx->verbose)
  printf("BCA: about to run \"%s\"\n", command);

 system(command);

 unlink("configuretestoutput");

 snprintf(command, 1024, "ldd configuretestfile > configuretestoutput");

 if(ctx->verbose)
  printf("BCA: about to run \"%s\"\n", command);

 system(command);

 if((results = read_file("configuretestoutput", &length, 0)) == NULL)
 {
  fprintf(stderr, "BCA: read_file(\"configuretestoutput\") failed\n");
  code = 0;
 } else if(contains_string(results, length, "lib64", 5)) {
  code = 1;
 } else if(contains_string(results, length, "lib32", 5)) {
  code = 2;
 }

 free(results);
 unlink("configuretestoutput");
 unlink("configuretestfile.c");
 unlink("configuretestfile.o");
 unlink("configuretestfile");
 return code;
}

int derive_install_paths(struct bca_context *ctx,
                         struct host_configuration *tc,
                         char *platform,
                         char *host_root)
{
 char install_prefix[512], *arch;

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
  if(host_root == NULL)
  {
   if(find_multiarch_libpath(ctx, tc, &arch))
    return 1;

   if(arch != NULL)
   {
    snprintf(temp, 512, "%s/lib/%s", install_prefix, arch);
    free(arch);
   } else {
    switch(find_multilib_suffix(ctx, tc))
    {
     case 1:
          snprintf(temp, 512, "%s/lib64", install_prefix);
          break;

     case 2:
          snprintf(temp, 512, "%s/lib32", install_prefix);
          break;

     default:
          snprintf(temp, 512, "%s/lib", install_prefix);
    }
   }
  } else {
   snprintf(temp, 512, "%s/lib", install_prefix);
  }
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
  snprintf(temp, 512, "%s/pkgconfig", tc->install_lib_dir);
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

struct disables_and_enables_context
{
 char **disables;
 int n_disables;
 char *host;
};

int check_disabled_swap(struct bca_context *ctx,
                        char *p, char *q, char *k, char *v, void *data)
{
 struct disables_and_enables_context *cb_data;
 int i;

 cb_data = (struct disables_and_enables_context *) data;

 for(i = 0; i < cb_data->n_disables; i++)
 {
  if(strcmp(q, cb_data->disables[i]) == 0)
  {
   if(strcmp(cb_data->host, v) == 0)
   {
    fprintf(stderr,
            "BCA: I can not disable component \"%s\" on host \"%s\", "
            "because it is swapped to from host \"%s\". Remove that swap first.\n",
            cb_data->disables[i], cb_data->host, p);
    return 1;
   }
  }
 }

 return 0;
}

int disables_and_enables(struct bca_context *ctx, char *host,
                         struct file_modification_set *fms)
{
 char *value, **project_components, **disabled_by_default, **disabled_in_bc,
      **effective_disables;
 int n_project_components, n_disabled_by_default, n_disabled_in_bc,
     n_effective_disables;
 struct disables_and_enables_context cb_data;

 /* gather additional needed information */
 if(list_of_project_components(ctx,
                               &project_components, NULL,
                               &n_project_components, 0))
 {
  fprintf(stderr, "BCA: list_of_project_components() failed\n");
  return 1;
 }

 if(lookup_value_as_list(ctx, OPERATE_PROJECT_CONFIGURATION,
                         "NONE", "NONE", "DISABLES",
                         &disabled_by_default, &n_disabled_by_default))
 {
  fprintf(stderr, "BCA: lookup_list(project, NONE, NONE, DISABLES) failed\n");
  return 1;
 }

 if(lookup_value_as_list(ctx, OPERATE_BUILD_CONFIGURATION,
                         host, "ALL", "DISABLES",
                         &disabled_in_bc, &n_disabled_in_bc))
 {
  fprintf(stderr, "BCA: lookup_list(build, ALL, ALL, DISABLES) failed\n");
  return 1;
 }

 effective_disables = NULL;
 n_effective_disables = 0;

 if(append_masked_array(disabled_by_default, n_disabled_by_default,
                        ctx->enabled_components, ctx->n_enables,
                        &effective_disables, &n_effective_disables, 1))
  return 1;

 if(append_masked_array(disabled_in_bc, n_disabled_in_bc,
                        ctx->enabled_components, ctx->n_enables,
                        &effective_disables, &n_effective_disables, 1))
  return 1;

 if(append_masked_array(ctx->disabled_components, ctx->n_disables,
                        ctx->enabled_components, ctx->n_enables,
                        &effective_disables, &n_effective_disables, 1))
  return 1;


 /* Now we want to make sure than none of disabled componts are being
    swapped to from other host.*/
 cb_data.disables = effective_disables;
 cb_data.n_disables = n_effective_disables;
 cb_data.host = host;
 if(iterate_over_values(ctx, OPERATE_BUILD_CONFIGURATION, &cb_data,
                        NULL, NULL, "SWAP", check_disabled_swap))
  return 1;


 /* finish me - ux test for typos with --enable-* and --disable-*/


 /* DISABLED persistance */
 value = join_strings(effective_disables, n_effective_disables);

 if(file_modification_set_append(fms,
                                 host, -1,
                                 "ALL", 3,
                                 "DISABLES", 8,
                                 value, -1))
  return 1;

 if(value != NULL)
  free(value);

 free_string_array(ctx->disabled_components, ctx->n_disables);
 ctx->disabled_components = effective_disables;
 ctx->n_disables = n_effective_disables;

 free_string_array(disabled_by_default, n_disabled_by_default);
 free_string_array(disabled_in_bc, n_disabled_in_bc);
 free_string_array(project_components, n_project_components);

 return 0;
}

int persist_host_swap_configuration(struct bca_context *ctx,
                                    struct file_modification_set *fms,
                                    char *host)
{
 char o_principle[256], o_component[256], o_key[256], *o_value;
 int *handled, i, allocation_size, end, yes;

 allocation_size = sizeof(int) * (ctx->n_swaps + 1);
 if((handled = (int *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }
 memset(handled, 0, allocation_size);

 end = -1;

 while(iterate_key_primitives(ctx, ctx->build_configuration_contents,
                              ctx->build_configuration_length, &end,
                              host, NULL, "SWAP",
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
      if(file_modification_set_append(fms,
                                      host, -1,
                                      o_component, -1,
                                      "SWAP", 4,
                                      ctx->swapped_component_hosts[i], -1))
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
   if(file_modification_set_append(fms,
                                   host, -1,
                                   o_component, -1,
                                   "SWAP", 4,
                                   NULL, -1))
   return 1;
  }

  free(o_value);
 }

 for(i=0; i < ctx->n_swaps; i++)
 {
  if(handled[i] == 0)
  {
   /* case 4: add a swap value */
   if(file_modification_set_append(fms,
                                   host, -1,
                                   ctx->swapped_components[i], -1,
                                   "SWAP", 4,
                                   ctx->swapped_component_hosts[i], -1))
    return 1;
  }
 }

 free(handled);
 return 0;
}

int append_host_configuration(struct bca_context *ctx, char *host,
                              struct host_configuration *tc,
                              struct file_modification_set *fms)
{
 int i, p_length;
 FILE *output;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: append_host_configuration()\n");

 char *host_updates[(28 * 2) + 1] =

 { "CC", tc->cc,
   "CXX", tc->cxx,
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
   "CCFLAGS", tc->ccflags,
   "CXXFLAGS", tc->cxxflags,
   "LDFLAGS", tc->ldflags,
   "INSTALL_PREFIX", tc->install_prefix,
   "INSTALL_BIN_DIR", tc->install_bin_dir,
   "INSTALL_LIB_DIR", tc->install_lib_dir,
   "INSTALL_INCLUDE_DIR", tc->install_include_dir,
   "INSTALL_PKG_CONFIG_DIR", tc->install_pkg_config_dir,
   "INSTALL_LOCALE_DATA_DIR", tc->install_locale_data_dir,
   "ERLC", tc->erlc,
   "ERLCFLAGS", tc->erlc_flags,
   "ERLC_OUTPUT_DIR_FLAG", tc->erlc_output_dir_flag
 };

 p_length = strlen(host);

 for(i=0; i < (28 * 2); i += 2)
 {
  if(file_modification_set_append(fms,
                                  host, -1,
                                  "ALL", 3,
                                  host_updates[i], -1,
                                  host_updates[i + 1], -1))
  return 1;
 }

 if(ctx->verbose > 0)
 {
  fprintf(stderr, "BCA: About to modify the build configuration with:\n");
  file_modification_set_print(fms, stderr);
 }

 if((output = fopen("./buildconfiguration/buildconfiguration", "w")) == NULL)
 {
  fprintf(stderr, "BCA: failed to open output file ./buildconfiguration/buildconfiguration\n");
  return 1;
 }

 if(file_modification_set_apply(ctx, output,
                                ctx->build_configuration_contents,
                                ctx->build_configuration_length,
                                fms))
 {
  fprintf(stderr, "BCA: apply_modification_set() failed\n");
  return 1;
 }

 fclose(output);
 return 0;
}

int configure(struct bca_context *ctx)
{
 char *s, *host;
 struct file_modification_set *fms;
 struct host_configuration *tc;
 char *platform = "", *host_root;
 char **source_files_in_use = NULL, **source_file_extensions;
 int n_source_files_in_use = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: configure()\n");

 if(ctx->verbose == 0)
  fprintf(stdout, "BCA: configure() use -v to increase verbosity\n");

 host = ctx->principle;

 if((fms = file_modification_set_init()) == NULL)
  return 1;

 if(load_project_config(ctx, 0))
  return 1;

 if(load_build_config(ctx, 1) == 0)
 {
  /* component is NULL here because configure-time does not have a notion
     of component specific build environment details. Makefile generate time
     however does. This is realized by user edits to the buildconfiguration. */
  if((tc = resolve_host_configuration(ctx, host, NULL)) == NULL)
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
     snprintf(temp, 512, "./%s", host);
    }
   } else {
    s = build_prefix_from_host_prefix(ctx);
    snprintf(temp, 512, "%s", s);
    free(s);
   }
   tc->build_prefix = strdup(temp);
  }
 }

 if(derive_file_suffixes(ctx, tc, platform))
  return 1;


 /* we need to find out what is enabled next so as to be able
    to skip configure logic for parts not enabled */
 if(disables_and_enables(ctx, host, fms))
  return 1;

 if(swap_checks(ctx))
  return 1;

 if(assemble_list_of_used_source_files(ctx,
                                       &source_files_in_use,
                                       &source_file_extensions,
                                       &n_source_files_in_use))
  return 1;

 if(c_family_configuration(ctx, tc, host,
                           source_files_in_use,
                           source_file_extensions,
                           n_source_files_in_use))
  return 1;

 if(erlang_family_configuration(ctx, tc, host,
                                source_files_in_use,
                                source_file_extensions,
                                n_source_files_in_use))
  return 1;

 if(free_string_array(source_files_in_use, n_source_files_in_use))
  return 1;

 if(free_string_array(source_file_extensions, n_source_files_in_use))
  return 1;

 source_files_in_use = NULL;
 source_file_extensions = NULL;
 n_source_files_in_use = 0;

 if(pkg_config_tests(ctx, tc))
  return 1;

 if(process_dependencies(ctx, tc, host, fms))
  return 1;

 if(persist_host_swap_configuration(ctx, fms, host))
  return 1;

 if(derive_install_paths(ctx, tc, platform, host_root))
  return 1;

 if(append_host_configuration(ctx, host, tc, fms))
  return 1;

 if(file_modification_set_free(fms))
  return 1;

 return 0;
}
