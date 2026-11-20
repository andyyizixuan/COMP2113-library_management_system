#include "library.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>

namespace {
// Bound every input line, even when reading a malformed file.
bool readLine(std::istream& in, std::string& line) {
    line.clear();
    char c;
    while (in.get(c)) {
        if (c == '\n') {
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            return true;
        }
        if (line.size() >= 4096) return false;
        line += c;
    }
    return false; // Every saved field, including END, must end with a newline.
}
bool readNumber(std::istream& in, long long minimum, long long maximum, long long& value) {
    std::string line;
    return readLine(in, line) && parseInteger(line, minimum, maximum, value);
}
bool readText(std::istream& in, std::string& text, bool isID) {
    if (!readLine(in, text)) return false;
    return isID ? validID(text) : validText(text);
}
bool readEnd(std::istream& in) {
    std::string line;
    if (!readLine(in, line) || line != "END") return false;
    char extra;
    if (in.get(extra)) return false;
    return in.eof() && !in.bad();
}
// 0 = absent, 1 = readable, -1 = cannot establish absence safely.
// ENOENT is supplied by the target macOS/Ubuntu C runtime; other errors are not absence.
int fileState(const std::string& filename) {
    errno = 0;
    std::FILE* file = std::fopen(filename.c_str(), "rb");
    if (file != NULL) { std::fclose(file); return 1; }
    if (errno == ENOENT) return 0;
    return -1;
}
bool removeOldBackup(const std::string& filename, std::string& error) {
    if (fileState(filename) == 0) return true;
    // Do not remove a directory or an unreadable path mistaken for a backup file.
    std::ifstream probe(filename.c_str(), std::ios::binary);
    char first;
    if (!probe.get(first)) { error = "Cannot read old backup: " + filename; return false; }
    probe.close();
    if (std::remove(filename.c_str()) != 0) {
        error = "Cannot remove old backup: " + filename + ": " + std::strerror(errno);
        return false;
    }
    return true;
}
bool renameFile(const std::string& from, const std::string& to, std::string& error) {
    if (std::rename(from.c_str(), to.c_str()) == 0) return true;
    error = "Rename failed: " + from + " -> " + to + ": " + std::strerror(errno);
    return false;
}
bool closeOutput(std::ofstream& out, const std::string& filename, std::string& error) {
    out.flush();
    bool written = static_cast<bool>(out);
    out.close();
    if (!written || out.fail()) {
        error = "Write/flush/close failed: " + filename;
        return false;
    }
    return true;
}
}

std::string Library::path(const std::string& filename) const { return dataDirectory + "/" + filename; }

bool Library::canSave() const { return saveAllowed; }

void Library::allocateReservations(Timestamp now, std::ostream& out) {
    // Book insertion order resolves competition for a member's last free loan slot.
    for (std::size_t i = 0; i < allBooks.size(); ++i) {
        Book& book = *allBooks[i];
        while (book.availableCopies > 0 && !book.reservations.empty()) {
            std::string memberID = book.reservations.front();
            Member* member = mutableMember(memberID);
            if (member == NULL || member->hasBook(book.id)) {
                // Unreachable through validated public operations; never drop silently.
                out << "Removed stale reservation: " << memberID << " / " << book.id << '\n';
                book.reservations.pop();
                continue;
            }
            if (member->loans.size() >= MAX_LOANS) {
                out << "Queue held for " << memberID << " / " << book.id << ": loan limit reached.\n";
                break; // Preserve FIFO and avoid an infinite loop.
            }
            member->loans.push_back(BorrowRecord(book.id, now));
            --book.availableCopies;
            book.reservations.pop();
            out << "Automatically borrowed: " << book.id << " -> " << memberID
                << "; due " << formatTime(now + LOAN_PERIOD) << '\n';
        }
    }
}

// D: one timestamp is used for the fine and every reservation assigned by this return.
bool Library::returnBook(const std::string& memberID, const std::string& bookID,
                         Timestamp now, std::ostream& out) {
    Member* member = mutableMember(memberID);
    Book* book = mutableBook(bookID);
    if (member == NULL || book == NULL) { out << "Unknown book or member.\n"; return false; }
    std::size_t index = 0;
    while (index < member->loans.size() && member->loans[index].bookID != bookID) ++index;
    if (index == member->loans.size()) { out << "Member has not borrowed this book.\n"; return false; }
    Timestamp borrowed = member->loans[index].borrowTime;
    if (!validTime(now) || now < borrowed || book->availableCopies >= book->totalCopies) {
        out << "Invalid return time or inconsistent inventory.\n"; return false;
    }
    long long days = overdueDays(borrowed, now);
    long long fine = calculateFine(borrowed, now);
    // erase invalidates positions at/after index; no old loan references are used below.
    member->loans.erase(member->loans.begin() + index);
    ++book->availableCopies;
    out << "Returned: " << bookID << "; overdue " << days << " day(s); fine HKD " << fine
        << " (quote only; no payment recorded).\n";
    // Returning any book may free a slot for a reservation on another book.
    allocateReservations(now, out);
    return true;
}

)bool Library::validate(std::string& error) const {
    if (allBooks.size() > MAX_ENTITIES || members.size() > MAX_ENTITIES ||
        bookIndex.size() != allBooks.size() || reservationCount() > MAX_RESERVATIONS) {
        error = "Container count or index mismatch."; return false;
    }
    std::map<std::string, int> borrowedCounts;
    for (std::map<std::string, Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
        const Member& member = it->second;
        if (member.id != it->first || !validID(member.id) || !validText(member.name) || member.loans.size() > MAX_LOANS) {
            error = "Invalid member fields or loan count."; return false;
        }
        std::map<std::string, bool> seen;
        for (std::size_t j = 0; j < member.loans.size(); ++j) {
            const BorrowRecord& record = member.loans[j];
            if (findBook(record.bookID) == NULL || !validTime(record.borrowTime) || seen.count(record.bookID) != 0) {
                error = "Invalid, duplicate or dangling loan."; return false;
            }
            seen[record.bookID] = true;
            ++borrowedCounts[record.bookID];
        }
    }
    for (std::size_t i = 0; i < allBooks.size(); ++i) {
        const Book& book = *allBooks[i];
        std::map<std::string, std::size_t>::const_iterator index = bookIndex.find(book.id);
        if (!validID(book.id) || !validText(book.title) || !validText(book.author) ||
            index == bookIndex.end() || index->second != i || book.totalCopies < 1 ||
            book.totalCopies > MAX_ENTITIES || book.availableCopies < 0 ||
            book.availableCopies + borrowedCounts[book.id] != book.totalCopies) {
            error = "Invalid book fields, index or inventory/loan balance."; return false;
        }
        std::queue<std::string> copy = book.reservations;
        std::map<std::string, bool> seen;
        while (!copy.empty()) {
            const Member* member = findMember(copy.front());
            if (member == NULL || member->hasBook(book.id) || seen.count(copy.front()) != 0) {
                error = "Invalid, duplicate or dangling reservation."; return false;
            }
            seen[copy.front()] = true;
            copy.pop();
        }
    }
    return true;
}

// Parse into a temporary Library only. The caller commits using container swaps.
bool Library::readPair(const std::string& booksPath, const std::string& membersPath, std::string& error) {
    std::ifstream books(booksPath.c_str(), std::ios::binary);
    std::ifstream people(membersPath.c_str(), std::ios::binary);
    if (!books.is_open()) { error = "Cannot open " + booksPath; return false; }
    if (!people.is_open()) { error = "Cannot open " + membersPath; return false; }
    std::string line;
    long long bookVersion, memberVersion, count;
    error = "Invalid header/count in " + booksPath;
    if (!readLine(books, line) || line != "LMS_BOOKS_V1" ||
        !readNumber(books, 1, 1000000000, bookVersion) || !readNumber(books, 0, MAX_ENTITIES, count)) return false;
    long long totalReservations = 0;
    for (long long i = 0; i < count; ++i) {
        std::string id, title, author;
        long long total, available, queueSize;
        error = "Invalid/truncated/duplicate book record in " + booksPath;
        if (!readText(books, id, true) || !readText(books, title, false) || !readText(books, author, false) ||
            !readNumber(books, 1, MAX_ENTITIES, total) || !readNumber(books, 0, total, available) ||
            !readNumber(books, 0, MAX_ENTITIES, queueSize) || bookIndex.count(id) != 0) return false;
        totalReservations += queueSize;
        if (totalReservations > MAX_RESERVATIONS) { error = "Too many reservations in " + booksPath; return false; }
        std::unique_ptr<Book> book(new Book(id, title, author, static_cast<int>(total)));
        book->availableCopies = static_cast<int>(available);
        for (long long j = 0; j < queueSize; ++j) {
            std::string memberID;
            if (!readText(books, memberID, true)) return false;
            book->reservations.push(memberID);
        }
        allBooks.push_back(std::move(book));
        bookIndex[id] = allBooks.size() - 1;
    }
    error = "Missing END or trailing data in " + booksPath;
    if (!readEnd(books)) return false;
    error = "Invalid header/count or snapshot mismatch in " + membersPath;
    if (!readLine(people, line) || line != "LMS_MEMBERS_V1" ||
        !readNumber(people, 1, 1000000000, memberVersion) || memberVersion != bookVersion ||
        !readNumber(people, 0, MAX_ENTITIES, count)) return false;
    for (long long i = 0; i < count; ++i) {
        std::string id, name;
        long long loanCount;
        error = "Invalid/truncated/duplicate member record in " + membersPath;
        if (!readText(people, id, true) || !readText(people, name, false) ||
            !readNumber(people, 0, MAX_LOANS, loanCount) || members.count(id) != 0) return false;
        Member member(id, name);
        for (long long j = 0; j < loanCount; ++j) {
            std::string bookID;
            long long time;
            if (!readText(people, bookID, true) || !readNumber(people, 0, MAX_TIMESTAMP, time)) return false;
            member.loans.push_back(BorrowRecord(bookID, time));
        }
        members.insert(std::make_pair(id, member));
    }
    error = "Missing END or trailing data in " + membersPath;
    if (!readEnd(people) || !validate(error)) return false;
    generation = bookVersion;
    return true;
}

bool Library::writePair(const std::string& booksPath, const std::string& membersPath,
                        long long version, std::string& error) const {
    std::ofstream books(booksPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!books.is_open()) { error = "Cannot open for writing: " + booksPath; return false; }
    books << "LMS_BOOKS_V1\n" << version << '\n' << allBooks.size() << '\n';
    for (std::size_t i = 0; i < allBooks.size(); ++i) {
        const Book& book = *allBooks[i];
        books << book.id << '\n' << book.title << '\n' << book.author << '\n'
              << book.totalCopies << '\n' << book.availableCopies << '\n' << book.reservations.size() << '\n';
        std::queue<std::string> copy = book.reservations;
        while (!copy.empty()) { books << copy.front() << '\n'; copy.pop(); }
    }
    books << "END\n";
    if (!closeOutput(books, booksPath, error)) return false;
    std::ofstream people(membersPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!people.is_open()) { error = "Cannot open for writing: " + membersPath; return false; }
    people << "LMS_MEMBERS_V1\n" << version << '\n' << members.size() << '\n';
    for (std::map<std::string, Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
        const Member& member = it->second;
        people << member.id << '\n' << member.name << '\n' << member.loans.size() << '\n';
        for (std::size_t i = 0; i < member.loans.size(); ++i)
            people << member.loans[i].bookID << '\n' << member.loans[i].borrowTime << '\n';
    }
    people << "END\n";
    return closeOutput(people, membersPath, error);
}

bool Library::loadData(std::ostream& out) {
    saveAllowed = false; // Any failure prevents an exit-save from overwriting evidence.
    const std::string names[] = {"books.txt", "members.txt", "books.txt.bak", "members.txt.bak", "books.txt.tmp", "members.txt.tmp"};
    bool allAbsent = true;
    for (int i = 0; i < 6; ++i) {
        if (fileState(path(names[i])) != 0) allAbsent = false;
    }
    if (allAbsent) {
        if (!allBooks.empty() || !members.empty() || generation != 0) {
            out << "Load failed: data files disappeared; memory preserved, saving blocked.\n";
            return false;
        }
        saveAllowed = true;
        out << "First run: no data files; starting an empty library.\n";
        return true;
    }
    // Also recover old snapshots split between primary and backup during a rename.
    const std::string bookNames[] = {"books.txt", "books.txt.bak", "books.txt", "books.txt.bak"};
    const std::string memberNames[] = {"members.txt", "members.txt.bak", "members.txt.bak", "members.txt"};
    std::string primaryError;
    for (int candidate = 0; candidate < 4; ++candidate) {
        Library temporary(dataDirectory);
        std::string error;
        if (temporary.readPair(path(bookNames[candidate]), path(memberNames[candidate]), error)) {
            allBooks.swap(temporary.allBooks);
            bookIndex.swap(temporary.bookIndex);
            members.swap(temporary.members);
            generation = temporary.generation;
            saveAllowed = candidate == 0;
            if (candidate == 0) out << "Loaded snapshot " << generation << ".\n";
            else {
                out << "Primary data invalid: " << primaryError << '\n'
                    << "RECOVERED snapshot " << generation << " from " << bookNames[candidate]
                    << " + " << memberNames[candidate] << ". Saving BLOCKED.\n"
                    << "Preserve all files, restore this matching pair to books.txt/members.txt, then restart.\n";
            }
            return true;
        }
        if (candidate == 0) primaryError = error;
    }
    out << "Load failed: " << primaryError << ". No valid backup pair; memory preserved, saving blocked.\n";
    return false;
}

bool Library::saveData(std::ostream& out) {
    if (!saveAllowed) { out << "Save blocked: load/repair the data files first.\n"; return false; }
    std::string error;
    if (!validate(error) || generation >= 1000000000) {
        out << "Save failed: invalid state or snapshot limit. " << error << '\n'; return false;
    }
    // Detect a missing, damaged or newer on-disk pair before touching it.
    if (generation != 0) {
        Library disk(dataDirectory);
        if (!disk.readPair(path("books.txt"), path("members.txt"), error) || disk.generation != generation) {
            saveAllowed = false;
            out << "Save blocked: disk snapshot changed or became invalid. " << error << '\n'; return false;
        }
    } else if (fileState(path("books.txt")) != 0 || fileState(path("members.txt")) != 0 ||
               fileState(path("books.txt.bak")) != 0 || fileState(path("members.txt.bak")) != 0) {
        saveAllowed = false;
        out << "Save blocked: unexpected data files appeared.\n"; return false;
    }
    if (!writePair(path("books.txt.tmp"), path("members.txt.tmp"), generation + 1, error)) {
        out << "Save failed: " << error << ". Primary files unchanged.\n"; return false;
    }
    // Both new files are complete before any primary file is moved.
    if (generation != 0) {
        if (!removeOldBackup(path("books.txt.bak"), error) || !removeOldBackup(path("members.txt.bak"), error)) {
            out << "Save failed: " << error << ". Primary files unchanged.\n"; return false;
        }
        if (!renameFile(path("books.txt"), path("books.txt.bak"), error) ||
            !renameFile(path("members.txt"), path("members.txt.bak"), error)) {
            saveAllowed = false;
            out << "Save failed: " << error << ". Preserve files and restart for recovery.\n"; return false;
        }
    }
    if (!renameFile(path("books.txt.tmp"), path("books.txt"), error) ||
        !renameFile(path("members.txt.tmp"), path("members.txt"), error)) {
        saveAllowed = false;
        out << "Save failed: " << error << ". Preserve files and restart for recovery.\n"; return false;
    }
    ++generation;
    out << "Saved snapshot " << generation << ".\n";
    return true;
}
