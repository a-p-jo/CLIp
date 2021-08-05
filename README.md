# CLIp v2.1.1

### About

**CLIp  is a powerful yet easy to use and minimal clipboard manager for a command line environment, with no dependencies or bloat.**

Key Features : If you like these, CLIp is for you !
- Tiny code size ( ~170 lines) and correspondingly an almost negligible level of resource consumption.
- Not a perpetually-running daemon that grows slower and larger over time.  CLIp runs when called, does what it's told and exits.
- No malloc calls whatsoever, and correspondingly rock-stable with no memory management bugs.
- Infinitely many clipboards, with as much data that your disk can hold, persistent through crashes/shutdowns.
- Clipboards are files, exposing file manipulation abilities to the clipboard.
- Clipboard data is forrmatted *exactly* as you copied it. CLIp doesn't see strange glyphs , it only sees bits & bytes.

Drawbacks : If these are deal-breakers, CLIp isn't it for you :(
- No interface with system clipboard. No plans to integrate support for this as it would considerably bloat the program, and there's plenty of those out there.
CLIp was also designed to be used in a text-only UI, where this isn't essential.
- You'll have to delete clipboards yourself when done with them, using `clip r <clipboard_name>` or simply `clip r` for the default clipboard.
If you don't, they'll keep sitting on disk with the data you put in them.
- No GUI/right-click. Not system-wide, across all windows. But then, it was never designed for that, there's probably plenty of great tools for that out there. It's also a *lot* more powerful than right clicking all the time.

### Usage

**Syntax :** 

` clip <operation> [clipboard name (optional)] [clipboard name (optional)]...`

**Operations :**

1. `c` _or_ `copy`   : Copy stdin to clipboard(s)
2. `a` _or_ `append` : Append stdin to clipboard(s)
3. `p` _or_ `paste`  : Paste clipboard(s) to stdout
4. `r` _or_ `remove` : Remove clipboard(s)

**Brief :**

- CLIp reads from _stdin_ when copying / appending and writes to _stdout_ when pasting, so you'll use shell redirection operators.

- It takes arguments for the _operation_ and , optionally, clipboard name(s).

- Clipboards are files on disk.

- It can create and save to named clipboards, and when given no names or given an empty argument (`""`) , it saves to the _default clipboard_ , "`.clipboard.clip`" .
- Clipboards can be copied to (_overwritten_ with new data), appended to (new data _added_ at the end) , pasted from (write contents to stdout) and removed (file deleted).

**Examples :**
```
dmesg | clip c                 # Copy output of dmesg to default clipboard in current working directory (cwd)
lsblk | clip a "" drives       # Append output of lsblk to default clipboard and clipboard named drives in cwd
clip p | gzip > logs.zip       # Paste default clipboard in cwd to gzip
clip p drives > drive_log.txt  # Pipe clipboard named drives in cwd to drive_log.txt 
clip r "" drives	       # Remove the default clipboard and the clipboard named drives in the cwd
```
