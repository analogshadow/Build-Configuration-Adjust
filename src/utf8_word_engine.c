#include "prototypes.h"

#define UWC_WORD 1
#define UWC_BROKEN_WORD 2

struct unicode_word_context *
unicode_word_engine_initialize(void *data,
                               int (*consume_word) (struct unicode_word_context *uwc,
                                                    void *data, int flags))
{
 struct unicode_word_context *uwc;
 int allocation_size;

 allocation_size = sizeof(struct unicode_word_context);
 if((uwc = (struct unicode_word_context *)
           malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: unicode_word_engine_initialize() malloc(%d) failed\n",
          allocation_size);
  return NULL;
 }

 uwc->data = data;
 uwc->consume_word = consume_word;
 uwc->buffer_size = 256;
 uwc->buffer_length = 0;
 uwc->n_characters = 0;
 uwc->direction = 0;
 uwc->character_buffer_length = 0;
 uwc->expected_character_length = 0;

 return uwc;
}

int unicode_word_engine_finalize(struct unicode_word_context *uwc)
{
 free(uwc);
 return 0;
}

int unicode_word_engine_consume_byte(struct unicode_word_context *uwc, unsigned char byte)
{
 uwc->character_buffer[uwc->character_buffer_length++] = (char) byte;

 if(uwc->expected_character_length == 0)
 {
  if( (byte & 128) == 0)
  {
   /* 1 byte UTF-8 character */
   uwc->expected_character_length = 1;

  } else if( (byte & 224) == 192) {
   /* 2 byte UTF-8 character */
   uwc->expected_character_length = 2;

  } else if( (byte & 240) == 224) {
   /* 3 byte UTF-8 character */
   uwc->expected_character_length = 3;

  } else if( (byte & 248) == 240) {
   /* 4 byte UTF-8 character */
   uwc->expected_character_length = 4;

  } else if( (byte & 252) == 248) {
   /* 5 byte UTF-8 character */
   uwc->expected_character_length = 5;

  } else if( (byte & 254) == 252) {
   /* 6 byte UTF-8 character */
   uwc->expected_character_length = 6;

  } else {
   fprintf(stderr,
           "BCA: unicode_word_engine_consume_byte(): UTF-8 encoding error:"
           "byte value out of sequence: %xh\n", byte);
   return 1;
  }

 } else {
  if( (byte & 192) != 128)
  {
   fprintf(stderr,
           "BCA: unicode_word_engine_consume_byte(): UTF-8 encoding error:"
           "byte value out of sequence: %xh (expected byte %d of a %d byte value)\n",
           byte, uwc->character_buffer_length + 1, uwc->expected_character_length);
   return 1;
  }
 }

 if(uwc->character_buffer_length < uwc->expected_character_length)
 {
  /* need more bytes */
  return 0;
 }

 /* NULL terminate */
 uwc->character_buffer[uwc->character_buffer_length] = 0;

 if(is_white_space((char *) uwc->character_buffer))
 {
  /* if it is white space, does this terminate a word? */
  if(uwc->n_characters > 0)
  {
   uwc->word_buffer[uwc->buffer_length] = 0;
   if(uwc->consume_word(uwc, uwc->data, UWC_WORD))
    return 1;

   uwc->buffer_length = 0;
   uwc->n_characters = 0;
  }

  /* character was white space before a word; do nothing */
  uwc->character_buffer_length = 0;
  uwc->expected_character_length = 0;
  return 0;
 }

 /* do we need to break the word just because of it's size? */
 if(uwc->buffer_length + uwc->character_buffer_length + 1 > uwc->buffer_size)
 {
  uwc->word_buffer[uwc->buffer_length] = 0;
  if(uwc->consume_word(uwc, uwc->data, UWC_BROKEN_WORD))
   return 1;

  uwc->buffer_length = 0;
  uwc->n_characters = 0;
 }

 /* append character buffer to word buffer, then reset character buffer */
 memcpy(uwc->word_buffer + uwc->buffer_length,
        uwc->character_buffer,
        uwc->character_buffer_length);
 uwc->buffer_length += uwc->character_buffer_length;
 uwc->n_characters++;

 uwc->character_buffer_length = 0;
 uwc->expected_character_length = 0;
 return 0;
}

int is_white_space(char *utf8_character)
{
 /* space */
 if(utf8_character[0] == ' ')
  return 1;

 /* tab */
 if(utf8_character[0] == '\t')
  return 1;

 /* newline */
 if(utf8_character[0] == '\n')
  return 1;

 return 0;
}
