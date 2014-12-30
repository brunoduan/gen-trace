#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#ifndef __ANDROID__
#include <sys/user.h>
#else
#include <asm/user.h>
#endif //__ANDROID__
#include <string.h>
#include <errno.h>

#include "log.h"
#include "code_manager_impl.h"

code_manager::~code_manager () {}

code_manager_impl::code_manager_impl () : left_ (0), current_page_ (NULL) {}
code_manager_impl::~code_manager_impl ()
{
  for (context_vector::iterator i = contexts_.begin (); i != contexts_.end ();
       ++i)
    {
      free (*i);
    }
}

code_context *
code_manager_impl::new_context (const char *function_name)
{
  int function_name_len = strlen (function_name);
  void *mem = malloc (sizeof (code_context) + function_name_len + 1);
  memset (mem, 0, sizeof (code_context) + function_name_len + 1);
  char *deep_copy_str = static_cast<char *> (mem);
  deep_copy_str += sizeof (code_context);
  strcpy (deep_copy_str, function_name);

  code_context *new_context = static_cast<code_context *> (mem);
  new_context->function_name = deep_copy_str;
  contexts_.push_back (new_context);
  return new_context;
}

void *
code_manager_impl::new_code_mem (void *hint, size_t s)
{
  if (hint == NULL)
    return new_code_mem_no_hint (s);

  // check if current_page_ suitable.
  intptr_t hint_i = reinterpret_cast<intptr_t> (hint);
  intptr_t current_page_i = reinterpret_cast<intptr_t> (current_page_);
  if (current_page_i && (current_page_i <= (hint_i + 0x7fffffff))
      && (current_page_i >= (hint_i - 0x7fffffff)) && (left_ >= s))
    {
      void *ret = current_page_;
      current_page_ += s;
      left_ -= s;
      return ret;
    }
  // Make a suitable page.
  intptr_t page_mask = ~(PAGE_SIZE - 1);
  hint_i &= page_mask;
  hint_i += PAGE_SIZE;
  query_status q_status;

  while ((q_status = query (reinterpret_cast<void *> (hint_i)))
         == query_occupied)
    {
      hint_i += PAGE_SIZE;
    }
  if (q_status != query_okay)
    {
      LOGE ("mincore fails %s\n", strerror (errno));
      return NULL;
    }
  // allocate mem using hint_i.
  {
    void *new_page = mmap (reinterpret_cast<void *> (hint_i), PAGE_SIZE,
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (new_page == MAP_FAILED)
      {
        LOGE ("mmap fails %s\n", strerror (errno));
        return NULL;
      }
    current_page_ = static_cast<char *> (new_page);
    left_ = PAGE_SIZE;
    codes_.push_back (new_page);
    return new_code_mem_no_hint (s);
  }
}

void *
code_manager_impl::new_code_mem_no_hint (size_t s)
{
  if (current_page_ && left_ >= s)
    {
      void *ret = current_page_;
      current_page_ += s;
      left_ -= s;
      return ret;
    }
  else
    {
      void *new_page
          = mmap (NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (new_page == MAP_FAILED)
        {
          LOGE ("mmap fails %s\n", strerror (errno));
          return NULL;
        }
      current_page_ = static_cast<char *> (new_page);
      left_ = PAGE_SIZE;
      codes_.push_back (new_page);
      return new_code_mem_no_hint (s);
    }
  return NULL;
}

code_manager_impl::query_status
code_manager_impl::query (void *q)
{
  typedef std::set<void *>::iterator iterator;
  std::pair<iterator, bool> r = queried_.insert (q);
  if (r.second == false)
    {
      return query_occupied;
    }
  unsigned char whatever;
  int mincore_return = mincore (q, PAGE_SIZE, &whatever);
  if (mincore_return == 0)
    {
      return query_occupied;
    }
  else if (errno != ENOMEM)
    {
      return query_mincore_fail;
    }
  return query_okay;
}
