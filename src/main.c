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

#ifndef WITHOUT_GTK__2_0
#include <gtk/gtk.h>
#define HAVE_GTK
#endif


void help(void)
{
 printf("\n                         Build Configuration Adjust\n"
        "This is part of the build system, and not part of the software it builds.\n"
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
        " --generate-graphviz\n"
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
#ifndef IN_SINGLE_FILE_DISTRIBUTION
        " --selftest (help debug buildconfigurationadjust itself)\n"
#endif
#ifndef WITHOUT_LIBNEWT
        " --newt-interface\n"
#endif
#ifndef WITHOUT_MONGOOSE
        " --mongoose-interface\n"
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

        "\n Copyright 2012 Stover Enterprises. All rights reserved. Licensed under the terms of GNU GPLv3.\n"
        "\tSee http://bca.stoverenterprises.com for more information.\n\n");

}

int main(int argc, char **argv)
{
 struct bca_context *ctx;
 char *value, *contents, *file, **list = NULL;
 int length, offset, n_items = 0, i;
 char principle[256], qualifier[256], key[256], code;
 struct component_details cd;
 FILE *output;

 memset(&cd, 0, sizeof(struct component_details));

 if((ctx = setup(argc, argv)) == NULL)
  return 1;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: version 0.01\n");

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
           printf("BCA: SET_VALUE_MODE finished - value unchanged for build configuration - noop\n");
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

/*
this doesn't make any sense. There does need to be some detection for .NAME conflicts somewhere though.

       offset = -1;
       while(iterate_key_primitives(ctx, contents, length, &offset,
                                    NULL, NULL, ctx->search_value_key,
                                    principle, qualifier, key, NULL))
       {
        if((value = lookup_key(ctx, contents, length, principle, qualifier, key)) != NULL)
        {

         if(strcmp(qualifier, ctx->qualifier) == 0)
         {
          fprintf(stderr,
                  "BCA: Conflict in file %s: %s.%s.%s = %s conflicts. "
                  "Try another component name\n.",
                  file, principle, qualifier, key, value);

          free(value);
          return 1;
         }

         if(strcmp(value, ctx->new_value_string) == 0)
         {
          fprintf(stderr,
                  "BCA: Conflict in file %s: %s.%s.%s = %s conflicts. "
                  "Try another output name\n.",
                  file, principle, qualifier, key, value);
 
          free(value);
          return 1;
         }
        }
        free(value);
       }
*/
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

  case GENERATE_GRAPHVIZ_MODE:
       if( ((code = generate_graphviz_mode(ctx)) == 0) &&
           (ctx->verbose> 1) )
        printf("BCA: generate_graphviz_mode() finished\n");
       return code;
       break;

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

#ifndef WITHOUT_MONGOOSE
  case MONGOOSE_INTERFACE_MODE:
       if( ((code = mongoose_interface(ctx)) == 0) &&
           (ctx->verbose > 1))
        fprintf(stderr, "BCA: mongoose_interfaace() finished\n");
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
 int allocation_size, current_arg = 1, handled;

 allocation_size = sizeof(struct bca_context);
 if((ctx = (struct bca_context *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
  return NULL;
 }

 memset(ctx, 0, allocation_size);
 ctx->manipulation_type = MANIPULATE_BUILD_CONFIGURATION;

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
   handled = 1;
   ctx->mode = GENERATE_GRAPHVIZ_MODE;
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
           "BCA: This build configuration adjust was not build with support "
           "for the newt interface. If you want to use this feature, consider "
           "installing a local copy on this system or somewhere in your $PATH.\n");
   return NULL;
#endif
  }

  if(strcmp(argv[current_arg], "--mongoose-interface") == 0)
  {
#ifndef WITHOUT_MONGOOSE
   handled = 1;
   ctx->mode = MONGOOSE_INTERFACE_MODE;
#else
   fprintf(stderr, 
           "BCA: This build configuration adjust was not build with support "
           "for the mongoose interface. If you want to use this feature, consider "
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
    fprintf(stderr, "BCA: you must key string with --host\n");
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
   fprintf(stderr, "BCA: I don't know what to do with the parameter \"%s\".\n", argv[current_arg]);
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



