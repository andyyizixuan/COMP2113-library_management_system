#include "member.h"
#include "book.h"

#include <cstddef>
#include <iostream>
#include <limits>
#include <utility>

namespace {

const char whitespace[] = " \t\n\r\f\v";

bool isValidMemberId(const std::string& memberID) {
    return !memberID.empty() && memberID.find_first_of(whitespace) == std::string::npos;
}

} // namespace

std::vector<Member> allMembers;
std::vector<BorrowRecord> borrowRecords;
std::map<std::string, std::queue<std::string>> reservations;

Member::Member(const std::string& memberID, const std::string& name)
    : memberID(memberID), name(name) {}

std::string Member::getId() const {
    return memberID;
}

std::string Member::getName() const {
    return name;
}

void registerMember() {
    std::string memberID;
    std::string name;

    // The menu reads its choice with operator>>, leaving the rest of that line.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Member ID: ";
    if (!std::getline(std::cin, memberID)) {
        return;
    }
    if (!isValidMemberId(memberID)) {
        std::cout << "Registration failed. Enter a non-empty ID without whitespace.\n";
        return;
    }

    std::cout << "Member name: ";
    if (!std::getline(std::cin, name)) {
        return;
    }

    if (registerMember(memberID, name)) {
        std::cout << "Member registered successfully.\n";
    } else {
        std::cout << "Registration failed. Enter a unique ID and a non-blank name.\n";
    }
}

bool registerMember(const std::string& memberID, const std::string& name) {
    if (!isValidMemberId(memberID) ||
        name.find_first_not_of(whitespace) == std::string::npos ||
        findMemberById(memberID) != nullptr) {
        return false;
    }

    allMembers.emplace_back(memberID, name);
    return true;
}

const Member* findMemberById(const std::string& memberID) {
    for (const Member& member : allMembers) {
        if (member.getId() == memberID) {
            return &member;
        }
    }
    return nullptr;
}

bool addReservation(const std::string& bookID, const std::string& memberID) {
    if (bookID.empty() || memberID.empty() || findMemberById(memberID) == nullptr ||
        searchBook_ID(bookID) == nullptr) {
        return false;
    }

    auto reservation = reservations.find(bookID);
    if (reservation == reservations.end()) {
        std::queue<std::string> waiting;
        waiting.push(memberID);
        reservations.emplace(bookID, std::move(waiting));
        return true;
    }

    // Scan a copy because std::queue does not expose iterators.
    std::queue<std::string> waiting = reservation->second;
    while (!waiting.empty()) {
        if (waiting.front() == memberID) {
            return false;
        }
        waiting.pop();
    }

    reservation->second.push(memberID);
    return true;
}

std::string getNextReserve(const std::string& bookID) {
    auto reservation = reservations.find(bookID);
    if (reservation == reservations.end() || reservation->second.empty()) {
        return std::string();
    }
    return reservation->second.front();
}

bool removeNextReservation(const std::string& bookID,
                           const std::string& expectedMemberID) {
    auto reservation = reservations.find(bookID);
    if (reservation == reservations.end() || reservation->second.empty() ||
        reservation->second.front() != expectedMemberID) {
        return false;
    }

    reservation->second.pop();
    if (reservation->second.empty()) {
        reservations.erase(reservation);
    }
    return true;
}

void printReservations() {
    bool printed = false;
    for (const auto& reservation : reservations) {
        if (reservation.second.empty()) {
            continue;
        }

        printed = true;
        std::cout << "Book ID: " << reservation.first << '\n';
        std::queue<std::string> waiting = reservation.second;
        std::size_t position = 1;
        while (!waiting.empty()) {
            std::cout << "  " << position << ". " << waiting.front() << '\n';
            waiting.pop();
            ++position;
        }
    }

    if (!printed) {
        std::cout << "No reservations.\n";
    }
}
