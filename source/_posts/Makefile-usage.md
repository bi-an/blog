---
title: Makefile usage
date: 2023-11-17 14:19:35
categories: Programming
tags: makefile
---

## "make run" argument:

https://stackoverflow.com/questions/2214575/passing-arguments-to-make-run

"@:"
https://unix.stackexchange.com/questions/92978/what-does-this-2-mean-in-shell-scripting

This works fine for me:

```makefile
# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  # TODO: What does the following line mean?
  # $(eval $(RUN_ARGS):;@:)
endif

# "cmd" refers to any command
run:
	cmd $(RUN_ARGS)
```
