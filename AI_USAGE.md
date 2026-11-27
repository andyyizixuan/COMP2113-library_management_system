# AI Usage Declaration and Log

| Course     | COMP2113 Programming Technologies (2026–27 Semester 1)                 |
| ---------- | ---------------------------------------------------------------------- |
| Project    | Library Management System                                              |
| Repository | https://github.com/andyyizixuan/COMP2113-library_management_system.git |
| Team       | Liu Xuanyu3036590958;                                                  |

Each member documents their own module in four parts: (1) the models and tools used, (2) the prompt
history, (3) the integration log, (4) critical reflection. Raw chat exports are in `ai_logs/`; the page
numbers below refer to those PDFs.

---

## A — <!-- Name (UID) --> · `book.h` / `book.cpp`

### A.1 Models and tools

- DeepSeek web chat (chat.deepseek.com), model <!-- TODO: version shown in the chat -->, used

  DeepSeek-V4.1-Flash

- Used for: interface design, `Book` class, `book.cpp` implementation, compiler and linker errors.

- No other AI tools used.

### A.2 Prompt history

https://chat.deepseek.com/share/eqpd0h9xvhh7f0sxan

1. *(p.1)* “我是A工作的，我需要做什么，具体点告诉我，有需要的信息问我”
   → module boundary and interface list (`addBook`, `findBookById`, `findBookByIdMutable`,
   `searchBooksByTitle`, `handleAddBook`, `handleSearchBook`), `Book` fields `id / title / author /
   total_number / available_number`. Adopted; the suggested ISBN and category fields were dropped.

2. *(p.72)* “所以uniqueptr到底有啥优势，我一个vector书柜有啥书就写在booklist，加书就add去掉就delete…”
   → explained copy/dangling/double-free problems with `vector<Book>`. Kept
   `vector<std::unique_ptr<Book>>`.

3. *(p.88)* “那就听你的写外面”
   → moved `addBook` out of `class Book` into a free function over the global `Booklist`;
   `borrowCopy()` / `returnCopy()` stay members.

4. *(p.97)* header review of my own `book.h` → three fixes: no `using namespace std;` in a header, no
   redundant `title` parameter on `borrowBook`, and add `findBookById` returning `Book*` plus getters
   for C and D.

5. *(p.121)* “欸我h文件里面，class内函数是不是没加调用参数”
   → member functions receive `this`, so `borrowCopy()` takes no book argument; only free functions
   take `Book&`.

6. *(pp.196–197)* pasted compiler output: `unable to find string literal operator 'operator""；'`
   (a full-width Chinese semicolon at `main.cpp:31`) and `undefined reference to 'handleAddBook()'`
   (a source file missing from the build). Both diagnosed.

### A.3 Integration log

- `book.h` committed first on a branch and merged through PR #1 (pp.112–113), so B–E could code
  against it before `book.cpp` existed.
- `book.cpp` written against the final header, compiled with
  `g++ -pedantic-errors -std=c++11 -Wall -Wextra`.
- Global container renamed `allBooks` → `Booklist` (p.115) after aligning names with B–E.
- During integration, fixed the full-width punctuation in `main.cpp` (E's file) and reported it to E;
  link errors were traced to files missing from the build. <!-- TODO: 写下最后实际怎么修的 -->
- Transcript: `ai_logs/A_book_module.pdf`, 198 pages.

### A.4 Critical reflection

**The assistant put `addBook` in the wrong place.** It first made `addBook` a member of `Book`,
which reads naturally (`b1.addBook(...)`) but cannot work: adding a book means scanning the whole
collection for a duplicate ID, and one object cannot see its siblings. I caught this while reasoning
about the data flow (p.88), not from a compiler error — the code would have compiled and silently
accepted duplicates. The fix was free functions for anything that touches the whole collection, and
member functions only for operations on a single book. Lesson: data ownership and operation ownership
are different questions.

**The useful AI output was the review, not the code.** At p.97 it reviewed a screenshot of my header
and named three real defects, including `using namespace std;` in a header and a missing
`findBookById`. I checked each claim rather than pasting the replacement: two translation units
including the header, then a clean compile. <!-- TODO: 补你实际的验证动作和证据 -->

**Two errors that look alike but are not.** The `operator""` error looked like a template problem and
was a Chinese semicolon; the `undefined reference` looked like a code problem and was a build problem.
Both cost time because I trusted the error text before checking the actual cause. I now check encoding
and the build file before editing code. <!-- TODO: 补你实际的修复记录（commit / 重编结果） -->

**What I would do differently:** compile the header standalone before writing `book.cpp`, and test on
the Linux build earlier instead of only in CLion.

---

## B — <!-- Name (UID) --> · `member.h` / `member.cpp`

### B.1 Models and tools

- <!-- TODO: tool, model version, date range -->
- Used for: <!-- TODO -->

### B.2 Prompt history

1. *(p.<!-- -->)* “<!-- verbatim prompt -->”
   → <!-- what it proposed; what you kept or rejected -->
2. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
3. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->

### B.3 Integration log

- <!-- TODO: branch/commit, which interface it exposed to C/D, what broke during integration -->
- Transcript: `ai_logs/B_<!-- -->.pdf`, <!-- n --> pages.

### B.4 Critical reflection

<!-- TODO: one case. What the assistant got wrong or what you had to prove before trusting it;
     how you noticed (compile error, test, teammate's interface mismatch); what you changed. -->

---

## C — <!-- Name (UID) --> · `library.cpp` (borrowing, overdue)

### C.1 Models and tools

- <!-- TODO -->

### C.2 Prompt history

1. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
2. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
3. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->

### C.3 Integration log

- <!-- TODO: 5-book limit, stock check, reservation hand-off; which modules you called -->
- Transcript: `ai_logs/C_<!-- -->.pdf`, <!-- n --> pages.

### C.4 Critical reflection

<!-- TODO: one case, e.g. off-by-one on the borrow limit, or a record left behind after a failed borrow. -->

---

## D — <!-- Name (UID) --> · `library.cpp` (returns) + `saveData` / `loadData`

### D.1 Models and tools

- <!-- TODO -->

### D.2 Prompt history

1. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
2. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
3. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->

### D.3 Integration log

- <!-- TODO: file formats, load on start / save on exit, what happens on a missing or malformed file -->
- Transcript: `ai_logs/D_<!-- -->.pdf`, <!-- n --> pages.

### D.4 Critical reflection

<!-- TODO: one case, e.g. a malformed line in books.txt, or a missing file on first run. -->

---

## E — <!-- Name (UID) --> · `main.cpp`, `Makefile`, utilities

### E.1 Models and tools

- <!-- TODO -->

### E.2 Prompt history

1. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
2. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->
3. *(p.<!-- -->)* “<!-- -->”
   → <!-- -->

### E.3 Integration log

- <!-- TODO: menu loop, input validation, calcDayDiff/isOverdue/calculateFine, link errors fixed -->
- Transcript: `ai_logs/E_<!-- -->.pdf`, <!-- n --> pages.

### E.4 Critical reflection

<!-- TODO: one case, e.g. invalid menu input looping forever, or a compile error caused by another
     module's header. -->
