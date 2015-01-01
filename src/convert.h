#ifndef _CONVERT_H_
#define _CONVERT_H_

struct locolisting_context
{
 char *title;
 char *filename;
 int verbose;
 char *conversion_type;
 char *line_buffer;
 char *input_buffer;
 int line_buffer_size;
 int input_buffer_size;
 int line_length;
 int input_length;
 int line_number;
 char *current_filename;
 FILE *current_file;
 int line_numbers;
 int width, height, l_crop, r_crop, t_crop, b_crop;
};

int add_to_string_array(char ***array, int array_size, 
                        char *string, int string_length,
                        int prevent_duplicates);

int free_string_array(char **array, int n_elements);

int read_line(struct locolisting_context *ctx);

int convert_c_source(struct locolisting_context *ctx, char *filename);

int convert_screen_log(struct locolisting_context *ctx, char *filename);

int convert_plaintext(struct locolisting_context *ctx, char *filename);


#endif

