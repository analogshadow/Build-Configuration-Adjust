/* Begin selftest.c ------------------------------------------ (sfd organizer) */

/* GPLv3

    Build Configuration Adjust, is a source configuration and Makefile
    generation tool.
    Copyright © 2012,2013,2014,2015 C. Thomas Stover.
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

int setup_test(struct bca_context *real_ctx)
{
 struct bca_context *test_ctx;
 int argc;
 char *argv[16];

 printf("suite: setup()\n");

 printf("test: noargs: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(shutdown(test_ctx))
  {
   printf("failed: shutdown() error\n");
  } else {
   printf("passed\n");
  }
 }

 printf("test: modeshorthelp: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--help=short";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != SHORT_HELP_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: verbose: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "-v";
 argv[argc++] = "--verbose";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->verbose != 2)
  {
   printf("failed: wrong verbosity level\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modegengraphviz: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--generate-graphviz";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != GENERATE_GRAPHVIZ_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modeconfigure: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--configure";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != CONFIGURE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modecat: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--concatenate";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != CONCATENATE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modereplacestrings: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--replacestrings";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != STRING_REPLACE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modegemmakefile: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--generate-gmakefile";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != GENERATE_GMAKEFILE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modelistprojcomponents: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--listprojectcomponents";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != LIST_PROJECT_COMPONENTS_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modelistbildhosts: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--listbuildhosts";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != LIST_HOSTS_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modelistprojtypes: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--listprojecttypes";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != LIST_PROJECT_TYPES_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modenewproj: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--newproject";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != NEW_PROJECT_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }

 printf("test: modevalueremove: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--removevalue";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->mode != REMOVE_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevalueshow: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--showvalue";
 argv[argc++] = "abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != SHOW_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevaluecheck: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--checkvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "taco";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != CHECK_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevalueset: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--setvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != SET_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->new_value_string, "tacofao") != 0) {
  printf("failed: wrong value data\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevalueadd: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--addvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != ADD_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevaluesmartadd: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--smartaddvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != SMART_ADD_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->new_value_string, "tacofao") != 0) {
  printf("failed: wrong value data\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevaluepull: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--pullvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != PULL_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->new_value_string, "tacofao") != 0) {
  printf("failed: wrong value data\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevaluesmartpull: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--smartpullvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != SMART_PULL_VALUE_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->new_value_string, "tacofao") != 0) {
  printf("failed: wrong value data\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modevaluenew: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--newvalue";
 argv[argc++] = "abcdefg";
 argv[argc++] = "tacofao";
 if((test_ctx = setup(argc, argv)) == NULL) {
  printf("failed\n");
 } else if(test_ctx->mode != NEW_COMPONENT_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->search_value_key, "abcdefg") != 0) {
  printf("failed: wrong value key\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->new_value_string, "tacofao") != 0) {
  printf("failed: wrong value data\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifyproject: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--project";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->manipulation_type != OPERATE_PROJECT_CONFIGURATION) {
  printf("failed: wrong manipulation type\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifybuild: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--build";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->manipulation_type != OPERATE_BUILD_CONFIGURATION) {
  printf("failed: wrong manipulation type\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifytype: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--type";
 argv[argc++] = "abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->manipulation_type != OPERATE_PROJECT_CONFIGURATION) {
  printf("failed: wrong manipulation type\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->principle, "abcdefg") != 0) {
  printf("failed: wrong principle value\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifyhost: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--host";
 argv[argc++] = "abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->manipulation_type != OPERATE_BUILD_CONFIGURATION) {
  printf("failed: wrong manipulation type\n");
  shutdown(test_ctx);
 } else if(strcmp(test_ctx->principle, "abcdefg") != 0) {
  printf("failed: wrong principle value\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifycomponent: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--component";
 argv[argc++] = "abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(strcmp(test_ctx->qualifier, "abcdefg") != 0) {
  printf("failed: wrong component value\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modelistcomptoutnames: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--component";
 argv[argc++] = "--abcdefg";
 argv[argc++] = "--componentoutputnames";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->mode != LIST_COMPONENT_OUTPUT_NAMES_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modelistbuildoutnames: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--component";
 argv[argc++] = "--abcdefg";
 argv[argc++] = "--componentbuildoutputnames";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->mode != LIST_COMPONENT_BUILD_OUTPUT_NAMES_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: modelistinstoutnames: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--component";
 argv[argc++] = "--abcdefg";
 argv[argc++] = "--componentinstallnames";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->mode != LIST_COMPONENT_INSTALL_OUTPUT_NAMES_MODE) {
  printf("failed: wrong mode\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifyinstallprefix: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--prefix=abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(strcmp(test_ctx->install_prefix, "abcdefg") != 0) {
  printf("failed: wrong install prefix value\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifyhostprefix: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--host=abcdefg";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(strcmp(test_ctx->host_prefix, "abcdefg") != 0) {
  printf("failed: wrong host prefix value\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifywithouts: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--without-alpha";
 argv[argc++] = "--without-beta";
 argv[argc++] = "--without-gama";
 argv[argc++] = "--without-delta";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->n_withouts != 4) {
  printf("failed: wrong n_withouts value\n");
  shutdown(test_ctx);
 } else if( (strcmp(test_ctx->without_strings[0], "alpha") != 0) ||
            (strcmp(test_ctx->without_strings[1], "beta") != 0) ||
            (strcmp(test_ctx->without_strings[2], "gama") != 0) ||
            (strcmp(test_ctx->without_strings[3], "delta") != 0) ) {
  printf("failed: wrong without_strings array\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

 printf("test: specifydisables: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--disable-alpha";
 argv[argc++] = "--disable-beta";
 argv[argc++] = "--disable-gama";
 argv[argc++] = "--disable-delta";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else if(test_ctx->n_disables != 4) {
  printf("failed: wrong n_disables value\n");
  shutdown(test_ctx);
 } else if( (strcmp(test_ctx->disabled_components[0], "alpha") != 0) ||
            (strcmp(test_ctx->disabled_components[1], "beta") != 0) ||
            (strcmp(test_ctx->disabled_components[2], "gama") != 0) ||
            (strcmp(test_ctx->disabled_components[3], "delta") != 0) ) {
  printf("failed: wrong without_strings array\n");
  shutdown(test_ctx);
 } else if(shutdown(test_ctx)) {
  printf("failed: shutdown() error\n");
 } else {
  printf("passed\n");
 }

#ifndef WITHOUT_LIBNEWT
 printf("test: modenewt: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--newt-interface";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != NEWT_INTERFACE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }
#endif

#ifndef WITHOUT_GTK
 printf("test: modegtk: ");
 test_ctx = NULL;
 argc = 0;
 argv[argc++] = "bca";
 argv[argc++] = "--gtk-interface";
 if((test_ctx = setup(argc, argv)) == NULL)
 {
  printf("failed\n");
 } else {
  if(test_ctx->mode != GTK_INTERFACE_MODE)
  {
   printf("failed: wrong mode\n");
   shutdown(test_ctx);
  } else {
   if(shutdown(test_ctx))
   {
    printf("failed: shutdown() error\n");
   } else {
    printf("passed\n");
   }
  }
 }
#endif

 return 0;
}

int string_test(struct bca_context *real_ctx)
{
 char *out, *in, **array, *base, *extension;
 int n_elements, length, start, end;
 FILE *f;

 printf("suite: strings\n");

 printf("test: escapenull: ");
 in = NULL;
 if((out = escape_value(real_ctx, in, -1)) != NULL)
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: escapeempty: ");
 in = "";
 if((out = escape_value(real_ctx, in, -1)) == NULL)
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }
 if(in != out)
  free(out);

 printf("test: escapenotneeded: ");
 in = "alpha";
 if((out = escape_value(real_ctx, in, -1)) == NULL)
 {
  printf("failed\n");
 } else if (strcmp(out, "alpha") != 0) {
  printf("failed: wrong output\n");
 } else {
  printf("passed\n");
 }
 if(in != out)
  free(out);

 printf("test: escapeqoutes: ");
 in = "a\"lp\"h\"a";
 if((out = escape_value(real_ctx, in, -1)) == NULL)
 {
  printf("failed\n");
 } else if (strcmp(out, "a\\\"lp\\\"h\\\"a") != 0) {
  printf("failed: wrong output\n");
 } else {
  printf("passed\n");
 }
 if(in != out)
  free(out);

 printf("test: escapespaces: ");
 in = "al pha";
 if((out = escape_value(real_ctx, in, -1)) == NULL)
 {
  printf("failed\n");
 } else if (strcmp(out, "\"al pha\"") != 0) {
  printf("failed: wrong output\n");
 } else {
  printf("passed\n");
 }
 if(in != out)
  free(out);

 printf("test: escapemixed: ");
 in = "al ph\"a";
 if((out = escape_value(real_ctx, in, -1)) == NULL)
 {
  printf("failed\n");
 } else if (strcmp(out, "\"al ph\\\"a\"") != 0) {
  printf("failed: wrong output\n");
 } else {
  printf("passed\n");
 }
 if(in != out)
  free(out);

 printf("test: containsnullsource: ");
 if(contains_string(NULL, -1, "abc", -1))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: containsnullsearch: ");
 if(contains_string("alpaabcdelta", -1, NULL, -1))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: containsnot: ");
 if(contains_string("alpaabcdelta", -1, "xyz", -1))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: containstring: ");
 if(contains_string("alpaabcdelta", -1, "abc", -1) == 0)
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: stringarrayaddnull: ");
 array = NULL;
 n_elements = 0;
 if(add_to_string_array(&array, n_elements, NULL, -1, 0) != 0)
 {
  printf("failed\n");
 } else if(array[n_elements] != NULL) {
  printf("failed: element not NULL\n");
 } else {
  printf("passed\n");
  n_elements++;
  printf("test: stringarrayaddstr: ");

  if(add_to_string_array(&array, n_elements, "beta", -1, 0) != 0)
  {
   printf("failed\n");
  } else if(strcmp(array[n_elements], "beta") != 0) {
   printf("failed: element wrong value\n");
  } else {
   printf("passed\n");
   n_elements++;
  }
 }

 printf("test: freestringarray: ");
 if(free_string_array(array, n_elements))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: stringarraynoduplicates: ");
 array = NULL;
 n_elements = 0;
 if(add_to_string_array(&array, n_elements, "alpha", -1, 1) != 0)
 {
  printf("failed: first unique\n");
 } else {
  n_elements++;
  if(add_to_string_array(&array, n_elements, "beta", -1, 1) != 0)
  {
   printf("failed: second unique\n");
  } else {
   n_elements++;

   if(add_to_string_array(&array, n_elements, "alpha", -1, 1) != 1)
   {
    printf("failed: duplicate\n");
    n_elements++;
   } else {
    printf("passed\n");
   }
  }
  free_string_array(array, n_elements);
 }

 /* the test is that this should not crash, not work */
 printf("test: pathextractnullpath: ");
 if(path_extract(NULL, NULL, NULL) == 0)
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: pathextractnulloutput: ");
 if(path_extract("/dir1/dir2/dir3/file.ext", NULL, NULL))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: pathextractabsolute: ");
 if(path_extract("/dir1/dir2/dir3/file.ext", &base, &extension))
 {
  printf("failed\n");
 } else if (base == NULL) {
  printf("failed: base == NULL\n");
 } else if (extension == NULL) {
  printf("failed: extension == NULL\n");
 } else if (strcmp(base, "file")) {
  printf("failed: base is wrong\n");
 } else if (strcmp(extension, ".ext")) {
  printf("failed: extension is wrong\n");
 } else {
  printf("passed\n");
 }
 if(base != NULL)
  free(base);
 if(extension != NULL)
  free(extension);

 printf("test: pathextractnoext: ");
 if(path_extract("../../file", &base, &extension))
 {
  printf("failed\n");
 } else if (base == NULL) {
  printf("failed: base == NULL\n");
 } else if (extension != NULL) {
  printf("failed: extension should be NULL\n");
 } else if (strcmp(base, "file")) {
  printf("failed: base is wrong, '%s'\n", base);
 } else {
  printf("passed\n");
 }
 if(base != NULL)
  free(base);
 if(extension != NULL)
  free(extension);

 printf("test: splitstringsnull: ");
 in = NULL;
 if(split_strings(real_ctx, in, -1, &n_elements, &array))
 {
  printf("failed\n");
 } else if (n_elements != 0) {
  printf("failed: wrong number of substrings\n");
 } else {
  printf("passed\n");
 }

 printf("test: splitstringsempty: ");
 in = "";
 if(split_strings(real_ctx, in, -1, &n_elements, &array))
 {
  printf("failed\n");
 } else if (n_elements != 0) {
  printf("failed: wrong number of substrings\n");
 } else {
  printf("passed\n");
 }

 printf("test: splitstringscontiguous: ");
 in = "alpha";
 if(split_strings(real_ctx, in, -1, &n_elements, &array))
 {
  printf("failed\n");
 } else if (n_elements != 1) {
  printf("failed: wrong number of substrings\n");
 } else if (strcmp(array[0], "alpha") != 0) {
  printf("failed: array element 0 is wrong\n");
 } else {
  printf("passed\n");
 }
 printf("test: splitstringsfreearray: ");
 if(free_string_array(array, n_elements))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: splitstringslist: ");
 in = "alpha beta gama delta";
 if(split_strings(real_ctx, in, -1, &n_elements, &array))
 {
  printf("failed\n");
 } else if (n_elements != 4) {
  printf("failed: wrong number of substrings\n");
 } else if (strcmp(array[0], "alpha") != 0) {
  printf("failed: array element 0 is wrong\n");
 } else if (strcmp(array[1], "beta") != 0) {
  printf("failed: array element 0 is wrong\n");
 } else if (strcmp(array[2], "gama") != 0) {
  printf("failed: array element 0 is wrong\n");
 } else if (strcmp(array[3], "delta") != 0) {
  printf("failed: array element 0 is wrong\n");
 } else {
  printf("passed\n");
 }
 free_string_array(array, n_elements);

 printf("test: readfilenonexistant: ");
 if((out = read_file("./doesnotexist", &length, 1)) != NULL)
 {
  printf("failed\n");
  free(out);
 } else {
  printf("passed\n");
 }

 printf("test: readfileblank: ");
 if((f = fopen("./doesexist", "w")) == NULL)
 {
  printf("failed: fopen() failed: %s\n", strerror(errno));
 } else {
  if((out = read_file("./doesexist", &length, 1)) == NULL)
  {
   printf("failed\n");
  } else if (length != 0) {
   printf("failed: output not 0 length\n");
  } else {
   printf("passed\n");
  }
  if(out != NULL)
   free(out);
 }

 printf("test: readfilesmall: ");
 fprintf(f, "hello world");
 fclose(f);
 if((out = read_file("./doesexist", &length, 1)) == NULL)
 {
  printf("failed\n");
 } else if (length != 11) {
  printf("failed: length is wrong\n");
 } else if (strcmp(out, "hello world") != 0) {
  printf("failed: file contents read wrong");
 } else {
  printf("passed\n");
 }
 if(out != NULL)
  free(out);

 unlink("./doesexist");

 printf("test: findlinenull: ");
 in = NULL;
 start = 0;
 end = -1;
 length = -1;
 if(find_line(in, -1, &start, &end, &length))
 {
  printf("failed\n");
 } else if (length != 0) {
  printf("failed: wrong length\n");
 } else {
  printf("passed\n");
 }

 printf("test: findlineempty: ");
 in = "";
 start = 0;
 end = -1;
 length = -1;
 if(find_line(in, 0, &start, &end, &length))
 {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: findlinepartial: ");
 in = "this has no newline char";
 start = 0;
 end = -1;
 length = -1;
 if(find_line(in, 24, &start, &end, &length) == 0)
 {
  printf("failed\n");
 } else if (strncmp(in + start, "this has no newline char", length) != 0) {
  printf("failed: wrong result\n");
 } else {
  printf("passed\n");
 }

 printf("test: findlinesimple: ");
 in = "line one\nline two\nanother line\n";
 start = 0;
 end = -1;
 length = -1;
 if(find_line(in, 21, &start, &end, &length) == 0)
 {
  printf("failed: couldn't find the first line\n");
 } else if (length != 8) {
  printf("failed: wrong length\n");
 } else if (strncmp(in + start, "line one", length) != 0) {
  printf("failed: first line is wrong\n");
 } else if (find_line(in, 21, &start, &end, &length) == 0) {
  printf("failed: couldn't find the second line\n");
 } else if (strncmp(in + start, "line two", length) != 0) {
  printf("failed: second line is wrong\n");
 } else if (find_line(in, 21, &start, &end, &length) == 0) {
  printf("failed: couldn't find the third line\n");
 } else if (strncmp(in + start, "another line", length) != 0) {
  printf("failed: second line is wrong\n");
 } else {
  printf("passed\n");
 }

 return 0;
}

/* see that both left and right are logically the same */
int compare_config_files(struct bca_context *ctx,
                         char *left_file, int left_length,
                         char *right_file, int right_length)
{
 int offset, equals_pos, n_records;
 char principle[256], component[256], key[256], *left_value, *right_value, **left_records,
      temp[772];

 n_records = 0;
 left_records = NULL;
 offset = -1;
 while(iterate_key_primitives(ctx, left_file, left_length, &offset, NULL, NULL, NULL,
                              principle, component, key, &equals_pos))
 {
  if((left_value = lookup_key(ctx, left_file, left_length,
                              principle, component, key)) == NULL)
  {
   return -1;
  }

  if((right_value = lookup_key(ctx, right_file, right_length,
                               principle, component, key)) == NULL)
  {
   free(left_value);
   free_string_array(left_records, n_records);
   return -1;
  }

  if(strcmp(left_value, right_value) != 0)
  {
   free(right_value);
   free(left_value);
   free_string_array(left_records, n_records);
   return 1;
  }

  free(right_value);
  free(left_value);

  snprintf(temp, 772, "%s.%s.%s", principle, component, key);
  if(add_to_string_array(&left_records, n_records, temp, -1, 1) != 0)
  {
   free_string_array(left_records, n_records);
   return -1;
  }
 }

 /* now we need to see that there are not any records in right file that are not in left file */
 while(iterate_key_primitives(ctx, left_file, left_length, &offset, NULL, NULL, NULL,
                              principle, component, key, &equals_pos))
 {
  snprintf(temp, 772, "%s.%s.%s", principle, component, key);
  if(add_to_string_array(&left_records, n_records, temp, -1, 1) == 0)
  {
   /* successfully adding to the array with the prevent duplicates flag, means that there was no
      duplicate - ie that left file did not have this record. */
   free_string_array(left_records, n_records);
   return 1;
  }
 }

 free_string_array(left_records, n_records);
 return 0;
}

int config_files_test(struct bca_context *real_ctx)
{
 int offset, equals_pos, file_length, n_elements, in_length, code;
 char principle[256], component[256], key[256], *file, *out, **array,
      **principle_modifications, **component_modifications, **key_modifications,
      **value_modifications, *in_file, *right_value;
 FILE *output;

 printf("suite: config_files\n");

 file = "alpha.blue.one = tacos\n"
        "alpha.green.two = rats\n"
        "beta.white.six = birds\n"
        "alpha.blue.nine = snakes\n"
        "\n"
        "beta.purple.five = pizza\n"
        "gama.purple.ten = salad\n"
        "gama.blue.eleven = robot\n"
        "delta.orange.eight = glue\n"
        "delta.black.two = towel\n"
        "NONE.NONE.PROJECT_NAME = foobar\n";

 file_length = strlen(file);

 printf("test: iterkeyprimsunfiltered: ");
 offset = -1;
 if(iterate_key_primitives(real_ctx, file, file_length, &offset,
                           NULL, NULL, NULL,
                           principle, component, key, &equals_pos) == 0)
 {
  printf("failed\n");
 } else if( (strcmp(principle, "alpha") != 0) ||
            (strcmp(component, "blue") != 0) ||
            (strcmp(key, "one") != 0) ||
            (strncmp(file + equals_pos + 2, "tacos", 5) != 0) ) {
  printf("failed: first record not what was expected\n");
 } else if (iterate_key_primitives(real_ctx, file, file_length, &offset,
                                   NULL, NULL, NULL,
                                   principle, component, key, &equals_pos) == 0) {
  printf("failed\n");
 } else if( (strcmp(principle, "alpha") != 0) ||
            (strcmp(component, "green") != 0) ||
            (strcmp(key, "two") != 0) ||
            (strncmp(file + equals_pos + 2, "rats", 4) != 0) ) {
  printf("failed: second record not what was expected\n");
 } else {
  printf("passed\n");
 }

 printf("test: iterkeyprimsfilterp: ");
 offset = -1;
 if(iterate_key_primitives(real_ctx, file, file_length, &offset,
                           "beta", NULL, NULL,
                           principle, component, key, &equals_pos) == 0)
 {
  printf("failed\n");
 } else if( (strcmp(principle, "beta") != 0) ||
            (strcmp(component, "white") != 0) ||
            (strcmp(key, "six") != 0) ||
            (strncmp(file + equals_pos + 2, "birds", 5) != 0) ) {
  printf("failed: first record not what was expected\n");
 } else if (iterate_key_primitives(real_ctx, file, file_length, &offset,
                                   "beta", NULL, NULL,
                                   principle, component, key, &equals_pos) == 0) {
  printf("failed\n");
 } else if( (strcmp(principle, "beta") != 0) ||
            (strcmp(component, "purple") != 0) ||
            (strcmp(key, "five") != 0) ||
            (strncmp(file + equals_pos + 2, "pizza", 5) != 0) ) {
  printf("failed: second record not what was expected\n");
 } else {
  printf("passed\n");
 }

 printf("test: iterkeyprimsfilterc: ");
 offset = -1;
 if(iterate_key_primitives(real_ctx, file, file_length, &offset,
                           NULL, "purple", NULL,
                           principle, component, key, &equals_pos) == 0)
 {
  printf("failed\n");
 } else if( (strcmp(principle, "beta") != 0) ||
            (strcmp(component, "purple") != 0) ||
            (strcmp(key, "five") != 0) ||
            (strncmp(file + equals_pos + 2, "pizza", 5) != 0) ) {
  printf("failed: first record not what was expected\n");
 } else if (iterate_key_primitives(real_ctx, file, file_length, &offset,
                                   NULL, "purple", NULL,
                                   principle, component, key, &equals_pos) == 0) {
  printf("failed\n");
 } else if( (strcmp(principle, "gama") != 0) ||
            (strcmp(component, "purple") != 0) ||
            (strcmp(key, "ten") != 0) ||
            (strncmp(file + equals_pos + 2, "salad", 5) != 0) ) {
  printf("failed: second record not what was expected\n");
 } else {
  printf("passed\n");
 }

 printf("test: iterkeyprimsfilterk: ");
 offset = -1;
 if(iterate_key_primitives(real_ctx, file, file_length, &offset,
                           NULL, NULL, "two",
                           principle, component, key, &equals_pos) == 0)
 {
  printf("failed\n");
 } else if( (strcmp(principle, "alpha") != 0) ||
            (strcmp(component, "green") != 0) ||
            (strcmp(key, "two") != 0) ||
            (strncmp(file + equals_pos + 2, "rats", 4) != 0) ) {
  printf("failed: first record not what was expected\n");
 } else if (iterate_key_primitives(real_ctx, file, file_length, &offset,
                                   NULL, NULL, "two",
                                   principle, component, key, &equals_pos) == 0) {
  printf("failed\n");
 } else if( (strcmp(principle, "delta") != 0) ||
            (strcmp(component, "black") != 0) ||
            (strcmp(key, "two") != 0) ||
            (strncmp(file + equals_pos + 2, "towel", 5) != 0) ) {
  printf("failed: second record not what was expected\n");
 } else {
  printf("passed\n");
 }

 printf("test: iterkeyprimsfilterall: ");
 offset = -1;
 if(iterate_key_primitives(real_ctx, file, file_length, &offset,
                           "beta", "white", "six",
                           principle, component, key, &equals_pos) == 0)
 {
  printf("failed\n");
 } else if( (strcmp(principle, "beta") != 0) ||
            (strcmp(component, "white") != 0) ||
            (strcmp(key, "six") != 0) ||
            (strncmp(file + equals_pos + 2, "birds", 5) != 0) ) {
  printf("failed: first record not what was expected\n");
 } else if (iterate_key_primitives(real_ctx, file, file_length, &offset,
                                   NULL, NULL, "two",
                                   principle, component, key, &equals_pos) == 0) {
  printf("failed\n");
 } else {
  printf("passed\n");
 }

 printf("test: lookupkey: ");
 if((out = lookup_key(real_ctx, file, file_length, "delta", "orange", "eight")) == NULL)
 {
  printf("failed\n");
 } else if (strcmp(out, "glue") != 0) {
  printf("failed: value not what expected\n");
 } else {
  printf("passed\n");
 }
 if(out != NULL)
  free(out);

 printf("test: listuniqueprncpls: ");
 if(list_unique_principles(real_ctx, "blue", file, file_length, &array, &n_elements))
 {
  printf("failed\n");
 } else if (n_elements != 2) {
  printf("failed: unexpected number of elements\n");
 } else if ( (strcmp(array[0], "alpha") != 0) ||
             (strcmp(array[1], "gama") != 0) ) {
  printf("failed: unexpected array contents\n");
 } else {
  printf("passed\n");
 }
 free_string_array(array, n_elements);

 printf("test: resolveprojectname: ");
 real_ctx->project_configuration_contents = file;
 real_ctx->project_configuration_length = file_length;
 if(resolve_project_name(real_ctx))
 {
  printf("failed\n");
 } else if (strcmp(real_ctx->project_name, "foobar") != 0) {
  printf("failed: wrong project name\n");
 } else {
  printf("passed\n");
 }
 real_ctx->project_configuration_contents = NULL;
 real_ctx->project_configuration_length = 0;

 printf("test: outputmodpassthrough: ");
 n_elements = 0;
 principle_modifications = NULL;
 component_modifications = NULL;
 key_modifications = NULL;
 value_modifications = NULL;
 in_file = NULL;
 if((output = fopen("./selftestfile", "w")) == NULL)
 {
  printf("failed: fopen(./selftestfile) %s\n", strerror(errno));
 } else {
  if(output_modifications(real_ctx, output, file, file_length, n_elements,
                          principle_modifications, component_modifications,
                          key_modifications, value_modifications))
  {
   printf("failed\n");
  } else {
   fclose(output);
   if ((in_file = read_file("./selftestfile", &in_length, 1)) == NULL) {
    printf("failed: read_file()\n");
   } else {

    if((code = compare_config_files(real_ctx, file, file_length, in_file, in_length)) == -1)
    {
     printf("failed: compare_config_files() failed\n");
    } else if (code == 1) {
     printf("failed: logical modifications to the file were made\n");
    } else {
     printf("passed\n");
    }
   }
  }
 }
 unlink("./selftestfile");
 if(in_file != NULL)
  free(in_file);

 printf("test: outputmodvalchange: ");
 n_elements = 0;
 principle_modifications = NULL;
 component_modifications = NULL;
 key_modifications = NULL;
 value_modifications = NULL;
 right_value = NULL;
 code = 0;
 code += add_to_string_array(&principle_modifications, n_elements, "gama", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "purple", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "ten", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "burger", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 code += add_to_string_array(&principle_modifications, n_elements, "delta", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "black", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "two", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "curtain", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 in_file = NULL;
 if((output = fopen("./selftestfile", "w")) == NULL)
 {
  printf("failed: fopen(./selftestfile) %s\n", strerror(errno));
 } else {
  if(output_modifications(real_ctx, output, file, file_length, n_elements,
                          principle_modifications, component_modifications,
                          key_modifications, value_modifications))
  {
   printf("failed\n");
  } else {
   fclose(output);
   if ((in_file = read_file("./selftestfile", &in_length, 1)) == NULL) {
    printf("failed: read_file()\n");
   } else if ( (right_value = lookup_key(real_ctx, in_file, in_length,
                                         "gama", "purple", "ten")) == NULL)  {
    printf("failed: lookup of first modified record failed\n");
   } else if (strcmp(right_value, "burger") != 0) {
    printf("failed: first modified value not what expected\n");
   } else {
    free(right_value);
    right_value = NULL;
    if((right_value = lookup_key(real_ctx, in_file, in_length,
                                 "delta", "black", "two")) == NULL) {
     printf("failed: lookup of second modified record failed\n");
    } else if (strcmp(right_value, "curtain") != 0) {
     printf("failed: second modified value not what expected\n");
    } else {
     free(right_value);
     right_value = NULL;
     if((right_value = lookup_key(real_ctx, in_file, in_length,
                                  "beta", "white", "six")) == NULL) {
      printf("failed: lookup of unmodified record failed\n");
     } else if (strcmp(right_value, "birds") != 0) {
      printf("failed: unmodified value not what expected\n");
     } else {
      printf("passed\n");
     }
    }
   }
  }
 }
 unlink("./selftestfile");
 if(right_value != NULL)
  free(right_value);
 if(in_file != NULL)
  free(in_file);

 printf("test: outputmodvalremove: ");
 n_elements = 0;
 principle_modifications = NULL;
 component_modifications = NULL;
 key_modifications = NULL;
 value_modifications = NULL;
 right_value = NULL;
 code = 0;
 code += add_to_string_array(&principle_modifications, n_elements, "alpha", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "green", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "two", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, NULL, -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 code += add_to_string_array(&principle_modifications, n_elements, "gama", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "blue", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "eleven", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, NULL, -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 in_file = NULL;
 if((output = fopen("./selftestfile", "w")) == NULL)
 {
  printf("failed: fopen(./selftestfile) %s\n", strerror(errno));
 } else {
  if(output_modifications(real_ctx, output, file, file_length, n_elements,
                          principle_modifications, component_modifications,
                          key_modifications, value_modifications))
  {
   printf("failed\n");
  } else {
   fclose(output);
   if ((in_file = read_file("./selftestfile", &in_length, 1)) == NULL) {
    printf("failed: read_file()\n");
   } else if ( (right_value = lookup_key(real_ctx, in_file, in_length,
                                         "alpha", "green", "two")) != NULL)  {
    printf("failed: lookup of first removed record should have failed\n");
   } else if ( (right_value = lookup_key(real_ctx, in_file, in_length,
                                        "gama", "blue", "eleven")) != NULL) {
    printf("failed: lookup of second removed record should have failed\n");
   } else {
    printf("passed\n");
   }
  }
 }
 unlink("./selftestfile");
 if(right_value != NULL)
  free(right_value);
 if(in_file != NULL)
  free(in_file);

 printf("test: outputmodvalappend: ");
 n_elements = 0;
 principle_modifications = NULL;
 component_modifications = NULL;
 key_modifications = NULL;
 value_modifications = NULL;
 right_value = NULL;
 code = 0;
 code += add_to_string_array(&principle_modifications, n_elements, "pi", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "purple", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "ten", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "burger", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 code += add_to_string_array(&principle_modifications, n_elements, "omega", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "black", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "two", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "curtain", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 in_file = NULL;
 if((output = fopen("./selftestfile", "w")) == NULL)
 {
  printf("failed: fopen(./selftestfile) %s\n", strerror(errno));
 } else {
  if(output_modifications(real_ctx, output, file, file_length, n_elements,
                          principle_modifications, component_modifications,
                          key_modifications, value_modifications))
  {
   printf("failed\n");
  } else {
   fclose(output);
   if ((in_file = read_file("./selftestfile", &in_length, 1)) == NULL) {
    printf("failed: read_file()\n");
   } else if ( (right_value = lookup_key(real_ctx, in_file, in_length,
                                         "pi", "purple", "ten")) == NULL)  {
    printf("failed: lookup of first appended record failed\n");
   } else if (strcmp(right_value, "burger") != 0) {
    printf("failed: first appended value not what expected\n");
   } else {
    free(right_value);
    right_value = NULL;
    if((right_value = lookup_key(real_ctx, in_file, in_length,
                                 "omega", "black", "two")) == NULL) {
     printf("failed: lookup of second appended record failed\n");
    } else if (strcmp(right_value, "curtain") != 0) {
     printf("failed: second appended value not what expected\n");
    } else {
     free(right_value);
     right_value = NULL;
     if((right_value = lookup_key(real_ctx, in_file, in_length,
                                  "beta", "white", "six")) == NULL) {
      printf("failed: lookup of unmodified record failed\n");
     } else if (strcmp(right_value, "birds") != 0) {
      printf("failed: unmodified value not what expected\n");
     } else {
      printf("passed\n");
     }
    }
   }
  }
 }
 unlink("./selftestfile");
 if(right_value != NULL)
  free(right_value);
 if(in_file != NULL)
  free(in_file);

 printf("test: outputmodvalcombo: ");
 n_elements = 0;
 principle_modifications = NULL;
 component_modifications = NULL;
 key_modifications = NULL;
 value_modifications = NULL;
 right_value = NULL;
 code = 0;
 code += add_to_string_array(&principle_modifications, n_elements, "gama", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "purple", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "ten", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "burger", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 code += add_to_string_array(&principle_modifications, n_elements, "omega", -1, 1);
 code += add_to_string_array(&component_modifications, n_elements, "black", -1, 1);
 code += add_to_string_array(&key_modifications, n_elements, "two", -1, 1);
 code += add_to_string_array(&value_modifications, n_elements, "curtain", -1, 1);
 if(code != 0)
 {
  printf("failed: add_to_string_array()\n");
  return 1;
 }
 n_elements++;
 code += add_to_string_array(&principle_modifications, n_elements, "gama", -1, 0);
 code += add_to_string_array(&component_modifications, n_elements, "blue", -1, 0);
 code += add_to_string_array(&key_modifications, n_elements, "eleven", -1, 0);
 code += add_to_string_array(&value_modifications, n_elements, NULL, -1, 0);
 n_elements++;
 in_file = NULL;
 if((output = fopen("./selftestfile", "w")) == NULL)
 {
  printf("failed: fopen(./selftestfile) %s\n", strerror(errno));
 } else {
  if(output_modifications(real_ctx, output, file, file_length, n_elements,
                          principle_modifications, component_modifications,
                          key_modifications, value_modifications))
  {
   printf("failed\n");
  } else {
   fclose(output);
   if ((in_file = read_file("./selftestfile", &in_length, 1)) == NULL) {
    printf("failed: read_file()\n");
   } else if ( (right_value = lookup_key(real_ctx, in_file, in_length,
                                         "gama", "purple", "ten")) == NULL)  {
    printf("failed: lookup of modified record failed\n");
   } else if (strcmp(right_value, "burger") != 0) {
    printf("failed: modified value not what expected\n");
   } else {
    free(right_value);
    right_value = NULL;
    if((right_value = lookup_key(real_ctx, in_file, in_length,
                                 "omega", "black", "two")) == NULL) {
     printf("failed: lookup of appended record failed\n");
    } else if (strcmp(right_value, "curtain") != 0) {
     printf("failed: appended value not what expected\n");
    } else {
     free(right_value);
     right_value = NULL;
     if((right_value = lookup_key(real_ctx, in_file, in_length,
                                  "beta", "white", "six")) == NULL) {
      printf("failed: lookup of unmodified record failed\n");
     } else if (strcmp(right_value, "birds") != 0) {
      printf("failed: unmodified value not what expected\n");
     } else if ((right_value = lookup_key(real_ctx, in_file, in_length,
                                          "gama", "blue", "eleven")) != NULL) {
      printf("failed: lookup of removed record should have failed\n");
     } else {
      printf("passed\n");
     }
    }
   }
  }
 }
 unlink("./selftestfile");
 if(right_value != NULL)
  free(right_value);
 if(in_file != NULL)
  free(in_file);

 real_ctx->project_configuration_contents = NULL;
 real_ctx->project_configuration_length = 0;

 return 0;
}

int self_test(struct bca_context *real_ctx)
{
 if(string_test(real_ctx))
  return 1;

 if(setup_test(real_ctx))
  return 1;

 if(config_files_test(real_ctx))
  return 1;

 return 0;
}

/* End selftest.c -------------------------------------------- (sfd organizer) */

