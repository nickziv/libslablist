What Is a Slab List?
--------------------

Slab lists are a memory efficient data structure, that can optionally keep data
sorted in logarithmic time.


What Is `libslablist`?
----------------------

`libslablist` is a C library that that implements slab lists. The _concept_ of
the Slab List is not tied to any particular language, just as the _concept_ of
an AVL Tree is can be implemented in any Turing-complete langauge.

Why Slab Lists?
---------------

I designed slab lists with quality and performance in mind. I had a lot of code
using ad-hoc linked lists and pre-canned AVL Trees at the same time. While
these are good solutions to well-known problems, I was tired of (a) using two
different structures for the same data and (b) not being able to hold all of my
data in memory -- I had to search it by chunks. So slab lists address both of
those problems. They can be used for sorted and unsorted data, which allows us
to use the same umem caches for both. They are also 4x to 5x more memory
efficient than AVL Trees, allowing one to reduce the number of times one goes
to disk.

Where Can I Use `libslablist`?
------------------------------

You can use this library from any C program that runs on Linux or Illumos.
Support for FreeBSD is forthcoming. If you want a rock-solid data data
structure that can implement a sorted set or even a simple sequential list,
`libslablist` is more than up to the task. A lot of useful things can be
implemented with sorted sets.

Where Is `libslablist` Used?
----------------------------

`libslablist` is reported to be used by developers in multiple stealth
projects, to great effect. Publicly, however, `libslablist` is used in another
project called `libgraph`. See: `https://github.com/nickziv/libgraph`

`libgraph` is a library that manipulates graphs (in the graph-theory sense).
`libgraph` uses `libslablist` as the backend for implementing a graph as a list
of edges. Typically, a graph is implemented using a matrix or adjacency list to
represent the edges. Using `libslablist` has the advantage of separating the
data (the nodes) from the container (the edges connecting the nodes). Which
means the same nodes can be present in multiple graphs, for example.

Where Can I Learn More?
-----------------------

Check out the development blog for historical or technical information:
http://slablist.wordpress.com

See the block comments peppered throughout `src/slablist_impl.h` for an
implementation overview. Trust me, you won't understand what's going on if you
don't read those comments.

How Is The Code Organized?
--------------------------

The actual C implementation of the slab list is in the src/ directory. 

All of the code in that directory follows the Illumos kernel coding style,
sometimes referred to as BJNF (Bill Joy Normal Form).

Here is an explanation of what each file does.

* `slablist_impl.h`: Common structures, constants, as well as detailed
documentation of the implementation.

* `slablist.h`: The consumer-facing function declarations and constants.

* `slablist_add.c`: The element insertion routines.

* `slablist_rem.c`: The element removal routines.

* `slablist_umem.c`: The memory allocation routines.

* `slablist_find.c`: The search routines. Everything for searching slabs,
subslabs, and slablists.

* `slablist_cons.c`: Slablist creation, destruction, reaping routines.
Also, linking routines for slabs, subslabs, and `small_lists`. Also, sublayer
attach/detach routines. Routines for converting between singly-linked-lists and
slab lists. Finally foldr, foldl, and map routines, as well as their ranged
variants. Basically, a dumping ground for everything that doesn't fit in
`_add`, `_rem`, or `_umem source files`.

* `slablist_test.c`: A large collection of testing routines. These
routines sanity check the state of the slablist. For example, it checks that
the number of elements in a slab never exceeds the maximum, and that a removal
doesn't leave a gap in the slab, and so forth.  Basically, these tests are like
very powerful and very expensive `ASSERT`s. They are very expensive to run all
the time, so they are wrapped in DTrace `IS_ENABLED()` probes. This means they
won't be run unless we enable specific DTrace probes at run time. This means
that they are _always_ present but remain dormant until activated by someone.
For this reason there is no `DEBUG` conditional-macro in the code. We ship with
all of our probes and test functions, because they have negligible costs when
disabled.

* `slablist_provider.d`: A DTrace source file that defines the DTrace
probes provided by the library.

* `slablist_provider.h`: A header file generated from the
`slablist_provider.d` file. It has to live in the tree, to facilitate
compilation on systems where the `dtrace` command may not be present (such as
Linux).

The `tools/` directory contains tools that can be used in development, or that
are used by the benchmarking code. The `tests/` directory contains D scripts
that trigger the test-code (see section below for more details). The `bench/`
directory contains code that generates R code that can be used to process data
generated by the DTrace durint the running of the benchmarks. There is also a
script that fires off R jobs, to generate plots in parallel (serial is too slow
-- took 45 minutes to generate all plots, now takes 5). The `build/` directory
contains subdirectories that correspond to different operating systems, as well
as a `Makefile.master` file which contains common variables that are used by
makefiles within the OS-subdirectories. The `build/$OS` directory contains a
makefile that can build the library and -- if DTrace is supported -- run
benchmarks and generate plots.


How Stable Is The API?
----------------------

The `libslablist` API is not yet stable. It is volatile and may change.


How Can I Install `libslablist`?
--------------------------------

To install:

	cd build/illumos
	make libslablist.so.1
	make install

How Does `libslablist` Perform Compared to `$X`?
------------------------------------------------

Here are some comparisons, so that you can get an idea of how Slab Lists
perform. First we will compare to `uuavl`. `uuavl` is the AVL Tree
implementation used in the Illumos kernel. It is among the most
memory-efficient and cpu-efficient implementations in the world.

`uuavl` uses 10% _less_ time than `libslablist` on sequential input.

`uuavl` uses 365% _more_ memory than `libslablist` on sequential input.

`uuavl` uses 7% _less_ time than `libslablist` on random input.

`uuavl` uses 271% _more_ memory than `libslablist` on random input.

You'll find a full report here:
`http://nickziv.files.wordpress.com/2014/02/vis_ds.pdf`

Other AVL Tree implementations, such as those found in GNU libavl use 50% to
100% more memory than `uuavl`. Which makes them half as competitive as `uuavl`.
GNU libavl implementations also perform as well as `libslablist`.

We have a bunch of foreign data structure implementations that we use to
evaluate Slab List performance. Most of these are self-contained and can be
built from this tree. Others are simply too large to be included, or were
designed as shared objects. These include: libuutil, libredblack, and myskl.
You'll have to fetch them and install them manually.

How Do I Run Those DTrace Tests?
--------------------------------

The tests have to be activated using code written in DTrace's D language
through the `dtrace` utility. We have some examples in the tests/ directory and
they can be executed like so:

	dtrace -c 'build/drv_gen ...' -L <libpath> -s tests/<script-name>.d

By default, libslablist uses the following library path for DTrace libraries:
/opt/libslablist/include/dtrace.
