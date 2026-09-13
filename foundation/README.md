# Repository Foundation

This directory contains bootstrap material used to verify the laboratory
environment before chapter work begins.

## Smoke Test

Source:

~~~text
foundation/smoke_test.c
~~~

Build and run:

~~~bash
make run
~~~

Generated executable:

~~~text
build/foundation/smoke_test
~~~

The observations/ directory preserves the original evidence collected with
strace. Chapter-specific observations belong inside their chapter directories.
