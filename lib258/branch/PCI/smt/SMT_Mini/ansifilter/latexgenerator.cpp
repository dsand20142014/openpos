		     size_t __buflen) __THROW;
extern size_t __REDIRECT_NTH (__confstr_alias, (int __name, char *__buf,
						size_t __len), confstr);
extern size_t __REDIRECT_NTH (__confstr_chk_warn,
			      (int __name, char *__buf, size_t __len,
			       size_t __buflen), __confstr_chk)
     __warnattr ("confstr called with bigger length than size of destination "
		 "buffer");

__extern_always_inline size_t
__NTH (confstr (int __name, char *__buf, size_t __len))
{
  if (__bos (__buf) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
	return __confstr_chk (__name, __buf, __len, __bos (__buf));

      if (__bos (__buf) < __len)
	return __confstr_chk_warn (__name, __buf, __len, __bos (__buf));
    }
  return __confstr_alias (__name, __buf, __len);
}


extern int __getgroups_chk (int __size, __gid_t __list[], size_t __listlen)
     __THROW __wur;
extern int __REDIRECT_NTH (__getgroups_alias, (int __size, __gid_t __list[]),
			   getgroups) __wur;
extern int __REDIRECT_NTH (__getgroups_chk_warn,
			   (int __size, __gid_t __list[], size_t __listlen),
			   __getgroups_chk)
     __wur __warnattr ("getgroups called with bigger group count than what "
		       "can fit into destination buffer");

__extern_always_inline int
__NTH (getgroups (int __size, __gid_t __list[]))
{
  if (__bos (__list) != (size_t) -1)
    {
      if (!__builtin_constant_p (__size) || __size < 0)
	return __getgroups_chk (__size, __list, __bos (__list));

      if (__size * sizeof (__gid_t) > __bos (__list))
	return __getgroups_chk_warn (__size, __list, __bos (__list));
    }
  return __getgroups_alias (__size, __list);
}


extern int __ttyname_r_chk (int __fd, char *__buf, size_t __buflen,
			    size_t __nreal) __THROW __nonnull ((2));
extern int __REDIRECT_NTH (__ttyname_r_alias, (int __fd, char *__buf,
					       size_t __buflen), ttyname_r)
     __nonnull ((2));
extern int __REDIRECT_NTH (__ttyname_r_chk_warn,
			   (int __fd, char *__buf, size_t __buflen,
			    size_t __nreal), __ttyname_r_chk)
     __nonnull ((2)) __warnattr ("ttyname_r called with bigger buflen than "
				 "size of destination buffer");

__extern_always_inline int
__NTH (ttyname_r (int __fd, char *__buf, size_t __buflen))
{
  if (__bos (__buf) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
	return __ttyname_r_chk (__fd, __buf, __buflen, __bos (__buf));

      if (__buflen > __bos (__buf))
	return __ttyname_r_chk_warn (__fd, __buf, __buflen, __bos (__buf));
    }
  return __ttyname_r_alias (__fd, __buf, __buflen);
}


#if defined __USE_REENTRANT || defined __USE_POSIX199506
extern int __getlogin_r_chk (char *__buf, size_t __buflen, size_t __nreal)
     __nonnull ((1));
extern int __REDIRECT (__getlogin_r_alias, (char *__buf, size_t __buflen),
		       getlogin_r) __nonnull ((1));
extern int __REDIRECT (__getlogin_r_chk_warn,
		       (char *__buf, size_t __buflen, size_t __nreal),
		       __getlogin_r_chk)
     __nonnull ((1)) __warnattr ("getlogin_r called with bigger buflen than "
				 "size of destination buffer");

__extern_always_inline int
getlogin_r (char *__buf, size_t __buflen)
{
  if (__bos (__buf) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
	return __getlogin_r_chk (__buf, __buflen, __bos (__buf));

      if (__buflen > __bos (__buf))
	return __getlogin_r_chk_warn (__buf, __buflen, __bos (__buf));
    }
  return __getlogin_r_alias (__buf, __buflen);
}
#endif


#if defined __USE_BSD || defined __USE_UNIX98
extern int __gethostname_chk (char *__buf, size_t __buflen, size_t __nreal)
     __THROW __nonnull ((1));
extern int __REDIRECT_NTH (__gethostname_alias, (char *__buf, size_t __buflen),
			   gethostname) __nonnull ((1));
extern int __REDIRECT_NTH (__gethostname_chk_warn,
			   (char *__buf, size_t __buflen, size_t __nreal),
			   __gethostname_chk)
     __nonnull ((1)) __warnattr ("gethostname called with bigger buflen than "
				 "size of destination buffer");

__extern_