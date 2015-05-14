) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  && __string2_1bptr_p (src) && n <= 8			      \
		  ? __builtin_memcpy (dest, src, n) + (n)		      \
		  : __mempcpy (dest, src, n)))
#   else
#    define __mempcpy(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  && __string2_1bptr_p (src) && n <= 8			      \
		  ? __mempcpy_small (dest, __mempcpy_args (src), n)	      \
		  : __mempcpy (dest, src, n)))
#   endif
/* In glibc we use this function frequently but for namespace reasons
   we have to use the name `__mempcpy'.  */
#   define mempcpy(dest, src, n) __mempcpy (dest, src, n)
#  endif

#  if !__GNUC_PREREQ (3, 0) || defined _FORCE_INLINES
#   if _STRING_ARCH_unaligned
#    ifndef _FORCE_INLINES
#     define __mempcpy_args(src) \
     ((__const char *) (src))[0], ((__const char *) (src))[2],		      \
     ((__const char *) (src))[4], ((__const char *) (src))[6],		      \
     __extension__ __STRING2_SMALL_GET16 (src, 0),			      \
     __extension__ __STRING2_SMALL_GET16 (src, 4),			      \
     __extension__ __STRING2_SMALL_GET32 (src, 0),			      \
     __extension__ __STRING2_SMALL_GET32 (src, 4)
#    endif
__STRING_INLINE void *__mempcpy_small (void *, char, char, char, char,
				       __uint16_t, __uint16_t, __uint32_t,
				       __uint32_t, size_t);
__STRING_INLINE void *
__mempcpy_small (void *__dest1,
		 char __src0_1, char __src2_1, char __src4_1, char __src6_1,
		 __uint16_t __src0_2, __uint16_t __src4_2,
		 __uint32_t __src0_4, __uint32_t __src4_4,
		 size_t __srclen)
{
  union {
    __uint32_t __ui;
    __uint16_t __usi;
    unsigned char __uc;
    unsigned char __c;
  } *__u = __dest1;
  switch ((unsigned int) __srclen)
    {
    case 1:
      __u->__c = __src0_1;
      __u = __extension__ ((void *) __u + 1);
      break;
    case 2:
      __u->__usi = __src0_2;
      __u = __extension__ ((void *) __u + 2);
      break;
    case 3:
      __u->__usi = __src0_2;
      __u = __extension__ ((void *) __u + 2);
      __u->__c = __src2_1;
      __u = __extension__ ((void *) __u + 1);
      break;
    case 4:
      __u->__ui = __src0_4;
      __u = __extension__ ((void *) __u + 4);
      break;
    case 5:
      __u->__ui = __src0_4;
      __u = __extension__ ((void *) __u + 4);
      __u->__c = __src4_1;
      __u = __extension__ ((void *) __u + 1);
      break;
    case 6:
      __u->__ui = __src0_4;
      __u = __extension__ ((void *) __u + 4);
      __u->__usi = __src4_2;
      __u = __extension__ ((void *) __u + 2);
      break;
    case 7:
      __u->__ui = __src0_4;
      __u = __extension__ ((void *) __u + 4);
      __u->__usi = __src4_2;
      __u = __extension__ ((void *) __u + 2);
      __u->__c = __src6_1;
      __u = __extension__ ((void *) __u + 1);
      break;
    case 8:
      __u->__ui = __src0_4;
      __u = __extension__ ((void *) __u + 4);
      __u->__ui = __src4_4;
      __u = __extension__ ((void *) __u + 4);
      break;
    }
  return (void *) __u;
}
#   else
#    ifndef _FORCE_INLINES
#     define __mempcpy_args(src) \
     ((__const char *) (src))[0],					      \
     __extension__ ((__STRING2_COPY_ARR2)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1] } }),      \
     __extension__ ((__STRING2_COPY_ARR3)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2] } }),				      \
     __extension__ ((__STRING2_COPY_ARR4)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2], ((__const char *) (src))[3] } }),      \
     __extension__ ((__STRING2_COPY_ARR5)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2], ((__const char *) (src))[3],	      \
	  ((__const char *) (src))[4] } }),				      \
     __extension__ ((__STRING2_COPY_ARR6)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2], ((__const char *) (src))[3],	      \
	  ((__const char *) (src))[4], ((__const char *) (src))[5] } }),      \
     __extension__ ((__STRING2_COPY_ARR7)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2], ((__const char *) (src))[3],	      \
	  ((__const char *) (src))[4], ((__const char *) (src))[5],	      \
	  ((__const char *) (src))[6] } }),				      \
     __extension__ ((__STRING2_COPY_ARR8)				      \
      { { ((__const char *) (src))[0], ((__const char *) (src))[1],	      \
	  ((__const char *) (src))[2], ((__const char *) (src))[3],	      \
	  ((__const char *) (src))[4], ((__const char *) (src))[5],	      \
	  ((__const char *) (src))[6], ((__const char *) (src))[7] } })
#    endif
__STRING_INLINE void *__mempcpy_small (void *, char, __STRING2_COPY_ARR2,
				       __STRING2_COPY_ARR3,
				       __STRING2_COPY_ARR4,
				       __STRING2_COPY_ARR5,
				       __STRING2_COPY_ARR6,
				       __STRING2_COPY_ARR7,
				       __STRING2_COPY_ARR8, size_t);
__STRING_INLINE void