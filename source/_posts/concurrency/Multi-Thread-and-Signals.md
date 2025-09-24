---
title: Multi Thread and Signals
date: 2024-03-22 15:20:14
tags:
 - thread
 - signal
---

From ChatGPT:

In general, when a program enters a signal handler in a multi-threaded environment, the behavior regarding other threads depends on how the signal handler is set up and the specific signal that is being handled.

1. Default Behavior: By default, when a signal is delivered to a process, it interrupt the thread that is currently running and executes the signal handler in the context of that thread. Other threads in the process continue running unless they are also interrupted by signals.
2. Thread-Specific Signal Handling: Some signals, such as SIGINT (interrupt signal), SIGTERM (termination signal), or SIGABRT (abort signal), are typically delivered to the entire process, which means they can interrupt any thread. However, other signals, like SIGSEGV (segmentation fault) or SIGILL (illegal signal), are usually delivered to the specific thread that caused the signal.
3. Signal Masking: In a multi-threaded program, you can use signal masking (sigprocmask in POSIX systems) to block certain signals in specific threads. This can affect whether a signal handler interrupts a particular thread or not.
4. Asynchronous-Signal-Safe Functions: Signal handlers should only execute functions are considered "asynchronous-signal-safe" according to POSIX standards. These functions are designed to be safe to call from within a signal handler. Using non-safe functions in a signal handler can lead to undefined behavior.