# AI Usage Declaration and Log

| Course     | COMP2113 Programming Technologies (2026–27 Semester 1)                 |
| ---------- | ---------------------------------------------------------------------- |
| Project    | Library Management System                                              |
| Repository | https://github.com/andyyizixuan/COMP2113-library_management_system.git |
| Team       | Liu Xuanyu 3036590958;                                                  |

Each member documents their own module in four parts: (1) the models and tools used, (2) the prompt
history, (3) the integration log, (4) critical reflection. Raw chat exports are in `ai_logs/`; the page
numbers below refer to those PDFs.

---

## A — <!-- Name (UID) --> · `book.h` / `book.cpp`

### A.1 Models and tools

- DeepSeek web chat (chat.deepseek.com), model: the default model served by the web app in 2026-11. 

- Used for: interface design, `Book` class, `book.cpp` implementation, compiler and linker errors.
- Also used for documentation: `README.md` and `LEARNING_MATERIALS.md` were drafted with this tool; every technical claim in them was checked.
- No other AI tools used.

### A.2 Prompt history


1. *(p.1)* “我是A工作的，我需要做什么，具体点告诉我，有需要的信息问我”
   → module boundary and interface list (`addBook`, `findBookById`, `findBookByIdMutable`,
   `searchBooksByTitle`, `handleAddBook`, `handleSearchBook`), `Book` fields `id / title / author /
   total_number / available_number`. Adopted; the suggested ISBN and category fields were dropped. But the functions findBookById, findBookByIdMutable, and searchBooksByTitle have been deprecated. Instead, searchBook_ID and searchBook_title are used ultimately.

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
  `g++ -pedantic-errors -std=c++11 -Wall`.
- Global container renamed `allBooks` → `Booklist` (p.115) after aligning names with B–E.
- During integration, fixed the full-width punctuation in `main.cpp` (E's file) and reported it to E;
  link errors were traced to files missing from the build.
- Transcript: `Liu Xuanyu_AI_Prompt.pdf`, 198 pages.

### A.4 Critical reflection

**Where should "addBook" be placed:** At first, I made a mistake in writing it. Even after the AI pointed it out, I didn't immediately correct it. Initially, I placed the "addBook" function as a member function of the class. However, from p.37 to 40, AI pointed out that for the ID lookup in member functions, it is necessary to traverse the entire Booklist. At that time, I did not accept the proposal and did not make a decision until p.88 to make addBook a free function. In book.cpp:74, searchBook_ID is called - only functions that can see the entire Booklist can do this. Lesson: Do not blindly accept the AI's judgment; one must independently judge whether it is reasonable before starting to make modifications.

*__File review on page 97: What is valuable is the review, not the ghostwriting.__ It examines my book.h screenshot and points out three issues: "using namespace std;" is written in the header file, borrowBook(const string title) receives an additional book title parameter, and a search function and getter that can return a Book* are missing. I have verified and corrected each of these three issues: Now there is no "using namespace std;" in book.h, borrowCopy() does not have parameters, the search function is ultimately called searchBook_ID (in book.h:35), and the getter is in book.h:16–20. The verification method is that both the main.cpp and tests/test_member.cpp translation units include this header file and can be compiled successfully. Lesson: In the scenario of multiple cpp files, using "using namespace std" will pollute all namespaces, leading to unpredictable compilation errors.

**Cross-session naming conflicts:** Looks fine individually, but when combined, it blows up. Initially, book.h declared allBooks, book.cpp defined Booklist, and library_d.cpp used allBooks - three sessions gave different names each. Seen separately, they are all reasonable, but when combined, it results in undefined reference. After modification, it was unified to Booklist (book.h:31 / book.cpp:12), and the problem disappeared. Lesson: It is again emphasized that the output content proposed by AI must undergo review, and the functions called in different files must maintain consistent naming.

**Final compilation issue:** I attempted to compile the Makefile in CLion, but kept receiving errors. The problem of CLion's compilation errors has not been resolved even after multiple attempts of asking the AI for help. Finally, the team members reviewed each other's codes together and found that the errors in the codes prevented them from compiling, which was resolved by correcting the codes and then they could run normally. Lesson: When the AI repeatedly makes unreasonable suggestions, one should change the approach.


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
