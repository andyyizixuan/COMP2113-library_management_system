# Library Management System

**COMP2113 Programming Technologies — Group Project (2026–2027, Semester 1)**

A menu-driven C++11 library system for managing a book catalogue, members, loans, overdue
fines, and per-book reservation queues. The full state can be saved to plain-text snapshots
and restored later. The project is split across four headers and six `.cpp` files, and every
`Book` is owned by a `std::unique_ptr` — the group's self-learned non-syllabus topic.

---

## 1. Team Members

<!-- TODO: fill in names and student IDs before submission. -->

| Name       | Student ID |
| ---------- | ---------- |
| Liu Xuanyu | 3036590958 |
|            |            |
|            |            |
|            |            |
|            |            |

---

## 2. Overview

### 2.1 Features

| Menu | Feature         | Behaviour                                                                                                                    |
|:----:| --------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| 1    | Add Book        | Reads ID, title, author, total copies; rejects empty fields, non-positive totals, duplicate IDs.                             |
| 2    | Search Book     | Exact ID search, or title substring search; prints full book details.                                                        |
| 3    | Register Member | Unique, whitespace-free member ID and non-blank name.                                                                        |
| 4    | Borrow Book     | Validates member/book, enforces the 5-book limit, decrements stock, records the loan; queues the member if no copy is free.  |
| 5    | Return Book     | Finds the loan, computes overdue days/fine, restores stock, removes the loan, and serves the next reservation automatically. |
| 6    | View Overdue    | Lists every loan past 14 days with member ID, book ID, overdue days, and fine.                                               |
| 7    | Save Data       | Validates state and writes a new versioned snapshot with a `.bak` backup.                                                    |
| 8    | Load Data       | Validates and loads the primary snapshot, falling back to backup files if needed.                                            |
| 0    | Exit            | Leaves the menu; save first with option 7.                                                                                   |

**Rules:** 14-day loan period · 2 HKD/day fine after day 14 · maximum 5 concurrent loans per
member · FIFO reservation queue per book · data stored in `books.txt` and `members.txt`.

### 2.2 Build and Run

1. Clone the repository and enter its root directory.

2. Build:

   ```bash
   make clean
   make
   ```

3. (Optional) Run the unit tests:

   ```bash
   make test
   ```

4. Run from the repository root:

   ```bash
   ./main
   ```

5. For the first run, simply add the books → Select 7 and save (at this point, there are no snapshots on the disk, so saving is allowed); if you already have books.txt/members.txt, first select 8 and then make the change.

6. Choose **7 (Save Data)** before exiting; the program does not autosave.

Manual build if `make` is unavailable:

```bash
g++ -pedantic-errors -std=c++11 *.cpp -o main
```

**Makefile targets:** `make` / `make all` (build), `make test` (build and run unit tests),
`make clean`, `make tar`.

---

## 3. Repository Layout

| File                      | Responsibility                                                               |
| ------------------------- | ---------------------------------------------------------------------------- |
| `main.cpp`                | Menu loop, input validation, dispatch to feature handlers.                   |
| `book.h` / `book.cpp`     | `Book` class, `Booklist`, add book, ID/title search, add/search handlers.    |
| `member.h` / `member.cpp` | `Member`, `BorrowRecord`, registration, member lookup, reservation queues.   |
| `library.h`               | Public interface for borrow, return, overdue, save, load.                    |
| `library.cpp`             | Borrowing logic and overdue report.                                          |
| `library_d.cpp`           | Returning, reservation allocation on return, save/load, snapshot validation. |
| `helper.h` / `helper.cpp` | Day difference, overdue test, fine calculation.                              |
| `Makefile`                | Build, test, clean, and archive targets.                                     |
| `tests/test_member.cpp`   | 25 deterministic unit tests for member/reservation logic.                    |

**Module ownership:** A — book catalogue (`book.h`, `book.cpp`); B — members and reservations;
C — borrowing and overdue; D — returns and persistence; E — menu, Makefile, time helpers.

---

## 4. Mandatory Requirements — Where and How

| Requirement          | Implementation                                                                                                                                                                                      |
| -------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Structured data      | `std::vector` for books, members, loans; `std::map<std::string, std::queue<std::string>>` for reservations; duplicate checks by linear scan over those containers.                                  |
| Dynamic memory       | Each `Book` is allocated with `new` and immediately adopted by `std::unique_ptr`; catalogue is `std::vector<std::unique_ptr<Book>>`; no manual `delete`; ownership moves with `std::move` / `swap`. |
| File I/O             | Versioned snapshots in `books.txt` and `members.txt`, written via `.tmp` files, renamed into place, with `.bak` backups and validation/recovery.                                                    |
| Multi-file structure | Four headers + six source files; declarations separated from definitions; separate object files linked by `make`.                                                                                   |
| Non-syllabus topic   | Smart pointers, RAII, and move semantics (`std::unique_ptr`, `std::move`, `swap`).                                                                                                                  |
| Modular functions    | One handler per menu action; business logic in module functions; validation helpers in anonymous namespaces.                                                                                        |

### 4.1 Data Structures

* `std::vector<std::unique_ptr<Book>> Booklist` — catalogue; stable heap addresses for each
  `Book` even when the vector grows.
* `std::vector<Member> allMembers` and `std::vector<BorrowRecord> borrowRecords` — members and
  active loans, scanned linearly (the dataset is small).
* `std::map<std::string, std::queue<std::string>> reservations` — one FIFO queue per book ID,
  managed by `addReservation()`, `getNextReserve()`, `removeNextReservation()`, and
  `printReservations()`.
* `std::map` / `std::queue` in `library_d.cpp` — duplicate-ID, duplicate-loan, and
  duplicate-reservation checks while loading.

### 4.2 Dynamic Memory and Ownership

`book.h` declares the catalogue; `book.cpp` defines it once:

```cpp
extern std::vector<std::unique_ptr<Book>> Booklist;   // book.h
std::vector<std::unique_ptr<Book>> Booklist;          // book.cpp
```

A book is created by immediately handing the `new` result to a `unique_ptr`:

```cpp
Booklist.push_back(std::unique_ptr<Book>(
    new Book(id, title, author, total_number, total_number)));
```

If `push_back` throws, the temporary `unique_ptr` destroys the `Book`, so there is no leak.
Search returns non-owning observers (`Book*`), never a second owner. The loader builds a
`Snapshot` of `vector<unique_ptr<Book>>`, moves each parsed book into it, and commits the
catalogue with `Booklist.swap(snapshot.books)` only after full validation.

### 4.3 File I/O and Persistence

`books.txt` stores the catalogue, availability, and each book's reservation queue.
`members.txt` stores members and their active loans (book ID + borrow timestamp). Both files
start with a magic header and version, then a count, then records, then `END`; the two versions
must match on load.

Saving is crash-aware:

1. Validate all in-memory invariants.
2. Refuse to overwrite a snapshot that changed on disk since loading.
3. Write `books.txt.tmp` and `members.txt.tmp`.
4. Move the current files to `.bak`, then rename the `.tmp` files into place.
5. Increment the snapshot version.

Loading tries the primary pair, then backup combinations (both `.bak`, or one primary plus one
`.bak`). If a backup is used, saving is blocked until the recovered files are restored to the
primary names and the program restarted. Saving is also blocked when snapshot files exist but
were never loaded, so an empty in-memory library cannot destroy real data.

### 4.4 Multi-File Architecture

Headers contain class declarations, prototypes, `extern` declarations, and include guards.
Each `.cpp` implements one module; `main.cpp` only routes menu choices. The Makefile compiles
each module with `g++ -pedantic-errors -std=c++11 -g -Wall` and links the objects, so changing
one module recompiles only that module.

### 4.5 Non-Syllabus Topic

Smart pointers and RAII, with `std::unique_ptr` as the sole owner of each book and
`std::move` / `swap` for ownership transfer. This keeps the catalogue leak-free, makes
double deletion a compile-time error, and gives search results stable addresses. See
[`LEARNING_MATERIALS.md`](LEARNING_MATERIALS.md) for the full explanation and code walkthrough.

### 4.6 Modular Functions

`main()` reads the menu choice and calls one handler (`handleAddBook`, `handleSearchBook`,
`registerMember`, `borrowBook`, `returnBook`, `viewOverdue`, `saveData`, `loadData`). Each
handler owns its prompts; business rules and validation live in the module functions. Failed
numeric input clears and drains `std::cin` so the menu loop cannot spin.

---

## 5. Third-Party and Header-Only Libraries

**None.** The project uses only the C++11 standard library and has no external headers,
packages, or root-installed dependencies.

---

## 6. Testing

`make test` builds and runs `tests/test_member.cpp`, containing **25 deterministic tests** for:

* registration success, duplicate IDs, blank/whitespace/space-containing IDs;
* full-name input after a menu choice, duplicate console input, EOF handling;
* exact-ID member lookup;
* reservation add/reject, unknown book/member, duplicate queue entries;
* FIFO order, independent queues per book, peek/remove semantics, empty queues;
* printing queue positions without mutating state.

Each test resets global member/loan/reservation state and reports `PASS` or `FAIL`; the
executable returns non-zero on failure.

Suggested manual checks: add/search a book; register a duplicate member; borrow five books and
a sixth; borrow the last copy as a second member to trigger a reservation; return it and watch
the queue serve the next member; save, restart, load; corrupt `books.txt` and recover from
`.bak`; try saving without loading an existing snapshot.

---

## 7. Save-File Format

Fields are one per line. `#` comments below are explanatory, not written to disk.

```text
books.txt:
  LMS_BOOKS_V1
  <version>
  <book count>
  per book:
    <book ID>
    <title>
    <author>
    <total copies>
    <available copies>
    <queue length>
    <member ID>        # repeated queue-length times, head first
  END

members.txt:
  LMS_MEMBERS_V1
  <version>
  <member count>
  per member:
    <member ID>
    <name>
    <active loan count>
    per loan:
      <book ID>
      <borrow unix time>
  END
```

On load, the program rejects duplicate IDs, unknown book/member references, inventory that
does not satisfy `available + active loans == total`, more than five loans per member, and
duplicate or inconsistent queue entries.

---

## 8. Video Demonstration

> **Video link:** _TODO — add the YouTube / Panopto / Google Drive / OneDrive / Bilibili URL
> before submission._

The two-minute video should show adding a book, registering a member, borrowing and returning,
a reservation being served, an overdue record, and a save/load cycle.

---

## 9. Known Limitations

* Single-user console program; no concurrency control or database.
* Data files are opened relative to the current working directory.
* Persistence is explicit — no autosave.
* Title search is case-sensitive; an empty keyword matches every book.
* Fines are calculated and displayed but no payment is recorded.
* `Book*` results from search are non-owning and invalid after `loadData()`.
* The save validator treats duplicate `(member, book)` loans as invalid state.
* Code stays within C++11, so `std::make_unique` (C++14) is not used.
