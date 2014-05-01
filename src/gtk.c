/* GPLv3

    Build Configuration Adjust, a source configuration and Makefile
    generation tool. Copyright © 2013,2014 Stover Enterprises, LLC
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

#include "config.h"

#ifndef WITHOUT_GTK__2_0
#include "prototypes.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <gtk/gtk.h>

struct gtk_interface_context
{
 struct bca_context *ctx;
 GtkWidget *window;
 GtkTreeStore *project_configuration_store, *build_configuration_store;
 char *execute_in_terminal_command;
};

int load_preferences(struct gtk_interface_context *i);
int save_preferences(struct gtk_interface_context *i);

int reload(struct gtk_interface_context *i)
{
 GtkTreeIter iter, child;
 int x, y, offset, n_hosts;
 struct bca_context *ctx = i->ctx;
 struct component_details cd;
 char type[256], component[256], key[256], *value, **hosts;

 if(resolve_project_name(ctx) == 0)
 {
  snprintf(component, 256, "(%s) - BCA", ctx->project_name);
  gtk_window_set_title((GtkWindow *) i->window, component);
 }

 if(list_project_components(ctx, &cd))
 {
  fprintf(stderr, "BCA: list_project_components() failed.\n");
  return 1;
 }

 for(x=0; x<cd.n_components; x++)
 {
  gtk_tree_store_append(i->project_configuration_store, &iter, NULL);
  gtk_tree_store_set(i->project_configuration_store, &iter, 
                     0, cd.project_component_types[x],
                     1, cd.project_components[x], -1);

  offset = -1;
  while(iterate_key_primitives(ctx, ctx->project_configuration_contents,
                               ctx->project_configuration_length, &offset,
                               cd.project_component_types[x], 
                               cd.project_components[x], NULL,
                               type, component, key, NULL))
  {
   if((value = lookup_key(ctx, ctx->project_configuration_contents,
                          ctx->project_configuration_length,
                          type, component, key)))
   {
    gtk_tree_store_append(i->project_configuration_store, &child, &iter);
    gtk_tree_store_set(i->project_configuration_store, &child, 
                       2, key, 3, value, -1);
    free(value);
   }
  }

 }

 if(list_unique_principles(ctx, NULL, ctx->build_configuration_contents,
                           ctx->build_configuration_length, &hosts, &n_hosts))
 {
  fprintf(stderr, "BCA: list_build_hosts() failed\n");
  return 1;
 }

 for(y=0; y<n_hosts; y++)
 {
  for(x=0; x<cd.n_components; x++)
  {

   gtk_tree_store_append(i->build_configuration_store, &iter, NULL);
   gtk_tree_store_set(i->build_configuration_store, &iter, 
                      0, hosts[y], 1, cd.project_components[x], -1);

   offset = -1;
   while(iterate_key_primitives(ctx, ctx->build_configuration_contents,
                                ctx->build_configuration_length, &offset,
                                hosts[y], 
                                cd.project_components[x], NULL,
                                type, component, key, NULL))
   {
    if((value = lookup_key(ctx, ctx->build_configuration_contents,
                           ctx->build_configuration_length,
                           type, component, key)))
    {
     gtk_tree_store_append(i->build_configuration_store, &child, &iter);
     gtk_tree_store_set(i->build_configuration_store, &child, 
                       2, key, 3, value, -1); 
     free(value);
    }
   }
  }
 }

 return 0;
}

static GtkWidget *create_project_configuration_tree(struct gtk_interface_context *i)
{
 GtkCellRenderer *renderer;
 GtkTreeModel *model = (GtkTreeModel *) i->project_configuration_store;
 GtkWidget *view;

 view = gtk_tree_view_new();

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Component Type", renderer,
                                             "text", 0, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Component Name", renderer,
                                             "text", 1, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Key", renderer,
                                             "text", 2, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Value", renderer,
                                             "text", 3, NULL);

 gtk_tree_view_set_model((GtkTreeView *) view, model);

 g_object_unref(model);
 return view;
}

static GtkWidget *create_build_configuration_tree(struct gtk_interface_context *i)
{
 GtkCellRenderer *renderer;
 GtkTreeModel *model = (GtkTreeModel *) i->build_configuration_store;
 GtkWidget *view;

 view = gtk_tree_view_new();

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Host", renderer,
                                             "text", 0, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Component", renderer,
                                             "text", 1, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Key", renderer,
                                             "text", 2, NULL);

 renderer = gtk_cell_renderer_text_new();
 gtk_tree_view_insert_column_with_attributes((GtkTreeView *) view, -1,  
                                             "Value", renderer,
                                             "text", 3, NULL);
 gtk_tree_view_set_model((GtkTreeView *) view, model);

 g_object_unref(model);
 return view;
}

void run_make(gpointer callback_data, guint callback_action, GtkWidget  *widget)
{
 struct gtk_interface_context *i;
 char temp[2048];
 FILE *f;

 snprintf(temp, 2048, "./bca-gtk-temp");
 if((f = fopen(temp, "w")) == NULL)
 {
  fprintf(stderr, "BCA: fopen(%s) failed\n", temp);
  return;
 }

 fprintf(f, "#!/bin/bash\n");
 fprintf(f, "make -f Makefile.bca\n");
 fprintf(f, "read -p \"Press [Enter] to close this terminal...\"\n");
 fclose(f);
 chmod(temp, S_IRUSR | S_IXUSR);

 i = (struct gtk_interface_context *) callback_data;

 snprintf(temp, 2048, "%s ./bca-gtk-temp &", i->execute_in_terminal_command);
 system(temp);

}

void edit_preferences(gpointer callback_data, guint callback_action, GtkWidget  *widget)
{
 struct gtk_interface_context *i;
 GtkWidget *dialog, *content_area, *vbox, *label, *entry;

 i = (struct gtk_interface_context *) callback_data;

 dialog = gtk_dialog_new_with_buttons("Build Configuration Adjust Gtk+ Interface Preferences",
                                      (GtkWindow *) i->window,
                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                      "OK", "Cancel", NULL);

 content_area = gtk_dialog_get_content_area((GtkDialog *) dialog);

 vbox = gtk_vbox_new(FALSE, 3);
 gtk_container_add((GtkContainer *) content_area, vbox);

 label = gtk_label_new("Command to run commands in prerred terminal emulator:");
 gtk_box_pack_start((GtkBox *) vbox, label, FALSE, FALSE, 1);


 entry = gtk_entry_new();
 gtk_entry_set_text((GtkEntry *) entry, i->execute_in_terminal_command);
 gtk_box_pack_start((GtkBox *) vbox, entry, FALSE, FALSE, 1);

 gtk_widget_show_all(content_area);

 gtk_dialog_run((GtkDialog *) dialog);

 if(i->execute_in_terminal_command != NULL)
  free(i->execute_in_terminal_command);

 i->execute_in_terminal_command = strdup(gtk_entry_get_text((GtkEntry *) entry));

 gtk_widget_destroy(dialog);

 save_preferences(i);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_Actions",            NULL,         NULL,             0, "<Branch>" },
  { "/_Actions/_Make",      "<control>M", run_make,         0, "<Item>" },
  { "/_Actions/_Exit",      "<control>Q", gtk_main_quit,    0, "<StockItem>", GTK_STOCK_QUIT },
  { "/_Edit",               NULL,         NULL,             0, "<Branch>" },
  { "/_Edit/_Preferences",  NULL,         edit_preferences, 0, "<Item>", },
  { "/_Help",               NULL,         NULL,             0, "<LastBranch>" },
  { "/_Help/About",         NULL,         NULL,             0, "<Item>", GTK_STOCK_ABOUT},
};
static gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

static GtkWidget *build_menu(struct gtk_interface_context *i)
{
 GtkItemFactory *item_factory;
 GtkAccelGroup *accel_group;

 accel_group = gtk_accel_group_new ();

 item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", accel_group);
 gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, (gpointer) i);
 gtk_window_add_accel_group (GTK_WINDOW(i->window), accel_group);

 return gtk_item_factory_get_widget (item_factory, "<main>");
}

int load_preferences(struct gtk_interface_context *i)
{
 char *f, *home, filename[2048];
 int f_length, end, start, index, line_length, equals_pos, allocation_size, handled;

 if((home = getenv("HOME")) == NULL)
  fprintf(stderr, "BCA: $HOME not set\n");

 if(i->execute_in_terminal_command != NULL)
  free(i->execute_in_terminal_command);

 if(home == NULL)
  return 0;

 snprintf(filename, 2048, "%s/.bca/gtk-interface-preferences", home);

 if((f = read_file(filename, &f_length, 0)) == NULL)
 {
  i->execute_in_terminal_command = strdup("xterm -e");
  return 0;
 }

 end = -1;
 while(find_line(f, f_length, &start, &end, &line_length))
 {
  index = start;
  equals_pos = -1;
  
  while(index < index + line_length)
  {
   if(f[index] == '=')
   {
    equals_pos = index;
    break;
   }
   index++;
  }

  if(equals_pos < -1)
  {
   f[start + line_length] = 0;
   fprintf(stderr, "BCA: Bad line in ~/.bca/gtk-interface-preferences: \"%s\"\n", f + start);
   return 1;
  }

  handled = 0;

  f[equals_pos] = 0;
  if(strcmp(f + start, "terminal-command") == 0)
  {
   allocation_size = start + line_length - equals_pos;
   if((i->execute_in_terminal_command = (char *) malloc(allocation_size)) == NULL)
   {
    fprintf(stderr, "BCA: malloc(%d) failed\n", allocation_size);
    return 1;
   }

   memcpy(i->execute_in_terminal_command, f + equals_pos + 1, allocation_size - 1);
   i->execute_in_terminal_command[allocation_size] = 0;

   handled = 1;
  }

  if(handled == 0)
  {
   fprintf(stderr, "unknown key in ~/.bca/gtk-interface-preferences: \"%s\"\n", f + start);
   return 1;
  }

 }

 free(f);
 return 0;
}

int save_preferences(struct gtk_interface_context *i)
{
 DIR *d;
 FILE *f;
 char *home, filename[2048];

 if((home = getenv("HOME")) == NULL)
 {
  fprintf(stderr, "BCA: $HOME not set\n");
  return 1;
 }
 
 snprintf(filename, 2048, "%s/.bca", home);
 if((d = opendir(filename)) == NULL)
 {
  if(mkdir(filename, S_IRWXU | S_IRWXG))
  {
   snprintf(filename, 2048, "%s/.bca", home);
   fprintf(stderr, "BCA: mkdir(%s) failed\n", filename);
   return 1;
  }
 }
 closedir(d); 

 snprintf(filename, 2048, "%s/.bca/gtk-interface-preferences", home);
 if((f = fopen(filename, "w")) == NULL)
 {
  fprintf(stderr, "BCA: fopen(~/.bca/gtk-interface-preferences, w) failed\n");
  return 1;
 }

 fprintf(f, "terminal-command=%s\n", i->execute_in_terminal_command);

 fclose(f);

 return 0;
}


int gtk_interface(struct bca_context *ctx)
{
 GtkWidget *view, *notebook, *vbox, *label, *menu, *scrolled_window;
 struct gtk_interface_context *i;

 if((i = (struct gtk_interface_context *)
         malloc(sizeof(struct gtk_interface_context))) == NULL)
 {
  fprintf(stderr, "malloc() failed\n");
  return 1;
 }

 memset(i, 0, sizeof(struct gtk_interface_context));
 i->ctx = ctx;

 if(load_preferences(i))
 {
  return 1;
 }

 if((ctx->project_configuration_contents = 
    read_file("./buildconfiguration/projectconfiguration", 
    &(ctx->project_configuration_length), 0)) == NULL)
 {
  return 1;
 }

 if((ctx->build_configuration_contents = 
     read_file("./buildconfiguration/buildconfiguration", 
     &(ctx->build_configuration_length), 0)) == NULL)
 {
  fprintf(stderr, "BCA: could not read ./buildconfiguration/buidconfiguration\n");
  return 1;
 }


 i->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 g_signal_connect(i->window, "delete_event", gtk_main_quit, (void *) ctx); 
 gtk_window_set_title((GtkWindow *) i->window, "Build Configuration Adjust");

 vbox = gtk_vbox_new(FALSE, 3);
 gtk_container_add((GtkContainer *) i->window, vbox);

 menu = build_menu(i);
 gtk_box_pack_start((GtkBox *) vbox, menu, FALSE, FALSE, 1);

 notebook = gtk_notebook_new();

 gtk_box_pack_start((GtkBox *) vbox, notebook, TRUE, TRUE, 1);

 i->project_configuration_store = gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING,
                                                     G_TYPE_STRING, G_TYPE_STRING);

 i->build_configuration_store = gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING);

 view = create_project_configuration_tree(i);

 scrolled_window = gtk_scrolled_window_new(NULL, NULL);
 gtk_scrolled_window_add_with_viewport((GtkScrolledWindow *) scrolled_window, view); 

 label = gtk_label_new("Project Configuration");

 gtk_notebook_append_page((GtkNotebook *) notebook, scrolled_window, label);

 label = gtk_label_new("Build Configuration");

 view = create_build_configuration_tree(i);

 scrolled_window = gtk_scrolled_window_new(NULL, NULL);
 gtk_scrolled_window_add_with_viewport((GtkScrolledWindow *) scrolled_window, view); 

 gtk_notebook_append_page((GtkNotebook *) notebook, scrolled_window, label);

 gtk_widget_show_all(i->window);

 reload(i);

 gtk_main();

 return 0;
}

#endif

