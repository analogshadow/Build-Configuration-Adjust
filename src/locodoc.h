#ifndef _locodoc_h_
#define _locodoc_h_

#include <stdio.h>

#define LD_DEPTH_PROJECT    1
#define LD_DEPTH_SET        2
#define LD_DEPTH_VOLUME     3
#define LD_DEPTH_PART       4
#define LD_DEPTH_CHAPTER    5
#define LD_DEPTH_SECTION    6
#define LD_DEPTH_SUBSECTION 7
#define LD_DEPTH_INSET      8
#define LD_DEPTH_LISTING    9

#define LD_FAIL      1
#define LD_DONE      0
#define LD_MULTIPASS 2

#define LD_OBJECT_MODE_CONTINUE        12
#define LD_OBJECT_MODE_PARAGRAPH_BREAK 1
#define LD_OBJECT_MODE_LINE_BREAK      2
#define LD_OBJECT_MODE_INDEX_TERM      3
#define LD_OBJECT_MODE_FOOTNOTE        4
#define LD_OBJECT_MODE_IMAGE           5
#define LD_OBJECT_MODE_TABLE           6
#define LD_OBJECT_MODE_LIST            7
#define LD_OBJECT_MODE_GLOSSARY_TERM   8
#define LD_OBJECT_MODE_LINK            9
#define LD_OBJECT_MODE_QUOTE           10
#define LD_OBJECT_MODE_CITATION        11
#define LD_OBJECT_MODE_WRAP            13

struct loco_attribute
{
 int mode;
 char *name;
 char *value;
 void *data;
};

struct loco_object
{
 int mode;
 char *value;
 void *data;
 struct loco_attribute *attributes;
};

struct loco_project;
struct loco_set;
struct loco_volume;
struct loco_part;
struct loco_chapter;
struct loco_section;
struct loco_subsection;
struct loco_inset;
struct loco_listing;

struct loco_project
{
 int mode;
 char *name;
 struct loco_set *sets;
 struct loco_attribute *attributes;
 int topmost_effective_division_element;
 void *backend_toc_data;
};

struct loco_set
{
 int mode;
 char *name;
 struct loco_volume *volumes;
 struct loco_attribute *attributes;
 void *backend_toc_data;
};

struct loco_volume
{
 int mode;
 char *name;
 struct loco_part *parts;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_set *parrent;
};

struct loco_part
{
 int mode;
 char *name;
 struct loco_chapter *chapters;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_volume *parrent;
};

struct loco_chapter
{
 int mode;
 char *name;
 struct loco_section *sections;
 struct loco_object *objects;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_part *parrent;
};

struct loco_section
{
 int mode;
 char *name;
 struct loco_subsection *subsections;
 struct loco_object *objects;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_chapter *parrent;
};

struct loco_subsection
{
 int mode;
 char *name;
 struct loco_inset *insets;
 struct loco_object *objects;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_section *parrent;
};

struct loco_inset
{
 int mode;
 char *name;
 struct loco_listing *listings;
 struct loco_object *objects;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_subsection *parrent;
};

struct loco_listing
{
 int mode;
 char *name;
 struct loco_object *objects;
 struct loco_attribute *attributes;
 void *backend_toc_data;
 struct loco_inset *parrent;
};

struct locdoc_vtable;

struct loco_parse_context
{
 struct locodoc_context *ctx;
 struct loco_project *project;
 char *line_buffer;
 char *input_buffer;
 int line_buffer_size;
 int input_buffer_size;
 int line_length;
 int input_length;
 int line_number;
 char *current_filename;
 FILE *current_file;

 struct loco_object *current_object;
 int current_parrent_type;
 void *current_parrent, *current_base;
};


struct locodoc_context
{
 struct loco_project *project;
 char *filename;
 void *vtable_data;
 struct locdoc_vtable *ops;
 int verbose, project_created_at_command_line;
 struct loco_parse_context parser;
};

struct loco_table
{
 struct loco_object *objects;
 struct loco_attribute *attributes;
 char *style_name;
 struct loco_table *down, *right;
};

struct loco_project *loco_parse(struct locodoc_context *ctx, char *first_file);

struct locdoc_vtable
{
 int (*project_start)        (struct locodoc_context *ctx);
 int (*project_attribute)    (struct locodoc_context *ctx, struct loco_attribute *a);
 int (*project_finish)       (struct locodoc_context *ctx);
 int (*set_start)            (struct locodoc_context *ctx, struct loco_set *x);
 int (*set_attribute)        (struct locodoc_context *ctx, struct loco_set *x, struct loco_attribute *a);
 int (*set_finish)           (struct locodoc_context *ctx, struct loco_set *x);
 int (*volume_start)         (struct locodoc_context *ctx, struct loco_volume *x);
 int (*volume_attribute)     (struct locodoc_context *ctx, struct loco_volume *x, struct loco_attribute *a);
 int (*volume_finish)        (struct locodoc_context *ctx, struct loco_volume *x);
 int (*part_start)           (struct locodoc_context *ctx, struct loco_part *x);
 int (*part_attribute)       (struct locodoc_context *ctx, struct loco_part *x, struct loco_attribute *a);
 int (*part_finish)          (struct locodoc_context *ctx, struct loco_part *x);
 int (*chapter_start)        (struct locodoc_context *ctx, struct loco_chapter *x);
 int (*chapter_attribute)    (struct locodoc_context *ctx, struct loco_chapter *x, struct loco_attribute *a);
 int (*chapter_object)       (struct locodoc_context *ctx, struct loco_chapter *x, struct loco_object *o);
 int (*chapter_finish)       (struct locodoc_context *ctx, struct loco_chapter *x);
 int (*section_start)        (struct locodoc_context *ctx, struct loco_section *x);
 int (*section_attribute)    (struct locodoc_context *ctx, struct loco_section *x, struct loco_attribute *a);
 int (*section_object)       (struct locodoc_context *ctx, struct loco_section *x, struct loco_object *o);
 int (*section_finish)       (struct locodoc_context *ctx, struct loco_section *x);
 int (*subsection_start)     (struct locodoc_context *ctx, struct loco_subsection *x);
 int (*subsection_attribute) (struct locodoc_context *ctx, struct loco_subsection *x, struct loco_attribute *a);
 int (*subsection_object)    (struct locodoc_context *ctx, struct loco_subsection *x, struct loco_object *o);
 int (*subsection_finish)    (struct locodoc_context *ctx, struct loco_subsection *x);
 int (*inset_start)          (struct locodoc_context *ctx, struct loco_inset *x);
 int (*inset_attribute)      (struct locodoc_context *ctx, struct loco_inset *x, struct loco_attribute *a);
 int (*inset_object)         (struct locodoc_context *ctx, struct loco_inset *x, struct loco_object *o);
 int (*inset_finish)         (struct locodoc_context *ctx, struct loco_inset *x);
 int (*listing_start)        (struct locodoc_context *ctx, struct loco_listing *x);
 int (*listing_attribute)    (struct locodoc_context *ctx, struct loco_listing *x, struct loco_attribute *a);
 int (*listing_object)       (struct locodoc_context *ctx, struct loco_listing *x, struct loco_object *o);
 int (*listing_finish)       (struct locodoc_context *ctx, struct loco_listing *x);
 int (*object_attribute)     (struct locodoc_context *ctx, struct loco_object *x, struct loco_attribute *a);
};

int loco_n_sets(struct loco_set *x);
int loco_n_volumes(struct loco_volume *x);
int loco_n_parts(struct loco_part *x);
int loco_n_chapters(struct loco_chapter *x);
int loco_n_sections(struct loco_section *x);
int loco_n_subsections(struct loco_subsection *x);
int loco_n_insets(struct loco_inset *x);
int loco_n_listings(struct loco_listing *x);
int loco_n_attributes(struct loco_attribute *x);
int loco_n_objects(struct loco_object *x);

struct locdoc_vtable *debug_backend_init(int argc, int current_arg, char **argv);
struct locdoc_vtable *html_backend_init(int argc, int current_arg, char **argv);

int loco_utf8_to_utf32(char *utf8, int length, unsigned int **utf32, int **size);
int loco_utf32_to_utf8(unsigned int *utf32, int length, char **utf, int **size);

struct loco_index_iterator
{
 char *utf8;
 unsigned int *utf32;
 void *backend_data;
 int buffer_offset;
};

struct loco_index_iterator *
loco_index_iterate(void *index_buffer, struct loco_index_iterator *start, int *error);

struct loco_index_iterator *
loco_index_insert(void **index_buffer, int *buffer_size,
                  char *utf8, int utf8_length);

int loco_index_entry_update(void *index_buffer, struct loco_index_iterator *i);

struct loco_attribute *loco_attribute_find(struct loco_attribute *set, char *name);

struct loco_attribute *loco_attribute_merge(struct loco_attribute *set1, 
                                            struct loco_attribute *set2);

char *loco_string_to_filename(char *string);

struct loco_set *loco_append_set_array(struct loco_set **base);
struct loco_volume *loco_append_volume_array(struct loco_volume **base);
struct loco_part *loco_append_part_array(struct loco_part **base);
struct loco_chapter *loco_append_chapter_array(struct loco_chapter **base);
struct loco_section *loco_append_section_array(struct loco_section **base);
struct loco_subsection *loco_append_subsection_array(struct loco_subsection **base);
struct loco_inset *loco_append_inset_array(struct loco_inset **base);
struct loco_listing *loco_append_listing_array(struct loco_listing **base);
struct loco_attribute *loco_append_attribute_array(struct loco_attribute **base);
struct loco_object *loco_append_object_array(struct loco_object **base);

int loco_utf8_char_read(char *input, int length, int *skip);

int read_line(struct loco_parse_context *parser);


#endif

