#define __STDC_FORMAT_MACROS
// C Headers
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
// POSIX Headers
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syscall.h>
// C++ Headers
#include <new>

#ifndef CTRACE_FILE_NAME
#define CTRACE_FILE_NAME "trace_%d.json"
#endif // CTRACE_FILE_NAME
#define CRASH()                                                               \
  do                                                                          \
    {                                                                         \
      (*(int *)0xeadbaddc = 0);                                               \
    }                                                                         \
  while (0)

namespace
{
pthread_key_t thread_info_key;
FILE *file_to_write;
static const int64_t invalid_time = static_cast<int64_t> (-1);
static const int frequency = 100000;
static const int ticks = 1;
static const int max_idle_times = 1000;
static const int signo = SIGRTMIN + 4;
static const int CLOCKID = CLOCK_MONOTONIC;

// for WriterThread
pthread_mutex_t writer_waitup_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool writer_waitup = false;
pthread_cond_t writer_waitup_cond = PTHREAD_COND_INITIALIZER;
struct Record;
struct Record *pending_records_head;

#ifdef __ARM_EABI__

struct sigcontext
{
  unsigned long trap_no;
  unsigned long error_code;
  unsigned long oldmask;
  unsigned long arm_r0;
  unsigned long arm_r1;
  unsigned long arm_r2;
  unsigned long arm_r3;
  unsigned long arm_r4;
  unsigned long arm_r5;
  unsigned long arm_r6;
  unsigned long arm_r7;
  unsigned long arm_r8;
  unsigned long arm_r9;
  unsigned long arm_r10;
  unsigned long arm_fp;
  unsigned long arm_ip;
  unsigned long arm_sp;
  unsigned long arm_lr;
  unsigned long arm_pc;
  unsigned long arm_cpsr;
  unsigned long fault_address;
};

#endif

struct ucontext
{
  unsigned long uc_flags;
  struct ucontext *uc_link;
  stack_t uc_stack;
  struct sigcontext uc_mcontext;
  sigset_t uc_sigmask; /* mask last for extensibility */
};

struct CTraceStruct
{
  int64_t start_time_;
  int64_t min_end_time_;
  uint64_t name_;
  CTraceStruct (void *);
  CTraceStruct () {}
};

struct ThreadInfo
{
  static const int max_stack = 1000;
  int pid_;
  int tid_;
  CTraceStruct stack_[max_stack];
  int stack_end_;
  int64_t current_time_;
  int idle_times_;
  bool blocked_;
  timer_t timerid_;
  ThreadInfo ();
  ~ThreadInfo ();
  void UpdateCurrentTime ();
  void SetBlocked ();
  void NewBack (void *);
  CTraceStruct *PoppedBack ();
  static ThreadInfo *New ();
  static ThreadInfo *Find ();
};

static const int MAX_THREADS = 100;
char info_store_char[MAX_THREADS * sizeof (ThreadInfo)];

struct FreeListNode
{
  const struct FreeListNode *next_;
};

FreeListNode *free_head;

struct Record
{
  int pid_;
  int tid_;
  int64_t start_time_;
  int64_t dur_;
  uint64_t name_;
  struct Record *next_;
};

struct Lock
{
  Lock (pthread_mutex_t *mutex) : mutex_ (mutex)
  {
    pthread_mutex_lock (mutex_);
  }
  ~Lock () { pthread_mutex_unlock (mutex_); }
  pthread_mutex_t *mutex_;
};

void
ThreadInfo::SetBlocked ()
{
  blocked_ = true;
  idle_times_ = 0;
  timerid_ = 0;
}

ThreadInfo *
ThreadInfo::Find ()
{
  return static_cast<ThreadInfo *> (pthread_getspecific (thread_info_key));
}

int64_t GetTimesFromClock (int clockid);

void
ThreadInfo::UpdateCurrentTime ()
{
  int64_t time = GetTimesFromClock (CLOCK_MONOTONIC);
  if (time < current_time_)
    current_time_ += 1;
  else
    current_time_ = time;
}

ThreadInfo *
ThreadInfo::New ()
{
  ThreadInfo *free_thread_info;
  while (true)
    {
      FreeListNode *current_free = free_head;
      if (current_free == NULL)
        CRASH ();
      if (!__sync_bool_compare_and_swap (&free_head, current_free,
                                         current_free->next_))
        continue;
      free_thread_info = reinterpret_cast<ThreadInfo *> (current_free);
      break;
    }
  if (free_thread_info == NULL)
    CRASH ();
  pthread_setspecific (thread_info_key, free_thread_info);
  return new (free_thread_info) ThreadInfo ();
}

ThreadInfo::ThreadInfo ()
{
  pid_ = getpid ();
  tid_ = syscall (__NR_gettid, 0);
  stack_end_ = 0;
  idle_times_ = 0;
  current_time_ = 0;
  blocked_ = true;
  timerid_ = 0;
}

ThreadInfo::~ThreadInfo () { timer_delete (timerid_); }

void
ThreadInfo::NewBack (void *name)
{
  assert (stack_end_ < max_stack);
  CTraceStruct *cs = &stack_[stack_end_];
  new (cs) CTraceStruct (name);
}

CTraceStruct *
ThreadInfo::PoppedBack ()
{
  return &stack_[stack_end_];
}

CTraceStruct::CTraceStruct (void *name)
{
  start_time_ = invalid_time;
  min_end_time_ = invalid_time;
  name_ = reinterpret_cast<uint64_t> (name);
}

ThreadInfo *
_GetThreadInfo ()
{
  ThreadInfo *tinfo = ThreadInfo::Find ();
  if (tinfo)
    return tinfo;
  tinfo = ThreadInfo::New ();
  return tinfo;
}

int64_t
GetTimesFromClock (int clockid)
{
  static const int64_t kMillisecondsPerSecond = 1000;
  static const int64_t kMicrosecondsPerMillisecond = 1000;
  static const int64_t kMicrosecondsPerSecond = kMicrosecondsPerMillisecond
                                                * kMillisecondsPerSecond;
  static const int64_t kNanosecondsPerMicrosecond = 1000;

  struct timespec ts_thread;
  clock_gettime (clockid, &ts_thread);
  return (static_cast<int64_t> (ts_thread.tv_sec) * kMicrosecondsPerSecond)
         + (static_cast<int64_t> (ts_thread.tv_nsec)
            / kNanosecondsPerMicrosecond);
}

void SetupTimer (ThreadInfo *tinfo);

ThreadInfo *
GetThreadInfo ()
{
  ThreadInfo *tinfo = _GetThreadInfo ();
  if (tinfo->blocked_)
    {
      tinfo->UpdateCurrentTime ();
      sigset_t unblock_set;
      sigemptyset (&unblock_set);
      sigaddset (&unblock_set, signo);
      sigprocmask (SIG_UNBLOCK, &unblock_set, 0);
      SetupTimer (tinfo);
      tinfo->blocked_ = false;
    }
  return tinfo;
}

void
DeleteThreadInfo (void *tinfo)
{
  static_cast<ThreadInfo *> (tinfo)->~ThreadInfo ();
  FreeListNode *free_node = static_cast<FreeListNode *> (tinfo);
  while (true)
    {
      FreeListNode *current_free = free_head;
      free_node->next_ = current_free;
      if (__sync_bool_compare_and_swap (&free_head, current_free, free_node))
        break;
    }
}

void
MyHandler (int, siginfo_t *, void *context)
{
  // we don't use GetThreadInfo , because
  // it make no sense to deal
  // with the thread without this structure
  // created in __start_ctrace__.
  ThreadInfo *tinfo = ThreadInfo::Find ();
  if (!tinfo)
    {
      assert (tinfo);
      return;
    }
  int64_t old_time = tinfo->current_time_;
  tinfo->current_time_ += frequency * ticks / 1000;
  int64_t current_time = tinfo->current_time_;

  if (tinfo->stack_end_ >= ThreadInfo::max_stack)
    {
      CRASH ();
    }
  for (int i = 0; i < tinfo->stack_end_; ++i, old_time += ticks)
    {
      CTraceStruct *cur = &tinfo->stack_[i];
      if (cur->start_time_ != invalid_time)
        continue;
      cur->start_time_ = old_time;
    }
  if (old_time > current_time)
    {
      tinfo->current_time_ = old_time + ticks;
      current_time = tinfo->current_time_;
    }
  if (tinfo->stack_end_ != 0)
    {
      tinfo->stack_[tinfo->stack_end_ - 1].min_end_time_ = current_time
                                                           + ticks;
    }
  else
    {
      tinfo->idle_times_++;
      if (tinfo->idle_times_ >= max_idle_times)
        {
          // will block signo
          timer_delete (tinfo->timerid_);
          tinfo->SetBlocked ();
          tinfo->idle_times_ = 0;
        }
    }
}

void *WriterThread (void *);

struct Initializer
{
  void
  InitFreeList ()
  {
    ThreadInfo *info_store = reinterpret_cast<ThreadInfo *> (info_store_char);
    free_head
        = reinterpret_cast<FreeListNode *> (&info_store[MAX_THREADS - 1]);
    free_head->next_ = NULL;
    for (int i = MAX_THREADS - 2; i >= 0; --i)
      {
        FreeListNode *current
            = reinterpret_cast<FreeListNode *> (&info_store[i]);
        current->next_ = free_head;
        free_head = current;
      }
  }

  class MapLineData
  {
  public:
    MapLineData (char *str);

    intptr_t getBase ();
    intptr_t getEnd ();

  private:
    intptr_t m_base;
    intptr_t m_end;
  };

  static void StartFile (FILE *f);

  Initializer ()
  {
    pthread_key_create (&thread_info_key, DeleteThreadInfo);
    InitFreeList ();
    struct sigaction myaction = { 0 };
    myaction.sa_sigaction = MyHandler;
    myaction.sa_flags = SA_SIGINFO | SA_RESTART;
    if (-1 == sigaction (signo, &myaction, NULL))
      {
        perror ("");
        abort ();
      }

    char buffer[256];
    sprintf (buffer, CTRACE_FILE_NAME, getpid ());
    file_to_write = fopen (buffer, "wb");
    StartFile (file_to_write);
    pthread_t my_writer_thread;
    pthread_create (&my_writer_thread, NULL, WriterThread, NULL);
  }

  ~Initializer () { fclose (file_to_write); }
};

Initializer::MapLineData::MapLineData (char *str) : m_base (-1), m_end (-1)
{
  int len = strlen (str);
  // strip the '\n'
  str[len - 1] = '\0';

  char *address_sep = strchr (str, '-');
  if (!address_sep)
    {
      return;
    }
  address_sep[0] = '\0';
  errno = 0;
  m_base = strtoll (str, NULL, 16);
  if (errno)
    {
      m_base = -1;
      return;
    }
  str = address_sep + 1;
  char *space = strchr (str, ' ');
  if (!space)
    {
      return;
    }
  space[0] = '\0';
  errno = 0;
  m_end = strtoll (str, NULL, 16);
  if (errno)
    {
      m_end = -1;
      return;
    }
}

intptr_t
Initializer::MapLineData::getBase ()
{
  return m_base;
}

intptr_t
Initializer::MapLineData::getEnd ()
{
  return m_end;
}

void
Initializer::StartFile (FILE *f)
{
  char sz[1024] = "/proc/self/maps";
  FILE *fp = fopen (sz, "r");
  if (!fp)
    {
      return;
    }
  intptr_t return_address
      = reinterpret_cast<intptr_t> (__builtin_return_address (0));
  while (fgets (sz, 1024, fp))
    {
      MapLineData data (sz);
      if (data.getBase () == -1)
        break;
      if (data.getBase () <= return_address && data.getEnd () > return_address)
        {
          Record r = { 0, 0, 0, 0, static_cast<uint64_t> (data.getBase ()) };
          fwrite (&r, sizeof (r) - sizeof (void *), 1, f);
          fflush (f);
          fclose (fp);
          return;
        }
    }
  __builtin_unreachable ();
}

Initializer __init__;

void
RecordThis (CTraceStruct *c, ThreadInfo *tinfo)
{
  Record *r = static_cast<Record *> (malloc (sizeof (Record)));
  if (!r)
    CRASH ();
  r->pid_ = tinfo->pid_;
  r->tid_ = tinfo->tid_;
  r->start_time_ = c->start_time_;
  r->name_ = c->name_;
  assert (c->min_end_time_ != invalid_time && c->start_time_ != invalid_time);
  if (c->min_end_time_ > c->start_time_)
    r->dur_ = c->min_end_time_ - c->start_time_;
  else
    r->dur_ = 1;
  assert (r->dur_ != invalid_time);

  while (true)
    {
      Record *current_head = pending_records_head;
      r->next_ = current_head;
      if (__sync_bool_compare_and_swap (&pending_records_head, current_head,
                                        r))
        break;
    }
  {
    Lock lock (&writer_waitup_mutex);
    writer_waitup = true;
    pthread_cond_signal (&writer_waitup_cond);
  }
}

void
DoWriteRecursive (struct Record *current)
{
  if (current->next_)
    DoWriteRecursive (current->next_);

  fwrite (current, sizeof (*current) - sizeof (void *), 1, file_to_write);
  static int flushCount = 0;
  if (flushCount++ == 5)
    {
      fflush (file_to_write);
      flushCount = 0;
    }
  free (current);
}

void *
WriterThread (void *)
{
  pthread_setname_np (pthread_self (), "WriterThread");

  while (true)
    {
      Record *record_to_write;

      {
        Lock lock (&writer_waitup_mutex);
        if (writer_waitup == false)
          pthread_cond_wait (&writer_waitup_cond, &writer_waitup_mutex);
        assert (writer_waitup == true);
        writer_waitup = false;
      }
      while (pending_records_head)
        {
          while (true)
            {
              record_to_write = pending_records_head;
              if (record_to_write == NULL)
                break;
              if (__sync_bool_compare_and_swap (&pending_records_head,
                                                record_to_write, NULL))
                break;
            }
          if (record_to_write == NULL)
            break;
          DoWriteRecursive (record_to_write);
        }
    }
  return NULL;
}

void
SetupTimer (ThreadInfo *tinfo)
{
  struct itimerspec timer;
  struct sigevent sev = { 0 };
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_nsec = frequency;
  timer.it_interval = timer.it_value;
  sev.sigev_notify = SIGEV_THREAD_ID;
  sev.sigev_signo = signo;
  sev.sigev_value.sival_ptr = &tinfo;
  sev._sigev_un._tid = tinfo->tid_;
  if (timer_create (CLOCKID, &sev, &tinfo->timerid_) == -1)
    {
      abort ();
    }
  if (timer_settime (tinfo->timerid_, 0, &timer, NULL) == -1)
    {
      abort ();
    }
}
}

extern "C" {
void __cyg_profile_func_enter (void *this_fn, void *call_site);
void __cyg_profile_func_exit (void *this_fn, void *call_site);
}

void
__cyg_profile_func_enter (void *this_fn, void *call_site)
{
  if (file_to_write == 0)
    return;
  ThreadInfo *tinfo = GetThreadInfo ();
  if (tinfo->stack_end_ == 0)
    {
      // always update the time in the first entry.
      // Or if it sleep too long, will make this entry looks
      // very time consuming.
      tinfo->UpdateCurrentTime ();
    }
  if (tinfo->stack_end_ < ThreadInfo::max_stack)
    {
      tinfo->NewBack (this_fn);
    }
  tinfo->stack_end_++;
}

void
__cyg_profile_func_exit (void *this_fn, void *call_site)
{
  if (file_to_write == 0)
    return;
  ThreadInfo *tinfo = GetThreadInfo ();
  tinfo->stack_end_--;
  if (tinfo->stack_end_ < ThreadInfo::max_stack)
    {
      assert (tinfo->stack_end_ >= 0);
      CTraceStruct *c = tinfo->PoppedBack ();
      if (c->start_time_ != invalid_time)
        {
          // we should record this
          RecordThis (c, tinfo);
          if (tinfo->stack_end_ != 0)
            {
              // propagate the back's mini end time
              tinfo->stack_[tinfo->stack_end_ - 1].min_end_time_
                  = c->min_end_time_ + ticks;
            }
        }
    }
}