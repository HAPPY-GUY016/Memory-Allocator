#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define BLOCK_META_SIZE sizeof(block_meta)
void *global_base = NULL;

typedef struct block_meta
{
    size_t size;
    struct block_meta *next;
    int free;

} block_meta;


block_meta *find_free_block(block_meta **last, size_t size)
{
    block_meta *current_b = global_base;
    while (current_b && !(current_b->free && current_b->size >= size))
    {
        *last = current_b;
        current_b = current_b->next;
    }
    return current_b;
}

block_meta *request_space(block_meta *last, size_t size)
{
    block_meta *block;
    block = sbrk(0);
    void *req = sbrk(size + BLOCK_META_SIZE);
    assert((void *)block == req);
    if (req == (void *)-1)
    {
        return NULL; // sbrk failed.
    }

    if (last)
    {
        last->next = block;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
}

void *malloc_imp(size_t size)
{
    block_meta *block;
    if (size <= 0)
    {
        return NULL;
    }

    if (!global_base)
    { // first call
        block = request_space(NULL, size);
        if (!block)
        {
            return NULL;
        }
        global_base = block;
    }
    else
    {
        block_meta *last = global_base;
        block = find_free_block(&last, size);
        if (!block)
        { // failed to find free block
            block = request_space(last, size);
            if (!block)
            {
                return NULL;
            }
        }
        else
        {
            printf("<string has been found freed>\n");
            block->free = 0;
        }
    }
    return (block + 1);
}

block_meta *get_block(void *ptr)
{
    return (block_meta *)ptr - 1;
}

void free_imp(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    block_meta *block_ptr = get_block(ptr);
    // printf("value of block_ptr->free %d\n",block_ptr->free);
    assert(block_ptr->free == 0);
    block_ptr->free = 1;
    printf("<marking string as freed>\n");
}

int main()
{

    /*
    Few tests
    void* current_brk = sbrk(0);
    void* new_brk = sbrk(4);
    printf("Current top of heap: %p\n", current_brk);
    printf("New top of heap: %p\n", new_brk);
    */

    // void* current_brk = sbrk(0);
    // printf("Current top of heap: %p\n", current_brk);
    // char *str=(char *)malloc_imp(7);
    // printf("%p\n",str);
    // scanf("%s",str);
    // printf("%s\n",str);
    // printf("%p\n",str+3);

    char *str = (char *)malloc_imp(5);
    strcpy(str, "abcd");
    printf("%s\n", str);

    char *str2 = (char *)malloc_imp(4);
    strcpy(str2, "aaa");
    printf("%s\n", str2);

    free_imp(str);

    char *str3 = (char *)malloc_imp(3);
    strcpy(str3, "hi");
    printf("%s\n", str3);

    return 0;
}
