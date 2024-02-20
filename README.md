# IOS (Operating Systems) Project 2
From the 2nd semester, focused on working with semaphores and synchronization. The task was to write a console app in C which simulates Santa, reindeers, and elves working on Christmas and synchronize them to perform tasks in the correct order. 

Result was 15/15 points, but I know about one issue :)

## Usage:

```
$ ./proj2 NE NR TE TR
```

* NE: Number of elves. 0 < NE < 1000
* NR: Number of reindeers. 0 < NR < 20
* TE: Maximum duration in milliseconds for which an elf works independently. 0 <= TE <= 1000.
* TR: Maximum duration in milliseconds for which a reindeer returns home from vacation. 0 <= TR <= 1000.

All parameters are non-negative integers.