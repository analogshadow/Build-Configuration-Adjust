#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "convert.h"

#define TSTART         -3
#define TWAIT          -2
#define TERROR         -1
#define TNORMAL         0
#define TPPD            1
#define TKEYWORD        2
#define TOPERATOR       3
#define TARGUMENT       4
#define TFUNCTION       5
#define TSTRING         6
#define TESCAPESEQUENCE 7
#define TDECLARATOR     8
#define TFORMATMODIFIER 9
#define TCOMMENT        10
#define TMEMBER         11
#define TOBJECT         12

struct c_source_state
{
 char *line;
 int line_length;
 int i;
 int only_whitespace;
 int in_preprocessor_line;
 int in_comment;
 int in_comment_line;
 int in_quotes;
 int in_escape;
 int in_format_modifier;
 int in_prototype;
 int in_argument_name;
 int in_relooped_char;
 int in_object_definition;
 int in_object_prototype;
 int in_object_name;
 int follows_operator;
 int follows_declaration;
 int inside_braces;
 int type;
 int text_length;
 char text[256];
 int delay_line_break;
 char **args;
 int n_args;
};

struct word
{
 char *w;
 int l;
};

struct word keywords[] = {
 { "default ", 8 },
 { "return ", 7 },
 { "switch ", 7 },
 { "sizeof ", 7 },
 { "break ", 6 },
 { "while ", 6 },
 { "case ", 5 },
 { "if ", 3 },
 { NULL, 0 }
};

struct word declorations[] = {
 { "char *** ", 9 },
 { "char ** ", 8 },
 { "int ", 4 },
 { NULL, 0 }
};

struct word operators[] = {
 { "==", 2 },
 { "++", 2 },
 { "--", 1 },
 { "->", 2 },
 { "<", 1 },
 { ">", 1 },
 { "[", 1 },
 { "]", 1 },
 { ".", 1 },
 { "&", 1 },
 { "*", 1 },
 { "=", 1 },
 { "+", 1 },
 { "-", 1 },
 { NULL, 0 }
};

struct word escape_sequences[] = {
 { "\\n", 2 },
 { "\\t", 2 },
 { NULL, 0 }
};

int next(struct c_source_state *state);


int next(struct c_source_state *state)
{
 int yes, x, y;

 if(state->i == state->line_length)
  return 0;

 if(state->in_comment_line)
 {
  snprintf(state->text, 256, "%c", state->line[state->i]);
  state->type = TNORMAL;
  state->i++;
  state->text_length = 0;
  state->in_relooped_char = 0;
  return 1;
 }
 
 if(state->only_whitespace)
 {
  if(!isblank(state->line[state->i]))
  {
   state->only_whitespace = 0;
   if(state->line[state->i] == '#')
    state->in_preprocessor_line = 1;
  }
 }

 if(state->in_preprocessor_line)
 {
  if(strncmp(state->line + state->i, "#include", 8) == 0)
  {
   snprintf(state->text, 256, "#include");
   state->type = TPPD;
   state->i += 8;
   return 1;
  }
 }

 if(state->i + 1 < state->line_length)
 {
  if( (state->line[state->i] == '/') &&
      (state->line[state->i + 1] == '/') )
  {
   if(state->text_length == 0)
    state->in_relooped_char = 1;

   if(state->in_relooped_char)
   {
    state->in_relooped_char = 0;
    state->in_comment_line = 1;
    snprintf(state->text, 256, "//");
    state->type = TSTART;
    state->i += 2;
    state->text_length = 0;
    return 1;
   } else {
    state->text[state->text_length++] = 0;
    state->type = TNORMAL;
    state->i += 0;
    state->text_length = 0;
    state->in_relooped_char = 1;
    return 1;
   }
  }
 }

 if(state->in_comment)
 {
  if(strncmp(state->line + state->i, "*/", 2) == 0)
  {
   state->in_comment = 0;
   snprintf(state->text, 256, "*/");
   state->type = TCOMMENT;
   state->i += 2;
   state->text_length = 0;
   return 1;
  } else {
   snprintf(state->text, 256, "%c", state->line[state->i]);
   state->type = TNORMAL;
   state->i++;
   state->text_length = 0;
   return 1;
  }
 } else {
  if(strncmp(state->line + state->i, "/*", 2) == 0)
  {
   if(state->text_length == 0)
    state->in_relooped_char = 1;

   if(state->in_relooped_char)
   {
    state->in_relooped_char = 0;
    state->in_comment = 1;
    snprintf(state->text, 256, "/*");
    state->type = TSTART;
    state->i += 2;
    state->text_length = 0;
    return 1;
   } else {
    state->text[state->text_length++] = 0;
    state->type = TNORMAL;
    state->i += 0;
    state->text_length = 0;
    state->in_relooped_char = 1;
    return 1;
   }
  }
 }

 if(state->in_relooped_char == 0)
 {
  if(state->line[state->i] == '\\')
   state->text[state->text_length++] = '\\';

  if(state->line[state->i] == '"')
   state->text[state->text_length++] = '\\';
 }

 if(state->in_preprocessor_line)
 {
  snprintf(state->text, 256, "%c", state->line[state->i]);
  state->type = TNORMAL;
  state->i += 1;
  state->text_length = 0;
  return 1;
 }

 if( (state->inside_braces == 0) &&
     (state->in_argument_name == 0) )
 {
  if(strncmp(state->line + state->i, "struct",  6) == 0)
  {
   if(state->text_length == 0)
    state->in_relooped_char = 1;

   if(state->in_relooped_char)
   {
    state->in_relooped_char = 0;
    state->in_object_prototype = 1;
    snprintf(state->text, 256, "struct");
    state->type = TSTART;
    state->i += 6;
    state->text_length = 0;
    return 1;
   } else {
    /* I'm not even sure if this case is real */
    state->text[state->text_length++] = 0;
    state->type = TNORMAL;
    state->i += 0;
    state->text_length = 0;
    state->in_relooped_char = 1;
    return 1;
   }
  }
 }

 if(state->line[state->i] == '{')
 {
  if(state->in_quotes == 0)
   state->inside_braces++;

  if( (state->inside_braces == 1) &&
      (state->in_object_prototype == 1) )
  {
   if(state->text_length == 0)
    state->in_relooped_char = 1;

   if(state->in_relooped_char)
   {
    state->in_relooped_char = 0;
    state->in_object_prototype = 0;
    state->in_object_definition = 1;
    state->i++;
    state->type = TNORMAL;
    state->text_length = 0;
    snprintf(state->text, 256, "{");
    return 1;
   } else {
    state->in_relooped_char = 1;
    state->inside_braces--;
    state->text[state->text_length++] = 0;
    state->type = TOBJECT;
    state->i += 0;
    state->text_length = 0;
    return 1;
   }
  }
 }

 if(state->line[state->i] == '}')
 {
  if(state->in_quotes == 0)
   state->inside_braces--;

  if( (state->inside_braces == 0) &&
      (state->in_object_definition) )
   state->in_object_definition = 0;
 }

 if(state->in_quotes)
 {
  if(state->line[state->i] == '"')
  {
   if(state->in_escape == 0)
   {
    state->in_quotes = 0;
    snprintf(state->text + state->text_length, 256, "\"");
    state->type = TSTRING;
    state->i += 1;
    state->text_length = 0;
    return 1;
   } else {
    state->in_escape = 0;
   }
  } else {

   if(state->in_format_modifier)
   {
    for(x=0; x<5; x++)
    {
     if(state->line[state->i] == "csduoxXfge"[x])
     {
      state->text[state->text_length++] = state->line[state->i];
      state->text[state->text_length] = 0;
      state->text_length = 0;
      state->in_format_modifier = 0;
      state->type = TFORMATMODIFIER;
      state->i += 1;
      return 1;
     }
    }

    if( (state->text_length > 12) ||
        isblank(state->line[state->i]) )
    {
     snprintf(state->text + state->text_length, 256, "%c", state->line[state->i]);
     state->text_length = 0;
     state->in_format_modifier = 0;
     state->type = TFORMATMODIFIER;
     state->i++;
     return 1;
    }

    state->text[state->text_length++] = state->line[state->i];
    state->type = TWAIT;
    state->i++;
    return 1;
   }

   if(state->line[state->i] == '%')
   {
    if(state->in_relooped_char)
    {
     state->in_relooped_char = 0;
     state->in_format_modifier = 1;
     state->type = TWAIT;
     state->i++;
     state->text_length = 0;
     state->text[state->text_length++] = '%';
     return 1;
    } else {
     state->in_relooped_char = 1;
     state->text[state->text_length] = 0;
     state->type = TSTRING;
     state->i += 0;
     state->text_length = 0;
     return 1;
    }
   }

   if(state->line[state->i] == '\\')
   {
    if(state->in_escape)
     state->in_escape = 0;
    else
     state->in_escape = 1;
   }

   x = 0;
   while(escape_sequences[x].w != NULL)
   {
    if(strncmp(state->line + state->i, escape_sequences[x].w, escape_sequences[x].l) == 0)
    {
     if(state->in_relooped_char)
     {
      state->in_relooped_char = 0;
      state->text_length = 0;
      snprintf(state->text, 256, "\\%s", escape_sequences[x].w);
      state->type = TESCAPESEQUENCE;
      state->i += (escape_sequences[x].l);
      return 1;
     } else {
      state->in_relooped_char = 1;
      state->text[--state->text_length] = 0;
      state->type = TSTRING;
      state->i += 0;
      state->text_length = 0;
      return 1;
     }
    } 
    x++;
   }

   snprintf(state->text + state->text_length, 256, "%c", state->line[state->i]);
   state->type = TNORMAL;
   state->i += 1;
   state->text_length = 0;
   return 1;
  }

 } else {
  if(state->line[state->i] == '"')
  {
   if(state->in_relooped_char)
    state->in_relooped_char = 0;

   if(state->text_length)
   {
    state->text[--state->text_length] = 0;
    state->type = TNORMAL;
    state->i += 0;
    state->in_relooped_char = 1;
    state->text_length = 0;
    return 1;
   } else {
    snprintf(state->text + state->text_length, 256, "\\\"");
    state->type = TSTART;
    state->i += 1;
    state->text_length = 0;
    state->in_quotes = 1;
    return 1;
   }
  }
 }

 x = 0;
 while(declorations[x].w != NULL)
 {
  if(strncmp(state->line + state->i, declorations[x].w, declorations[x].l - 1) == 0)
  {
   state->follows_declaration = 1;

   yes = 1;
   if(state->i > 0)
    if( (state->line[state->i - 1] != ' ') &&
        (state->line[state->i - 1] != '(') &&
        (state->line[state->i - 1] != '{') )
     yes = 0;
 
   if(yes)
   {
    if(state->in_argument_name)
    {
     if(state->text_length > 0)
     {
      state->text[state->text_length++] = 0;
      state->type = TNORMAL;
      state->i += 0;
      state->text_length = 0;
      return 1;
     }
    }
    state->text_length = 0;
    memcpy(state->text, declorations[x].w, declorations[x].l - 1);
    state->text[declorations[x].l - 1] = 0;
    state->type = TDECLARATOR;
    state->i += (declorations[x].l - 1);

    if(state->in_prototype)
     state->in_argument_name = 1;

    return 1; 
   }
  }
  x++;
 }

// if( (state->in_object_definition == 0) &&
//     (state->in_object_prototype == 0) )
// {
  yes = 0;

  if(strncmp(state->line + state->i, "struct ", 7) == 0)
  {
   yes = 1;
   if(state->i > 0)
    if( (state->line[state->i - 1] != ' ') &&
        (state->line[state->i - 1] != '(') )
     yes = 0;
  }

  if(yes)
  {
   if(state->text_length == 0)
    state->in_relooped_char = 1;

   if(state->in_relooped_char)
   {
    state->in_relooped_char = 0;
    state->in_object_name = 1;
    snprintf(state->text, 256, "struct ");
    state->text_length += 7;
    state->i += 7;
    state->type = TWAIT;
    return 1;
   } else {
    state->in_relooped_char = 1;
    state->text[state->text_length++] = 0;
    state->type = TNORMAL;
    state->i += 0;
    state->text_length = 0;
    return 1;
   }
  }
// }

 if( (state->in_object_definition == 1) &&
     (state->follows_declaration == 1) )
 {
  if(state->in_relooped_char)
  {
   state->in_relooped_char = 0;
  } else {
   if( (state->line[state->i] == ';') ||
       (state->line[state->i] == ',') )
   {
    state->in_relooped_char = 1;
    state->text[state->text_length] = 0;
    state->type = TMEMBER;
    state->i += 0;
    state->text_length = 0;
    if(state->line[state->i] == ';')
     state->follows_declaration = 0;
    return 1;    
   }
  }
 }

 if(state->in_argument_name)
 {
  if(state->in_relooped_char)
  {
   state->in_relooped_char = 0;
  } else {
   if(state->line[state->i] == ')')
   {
    state->delay_line_break = 0;
    state->in_argument_name = 0;
    state->in_prototype = 0;
    state->text[state->text_length++] = 0;
    if(add_to_string_array(&(state->args), state->n_args, state->text, state->text_length, 1))
    {
     fprintf(stderr, "add_to_string_array() failed\n");
     return -1;
    }
    state->n_args++;
    state->type = TARGUMENT;
    state->i += 0;
    state->in_relooped_char = 1;
    state->text_length = 0;
    return 1;
   }

   if(state->line[state->i] == ',')
   {
    state->delay_line_break = 0;
    state->in_prototype = 0;
    state->text[state->text_length++] = 0;
    if(add_to_string_array(&(state->args), state->n_args, state->text, state->text_length, 1))
    {
     fprintf(stderr, "add_to_string_array() failed\n");
     return -1;
    }
    state->n_args++;
    state->type = TARGUMENT;
    state->i += 0;
    state->in_relooped_char = 1;
    state->text_length = 0;
    return 1;
   }

   if(isblank(state->line[state->i]))
   {
    state->text[state->text_length++] = state->line[state->i];
    state->text[state->text_length++] = 0;
    if(state->in_object_name)
    {
     state->type = TOBJECT;
     state->in_object_name = 0;
    } else {
     state->type = TNORMAL;
    }
    state->i += 1;
    state->text_length = 0;
    return 1;
   }

   state->text[state->text_length++] = state->line[state->i];
   state->delay_line_break = 1;
   state->type = TWAIT;
   state->i += 1;
   return 1; 
  }
 }

 if(state->line[state->i] == '(')
 {
  if(state->follows_operator == 0)
   if(state->inside_braces == 0)
    if(state->in_quotes == 0)
     state->in_argument_name = 1;
 }

 state->follows_operator = 0;

 x = 0;
 while(keywords[x].w != NULL)
 {
  if(strncmp(state->line + state->i, keywords[x].w, keywords[x].l - 1) == 0)
  {
   yes = 1;
   if(state->i > 0)
    if( (state->line[state->i - 1] != ' ') &&
        (state->line[state->i - 1] != '(') &&
        (state->line[state->i - 1] != '{') )
     yes = 0;
 
   if(yes)
   {
    memcpy(state->text, keywords[x].w, keywords[x].l - 1);
    state->text[keywords[x].l - 1] = 0;
    state->type = TKEYWORD;
    state->i += (keywords[x].l - 1);
    return 1; 
   }
  }
  x++;
 }

 x = 0;
 while(operators[x].w != NULL)
 {
  if(strncmp(state->line + state->i, operators[x].w, operators[x].l) == 0)
  {
   yes = 1;
 
   if(yes)
   {
    if(state->text_length == 0)
     state->in_relooped_char = 1;

    if(state->in_relooped_char == 1)
    {
     state->in_relooped_char = 0;
     memcpy(state->text, operators[x].w, operators[x].l);
     state->text[operators[x].l] = 0;
     state->type = TOPERATOR;
     state->i += (operators[x].l);
     state->follows_operator = 1;
     return 1; 
    } else {
     state->in_relooped_char = 1;
     state->text[state->text_length] = 0;
     state->i += 0;
     state->text_length = 0;
     state->type = TNORMAL; 
     for(y=0; y<state->n_args; y++)
     {
      if(strcmp(state->args[y], state->text) == 0)
      {
       state->type = TARGUMENT;
       return 1;
      }
     }
     return 1;
    }
   }
  }
  x++;
 }

 if(state->in_relooped_char)
  state->in_relooped_char = 0;

 if(state->text_length > 250)
 {
  snprintf(state->text + state->text_length, 256, "%c", state->line[state->i]);
  state->text_length = 0;
  state->type = TNORMAL;
  state->i++;
  return 1;
 }  

 if( isalnum(state->line[state->i]) || (state->line[state->i] == '_') )
 {
  state->text[state->text_length++] = state->line[state->i];
  state->type = TWAIT;
  state->i++;
  return 1;
 } else {

  if(state->text_length > 0)
  {
   /* was that a function name? */
   if(isblank(state->line[state->i]) ||
      (state->line[state->i] == '(') )
   {
    yes = state->line[state->i] == '(';
    y = state->i + 1;
    while( (yes == 0) && (y < state->line_length) )
    {
     if(state->line[state->i + y] == '(')
      yes = 1;
     y++;
    }

    if(yes)
    {
     state->text[state->text_length] = 0;
     state->in_relooped_char = 1;
     state->text_length = 0;
     state->type = TFUNCTION; 
     state->i += 0;
     return 1;
    }
   }

   /* was that an argument? */
   for(y=0; y<state->n_args; y++)
   {
    if(strcmp(state->args[y], state->text) == 0)
    {
     state->text[state->text_length] = 0;
     state->in_relooped_char = 1;
     state->text_length = 0;
     state->type = TARGUMENT;
     state->i += 0;
     return 1;
    }
   }

   /* was that an object name? */
   yes = 0;
   if(state->in_object_prototype)
    yes = 1;

   if(state->in_object_name)
   {
    state->in_object_name = 0;
    yes = 1;
   }

   if(yes)
   {
    state->text[state->text_length] = 0;
    state->in_relooped_char = 1;
    state->text_length = 0;
    state->type = TOBJECT;
    state->i += 0;
    return 1;
   }

  }

  snprintf(state->text + state->text_length, 256, "%c", state->line[state->i]);
  state->text_length = 0;
  state->type = TNORMAL;
  state->i++;
  return 1;
 }


}

int convert_c_source(struct locolisting_context *ctx, char *filename)
{
 struct c_source_state tstate, *state = &tstate;
 int first_line, output_this_line, code;

 memset(state, 0, sizeof(struct c_source_state));

 if((ctx->current_file = fopen(filename, "r")) == NULL)
 {
  fprintf(stderr, "fopen(%s, r) failed\n", filename);
  return 1;
 }

 printf("listing:%s;\n", filename);
 printf(" attribute:listingtype,sourcecode;\n");

 if(ctx->line_numbers == 1)
  printf(" attribute:linenumbers,yes;\n");

 if(ctx->line_numbers == 0)
  printf(" attribute:linenumbers,no;\n");

 first_line = 1;

 while(read_line(ctx) == 0)
 {
  state->line = ctx->line_buffer;
  state->line_length = ctx->line_length;
  ctx->line_number++;

  if(first_line)
  {
   printf(" object:text,\"");
   first_line = 0;
  } else {
   if(state->in_comment_line)
   {
    printf("  attribute:syntaxhighlight,c_comment;\n");
    state->in_comment_line = 0;
   } 

   if(state->in_object_prototype)
    printf("  attribute:syntaxhighlight,c_object;\n");

   if(state->delay_line_break == 0)
    printf(" object:linebreak,\"");
  }

  if(state->line_length == 0)
  {
   printf("\";\n");
  } else {
   state->i = 0;
   state->only_whitespace = 1;
   state->in_preprocessor_line = 0;
   output_this_line = 0;
   state->in_comment_line = 0;
   while((code = next(state)) != 0)
   {
    switch(state->type)
    {
     case TERROR:
          fprintf(stderr, "failure line(%d)\n", ctx->line_number);
          return 1;
          break;

     case TPPD:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_ppd;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TKEYWORD:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_keyword;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TOPERATOR:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_operator;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TDECLARATOR:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_declarator;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TARGUMENT:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_argument;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TMEMBER:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_member;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TFUNCTION:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_function;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TESCAPESEQUENCE:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_escapesequence;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TFORMATMODIFIER:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_formatmodifier;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TOBJECT:
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_object;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TSTRING:
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_string;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TCOMMENT:
          printf("%s\";\n", state->text);
          printf("  attribute:syntaxhighlight,c_comment;\n");
          printf(" object:text,\"");
          output_this_line = 0;
          break;

     case TSTART:
          if(output_this_line)
           printf("\";\n object:text,\"");
          printf("%s", state->text);
          output_this_line = 1;
          break;

     case TNORMAL:
          printf("%s", state->text);
          output_this_line = 1;
          break;
     
    }

   }

   if(state->delay_line_break == 0)
   {
    if(state->text_length > 0)
    {
     state->text[state->text_length] = 0;
     printf("%s", state->text);
     state->text_length = 0;
    }
    printf("\";\n");
   }
  }
 }

 fclose(ctx->current_file);
 return 0;
}


