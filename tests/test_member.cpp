#include "member.h"
#include "book.h"

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

const char bookA[] = "001";
const char bookB[] = "1";

void expect(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Restore console buffers and input state even if a check throws.
class ConsoleCapture {
public:
    explicit ConsoleCapture(const std::string& inputText = "")
        : input(inputText), previousInputState(std::cin.rdstate()),
          previousInput(std::cin.rdbuf(input.rdbuf())),
          previousOutput(std::cout.rdbuf(output.rdbuf())) {}

    ~ConsoleCapture() {
        std::cout.rdbuf(previousOutput);
        std::cin.rdbuf(previousInput);
        std::cin.clear(previousInputState);
    }

    std::string text() const {
        return output.str();
    }

private:
    std::istringstream input;
    std::ostringstream output;
    std::ios::iostate previousInputState;
    std::streambuf* previousInput;
    std::streambuf* previousOutput;
};

// Match the formatted extraction used by the production menu before dispatch.
void expectMenuChoice(int expected) {
    int choice = 0;
    expect(static_cast<bool>(std::cin >> choice), "Could not read a menu choice.");
    expect(choice == expected, "The expected menu choice was not preserved.");
}

// Seed state directly when testing a function other than registration or enqueue.
void seedMember(const std::string& memberID) {
    allMembers.emplace_back(memberID, "Test Member");
}

void seedQueue(const std::string& bookID,
               std::initializer_list<std::string> memberIDs) {
    auto& queue = reservations[bookID];
    for (const auto& memberID : memberIDs) {
        queue.push(memberID);
    }
}

struct TestCase {
    const char* name;
    void (*run)();
};

const TestCase cases[] = {
    {"Register a new member", [] {
        expect(registerMember("M01", "Alice Wong"), "Registration returned false.");
        expect(allMembers.size() == 1, "Expected exactly one member.");
        expect(allMembers.front().getId() == "M01", "Stored ID differs.");
        expect(allMembers.front().getName() == "Alice Wong", "Stored name differs.");
    }},
    {"Reject a duplicate member ID", [] {
        allMembers.emplace_back("M01", "Alice Wong");
        expect(!registerMember("M01", "Another Name"), "Duplicate ID was accepted.");
        expect(allMembers.size() == 1, "Duplicate changed the member count.");
        expect(allMembers.front().getName() == "Alice Wong", "Original name changed.");
    }},
    {"Reject empty and whitespace-only member IDs", [] {
        expect(!registerMember("", "Alice Wong"), "Empty ID was accepted.");
        expect(!registerMember(" \t", "Alice Wong"), "Whitespace-only ID was accepted.");
        expect(allMembers.empty(), "Invalid IDs changed the member list.");
    }},
    {"Reject empty and whitespace-only names", [] {
        expect(!registerMember("M01", ""), "Empty name was accepted.");
        expect(!registerMember("M01", " \t"), "Whitespace-only name was accepted.");
        expect(allMembers.empty(), "Invalid names changed the member list.");
    }},
    {"Reject spaces inside an ID passed as an argument", [] {
        expect(!registerMember("M 01", "Alice Wong"), "ID with a space was accepted.");
        expect(allMembers.empty(), "Invalid ID changed the member list.");
    }},
    {"Allow different IDs with the same name", [] {
        allMembers.emplace_back("M01", "Alice Wong");
        expect(registerMember("M02", "Alice Wong"), "A shared name was rejected.");
        expect(allMembers.size() == 2, "Expected two members.");
        expect(allMembers.front().getId() == "M01" &&
                   allMembers.back().getId() == "M02" &&
                   allMembers.back().getName() == "Alice Wong",
               "The two stored members do not have the expected IDs and name.");
    }},
    {"Find an existing member and reject missing IDs", [] {
        seedMember("M01");
        seedMember("M02");
        const Member* member = findMemberById("M02");
        expect(member != nullptr && member->getId() == "M02", "Wrong lookup result.");
        expect(findMemberById("missing") == nullptr, "Unknown ID resolved.");
        expect(findMemberById("") == nullptr, "Empty ID resolved.");
        expect(findMemberById("m02") == nullptr, "Lookup should use the exact ID.");
    }},
    {"Read a full name after a menu choice", [] {
        ConsoleCapture console("3 \nM01\nAlice Chan Wong\n4\n");
        expectMenuChoice(3);
        registerMember();
        expect(allMembers.size() == 1, "Console registration did not add one member.");
        expect(allMembers.front().getId() == "M01", "Console stored the wrong ID.");
        expect(allMembers.front().getName() == "Alice Chan Wong",
               "Console did not preserve the full name.");
        expect(console.text().find("Member registered successfully.") != std::string::npos,
               "The success message is missing.");
        expectMenuChoice(4);
    }},
    {"Reject a duplicate ID entered at the console", [] {
        allMembers.emplace_back("M01", "Alice Wong");
        ConsoleCapture console("3\nM01\nAnother Name\n4\n");
        expectMenuChoice(3);
        registerMember();
        expect(allMembers.size() == 1 && allMembers.front().getName() == "Alice Wong",
               "A duplicate console registration changed the member list.");
        expect(console.text().find("Registration failed.") != std::string::npos,
               "The rejection message is missing.");
        expectMenuChoice(4);
    }},
    {"Reject blank fields and handle EOF after the menu choice", [] {
        const char* blankFields[] = {"3\nM01\n \t\n4\n", "3\n\n4\n", "3\n \t\n4\n"};
        for (const char* input : blankFields) {
            ConsoleCapture console(input);
            expectMenuChoice(3);
            registerMember();
            expect(allMembers.empty(), "A blank console field created a member.");
            expectMenuChoice(4);
        }

        const char* incompleteInputs[] = {"3\nM01\n", "3\n", "3"};
        for (const char* input : incompleteInputs) {
            ConsoleCapture console(input);
            expectMenuChoice(3);
            registerMember();
            expect(allMembers.empty(), "Incomplete console input created a member.");
        }
    }},
    {"Add a reservation for an existing book and member", [] {
        seedMember("M01");
        expect(addReservation(bookA, "M01"), "Valid reservation was rejected.");
        expect(reservations.size() == 1, "Expected exactly one book queue.");
        const auto& queue = reservations.at(bookA);
        expect(queue.size() == 1 && queue.front() == "M01",
               "The new reservation was not stored correctly.");
        expect(borrowRecords.empty(), "Reservation created a borrow record.");
    }},
    {"Reject a duplicate reservation behind the queue head", [] {
        seedMember("M01");
        seedMember("M02");
        seedMember("M03");
        seedQueue(bookA, {"M01", "M02", "M03"});
        const auto before = reservations;
        expect(!addReservation(bookA, "M02"), "A duplicate reservation was accepted.");
        expect(reservations == before, "Duplicate attempt changed a queue.");
    }},
    {"Reject a reservation for an unknown member", [] {
        expect(!addReservation(bookA, "missing"), "Unknown member was accepted.");
        expect(reservations.empty(), "Unknown member caused a queue entry.");
    }},
    {"Reject a reservation for an unknown book", [] {
        seedMember("M01");
        expect(!addReservation("missing", "M01"), "Unknown book was accepted.");
        expect(reservations.empty(), "Unknown book caused a queue entry.");
    }},
    {"Reject empty IDs in reservations", [] {
        seedMember("M01");
        expect(!addReservation("", "M01"), "Empty book ID was accepted.");
        expect(!addReservation(bookA, ""), "Empty member ID was accepted.");
        expect(reservations.empty(), "Empty IDs caused a queue entry.");
    }},
    {"Serve three reservations in their arrival order", [] {
        const char* arrival[] = {"M03", "M01", "M02"};
        for (const char* memberID : arrival) {
            seedMember(memberID);
            expect(addReservation(bookA, memberID), "Could not add a FIFO fixture.");
        }
        for (const char* memberID : arrival) {
            expect(getNextReserve(bookA) == memberID, "FIFO order was not preserved.");
            expect(removeNextReservation(bookA, memberID), "Could not remove the head.");
        }
        expect(reservations.count(bookA) == 0, "Queue entry remained after draining it.");
    }},
    {"Keep queues independent for book IDs 001 and 1", [] {
        seedMember("M01");
        seedMember("M02");
        expect(addReservation(bookA, "M01"), "Could not reserve book 001.");
        expect(addReservation(bookB, "M02"), "Could not reserve book 1.");
        expect(addReservation(bookB, "M01"), "Same member could not reserve a different book.");
        expect(reservations.size() == 2, "Different books did not get separate queues.");
        const auto before = reservations.at(bookB);
        expect(removeNextReservation(bookA, "M01"), "Could not remove book 001's head.");
        expect(reservations.at(bookB) == before && getNextReserve(bookB) == "M02",
               "Removing from book 001 changed book 1's queue.");
    }},
    {"Handle missing and explicitly empty queues", [] {
        seedQueue(bookA, {});
        const auto before = reservations;
        expect(getNextReserve(bookA).empty(), "Empty queue returned a member.");
        expect(getNextReserve(bookB).empty(), "Missing queue returned a member.");
        expect(!removeNextReservation(bookA, "M01"), "Removed from an empty queue.");
        expect(!removeNextReservation(bookB, "M01"), "Removed from a missing queue.");
        expect(reservations == before, "Empty or missing queue operations changed state.");
    }},
    {"Peek at the head twice without removing it", [] {
        seedMember("M01");
        seedMember("M02");
        seedQueue(bookA, {"M02", "M01"});
        const auto before = reservations;
        expect(getNextReserve(bookA) == "M02", "First peek returned the wrong member.");
        expect(getNextReserve(bookA) == "M02", "Second peek returned a different member.");
        expect(reservations == before, "Peeking changed a queue.");
    }},
    {"Reject removal when the expected member is not the head", [] {
        seedMember("M01");
        seedMember("M02");
        seedQueue(bookA, {"M02", "M01"});
        const auto before = reservations;
        expect(!removeNextReservation(bookA, "M01"), "A non-head member was removed.");
        expect(reservations == before, "A rejected removal changed a queue.");
    }},
    {"Remove the matching head and advance to the next member", [] {
        seedMember("M01");
        seedMember("M02");
        seedQueue(bookA, {"M02", "M01"});
        expect(removeNextReservation(bookA, "M02"), "Correct head was not removed.");
        expect(getNextReserve(bookA) == "M01", "The next member did not become the head.");
        expect(reservations.at(bookA).size() == 1, "Removal changed more than one entry.");
    }},
    {"Erase the queue entry after removing its final member", [] {
        seedMember("M01");
        seedQueue(bookA, {"M01"});
        expect(removeNextReservation(bookA, "M01"), "Final member was not removed.");
        expect(reservations.count(bookA) == 0, "An empty map entry remains.");
        expect(getNextReserve(bookA).empty(), "The removed queue still has a head.");
    }},
    {"Print FIFO positions without mutating any queue", [] {
        seedMember("M01");
        seedMember("M02");
        seedMember("M03");
        seedQueue(bookB, {"M03"});
        seedQueue(bookA, {"M02", "M01"});
        const auto before = reservations;
        const std::string expected =
            "Book ID: 001\n  1. M02\n  2. M01\n"
            "Book ID: 1\n  1. M03\n";

        ConsoleCapture console;
        printReservations();
        printReservations();
        expect(console.text() == expected + expected,
               "Printed book IDs, positions, or member order differs.");
        expect(reservations == before, "Printing changed a reservation queue.");
    }},
    {"Print an empty reservation list", [] {
        seedQueue(bookA, {});
        const auto before = reservations;
        ConsoleCapture console;
        printReservations();
        expect(console.text() == "No reservations.\n", "Wrong output for empty queues.");
        expect(reservations == before, "Printing an empty list changed state.");
    }},
    {"Reject an ID with an embedded space at the console", [] {
        {
            ConsoleCapture console("3\nM 01\nAlice Wong\n");
            expectMenuChoice(3);
            registerMember();
            if (!allMembers.empty()) {
                throw std::runtime_error("Input 'M 01' registered member '" +
                                         allMembers.front().getId() + "'.");
            }
        }

        ConsoleCapture console("3\nM 01\n4\n");
        expectMenuChoice(3);
        registerMember();
        expect(allMembers.empty(), "Invalid ID changed the member list.");
        expectMenuChoice(4);
    }},
};

} // namespace

int main() {
    // The real catalog is shared read-only by the cases after creating these books.
    if (!addBookFromFile(bookA, "Algorithms", "Author A", 1, 0) ||
        !addBookFromFile(bookB, "Programming", "Author B", 1, 0)) {
        std::cerr << "FAIL: Could not create the test books.\n";
        return 1;
    }

    std::size_t passed = 0;
    std::size_t failed = 0;
    for (const auto& test : cases) {
        allMembers.clear();
        borrowRecords.clear();
        reservations.clear();

        const std::size_t number = passed + failed + 1;
        try {
            test.run();
            ++passed;
            std::cout << "PASS " << number << " - " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cout << "FAIL " << number << " - " << test.name
                      << '\n' << "  " << error.what() << '\n';
        }
    }

    std::cout << '\n' << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
