#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "prototypes.h"

char fg_n_black[6] =   { 0x1b, '[', '3', '0', 'm', 0 };
char fg_n_red[6] =     { 0x1b, '[', '3', '1', 'm', 0 };
char fg_n_green[6] =   { 0x1b, '[', '3', '2', 'm', 0 };
char fg_n_yellow[6] =  { 0x1b, '[', '3', '3', 'm', 0 };
char fg_n_blue[6] =    { 0x1b, '[', '3', '4', 'm', 0 };
char fg_n_magenta[6] = { 0x1b, '[', '3', '5', 'm', 0 };
char fg_n_cyan[6] =    { 0x1b, '[', '3', '6', 'm', 0 };
char fg_n_white[6] =   { 0x1b, '[', '3', '7', 'm', 0 };

char fg_hi_black[6] =   { 0x1b, '[', '9', '0', 'm', 0 };
char fg_hi_red[6] =     { 0x1b, '[', '9', '1', 'm', 0 };
char fg_hi_green[6] =   { 0x1b, '[', '9', '2', 'm', 0 };
char fg_hi_yellow[6] =  { 0x1b, '[', '9', '3', 'm', 0 };
char fg_hi_blue[6] =    { 0x1b, '[', '9', '4', 'm', 0 };
char fg_hi_magenta[6] = { 0x1b, '[', '9', '5', 'm', 0 };
char fg_hi_cyan[6] =    { 0x1b, '[', '9', '6', 'm', 0 };
char fg_hi_white[6] =   { 0x1b, '[', '9', '7', 'm', 0 };

char bg_n_black[6] =   { 0x1b, '[', '4', '0', 'm', 0 };
char bg_n_red[6] =     { 0x1b, '[', '4', '1', 'm', 0 };
char bg_n_green[6] =   { 0x1b, '[', '4', '2', 'm', 0 };
char bg_n_yellow[6] =  { 0x1b, '[', '4', '3', 'm', 0 };
char bg_n_blue[6] =    { 0x1b, '[', '4', '4', 'm', 0 };
char bg_n_magenta[6] = { 0x1b, '[', '4', '5', 'm', 0 };
char bg_n_cyan[6] =    { 0x1b, '[', '4', '6', 'm', 0 };
char bg_n_white[6] =   { 0x1b, '[', '4', '7', 'm', 0 };

char bg_hi_black[8] =   { 0x1b, '[', '1', '0', '0', 'm', 0 };
char bg_hi_red[8] =     { 0x1b, '[', '1', '0', '1', 'm', 0 };
char bg_hi_green[8] =   { 0x1b, '[', '1', '0', '2', 'm', 0 };
char bg_hi_yellow[8] =  { 0x1b, '[', '1', '0', '3', 'm', 0 };
char bg_hi_blue[8] =    { 0x1b, '[', '1', '0', '4', 'm', 0 };
char bg_hi_magenta[8] = { 0x1b, '[', '1', '0', '5', 'm', 0 };
char bg_hi_cyan[8] =    { 0x1b, '[', '1', '0', '6', 'm', 0 };
char bg_hi_white[8] =   { 0x1b, '[', '1', '0', '7', 'm', 0 };

char bold[5] =   { 0x1b, '[', '1', 'm', 0 };

char normal[5] = { 0x1b, '[', '0', 'm', 0 };


struct colorize_context
{
 struct bca_context *bctx;

 char **source_file_names;
 int n_source_files;

 char **command_names;
 int n_command_names;

 char **artifact_file_names;
 int n_artifact_files;

 char **temporary_file_names;
 int n_temporary_files;

 int n_warnings;
};

#define lt_other            100
#define lt_warning          101
#define lt_error            102
#define lt_command          103
#define lt_column_indicator 104

#define st_normal             20
#define st_backticks          21
#define st_source_filename    22
#define st_command_name       23
#define st_shell_lexical      24
#define st_artifact_filename  25
#define st_temporary_filename 26

/* line is a buffer of buffer_size, with 1 or more null terminated, 1 byte type prefixed,
   strings. The total used portion is pointed to by length. */
int split(char *line, unsigned int *length, unsigned int split_at, unsigned int buffer_size)
{
 unsigned int bytes_to_move, i;
 char existing_type = st_normal;

 bytes_to_move = (*length - split_at) + 2;

 if(*length + 4 > buffer_size)
 {
  return 1;
 }

 i = split_at;
 while(i > 0)
 {
  if(line[i] == 0)
  {
   existing_type= line[i+1];
   break;
  }

  i--;
 }

 memmove(line + split_at + 2, line + split_at, bytes_to_move);
 line[split_at] = 0;
 line[split_at + 1] = existing_type;
 (*length) += 2;

 return 0;
}

int split_on_backticks(char *line, unsigned int *length)
{
 unsigned int i, section_length, inside = 0;
 char *type;

 /* 1. introduce splits on backticks */
 i = 1;
 while(i < *length)
 {
  switch(line[i])
  {
   case '`':
        if(inside)
        {

         if(i + 1 < *length)
         {
          if(split(line, length, ++i, 4096))
           return 1;
         }
         inside = 0;
        } else {
         if(split(line, length, i, 4096))
          return 1;
         inside = 1;
        }
        break;

   case 0:
        i += 2;

   default:
        i++;
  }
 }

 /* 2. classify qualifying segments as backticked */
 i = 0;
 while(i < *length)
 {
  type = line + i++;
  section_length = strlen(line + i) + 1;
  if(*type == st_normal)
  {
   if((line[i] == '`') && (line[i + section_length - 2] == '`'))
   {
    *type = st_backticks;
   }
  }
  i += section_length;
 }

 return 0;
}


int split_on_list(char *line, unsigned int *length,
                  char **list, int n_items, char st,
                  int file_name_mode)
{
 unsigned int i, fn_i, z, w, match;
 char *type;

 if(n_items == 0)
  return 0;

 i = 1;
 while(i < *length)
 {
  if(line[i] == 0)
  {
   i += 2;
  } else {

   for(fn_i = 0; fn_i < n_items; fn_i++)
   {
    w = 0;
    z = 0;
    match = 1;
    while( (list[fn_i][z] != 0) && (line[i + z] != 0) )
    {
     if(list[fn_i][z] != line[i + z])
     {
      match = 0;
      break;
     }
     z++;
    }

    if( (file_name_mode == 1) && (match == 0) )
    {
     if( (list[fn_i][0] == '.') && (list[fn_i][1] == '/') )
     {
      z = 0;
      w = 2;
      match = 1;
      while( (list[fn_i][z + w] != 0) && (line[i + z] != 0) )
      {
       if(list[fn_i][z + w] != line[i + z])
       {
        match = 0;
        w = 0;
        break;
       }
       z++;
      }
     }
    }

    if( (list[fn_i][z] != 0) && (line[i + z] == 0) )
     match = 0;

    if(match == 1)
    {

     if(file_name_mode == 1)
     {
      while(line[i + z] != 0)
      {
       if( ((line[i + z] >= '0') && (line[i + z] <= '9')) ||
           (line[i + z] == ':') )
       {
        z++;
       } else {
        break;
       }
      }
     }

     if(i > 2)
     {
      if(split(line, length, i, 4096))
       return 1;

      i++;
      type = line + i++;
     } else {
      type = line;
     }

     if(split(line, length, i + z, 4096))
      return 1;

     *type = st;

     fn_i = n_items; /* break out of for() */
     i += z;
    }

   }

   if(match == 0)
    i++;
  }

 }

 return 0;
}

int split_on_file_names(struct colorize_context *cc, char *line, unsigned int *length)
{
 return split_on_list(line, length,
                      cc->source_file_names,
                      cc->n_source_files,
                      st_source_filename, 1);
}

int split_on_artifact_filenames(struct colorize_context *cc, char *line, unsigned int *length)
{
 return split_on_list(line, length,
                      cc->artifact_file_names,
                      cc->n_artifact_files,
                      st_artifact_filename, 1);
}

int split_on_temporary_filenames(struct colorize_context *cc, char *line, unsigned int *length)
{
 return split_on_list(line, length,
                      cc->temporary_file_names,
                      cc->n_temporary_files,
                      st_temporary_filename, 1);
}

int split_on_command_names(struct colorize_context *cc, char *line, unsigned int *length)
{
 return split_on_list(line, length,
                      cc->command_names,
                      cc->n_command_names,
                      st_command_name, 0);
}

int split_on_shell_lexical(struct colorize_context *cc, char *line, unsigned int *length)
{
 char *list[6] = { " < ", " > ", " << ", " >> ", "; ", " && " };

 return split_on_list(line, length,
                      list, 6, st_shell_lexical, 0);
}

int is_command_line(struct colorize_context *cc, char *line, int length)
{
 int i;

 for(i=0; i<cc->n_command_names; i++)
 {
  if(in_string(line, cc->command_names[i], -1) > -1)
   return 1;
 }

 return 0;
}

int handle_line(char *line, unsigned int length, void *data1, void *data2)
{
 struct colorize_context *cc = (struct colorize_context *) data1;
 FILE *out = (FILE *) data2;
 unsigned int segment_length, i, no;
 int line_type = -1;
 char *section_type, last_st = -1;

 /* reformat line for type prefix storage */
 memmove(line + 1, line, length + 1);
 i = 0;
 section_type = line + i++;
 *section_type = st_normal;

 /* classify the line type */
 if(in_string(line + i, "Error", 5) > -1)
 {
  line_type = lt_error;
 } else if(in_string(line + i, " error:", 7) > -1)  {
  line_type = lt_error;
 } else if(in_string(line + i, " warning:", 9) > -1)  {
  line_type = lt_warning;
  cc->n_warnings++;
 } else if(is_command_line(cc, line + i, length - i))  {
  line_type = lt_command;
 } else {

  no = 0;
  i=1;
  while(line[i] != 0)
  {
   if( (line[i] == ' ') || (line[i] == '^') )
   {
    i++;
   } else {
    no = 1;
    break;
   }
  }

  if(no == 0)
   line_type = lt_column_indicator;

 }

 if(line_type == -1)
  line_type = lt_other;

 /* split into segments */
 if(line_type == lt_command)
 {
  if(split_on_backticks(line, &length))
   return 1;

  if(split_on_command_names(cc, line, &length))
   return 1;

  if(split_on_shell_lexical(cc, line, &length))
   return 1;
 }

 if(line_type != lt_column_indicator) 
 {
  if(split_on_file_names(cc, line, &length))
   return 1;

  if(split_on_artifact_filenames(cc, line, &length))
   return 1;

  if(split_on_temporary_filenames(cc, line, &length))
   return 1;
 }

 /* render the segments */
 i = 0;
 while(i < length)
 {
  section_type = line + i++;
  segment_length = strlen(line + i) + 1;

  switch(line_type)
  {
   case lt_other:
        switch(*section_type)
        {
        case st_source_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_hi_white, bold, line + i, normal);
             break;

        case st_artifact_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_n_green, bold, line + i, normal);
             break;

        case st_temporary_filename:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_magenta, line + i, normal);
             break;

         default:
              fprintf(out, "%s", line + i);
        }
        break;

  case lt_error:
       switch(*section_type)
       {
        case st_source_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_hi_white, bold, line + i, normal);
             break;

        case st_artifact_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_n_green, bold, line + i, normal);
             break;

        case st_temporary_filename:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_magenta, line + i, normal);
             break;

        default:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_red, line + i, normal);
       }
       break;

  case lt_warning:
       switch(*section_type)
       {
        case st_source_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_hi_white, bold, line + i, normal);
             break;

        case st_artifact_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_n_green, bold, line + i, normal);
             break;

        case st_temporary_filename:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_magenta, line + i, normal);
             break;

        default:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_yellow, line + i, normal);
       }
       break;

  case lt_command:
       switch(*section_type)
       {
        case st_command_name:
             if( (last_st == st_backticks) &&
                 (line[i + segment_length] == st_backticks) )
              fprintf(out, "%s%s%s%s%s", bg_n_magenta, fg_n_blue, bold, line + i, normal);
             else
              fprintf(out, "%s%s%s%s%s", bg_n_black, fg_n_blue, bold, line + i, normal);
             break;

        case st_source_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_hi_white, bold, line + i, normal);
             break;

        case st_artifact_filename:
             fprintf(out, "%s%s%s%s%s", bg_n_black, fg_n_green, bold, line + i, normal);
             break;

        case st_temporary_filename:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_magenta, line + i, normal);
             break;

        case st_backticks:
             fprintf(out, "%s%s%s%s", bg_n_magenta, fg_n_white, line + i, normal);
             break;

        case st_shell_lexical:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_hi_red, line + i, normal);
             break;

        default:
             fprintf(out, "%s%s%s%s", bg_n_black, fg_n_cyan, line + i, normal);
       }
       break;

  case lt_column_indicator:
       fprintf(out, "%s%s%s", fg_hi_green, line + i, normal);
       break;
  }
  last_st = *section_type;
  i += segment_length;
 }

 fprintf(out, "\n");
 return 0;
}

struct line_processor
{
 char stdout_line[4096], stderr_line[4096];
 int stdout_length, stderr_length;
 void *data;
};

int line_finder(char *line, int size, int *line_length,
                char *new, int new_length,
                void *data1, void *data2,
                int (*handle_line) (char *line, unsigned int length, void *data1, void *data2) )
{
 int new_i;

 for(new_i = 0; new_i < new_length; new_i++)
 {
  if(new[new_i] == '\n')
  {
   if(*line_length > 0)
   {
    line[*line_length] = 0;

    if(handle_line(line, *line_length, data1, data2))
     return 1;

    *line_length = 0;
   }
  } else {
   if(*line_length == 4095)
   {
    line[*line_length] = 0;

    if(handle_line(line, *line_length, data1, data2))
     return 1;

    *line_length = 0;
   }

   line[(*line_length)++] = new[new_i];
  }
 }

 return 0;
}

int handle_stdout(struct spawn_context *ctx, void *data, char *buffer, int n_bytes)
{
 struct line_processor *lp = (struct line_processor *) data;

// fprintf(stderr, "handle_stdout(%d): '%s'\n", n_bytes, buffer);

 return line_finder(lp->stdout_line, 4096, &(lp->stdout_length),
                    buffer, n_bytes, lp->data, stdout, handle_line);
}

int handle_stderr(struct spawn_context *ctx, void *data, char *buffer, int n_bytes)
{
 struct line_processor *lp = (struct line_processor *) data;

// fprintf(stderr, "handle_stderr(%d): '%s'\n", n_bytes, buffer);

 return line_finder(lp->stderr_line, 4096, &(lp->stderr_length),
                    buffer, n_bytes, lp->data, stderr, handle_line);
}

int handle_exit(struct spawn_context *ctx, void *data, int condition, int exit_code)
{
// fprintf(stderr, "handle_exit(%d, %d)\n", condition, exit_code);

 if(condition != 0)
  return 1;

 return 0;
}

int collect_file_names(struct colorize_context *cc, struct project_details *pd)
{
 struct component_details cd;
 int x, y;

 memset(&cd, 0, sizeof(struct component_details));

 cc->source_file_names = NULL;
 cc->n_source_files = 0;

 for(x=0; x < pd->n_components; x++)
 {
  cd.component_name = pd->component_names[x];
  cd.component_type = pd->component_types[x];
  cd.component_output_name = pd->component_output_names[x];

  if(resolve_component_source_files(cc->bctx, &cd))
   return 1;

  if(resolve_component_extra_file_dependencies(cc->bctx, &cd))
   return 1;

  for(y=0; y<cd.n_source_files; y++)
  {
   if(add_to_string_array(&(cc->source_file_names),
                          cc->n_source_files,
                          cd.source_file_names[y], -1, 1) == 0)
    cc->n_source_files++;
  }

  for(y=0; y<cd.n_extra_file_deps; y++)
  {
   if(add_to_string_array(&(cc->source_file_names),
                          cc->n_source_files,
                          cd.extra_file_deps[y], -1, 1) == 0)
    cc->n_source_files++;
  }

  for(y=0; y<cd.n_lib_headers; y++)
  {
   if(add_to_string_array(&(cc->source_file_names),
                          cc->n_source_files,
                          cd.lib_headers[y], -1, 1) == 0)
    cc->n_source_files++;
  }


  if(component_details_cleanup(&cd))
   return 1;
 }

 if(add_to_string_array(&(cc->source_file_names), cc->n_source_files,
                        "Makefile", 8, 1) == 0)
  cc->n_source_files++;

 if(cc->bctx->verbose > 0)
  fprintf(stderr, "BCA: found %d source files for make colorization process.\n",
          cc->n_source_files);

 return 0;
}

int collect_artifact_file_names(struct colorize_context *cc, struct project_details *pd)
{
 struct component_details cd;
 struct build_details bd;
 struct host_configuration *tc;
 char temp[512];
 int x, y, host_i, component_i, source_i, i, do_object_files;

 if(load_build_config(cc->bctx, 0))
  return 0;

 if(list_build_hosts(cc->bctx, &bd))
  return 0;

 /* since all we are trying to do is list of possible artifact files for
    syntax highlighting, we don't need to do disables and swaps processing */

 for(host_i = 0; host_i < bd.n_hosts; host_i++)
 {
  for(component_i = 0; component_i < pd->n_components; component_i++)
  {
   memset(&cd, 0, sizeof(struct component_details));
   cd.host = bd.hosts[host_i];

   if((tc = resolve_host_configuration(cc->bctx, cd.host,
                                       pd->component_names[component_i])) != NULL)
   {
    cd.component_type = pd->component_types[component_i];
    cd.component_name = pd->component_names[component_i];
    cd.component_output_name = pd->component_output_names[component_i];

    resolve_component_version(cc->bctx, &cd);

    if(render_project_component_output_names(cc->bctx, &cd, tc, RENDER_BUILD_OUTPUT_NAME) == 0)
    {

     if(strcmp(pd->component_types[component_i], "SHAREDLIBRARY") == 0)
     {
      /* don't forget the symlink */
      snprintf(temp, 512, "%s/%s.pc",
               tc->build_prefix,
               pd->component_output_names[component_i]);

      if(add_to_string_array(&(cc->artifact_file_names),
                             cc->n_artifact_files,
                             temp, -1, 1) == 0)
       cc->n_artifact_files++;
     }

     for(i=0; i < cd.n_rendered_names; i++)
     {
      if(cd.rendered_names[i][0] != 0)
      {
       snprintf(temp, 512, "%s", cd.rendered_names[i]);

       if(add_to_string_array(&(cc->artifact_file_names),
                              cc->n_artifact_files,
                              temp, -1, 1) == 0)
        cc->n_artifact_files++;
      }
     }

     if(free_rendered_names(&cd))
      return 1;
    }

    do_object_files = 0;

    if(strcmp(pd->component_types[component_i], "SHAREDLIBRARY") == 0)
     do_object_files = 1;

    if(strcmp(pd->component_types[component_i], "BINARY") == 0)
     do_object_files = 1;

    if(do_object_files)
    {
     if(resolve_component_source_files(cc->bctx, &cd) == 0)
     {
      for(source_i=0; source_i<cd.n_source_files; source_i++)
      {
       snprintf(temp, 512, "%s/obj/%s-%s%s",
                tc->build_prefix, cd.component_name,
                cd.source_file_base_names[source_i], tc->obj_suffix);

       if(add_to_string_array(&(cc->temporary_file_names),
                              cc->n_temporary_files,
                              temp, -1, 1) == 0)
        cc->n_temporary_files++;
      }

      if(component_details_cleanup(&cd))
       return 1;
     }
    }

    if(free_host_configuration(cc->bctx, tc))
     return 1;
   }
  }
 }

 return 0;
}

int collect_command_names(struct colorize_context *cc, struct project_details *pd)
{
 /* this should consult the build configuration */

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "gcc ", 4, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "./bca ", 6, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "pkg-config ", 11, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "echo ", 5, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "rm ", 3, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "ln ", 3, 1) == 0)
  cc->n_command_names++;

 if(add_to_string_array(&(cc->command_names),
                        cc->n_command_names,
                        "cd ", 3, 1) == 0)
  cc->n_command_names++;

 return 0;
}

int make_colorize_mode(struct bca_context *bctx, int argc, char **argv)
{
 int code, i;
 struct line_processor lp;
 struct colorize_context cc;
 struct project_details *pd = NULL;

 if(bctx->verbose > 1)
  fprintf(stderr, "BCA: make_colorize_mode()\n");

 memset(&cc, 0, sizeof(struct colorize_context));
 cc.bctx = bctx;

 lp.stdout_length = 0;
 lp.stderr_length = 0;
 lp.data = &cc;

 if(argc < 1)
  return 1;

 if(load_project_config(bctx, 0) == 0)
 {
  if((pd = resolve_project_details(bctx)) == NULL)
  {
   fprintf(stderr, "BCA: resolve_project_details() failed\n");
   return 1;
  }

  if(collect_file_names(&cc, pd))
  {
   fprintf(stderr, "BCA: collect_file_names() failed\n");
   return 1;
  }

  if(collect_artifact_file_names(&cc, pd))
  {
   fprintf(stderr, "BCA: collect_artifact_file_names() failed\n");
   return 1;
  }

  if(collect_command_names(&cc, pd))
  {
   fprintf(stderr, "BCA: collect_command_names() failed\n");
   return 1;
  }
 }

 if(bctx->verbose > 0)
 {
  fprintf(stdout, "BCA: invoking \"");
  for(i=0; i<argc; i++)
  {
   fprintf(stdout, "%s ", argv[i]);
  }
  fprintf(stdout, "\"\n");
 }

 code = spawn(argv[0], argc, argv, &lp, handle_stdout, handle_stderr, handle_exit);

 if(pd != NULL)
 {
  free_project_details(pd);
 }

 if(cc.n_warnings > 0)
  fprintf(stderr, "BCA: %d warnings\n", cc.n_warnings);

 return code;
}

