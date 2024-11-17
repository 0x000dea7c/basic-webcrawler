
# Table of Contents

1.  [basic-webcrawler](#orgef853cf)
2.  [Features](#orgd107e7b)
3.  [Dependencies](#org592d7ac)
4.  [Notes](#org8407628)
5.  [Compilation](#org54850bc)


<a id="orgef853cf"></a>

# basic-webcrawler

The goal of this mini project is to learn the basics of multi-threading.


<a id="orgd107e7b"></a>

# Features

-   Respects robots.txt file.
-   Use delays between requests to not be annoying. Default value of 1 second between requests, but prefer delay specified in robots.txt.
-   Multi-threaded.
-   Configurable parameters in `CMakeLists`:
    -   `SEED_URL_1`: specifies initial URL to start crawling.
    -   `DEFAULT_REQUEST_DELAY_`: specifies the delay in seconds between requests to the same domain.
    -   `DEFAULT_DEPTH_LIMIT`: specifies the depth of the current search.
    -   `METADATA_FILENAME`: specifies the name of the file where the program will dump its output on.
-   Avoids doing requests to useless links like pdfs, jpgs, pngs, login/auth pages, embedded javascript, etc.


<a id="org592d7ac"></a>

# Dependencies

-   cmake
-   c++ compiler with c++17 support
-   vcpkg
-   lexbor library (for http parsing)
-   curl library (for http requests)

I still don&rsquo;t know how to use vcpkg effectively so I can&rsquo;t provide a good tutorial on how to do install dependencies using it, but the way I did it was:

1.  Create a directory (parent of this project) called vcpkg.
2.  Install vcpkg there.
3.  Use vcpkg to manually install lexbor and curl.
4.  Then, in `compile.sh`, use this flag: `-DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake`.
5.  Notice that inside `CMakeLists.txt` there are also references to this directory. This is not ideal, but lexbor didn&rsquo;t provide a cmake integration so couldn&rsquo;t do anything about it.


<a id="org8407628"></a>

# Notes

There&rsquo;s a lot of room for improvement. This is just a very basic example of me learning the basics of multi-threading.

I didn&rsquo;t consider serious stuff like load-balancing, lock-free data structures, and so on. There&rsquo;s probably a lot of contention.

Also, because of the focus of this project, I didn&rsquo;t provide enough and serious tests (besides a basic one to know if I&rsquo;m parsing links correctly). Ideally I&rsquo;d want them. It&rsquo;d be nice to also use a lib like GTest for more expressive tests with cmake integration.


<a id="org54850bc"></a>

# Compilation

Run `compile.sh` and then `./build/basic-webcrawler`.

You&rsquo;ll get messages in stderr if there are any errors like timeouts to certain websites, couldn&rsquo;t access website, etc.

For every successful website processed, you&rsquo;ll have an entry on `METADATA_FILENAME`.

