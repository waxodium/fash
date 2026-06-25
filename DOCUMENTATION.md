# Turgen Shell Documentation
Welcomee to Turgen Shell documentations. Where this will show you the contributing rules and tour around the repository.


Turgen Shell, is a command interpreter with modern interactive UI  designed for fast paced and convenient usage.

## Project Tree
Before understanding how Turgen links together. Take a look at turgen's project tree:

```txt
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
#include "turgen.h"

#include "navigation.h"
#include "sout.h"
#include "terminal.h"
#include "globs.h"

...
sout("\r%s: %s: command not found\r\n", shellname, finalArgv[0]);
```

### Prohibition of `printf()`
- Do NOT use: `printf()`
- Must use: `write()` **(system call)** or the custom wrapper function `sout()`.

`sout()` is forwarded from an external library. Learn more about library from Source: https://github.com/waxodium/sout

## How to make built-in commands
Turgen is a shell command interpreter, just like any other shells. Turgen Shell should have built-in shell commands. Completely written in into the shell, not as an external command or any outside binaries.

Below, are the steps as detailed format and guide to show how to code any new built-in commands and to learn how turgen code structure link together for commands.

`commands/` directory were created for storing built-in commands. This is where you will create turgen shell commands. 

- **New creation**: First, create a **named** file with `.c` extension for your desirable command.
- Include all of your needed headers into your file while following the inclusion format from [Header's Inclusion](https://github.com/waxodium/turgen/blob/main/DOCUMENTATION.md#headers-inclusions) section. 
- **Create new commands**: Every turgen's command *MUST* be an `int` type function using with an exact argument of `(char **argv, ShellState *state)` for each of your every single command function. 
>
> The source of typedef argument for turgen's commands are currently written in the [./include/execute.h](https://github.com/waxodium/turgen/blob/main/include/execute.h) file.
>
> `typedef int (*Handler)(char **, ShellState *);`

- **Create a Header file**: After, your command function is written somewhere from a file that is surely placed in `./command` folder. Make a header file for your created **named** `.c` file. Define the command function into your header file. 
> In turgen, registering a new command requires to directly call the command function from the `static Command builtins[]`. But, because for neat organizing purposes. All built-in commands are written and placed inside of `./command` directory. 

- **Registering command**: To initialize a new command. Turgen uses the [./include/execute.h](https://github.com/waxodium/turgen/blob/main/include/execute.h) file to call function of the new command. Should starts by the command name, then follow by with the function name calls. (INCLUDE your command logic file into the [./include/execute.h](https://github.com/waxodium/turgen/blob/main/include/execute.h) directly.)


Example:
```c
#ifndef EXECUTE
#define EXECUTE

#include "navigation.h"

typedef int (*Handler)(char **, ShellState *);

typedef struct {
    const char *name;
    Handler func;
} Command;

static Command builtins[] = {
    {"clear", tclear},
    {"cls",   tclear},
    {"exit",  texit},
    {"cd",    cd},
    // New Commands...
};

#endif
```

> [!WARNING]
> If a command is named after another existing command name on any machine. Turgen will prioritize on checking the built-in commands and *entirely* while also *purposely* ignore the conflicting binary or an alias name.

---


