# Turgens Shell Documentation
Welcomee to Turgen Shell documentations. Where this will show you the contributing rules and tour around the repository.


Turgen Shell, is a command interpreter with modern interactive UI  designed for fast paced and convenient usage.

## Project Tree
Before understanding how Turgen links together. Take a look at turgen's project tree:

```
./turgen/
├── build/
├── command/
├── docs/
├── include/
├── lib/
├── module/
├── parser/
└── ui/
```

- `build/`: a folder where compiled binary will be placed. **(this folder is ignored by .gitignore)**
- `command/`: a directory of all built-in commands of the turgen shell.
- `docs/`: directory that contained detailed documentations.
- `include/`: directory for all turgen's public header files.
- `lib/`: directory for turgen's external libraries.
- `module/`: directory for shell's component and API source code.
- `parser/`: a directory for command parsing.
- `ui/`: directory for turgen user interface.

## How to Code in turgen repository
These are the *do's and don'ts* on turgen repository

### No `malloc()`
turgen uses **Static Memory Allocation** to avoid memory leaks and heap fragmentation. 
Exept the initial `globs.c` file set up. For performance purposes

If you need to add new buffers, please consider if they can be declared as static or sized using constant defines **(#define MAX_BUFFER_SIZE)** rather than dynamically allocating them at runtime.

Strictly avoid dynamic memory allocation **(malloc/free)**.

### Prohibition of Ternary Operators
The use of the ternary conditional operator `(? :)` is strictly prohibited within this codebase.

To ensure readability at a glance which is best for debugging. Make sure to EXPLICITLY not to use Ternary Conditional Operator, all conditional logic must be expressed using standard `if-else` blocks.

**Ternary** condition operator is **PROHITBITED**: 
``` c
x = (condition) ? a : b;
```
instead, use:
```c
if (condition) {
    x = a;
} else {
    x = b;
}
```

### Header's Inclusions
Turgen Shell does not **manually** include all of the standard libraries into each source file. But, manual inclusions for local header files is approachable.


All standard libraries inclusions are written down in a single header file at [include/turgen.h](./include/turgen.h)

[include/turgen.h](./include/turgen.h)'s content:
```c
#ifndef TURGEN
#define TURGEN

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <pwd.h>
#include <poll.h>
#include <libgen.h>
#include <dirent.h>
#include <glob.h>


#define shellname "turgen"


#endif
```

`turgen.h` file should defined all of the standard libraries. There's a define of `shellname` as a constant string to use when making shell responding messages. 

examples from source code:

```c
#include "navigation.h"
#include "sout.h"
#include "terminal.h"
#include "globs.h"

...
sout("\r%s: %s: command not found\r\n", shellname, finalArgv[0]);
```

### Prohibition of `printf()` and `fprintf()`
- Do NOT use: `printf()`, `fprintf()`, `sprintf()`, or any related standard.
- Must use: `write()` **(system call)** or the custom wrapper function `sout()`.

`sout()` is forwarded from an external library. Learn more about library from Source: https://github.com/waxodium/sout

---
