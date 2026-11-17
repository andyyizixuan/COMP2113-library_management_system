#ifndef MEMBER_H
#define MEMBER_H

#include <ctime>
#include <map>
#include <queue>
#include <string>
#include <vector>

class Member {
public:
    // Create a member with the given ID and name.
    Member(const std::string& memberID, const std::string& name);

    std::string getId() const;
    std::string getName() const;

private:
    std::string memberID;
    std::string name;
};

// One active loan. The borrowing and return modules maintain these records.
struct BorrowRecord {
    std::string bookID;
    std::string memberID;
    std::time_t borrowTime;
};

extern std::vector<Member> allMembers;
extern std::vector<BorrowRecord> borrowRecords;
extern std::map<std::string, std::queue<std::string>> reservations;

// Read a member ID and name from the console and report the result.
void registerMember();

// Add the member and return true if the ID and name are valid and the ID is unique.
bool registerMember(const std::string& memberID, const std::string& name);

// Return the member with this ID, or nullptr if absent.
// Reallocation, erasure of the member, or reloading can invalidate the pointer.
const Member* findMemberById(const std::string& memberID);

// Return true if the registered member is added to the existing book's queue.
bool addReservation(const std::string& bookID, const std::string& memberID);

// Return the queue head without removing it, or an empty string if no one is waiting.
std::string getNextReserve(const std::string& bookID);

// Return true only if the head of the book's queue matches and is removed.
bool removeNextReservation(const std::string& bookID,
                           const std::string& expectedMemberID);

// Print each book's reservations in FIFO order without changing any queue.
void printReservations();

#endif
