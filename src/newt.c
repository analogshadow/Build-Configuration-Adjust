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

#include "config.h"
#ifndef WITHOUT_LIBNEWT

#ifndef IN_SINGLE_FILE_DISTRIBUTION
#include "prototypes.h"
#endif

#include <newt.h>
#include <stdlib.h>

#define NEWT_NEW_PROJECT  101
#define NEWT_EDIT_PROJECT 102
#define NEWT_CONFIGURE    103
#define NEWT_TASKS        104
#define NEWT_ABOUT        105
#define NEWT_GMAKEFILE    106
#define NEWT_GRAPHVIZ     107


/*
struct newtExitStruct {
    enum { NEWT_EXIT_HOTKEY, NEWT_EXIT_COMPONENT, NEWT_EXIT_FDREADY,
	   NEWT_EXIT_TIMER } reason;
    union {
	int watch;
	int key;
	newtComponent co;
    } u;
} ;

*/

int newt_interface_main_menu(struct bca_context *ctx)
{
 newtComponent form, b1, b2, b3, b4, b5, b6;
 struct newtExitStruct results; 
 int code = 0;

 newtCls();
 newtDrawRootText(2, 1, "Project Name");
 newtPushHelpLine("Configure build environment to compile this software");

 newtOpenWindow(3, 5, 36, 6, "Main");
 b1 = newtCompactButton(0, 0, "New Project");
 b2 = newtCompactButton(0, 1, "Manipulate Project");
 b3 = newtCompactButton(0, 2, "Congiure Build Environment");
 b4 = newtCompactButton(0, 3, "Tasks");
 b5 = newtCompactButton(0, 4, "About Build Configuration Adjust");
 b6 = newtCompactButton(0, 5, "Exit");
 form = newtForm(NULL, NULL, 0);
 newtFormAddComponents(form, b1, b2, b3, b4, b5, b6, NULL);

 newtFormRun(form, &results);

 if(results.reason == NEWT_EXIT_COMPONENT)
 {
  if(results.u.co == b1)
   code = NEWT_NEW_PROJECT;

  if(results.u.co == b2)
   code = NEWT_EDIT_PROJECT;

  if(results.u.co == b3)
   code = NEWT_CONFIGURE;

  if(results.u.co == b4)
   code = NEWT_TASKS;

  if(results.u.co == b5)
   code = NEWT_ABOUT;

 }

 newtFormDestroy(form);
 newtPopHelpLine();
 return code;
}

int newt_interface_tasks(struct bca_context *ctx)
{
 newtComponent form, b1, b2, b3;
 struct newtExitStruct results; 
 int code = 0;

 newtOpenWindow(5, 6, 36, 3, "Tasks");
 b1 = newtCompactButton(0, 0, "Generate Makefile for GNU Make");
 b2 = newtCompactButton(0, 1, "Generate Graphviz build plot");
 b3 = newtCompactButton(0, 2, "Exit");
 form = newtForm(NULL, NULL, 0);
 newtFormAddComponents(form, b1, b2, b3, NULL);

 newtFormRun(form, &results);

 if(results.reason == NEWT_EXIT_COMPONENT)
 {
  if(results.u.co == b1)
   code = NEWT_GMAKEFILE;

  if(results.u.co == b2)
   code = NEWT_GRAPHVIZ;
 }

 newtFormDestroy(form);
 return code;
}

int newt_interface(struct bca_context *ctx)
{
 int code;
 newtInit();

 while((code = newt_interface_main_menu(ctx)) != 0)
 {
  switch(code)
  {
   case NEWT_NEW_PROJECT:
        break;

   case NEWT_EDIT_PROJECT:
        break;

   case NEWT_CONFIGURE:
        break;

   case NEWT_TASKS:
        while((code = newt_interface_tasks(ctx)) != 0)
        {
         switch(code)
         {
          case NEWT_GMAKEFILE:
               break;

          case NEWT_GRAPHVIZ:
               break;
         }
        }
        break;

   case NEWT_ABOUT:
        break;
  }
 }

 newtFinished();

 return 0;
}

#endif

