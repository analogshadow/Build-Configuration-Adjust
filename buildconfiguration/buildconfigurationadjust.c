#define IN_SINGLE_FILE_DISTRIBUTION
#define WITHOUT_LIBNEWT
#define WITHOUT_MONGOOSE
#define WITHOUT_GTK__2_0
#define BCA_MAJOR "0"
#define BCA_MINOR "2"

/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2011-2014 Stover Enterprises, LLC
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


#ifndef _prototypes_h_
#define _prototypes_h_

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef HAVE_CWD
#include <unistd.h>
#endif

#define NEW_PROJECT_MODE 2
#define REMOVE_VALUE_MODE 4
#define SHOW_VALUE_MODE 5
#define SET_VALUE_MODE 6
#define ADD_VALUE_MODE 7
#define PULL_VALUE_MODE 8
#define CHECK_VALUE_MODE 9
#define SMART_ADD_VALUE_MODE 10
#define SMART_PULL_VALUE_MODE 11
#define STRING_REPLACE_MODE 15
#define SHORT_HELP_MODE 19
#define NEW_COMPONENT_MODE 20
#define CONFIGURE_MODE 30
#define GENERATE_GMAKEFILE_MODE 40
#define CONCATENATE_MODE 50
#define GENERATE_GRAPHVIZ_MODE 60
#define NEWT_INTERFACE_MODE 70
#define MONGOOSE_INTERFACE_MODE 71
#define GTK_INTERFACE_MODE 72
#define LIST_HOSTS_MODE 80
#define LIST_PROJECT_TYPES_MODE 81
#define LIST_PLATFORM_DETIALS_MODE 82
#define LIST_PROJECT_COMPONENTS_MODE 83
#define LIST_COMPONENT_OUTPUT_NAMES_MODE 84
#define LIST_COMPONENT_BUILD_OUTPUT_NAMES_MODE 85
#define LIST_COMPONENT_INSTALL_OUTPUT_NAMES_MODE 86
#define LIST_COMPONENT_EFFECTIVE_OUTPUT_NAMES_MODE 87
#define SELF_TEST_MODE 99

#define MANIPULATE_PROJECT_CONFIGURATION   3
#define MANIPULATE_BUILD_CONFIGURATION     4

#define EFFECTIVE_PATHS_LOCAL   6
#define EFFECTIVE_PATHS_INSTALL 7

struct bca_context
{
 int mode, extra_args_start, manipulation_type, verbose,
     n_withouts, n_disables, n_enables, n_swaps;
 char *search_value_key, *new_value_string;
 char value_key[1024], value_string[1024];
 char *principle, *qualifier;

 char *install_prefix, *host_prefix, *build_prefix;
 char **without_strings, **disabled_components, **enabled_components,
      **swapped_components, **swapped_component_hosts;

#ifdef HAVE_CWD
 char *cwd;
#endif

 char *project_name;
 char *build_configuration_contents;
 char *project_configuration_contents;
 int build_configuration_length;
 int project_configuration_length;
};

struct component_details
{
 char *host;
 char *project_component;
 char *project_component_type;
 char *project_component_output_name;

 char **file_names;
 char **file_base_names;
 char **file_extensions;
 int n_file_names;

 char **inputs;
 int n_inputs;

 char **file_deps;
 int n_file_deps;

 char **lib_headers;
 int n_lib_headers;

 char **dependencies;
 int n_dependencies;

 char **include_dirs;
 int n_include_dirs;

 char **project_components;
 char **project_component_types;
 char **project_output_names;
 int n_components;

 char *major;
 char *minor;

 char **withouts;
 int n_withouts;
};

struct host_configuration
{
 /* tools */
 char *cc;                           /* C compiler */
 char *cxx;                          /* C++ compiler */
 char *python;                       /* python */
 char *erlc;                         /* erlang compiler */
 char *pkg_config;                   /* pkg-config */
 char *xgettext, *msgmerge, *msgfmt; /* gettext */

 /* flags for tools */
 char *cppflags;                           /* C & C++ preprocessor flags */
 char *cflags;                             /* C compiler flags */
 char *cxxflags;                           /* C++ compiler flags */
 char *ldflags;                            /* C & C++ linker flags */
 char *cc_output_flag;                     /* C & C++ specify output filename flag (ie -o) */
 char *cc_compile_bin_obj_flag;            /* C & C++ compile object flag (ie -c) */
 char *cc_compile_shared_library_obj_flag; /* C & C++ compile shared object flag (ie -c -fpic) */
 char *cc_include_dir_flag;                /* C & C++ add include dir flag (ie -I) */
 char *cc_define_macro_flag;               /* C & C++ define preprocessor macro (ie -D) */
 char *erlc_flags;                         /* erlang compiler flags */
 char *python_flags;                       /* python flags */

 /* output file characteristics */
 char *binary_suffix;
 char *shared_library_prefix, *shared_library_suffix;
 char *obj_suffix;

 /* input directories */
 char *pkg_config_path, *pkg_config_libdir;

 /* output directories */
 char *build_prefix;           /* prefix for run from local */
 char *install_prefix;         /* autoconf's prefix */
 char *install_bin_dir;        /* autoconf's bindir */
 char *install_lib_dir;        /* autoconf's libdir */
 char *install_include_dir;    /* autoconf's includedir */
 char *install_pkg_config_dir;
 char *install_locale_data_dir;
};

/* selftest.c ----------------------------------- */
int self_test(struct bca_context *ctx);

/* replace.c ------------------------------------ */
int string_replace(struct bca_context *ctx);

/* conversions.c -------------------------------- */
char *lib_file_name_to_link_name(const char *file_name);

char *without_string_to_without_macro(struct bca_context *ctx, char *in);

int render_project_component_output_name(struct bca_context *ctx,
                                         char *host, char *component, int edition,
                                         char ***array_ptr, char ***extensions);

char *host_identifier_from_host_prefix(struct bca_context *ctx);

char *build_prefix_from_host_prefix(struct bca_context *ctx);

char *component_type_file_extension(struct bca_context *ctx, struct host_configuration *tc,
                                    char *project_component_type,
                                    char *project_component_output_name);

/* main.c --------------------------------------- */
struct bca_context *setup(int argc, char **argv);  // selftested

int shutdown(struct bca_context *ctx);  // selftested

void help(void);

int short_help_mode(struct bca_context *ctx);

int concatenate(struct bca_context *ctx, int argc, char **argv);

/* strings.c ------------------------------------ */
int contains_string(char *source, int source_length, char *search, int search_length);  // selftested

int add_to_string_array(char ***array, int array_size,
                        char *string, int string_length,
                        int prevent_duplicates);  // selftested

int free_string_array(char **array, int n_elements);  // selftested

int path_extract(const char *full_path, char **base_file_name, char **extension);  // selftested

char *read_file(char *name, int *length, int silent_test);  // selftested

int find_line(char *buffer, int buffer_length, int *start, int *end, int *line_length); // selftested

int split_strings(struct bca_context *ctx, char *source, int length,
                  int *count, char ***strings);  // selftested

char *escape_value(struct bca_context *ctx, char *source, int length);  // selftested

/* config_files.c ------------------------------- */
int iterate_key_primitives(struct bca_context *ctx, char *file, int file_length, int *offset,
                           char *principle_filter, char *component_filter, char *key_filter,
                           char principle[256], char component[256], char key[256],
                           int *equals_pos);    // selftested

char *lookup_key(struct bca_context *ctx, char *file, int file_length,
                 char *principle_filter, char *component_filter, char *key_filter);   // selftested

int list_unique_principles(struct bca_context *ctx, char *qualifier,
                           char *contents, int length,
                           char ***principle_list, int *n_principles);   // selftested

int output_modifications(struct bca_context *ctx, FILE *output,
                         char *contents, int length, int n_records,
                         char **principle, char **component, char **key, char **value); // selftested

int output_modification(struct bca_context *ctx, FILE *output,
                        char *contents, int length,
                        char *principle, char *component, char *key, char *value);

int modify_file(struct bca_context *ctx, char *filename,
                char *principle, char *component, char *key, char *value);

int add_value(struct bca_context *ctx);

int pull_value(struct bca_context *ctx);

int check_value_inline(struct bca_context *ctx,
                       char *contents, int length,
                       char *principle, char *qualifier,
                       char *key, char *check_value);

int check_value(struct bca_context *ctx);

int smart_add_value(struct bca_context *ctx);

int smart_pull_value(struct bca_context *ctx);

int engage_build_configuration_disables_for_host(struct bca_context *ctx, char *host);

int engage_build_configuration_swaps_for_host(struct bca_context *ctx, char *host);

int check_project_component_types(struct bca_context *ctx);

int list_project_components(struct bca_context *ctx,
                            struct component_details *cd);

int list_component_internal_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements);

int list_component_external_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements);

int list_component_opt_external_dependencies(struct bca_context *ctx,
                                             struct component_details *cd,
                                             char ***list, int *n_elements);

int resolve_project_name(struct bca_context *ctx);  // selftested

int resolve_component_dependencies(struct bca_context *ctx,
                                   struct component_details *cd);

int resolve_component_version(struct bca_context *ctx,
                              char *contents, int contents_length,
                              struct component_details *cd,
                              char *component_type,
                              char *project_component);

int resolve_component_file_dependencies(struct bca_context *ctx,
                                        struct component_details *cd,
                                        int component_index);

int resolve_component_input_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         int component_index);

char *resolve_build_host_variable(struct bca_context *ctx,
                                  char *host,
                                  char *project_component,
                                  char *key);

struct host_configuration *
resolve_host_build_configuration(struct bca_context *ctx, struct component_details *cd);

struct host_configuration *
resolve_host_configuration(struct bca_context *ctx, struct component_details *cd);

int free_host_configuration(struct bca_context *ctx, struct host_configuration *tc);

int resolve_effective_path_mode(struct bca_context *ctx);

int resolve_component_installation_path(struct bca_context *ctx,
                                        char *host,
                                        char *component_type,
                                        char *component,
                                        char **path);

int check_duplicate_output_names(struct bca_context *ctx, struct component_details *cd);

int is_project_using_config_h(struct bca_context *ctx);

/* configure.c ---------------------------------- */
int is_c_compiler_needed(struct bca_context *ctx,
                         struct component_details *cd);

int is_pkg_config_needed(struct bca_context *ctx,
                         struct component_details *cd);

int test_package_exist(struct bca_context *ctx,
                       struct component_details *cd,
                       struct host_configuration *tc,
                       char *package, int optional);

int test_runnable(struct bca_context *ctx, char *command);

int configure(struct bca_context *ctx);

int append_host_configuration(struct bca_context *ctx,
                              struct host_configuration *tc,
                              struct component_details *cd,
                              int *n_modify_records,
                              char ***mod_principles,
                              char ***mod_components,
                              char ***mod_keys,
                              char ***mod_values);

/* gmakefile.c ---------------------------------- */
int generate_gmakefile_mode(struct bca_context *ctx);

int generate_gmake_host_components(struct bca_context *ctx, FILE *output,
                                   char **hosts, int n_hosts,
                                   struct component_details *cd);

int generate_gmake_clean_rules(struct bca_context *ctx, FILE *output,
                               char **hosts, int n_hosts,
                               struct component_details *cd);

int generate_gmake_host_component_file_rules(struct bca_context *ctx, FILE *output,
                                             struct component_details *cd);

int generate_gmake_install_rules(struct bca_context *ctx, FILE *output,
                                 char **hosts, int n_build_hosts,
                                 struct component_details *cd,
                                 int uninstall_version);

/* graphviz.c ----------------------------------- */
int graphviz_edges(struct bca_context *ctx, FILE *output,
                   struct component_details *cd);

int graphviz_nodes(struct bca_context *ctx, FILE *output,
                   char **hosts, int n_build_hosts,
                   struct component_details *cd);

int generate_graphviz_mode(struct bca_context *ctx);

int graphviz_string_clean(struct bca_context *ctx,
                          char *input, int length,
                          char *output, int size);

/* newt.c --------------------------------------- */
#ifndef WITHOUT_LIBNEWT
int newt_interface(struct bca_context *ctx);
#endif

/* gtk.c ---------------------------------------- */
#ifdef HAVE_GTK
int gtk_interface(struct bca_context *ctx);
#endif

#endif


/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012,2013,2014 Stover Enterprises, LLC
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

char *lookup_key(struct bca_context *ctx, char *file, int file_length,
                 char *principle_filter, char *component_filter, char *key_filter)
{
 char principle[256], component[256], key[256];
 char *value = NULL;
 int start, index, equals, in_quotes, end, value_length, offset;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: lookup_key()\n");

 offset = -1;
 if(iterate_key_primitives(ctx, file, file_length, &offset,
                           principle_filter, component_filter, key_filter,
                           principle, component, key, &equals) == 0)
  return NULL;

 end = offset;
 index = equals + 1;
 in_quotes = 0;
 start = -1;
 while(index < offset)
 {
  if(start == -1)
  {
   if(file[index] != ' ')
    start = index;
  }

  if(start > -1)
  {
   if(in_quotes == 0)
   {
    if(file[index] == '"')
    {
     in_quotes = 1;
     if(index + 1 < end)
     {
      start = ++index;
      continue;
     } else {
      file[end] = 0;
      fprintf(stderr, "BCA: quotation error (1) in value of '%s'\n", file + equals + 1);
      exit(1);
      return NULL;
     }
    }
   }

   if(in_quotes == 1)
   {
    if(file[index] == '"')
    {
     if( (start != index - 1) && (file[index-1] == '\\') )
     {
      /* escape out double quotation marks in values with backslash */
      index++;
      continue;
     }

     end = index++;
     in_quotes = 0;
     break;
    }
   }

  }
  index++;
 }

 if(in_quotes == 1)
 {
  fprintf(stderr, "BCA: quotation error (3) in value of '%s'\n", file + equals + 1);
  exit(1);
  return NULL;
 }

 if(start < 0)
  value_length = 0;
 else
  value_length = end - start;

 if((value = malloc(value_length + 1)) == NULL)
 {
  perror("BCA: malloc");
  exit(1);
  return NULL;
 }

 memcpy(value, file + start, value_length);
 value[value_length] = 0;

 /* escape out " marks in values with \" */
 index=0;
 while(index < (value_length - 1))
 {
  if( (value[index] == '\\') &&
      (value[index+1] == '"') )
  {
   memmove(value + index, value + index + 1, --value_length - index );
   value[value_length] = 0;
  } else {
   index++;
  }
 }

 return value;
}

int output_modifications(struct bca_context *ctx, FILE *output,
                         char *contents, int length, int n_records,
                         char **principle, char **component, char **key, char **value)
{
 char o_principle[256], o_component[256], o_key[256], *o_value, *output_value, *temp;
 int end = -1, action, *handled, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: output_modification()\n");

 if((handled = (int *) malloc(sizeof(int) * (n_records + 1))) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%u) failed\n",
          (unsigned int) (sizeof(int) * (n_records + 1)));
  return 1;
 }

 for(i=0; i<n_records; i++)
 {
  handled[i] = 0;
 }

 while(iterate_key_primitives(ctx, contents, length, &end,
                              NULL, NULL, NULL,
                              o_principle, o_component, o_key, NULL))
 {
  o_value = lookup_key(ctx, contents, length, o_principle, o_component, o_key);

  action = 1;

  i = 0;
  while(i < n_records)
  {
   if(strcmp(o_principle, principle[i]) == 0)
   {
    if(strcmp(o_component, component[i]) == 0)
    {
     if(strcmp(o_key, key[i]) == 0)
     {
      handled[i] = 1;
      action = 2;
      if(value[i] == NULL)
       action = 3;

      break;
     }
    }
   }
   i++;
  }

  switch(action)
  {
   case 1:  /* copy original */
        fprintf(output, "%s.%s.%s = ", o_principle, o_component, o_key);
        temp = o_value;
 
        if((output_value = escape_value(ctx, temp, -1)) == NULL)
        {
         /* bail */
         free(handled);
         return 1;
        }
        fprintf(output, "%s\n", output_value);

        if(ctx->verbose > 3) 
         fprintf(stderr, "BCA: keeping record not in modify array %s.%s.%s = %s\n",
                 o_principle, o_component, o_key, output_value);

        if(output_value != temp)
         free(output_value);
        break;

   case 2:  /* replace with updated value case */
        fprintf(output, "%s.%s.%s = ", principle[i], component[i], key[i]);
        temp = value[i];

        if((output_value = escape_value(ctx, temp, -1)) == NULL)
        {
         /* bail */
         free(handled);
         return 1;
        }
        fprintf(output, "%s\n", output_value);

        if(ctx->verbose > 3) 
         fprintf(stderr, "BCA: updating record %s.%s.%s = %s\n",
                 principle[i], component[i], key[i], output_value);

        if(output_value != temp)
         free(output_value);
        break;

   case 3:  /* leave out this record */
        if(ctx->verbose > 3)
         fprintf(stderr, "BCA: dropping set-NULL value record %s.%s.%s\n",
                 principle[i], component[i], key[i]);
        break;

  }

  free(o_value);
 }

 /* append cases */
 for(i=0; i<n_records; i++)
 {
  if( (handled[i] == 0) && (value[i] != NULL) )
  {
   if((output_value = escape_value(ctx, value[i], -1)) == NULL)
   {
    /* bail */
    free(handled);
    return 1;
   }

   if(ctx->verbose > 3)
    fprintf(stderr, "BCA: appending record %s.%s.%s = %s\n",
            principle[i], component[i], key[i], output_value);

   fprintf(output, "%s.%s.%s = %s\n", principle[i], component[i], key[i], output_value);

   if(output_value != value[i])
    free(output_value);
  }
 }

 free(handled);
 return 0;
}

int output_modification(struct bca_context *ctx, FILE *output,
                        char *contents, int length,
                        char *principle, char *component, char *key, char *value)
{
 return output_modifications(ctx, output, contents, length, 1,
                             &principle, &component, &key, &value);
}


int modify_file(struct bca_context *ctx, char *filename,
                char *principle, char *component, char *key, char *value)
{
 FILE *output;
 char *contents;
 int length;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: modify_file()\n");

 if((contents = read_file(filename, &length, 0)) == NULL)
 {
  fprintf(stderr, "BCA: modify_file(): read_file() failed\n");
  return 1;
 }

 if((output = fopen(filename, "w")) == NULL)
 {
  perror("BCA: modify_file()\n");
  return 1;
 }

 if(output_modification(ctx, output, contents, length, principle, component, key, value))
 {
  fprintf(stderr, "BCA: modify_file(): output_modifications() failed\n");
  fclose(output);
  return 1;
 }

 fclose(output);

 return 0;
}

int iterate_key_primitives(struct bca_context *ctx, char *file, int file_length, int *offset,
                           char *principle_filter, char *component_filter, char *key_filter,
                           char principle[256], char component[256], char key[256],
                           int *equals_pos)
{
 int start, end, line_length = 0, n_periods, periods[2], index, equals = -1, no,
     principle_length, component_length, key_length, principle_filter_length,
     component_filter_length, key_filter_length;

 if(ctx->verbose > 3)
  fprintf(stderr, "BCA: iterate_key_primitives()\n");

 if(principle_filter == NULL)
 {
  principle_filter = "*";
  principle_filter_length = 1;
 } else
  principle_filter_length = strlen(principle_filter);

 if(component_filter == NULL)
 {
  component_filter = "*";
  component_filter_length = 1;
 } else
  component_filter_length = strlen(component_filter);

 if(key_filter == NULL)
 {
  key_filter = "*";
  key_filter_length = 1;
 } else
  key_filter_length = strlen(key_filter);

 end = *offset;
 while(find_line(file, file_length, &start, &end, &line_length))
 {
  *offset = end;
  index = start;
  n_periods = 0;
  while(index < end)
  {
   if(file[index] == '.')
   {
    if(n_periods > 1)
    {
     file[end] = 0; /* destructive but we are aborting anyway */
     fprintf(stderr, "BCA: more than 2 periods on the left of '=' on the line \"%s\"\n",
             file + start);
     exit(1);

    }

    periods[n_periods++] = index;
   }

   if(file[index] == '=')
   {
    equals = index;
    no = 0;

    if(n_periods != 2)
     no = 1;

    if((principle_length = (periods[0] - start)) < 1)
     no = 1;

    if(principle_length > 255)
     no = 1;

    if((component_length = (periods[1] - (periods[0] + 1))) < 1)
     no = 1;

    if(component_length > 255)
     no = 1;

    if((key_length = (equals - (periods[1] + 1))) < 1)
     no = 1;

    while(key_length > 1)
    {
     if(file[periods[1] + key_length] == ' ') {
      key_length--;
     } else
      break;
    }

    if(key_length > 255)
     no = 1;

    if(no)
    {
     file[end] = 0;
     fprintf(stderr,
             "BCA: left of '=' not in the format PRINCIPLE.QUALIFIER.KEY on the line \"%s\"\n",
             file + start);
     exit(1);
    }

    index++;
    break;
   }

   index++;
  }

  if(equals == -1)
  {
   file[end] = 0;
   fprintf(stderr, "BCA: no '=' on the line \"%s\"\n", file + start);
   exit(1);
  }

  if(principle_filter[0] != '*')
  {
   if(principle_length != principle_filter_length)
    continue;

   if(strncmp(principle_filter, file + start, principle_length) != 0)
    continue;
  }

  memcpy(principle, file + start, principle_length);
  principle[principle_length] = 0;

  if(component_filter[0] != '*')
  {
   if(component_length != component_filter_length)
    continue;
   if(strncmp(component_filter, file + periods[0] + 1, component_length) != 0)
    continue;
  }

  memcpy(component, file + periods[0] + 1, component_length);
  component[component_length] = 0;

  if(key_filter[0] != '*')
  {
   if(key_length != key_filter_length)
    continue;
   if(strncmp(key_filter, file + periods[1] + 1, key_length) != 0)
    continue;
  }

  memcpy(key, file + periods[1] + 1, key_length);
  key[key_length] = 0;

  if(equals_pos != NULL)
   *equals_pos = equals;

  return 1;
 }

 return 0;
}

int list_component_internal_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         char ***list, int *n_elements)
{
 char *value = NULL;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: list_component_internal_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "INT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No internal dependencies found for component \"%s\".\n",
          cd->project_component);

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
  fprintf(stderr, "BCA: list_component_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No external dependencies found for component \"%s\".\n",
          cd->project_component);

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
  fprintf(stderr, "BCA: list_component_opt_external_dependencies()\n");

 if((value = lookup_key(ctx,
                        ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_type,
                        cd->project_component,
                        "OPT_EXT_DEPENDS")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: No optional external dependencies found for component \"%s\".\n",
          cd->project_component);

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

int check_duplicate_output_names(struct bca_context *ctx, struct component_details *cd)
{
 char **output_names = NULL;
 int n_output_names = 0, x;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: check_duplicate_output_names()\n");

 for(x=0; x<cd->n_components; x++)
 {
  if(add_to_string_array(&output_names, n_output_names,
                         cd->project_output_names[x], -1, 1))
  {
   fprintf(stderr,
           "BCA: The component output name \"%s\" is used more than once.\n",
           cd->project_output_names[x]);

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
 int host_length, n_hosts = 0, i, ok, end = -1;

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

 host_length = strlen(host);

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

 char *component_types[9] = { "NONE", "BINARY", "SHAREDLIBRARY", "STATICLIBRARY", "CAT",
                              "MACROEXPAND", "PYTHONMODULE", "CUSTOM" };

 char *component_keys[20] = { "PROJECT_NAME", "NAME", "MAJOR", "MINOR", "AUTHOR", "EMAIL",
                              "URL", "FILES", "INPUT", "DRIVER", "INCLUDE_DIRS",
                              "FILE_DEPENDS", "INT_DEPENDS", "EXT_DEPENDS", "OPT_EXT_DEPENDS",
                              "LIB_HEADERS", "DISABLES", "DESCRIPTION", "PACKAGE_NAME" };

 while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                              ctx->project_configuration_length, &offset,
                              NULL, NULL, NULL,
                              type, component, key, NULL))
 {
  handled = 0;
  i=0;
  while(i<8)
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
  while(i<19)
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

int list_project_components(struct bca_context *ctx,
                            struct component_details *cd)
{
 int pass = 0, allocation_size, string_length, offset, i, disabled, ok;
 char *string, type[256], component[256], key[256];

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_project_components()\n");

 while(pass < 2)
 {
  offset = -1;
  cd->n_components = 0;
  while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                               ctx->project_configuration_length, &offset,
                               NULL, NULL, "NAME",
                               type, component, key, NULL))
  {
   i = 0;
   disabled = 0;
   while(i < ctx->n_disables)
   {
    if(strcmp(component, ctx->disabled_components[i]) == 0)
    {
     disabled = 1;
     break;
    }
    i++;
   }

   if(pass == 0)
   {
    /* validate and count */
    if(disabled == 0)
    {
     ok = 1;

     if(strcmp(component, "ALL") == 0)
      ok = 0;

     if(strcmp(component, "NONE") == 0)
      ok = 0;

     if(ok == 0)
     {
      fprintf(stderr,
              "BCA: component name is a reserved word, %s.%s.%s\n",
              type, component, key);
      return 1;
     }

     cd->n_components++;
    }
   }

   if( (pass == 1) && (disabled == 0) )
   {
    string_length = strlen(component);
    allocation_size = string_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: malloc(%d) failed 1\n", allocation_size);
//free array
     return 1;
    }

    snprintf(string, allocation_size, "%s", component);
    cd->project_components[cd->n_components] = string;


    string_length = strlen(type);
    allocation_size = string_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: malloc(%d) failed 2\n", allocation_size);
//free array
     return 1;
    }

    snprintf(string, allocation_size, "%s", type);
    cd->project_component_types[cd->n_components] = string;


    if((cd->project_output_names[cd->n_components] =
        lookup_key(ctx, ctx->project_configuration_contents,
                   ctx->project_configuration_length, type, component, "NAME")) == NULL)
    {
     fprintf(stderr, "BCA: lookup_key() failed 3\n");
     return 1;
    }

    cd->n_components++;
   }

  }

  if(pass == 0)
  {
   if(cd->n_components == 0)
   {
    cd->project_components = NULL;
    cd->project_component_types = NULL;
    cd->project_output_names = NULL;
    return 0;
   }

   /* allocate array of pointers */
   allocation_size = cd->n_components * sizeof(char *);
   if((cd->project_components = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 4\n", allocation_size);
    return 1;
   }

   if((cd->project_component_types = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 5\n", allocation_size);
    free(cd->project_components);
    return 1;
   }

   if((cd->project_output_names = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed 6\n", allocation_size);
    free(cd->project_components);
    free(cd->project_component_types);
    return 1;
   }

  }

  pass++;
 }

 return 0;
}


int list_unique_principles(struct bca_context *ctx, char *search_qualifier,
                           char *contents, int length,
                           char ***principle_list, int *n_principles)
{
 char principle[256], qualifier[256], key[256], **principles = NULL, **new_ptr, *string;
 int offset, matched, x, allocation_size;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: list_unique_principles()\n");

 *n_principles = 0;

 offset = -1;
 while(iterate_key_primitives(ctx, contents, length, &offset,
                              NULL, search_qualifier, NULL,
                              principle, qualifier, key, NULL))
 {
  matched = 0;
  x = 0;

  if(strcmp(principle, "ALL") == 0)
   matched = 1;

  if(strcmp(principle, "NONE") == 0)
   matched = 1;

  if(matched == 0)
  {
   while(x < *n_principles)
   {
    if(strcmp(principles[x], principle) == 0)
    {
     matched = 1;
     break;
    }

    x++;
   }
  }

  if(matched == 0)
  {
   (*n_principles)++;
   allocation_size = *n_principles * sizeof(char *);

   if((new_ptr = (char **) realloc(principles, allocation_size)) == NULL)
   {
    perror("BCA: list_unique_principles(): realloc()");
    return 1;
   }

   principles = new_ptr;

   allocation_size = strlen(principle) + 1;
   if((string = (char *) malloc(allocation_size)) == NULL)
   {
    perror("BCA: list_unique_principles(): malloc()");
    return 1;
   }

   snprintf(string, allocation_size, "%s", principle);
   principles[*n_principles - 1] = string;
  }

 }

 *principle_list = principles;
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
resolve_host_configuration(struct bca_context *ctx, struct component_details *cd)
{
 int allocation_size, i;
 struct host_configuration *tc;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: resolve_host_configuration()\n");
  fflush(stderr);
 }

 allocation_size = sizeof(struct host_configuration);
 if((tc = (struct host_configuration *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }
 memset(tc, 0, allocation_size);

 char **host_resolve_vars[24] =
 {
  &(tc->build_prefix),
  &(tc->cc),
  &(tc->cc_output_flag),
  &(tc->cc_compile_bin_obj_flag),
  &(tc->cc_compile_shared_library_obj_flag),
  &(tc->cc_include_dir_flag),
  &(tc->cc_define_macro_flag),
  &(tc->cflags),
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
  &(tc->python)
 };

 char *host_resolve_keys[24] =
 {
  "BUILD_PREFIX",
  "CC",
  "CC_SPECIFY_OUTPUT_FLAG",
  "CC_COMPILE_BIN_OBJ_FLAG",
  "CC_COMPILE_SHARED_LIBRARY_OBJ_FLAG",
  "CC_INCLUDE_DIR_FLAG",
  "CC_DEFINE_MACRO_FLAG",
  "CFLAGS",
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
  "PYTHON"
 };

 for(i=0; i<23; i++)
 {
  *(host_resolve_vars[i]) = resolve_build_host_variable(ctx, cd->host,
                                                        cd->project_component,
                                                        host_resolve_keys[i]);
 }

 if(ctx->verbose > 2)
 {
  for(i=0; i<23; i++)
  {
   printf("BCA: %s.%s.%s resolves to %s\n",
          cd->host, cd->project_component,  host_resolve_keys[i], *(host_resolve_vars[i]));
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

  if(tc->cflags != NULL)
   free(tc->cflags);

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
                     cd->host, cd->project_component, "DEPENDS");

 if(value == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: No DEPENDS key found for component \"%s\" "
           "on host \"%s\", implying dependencies.\n",
            cd->project_component, cd->host);

  cd->dependencies = NULL;
  cd->n_dependencies = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1, &(cd->n_dependencies), &(cd->dependencies)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: Found the following dependencies for component \"%s\" on host \"%s\" (%d): ",
         cd->project_component, cd->host, cd->n_dependencies);

  for(i=0; i < cd->n_dependencies; i++)
  {
   printf("%s ", cd->dependencies[i]);
  }
  printf("\n");
 }

 free(value);
 return 0;
}

int resolve_component_file_dependencies(struct bca_context *ctx,
                                        struct component_details *cd,
                                        int component_index)
{
 char *value, *base_file_name, *extension;
 int z, allocation_size;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_file_dependencies()\n");

 if(component_index >= cd->n_components)
 {
  fprintf(stderr,
          "BCA: resolve_component_file_dependencies(): invalid component_index %d\n",
          component_index);
  return 1;
 }

 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_types[component_index],
                        cd->project_components[component_index],
                        "FILES")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: Could not find %s.%s.FILES\n",
          cd->project_component_types[component_index],
          cd->project_components[component_index]);

  cd->n_file_names = 0;
  return 0;
 }

 if(split_strings(ctx, value, -1,
                  &(cd->n_file_names),
                  &(cd->file_names)))
 {
  fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
  return 1;
 }

 if(ctx->verbose)
 {
  printf("BCA: Found the following files for component \"%s\" (%d): ",
         cd->project_components[component_index], cd->n_file_names);

  for(z=0; z < cd->n_file_names; z++)
  {
   printf("%s ", cd->file_names[z]);
  }
  printf("\n");
  fflush(stdout);
 }

 allocation_size = cd->n_file_names * sizeof(char *);
 if((cd->file_base_names = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 if((cd->file_extensions = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return 1;
 }

 for(z=0; z < cd->n_file_names; z++)
 {
  if(path_extract(cd->file_names[z], &base_file_name, &extension))
  {
   return 1;
  }
  cd->file_base_names[z] = base_file_name;
  cd->file_extensions[z] = extension;
 }

 return 0;
}

int resolve_component_input_dependencies(struct bca_context *ctx,
                                         struct component_details *cd,
                                         int component_index)
{
 char *value, **list = NULL;
 int z, i, n_elements = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_input_dependencies()\n");

 if(component_index >= cd->n_components)
 {
  fprintf(stderr,
          "BCA: resolve_component_file_dependencies(): invalid component_index %d\n",
          component_index);
  return 1;
 }

 if((value = lookup_key(ctx, ctx->project_configuration_contents,
                        ctx->project_configuration_length,
                        cd->project_component_types[component_index],
                        cd->project_components[component_index],
                        "INPUT")) == NULL)
 {
  if(ctx->verbose)
   printf("BCA: Could not find %s.%s.INPUT\n",
          cd->project_component_types[component_index],
          cd->project_components[component_index]);

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
  i = 0;
  while(i< cd->n_components)
  {
   if(strcmp(list[z], cd->project_components[i]) == 0)
    break;
   i++;
  }

  if(i == component_index)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project "
           "component %s apears to list itself as an INPUT element\n",
           cd->project_components[component_index]);

   free_string_array(list, n_elements);
   return 1;
  }

  if(i  == cd->n_components)
  {
   fprintf(stderr,
           "BCA: resolve_component_input_dependencies(): project component %s list INPUT element"
           " %s that is not itself a project component. Perhapes the element belongs in "
           "a FILES record?\n",
           cd->project_components[component_index], list[z]);
   free_string_array(list, n_elements);
   return 1;
  }

  if(add_to_string_array(&(cd->inputs), cd->n_inputs, cd->project_components[i], -1, 1))
  {
   fprintf(stderr, "BCA: resolve_component_file_dependencies(): add_to_string_array()\n");
   return 1;
  }
  cd->n_inputs++;
 }

 free_string_array(list, n_elements);

 return 0;
}

int resolve_component_version(struct bca_context *ctx,
                              char *contents, int contents_length,
                              struct component_details *cd,
                              char *component_type,
                              char *project_component)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: resolve_component_version()\n");

 if((cd->major = lookup_key(ctx, contents, contents_length,
                            component_type, project_component, "MAJOR")) == NULL)
 {
  if((cd->major = lookup_key(ctx, contents, contents_length,
                             "NONE", "NONE", "MAJOR")) == NULL)
  {
   cd->major = strdup("0");
  }
 }

 if((cd->minor = lookup_key(ctx, contents, contents_length,
                            component_type, project_component, "MINOR")) == NULL)
 {
  if((cd->minor = lookup_key(ctx, contents, contents_length,
                             "NONE", "NONE", "MINOR")) == NULL)
  {
   cd->minor = strdup("0");
  }
 }


 if(ctx->verbose)
    printf("BCA: Component \"%s\" version string set to %s.%s\n",
           project_component, cd->major, cd->minor);

 return 0;
}

int smart_pull_value(struct bca_context *ctx)
{
 char *new_value, **values, *file, *contents, *value, *q;
 int code, length, nv_length, allocation_size, n_values, i, handled;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: smart_pull_value()\n");

 if((code = check_value(ctx)) == 1)
 {
  fprintf(stderr, "BCA: check_value() failed.\n");
  return 1;
 }

 if(code != 2)
 {
  if(ctx->verbose)
   printf("BCA: value not in effective value\n");
  return 0;
 }

 /* at this point the value is present either the exact or the .ALL. variant */

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 q = ctx->qualifier;
 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  if(strcmp(ctx->qualifier, "ALL") != 0)
  {
   q = "ALL";
   value = lookup_key(ctx, contents, length, ctx->principle, "ALL", ctx->search_value_key);
  }

  if(value == NULL)
  {
   fprintf(stderr,
           "BCA: This should not happen!  No exact or .ALL. variant present"
           " after positive check_value().\n");
   return 1;
  }
 }

 if( (strcmp(q, "ALL") != 0) ||
     (strcmp(ctx->qualifier, "ALL") == 0) )
 {
  /* at this point, either the value was in an exact record so we do a regular pull_value(), or
     the intended modification for a .ALL. record itself and we do the same thing.  */

/* todo: if the exact record will now hold the same thing as the ALL record, remove the exact record ?*/

  free(value);
  free(contents);
  return pull_value(ctx);
 }

 /* here the value is present in a .ALL. record, but we want to remove it for the exact record
    case. so we copy and modify the .ALL. record and store it as an exact record */

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 allocation_size += 2;
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 new_value[nv_length = 0] = 0;
 for(i=0; i < n_values; i++)
 {
  if(strcmp(values[i], ctx->new_value_string) == 0)
  {
   handled = 1;
  } else {
   nv_length +=
   snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
  }
 }

 if(handled == 0)
 {
  fprintf(stderr,
          "BCA: should not happen: substring not in .ALL. value being to be copied "
          "and modified\n");
  return 1;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int smart_add_value(struct bca_context *ctx)
{
 char *new_value, **values, *file, *contents, *value;
 int code, length, allocation_size, n_values, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: smart_add_value()\n");

 if((code = check_value(ctx)) == 1)
 {
  fprintf(stderr, "BCA: check_value() failed.\n");
  return 1;
 }

 if(code == 2)
 {
  if(ctx->verbose)
   printf("BCA: value already in effective value\n");
  return 0;
 }

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  if(strcmp(ctx->qualifier, "ALL") != 0)
   value = lookup_key(ctx, contents, length, ctx->principle, "ALL", ctx->search_value_key);
 }

 if(value != NULL)
 {

  allocation_size = strlen(value);

  if(split_strings(ctx, value, allocation_size, &n_values, &values))
  {
   fprintf(stderr, "BCA: split_string() failed on %s\n", value);
   return 1;
  }

  length = strlen(ctx->new_value_string);

  if(add_to_string_array(&values, n_values,
                         ctx->new_value_string, length, 1) != 0)
  {
   fprintf(stderr, "BCA: add_to_string_array() failed\n");
   return 1;
  }
  n_values++;

  allocation_size += (n_values + length + 2);
  if((new_value = malloc(allocation_size)) == NULL)
  {
   perror("BCA: malloc()");
   return 1;
  }

  length = 0;
  for(i=0; i < n_values; i++)
  {
   length +=
   snprintf(new_value + length, allocation_size - length,
            "%s ", values[i]);
  }

 } else {
  new_value = ctx->new_value_string;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(contents);
 return 0;
}

int add_value(struct bca_context *ctx)
{
 char *contents, *file, **values, *new_value, *value;
 int length, n_values, allocation_size, nv_length, i;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: add_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  fprintf(stderr,
          "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
          ctx->principle, ctx->qualifier, ctx->search_value_key, file);
  return 1;
 }

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 nv_length = strlen(ctx->new_value_string);

 if(add_to_string_array(&values, n_values,
                        ctx->new_value_string, nv_length, 1) != 0)
 {
  fprintf(stderr, "BCA: add_to_string_array() failed\n");
  return 1;
 }
 n_values++;

 allocation_size += (n_values + nv_length + 2);
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 nv_length = 0;
 for(i=0; i < n_values; i++)
 {
  nv_length +=
  snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
 }


 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int pull_value(struct bca_context *ctx)
{
 char *contents, *file, **values, *new_value, *value;
 int length, n_values, allocation_size, nv_length, i, handled = 0;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: pull_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 if((value = lookup_key(ctx, contents, length, ctx->principle,
                        ctx->qualifier, ctx->search_value_key)) == NULL)
 {
  fprintf(stderr,
          "BCA: lookup_key() failed for %s.%s.%s in file %s\n",
          ctx->principle, ctx->qualifier, ctx->search_value_key, file);
  return 1;
 }

 allocation_size = strlen(value);

 if(split_strings(ctx, value, allocation_size, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return 1;
 }

 allocation_size += 2;
 if((new_value = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return 1;
 }

 new_value[nv_length = 0] = 0;
 for(i=0; i < n_values; i++)
 {
  if(strcmp(values[i], ctx->new_value_string) == 0)
  {
   handled = 1;
  } else {
   nv_length +=
   snprintf(new_value + nv_length, allocation_size - nv_length,
           "%s ", values[i]);
  }
 }

 if(handled == 0)
 {
  fprintf(stderr, "BCA: pull_value(): substring '%s' no in original value\n",
          ctx->new_value_string);
  return 1;
 }

 if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                ctx->search_value_key, new_value))
 {
  fprintf(stderr, "BCA: modify_file() failed\n");
  return 1;
 }

 free(new_value);
 free_string_array(values, n_values);
 free(value);
 free(contents);
 return 0;
}

int check_value_inline(struct bca_context *ctx,
                       char *contents, int length,
                       char *principle, char *qualifier,
                       char *key, char *check_value)
{
 char *value, **values = NULL;
 int i, value_length, n_values = 0, checked = 0;

 if((value = lookup_key(ctx, contents, length,
                        principle, qualifier, key)) == NULL)
 {
  if(strcmp(qualifier, "ALL") != 0)
   value = lookup_key(ctx, contents, length,
                      principle, "ALL", key);
 }

 if(value == NULL)
 {
  if(ctx->verbose)
   fprintf(stderr,
           "BCA: lookup_key() failed for %s.%s.%s\n",
           ctx->principle, ctx->qualifier, ctx->search_value_key);

  return 0;
 }

 value_length = strlen(value);

 if(split_strings(ctx, value, value_length, &n_values, &values))
 {
  fprintf(stderr, "BCA: split_string() failed on %s\n", value);
  return -1;
 }

 i=0;
 while(i<n_values)
 {
  if(strcmp(values[i], check_value) == 0)
  {
   checked = 1;
   break;
  }
  i++;
 }

 free_string_array(values, n_values);
 free(value);

 return checked;
}

int check_value(struct bca_context *ctx)
{
 char *contents, *file;
 int code, length;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: check_value()\n");

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       break;
 }

 if((contents = read_file(file, &length, 0)) == NULL)
 {
  return 1;
 }

 code = check_value_inline(ctx, contents, length,
                           ctx->principle, ctx->qualifier, ctx->search_value_key,
                           ctx->new_value_string);

 switch(code)
 {
  case -1:
       code = 1;
       break;

  case 0:
       if(ctx->verbose)
        fprintf(stderr, "BCA: '%s' not found in %s.[%s/ALL].%s. Returning 3\n",
                ctx->new_value_string, ctx->principle, ctx->qualifier, ctx->search_value_key);

       code = 3;
       break;

  case 1:
       if(ctx->verbose)
        fprintf(stderr,
                "BCA: '%s' found in %s.[%s/ALL].%s. Returning 2.\n",
                ctx->new_value_string, ctx->principle, ctx->qualifier, ctx->search_value_key);
       code = 2;
       break;
 }

 free(contents);
 return code;
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
  return 1;

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
                    ctx->build_configuration_contents,
                    ctx->build_configuration_length,
                    "*", "*", "config.h");

 if(value != NULL)
 {
  config_h = 1;
  free(value);
 }

 return config_h;
}


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

int split_strings(struct bca_context *ctx, char *source, int length,
                  int *count, char ***strings)
{
 int pass = 0, allocation_size, index, n_strings = 0, substring_length, x, start;
 char *string, **array;

 if(ctx->verbose > 2)
 {
  fprintf(stderr, "BCA: split_strings(\"%s\")\n", source);
  fflush(stderr);
 }

 if(source == NULL)
 {
  *count = 0;
  *strings = NULL;
  return 0;
 }

 if(length == -1)
  length = strlen(source);

 if(length == 0)
 {
  *strings = NULL;
  *count = 0;
  return 0;
 }

 while(pass < 2)
 {
  start = -1;
  index = 0;
  x = 0;
  while(index < length)
  {

   while(index < length)
   {
    if(source[index] == ' ')
     break;

    index++;
   }

   if(pass == 0)
    n_strings++;

   if(pass == 1)
   {

    substring_length = (index - start) - 1;

    allocation_size = substring_length + 1;
    if((string = (char *) malloc(allocation_size)) == NULL)
    {
     fprintf(stderr, "BCA: split_strings(): malloc(%d) failed, %s\n",
             allocation_size, strerror(errno));
     return 1;
    }

    memcpy(string, source + start + 1, substring_length);
    string[substring_length] = 0;
    array[x++] = string;
   }

   start = index;
   index++;
  }

  if(pass == 0)
  {
   /* allocated the pointer array */
   allocation_size = n_strings * sizeof(char *);
   if((array = (char **) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: split_strings(): malloc(%d) failed, %s\n",
            allocation_size, strerror(errno));
    return 1;
   }
  }

  pass++;
 }

 *strings = array;
 *count = n_strings;
 return 0;
}

/* escapes quotes and white space */
char *escape_value(struct bca_context *ctx, char *source, int length)
{
 int allocation_size, n_quotes = 0, n_spaces = 0, i, x;
 char *new_string;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: escape_value()\n");

 if(source == NULL)
  return NULL;

 if(length == -1)
  length = strlen(source);

 for(i=0; i<length; i++)
 {
  if(source[i] == ' ')
   n_spaces++;

  if(source[i] == '"')
   n_quotes++;
 }

 if(n_spaces + n_quotes == 0)
  return source;

 allocation_size = length + 3 + n_quotes;
 if((new_string = malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  return NULL;
 }

 x = 0;
 if(n_spaces)
  new_string[x++] = '"';

 for(i=0; i<length; i++)
 {
  if(source[i] == '"')
  {
   new_string[x++] = '\\';
   new_string[x++] = '"';
  } else {
   new_string[x++] = source[i];
  }
 }

 if(n_spaces)
  new_string[x++] = '"';

 new_string[x] = 0;

 return new_string;
}

/* returns 1 is search is within source */
int contains_string(char *source, int source_length, char *search, int search_length)
{
 int x;

 if(source == NULL)
  return 0;

 if(search == NULL)
  return 0;

 if(source_length == -1)
  source_length = strlen(source);

 if(search_length == -1)
  search_length = strlen(search);

 x = 0;
 while( (x + search_length) < source_length)
 {
  if(strncmp(source + x, search, search_length) == 0)
   return 1;

  x++;
 }

 return 0;
}

int path_extract(const char *full_path, char **base_file_name, char **extension)
{
 int allocation_size, full_path_length, base_file_name_length, extension_length,
     index, period_position = -1, slash_position = -1;

 if(full_path == NULL)
  return 1;

 full_path_length = strlen(full_path);

 index = full_path_length;
 while(index > 0)
 {
  if(full_path[index] == '/')
  {
   index = full_path_length;
   break;
  }

  if(full_path[index] == '.')
   break;
  index--;
 }

 if((extension_length = full_path_length - (index + 1)) > 0)
  period_position = index;

 if(period_position == -1)
 {
  if(extension != NULL)
   *extension = NULL;

  period_position = index;
 } else {
  if(extension != NULL)
  {
   allocation_size = extension_length + 1;
   if((*extension = (char *) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
    return 1;
   }
   memcpy(*extension, full_path + period_position + 1, extension_length);
   (*extension)[extension_length] = 0;
  }
 }

 index = period_position;
 while(index > 0)
 {
  if(full_path[index] == '/')
  {
   slash_position = index;
   break;
  }

  index--;
 }

 base_file_name_length = period_position - (slash_position + 1);
 
 if(base_file_name != NULL)
 {
  allocation_size = base_file_name_length + 1;
  if((*base_file_name = (char *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
   return 1;
  } 
  memcpy(*base_file_name, full_path + slash_position + 1, base_file_name_length);
  (*base_file_name)[base_file_name_length] = 0;
 }

 return 0;
}


char *read_file(char *name, int *length, int silent_test)
{
 int file_length, read_length;
 int allocation_size;
 char *contents;
 FILE *f;

 if((f = fopen(name, "r")) == NULL)
 {
  if(silent_test == 0)
   fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  return NULL;
 }

 if(fseek(f, 0, SEEK_END))
 {
  fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  fclose(f);
  return NULL;
 }

 if((file_length = ftell(f)) < 0)
 {
  fprintf(stderr, "BCA: ftell(%s) failed", name);
  fclose(f);
  return NULL;
 }

 if(fseek(f, 0, SEEK_SET))
 {
  fprintf(stderr, "BCA: %s %s\n", name, strerror(errno));
  fclose(f);
  return NULL;
 }

 allocation_size = file_length + 1;
 if((contents = (char *)
                malloc(allocation_size)) == NULL)
 {
  perror("BCA: malloc()");
  fclose(f);
  return NULL;
 }

 if((read_length = fread(contents, 1, file_length, f)) < file_length)
 {
  fprintf(stderr, "BCA: fread() returned %d instead of %d",
            (int) read_length, (int) file_length);
  fclose(f);
  return NULL;
 }
 contents[read_length] = 0;

 fclose(f);

 *length = read_length;

 return contents;
}

int find_line(char *buffer, int buffer_length, int *start, int *end, int *line_length)
{
 int x;

 if(buffer == NULL)
 {
  *line_length = 0;
  return 0;
 }

 *start = *end + 1;
 x = *start;

 while(x < buffer_length)
 {
  if(*start > buffer_length)
   return 0;

  if(buffer[x] == 13)
  {
   if(x + 1 <= buffer_length)
   {
    if(buffer[x] == 10)
     x++;
   }
   break;
  }

  if(buffer[x] == 10)
   break;

  if(buffer[x] == 0)
   break;

  x++;
 }

 if((*line_length = x - *start) == 0)
 {
  if(buffer_length > x)
  {
   /*  fprintf(stderr, "BCA: Blank line in input file!\n"); */
   *end = x;
   return find_line(buffer, buffer_length, start, end, line_length);
  }
  return 0;
 }

 *end = x;
 return 1;
}

int add_to_string_array(char ***array, int array_size,
                        char *string, int string_length,
                        int prevent_duplicates)
{
 char **new_ptr, *new_string;
 int allocation_size, i;

 if(prevent_duplicates)
 {
  i = 0;
  while(i<array_size)
  {
   if( ( (*array)[i] == NULL) || (string == NULL) )
   {
    if((*array)[i] != NULL)
     return 1;
    if(string != NULL)
     return 1;
   } else {
    if(strcmp(string, (*array)[i]) == 0)
     return 1;
   }
   i++;
  }
 }

 if(string == NULL)
 {
  string_length = 0;
 } else {
  if(string_length == -1)
   string_length = strlen(string);
 }

 allocation_size = (array_size + 1) * sizeof(char *);

 if((new_ptr = (char **) realloc(*array, allocation_size)) == NULL)
 {
  perror("BCA: realloc()");
  return -1;
 }

 *array = new_ptr;

 if(string != NULL)
 {
  allocation_size = string_length + 1;
  if((new_string = (char *) malloc(allocation_size)) == NULL)
  {
   perror("BCA: malloc()");
   return -1;
  }
  snprintf(new_string, allocation_size, "%s", string);
  (*array)[array_size] = new_string;
 } else {
  (*array)[array_size] = NULL;
 }

 return 0;
}

int free_string_array(char **array, int n_elements)
{
 int x;

 for(x=0; x<n_elements; x++)
 {
  if(array[x] != NULL)
   free(array[x]);
 }
 free(array);

 return 0;
}



/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012,2013,2014 Stover Enterprises, LLC
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

#ifndef WITHOUT_GTK__2_0
#include <gtk/gtk.h>
#define HAVE_GTK
#endif


void help(void)
{
 printf("\n                         Build Configuration Adjust\n"
        "This is part of the build system, and not part of the project it builds.\n"
        "\nusage:\n"
        " --showvalue key\n"
        " --setvalue key newvalue\n"
        " --newvalue key newvalue\n"
        " --addvalue key additionalvalue\n"
        " --pullvalue key valuetoremove\n"
        " --removevalue key\n"
        " --checkvalue key\n"
        " --smartaddvalue key additionalvalue\n"
        " --smartpullvalue key sansvalue\n"
        " --host host (defaults to NATIVE)\n"
        " --component component (defaults to MAIN)\n"
        " --type type (defaults to BINARY)\n"
        " --build (print & manipulate values from build configuration) (default)\n"
        " --project (print & manipulate values from project configuration)\n"
        " --generate-gmakefile\n"
        " --configure\n"
        " --concatenate file list\n"
        " --replacestrings\n"
        " --listbuildhosts\n"
        " --listprojectcomponents\n"
        " --componentoutputnames\n"
        " --componentbuildoutputnames\n"
        " --componentinstallnames\n"
        " --componeneffectivenames\n"
        " --newproject \"project name\"\n"
        " --swap-* host\n"
        " --buildprefix=BUILD_PREFIX\n"

#ifndef IN_SINGLE_FILE_DISTRIBUTION
        " --generate-graphviz\n"
        " --selftest (help debug buildconfigurationadjust itself)\n"
#endif

#ifndef WITHOUT_LIBNEWT
        " --newt-interface\n"
#endif

#ifdef HAVE_GTK
        " --gtk-interface\n"
#endif

        "\n If you are just trying to run ./configure and have no idea what is going on,\n"
        " some of the autoconf compatibility options are:\n"
        " --prefix=INSTALL_PREFIX\n"
        " --host=HOST\n"
        " --help=short\n"
        " --without-*\n"
        " --disable-*\n"
        " --enable-*\n"
        " Some of the autoconf compatibility environment variables are:\n"
        " CC, and PKG_CONFIG_PATH\n"

        "\n Copyright 2012,2013,2014 Stover Enterprises. All rights reserved. "
        "Licensed under the terms of GNU GPLv3.\n"
        "\tSee http://bca.stoverenterprises.com for more information.\n\n");

}

int main(int argc, char **argv)
{
 struct bca_context *ctx;
 char *value, *contents, *file, **list = NULL;
 int length, n_items = 0, i;
 char code;
 struct component_details cd;
 FILE *output;

 memset(&cd, 0, sizeof(struct component_details));

 if((ctx = setup(argc, argv)) == NULL)
  return 1;

 if(ctx->verbose)
  fprintf(stderr, "BCA: Version %s.%s\n",
          BCA_MAJOR, BCA_MINOR);

 switch(ctx->manipulation_type)
 {
  case MANIPULATE_PROJECT_CONFIGURATION:
       file = "./buildconfiguration/projectconfiguration";
       if(ctx->principle == NULL)
        ctx->principle = "BINARY";
       if(ctx->qualifier == NULL)
        ctx->qualifier = "MAIN";
       break;

  case MANIPULATE_BUILD_CONFIGURATION:
       file = "./buildconfiguration/buildconfiguration";
       if(ctx->principle == NULL)
        ctx->principle = "NATIVE";
       if(ctx->qualifier == NULL)
        ctx->qualifier = "ALL";
       break;

 }

 switch(ctx->mode)
 {
  case NEW_PROJECT_MODE:
       if((output = fopen(file, "r")) != NULL)
       {
        fprintf(stderr, "BCA: '%s' should not already exist to use --newproject\n", file);
        fclose(output);
        return 1;
       }
       /* todo: if we have posix, then conditionally create directory buildconfiguration */
       if((output = fopen(file, "w")) == NULL)
       {
        fprintf(stderr, "BCA: couldn't create file '%s'\n", file);
        return 1;
       }
       if(output_modification(ctx, output, NULL, 0, "NONE", "NONE", "PROJECT_NAME",
                              ctx->new_value_string))
       {
        fprintf(stderr, "BCA: modify_file(): output_modifications() failed\n");
        fclose(output);
        return 1;
       }
       fclose(output);
       return 0;
       break;

  case SHOW_VALUE_MODE:
       if((contents = read_file(file, &length, 0)) == NULL)
       {
        return 1;
       }
       if((value = lookup_key(ctx, contents, length, ctx->principle,
                              ctx->qualifier, ctx->search_value_key)) == NULL)
       {

        if( (ctx->manipulation_type == MANIPULATE_PROJECT_CONFIGURATION) &&
            (strcmp(ctx->qualifier, "ALL") != 0) )
        {
         if(ctx->verbose)
          printf("BCA: component level build setting for \"%s\" not found, "
                 "checking host default...\n",
                 ctx->qualifier);

         value = lookup_key(ctx, contents, length, ctx->principle, "ALL", ctx->search_value_key);
        }
       }

       if(value == NULL)
       {
        fprintf(stderr,
                "BCA: no value in %s for %s.%s.%s\n",
                file, ctx->principle, ctx->qualifier, ctx->search_value_key);
        return 1;
       }
       if(ctx->verbose)
        printf("%s: %s.%s.%s = %s\n",
                file, ctx->principle, ctx->qualifier, ctx->search_value_key, value);
       else
        printf("%s", value);

       free(value);
       if(ctx->verbose > 1)
        printf("BCA: SHOW_VALUE_MODE finished\n");
       break;

  case SET_VALUE_MODE:
       /* special case: if the new value is the same, then we don't want to update the
          build configuration file's modified time. */
       if(ctx->manipulation_type == MANIPULATE_BUILD_CONFIGURATION)
       {
        if((contents = read_file(file, &length, 0)) == NULL)
         return 1;

        if((value = lookup_key(ctx, contents, length, ctx->principle,
                               ctx->qualifier, ctx->search_value_key)) != NULL)
        {
         code = strcmp(value, ctx->new_value_string);
         free(value);
         free(contents);
         if(code == 0)
         {
          if(ctx->verbose > 1)
           fprintf(stderr, "BCA: SET_VALUE_MODE finished - value unchanged for build"
                   " configuration - noop\n");
          break;
         }
        }
       }

       if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                      ctx->search_value_key, ctx->new_value_string))
       {
        fprintf(stderr, "BCA: modify_file() failed\n");
        return 1;
       }
       if(ctx->verbose > 1)
        printf("BCA: SET_VALUE_MODE finished\n");
       break;

  case REMOVE_VALUE_MODE:
       if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                      ctx->search_value_key, NULL))
       {
        fprintf(stderr, "BCA: modify_file() failed\n");
        return 1;
       }
       if(ctx->verbose > 1)
        printf("BCA: REMOVE_VALUE_MODE finished\n");
       break;

  case NEW_COMPONENT_MODE:
       if((contents = read_file(file, &length, 0)) == NULL)
       {
        return 1;
       }
       if((value = lookup_key(ctx, contents, length, ctx->principle,
                              ctx->qualifier, ctx->search_value_key)) != NULL)
       {
        fprintf(stderr,
                "BCA: Conflict in file %s: %s.%s.%s = %s already exists.\n",
                file, ctx->principle, ctx->qualifier, ctx->search_value_key, value);

        free(value);
        return 1;
       }

       if(modify_file(ctx, file, ctx->principle, ctx->qualifier,
                      ctx->search_value_key, ctx->new_value_string))
       {
        fprintf(stderr, "BCA: modify_file() failed\n");
        return 1;
       }
       if(ctx->verbose > 1)
        printf("BCA: NEW_COMPONENT_MODE finished\n");
       break;

  case LIST_HOSTS_MODE:
  case LIST_PROJECT_TYPES_MODE:

       if((contents = read_file(file, &length, 0)) == NULL)
       {
        return 1;
       }

       if((code = list_unique_principles(ctx, NULL,
                                         contents, length,
                                         &list, &n_items)) != 0)
       {
        fprintf(stderr, "BCA: list_unique_principles() failed. (%d)\n", code);
       }

       for(i=0; i<n_items; i++)
       {
        printf("%s\n", list[i]);
       }
       return code;
       break;

  case LIST_PROJECT_COMPONENTS_MODE:
       if((ctx->project_configuration_contents =
        read_file("./buildconfiguration/projectconfiguration",
                  &(ctx->project_configuration_length), 0)) == NULL)
       {
        return 1;
       }

       if(list_project_components(ctx, &cd))
       {
        fprintf(stderr, "BCA: list_project_components() failed.\n");
        return 1;
       }

       for(i=0; i<cd.n_components; i++)
       {
        printf("%s\n", cd.project_components[i]);
       }

       return 0;
       break;

  case CHECK_VALUE_MODE:
       if((code = check_value(ctx)) != 1)
       {
        if(ctx->verbose > 1)
         printf("BCA: check_value() finished\n");
       } else {
        fprintf(stderr, "BCA: check_value() failed.\n");
       }
       return code;
       break;

  case ADD_VALUE_MODE:
       if((code = add_value(ctx)) == 0)
       {
        if(ctx->verbose > 1)
         printf("BCA: add_value() finished\n");
       } else {
        fprintf(stderr, "BCA: add_value() failed.\n");
       }
       return code;
       break;

  case SMART_ADD_VALUE_MODE:
       if((code = smart_add_value(ctx)) == 0)
       {
        if(ctx->verbose > 1)
         printf("BCA: smart_add_value() finished\n");
       } else {
        fprintf(stderr, "BCA: smart_add_value() failed.\n");
       }
       return code;
       break;

  case SMART_PULL_VALUE_MODE:
       if((code = smart_pull_value(ctx)) == 0)
       {
        if(ctx->verbose > 1)
         printf("BCA: smart_pull_value() finished\n");
       } else {
        fprintf(stderr, "BCA: smart_pull_value() failed.\n");
       }
       return code;
       break;

  case PULL_VALUE_MODE:
       if((code = pull_value(ctx)) == 0)
       {
        if(ctx->verbose > 1)
         printf("BCA: pull_value() finished\n");
       } else {
        fprintf(stderr, "BCA: pull_value() failed.\n");
       }
       return code;
       break;

  case CONFIGURE_MODE:
       if((code = configure(ctx)) == 0)
       {
        if(ctx->verbose > 1)
         printf("BCA: configure() finished\n");
       } else {
        fprintf(stderr, "BCA: configure failed.\n");
       }
       return code;
       break;

  case GENERATE_GMAKEFILE_MODE:
       if( ((code = generate_gmakefile_mode(ctx)) == 0) &&
           (ctx->verbose > 1) )
        printf("BCA: generate_gmakefile_mode() finished\n");
       return code;
       break;

#ifndef IN_SINGLE_FILE_DISTRIBUTION
  case GENERATE_GRAPHVIZ_MODE:
       if( ((code = generate_graphviz_mode(ctx)) == 0) &&
           (ctx->verbose> 1) )
        printf("BCA: generate_graphviz_mode() finished\n");
       return code;
       break;
#endif

  case STRING_REPLACE_MODE:
       if( ((code = string_replace(ctx)) == 0) &&
           (ctx->verbose > 1) )
        fprintf(stderr, "BCA: string_replace() finished\n");
       return code;
       break;

  case CONCATENATE_MODE:
       if( ((code = concatenate(ctx, argc, argv)) == 0) &&
           (ctx->verbose > 1))
        fprintf(stderr, "BCA: concatenate() finished\n");
       return code;
       break;

  case LIST_COMPONENT_OUTPUT_NAMES_MODE:
       if((n_items = render_project_component_output_name(ctx, ctx->principle,
                                                          ctx->qualifier, 1, &list, NULL)) < 0)
        return 1;
       for(i=0; i<n_items; i++)
       {
        if(list[i][0] != 0)
         printf("%s\n", list[i]);
       }
       free_string_array(list, n_items);
       return 0;
       break;

  case LIST_COMPONENT_BUILD_OUTPUT_NAMES_MODE:
       if((n_items = render_project_component_output_name(ctx, ctx->principle,
                                                          ctx->qualifier, 2, &list, NULL)) < 0)
        return 1;
       for(i=0; i<n_items; i++)
       {
        if(list[i][0] != 0)
         printf("%s\n", list[i]);
       }
       free_string_array(list, n_items);
       return 0;
       break;

  case LIST_COMPONENT_INSTALL_OUTPUT_NAMES_MODE:
       if((n_items = render_project_component_output_name(ctx, ctx->principle,
                                                          ctx->qualifier, 3, &list, NULL)) < 0)
        return 1;
       for(i=0; i<n_items; i++)
       {
        if(list[i][0] != 0)
         printf("%s\n", list[i]);
       }
       free_string_array(list, n_items);
       break;

  case LIST_COMPONENT_EFFECTIVE_OUTPUT_NAMES_MODE:
       if((n_items = render_project_component_output_name(ctx, ctx->principle,
                                                          ctx->qualifier, 4, &list, NULL)) < 0)
        return 1;
       for(i=0; i<n_items; i++)
       {
        if(list[i][0] != 0)
         printf("%s\n", list[i]);
       }
       free_string_array(list, n_items);
       break;

  case SHORT_HELP_MODE:
       return short_help_mode(ctx);
       break;

#ifndef IN_SINGLE_FILE_DISTRIBUTION
  case SELF_TEST_MODE:
       return self_test(ctx);
       break;
#endif

#ifndef WITHOUT_LIBNEWT
  case NEWT_INTERFACE_MODE:
       if( ((code = newt_interface(ctx)) == 0) &&
           (ctx->verbose > 1))
        fprintf(stderr, "BCA: newt_interface() finished\n");
       return code;
       break;
#endif

#ifdef HAVE_GTK
  case GTK_INTERFACE_MODE:
       if( ((code = gtk_interface(ctx)) == 0) &&
           (ctx->verbose > 1))
        fprintf(stderr, "BCA: gtk_interfaace() finished\n");
       return code;
       break;
#endif

  default:
       help();
 }


 return 0;
}

int shutdown(struct bca_context *ctx)
{
 if(ctx == NULL)
  return 0;

 free_string_array(ctx->without_strings, ctx->n_withouts);

 free_string_array(ctx->disabled_components, ctx->n_disables);
 free_string_array(ctx->enabled_components, ctx->n_enables);
 free(ctx);
 return 0;
}

struct bca_context *setup(int argc, char **argv)
{
 struct bca_context *ctx;
 struct component_details cd;
 int allocation_size, current_arg = 1, handled, i;

 allocation_size = sizeof(struct bca_context);
 if((ctx = (struct bca_context *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }

 memset(ctx, 0, allocation_size);
 ctx->manipulation_type = MANIPULATE_BUILD_CONFIGURATION;

 memset(&cd, 0, sizeof(struct component_details));

#ifdef HAVE_CWD
 ctx->cwd = getcwd(NULL, 0);
#endif

#ifdef HAVE_GTK
 gtk_init(&argc, &argv);
#endif

 while(current_arg < argc)
 {
  handled = 0;

  if(strcmp(argv[current_arg], "--help=short") == 0)
  {
   handled = 1;
   ctx->mode = SHORT_HELP_MODE;
  }

  if(strncmp(argv[current_arg], "--help", 6) == 0)
  {
   if(ctx->mode != SHORT_HELP_MODE)
   {
    help();
    return NULL;
   }
  }

  if( (strcmp(argv[current_arg], "-v") == 0) ||
      (strcmp(argv[current_arg], "--verbose") == 0) )
  {
   handled = 1;
   ctx->verbose++;
  }

#ifndef IN_SINGLE_FILE_DISTRIBUTION
  if(strcmp(argv[current_arg], "--selftest") == 0)
  {
   handled = 1;
   ctx->mode = SELF_TEST_MODE;
  }
#endif


  if(strcmp(argv[current_arg], "--generate-graphviz") == 0)
  {
#ifndef IN_SINGLE_FILE_DISTRIBUTION
   handled = 1;
   ctx->mode = GENERATE_GRAPHVIZ_MODE;
#else
   fprintf(stderr,
           "BCA: graphviz plots not available in single file distribution, "
           "please install bca on this system instead.\n");
#endif
  }

  if(strcmp(argv[current_arg], "--configure") == 0)
  {
   handled = 1;
   ctx->mode = CONFIGURE_MODE;
  }

  if(strcmp(argv[current_arg], "--newt-interface") == 0)
  {
#ifndef WITHOUT_LIBNEWT
   handled = 1;
   ctx->mode = NEWT_INTERFACE_MODE;
#else
   fprintf(stderr,
           "BCA: This build configuration adjust was not built with support "
           "for the newt interface. If you want to use this feature, consider "
           "installing a local copy on this system or somewhere in your $PATH.\n");
   return NULL;
#endif
  }

  if(strcmp(argv[current_arg], "--gtk-interface") == 0)
  {
#ifndef WITHOUT_GTK
   handled = 1;
   ctx->mode = GTK_INTERFACE_MODE;
#else
   fprintf(stderr,
           "BCA: This build configuration adjust was not build with support "
           "for the Gtk+ interface. If you want to use this feature, consider "
           "installing a local copy on this system or somewhere in your $PATH.\n");
   return NULL;
#endif
  }

  if(strcmp(argv[current_arg], "--concatenate") == 0)
  {
   handled = 1;
   if(current_arg + 1 > argc)
   {
    fprintf(stderr, "BCA: --concatenate requires a file list\n");
    return NULL;
   }

   ctx->mode = CONCATENATE_MODE;
   ctx->extra_args_start = current_arg + 1;
   return ctx;
  }

  if(strcmp(argv[current_arg], "--replacestrings") == 0)
  {
   handled = 1;
   ctx->mode = STRING_REPLACE_MODE;
  }

  if(strcmp(argv[current_arg], "--generate-gmakefile") == 0)
  {
   handled = 1;
   ctx->mode = GENERATE_GMAKEFILE_MODE;
  }

  if(strcmp(argv[current_arg], "--project") == 0)
  {
   ctx->manipulation_type = MANIPULATE_PROJECT_CONFIGURATION;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--build") == 0)
  {
   ctx->manipulation_type = MANIPULATE_BUILD_CONFIGURATION;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--listbuildhosts") == 0)
  {
   ctx->manipulation_type = MANIPULATE_BUILD_CONFIGURATION;
   ctx->mode = LIST_HOSTS_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--listprojectcomponents") == 0)
  {
   ctx->manipulation_type = MANIPULATE_PROJECT_CONFIGURATION;
   ctx->mode = LIST_PROJECT_COMPONENTS_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--listprojecttypes") == 0)
  {
   ctx->manipulation_type = MANIPULATE_PROJECT_CONFIGURATION;
   ctx->mode = LIST_PROJECT_TYPES_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--newproject") == 0)
  {
   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must specify a project name --newproject\n");
    return NULL;
   }

   ctx->new_value_string = argv[++current_arg];
   ctx->mode = NEW_PROJECT_MODE;
   ctx->manipulation_type = MANIPULATE_PROJECT_CONFIGURATION;
   handled = 1;
  }

  if( (strcmp(argv[current_arg], "--type") == 0) || 
      (strcmp(argv[current_arg], "-T") == 0) )
  {
   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must specify a key string with --type\n");
    return NULL;
   }

   ctx->principle = argv[++current_arg];
   ctx->manipulation_type = MANIPULATE_PROJECT_CONFIGURATION;
   handled = 1;
  }

  if( (strcmp(argv[current_arg], "--host") == 0) || 
      (strcmp(argv[current_arg], "-H") == 0) )
  {
   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must specify key string with --host\n");
    return NULL;
   }

   if(ctx->host_prefix != NULL)
   {
    fprintf(stderr, "BCA: can not use both --host hostidentifier and --host=hostprefix\n");
    return NULL;
   }

   ctx->principle = argv[++current_arg];
   handled = 1;
  }


  if( (strcmp(argv[current_arg], "--component") == 0) || 
      (strcmp(argv[current_arg], "-C") == 0) )
  {
   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must key string with --component\n");
    return NULL;
   }

   ctx->qualifier = argv[++current_arg];
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--removevalue") == 0)
  {
   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must specify a key string with --removevalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   handled = 1;
   ctx->mode = REMOVE_VALUE_MODE;
  }

  if(strcmp(argv[current_arg], "--showvalue") == 0)
  {
   if(argc < current_arg + 2)
   {
    fprintf(stderr, "BCA: you must specify a key string with --showvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   handled = 1;
   ctx->mode = SHOW_VALUE_MODE;
  }

  if(strcmp(argv[current_arg], "--checkvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must supply a key and compare string with --checkvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = argv[++current_arg];
   handled = 1;
   ctx->mode = CHECK_VALUE_MODE;
  }

  if(strcmp(argv[current_arg], "--setvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --setvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = argv[++current_arg];
   ctx->mode = SET_VALUE_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--addvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --addvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = escape_value(ctx, argv[++current_arg], -1);
   ctx->mode = ADD_VALUE_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--smartaddvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --smartaddvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = escape_value(ctx, argv[++current_arg], -1);
   ctx->mode = SMART_ADD_VALUE_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--pullvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --pullvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = escape_value(ctx, argv[++current_arg], -1);
   ctx->mode = PULL_VALUE_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--smartpullvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --smartpullvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = escape_value(ctx, argv[++current_arg], -1);
   ctx->mode = SMART_PULL_VALUE_MODE;
   handled = 1;
  }

  if(strcmp(argv[current_arg], "--newvalue") == 0)
  {
   if(argc < current_arg + 3)
   {
    fprintf(stderr, "BCA: you must specify a key string and value with --newvalue\n");
    return NULL;
   }

   ctx->search_value_key = argv[++current_arg];
   ctx->new_value_string = argv[++current_arg];
   ctx->mode = NEW_COMPONENT_MODE;
   handled = 1;
  }

  if(strncmp(argv[current_arg], "--buildprefix=", 14) == 0)
  {
   handled = 1;
   ctx->build_prefix = argv[current_arg] + 14;
  }

  if(strncmp(argv[current_arg], "--prefix=", 9) == 0)
  {
   handled = 1;
   ctx->install_prefix = argv[current_arg] + 9;
  }

  if(strncmp(argv[current_arg], "--host=", 7) == 0)
  {
   if(ctx->principle != NULL)
   {
    fprintf(stderr, "BCA: can not use both --host / -H hostidentifier and --host=hostprefix\n");
    return NULL;
   }

   /* Build cofiguration adjust has at least two strings are involved in cross compilation. 
      The "host identifier" in the sense of host.component.key in the buildconfiguration file.
      The "build prefix" where the build output goes. (not to be confused with install prefix)

      Autotools uses a "host prefix" i.e. hostprefix-gcc tool file names
      BCA uses the full names for tools, and does not concatenate host prefix + tool name.
      This compatibility option accepts a host prefix and uses it to do the following:
      -change the default names for tools when the are not otherwise specified or detected
      -adjust the behavoir of tool deteching mechanisms
      -generate a host identifier
      -generate a build a prefix
   */
   handled = 1;
   ctx->host_prefix = argv[current_arg] + 7;
   if((ctx->principle = host_identifier_from_host_prefix(ctx)) == NULL)
    return NULL;
  }

  if(strncmp(argv[current_arg], "--without-", 10) == 0)
  {
   handled = 1;

   if(add_to_string_array(&(ctx->without_strings), ctx->n_withouts, 
                          argv[current_arg] + 10, -1, 1))
   {
    return NULL;
   }
   ctx->n_withouts++;
  }

  if(strcmp(argv[current_arg], "--disableall") == 0)
  {
   handled = 1;

   if((ctx->project_configuration_contents = 
       read_file("./buildconfiguration/projectconfiguration", 
                 &(ctx->project_configuration_length), 0)) == NULL)
   {
    fprintf(stderr, "BCA: can't open project configuration file\n");
    return NULL;
   }

   if(list_project_components(ctx, &cd))
   {
    return NULL;
   }

   for(i=0; i<cd.n_components; i++)
   {
    if(add_to_string_array(&(ctx->disabled_components),
                           ctx->n_disables, 
                           cd.project_components[i], -1, 1))
    {
     return NULL;
    }
    ctx->n_disables++;
   }
  }

  if(strncmp(argv[current_arg], "--disable-", 10) == 0)
  {
   handled = 1;

   if(add_to_string_array(&(ctx->disabled_components), ctx->n_disables, 
                          argv[current_arg] + 10, -1, 1))
   {
    return NULL;
   }
   ctx->n_disables++;
  }

  if(strncmp(argv[current_arg], "--enable-", 9) == 0)
  {
   handled = 1;

   if(add_to_string_array(&(ctx->enabled_components), ctx->n_enables, 
                          argv[current_arg] + 9, -1, 1))
   {
    return NULL;
   }
   ctx->n_enables++;
  }

  if(strncmp(argv[current_arg], "--swap-", 7) == 0)
  {
   handled = 1;

   if(argc < current_arg + 1)
   {
    fprintf(stderr, "BCA: you must give a host string with --swap-*\n");
    return NULL;
   }

   if(add_to_string_array(&(ctx->swapped_components), ctx->n_swaps, 
                          argv[current_arg] + 7, -1, 1))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return NULL;
   }

   if(add_to_string_array(&(ctx->swapped_component_hosts), ctx->n_swaps, 
                          argv[++current_arg], -1, 1))
   {
    fprintf(stderr, "BCA: add_to_string_array() failed\n");
    return NULL;
   }

   ctx->n_swaps++;
  }

  if(strcmp(argv[current_arg], "--componentoutputnames") == 0)
  {
   handled = 1;
   ctx->mode = LIST_COMPONENT_OUTPUT_NAMES_MODE;
   if(ctx->qualifier == NULL)
   {
    printf("BCA: need to specify a component with --component\n");
    return NULL;
   }
  }

  if(strcmp(argv[current_arg], "--componentbuildoutputnames") == 0)
  {
   handled = 1;
   ctx->mode = LIST_COMPONENT_BUILD_OUTPUT_NAMES_MODE;
   if(ctx->qualifier == NULL)
   {
    printf("BCA: need to specify a component with --component\n");
    return NULL;
   }
  }

  if(strcmp(argv[current_arg], "--componentinstallnames") == 0)
  {
   handled = 1;
   ctx->mode = LIST_COMPONENT_INSTALL_OUTPUT_NAMES_MODE;
   if(ctx->qualifier == NULL)
   {
    printf("BCA: need to specify a component with --component\n");
    return NULL;
   }
  }

  if(strcmp(argv[current_arg], "--componenteffectivenames") == 0)
  {
   handled = 1;
   ctx->mode = LIST_COMPONENT_EFFECTIVE_OUTPUT_NAMES_MODE;
   if(ctx->qualifier == NULL)
   {
    printf("BCA: need to specify a component with --component\n");
    return NULL;
   }
  }

  if(handled == 0)
  {
   fprintf(stderr, 
           "BCA: I don't know what to do with the parameter \"%s\".\n", argv[current_arg]);
   return NULL;
  }

  current_arg++;
 }

 return ctx;
}

int concatenate(struct bca_context *ctx, int argc, char **argv)
{
 int i;
 FILE *input;
 char buffer[2];

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: concatenate()\n");

 for(i=ctx->extra_args_start; i<argc; i++)
 {
  if(ctx->verbose)
   fprintf(stderr, "BCA: concatenate input file '%s'\n", argv[i]);

  if((input = fopen(argv[i], "r")) == NULL)
  {
   perror("BCA: fopen()\n");
   return 1;
  }

  while(!feof(input))
  {
   fread(buffer, 1, 1, input);
   fwrite(buffer, 1, 1, stdout);
  }

  fclose(input);
 }

 return 0;
}

int short_help_mode(struct bca_context *ctx)
{
 char **list = NULL, **package_list = NULL;
 int i, j, code, n_elements = 0, n_packages = 0;
 struct component_details cd;

 memset(&cd, 0, sizeof(struct component_details));

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: short_test_mode()\n");

 if((ctx->project_configuration_contents = 
     read_file("./buildconfiguration/projectconfiguration", 
               &(ctx->project_configuration_length), 0)) == NULL)
 {
  fprintf(stderr, "BCA: can't open project configuration file\n");
  return 1;
 }

 if(list_project_components(ctx, &cd))
  return 1;

 if(ctx->verbose)
 {
  printf("BCA: found (%d) project components: ", cd.n_components);
  for(i=0; i < cd.n_components; i++)
  {
   printf("%s ", cd.project_components[i]);
  }
  printf("\n");
 }

 for(i=0; i < cd.n_components; i++)
 {
  cd.project_component = cd.project_components[i];

  if(list_component_opt_external_dependencies(ctx, &cd, &list, &n_elements))
   return 1;

  for(j=0; j < n_elements; j++)
  {
   if((code = add_to_string_array(&package_list, n_packages, list[j], -1, 1)) < 0)
   {
    return 1;
   }

   if(code == 0)
    n_packages++;
  }
  free_string_array(list, n_elements);
 }

 printf("Avaiable without qualifiers:\n");
 for(i=0; i < n_packages; i++)
 {
  printf("\t--without-%s\n", package_list[i]);
 }

 return 0;
}





/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2012,2013,2014 Stover Enterprises, LLC
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

char *component_type_file_extension(struct bca_context *ctx, struct host_configuration *tc,
                                    char *project_component_type,
                                    char *project_component_output_name)
{
 int x;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: component_type_file_extension()\n");

 if(strcmp(project_component_type, "BINARY") == 0)
  return tc->binary_suffix;

 if(strcmp(project_component_type, "SHAREDLIBRARY") == 0)
  return tc->shared_library_suffix;

 if(strcmp(project_component_type, "CAT") == 0)
  return "";

 if(strcmp(project_component_type, "MACROEXPAND") == 0)
  return "";

 if(strcmp(project_component_type, "CUSTOM") == 0)
 {
  x = 0;
  while(x > 0)
  {
   if(project_component_output_name[x] == '.')
   {
    return project_component_output_name + x + 1;
   }
   x--;
  }
  return "";
 }

 if(strcmp(project_component_type, "PYTHONMODULE") == 0)
  return ".py";

 fprintf(stderr, "BCA: unknown component type \"%s\"\n",
         project_component_type);

 return NULL;
}

char *host_identifier_from_host_prefix(struct bca_context *ctx)
{
 int length, i;
 char *host_identifier = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: host_identifier_from_host_prefix()\n");

 if(ctx->host_prefix == NULL)
 {
  fprintf(stderr, "BCA: host_identifier_from_host_prefix(): no host_prefix\n");
  return NULL;
 }

 length = strlen(ctx->host_prefix);
 if((host_identifier = malloc(length + 1)) == NULL)
 {
  perror("BCA: host_identifier_from_host_prefix(): malloc()\n");
  return NULL;
 }

 for(i=0; i<length; i++)
 {
  host_identifier[i] = toupper(ctx->host_prefix[i]);
 }
 host_identifier[i] = 0;

 return host_identifier;
}

char *build_prefix_from_host_prefix(struct bca_context *ctx)
{
 int length, i;
 char *build_prefix = NULL;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: build_prefix_from_host_prefix()\n");

 if(ctx->host_prefix == NULL)
 {
  fprintf(stderr, "BCA: build_prefix_from_host_prefix(): no host_prefix\n");
  return NULL;
 }

 length = strlen(ctx->host_prefix);
 if((build_prefix = malloc(length + 1)) == NULL)
 {
  perror("BCA: build_prefix_from_host_prefix(): malloc()\n");
  return NULL;
 }

 for(i=0; i<length; i++)
 {
  build_prefix[i] = tolower(ctx->host_prefix[i]);
 }
 build_prefix[i] = 0;

 return build_prefix;
}


int render_project_component_output_name(struct bca_context *ctx,
                                         char *host, char *component, int edition,
                                         char ***array_ptr, char ***extensions)
{
 char **hosts, *extension, temp[1024], **names = NULL, *component_install_path;
 int n_hosts, x, y, handled, n_names = 0, code,
     prefix_length, import, effective_path_mode;
 struct component_details cd;
 struct host_configuration *tc;

 memset(&cd, 0, sizeof(struct component_details));

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: render_project_component_output_name(%s, %s)\n", host, component);

 if(host == NULL)
 {
  fprintf(stderr, "BCA: render_project_component_output_name(): host can not be NULL\n");
  return -1;
 }

 if(ctx->build_configuration_contents == NULL)
 {
  if((ctx->build_configuration_contents =
      read_file("./buildconfiguration/buildconfiguration",
                &(ctx->build_configuration_length), 0)) == NULL)
  {
   fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
   return -1;
  }
 }

 if(list_unique_principles(ctx, NULL, ctx->build_configuration_contents,
                           ctx->build_configuration_length, &hosts, &n_hosts))
 {
  fprintf(stderr, "BCA: list_build_hosts() failed\n");
  return -1;
 }

 if(ctx->project_configuration_contents == NULL)
 {
  if((ctx->project_configuration_contents =
       read_file("./buildconfiguration/projectconfiguration",
                 &(ctx->project_configuration_length), 0)) == NULL)
  {
   return -1;
  }
 }

 if(list_project_components(ctx, &cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed\n");
  return -1;
 }

 for(x=0; x<n_hosts; x++)
 {
  if(strcmp(hosts[x], host) == 0)
  {
   for(y=0; y < cd.n_components; y++)
   {
    if(strcmp(cd.project_components[y], component) == 0)
    {

     cd.project_component = cd.project_components[y];
     cd.project_component_type = cd.project_component_types[y];
     cd.host = hosts[x];

     if((tc = resolve_host_configuration(ctx, &cd)) == NULL)
      return -1;

     if((extension = component_type_file_extension(ctx, tc, cd.project_component_types[y],
                                                   cd.project_output_names[y])) == NULL)
     {
      fprintf(stderr, "BCA: component_type_file_extension(%s) failed\n",
              cd.project_component_types[y]);
      return -1;
     }

     handled = 0;

     /* resolve the prefix (if any) */
     switch(edition)
     {
      case 1: /* output name */
            prefix_length = 0;
            break;

      case 2: /* build output name */
           prefix_length = snprintf(temp, 1024, "%s/", tc->build_prefix);
           break;

      case 3: /* install output name */
           temp[0] = 0;
           prefix_length = 1;

           if(resolve_component_installation_path(ctx,
                                                  host,
                                                  cd.project_component_types[y],
                                                  cd.project_components[y],
                                                  &component_install_path))
           {
            fprintf(stderr,
                    "BCA: resolve_component_installation_path(%s, %s) failed\n",
                    cd.project_component_types[y], cd.project_components[y]);
            return -1;
           }

           if(component_install_path == NULL)
           {
            /* component has been manually specifed to not be installed */
            return 0;
           }
           prefix_length = snprintf(temp, 1024, "%s/", component_install_path);
           free(component_install_path);
           component_install_path = NULL;
           break;

      case 4: /* effective output name */
           if((effective_path_mode = resolve_effective_path_mode(ctx)) == -1)
            return -1;

           switch(effective_path_mode)
           {
            case EFFECTIVE_PATHS_LOCAL:
                 return render_project_component_output_name(ctx, host, component, 2,
                                                             array_ptr, extensions);
                 break;

            case EFFECTIVE_PATHS_INSTALL:
                 return render_project_component_output_name(ctx, host, component, 3,
                                                             array_ptr, extensions);
                 break;

            default:
                 fprintf(stderr, "BCA: render_project_component_name(): return code from "
                         "resolve_effective_path_mode() makes no sense\n");
                 return -1;
           }
           break;

      default:
           fprintf(stderr, "BCA: render_project_component_name(): invalid edition number, %d\n", 
                   edition);
           return -1;
           break;
     }

     if(strcmp(cd.project_component_types[y], "PYTHONMODULE") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length,
               "%s-setup.py", cd.project_output_names[y]);

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, ".py", -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(strcmp(cd.project_component_types[y], "BINARY") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length,
               "%s%s", cd.project_output_names[y], extension);

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(strcmp(cd.project_component_types[y], "BUILDBINARY") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length,
               "%s%s", cd.project_output_names[y], extension);

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(strcmp(cd.project_component_types[y], "SHAREDLIBRARY") == 0)
     {
      handled = 1;

      if(resolve_component_version(ctx, ctx->project_configuration_contents,
                                   ctx->project_configuration_length, &cd,
                                   "SHAREDLIBRARY", cd.project_components[y]))
      {
       fprintf(stderr, "BCA: resolve_component_version() failed\n");
       return -1;
      }

      if(strcmp(extension, ".so") == 0)
      {
       /* so name */
       snprintf(temp + prefix_length, 1024 - prefix_length,
                "%s%s%s.%s",
                tc->shared_library_prefix, cd.project_output_names[y], extension, cd.major); 
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;

       n_names++;

       /* pc file name */
       if(edition == 3)
       {
        snprintf(temp, 1024, "%s/%s-%s.pc",
                 tc->install_pkg_config_dir, cd.project_output_names[y], cd.major);
       } else {
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "%s-%s.pc",
                 cd.project_output_names[y], cd.major);
       }
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, "pc", -1, 0)) < 0)
         return -1;

       n_names++;

       /* import library */
       if((code = add_to_string_array(&names, n_names, "", -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, "", -1, 0)) < 0)
         return -1;
       n_names++;

       /* real name */
       snprintf(temp + prefix_length, 1024 - prefix_length,
                "%s%s%s.%s.%s",
                tc->shared_library_prefix, cd.project_output_names[y], extension,
                cd.major, cd.minor);

       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;
       n_names++;
      }

      if(strcmp(extension, ".dll") == 0)
      {
       /* so name */
       snprintf(temp + prefix_length, 1024 - prefix_length,
                "%s%s-%s.%s%s",
                tc->shared_library_prefix, cd.project_output_names[y],
                cd.major, cd.minor, extension);

       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;
       n_names++;

       /* pc file */
       if(edition == 3)
       {
        snprintf(temp, 1024, "%s/%s-%s.pc",
                 tc->install_pkg_config_dir, cd.project_output_names[y], cd.major);
       } else {
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "%s-%s.pc",
                 cd.project_output_names[y], cd.major);
       }
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, "pc", -1, 0)) < 0)
         return -1;
       n_names++;

       /* import library */
       import = 0;

       if(contains_string(tc->cc, -1, "mingw", -1))
       {
        import = 1;
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "%s%s-%s.%s%s.a",
                 tc->shared_library_prefix, cd.project_output_names[y],
                 cd.major, cd.minor, extension);

        if(extensions != NULL)
         if((code = add_to_string_array(extensions, n_names, "a", -1, 0)) < 0)
          return -1;
       }

       if(strcmp(tc->shared_library_prefix, "cyg") == 0)
       {
        import = 1;
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "lib%s-%s.%s%s.a",
                 cd.project_output_names[y], cd.major, cd.minor, extension);
        if(extensions != NULL)
         if((code = add_to_string_array(extensions, n_names, "a", -1, 0)) < 0)
          return -1;
       }

       if(import == 0)
       {
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "%s%s-%s.%s.lib",
                 tc->shared_library_prefix, cd.project_output_names[y], cd.major, cd.minor); 

        if(extensions != NULL)
         if((code = add_to_string_array(extensions, n_names, "lib", -1, 0)) < 0)
          return -1;
       }

       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;
       n_names++;

       temp[0] = 0;
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;

       n_names++;
      }

      if(strcmp(extension, ".dylib") == 0)
      {
       /* install name (soname analog) */
       snprintf(temp + prefix_length, 1024 - prefix_length,
                "%s%s.%s%s",
                tc->shared_library_prefix, cd.project_output_names[y], cd.major, extension); 
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;

       n_names++;

       /* pc file name */
       if(edition == 3)
       {
        snprintf(temp, 1024, "%s/%s-%s.pc",
                 tc->install_pkg_config_dir, cd.project_output_names[y], cd.major);
       } else {
        snprintf(temp + prefix_length, 1024 - prefix_length,
                 "%s-%s.pc",
                 cd.project_output_names[y], cd.major);
       }
       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, "pc", -1, 0)) < 0)
         return -1;

       n_names++;

       /* import library */
       if((code = add_to_string_array(&names, n_names, "", -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, "", -1, 0)) < 0)
         return -1;
       n_names++;

       /* real name */
       snprintf(temp + prefix_length, 1024 - prefix_length,
                "%s%s.%s.%s%s",
                tc->shared_library_prefix, cd.project_output_names[y],
                cd.major, cd.minor, extension);

       if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
        return -1;

       if(extensions != NULL)
        if((code = add_to_string_array(extensions, n_names, extension, -1, 0)) < 0)
         return -1;
       n_names++;
      }

     }

     if(strcmp(cd.project_component_types[y], "CAT") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length, "%s", cd.project_output_names[y]); 

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, "", -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(strcmp(cd.project_component_types[y], "MACROEXPAND") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length, "%s", cd.project_output_names[y]); 

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, "", -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(strcmp(cd.project_component_types[y], "CUSTOM") == 0)
     {
      handled = 1;
      snprintf(temp + prefix_length, 1024 - prefix_length, "%s", cd.project_output_names[y]); 

      if((code = add_to_string_array(&names, n_names, temp, -1, 0)) < 0)
       return -1;

      if(extensions != NULL)
       if((code = add_to_string_array(extensions, n_names, "", -1, 0)) < 0)
        return -1;

      n_names++;
     }

     if(handled == 0)
     {
      fprintf(stderr,
              "BCA: I don't know how to render an output file name for component "
              "type of \"%s\"\n",
              cd.project_component_types[y]);
      return -1;
     }

     free_host_configuration(ctx, tc);
    }
   }
  }
 }

 free_string_array(hosts, n_hosts);

 if(n_names < 1)
 {
  fprintf(stderr,
          "BCA: render_project_component_output_name(): is there really a component "
          "named \"%s\" ?\n",
          component);
  *array_ptr = NULL;
 }

 *array_ptr = names;
 return n_names;
}

char *without_string_to_without_macro(struct bca_context *ctx, char *in)
{
 int length, i;
 char *out;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: without_sting_to_without_macro(%s)\n", in);

 length = strlen(in);
 if((out = malloc(length + 1)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", length + 1);
  return NULL;
 }

 for(i=0; i<length; i++)
 {
  if(isalnum(in[i]))
  {
   if(isalpha(in[i]))
   {
    out[i] = toupper(in[i]);
   } else {
    out[i] = in[i];
   }
  } else {
   out[i] = '_';
  }
 }
 out[i] = 0;

 return out;
}

char *lib_file_name_to_link_name(const char *file_name)
{
 char *lib_name, *extension;
 int length;

 /* todo: make this better */

 if(path_extract(file_name, &lib_name, &extension))
 {
  fprintf(stderr, "BCA: path_extract(%s) failed\n", file_name);
  return NULL;
 }

 length = strlen(lib_name);

 if(length > 3)
 {
  if(strcmp(lib_name + length - 3, ".so") == 0)
  {
   lib_name[length - 3] = 0;
   length -= 3;
  }
 }

 if(length > 3)
 {
  if(strncmp(lib_name, "lib", 3) == 0)
  {
   memmove(lib_name, lib_name + 3, length - 3);
   length -= 3;
   lib_name[length] = 0;
  }
 }

 free(extension);
 return lib_name;
}

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
 int i, j, x, handled, skip, pre_loaded, yes = 0;
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
    handled = 0;
    x = 0;
    while(x < cd->n_components)
    {
     if(strcmp(cd->inputs[j], cd->project_components[x]) == 0)
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
             cd->project_component, cd->host, cd->inputs[j]);
     return 1;
    }


    if(path_extract(cd->project_output_names[x], NULL, &extension))
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

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: No files matching *.%s found.\n", type_extension);

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

int test_package_exist_helper(char *command,
                              struct bca_context *ctx,
                              struct component_details *cd,
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
}

int test_package_exist(struct bca_context *ctx,
                       struct component_details *cd,
                       struct host_configuration *tc,
                       char *package, int optional)
{
 char command[1024];
 int code, length;
 FILE *test;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: test_package_exist(%s, %d)\n", package, optional);

 if(test_package_exist_helper(command, ctx, cd, tc, package) == 0)
  return 0;

 length = snprintf(command, 1024, "./buildconfiguration/generate-pkg-config-%s",
                   package);

 if((test = fopen(command, "r")) != NULL)
 {
  fclose(test);
  snprintf(command + length, 1024 - length,
           "%s", cd->host);

  if(ctx->verbose > 1)
   fprintf(stderr, "BCA: attempting \"%s\"...\n", command);

  if( (code = system(command)) > -1)
  {
   if(WEXITSTATUS(code) == 0)
   {
    if(test_package_exist_helper(command, ctx, cd, tc, package) == 0)
     return 0;
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
   if(s[0] == 0)
    s = NULL;
  if(s != NULL)
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
     n_elements = 0, n_depends, n_opt_deps, n_withouts, x, i, j, yes, code, handled, p_length,
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
     if((code = add_to_string_array(&test_package_list, 
                                    n_test_packages, list[j], -1, 1)) < 0)
     {
      return 1;
     }

     if(code == 0)
     {
      n_test_packages++;

      /* we keep an array along side the list of packages to test for that
         holds if the package was an optional dependency. This way we don't
         test for the same package twice if it is in both catagories. The
         optional flag then determines if we can proceed if the package was
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
  if(s[0] == 0)
   s = NULL;
 if(s != NULL)
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
 char o_principle[256], o_component[256], o_key[256],
      *value, **project_disables;
 int i, j, n_project_disables, yes, p_length, end;

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

 if(check_duplicate_output_names(ctx, cd))
 {
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

 /* Now we want to make sure than none of disabled componts are being
    swapped to from other host.*/
 end = -1;
 while(iterate_key_primitives(ctx, ctx->build_configuration_contents,
                              ctx->build_configuration_length, &end,
                              NULL, NULL, "SWAP",
                              o_principle, o_component, o_key, NULL))
 {
  value = lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       o_principle, o_component, o_key);

  for(i = 0; i < ctx->n_disables; i++)
  {
   if(strcmp(o_component, ctx->disabled_components[i]) == 0)
   {
    if(strcmp(value, ctx->principle) == 0)
    {
     fprintf(stderr,
             "BCA: I can not disable component \"%s\" on host \"%s\", "
             "because it is swapped to from host \"%s\". Remove that swap first.\n",
             o_component, value, o_principle);
     return 1;
    }
   }
  }

  free(value);
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

 if(add_to_string_array(array, *array_length,
                        string + mark, i - mark, 0))
 {
  fprintf(stderr, "BCA: add_to_string_array() failed\n");
  free_string_array(*array, *array_length);
  return 1;
 }
 (*array_length)++;

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

 if(strncmp(key, "CHECK(", 6) == 0)
 {
  return check_function(ctx, key);
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

/* GPLv36

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
    along with this program.  If not, see http://www.gnu.org/licenses.
*/

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "prototypes.h"
#endif

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

/* make clean logic */
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
 fprintf(output, "\n\n\n");

 free_string_array(array, array_length);

 return 0;
}

/* make help */
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
         "version %s.%s\"\n\n\n", BCA_MAJOR, BCA_MINOR);

 return 0;
}

/* "make all" calls each host, but you can do make specific-host */
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
 fprintf(output, "\n");

 return 0;
}

/* both source files that first go to object files before linking (ie .c),
   and sources that compile and link in one step (ie .cs) will us these
   "compile flags".
 */
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

 if(is_project_using_config_h(ctx) == 0)
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

/* .INPUT is a list of other components from which to dynamically translate
   the output file name of into elements on the .FILES list.

   Note that there is no inheritance for thigs like: dependencies, .FILE_DEPS,
   and .INCLUDE_DIRS. Those must and can be added to the component(s)
   using other components as an input.

   The primary use case is for MACROEXPAND, CAT, and CUSTOM component types
   to create source files for other types. See the test examples.enerateddeps
   for an example.

   Chaining of multiple layers can also be done. ie a conCATenation of MACROEXPANDS.
   (that needs a test).

   The other allowed use is a CUSTOM component may have a BINARY compoent as
   an INPUT. This simply creates a makefile dependency. The idea is the the .DRIVER
   script would make use of said BINARY.

 */
int derive_file_dependencies_from_inputs(struct bca_context *ctx,
                                         struct host_configuration *tc,
                                         struct component_details *cd)
{
 int i, x, y, handled, valid_input, add_file_name;
 char temp[1024], *base_file_name, *extension;

 if(cd->n_file_names == 0)
 {
  cd->file_names = NULL;
  cd->file_base_names = NULL;
  cd->file_extensions = NULL;
 }

 for(i=0; i < cd->n_inputs; i++)
 {
  valid_input = 0;

  /* what kind of component is this input? */
  handled = 0;
  x = 0;
  while(x < cd->n_components)
  {
   if(strcmp(cd->inputs[i], cd->project_components[x]) == 0)
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
           cd->project_component, cd->host, cd->inputs[i]);
   return 1;
  }

  valid_input = 0;
  add_file_name = 0;

  if(strcmp(cd->project_component_types[x], "CUSTOM") == 0)
  {
   valid_input = 1;
   add_file_name = 1;
  }

  if(strcmp(cd->project_component_types[x], "CAT") == 0)
  {
   valid_input = 1;
   add_file_name = 1;
  }

  if(strcmp(cd->project_component_types[x], "MACROEXPAND") == 0)
  {
   valid_input = 1;
   add_file_name = 1;
  }

  if(strcmp(cd->project_component_type, "CUSTOM") == 0)

  {
   if(strcmp(cd->project_component_types[x], "BINARY") == 0)
    valid_input = 1;
  }

  if(valid_input == 0)
  {
   fprintf(stderr,
           "BCA: I don't know what to do with input component %s of type %s "
           "for component %s of type %s on host %s.\n",
           cd->inputs[i], cd->project_component_types[x],
           cd->project_component, cd->project_component_type, cd->host);
   return 1;
  }

  if(add_file_name)
  {
   handled = 0;


   /*  Normally for binary files, etc we want to use render_project_component_output_name()
       to yield the correct file name for the host. Although here, we are only talking
       about generate source files.
    */
   snprintf(temp, 1024, "%s/%s",
            tc->build_prefix, cd->project_output_names[x]);

   if(path_extract(temp, &base_file_name, &extension))
   {
    return 1;
   }

   /* this needs to be updated everytime new file types are added, this really
      should be an array that is shared everywhere.
      We could skip this check since later when the component target rules get
      generated, a file type with an unkown corse of action will be reported,
      but we wont know that it was from an .INPUT and we wont get to deal with
      header files specially here.
   */
   if(strcmp(extension, "c") == 0)
   {
    /* Do the actual adding of the ouptut name to .FILES. At this point,
       base names and extensions have already been expanded for the other
       .FILES, so these need to be added here. */
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

   /*  If the input is a header file, then add it to the .FILE_DEPS since
       it that there is some code path for creating the correct makefile dependency.
       We need to also add the directory to the list of include paths.
    */
   if(strcmp(extension, "h") == 0)
   {
    if(add_to_string_array(&(cd->file_deps), cd->n_file_deps,
                           temp, -1, 0))
    {
     fprintf(stderr, "BCA: add_to_string_array() failed\n");
     return 1;
    }

    cd->n_file_deps++;

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
   if(strcmp(cd->project_component_type, "CAT") == 0)
    handled = 1;

   if(strcmp(cd->project_component_type, "MACROEXPAND") == 0)
    handled = 1;

   /* along the same lines CUSTOM types could be doing anything */
   if(strcmp(cd->project_component_type, "CUSTOM") == 0)
    handled = 1;

   if(handled == 0)
   {
    fprintf(stderr,
            "BCA: I don't know what to do with input component %s of type %s "
           "for component %s of type %s on host %s, that is yielding an "
           "output file with the extension '%s'.\n",
           cd->inputs[i], cd->project_component_types[x],
           cd->project_component, cd->project_component_type,
           cd->host, extension);
    return 1;
   }

   free(base_file_name);
   free(extension);
  }
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
 int yes, i, j, driver_component;

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
 driver_component = i;
 if(yes == 0)
 {
  fprintf(stderr,
          "BCA: CUSTOM.%s.DRIVER = %s does not seem to be a MACROEXPAND project component\n",
          cd->project_component, value);
  return 1;
 }
 free(value);

 if( (cd->file_names == NULL) && (cd->n_inputs == 0) )
 {
  fprintf(stderr, "BCA: CUSTOM component \"%s\", does not seem to have .INPUT or .FILES\n",
          cd->project_component);
  return 1;
 }

 fprintf(output, "%s : $(%s-FILE_DEPENDENCIES) ",
         output_file_name, output_file_name);

 for(i=0; i<cd->n_file_names; i++)
 {
  fprintf(output, "%s ", cd->file_names[i]);
 }

 for(i=0; i<cd->n_inputs; i++)
 {
  j=0;
  while(j<cd->n_components)
  {
   if(strcmp(cd->project_components[j], cd->inputs[i]) == 0)
    break;
   j++;
  }

  fprintf(output, "%s/%s ", tc->build_prefix, cd->project_output_names[j]);
 }

 fprintf(output, "%s/%s\n", tc->build_prefix, cd->project_output_names[driver_component]);

 fprintf(output, "\tchmod +x %s/%s\n",
         tc->build_prefix, cd->project_output_names[driver_component]);

 fprintf(output, "\t%s/%s ",
         tc->build_prefix, cd->project_output_names[driver_component]);

 for(j=0; j<cd->n_file_names; j++)
 {
  fprintf(output, "%s ", cd->file_names[j]);
 }

 for(i=0; i<cd->n_inputs; i++)
 {
  j=0;
  while(j<cd->n_components)
  {
   if(strcmp(cd->project_components[j], cd->inputs[i]) == 0)
    break;
   j++;
  }

  fprintf(output, "%s/%s ", tc->build_prefix, cd->project_output_names[j]);
 }

 fprintf(output, "%s\n\n", output_file_name);
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

 fprintf(output, "> %s\n", output_file_name);

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
 int i, x, y, n_names, swapped;
 char **names = NULL;

 /* any component type may have a FILE_DEPENDS key */
 fprintf(output, "# dependencies and build rules for %s-%s\n",
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

    if(strcmp(cd->project_component_types[x], "SHAREDLIBRARY"))
    {
     fprintf(stderr,
             "BCA: project component \"%s\" has an internal dependency on component \"%s\" which "
             "has an unknown dependency relationship. i.e. type \"%s\" is not a library\n",
             cd->project_component, cd->project_components[x], cd->project_component_types[x]);
     return 1;
    }

    if(swapped == 0)
    {
     if((n_names =
         render_project_component_output_name(ctx, cd->host, cd->project_components[x],
                                              2, &names, NULL)) < 2)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
      return 1;
     }
    } else {
     if((n_names =
         render_project_component_output_name(ctx, ctx->swapped_component_hosts[i],
                                              cd->project_components[x],
                                              2, &names, NULL)) < 2)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
      return 1;
     }
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

  if(component_pkg_config_path(ctx, cd, tc, output))
   return 1;

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
                                            char *pkg_file_name,
                                            char **output_file_names,
                                            int n_output_file_names,
                                            FILE *output,
                                            int installed_version)
{
 int x, i, yes, length;
 struct component_details cd_d;
 char *build_prefix, *package_name = NULL, *package_description = NULL, *link_name;

/*
   Idea / question:
    reconcille package config's --variable and --define-variable with BCA variables
*/

  package_name = lookup_key(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            cd->project_component_type,
                            cd->project_component,
                            "PACKAGE_NAME");

 if(package_name == NULL)
 {
  package_name = lookup_key(ctx,
                            ctx->project_configuration_contents,
                            ctx->project_configuration_length,
                            cd->project_component_type,
                            cd->project_component,
                            "NAME");
 }

 if(package_name == NULL)
 {
  package_name = strdup(ctx->project_name);
 }

 package_description = lookup_key(ctx,
                                  ctx->project_configuration_contents,
                                  ctx->project_configuration_length,
                                  cd->project_component_type,
                                  cd->project_component,
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
          cd->project_component, pkg_file_name);
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

 if(tc->ldflags != NULL)
  fprintf(output, " %s", tc->ldflags);

 fprintf(output, "' >> %s\n", pkg_file_name);

 fprintf(output, "\techo 'Cflags:");

 if(cd->n_include_dirs > 0)
  fprintf(output, " -I$${includedir} ");

 if(gmake_host_component_file_rule_cflags(ctx, output, cd, tc))
  return 1;

 fprintf(output, "' >> %s\n", pkg_file_name);

 /* add unversioned symlink */
 if(installed_version)
 {
  fprintf(output, "\tcd %s; ln -sf %s %s.pc\n",
          tc->install_pkg_config_dir, output_file_names[1],
          cd->project_component_output_name);

 } else {
  fprintf(output, "\tcd %s; ln -sf %s %s.pc\n",
          tc->build_prefix, output_file_names[1],
          cd->project_component_output_name);
 }

 if(package_description != NULL)
  free(package_description);

 if(package_name != NULL)
  free(package_name);

 return 0;
}

int generate_gmake_host_component_bins_and_libs(struct bca_context *ctx,
                                                struct component_details *cd,
                                                struct host_configuration *tc,
                                                char **build_file_names,
                                                int n_build_file_names,
                                                FILE *output)
{
 char **dependency_build_filenames, **output_file_names;
 int i, x, y, handled, yes, n_dependency_build_filenames,
     n_output_file_names, swapped;
 struct component_details cd_d;

 memset(&cd_d, 0, sizeof(struct component_details));

 if((n_output_file_names =
     render_project_component_output_name(ctx, cd->host, cd->project_component,
                                          1, &output_file_names, NULL)) < 0)
 {
  fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
  return 1;
 }

 /* first build targets for each object file */
 for(i=0; i<cd->n_file_names; i++)
 {
  handled = 0;

  if(strcmp(cd->file_extensions[i], "c") == 0)
  {
   if(object_from_c_file(ctx, cd, tc,
                         cd->file_base_names[i],
                         cd->file_names[i],
                         build_file_names[0],
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
 fprintf(output, "%s-OBJECTS = ", build_file_names[0]);

 for(i=0; i < cd->n_file_names; i++)
 {
  fprintf(output, "%s/obj/%s-%s",
          tc->build_prefix, cd->project_component, cd->file_base_names[i]);

  fprintf(output, "%s ", tc->obj_suffix);
 }
 fprintf(output, "\n\n");

 /* now make the component depend on its objects */
 fprintf(output, "%s : $(%s-OBJECTS) ", build_file_names[0], build_file_names[0]);

 /* the component's target also depends on its internal dependencies targets */

 for(y=0; y < cd->n_dependencies; y++)
 {
  x = 0;
  while(x < cd->n_components)
  {
   if(strcmp(cd->dependencies[y], cd->project_components[x]) == 0)
   {
    /* internel dep */
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
     if((n_dependency_build_filenames =
         render_project_component_output_name(ctx, cd->host, cd->project_components[x],
                                              2, &dependency_build_filenames, NULL)) < 2)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
      return 1;
     }
    } else {
     if((n_dependency_build_filenames =
         render_project_component_output_name(ctx, ctx->swapped_component_hosts[i],
                                              cd->project_components[x],
                                              2, &dependency_build_filenames, NULL)) < 2)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed on internal dep\n");
      return 1;
     }
    }

    fprintf(output, "%s ", dependency_build_filenames[1]);

    free_string_array(dependency_build_filenames, n_dependency_build_filenames);
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
         build_file_names[0], tc->cc_output_flag);

 /* figure out what the name of the output file should be,
    different for libraries and binaries */
 yes = 1;
 if(n_build_file_names > 3)
 {
  if(build_file_names[3][0] != 0)
  {
   yes = 0;
   fprintf(output, " %s", build_file_names[3]);
  }
 }

 if(yes == 1)
  fprintf(output, " %s", build_file_names[0]);

 /* add shared libary switches */
 if(strcmp(cd->project_component_type, "SHAREDLIBRARY") == 0)
 {
  if(contains_string(tc->cc, -1, "mingw", -1))
  {
   if(n_build_file_names < 3)
   {
    fprintf(stderr, "BCA: there should have been an import name here\n");
    return 1;
   }
   fprintf(output, " -Wl,--out-implib,%s", build_file_names[2]);
  } else {
   if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                           cd->project_component_output_name), ".dll") == 0)
   {
    /* cygwin */
    if(n_build_file_names < 3)
    {
     fprintf(stderr, "BCA: there should have been an import name here\n");
     return 1;
    }
    fprintf(output, " -Wl,--out-implib,%s",
            build_file_names[2]);
   }
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".dylib") == 0)
  {
   fprintf(output, " -compatibility_version %s.%s -install_name %s",
           cd->major, cd->minor, build_file_names[0]);
  }

  if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                          cd->project_component_output_name), ".so") == 0)
  {
   fprintf(output, " -Wl,-soname,%s",
           output_file_names[0]);
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

  fprintf(output, "%s : %s\n",
          build_file_names[1], build_file_names[0]);

  if(generate_host_component_pkg_config_file(ctx, cd, tc,
                                             build_file_names[1],
                                             output_file_names,
                                             n_output_file_names,
                                             output, 0))
  {
   fprintf(stderr,
           "BCA: generate_host_component_pkg_config_file(%s.%s.%s) failed\n",
           cd->project_component_type, cd->host, cd->project_component);
   return 1;
  }
 }

 free_string_array(output_file_names, n_output_file_names);

 return 0;
}

int generate_gmake_host_component_file_rules(struct bca_context *ctx, FILE *output,
                                             struct component_details *cd)
{
 char **names;
 int handled, n_names;
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
 int n_hosts, host_i, component_i, i, handled, n_file_deps, n_include_dirs,
     n_lib_headers, swapped;
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
 fprintf(output, "\n\n\n");

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

 if(generate_gmake_install_rules(ctx, output, hosts, n_hosts, &cd, 0))
 {
  fclose(output);
  return 1;
 }

 if(generate_gmake_install_rules(ctx, output, hosts, n_hosts, &cd, 1))
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

  if(check_duplicate_output_names(ctx, &cd))
  {
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

  component_i=0;
  while(component_i < cd.n_components)
  {

   swapped = 0;
   i = 0;
   while(i<ctx->n_swaps)
   {
    if(strcmp(cd.project_components[component_i],
              ctx->swapped_components[i]) == 0)
    {
     swapped = 1;
     break;
    }
    i++;
   }

   if(swapped == 1)
   {
    component_i++;
    continue;
   }

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

   component_i++;
  } // while() compoents

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
                                 struct component_details *cd,
                                 int uninstall_version)
{
 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: generate_gmake_install_rules()\n");

 int x, y, n_build_names, n_install_names, n_output_names, i,
     length, index, yes, swapped, n_lib_headers;
 struct host_configuration *tc = NULL;
 char temp[1024], **build_names, *install_headers_dir, *base_filename, *extension,
      **install_names, *value, **output_names, **lib_headers;
 struct component_details cd_d;

 if(ctx->verbose > 2)
  fprintf(stderr, "BCA: gmake_install_rules()\n");

 memset(&cd_d, 0, sizeof(struct component_details));

 if(uninstall_version)
 {
  fprintf(output, "# uninstall rules\n");
  fprintf(output, "uninstall : ");
 } else {
  fprintf(output, "# install rules\n");
  fprintf(output, "install : ");
 }

 for(x=0; x<n_build_hosts; x++)
 {
  if(uninstall_version)
   fprintf(output, "%s-unintstall ", hosts[x]);
  else
   fprintf(output, "%s-intstall ", hosts[x]);
 }
 fprintf(output, "\n\n");

 for(x=0; x<n_build_hosts; x++)
 {

  if(engage_build_configuration_disables_for_host(ctx, hosts[x]))
  {
   fprintf(stderr,
           "BCA: engage_build_configuration_disables_for_host(%s) failed\n", hosts[x]);
   return 1;
  }

  if(engage_build_configuration_swaps_for_host(ctx, hosts[x]))
  {
   fclose(output);
   return 1;
  }

  if(list_project_components(ctx, cd))
  {
   fprintf(stderr, "BCA: list_project_components() failed\n");
   return 1;
  }

  if(uninstall_version)
   fprintf(output, "%s-unintstall: ", hosts[x]);
  else
   fprintf(output, "%s-intstall: ", hosts[x]);

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
    if((n_install_names =
        render_project_component_output_name(ctx, hosts[x],
                                             cd->project_components[y], 3,
                                             &install_names, NULL)) < 0)
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed ([un]install targets)\n");
     return 1;
    }

    if(n_install_names > 0)
    {
     if(uninstall_version)
      fprintf(output, "%s-%s-uninstall ", hosts[x], cd->project_components[y]);
     else
      fprintf(output, "%s-%s-install ", hosts[x], cd->project_components[y]);
     free_string_array(install_names, n_install_names);
    }
   }

  }
  fprintf(output, "\n\n");

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
    cd->project_component_output_name = cd->project_output_names[y];
    cd->host = hosts[x];

    if((tc = resolve_host_configuration(ctx, cd)) == NULL)
    {
     fprintf(stderr, "BCA: resolve_host_configuration() failed\n");
     return 1;
    }

    if((n_install_names =
        render_project_component_output_name(ctx, hosts[x],
                                             cd->project_components[y], 3,
                                             &install_names, NULL)) < 0)
    {
     fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
     return 1;
    }

    if(n_install_names > 0)
    {

     if((n_build_names =
         render_project_component_output_name(ctx, hosts[x],
                                              cd->project_components[y], 2,
                                              &build_names, NULL)) < 0)
     {
      fprintf(stderr, "BCA: render_project_component_ouput_name() failed\n");
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

     if(uninstall_version)
      fprintf(output, "%s-%s-uninstall :\n",
              hosts[x], cd->project_components[y]);
     else
     fprintf(output, "%s-%s-install : %s-%s\n",
             hosts[x], cd->project_components[y], hosts[x], cd->project_components[y]);

     if(strcmp(cd->project_component_types[y], "BINARY") == 0)
     {
      if(uninstall_version)
       fprintf(output, "\trm %s\n",
               install_names[0]);
      else
       fprintf(output, "\tinstall %s %s\n",
               build_names[0], install_names[0]);

     } else if(strcmp(cd->project_component_types[y], "CAT") == 0) {
      if(uninstall_version)
       fprintf(output, "\trm %s\n",
               install_names[0]);
      else
       fprintf(output, "\tinstall %s %s\n",
               build_names[0], install_names[0]);

     } else if(strcmp(cd->project_component_types[y], "MACROEXPAND") == 0) {
      if(uninstall_version)
       fprintf(output, "\trm %s\n",
               install_names[0]);
      else
       fprintf(output, "\tinstall %s %s\n",
               build_names[0], install_names[0]);

     } else if(strcmp(cd->project_component_types[y], "CUSTOM") == 0) {
      if(uninstall_version)
       fprintf(output, "\trm %s\n",
               install_names[0]);
      else
       fprintf(output, "\tinstall %s %s\n",
               build_names[0], install_names[0]);

     } else if(strcmp(cd->project_component_types[y], "SHAREDLIBRARY") == 0) {

      if(resolve_component_version(ctx,
                                   ctx->project_configuration_contents,
                                   ctx->project_configuration_length, cd,
                                   "SHAREDLIBRARY", cd->project_components[y]))
      {
       fprintf(stderr, "BCA: resolve_component_version() failed\n");
       return 1;
      }

      n_lib_headers = 0;
      lib_headers = NULL;
      install_headers_dir = NULL;
      if((value = lookup_key(ctx,
                             ctx->project_configuration_contents,
                             ctx->project_configuration_length,
                             cd->project_component_types[y],
                             cd->project_components[y],
                             "LIB_HEADERS")) != NULL)
      {
       if(split_strings(ctx, value, -1, &n_lib_headers, &lib_headers))
       {
        fprintf(stderr, "BCA: split_strings() failed on '%s'\n", value);
        return 1;
       }

       if((install_headers_dir =
            lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       hosts[x], cd->project_components[y], "INSTALL_INCLUDE_DIR")) == NULL)
       {
        if((install_headers_dir =
            lookup_key(ctx, ctx->build_configuration_contents,
                       ctx->build_configuration_length,
                       hosts[x], "ALL", "INSTALL_INCLUDE_DIR")) == NULL)
        {
         fprintf(stderr, "BCA: I need a value for %s.[%s/ALL].INSTALL_INCLUDE_DIR\n",
                 hosts[x], cd->project_components[y]);
         return 1;
        }
       }
      }

      if(uninstall_version)
      {
       for(i=0; i<n_install_names; i++)
       {
        if(install_names[i][0] != 0)
         fprintf(output, "\trm %s\n", install_names[i]);
       }
      } else {

       if(n_install_names > 3)
       {
        fprintf(output, "\tinstall %s %s\n", build_names[3], install_names[3]);

        if(strcmp(component_type_file_extension(ctx, tc, cd->project_component_type,
                                                cd->project_component_output_name), ".dll") != 0)
        {
         fprintf(output, "\tcd %s; ln -sf %s %s\n",
                 tc->install_lib_dir, output_names[3], output_names[0]);
        }
       }

       if(generate_host_component_pkg_config_file(ctx, cd, tc,
                                                  install_names[1],
                                                  output_names, n_output_names,
                                                  output, 1))
       {
        fprintf(stderr, "BCA: generate_host_component_pkg_config_file() failed\n");
        return 1;
       }

      }

      for(i=0; i<n_lib_headers; i++)
      {
       base_filename = NULL;
       extension = NULL;
       if(path_extract(lib_headers[i], &base_filename, &extension))
       {
        fprintf(stderr, "BCA: path_extract(%s) failed\n", lib_headers[i]);
        return 1;
       }

       snprintf(temp, 1024, "%s/%s.%s",
                install_headers_dir, base_filename, extension);
       free(base_filename);
       free(extension);

       if(uninstall_version)
        fprintf(output, "\trm %s\n", temp);
       else
        fprintf(output, "\tinstall %s %s\n", lib_headers[i], temp);
      }

      if(install_headers_dir != NULL)
       free(install_headers_dir);
      free_string_array(lib_headers, n_lib_headers);
     }

     fprintf(output, "\n");
     free_string_array(output_names, n_output_names);
     free_string_array(build_names, n_build_names);
     free_string_array(install_names, n_install_names);
    }

    free_host_configuration(ctx, tc);
   }
  }

 }

 fprintf(output, "\n");
 return 0;
}


