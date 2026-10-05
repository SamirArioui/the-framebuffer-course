# Solution: exercise 1 — Count on it

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 011 — the hashtable: hashing, buckets, lookup](../../lessons/part-0/lesson-011-hashtable.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction, checked against the real run:

```
and 1
bird 1
cat 1
dog 1
don 1
stop 1
t 1
the 3
```

Eight lines, and each one is a consequence of the tokenizer's two quiet
rules. Case folds first: `The` and `AND` count as `the` and `and`, which is
why `the` reaches 3. Punctuation splits: `Don't` is not a word to this
counter — the apostrophe is a separator, so the letters before and after it
become *two* words, `don` and `t`. That is the pair the prompt warned about,
and they show up as ordinary entries with count 1 each. The line-final `.`
and `!` separate the same way and leave nothing behind.

The confirming patch logs each token as the tokenizer emits it:

```
token: the
token: cat
token: and
...
token: don
token: t
```

Ten tokens from nine apparent words — and note the log is `stderr`, so the
counts on `stdout` stay clean and pipeable. Whether `don`/`t` is a defect or
a definition depends on your text: real word counters pre-process
apostrophes exactly because of this. The table itself does not care — to
`HtPut` they are just keys.
