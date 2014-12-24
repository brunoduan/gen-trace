#include "dis.h"
#include "dis_client.h"
#include "log.h"
#include <string>
#include <string.h>
#include <assert.h>

class test_dis_client : public dis_client
{
public:
  std::string s_;

private:
  void
  on_instr (const char *instr)
  {
    LOGI ("%s\n", instr);
    s_.assign (instr);
  }
  void on_addr (intptr_t) {}
  virtual bool
  is_accept ()
  {
    return true;
  }
};

const char code[] = "\x41"
                    "\x56"
                    "\x41"
                    "\x55"
                    "\x41"
                    "\x54"
                    "\x55"
                    "\x89"
                    "\xfd"
                    "\xc1"
                    "\xed"
                    "\x1f"
                    "\x53"
                    "\x01"
                    "\xfd"
                    "\xd1"
                    "\xfd"
                    "\x48"
                    "\x83"
                    "\xec"
                    "\x20"
                    "\x83"
                    "\xfd"
                    "\x01"
                    "\x74"
                    "\x19"
                    "\x41"
                    "\x89"
                    "\xfd"
                    "\x31"
                    "\xc0"
                    "\x89"
                    "\xef"
                    "\x41"
                    "\x89"
                    "\xd6"
                    "\x49"
                    "\x89"
                    "\xf4"
                    "\xe8"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x45"
                    "\x85"
                    "\xed"
                    "\x89"
                    "\xc3"
                    "\x75"
                    "\x0d"
                    "\x48"
                    "\x83"
                    "\xc4"
                    "\x20"
                    "\x5b"
                    "\x5d"
                    "\x41"
                    "\x5c"
                    "\x41"
                    "\x5d"
                    "\x41"
                    "\x5e"
                    "\xc3"
                    "\x31"
                    "\xc0"
                    "\x4c"
                    "\x89"
                    "\xe6"
                    "\x44"
                    "\x89"
                    "\xef"
                    "\xe8"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x85"
                    "\xdb"
                    "\x7e"
                    "\xe2"
                    "\x66"
                    "\x0f"
                    "\xef"
                    "\xc0"
                    "\x41"
                    "\xbd"
                    "\x01"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\xf2"
                    "\x41"
                    "\x0f"
                    "\x2a"
                    "\xc6"
                    "\x45"
                    "\x31"
                    "\xf6"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\xc0"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\x05"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x44"
                    "\x24"
                    "\x18"
                    "\x66"
                    "\x0f"
                    "\xef"
                    "\xc0"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x4c"
                    "\x24"
                    "\x18"
                    "\xf2"
                    "\x41"
                    "\x0f"
                    "\x2a"
                    "\xc5"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\xc0"
                    "\xf2"
                    "\x0f"
                    "\x5e"
                    "\xc8"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xc1"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x4c"
                    "\x24"
                    "\x10"
                    "\xe8"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x4c"
                    "\x24"
                    "\x10"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x44"
                    "\x24"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x05"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xc1"
                    "\xe8"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xc8"
                    "\x85"
                    "\xed"
                    "\x43"
                    "\x8d"
                    "\x44"
                    "\x2d"
                    "\x00"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x74"
                    "\x24"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\xc8"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xc1"
                    "\x7e"
                    "\x5e"
                    "\x42"
                    "\x8d"
                    "\x3c"
                    "\xad"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x48"
                    "\x63"
                    "\xd0"
                    "\x31"
                    "\xf6"
                    "\x49"
                    "\x8d"
                    "\x0c"
                    "\xd4"
                    "\x4c"
                    "\x89"
                    "\xe2"
                    "\x48"
                    "\x63"
                    "\xff"
                    "\x48"
                    "\xc1"
                    "\xe7"
                    "\x03"
                    "\x66"
                    "\x90"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x11"
                    "\x01"
                    "\xc6"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x1a"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x49"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xda"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x19"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x5a"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xd9"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x59"
                    "\x08"
                    "\x48"
                    "\x01"
                    "\xf9"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\x12"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\x4a"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x12"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x4a"
                    "\x08"
                    "\x48"
                    "\x01"
                    "\xfa"
                    "\x39"
                    "\xf5"
                    "\x7f"
                    "\xbf"
                    "\x41"
                    "\x83"
                    "\xfd"
                    "\x01"
                    "\x0f"
                    "\x84"
                    "\xec"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x43"
                    "\x8d"
                    "\x54"
                    "\x2d"
                    "\x02"
                    "\x44"
                    "\x8d"
                    "\x0c"
                    "\x00"
                    "\x49"
                    "\x8d"
                    "\x7c"
                    "\x24"
                    "\x10"
                    "\x66"
                    "\x0f"
                    "\xef"
                    "\xe4"
                    "\xbe"
                    "\x01"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x48"
                    "\x63"
                    "\xd2"
                    "\x4d"
                    "\x63"
                    "\xc9"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x0d"
                    "\x00"
                    "\x00"
                    "\x00"
                    "\x4c"
                    "\x8d"
                    "\x14"
                    "\xd5"
                    "\xf0"
                    "\xff"
                    "\xff"
                    "\xff"
                    "\x49"
                    "\xc1"
                    "\xe1"
                    "\x03"
                    "\x0f"
                    "\x1f"
                    "\x40"
                    "\x00"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xd6"
                    "\x45"
                    "\x31"
                    "\xc0"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xe9"
                    "\x85"
                    "\xed"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xd4"
                    "\x49"
                    "\x8d"
                    "\x0c"
                    "\x3a"
                    "\x48"
                    "\x89"
                    "\xfa"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xea"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xd0"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xd1"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xce"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xea"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\xcc"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xe0"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xcc"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xe1"
                    "\x7e"
                    "\x65"
                    "\x66"
                    "\x90"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x11"
                    "\x41"
                    "\x01"
                    "\xc0"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x49"
                    "\x08"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xdd"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xfc"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xda"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xf9"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xd4"
                    "\xf2"
                    "\x0f"
                    "\x59"
                    "\xcd"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xdf"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\xca"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x12"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xd3"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x11"
                    "\xf2"
                    "\x0f"
                    "\x10"
                    "\x52"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x5c"
                    "\xd1"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x51"
                    "\x08"
                    "\x4c"
                    "\x01"
                    "\xc9"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\x1a"
                    "\xf2"
                    "\x0f"
                    "\x58"
                    "\x4a"
                    "\x08"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x1a"
                    "\xf2"
                    "\x0f"
                    "\x11"
                    "\x4a"
                    "\x08"
                    "\x4c"
                    "\x01"
                    "\xca"
                    "\x44"
                    "\x39"
                    "\xc5"
                    "\x7f"
                    "\x9d"
                    "\x83"
                    "\xc6"
                    "\x01"
                    "\x48"
                    "\x83"
                    "\xc7"
                    "\x10"
                    "\x44"
                    "\x39"
                    "\xee"
                    "\x66"
                    "\x0f"
                    "\x28"
                    "\xcd"
                    "\x0f"
                    "\x85"
                    "\x49"
                    "\xff"
                    "\xff"
                    "\xff"
                    "\x41"
                    "\x83"
                    "\xc6"
                    "\x01"
                    "\x41"
                    "\x39"
                    "\xde"
                    "\x0f"
                    "\x84"
                    "\xff"
                    "\xfd"
                    "\xff"
                    "\xff"
                    "\x41"
                    "\x89"
                    "\xc5"
                    "\xe9"
                    "\x39"
                    "\xfe"
                    "\xff"
                    "\xff";

const char *dis_str[] = {
  "push   %r14", "push   %r13", "push   %r12", "push   %rbp",
  "mov    %edi,%ebp", "shr    $0x1f,%ebp", "push   %rbx", "add    %edi,%ebp",
  "sar    %ebp", "sub    $0x20,%rsp", "cmp    $0x1,%ebp", "jz     33 ",
  "mov    %edi,%r13d", "xor    %eax,%eax", "mov    %ebp,%edi",
  "mov    %edx,%r14d", "mov    %rsi,%r12", "callq  2c ", "test   %r13d,%r13d",
  "mov    %eax,%ebx", "jne    40 ", "add    $0x20,%rsp", "pop    %rbx",
  "pop    %rbp", "pop    %r12", "pop    %r13", "pop    %r14", "retq   ",
  "xor    %eax,%eax", "mov    %r12,%rsi", "mov    %r13d,%edi", "callq  4d ",
  "test   %ebx,%ebx", "jle    33 ", "'Uor   %xmm0,%xmm0", "mov    $0x1,%r13d",
  "cvtsi2sd %r14d,%xmm0", "xor    %r14d,%r14d", "addsd  %xmm0,%xmm0",
  "mulsd  0x0(%rip),%xmm0", "movsd  %xmm0,0x18(%rsp)", "'Uor   %xmm0,%xmm0",
  "movsd  0x18(%rsp),%xmm1", "cvtsi2sd %r13d,%xmm0", "addsd  %xmm0,%xmm0",
  "divsd  %xmm0,%xmm1", "movapd %xmm1,%xmm0", "movsd  %xmm1,0x10(%rsp)",
  "callq  9b ", "movsd  0x10(%rsp),%xmm1", "movsd  %xmm0,0x8(%rsp)",
  "movsd  0x0(%rip),%xmm0", "mulsd  %xmm1,%xmm0", "callq  b8 ",
  "movapd %xmm0,%xmm1", "test   %ebp,%ebp", "lea    0x0(%r13,%r13,1),%eax",
  "movsd  0x8(%rsp),%xmm6", "addsd  %xmm0,%xmm1", "mulsd  %xmm1,%xmm0",
  "jle    131 ", "lea    0x0(,%r13,4),%edi",
  "                                                                   ",
  "movslq %eax,%rdx", "xor    %esi,%esi", "lea    (%r12,%rdx,8),%rcx",
  "mov    %r12,%rdx", "movslq %edi,%rdi", "shl    $0x3,%rdi", "xchg   %ax,%ax",
  "movsd  (%rcx),%xmm2", "add    %eax,%esi", "movsd  (%rdx),%xmm3",
  "movsd  0x8(%rcx),%xmm1", "subsd  %xmm2,%xmm3", "movsd  %xmm3,(%rcx)",
  "movsd  0x8(%rdx),%xmm3", "subsd  %xmm1,%xmm3", "movsd  %xmm3,0x8(%rcx)",
  "add    %rdi,%rcx", "addsd  (%rdx),%xmm2", "addsd  0x8(%rdx),%xmm1",
  "movsd  %xmm2,(%rdx)", "movsd  %xmm1,0x8(%rdx)", "add    %rdi,%rdx",
  "cmp    %esi,%ebp", "jg     f0 ", "cmp    $0x1,%r13d", "jz     227 ",
  "lea    0x2(%r13,%r13,1),%edx", "lea    (%rax,%rax,1),%r9d",
  "lea    0x10(%r12),%rdi", "'Uor   %xmm4,%xmm4", "mov    $0x1,%esi",
  "movslq %edx,%rdx", "movsd  0x0(%rip),%xmm1", "lea    -0x10(,%rdx,8),%r10",
  "shl    $0x3,%r9", "nopl   0x0(%rax)", "movapd %xmm6,%xmm2",
  "xor    %r8d,%r8d", "movapd %xmm1,%xmm5", "test   %ebp,%ebp",
  "mulsd  %xmm4,%xmm2", "lea    (%r10,%rdi,1),%rcx", "mov    %rdi,%rdx",
  "subsd  %xmm2,%xmm5", "movapd %xmm0,%xmm2", "mulsd  %xmm1,%xmm2",
  "mulsd  %xmm6,%xmm1", "subsd  %xmm2,%xmm5", "addsd  %xmm4,%xmm1",
  "mulsd  %xmm0,%xmm4", "subsd  %xmm4,%xmm1", "movapd %xmm1,%xmm4",
  "jle    213 ", "xchg   %ax,%ax", "movsd  (%rcx),%xmm2", "add    %eax,%r8d",
  "movsd  0x8(%rcx),%xmm1", "movapd %xmm5,%xmm3", "movapd %xmm4,%xmm7",
  "mulsd  %xmm2,%xmm3", "mulsd  %xmm1,%xmm7", "mulsd  %xmm4,%xmm2",
  "mulsd  %xmm5,%xmm1", "subsd  %xmm7,%xmm3", "addsd  %xmm2,%xmm1",
  "movsd  (%rdx),%xmm2", "subsd  %xmm3,%xmm2", "movsd  %xmm2,(%rcx)",
  "movsd  0x8(%rdx),%xmm2", "subsd  %xmm1,%xmm2", "movsd  %xmm2,0x8(%rcx)",
  "add    %r9,%rcx", "addsd  (%rdx),%xmm3", "addsd  0x8(%rdx),%xmm1",
  "movsd  %xmm3,(%rdx)", "movsd  %xmm1,0x8(%rdx)", "add    %r9,%rdx",
  "cmp    %r8d,%ebp", "jg     1b0 ", "add    $0x1,%esi", "add    $0x10,%rdi",
  "cmp    %r13d,%esi", "movapd %xmm5,%xmm1", "jne    170 ",
  "add    $0x1,%r14d", "cmp    %ebx,%r14d", "jz     33 ", "mov    %eax,%r13d",
  "jmpq   75 ",
};

int
main ()
{
  test_dis_client c;
  disasm::Disassembler dis;
  dis.set_client (&c);
  const char *start = code;
  const char *end = start + sizeof (code) - 1;
  int i = 0;
  for (; start < end; ++i)
    {
      int len = dis.instruction_decode (const_cast<char *> (start));
      start += len;
    }
  return 0;
}
