#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include "buffer.h"

/* Initialize the fd_set objects, indicating which file handles we want to
 * listen on.
 */
void setup_fd_sets(fd_set* readfs, 
                   fd_set* writefs, 
                   fd_set* exceptfs, 
                   int     num_outfiles, 
             const int*    outfile) {
  int i;

  FD_ZERO(readfs);
  FD_ZERO(writefs);
  FD_ZERO(exceptfs);
  FD_SET(STDIN_FILENO, readfs);
  for (i = 0; i < num_outfiles; ++i) {
    FD_SET(outfile[i], writefs);
  }
}

/* Grab a chunk of input from stdin.
 */
int do_input(const fd_set* readfs) {
  int amount_read;

  if (FD_ISSET(STDIN_FILENO, readfs)) {
    amount_read = read(STDIN_FILENO, 
                       get_write_buffer(), 
                       get_available_to_write());
    if (amount_read < 0) {
      perror("dog");
      exit(1);
    }
    buffer_push(amount_read);

    return amount_read;
  } else {
    /* Return a non-zero value, otherwise we'll think we've reached EOF. */
    return 1;
  }
}

/* Write an appropriate chunk of data to each ready output file descriptor.
 */
void do_output(fd_set* writefs, int num_outfiles, const int* outfile) {
  int i;

  for (i = 0; i < num_outfiles; ++i) {
    if (FD_ISSET(outfile[i], writefs)) {
      int amount = get_available_to_read(i);

      if (amount > 0) {
        int w = write(outfile[i], get_read_buffer(i), amount);
        if (w < 0) {
          perror("dog");
          exit(1);
        }
        printf("Trying to write %d bytes to output file #%d\n", amount, i);
        buffer_pop(w, i);
        printf("Wrote           %d bytes to output file #%d\n", w, i);
      }
    }
  }
}

/* Test which inputs and outputs are ready, and act accordingly.
 */
int do_io(int highest_filedes, int num_outfiles, const int* outfile) {
  fd_set readfs, writefs, exceptfs;
  int retval;

  setup_fd_sets(&readfs, &writefs, &exceptfs, num_outfiles, outfile);
  retval = select(highest_filedes+1, &readfs, &writefs, &exceptfs, NULL);
  if (retval < 0) {
    perror("dog");
    return retval;
  }
  
  retval = do_input(&readfs);
  //printf("do_input() returned %d\n", retval);
  do_output(&writefs, num_outfiles, outfile);

  return retval;
}

/* Opens the file for output.
 */
int setup_output_file_descriptor(const char* filename) {
  int des = open(filename, 
                 O_NONBLOCK|O_ASYNC|O_WRONLY|O_CREAT|O_TRUNC, 
                 S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (des <= 0) {
    perror(filename);
    exit(des);
  }

  return des;
}

/* Set up the output file descriptors and open the files.
 */
int* setup_output_file_descriptors(int num_outfiles, 
                                   const const char** argv, 
                                   int* highest_filedes) {
  int i;
  int* outfile = calloc(num_outfiles, sizeof(int));

  for (i = 0; i < num_outfiles; ++i) {
    int des = setup_output_file_descriptor(argv[i+1]);
    if (des > *highest_filedes) 
      *highest_filedes = des;
    outfile[i] = des;
  }

  return outfile;
}

/* Free resources associated with file output.
 */
int teardown_output_file_descriptors(int num_outfiles, int* outfile) {
  int i;
  for (i = 0; i < num_outfiles; ++i) {
    close(outfile[i]);
  }
  free(outfile);
}

/* Black magic with fcntl()
 */
int set_stdin_nonblocking() {
  int old_settings;
  int new_settings;

  old_settings = fcntl(STDIN_FILENO, F_GETFL, 0);
  new_settings = old_settings|O_ASYNC|O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, new_settings);

  return old_settings;
}

int main(int argc, const const char** argv) {
  int old_stdin;
  int moar;
  int highest_filedes;
  int num_outfiles;
  int* outfile;

  /* setup */
  num_outfiles = argc - 1;
  highest_filedes = STDIN_FILENO;
  outfile = setup_output_file_descriptors(num_outfiles, argv, &highest_filedes);
  buffer_init(num_outfiles);
  old_stdin = set_stdin_nonblocking();

  /* I/O */
  moar = 1;
  while (moar) {
    moar = do_io(highest_filedes, num_outfiles, outfile);
  }

  /* teardown */
  fcntl(STDIN_FILENO, F_SETFL, old_stdin);
  buffer_free();
  teardown_output_file_descriptors(num_outfiles, outfile);

  return 0;
}
