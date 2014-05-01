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

int resolve_component_installation_path(struct bca_context *ctx, char *component_type,
                                        char *component, char **path);

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

