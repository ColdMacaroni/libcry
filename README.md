# LibCry

A neat c testing library/framework. 🥲

## Roadmap

- [ ] For 1.0
  - [ ] Forking test runner
  - [ ] Send message back to runner from test process
  - [ ] Prettier asserts
  - [ ] Some man pages.. [see meson](https://mesonbuild.com/Installing.html)
  - [ ] Test anything protocol implementation

In the future.. idk... threading?

## Why the name?

I tried to use some testing libraries and they made me want to cry.

## How do I use it?

Write a test file like:

```c
#include "cry.h"
#include <stdbool.h>

test(add_to_two, "Checks that 1 + 1 = 2") {
    cry_assert(1 + 1 == 2);
}

bool returns_true() {
    return true;
}

test(check_true, "Ensure returns_true returns true") {
    cry_assert(returns_true());
}
```

You can add more if you want, check the macros in `cry.h`. Don't include a `main` function.

Then, assuming `libcry.a` is in library path compile like

```sh
gcc yourtest.c -lcry -rdynamic -o yourtest
```

**NOTE:** It's **very** important that you compile with the **-rdynamic** flag. It won't discover the tests otherwise.

Now you can run the tests like `./yourtest`. The output executable runs the tests all by itself !! wow !!

In the future these will have more functionality.. stay tuned.. or add it yourself.

## Contributing

Yes ! please ! please ! please !

Feel free to open an issue, a PR, send an issue to my email, or even send a git
patch to my email ! (see `git format-patch` and `git send-email`). They are all
welcome and I'll try to get back to you in time.

You can just build normally with meson, nothing fancy. **Important:** Running
with a debugger is weird. It'll grab the wrong symbols. Make sure to run gdb or
whatever with a specific coredump to make it work.

## It threw an error at me

For most of the assumptions I've made, I left an error message about it so
please include that in the bug report or whatever. Hopefully it can be fixed
:'). If it just segfaults out of nowhere make sure to

## are you ok

i spent like 8 hours trying to read the symbols and i've barely slept. but hey it works :)
