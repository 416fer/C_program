#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>


typedef struct Arena Arena;

struct Arena {
        unsigned char *buf;
        size_t         buf_len;
        size_t         prev_offset;
        size_t         curr_offset; 
};

//Aligning memory(unintpter_t = unsigned long int)
bool is_power_of_two(uintptr_t x) {
        return (x & (x - 1)) == 0;
}

void arena_init(Arena* a, void *backing_buffer, size_t backing_buffer_length) {
    a->buf = (unsigned char *)backing_buffer;
    a->buf_len = backing_buffer_length;
    a->prev_offset = 0;
    a->curr_offset = 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;
    
    // check aligning memory
    assert(is_power_of_two(align));
    
    p = ptr;
    a = (uintptr_t)align;
    // Same as p & a but faster as 'a' is a power of two
    modulo = p & (a-1);

    if (modulo != 0) {
            p += a - modulo;
    }
    return p;
}

void *alloc_arena_align(Arena *a, size_t size, size_t align) {
    uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
    uintptr_t offset = align_forward(curr_ptr, align);
    offset -= (uintptr_t)a->buf;

    if (offset + size <= a->buf_len) {
            void *ptr = &a->buf[offset];
            a->prev_offset = offset;
            a->curr_offset = offset + size;

            memset(ptr, 0, size);
            return ptr;
    }
    return NULL;
}

void *arena_resize_alloc(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align) {
    unsigned char *old_mem = (unsigned char *)old_memory;

    assert(is_power_of_two(align));

    if (old_mem == NULL || old_size == 0) {
            return alloc_arena_align(a, new_size, align);
    } else if (a->buf <= old_mem && old_mem < a->buf + a->buf_len) {
            if (a->buf + a->prev_offset == old_mem) {
                    a->curr_offset = a->prev_offset + new_size;
                    if (new_size > old_size) {
                            memset(&a->buf[a->curr_offset], 0, new_size-old_size);
                    }
                    return old_memory;
            } else {
                    void *new_memory = alloc_arena_align(a, new_size, align);
                    size_t copy_size = old_size < new_size ? old_size : new_size;
                    // copy across old memory to new memory
                    memmove(new_memory, old_memory, copy_size);
                    return new_memory;
            }
    } else {
            assert(0 && "Memory is out of bounds of the buffer in the arena.");
            return NULL;
    }
}



#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

void *arena_alloc(Arena *a, size_t size) {
    return alloc_arena_align(a, size, DEFAULT_ALIGNMENT);
}

void *arena_resize(Arena* a, void *old_memory, size_t old_size, size_t new_size) {
        return arena_resize_alloc(a, old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}


void arena_free(Arena* a, void* ptr) {

}


void arena_free_all(Arena* a) {
        a->curr_offset = 0;
        a->prev_offset = 0;
}



int main() {
    int i;

	unsigned char backing_buffer[256];
	Arena a = {0};
	arena_init(&a, backing_buffer, 256);

	for (i = 0; i < 10; i++) {
		int *x;
		float *f;
		char *str;

		// Reset all arena offsets for each loop
		arena_free_all(&a);

		x = (int *)arena_alloc(&a, sizeof(int));
		f = (float *)arena_alloc(&a, sizeof(float));
		str = arena_alloc(&a, 10);

		*x = 123;
		*f = 987;
		memmove(str, "Hellope", 7);

		printf("%p: %d\n", x, *x);
		printf("%p: %f\n", f, *f);
		printf("%p: %s\n", str, str);

        str = arena_resize(&a, str, 10, 16);
		memmove(str+7, " world!", 7);
		printf("%p: %s\n", str, str);
	}

	arena_free_all(&a);

	return 0;
}



