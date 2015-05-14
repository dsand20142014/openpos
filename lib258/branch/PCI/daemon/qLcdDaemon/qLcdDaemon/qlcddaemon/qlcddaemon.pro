				      \
	pthread_mutexattr_t __attr;					      \
	__pthread_mutexattr_init (&__attr);				      \
	__pthread_mutexattr_settype (&__attr, PTHREAD_MUTEX_RECURSIVE_NP);    \
	__pthread_mutex_init (&(NAME).mutex, &__attr);			      \
	__pthread_mutexattr_destroy (&__attr);				      \
      }									      \
  } while (0)

/* Finalize the named lock variable, which must be locked.  It cannot be
   used again until __libc_lock_init is called again on it.  This must be
   called on a lock variable before the containing storage is reused.  */
#if defined _LIBC && (!defined NOT_IN_libc || defined IS_IN_libpthread)
# define __libc_lock_fini(NAME) ((void) 0)
#else
# define __libc_lock_fini(NAME) \
  __libc_maybe_call (__pthread_mutex_destroy, (&(NAME)), 0)
#endif
#if defined SHARED && !defined NOT_IN_libc
# define __libc_rwlock_fini(NAME) ((void) 0)
#else
# define __libc_rwlock_fini(NAME) \
  __libc_maybe_call (__pthread_rwlock_destroy, (&(NAME)), 0)
#endif

/* Finalize recursive named lock.  */
#if defined _LIBC && (!defined NOT_IN_libc || defined IS_IN_libpthread)
# define __libc_lock_fini_recursive(NAME) ((void) 0)
#else
# define __libc_lock_fini_recursive(NAME) \
  __libc_maybe_call (__pthread_mutex_destroy, (&(NAME)), 0)
#endif

/* Lock the named lock variable.  */
#if defined _LIBC && (!defined NOT_IN_libc || defined IS_IN_libpthread)
# if __OPTION_EGLIBC_BIG_MACROS != 1
/* EGLIBC: Declare wrapper function for a big macro if either
   !__OPTION_EGLIBC_BIG_MACROS or we are using a back door from
   small-macros-fns.c (__OPTION_EGLIBC_BIG