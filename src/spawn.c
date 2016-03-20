/* GPLv3

    Build Configuration Adjust, is a source configuration and Makefile
    generation tool.
    Copyright © 2015,2016 C. Thomas Stover.
    All rights reserved. See
    https://github.com/analogshadow/Build-Configuration-Adjust for more
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


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

#include "prototypes.h"

void *SIGNAL_PIPE_WRITE_CONTEXT = NULL;

struct spawn_context
{
 pid_t child_pid;
 int redirected_stdout[2], redirected_stderr[2], signal_pipe[2];

 void *data;

 int (*handle_stdout) (struct spawn_context *, void *data, char *buffer, int n_bytes);
 int (*handle_stderr) (struct spawn_context *, void *data, char *buffer, int n_bytes);
 int (*handle_exit) (struct spawn_context *, void *data, int condition, int exit_code);

 char **argv;
};

int cleanup(struct spawn_context *ctx, int condition)
{
 int code = 0;

 if(ctx != NULL)
 {

  if(ctx->redirected_stdout[0] != -1)
   close(ctx->redirected_stdout[0]);

  if(ctx->redirected_stdout[1] != -1)
   close(ctx->redirected_stdout[1]);

  if(ctx->redirected_stderr[0] != -1)
   close(ctx->redirected_stderr[0]);

  if(ctx->redirected_stderr[1] != -1)
   close(ctx->redirected_stderr[1]);

  if(ctx->signal_pipe[0] != -1)
   close(ctx->signal_pipe[0]);

  if(ctx->signal_pipe[1] != -1)
   close(ctx->signal_pipe[1]);

  if(ctx->child_pid != 0)
  {
   kill(SIGINT, ctx->child_pid);
   waitpid(-1, &code, WNOHANG);
   code = WEXITSTATUS(code);
  }

  if(ctx->handle_exit != NULL)
   code = ctx->handle_exit(ctx, ctx->data, condition, code);

  if(ctx->argv != NULL)
   free(ctx->argv);

  free(ctx);
 }

 return code;
}

void signal_handler_callback(int signum)
{
 if(SIGNAL_PIPE_WRITE_CONTEXT == NULL)
  return;

 struct spawn_context *ctx = (struct spawn_context *) SIGNAL_PIPE_WRITE_CONTEXT;

 if(signum == SIGCHLD)
  write(ctx->signal_pipe[1], "1", 1);
}

int spawn(const char *path, int argc, char **argv, void *data,
          int (*handle_stdout) (struct spawn_context *, void *data, char *buffer, int n_bytes),
          int (*handle_stderr) (struct spawn_context *, void *data, char *buffer, int n_bytes),
          int (*handle_exit) (struct spawn_context *, void *data, int condition, int exit_code))
{
 fd_set read_set;
 struct spawn_context *ctx;
 unsigned int allocation_size;
 char read_buffer[4096];
 int bytes_read, code, i, keep_going, max_fd;

 allocation_size = sizeof(struct spawn_context);
 if((ctx = (struct spawn_context *) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "malloc(%u) failed, %s\n",
          allocation_size, strerror(errno));
  return 1;
 }

 ctx->child_pid = 0;
 ctx->redirected_stdout[0] = -1;
 ctx->redirected_stdout[1] = -1;
 ctx->redirected_stderr[0] = -1;
 ctx->redirected_stderr[1] = -1;
 ctx->signal_pipe[0] = -1;
 ctx->signal_pipe[1] = -1;
 ctx->data = data;
 ctx->handle_stdout = handle_stdout;
 ctx->handle_stderr = handle_stderr;
 ctx->handle_exit = handle_exit;

 allocation_size = sizeof(char *) * (argc + 1);
 if((ctx->argv = (char **) malloc(allocation_size)) == NULL)
 {
  fprintf(stderr, "malloc(%u) failed: %s\n", allocation_size, strerror(errno));
  return cleanup(ctx, 1);
 }
 for(i=0; i<argc; i++)
 {
  ctx->argv[i] = argv[i];
 }
 ctx->argv[i] = NULL;

 if(pipe(ctx->redirected_stdout))
 {
  fprintf(stderr, "pipe2() failed: %s\n", strerror(errno));
  return cleanup(ctx, 1);
 }

 if(pipe(ctx->redirected_stderr))
 {
  fprintf(stderr, "pipe2() failed: %s\n", strerror(errno));
  return cleanup(ctx, 1);
 }

 if(pipe(ctx->signal_pipe))
 {
  fprintf(stderr, "pipe2() failed: %s\n", strerror(errno));
  return cleanup(ctx, 1);
 }

 SIGNAL_PIPE_WRITE_CONTEXT = ctx;
 if(SIG_ERR == signal(SIGCHLD, signal_handler_callback))
 {
  fprintf(stderr, "signal() failed: %s\n", strerror(errno));
  return cleanup(ctx, 1);
 }

 if((ctx->child_pid = fork()) == 0)
 {
  /* in child */
  close(ctx->redirected_stdout[0]);
  ctx->redirected_stdout[0] = -1;

  close(ctx->redirected_stderr[0]);
  ctx->redirected_stderr[0] = -1;

  close(ctx->signal_pipe[0]);
  ctx->signal_pipe[0] = -1;

  close(ctx->signal_pipe[1]);
  ctx->signal_pipe[1] = -1;

  if(dup2(ctx->redirected_stdout[1], 1) == -1)
  {
   fprintf(stderr, "dup2() failed, %s\n", strerror(errno));
   return cleanup(ctx, 1);
  }

  if(dup2(ctx->redirected_stderr[1], 2) == -1)
  {
   fprintf(stderr, "dup2() failed, %s\n", strerror(errno));
   return cleanup(ctx, 1);
  }

  if(execvp(path, argv) == -1)
  {
   fprintf(stderr, "execv() failed, %s\n", strerror(errno));
   return cleanup(ctx, 1);
  }

 } else {
  /* in parrent */
  close(ctx->redirected_stdout[1]);
  ctx->redirected_stdout[1] = -1;

  close(ctx->redirected_stderr[1]);
  ctx->redirected_stderr[1] = -1;

  max_fd = 0;

  if(ctx->redirected_stdout[0] > max_fd)
   max_fd = ctx->redirected_stdout[0];

  if(ctx->redirected_stderr[0] > max_fd)
   max_fd = ctx->redirected_stderr[0];

  if(ctx->signal_pipe[0] > max_fd)
   max_fd = ctx->signal_pipe[0];

  keep_going = 1;
  while(keep_going)
  {
   errno = 0;
   FD_ZERO(&read_set);
   FD_SET(ctx->redirected_stdout[0], &read_set);
   FD_SET(ctx->redirected_stderr[0], &read_set);
   FD_SET(ctx->signal_pipe[0], &read_set);

   if((code = select(max_fd, &read_set, NULL, NULL, NULL)) == -1)
   {
    if(errno != EINTR)
    {
     fprintf(stderr, "select() failed, %s\n", strerror(errno));
     return cleanup(ctx, 1);
    }
    if(errno == EINTR)
     continue;
   }

   if(FD_ISSET(ctx->signal_pipe[0], &read_set))
   {
    if((bytes_read = read(ctx->signal_pipe[0], read_buffer, 4096)) < 0)
    {
     if(errno == EINTR)
      continue;
    }

    keep_going = 0;
   }

   if(FD_ISSET(ctx->redirected_stdout[0], &read_set))
   {
    if((bytes_read = read(ctx->redirected_stdout[0], read_buffer, 4096)) < 1)
    {
     if(errno == EINTR)
      continue;

     keep_going = 0;
    } else {
     read_buffer[bytes_read] = 0;

     if(ctx->handle_stdout != NULL)
      if(ctx->handle_stdout(ctx, ctx->data, read_buffer, bytes_read))
       return cleanup(ctx, 0);
    }
   }

   if(FD_ISSET(ctx->redirected_stderr[0], &read_set))
   {
    if((bytes_read = read(ctx->redirected_stderr[0], read_buffer, 4096)) < 1)
    {
     if(errno == EINTR)
      continue;

     keep_going = 0;
    } else {
     read_buffer[bytes_read] = 0;

     if(ctx->handle_stderr != NULL)
      if(ctx->handle_stderr(ctx, ctx->data, read_buffer, bytes_read))
       return cleanup(ctx, 0);
    }
   }

  }

  waitpid(-1, &code, WNOHANG);
  code = WEXITSTATUS(code);
  ctx->child_pid = 0;

  code = ctx->handle_exit(ctx, ctx->data, 0, code);
  ctx->handle_exit = NULL;
  return cleanup(ctx, code);
 }

 return cleanup(ctx, 0);
}
