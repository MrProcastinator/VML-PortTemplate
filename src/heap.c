/* Use this file for:
    1. Increading the size of your vita heap if you need more memory
    2. Use it to switch between C++ and C mode for CMake changing this file extension accordingly
*/

/* Minimal size to keep memory for Mono */
unsigned int _newlib_heap_size_user = 4 * 1024 * 1024;
/* Must increment from vita C runtime */
unsigned int sceLibcHeapSize = 300 * 1024 * 1024;

