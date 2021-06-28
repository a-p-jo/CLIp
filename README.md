# CLIp v2.1

### About

**CLIp  is a powerful yet easy to use and minimal clipboard manager for a command line environment, with no dependencies or bloat.**

### Usage

**Syntax :** 

` clip [operation] [clipboard name (optional)] [clipboard name (optional)]...`

**Operations :**

1. `c` _or_ `copy`   : Copy stdin to clipboard(s)
2. `a` _or_ `append` : Append stdin to clipboard(s)
3. `p` _or_ `paste`  : Paste clipboard(s) to stdout
4. `r` _or_ `remove` : Remove clipboard(s)

**Brief :**

- CLIp reads from _stdin_ when copying / appending and writes to _stdout_ when pasting, so you'll use shell redirection operators.

- It takes arguments for the _operation_ and , optionally, clipboard name(s).

- Clipboards are files on disk, so they have **_no limitations_** of type of data stored or filesize in and of themselves.

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
