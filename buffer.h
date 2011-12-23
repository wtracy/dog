/* Set up the buffer, given a number of read handles to initialize.
 */
void buffer_init(int);

/* Tear down the buffer.
 */
void buffer_free();

/* Retrieve the amount of space, in bytes, allocated for the buffer that aren't
 * currently in use. Available mostly for diagnostic purpposes.
 */
int get_space_free();

/* Retrieve the amount of space, in bytes, allocated for the buffer that
 * is marked as containing data. Available mostly for diagnostic purposes.
 */
int get_space_used();

/* Get the amount of contiguous memory available for the next write operation.
 */
int get_available_to_write();

/* Get the amount of contiguous memory available to read, for the given read
 * handle.
 */
int get_available_to_read(int);

/* Grab a chunk of memory to write to. The amount of memory that can be written
 * to is returned by get_available_to_write().
 */
char* get_write_buffer();

/* Grab a chunk of memory that can be read from, for the given handle. The
 * amount of memory that can be read is returned by get_available_to_read().
 */
char* get_read_buffer(int);

/* Advances the head of the buffer by a given amount, indicating that the
 * indicated amount of memory now has data in it.
 */
char buffer_push(int);

/* Advance the given tail of the buffer by a given amount, indicating that the
 * indicated amount of memory has been read and may now be freed.
 */
char buffer_pop(int, int);



