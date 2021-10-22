MULAN
====

The Simplest gettext PO file compiler

## What's this?

MULAN compiles `*.po` file into a simple-formatted binary data.

```
$ cat in
msgid  "Hello World"
msgstr "こんにちは世界"

$ mulan compile out in

$ xxd out
00000000: 6c08 093a 6f57 206f 1500 e381 93e3 8293  l..:oW o........
00000010: e381 abe3 81a1 e381 afe4 b896 e795 8c    ...............
```

The compiled binary is formatted as below.
Please note that MULAN supports only `msgid` and `msgstr`.

```
+---+---+---+---+---+---+---+---+---------+---------+---+---+---+---+---+-----+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |    8    |    9    | A | B | C | D | E | ...
+---+---+---+---+---+---+---+---+---------+---------+---+---+---+---+---+-----+
|       u64 little endian       | u16 little endian |        string...
+-------------------------------+-------------------+-------------------------+
|           msgid hash          |    msgstr size    |        msgstr
+-------------------------------+-------------------+-------------------------+
```

You can see [test](https://github.com/falsycat/mulan/tree/main/test) directory to how this works actually.

## LICENSE

WTFPLv2

## Author

[falsycat](https://fasly.cat/)

<a href='https://ko-fi.com/falsycat' target='_blank'><img height='35' style='border:0px;height:46px;' src='https://az743702.vo.msecnd.net/cdn/kofi3.png?v=0' border='0' alt='Buy Me a Coffee at ko-fi.com' />
