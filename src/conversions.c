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
