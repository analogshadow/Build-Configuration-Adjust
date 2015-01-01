#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "convert.h"


#define SCREEN_COLOR_BLACK   256
#define SCREEN_COLOR_RED     257
#define SCREEN_COLOR_GREEN   258
#define SCREEN_COLOR_YELLOW  259
#define SCREEN_COLOR_BLUE    260
#define SCREEN_COLOR_MAGENTA 261
#define SCREEN_COLOR_CYAN    262
#define SCREEN_COLOR_WHITE   263

#define SCREEN_ATTR_BOLD   1

struct screen_cell
{
 int background;
 int foreground;
 int attributes;
 char utf8[8];
};

struct screen_log_state
{
 int current_background;
 int current_foreground;
 int current_attributes;
 int n_rows, n_columns;
 struct screen_cell *cells;
 int in_escape;
 int *tab_stops;
};

int set_char(struct locolisting_context *ctx, struct screen_log_state *state,
             int x, int y, char *utf8)
{
 if(x > ctx->width)
  return 1;

 if(y > ctx->height)
  return 1;

 snprintf(state->cells[(y * ctx->width) + x].utf8, 8, "%s", utf8);

 state->cells[(y * ctx->width) + x].background = state->current_background;
 state->cells[(y * ctx->width) + x].foreground = state->current_foreground;
 state->cells[(y * ctx->width) + x].attributes = state->current_attributes;

 return 0;
}

int scroll(struct locolisting_context *ctx, struct screen_log_state *state)
{
 int x;

 memmove(state->cells,
         state->cells + (1 * ctx->width),
         (ctx->height - 2) * ctx->width);

 for(x=0; x<ctx->width; x++)
 {
  if(set_char(ctx, state, x, ctx->height - 1, " "))
   return 1;
 }

 return 0;
}

int set_display_attributes(struct locolisting_context *ctx, 
                           struct screen_log_state *state,
                           unsigned char *buffer)
{
 int i = 2, length, z = 0, arg, handled;
 char temp[8];

 length = strlen(buffer);

 while(i < length)
 {
  if(isdigit(buffer[i]))
  {
   if(z > 7)
   {
    fprintf(stderr, "set_display_attribute(%s) - numeric arg too long\n", buffer + 1);
    return 0;
   }
   temp[z++] = buffer[i];
   i++;
   continue;
  }

  if( (buffer[i] == ';') ||
      (buffer[i] == 'm') )
  {
   temp[z] = 0;
   sscanf(temp, "%d", &arg);

   handled = 0;

   if(arg == 0)
   {
    state->current_attributes = 0;
    state->current_background = 0;
    state->current_foreground = 0;
    handled = 1;
   }

   if(arg == 1)
   {
    state->current_attributes = SCREEN_ATTR_BOLD;
    handled = 1;
   }

   if( (arg > 29) && (arg < 38) )
   {
    state->current_foreground = (arg - 30) + 256;
    handled = 1;
   }

   if( (arg > 39) && (arg < 48) )
   {
    state->current_background = (arg - 40) + 256;
    handled = 1;
   }

   if(handled == 0)
   {
    fprintf(stderr, "set_display_attribute(%s) - unknown numeric arg %d\n", buffer + 1, arg);
    return 0;
   }   
   
   z = 0;
   i++;
   continue;
  }

  fprintf(stderr, "set_display_attribute(%s) - no clue\n", buffer + 1);
  return 0;
 }

 return 0;
}

int handle_numeric(struct locolisting_context *ctx, 
                   struct screen_log_state *state,
                   unsigned char *buffer, int i)
{
 if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
  return 1;

 buffer[++i] = 0;

 if(buffer[i-1] == ';')
 {
  if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
   return 1;

  buffer[++i] = 0;
  return handle_numeric(ctx, state, buffer, i);
 }

 if(isdigit(buffer[i-1]))
 {
  return handle_numeric(ctx, state, buffer, i);
 }

 if(buffer[i-1] == 'h')
 {
  fprintf(stderr, "unhandled escape sequence: %s (terminal setup)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'l')
 {
  fprintf(stderr, "unhandled escape sequence: %s (terminal setup)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'm')
  return set_display_attributes(ctx, state, buffer);

 if(buffer[i-1] == 'K')
 {
  fprintf(stderr, "unhandled escape sequence: %s (text erase)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'J')
 {
  fprintf(stderr, "unhandled escape sequence: %s (text erase)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'i')
 {
  fprintf(stderr, "unhandled escape sequence: %s (printing)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'q')
 {
  fprintf(stderr, "unhandled escape sequence: %s (LED control)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'g')
 {
  fprintf(stderr, "unhandled escape sequence: %s (tab control)\n", buffer + 1);
  return 0;
 }

 if(buffer[i-1] == 'r')
 {
  fprintf(stderr, "unhandled escape sequence: %s (window size)\n", buffer + 1);
  return 0;
 }

 return 0;
}

int handle_escape(struct locolisting_context *ctx, struct screen_log_state *state)
{
 unsigned char buffer[16];
 int arg1, arg2, i = 1;

 buffer[0] = 0x1B;

 if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
  return 1;
 buffer[++i] = 0;

 if(buffer[1] == '=')
 {
  fprintf(stderr, "unhandled escape sequence: %s (alternate keypad mode)\n", buffer + 1);
  return 0;
 }

 if(buffer[1] == '>')
 {
  fprintf(stderr, "unhandled escape sequence: %s (numeric keypad mode)\n", buffer + 1);
  return 0;
 }

 if(buffer[1] == 'N')
 {
  fprintf(stderr, "unhandled escape sequence: %s (set single shift 2)\n", buffer + 1);
  return 0;
 }

 if(buffer[1] == 'O')
 {
  fprintf(stderr, "unhandled escape sequence: %s (set single shift 3)\n", buffer + 1);
  return 0;
 }

 if(buffer[1] == 'c')
 {
  fprintf(stderr, "unhandled escape sequence: %s (reset terminal to initial state)\n", buffer + 1);
  return 0;
 }

 if(buffer[1] == '[')
 {
  if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
   return 1;

  buffer[++i] = 0;

  if(isdigit(buffer[2]))
   return handle_numeric(ctx, state, buffer, i);

  if(buffer[2] == 'm')
  {
   return set_display_attributes(ctx, state, buffer);
  }

  if( (buffer[2] == 'H') ||
      (buffer[2] == 'f') )
  {
   fprintf(stderr, "unhandled escape sequence: %s (move cursor to upper left corner)\n", buffer + 1);
   return 0;
  }

  if(buffer[2] == ';')
  {
   if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
    return 1;

   buffer[++i] = 0;

   if( (buffer[3] == 'H') ||
       (buffer[3] == 'f') )
   {
    fprintf(stderr, "unhandled escape sequence: %s (move cursor to upper left corner)\n", buffer + 1);
    return 0;
   }

   fprintf(stderr, "unhandled escape sequence: %s\n", buffer + 1);
   return 0;
  }

  fprintf(stderr, "unhandled escape sequence: %s\n", buffer + 1);
  return 0;
 }

 if( (buffer[1] == '(') ||
     (buffer[1] == ')') )
 {
  if(fread(buffer + i, 1, 1, ctx->current_file) != 1)
   return 1;
  buffer[++i] = 0;

  fprintf(stderr, "unhandled escape sequence: %s (charset/rom control)\n", buffer + 1);
  return 0;
 }

 return 0;
}

int advance_utf8(char *buffer, FILE *input)
{
 int encoded_length = 0;
 int i = 1;

 if((buffer[0] & 128) == 0)
 {
  buffer[1] = 0;
  return 0;
 } else if((buffer[0] & 224) == 192) {
  encoded_length = 2;
 } else if((buffer[0] & 240) == 224) {
  encoded_length = 3;
 } else if((buffer[0] & 248) == 240) {
  encoded_length = 4;
 } else if((buffer[0] & 252) == 248) {
  encoded_length = 5;
 } else if((buffer[0] & 254) == 252) {
  encoded_length = 6;
 }

 if(encoded_length == 0)
 {
  fprintf(stderr, "malformed UTF-8 sequence");
  return 1;
 }

 if(fread(buffer + 1, 1, encoded_length - 1, input) != encoded_length - 1)
  return 1;

 for(i=1; i<(encoded_length - 1); i++)
 {
  if((buffer[i] & 192) != 128)
  {
   fprintf(stderr, "malformed UTF-8 sequence");
   return 1;
  }
 }

 buffer[encoded_length] = 0;
 return 0;
}

int color_code(int color)
{
 if(color == 0)
 {
  printf("normal");
  return 0;
 }

 if(color > 255)
 {
  switch(color)
  {
   case SCREEN_COLOR_BLACK:
        printf("black");
        break;

   case SCREEN_COLOR_RED:
        printf("red");
        break;

   case SCREEN_COLOR_GREEN:
        printf("green");
        break;

   case SCREEN_COLOR_YELLOW:
        printf("yellow");
        break;

   case SCREEN_COLOR_BLUE:
        printf("blue");
        break;

   case SCREEN_COLOR_MAGENTA:
        printf("magenta");
        break;

   case SCREEN_COLOR_CYAN:
        printf("cyan");
        break;

   case SCREEN_COLOR_WHITE:
        printf("white");
        break;

  }
  return 0;
 }

 return 0;
}

int render_screen_log_listing(struct locolisting_context *ctx, struct screen_log_state *state)
{
 int x, y, f = 0;
 char *c;
 int w_fg = 0, w_bg = 0, w_attr = 0, printed, text_this_line;

 if( (ctx->l_crop < 0) ||
     (ctx->r_crop < 0) ||
     (ctx->t_crop < 0) ||
     (ctx->b_crop < 0) ||
     (ctx->l_crop + ctx->r_crop > ctx->width) ||
     (ctx->t_crop + ctx->b_crop > ctx->height) )
 {
  fprintf(stderr, "invalid crop parameters\n");
  return 1;
 }

 printf("listing:\"%s\";\n", ctx->title);
 printf(" attribute:listingtype,terminalsession;\n");

 if(ctx->title != NULL)
  printf(" attribute:title,\"%s\";\n", ctx->title);

 if(ctx->line_numbers == 1)
  printf(" attribute:linenumbers,yes;\n");

 if(ctx->line_numbers == 0)
  printf(" attribute:linenumbers,no;\n");

 printf(" object:text,\"");
 for(y=ctx->t_crop; y < (ctx->height - ctx->b_crop); y++)
 {
  text_this_line = 0;
  if(f)
   printf(" object:linebreak,\"");

  f = 1;

  for(x=ctx->l_crop; x< (ctx->width - ctx->r_crop); x++)
  {
 
   if(state->cells[(y * ctx->width) + x].foreground != w_fg)
   {
    if(text_this_line)
    {
     printf("\";\n");
     printf("  attribute:terminalforeground,termfg-");
     color_code(w_fg);
     printf(";\n");
     printf(" object:text,\"");
    }

    w_fg = state->cells[(y * ctx->width) + x].foreground;
   }

   c = state->cells[(y * ctx->width) + x].utf8;
   printed = 0;
   if(c[0] == '\\')
   {
    printf("\\\\");
    printed = 1;
   }

   if(c[0] == '"')
   {
    printf("\"");
    printed = 1;
   }

   if(printed == 0)
    printf("%s", c);

   text_this_line = 1;
  }

  printf("\";\n");
  printf("  attribute:terminalforeground,termfg-");
  color_code(w_fg);
  printf(";\n");
 }

 printf("\n");
 return 0;
}

int convert_screen_log(struct locolisting_context *ctx, char *filename)
{
 struct screen_log_state tstate, *state = &tstate;
 int code, allocation_size, x = 0, y = 0;
 char read_buffer[16], *p;

 if(ctx->width == 0)
 {
  if((p = getenv("COLUMNS")) != NULL)
   sscanf(p, "%d", &(ctx->width));
 }

 if(ctx->height == 0)
 {
  if((p = getenv("LINES")) != NULL)
   sscanf(p, "%d", &(ctx->height));
 }

 if( (ctx->width == 0) ||
     (ctx->height == 0) )
 {
  fprintf(stderr, "set screen size with either --width / --height or $COLUMNS / $LINES\n");
  return 1;
 }

 if(ctx->verbose)
  fprintf(stderr, "Columns = %d, Lines = %d, Left Crop = %d, Right Crop = %d, "
          "Top Crop = %d, Bottom Crop = %d\n",
          ctx->width, ctx->height, ctx->l_crop, ctx->r_crop, ctx->t_crop, ctx->b_crop);

 memset(state, 0, sizeof(struct screen_log_state));

 if((ctx->current_file = fopen(filename, "r")) == NULL)
 {
  fprintf(stderr, "fopen(%s, r) failed\n", filename);
  return 1;
 }

 allocation_size = ctx->width * ctx->height * sizeof(struct screen_cell);
 if((state->cells = (struct screen_cell *)
                    malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "malloc(%d) failed\n", allocation_size);
  return 1;
 }
 memset(state->cells, 0, allocation_size);

 allocation_size = ctx->width * sizeof(int);
 if((state->tab_stops = (int *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "malloc(%d) failed\n", allocation_size);
  return 1;
 }

 for(x=0; x<ctx->width; x++)
 {
  if( (x != 0) &&
      (x % 8 == 0) )
  {
   state->tab_stops[x] = 1;
  } else {
   state->tab_stops[x] = 0;
  }
 }

 for(y=0; y<ctx->height; y++)
 {
  for(x=0; x<ctx->width; x++)
  {
   if(set_char(ctx, state, x, y, " "))
    return 1;
  }
 }

 x = 0;
 y = 0;
 while(!feof(ctx->current_file))
 {
  if(fread(read_buffer, 1, 1, ctx->current_file) != 1)
   break;

  /* line feed */
  if(read_buffer[0] == 0x0D)
  {
   if(y + 1 == ctx->height)
   {
    if(scroll(ctx, state))
     return 1;
   } else {
    y++;
   }

   continue;
  }

  /* carrage return */
  if(read_buffer[0] == 0x0A)
  {
   x = 0;
   continue;
  }

  /* tab */
  if(read_buffer[0] == 0x09)
  {
   while(x < ctx->width)
   {
    if(set_char(ctx, state, x, y, " "))
     return 1;

    x++;

    if(state->tab_stops[x] == 1)
     break;
   }

   continue;
  }

  /* escape sequence */
  if(read_buffer[0] == 0x1B)
  {
   if(handle_escape(ctx, state))
    return 1;
   continue;
  }

  /* printable character */
  if(advance_utf8(read_buffer, ctx->current_file))
   break;

  if(set_char(ctx, state, x, y, read_buffer))
   return 1;

  if(x + 1 == ctx->width)
  {
   x = 0;
   if(y + 1 == ctx->height)
   {
    if(scroll(ctx, state))
     return 1;
   } else {
    y++;
   }
  } else {
   x++;
  }

 }

 fclose(ctx->current_file);

 return render_screen_log_listing(ctx, state);
}

