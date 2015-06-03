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

 if(strcmp(project_component_type, "BEAM") == 0)
  return ".beam";

 if(strcmp(project_component_type, "PYTHONMODULE") == 0)
  return ".py";

 /* CAT and MACROEXPAND might need the same logic as CUSTOM */
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


int add_rendered_name(struct component_details *cd, char *name, char *extension)
{
 int code;

 if((code = add_to_string_array(&(cd->rendered_names),
                                cd->n_rendered_names,
                                name, -1, 0)) < 0)
  return -1;

 if((code = add_to_string_array(&(cd->rendered_extensions),
                                cd->n_rendered_names,
                                extension, -1, 0)) < 0)
 return -1;

 cd->n_rendered_names++;

 return 0;
}

int free_rendered_names(struct component_details *cd)
{
 free_string_array(cd->rendered_names, cd->n_rendered_names);
 free_string_array(cd->rendered_extensions, cd->n_rendered_names);
 cd->n_rendered_names = 0;
 cd->rendered_names = NULL;
 cd->rendered_extensions = NULL;
 return 0;
}

int render_project_component_output_names(struct bca_context *ctx,
                                          struct component_details *cd,
                                          int edition) //this should take tc instead of looking it up again
{
 char *extension, temp[1024], *component_install_path;
 int prefix_length, import, effective_path_mode;
 struct host_configuration *tc;

 if(ctx->verbose > 1)
  fprintf(stderr, "BCA: render_project_component_output_names(%s, %s)\n",
          cd->host, cd->component_name);

 if( (cd->n_rendered_names != 0) ||
     (cd->rendered_names != NULL) ||
     (cd->rendered_extensions != NULL) )
 {
  fprintf(stderr,
          "BCA: render_project_component_output_names(): component details structure "
          "in input invalid state\n");
  return 1;
 }

 if(cd->host == NULL)
 {
  fprintf(stderr, "BCA: render_project_component_output_names(): host can not be NULL\n");
  return 1;
 }

 if((tc = resolve_host_configuration(ctx, cd->host, cd->component_name)) == NULL)
  return 1;

 if((extension = component_type_file_extension(ctx, tc,
                                               cd->component_type,
                                               cd->component_output_name)) == NULL)
 {
  fprintf(stderr, "BCA: component_type_file_extension(%s, %s) failed\n",
          cd->component_type, cd->component_output_name);
  return 1;
 }

 /* resolve the prefix (if any) */
 switch(edition)
 {
  case RENDER_OUTPUT_NAME:
       prefix_length = 0;
       break;

  case RENDER_BUILD_OUTPUT_NAME:
       prefix_length = snprintf(temp, 1024, "%s/", tc->build_prefix);
       break;

  case RENDER_INSTALL_OUTPUT_NAME:
       temp[0] = 0;
       prefix_length = 1;

       if(resolve_component_installation_path(ctx,
                                              cd->host,
                                              cd->component_type,
                                              cd->component_name,
                                              &component_install_path))
       {
        fprintf(stderr,
                "BCA: resolve_component_installation_path() failed.\n");
        return 1;
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

  case RENDER_EFFECTIVE_OUTPUT_NAME:
       if((effective_path_mode = resolve_effective_path_mode(ctx)) == -1)
        return 1;

       switch(effective_path_mode)
       {
        case EFFECTIVE_PATHS_LOCAL:
             return render_project_component_output_names(ctx, cd, RENDER_BUILD_OUTPUT_NAME);
             break;

        case EFFECTIVE_PATHS_INSTALL:
             return render_project_component_output_names(ctx, cd, RENDER_INSTALL_OUTPUT_NAME);
             break;

        default:
             fprintf(stderr, "BCA: render_project_component_name(): return code from "
                     "resolve_effective_path_mode() makes no sense\n");
             return 1;
       }
       break;

 default:
      fprintf(stderr, "BCA: render_project_component_name(): invalid edition number, %d\n",
              edition);
      return 1;
      break;
 }

 if(strcmp(cd->component_type, "PYTHONMODULE") == 0)
 {
  snprintf(temp + prefix_length, 1024 - prefix_length,
           "%s-setup.py", cd->component_output_name);

  if(add_rendered_name(cd, temp, ".py"))
   return 1;

 } else if(strcmp(cd->component_type, "BEAM") == 0) {

  snprintf(temp + prefix_length, 1024 - prefix_length,
           "%s.beam", cd->component_output_name);

  if(add_rendered_name(cd, temp, ".beam"))
   return 1;

 } else if(strcmp(cd->component_type, "BINARY") == 0) {

  snprintf(temp + prefix_length, 1024 - prefix_length,
           "%s%s", cd->component_output_name, extension);

  if(add_rendered_name(cd, temp, extension))
   return 1;

 } else if(strcmp(cd->component_type, "SHAREDLIBRARY") == 0) {

  if(resolve_component_version(ctx, cd))
  {
   fprintf(stderr, "BCA: resolve_component_version() failed\n");
   return 1;
  }

  if(strcmp(extension, ".so") == 0)
  {
   /* so name */
   snprintf(temp + prefix_length, 1024 - prefix_length,
            "%s%s%s.%s",
            tc->shared_library_prefix,
            cd->component_output_name,
            extension,
            cd->major);

   if(add_rendered_name(cd, temp, extension))
    return 1;

   /* pc file name */
   if(edition == RENDER_INSTALL_OUTPUT_NAME)
   {
    snprintf(temp, 1024, "%s/%s-%s.pc",
             tc->install_pkg_config_dir,
             cd->component_output_name,
             cd->major);
   } else {
    snprintf(temp + prefix_length, 1024 - prefix_length,
             "%s-%s.pc",
             cd->component_output_name,
             cd->major);
   }

   if(add_rendered_name(cd, temp, ".pc"))
    return 1;

   /* import library */
   if(add_rendered_name(cd, "", ""))
    return 1;

   /* real name */
   snprintf(temp + prefix_length, 1024 - prefix_length,
            "%s%s%s.%s.%s",
            tc->shared_library_prefix,
            cd->component_output_name, extension,
            cd->major, cd->minor);

   if(add_rendered_name(cd, temp, extension))
    return 1;
  }

  if(strcmp(extension, ".dll") == 0)
  {
   /* so name */
   snprintf(temp + prefix_length, 1024 - prefix_length,
            "%s%s-%s.%s%s",
             tc->shared_library_prefix,
             cd->component_output_name,
             cd->major, cd->minor, extension);

   if(add_rendered_name(cd, temp, extension))
    return 1;

   /* pc file */
   if(edition == RENDER_INSTALL_OUTPUT_NAME)
   {
    snprintf(temp, 1024, "%s/%s-%s.pc",
             tc->install_pkg_config_dir,
             cd->component_output_name,
             cd->major);
   } else {
    snprintf(temp + prefix_length, 1024 - prefix_length,
             "%s-%s.pc",
             cd->component_output_name,
             cd->major);
   }

   if(add_rendered_name(cd, temp, ".pc"))
    return 1;

   /* import library */
   import = 0;

   if(contains_string(tc->cc, -1, "mingw", -1))
   {
    import = 1;
    snprintf(temp + prefix_length, 1024 - prefix_length,
             "%s%s-%s.%s%s.a",
             tc->shared_library_prefix,
             cd->component_output_name,
             cd->major, cd->minor, extension);

    if(add_rendered_name(cd, temp, ".a"))
     return 1;
   }

   if(strcmp(tc->shared_library_prefix, "cyg") == 0)
   {
    import = 1;
    snprintf(temp + prefix_length, 1024 - prefix_length,
             "lib%s-%s.%s%s.a",
             cd->component_output_name,
             cd->major, cd->minor,
             extension);

    if(add_rendered_name(cd, temp, ".a"))
     return 1;
   }
  }

  if(strcmp(extension, ".dylib") == 0)
  {
   /* install name (soname analog) */
   snprintf(temp + prefix_length, 1024 - prefix_length,
            "%s%s.%s%s",
            tc->shared_library_prefix,
            cd->component_output_name,
            cd->major, extension);

   if(add_rendered_name(cd, temp, extension))
    return 1;

   /* pc file name */
   if(edition == RENDER_INSTALL_OUTPUT_NAME)
   {
    snprintf(temp, 1024, "%s/%s-%s.pc",
             tc->install_pkg_config_dir,
             cd->component_output_name,
             cd->major);
   } else {
    snprintf(temp + prefix_length, 1024 - prefix_length,
             "%s-%s.pc",
             cd->component_output_name,
             cd->major);
   }

   if(add_rendered_name(cd, temp, ".pc"))
    return 1;

   /* import library */
   if(add_rendered_name(cd, "", ""))
    return 1;

   /* real name */
   snprintf(temp + prefix_length, 1024 - prefix_length,
            "%s%s.%s.%s%s",
            tc->shared_library_prefix,
            cd->component_output_name,
            cd->major, cd->minor, extension);

   if(add_rendered_name(cd, temp, extension))
    return 1;
  }

 } else if(strcmp(cd->component_type, "CAT") == 0) {

  snprintf(temp + prefix_length, 1024 - prefix_length, "%s",
           cd->component_output_name);

  if(add_rendered_name(cd, temp, extension))
   return 1;

 } else if(strcmp(cd->component_type, "MACROEXPAND") == 0) {

  snprintf(temp + prefix_length, 1024 - prefix_length, "%s",
           cd->component_output_name);

  if(add_rendered_name(cd, temp, extension))
   return 1;

 } else if(strcmp(cd->component_type, "CUSTOM") == 0) {

  snprintf(temp + prefix_length, 1024 - prefix_length, "%s",
           cd->component_output_name);

  if(add_rendered_name(cd, temp, extension))
   return 1;

 } else {

  fprintf(stderr,
          "BCA: I don't know how to render an output file name for component "
          "type of \"%s\"\n",
          cd->component_type);
  return 1;
 }

 if(cd->n_rendered_names < 1)
 {
  fprintf(stderr,
          "BCA: render_project_component_output_names(): is there really a component "
          "named \"%s\" ?\n",
          cd->component_name);
 }

 return 0;
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

char *file_name_to_array_name(char *file_name)
{
 char *array_name;
 int i, length = strlen(file_name);

 if((array_name = (char *) malloc(length + 1)) == NULL)
 {
  fprintf(stderr, "BCA: mallac(%d) failed\n", length + 1);
  return NULL;
 }

 for(i=0; i<length; i++)
 {
  if(isalpha(file_name[i]))
   array_name[i] = file_name[i];
  else
   array_name[i] = '_';
 }
 array_name[i] = 0;

 return array_name;
}

int file_to_C_source(struct bca_context *ctx, char *file_name)
{
 char *contents, *array_name;
 int length, n_cols = 0, index = 0;

 if(ctx->verbose > 0)
  fprintf(stderr, "BCA: file_to_c_source()\n");

 if((array_name = file_name_to_array_name(file_name)) == NULL)
 {
  fprintf(stderr, "BCA: file_name_to_array_name(%s) failed\n", file_name);
  return 1;
 }

 if((contents = read_file(file_name, &length, 0)) == NULL)
 {
  fprintf(stderr, "BCA: read_file(%s) failed\n", file_name);
  free(array_name);
  return 1;
 }

 fprintf(stdout, "const int %s_length = %d;\n",
         array_name, length);

 fprintf(stdout, "const char %s[%d] = \n{\n ",
         array_name, length);

 while(index < length)
 {
  if(n_cols == 13)
  {
   fprintf(stdout, "\n ");
   n_cols = 0;
  }

  fprintf(stdout, "0x%x", (unsigned char) contents[index]);

  n_cols++;
  index++;

  if(index < length)
  fprintf(stdout, ", ");
 }
 fprintf(stdout, "\n};\n\n");

 free(array_name);
 free(contents);
 return 0;
}
