#include <stdlib.h>
#include <limits.h>

#include "buffer.h"

#define BUFSIZE 524288

char buffer[BUFSIZE];

/* The next byte to be written to in
 * the buffer.
 */
int head;
/* The next byte to be read from the
 * buffer.
 */
int* tail;
/* The number of tails on the buffer.
 */
int num_tails;

void buffer_init(int new_tails) {
  head = 0;
  num_tails = new_tails;
  tail = calloc(num_tails, sizeof(int));
}

void buffer_free() {
  free(tail);
}

/* Utility functions */

/* Tests whether the data in the buffer is contiguous, or whether it is wrapped
 * past the end of the buffer and back to the front.
 */
char is_wrapped(int a_tail) {
  return (head < a_tail);
}

char is_index_wrapped(int t) {
  char result = (is_wrapped(tail[t]));
  return result;
}

int max(int a, int b) {
  return (a > b) ? a : b;
}

int min(int a, int b) {
  return (a < b) ? a : b;
}
/* End utility functions */

int get_longest_tail() {
  int longest_wrapped = INT_MAX;
  int longest_unwrapped = INT_MAX;
  int i;

  for (i = 0; i < num_tails; ++i) {
    if (is_wrapped(tail[i])) {
      longest_wrapped = min(tail[i], longest_wrapped);
    } else {
      longest_unwrapped = min(tail[i], longest_unwrapped);
    }
  }

  if (longest_wrapped < INT_MAX)
    return longest_wrapped;
  else
    return longest_unwrapped;
}

int get_space_free() {
  int t = get_longest_tail();
  if (!is_wrapped(t)) {
    return BUFSIZE - head + t - 1;
  } else {
    return t - head - 1;
  }
}

int get_space_used() {
  int t = get_longest_tail();
  char wrapped = is_wrapped(t);
  if (wrapped)
    return BUFSIZE - t + head;
  else
    return head - t;
}

int get_available_to_write() {
  int t = get_longest_tail();
  if (is_wrapped(t)) {
    return t - head - 1;
  } else if (t > 0) {
    return BUFSIZE - head;
  } else {
    return BUFSIZE - head - 1;
  }
}

int get_available_to_read(int t) {
  if (is_wrapped(tail[t])) {
    return BUFSIZE - tail[t];
  } else {
    return head - tail[t];
  }
}

char* get_write_buffer() {
  return buffer + head;
}

char* get_read_buffer(int t) {
  return buffer + tail[t];
}

char buffer_push(int amount) {
  char wrapped = is_wrapped(get_longest_tail());

  if (amount > get_available_to_write())
    return 0;
  head += amount;
  if (head >= BUFSIZE) {
    /* Buffer is wrapping around 
     */
    head -= BUFSIZE;
  }
  return 1;
}

char buffer_pop(int amount, int t) {
  if (amount > get_available_to_read(t))
    return 0;
  tail[t] += amount;
  tail[t] %= BUFSIZE;
  return 1;
}



/* Functions for unit testing 
 * purposes. 
 */
char head_at_start_position() {
  return head == 0;
}

char is_setup() {
  int i;
  if (!head_at_start_position()) 
    return 0;
  for (i = 0; i < num_tails; ++i) {
    if (tail[i] != 0) 
      return 0;
  }
  
  return 1;
}

