# INI-parser
A simple C program for parsing .ini files.
# Usage
To parse the file and search for a specific key use those arguments:
```
path-to-ini.ini section.key
```
The program will return the values of this key.

---
It also supports simple arithmetic operations on numbers, and the concatenation operation on strings (`+`).
To perform those operation on values from the .ini file use those arguments:
```
path-to-ini.ini "section1.key1 + section2.key2"
```
Supported operations: `+`, `-`, `*`, `/`. 
