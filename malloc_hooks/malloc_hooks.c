/* malloc_hooks.c revision 8, 2012-03-15 by Stephen Kell */
/* See below for license. */
/* */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* This file is a re-usable set of malloc hooks for glibc, intended to
 * enable easier creation of hooks. 

 **** NOTE: this file has been superseded by libmallochooks! ****
 **** See <http://github.com/stephenrkell/libmallochooks>.   ****
 
 * It is based on the example hooks in the GNU C library manual, which are
 * Copyright (c) Free Software Foundation and released under the GNU Free
 * Documentation License.
 *
 * It provides the user with a more abstract selection of hook primitives. 
 * These extensions were done by Stephen Kell <srk31@srcf.ucam.org>,
 * and are released into the public domain. I have no idea how to license
 * this file as a whole, but I invite anyone wishing to use it to appropriately
 * respect the FSF copyright.
 *
 * Currently provided:
 * pre_alloc
 * post_successful_alloc 
 * pre_nonnull_free
 * post_nonnull_free
 * pre_nonnull_nonzero_realloc
 * post_nonnull_nonzero_realloc
 *
 * Recommended usage of this file:
 * - #include it in a .c file;
 * - paste the signatures below into your C file, below the #include directives,
 *   and provide a body for each one;
 * - compile!
 * 
 * I left the printf statements in place from the example hooks, but commented
 * out, so it'll be easy to add your own debugging printouts if needed.
 *
 * It's fairly easy to link multiple sets of hooks by compiling and linking
 * this file multiple times. Each hook's __malloc_initialize_hook will look
 * for a weak symbol __malloc_previous_initialize_hook and call through that
 * if it's nonnull. You will need to do some symbol renaming though, probably
 * using objcopy, to correctly chain the initialization hooks. If anyone knows
 * a better way to do this, please let me know. 
 *
 * TODO:
 * add missing post-instrumentation
 * allow post-instrumentation modification of returned values
 * some kind of thread-safety perhaps...
 */

#include <stdio.h>
/* Prototypes for __malloc_hook, __free_hook */
#include <malloc.h>
#include <assert.h>
#include <errno.h>

/* Prototypes for our hooks.  */
static void my_init_hook (void);
static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);
static void *my_memalign_hook (size_t alignment, size_t size, const void *caller);
static void *my_realloc_hook(void *ptr, size_t size, const void *caller);

/* Local variables to hold the next-in-chain hooks. */
static void *(*old_malloc_hook) (size_t, const void *);
static void (*old_free_hook) (void*, const void *);
static void *(*old_memalign_hook) (size_t alignment, size_t size, const void *caller);
static void *(*old_realloc_hook)(void *ptr, size_t size, const void *caller);

/* Declare the variables that point to the active hooks. This isn't necessary
 * on glibc... adding it here as a precursor to supporting more platforms. */
extern void *(*__malloc_hook) (size_t, const void *) __attribute__((weak));
extern void (*__free_hook) (void*, const void *) __attribute__((weak));
extern void *(*__memalign_hook) (size_t alignment, size_t size, const void *caller) __attribute__((weak));
extern void *(*__realloc_hook)(void *ptr, size_t size, const void *caller) __attribute__((weak));
		
/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook) (void) = my_init_hook;
extern void __prev_malloc_initialize_hook (void) __attribute__((weak)); /* NOT a pointer to it */

/* High-level hook prototypes */
static void init_hook(void);
static void pre_alloc(size_t *p_size, const void *caller);
static void post_successful_alloc(void *begin, size_t modified_size, const void *caller);
static void pre_nonnull_free(void *ptr, size_t freed_usable_size);
static void post_nonnull_free(void *ptr);
static void pre_nonnull_nonzero_realloc(void *ptr, size_t size, const void *caller, void *__new);
static void post_nonnull_nonzero_realloc(void *ptr, 
	size_t modified_size, 
	size_t old_usable_size,
	const void *caller, void *__new);

static void
my_init_hook (void)
{
	if (__prev_malloc_initialize_hook) __prev_malloc_initialize_hook();
	/* save old hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_memalign_hook = __memalign_hook;
	old_realloc_hook = __realloc_hook;
	/* do our initialization */
	init_hook();
	/* install our hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__memalign_hook = my_memalign_hook;
	__realloc_hook = my_realloc_hook;
}

static void *
my_malloc_hook (size_t size, const void *caller)
{
	void *result;
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__memalign_hook = old_memalign_hook;
	__realloc_hook = old_realloc_hook;
	/* Call recursively */
	#ifdef TRACE_MALLOC_HOOKS
	printf ("calling malloc (%u)\n", (unsigned int) size);
	#endif
	size_t modified_size = size;
	pre_alloc(&modified_size, caller);
	result = malloc (modified_size);
	if (result) post_successful_alloc(result, modified_size, caller);
	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_memalign_hook = __memalign_hook;
	old_realloc_hook = __realloc_hook;
	/* printf might call malloc, so protect it too. */
	/* printf ("malloc (%u) returns %p (modified size: %zu)\n", 
	  (unsigned int) size, result, modified_size); */
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__memalign_hook = my_memalign_hook;
	__realloc_hook = my_realloc_hook;
	return result;
}

static void
my_free_hook (void *ptr, const void *caller)
{
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__memalign_hook = old_memalign_hook;
	__realloc_hook = old_realloc_hook;
	/* Call recursively */
	#ifdef TRACE_MALLOC_HOOKS
	if (ptr != NULL) printf ("freeing pointer %p\n", ptr);
	#endif 
	if (ptr != NULL) pre_nonnull_free(ptr, malloc_usable_size(ptr));
	free (ptr);
	if (ptr != NULL) post_nonnull_free(ptr);
	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_memalign_hook = __memalign_hook;
	old_realloc_hook = __realloc_hook;
	/* printf might call free, so protect it too. */
	#ifdef TRACE_MALLOC_HOOKS
	printf ("freed pointer %p\n", ptr);
	#endif
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__memalign_hook = my_memalign_hook;
	__realloc_hook = my_realloc_hook;
}

static void *
my_memalign_hook (size_t alignment, size_t size, const void *caller)
{
	void *result;
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__memalign_hook = old_memalign_hook;
	__realloc_hook = old_realloc_hook;
	/* Call recursively */
	size_t modified_size = size;
	pre_alloc(&modified_size, caller);
	result = memalign(alignment, modified_size);
	if (result) post_successful_alloc(result, modified_size, caller);
	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_memalign_hook = __memalign_hook;
	old_realloc_hook = __realloc_hook;
	/* printf might call free, so protect it too. */
	#ifdef TRACE_MALLOC_HOOKS
	printf ("memalign (%u, %u) returns %p\n", (unsigned) alignment, (unsigned) size, result);
	#endif
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__memalign_hook = my_memalign_hook;
	__realloc_hook = my_realloc_hook;
	return result;
}


static void *
my_realloc_hook(void *ptr, size_t size, const void *caller)
{
	void *result;
	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__memalign_hook = old_memalign_hook;
	__realloc_hook = old_realloc_hook;
	size_t old_usable_size;
	/* Split cases. First we eliminate the cases where
	 * realloc() degenerates into either malloc or free. */
	if (ptr == NULL)
	{
		/* We behave like malloc(). */
		pre_alloc(&size, caller);
	}
	else if (size == 0)
	{
		/* We behave like free(). */
		pre_nonnull_free(ptr, malloc_usable_size(ptr));
	}
	else
	{
		/* We are doing a bone fide realloc. This might fail, leaving the
		 * original block untouched. 
		 * If it changes, we'll need to know the old usable size to access
		 * the old trailer. */
		old_usable_size = malloc_usable_size(ptr);
		pre_nonnull_nonzero_realloc(ptr, size, caller, result);
	}
	
	/* Modify the size, as usual, *only if* size != 0 */
	size_t modified_size = size;
	if (size != 0)
	{
		pre_alloc(&modified_size, caller);
	}

	result = realloc(ptr, modified_size);
	
	if (ptr == NULL)
	{
		/* like malloc() */
		if (result) post_successful_alloc(result, modified_size, caller);
	}
	else if (size == 0)
	{
		/* like free */
		post_nonnull_free(ptr);
	}
	else
	{
		/* bona fide realloc */
		post_nonnull_nonzero_realloc(ptr, modified_size, old_usable_size, caller, result);
	}

	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_memalign_hook = __memalign_hook;
	old_realloc_hook = __realloc_hook;
	/* printf might call free, so protect it too. */
	#ifdef TRACE_MALLOC_HOOKS
	printf ("realigned pointer %p to %p (requested size %u, modified size %u)\n", ptr, result,  
	  (unsigned) size, (unsigned) modified_size);
	#endif
	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__memalign_hook = my_memalign_hook;
	__realloc_hook = my_realloc_hook;
	return result;
}
