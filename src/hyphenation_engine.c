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


#include "prototypes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef WITHOUT_LIBHNJ
#include <hyphen.h>
#endif

struct hyphenation_context
{
 char *language;

#ifndef WITHOUT_LIBHNJ
 HyphenDict *hyphenation_rules_db;
 char hyphens[256 + 6];
#endif
};

struct hyphenation_context *
hyphenation_engine_initialize(char *language)
{
 struct hyphenation_context *hc;
 int allocation_size;
#ifndef WITHOUT_LIBHNJ
 char rules_db_file_path[1024];
#endif

 allocation_size = sizeof(struct hyphenation_context);
 if((hc = (struct hyphenation_context *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "BCA: malloc(%d) failed: %s\n",
          allocation_size, strerror(errno));
  hyphenation_engine_finalize(hc);
  return NULL;
 }
 memset(hc, 0, allocation_size);

 if(language != NULL)
 {
  allocation_size = strlen(language) + 1;
  if((hc->language = (char *) malloc(allocation_size)) == NULL)
  {
   fprintf(stderr, "BCA: malloc(%d) failed: %s\n",
           allocation_size, strerror(errno));
   hyphenation_engine_finalize(hc);
   return NULL;
  }
  snprintf(hc->language, allocation_size, "%s", language);

#ifndef WITHOUT_LIBHNJ
  snprintf(rules_db_file_path, 1024, "/usr/share/hyphen/hyph_%s.dic", language);
  if((hc->hyphenation_rules_db = hnj_hyphen_load(rules_db_file_path)) == NULL)
  {
   fprintf(stderr,
           "BCA: hyphenation_engine_initialize(): hnj_hyphen_load(%s) failed\n",
           rules_db_file_path);
   return NULL;
  }
#endif
 }

 return hc;
}

int hyphenation_engine_finalize(struct hyphenation_context *hc)
{
 if(hc != NULL)
 {
  if(hc->language != NULL)
   free(hc->language);

#ifndef WITHOUT_LIBHNJ
  if(hc->hyphenation_rules_db != NULL)
   hnj_hyphen_free(hc->hyphenation_rules_db);
#endif

  free(hc);
 }
 return 0;
}

int is_capitalized(char *utf8_string, int n_bytes)
{
 /* this will need an external library */
 if(n_bytes > 1)
 {
  if(isupper(utf8_string[0]))
   return 1;
 }
 return 0;
}

/* hyphenation logic

 After word isolation (white space based), UTF-8 validation & character counting, hyphenation
 can take place.

 If the word is capitalized, it is considered either a proper noun or the first word of a sentence,
 and as a matter of style not hyphenated.

 If a word is less than four letters it is not hyphenated.

 If the word has an em dash in it, a warning is printed (since that is not correct) and the word
 is not hyphenated.

 If the word has an other hyphenation marks, those are used for hyphenation points.

 If enabled, and with a supported langauge, soft hyphenation are added to the word.

 Rendering engines then hyphenate or soft or real hyphens, and never printing soft hyphens unless
 they are used.

*/


/* Hyphenate words for splitting across lines.
   -1 failure, 0 hyphenation not found, 1 hyphenation found */
int hyphenation_engine_attempt(struct hyphenation_context *hc, int fit_size,
                               struct unicode_word_context *source,
                               struct unicode_word_context *first,
                               struct unicode_word_context *second)
{
 int i;
#ifndef WITHOUT_LIBHNJ
 int code;
 char **rep = NULL;
 int *pos = NULL, *cut = NULL;
#endif

 if(source->n_characters < fit_size)
 {
  fprintf(stderr, "BCA: hyphenation_engine_attempt(): "
          "word of length %d should fit into %d available characters\n",
          source->n_characters, fit_size);
  return -1;
 }

 /* In English at least, proper nouns are not hyphenated.
    Commonly, the first word of a sentense is also not. Or at
    least to simply here, we can call it a style. */
 if(is_capitalized(source->word_buffer, source->buffer_length))
  return 0;

 /* is soft hyphenation provided? */

 /* if the word has any multi-byte UTF-8 characters, we don't know
    how to hyphenate */
 if(source->buffer_length != source->n_characters)
  return 0;

 /* is it already hyphenated? (needs unicode support) */
 for(i=0; i< source->buffer_length; i++)
 {
  if(source->word_buffer[i] == '-')
  {
   if(i > fit_size)
   {
    return 0;
   } else {
    memcpy(first->word_buffer, source->word_buffer, i + 1);
    first->word_buffer[i + 1] = 0;
    first->buffer_length = first->n_characters = i;

    second->buffer_length = second->n_characters = source->n_characters - (i + 1);
    memcpy(second->word_buffer, source->word_buffer + i + 1, second->n_characters);
    second->word_buffer[second->n_characters] = 0;
    return 1;
   }
  }
 }

#ifndef WITHOUT_LIBHNJ
 /* hnj library available? */
 if(hc->hyphenation_rules_db != NULL)
 {
  if(hnj_hyphen_hyphenate2(hc->hyphenation_rules_db,
                           source->word_buffer, source->buffer_length,
                           hc->hyphens, NULL, &rep, &pos, &cut) == 0)
  {
   for(i = fit_size - 2; i > 0; i--)
   {
    /* the hyphens buffer store hnj processing state and the hyphenation points.
       possible break points are noted with odd numbers (in ASCII text) */
    code = hc->hyphens[i];
    if( (code > '9') || (code < '0') )
    {
     fprintf(stderr, "BCA: should this happen?\n");
     return -1;
    }

    code -= '0'; /* ASCII -> int */
    if((code % 2) == 1) /* is odd number? */
    {
     memcpy(first->word_buffer, source->word_buffer, ++i);
     first->word_buffer[i++] = '-';
     first->word_buffer[i] = 0;
     first->buffer_length = first->n_characters = i;

     second->buffer_length = second->n_characters = source->n_characters - (i - 1);
     memcpy(second->word_buffer, source->word_buffer + (i - 1), second->n_characters);
     second->word_buffer[second->n_characters] = 0;
     return 1;
    }
   }
  }
 }
#endif

 return 0;
}
