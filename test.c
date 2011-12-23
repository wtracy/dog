#include <stdio.h>
#include <glib.h>

#include "buffer.h"


void test_space_in_new_buffer() {
  buffer_init(1);
  g_assert(is_setup());
  g_assert_cmpint(0, ==, get_longest_tail());
  g_assert_cmpint(4095, ==, get_space_free());
  g_assert_cmpint(0, ==, get_space_used());
  g_assert_cmpint(get_space_free(), >=, get_available_to_write());
  g_assert_cmpint(get_space_used(), >=, get_available_to_read(0));
  buffer_free();
}

void test_space_after_push() {
  buffer_init(1);
  g_assert(buffer_push(95));
  g_assert_cmpint(4000, ==, get_space_free());
  g_assert_cmpint(95, ==, get_space_used());
  g_assert_cmpint(get_space_free(), >=, get_available_to_write());
  g_assert_cmpint(get_space_used(), >=, get_available_to_read(0));
  buffer_free();
}

void test_space_after_push_pop() {
  buffer_init(1);
  g_assert(buffer_push(96));
  g_assert(buffer_pop(96, 0));
  g_assert_cmpint(4095, ==, get_space_free());
  g_assert_cmpint(0, ==, get_space_used());
  g_assert_cmpint(get_space_free(), >=, get_available_to_write());
  g_assert_cmpint(get_space_used(), >=, get_available_to_read(0));
  buffer_free();
}

void test_overflow() {
  buffer_init(1);
  g_assert(!buffer_push(4096));
  buffer_free();
}

void test_space_after_fill_buffer() {
  buffer_init(1);
  g_assert(buffer_push(4095));
  g_assert_cmpint(0, ==, get_space_free());
  g_assert_cmpint(4095, ==, get_space_used());
  g_assert_cmpint(get_space_free(), >=, get_available_to_write());
  g_assert_cmpint(get_space_used(), >=, get_available_to_read(0));
  buffer_free();
}

void test_space_after_wrapped() {
  buffer_init(1);
  g_assert(buffer_push(100));
  g_assert(buffer_pop(100, 0));
  g_assert_cmpint(0, ==, get_available_to_read(0));
  g_assert(!is_wrapped(get_longest_tail()));
  g_assert(buffer_push(3996));
  g_assert(head_at_start_position());
  g_assert(is_wrapped(get_longest_tail()));
  g_assert(buffer_push(99));
  g_assert(is_wrapped(get_longest_tail()));
  g_assert_cmpint(0, ==, get_space_free());
  g_assert_cmpint(4095, ==, get_space_used());
  g_assert_cmpint(get_space_free(), >=, get_available_to_write());
  g_assert_cmpint(get_space_used(), >=, get_available_to_read(0));
  buffer_free();
}

void test_read_write_one_at_a_time() {
  int i;
  int* pointer;

  buffer_init(1);
  for (i = 0; i < 9000; ++i) {
    g_assert_cmpint(get_available_to_write(), >=, sizeof(int));
    pointer = (int*)get_write_buffer();
    *pointer = i;
    g_assert(buffer_push(sizeof(int)));
    g_assert_cmpint(get_available_to_read(0), >=, sizeof(int));
    pointer = (int*)get_read_buffer(0);
    g_assert_cmpint(*pointer, ==, i);
    g_assert(buffer_pop(sizeof(int), 0));
  }
  buffer_free();
}

void test_fill_drain() {
  int i;
  int* pointer;

  buffer_init(1);
  for (i = 0; i < 1023; ++i) {
    pointer = (int*)get_write_buffer();
    *pointer = i;
    g_assert(buffer_push(sizeof(int)));
  }
  for (i = 0; i < 1023; ++i) {
    pointer = (int*)get_read_buffer(0);
    g_assert_cmpint(*pointer, ==, i);
    g_assert(buffer_pop(sizeof(int), 0));
  }
  buffer_free();
}

void test_fill_drain_drain() {
  int i;
  int* pointer;

  buffer_init(2);
  for (i = 0; i < 1023; ++i) {
    pointer = (int*)get_write_buffer();
    *pointer = i;
    g_assert(buffer_push(sizeof(int)));
  }
  for (i = 0; i < 1023; ++i) {
    pointer = (int*)get_read_buffer(0);
    g_assert_cmpint(*pointer, ==, i);
    g_assert(buffer_pop(sizeof(int), 0));
  }
  for (i = 0; i < 1023; ++i) {
    pointer = (int*)get_read_buffer(1);
    g_assert_cmpint(*pointer, ==, i);
    g_assert(buffer_pop(sizeof(int), 1));
  }
  buffer_free();
}
  
void test_wrap_multi() {
  buffer_init(2);
  g_assert(buffer_push(4095));

  /*fprintf(stderr, "is_index_wrapped(0) = %d\n", is_index_wrapped(0));*/
  g_assert(!is_index_wrapped(0));
  g_assert(!is_index_wrapped(1));

  g_assert(buffer_pop(2048, 0));
  g_assert(buffer_pop(4095, 1));

  g_assert(!is_index_wrapped(0));
  g_assert(!is_index_wrapped(1));

  g_assert(buffer_push(1));
  g_assert(head_at_start_position());
  g_assert(buffer_push(1024));
  g_assert(buffer_pop(1, 1));
  g_assert(buffer_pop(100, 1));

  g_assert(is_index_wrapped(0));
  g_assert(!is_index_wrapped(1));
  
  g_assert(buffer_pop(2048, 0));
  g_assert(buffer_pop(120, 0));
 
  g_assert(!is_index_wrapped(0));
  g_assert(!is_index_wrapped(1));

  buffer_free();
}

int main (int   argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL); // initialize test program
  g_test_add_func ("/Dog/Space in Empty Buffer", test_space_in_new_buffer);
  g_test_add_func ("/Dog/Space after push", test_space_after_push);
  g_test_add_func ("/Dog/Space after push", test_space_after_push_pop);
  g_test_add_func ("/Dog/Space after Fill", test_space_after_fill_buffer);
  g_test_add_func ("/Dog/Overflow", test_overflow);
  g_test_add_func ("/Dog/Space after wrap", test_space_after_wrapped);
  g_test_add_func ("/Dog/Read and write one at a time", test_read_write_one_at_a_time);
  g_test_add_func ("/Dog/Test fill and drain", test_fill_drain);
  g_test_add_func ("/Dog/Test fill and drain and drain again", test_fill_drain_drain);
  g_test_add_func ("/Dog/Test fill and drain and drain again", test_wrap_multi);
  return g_test_run();
}
