#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>
#include <memory>

/**
 * @project: Trinket Core Structural Analyzer (v2.5)
 * @repository: trinket-core-analyzer
 * @author: Waseem Shahzad
 * @description: High-performance forensic utility to parse and map binary structure boundaries.
 */

using namespace std;

// ANSI Escape Codes for Terminal Colorization (Supported on Windows 10+ and Linux)
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string CYAN = "\033[36m";
const string BOLD = "\033[1m";

const size_t TARGET_BLOCK_SIZE = 72;
const uint8_t TARGET_VERSION_HEX = 0x01;

class PathUtility {
public:
    static string cleanPath(string path) {
        // Step 1: Remove all double quotes from the path string
        path.erase(remove(path.begin(), path.end(), '\"'), path.end());

        // Step 2: Trim spaces from the absolute beginning (Leading spaces)
        path.erase(path.begin(), find_if(path.begin(), path.end(), [](unsigned char ch) {
            return !isspace(ch);
            }));

        // Step 3: Trim spaces from the absolute end (Trailing spaces)
        path.erase(find_if(path.rbegin(), path.rend(), [](unsigned char ch) {
            return !isspace(ch);
            }).base(), path.end());

        return path;
    }
};

class TrinketCoreAnalyzer {
private:
    vector<uint8_t> buffer;
    string fileName;

    bool validateStructure(size_t offset) const {
        size_t nonZeroCount = 0;
        for (size_t j = 0; j < TARGET_BLOCK_SIZE; ++j) {
            if (buffer[offset + j] != 0x00) nonZeroCount++;
        }
        // 50% entropy threshold verification to filter out false-positive alignments
        return nonZeroCount > (TARGET_BLOCK_SIZE / 2);
    }

    void printHexBlock(size_t offset) const {
        cout << CYAN << "----------------------------------------------------------------------" << RESET << endl;
        for (size_t j = 0; j < TARGET_BLOCK_SIZE; ++j) {
            cout << setfill('0') << setw(2) << hex << (int)buffer[offset + j] << " ";
            if ((j + 1) % 16 == 0) cout << endl;
        }
        if (TARGET_BLOCK_SIZE % 16 != 0) cout << endl;
        cout << CYAN << "----------------------------------------------------------------------" << RESET << endl;
    }

public:
    explicit TrinketCoreAnalyzer(const string& file) : fileName(PathUtility::cleanPath(file)) {}

    bool loadFile() {
        ifstream file(fileName, ios::binary | ios::ate);
        if (!file.is_open()) return false;

        streamsize size = file.tellg();
        if (size <= 0) return false;

        file.seekg(0, ios::beg);
        buffer.resize(static_cast<size_t>(size));

        return (bool)file.read(reinterpret_cast<char*>(buffer.data()), size);
    }

    void executeForensicScan() {
        cout << "\n" << CYAN << "[+]" << RESET << " Analyzing Source: " << BOLD << fileName << RESET << endl;
        cout << CYAN << "[+]" << RESET << " Payload Size: " << YELLOW << buffer.size() << RESET << " bytes" << endl;
        cout << CYAN << "[+]" << RESET << " Scanning for Structural Target Headers (" << hex << (int)TARGET_VERSION_HEX << ")..." << dec << endl;

        if (buffer.size() < TARGET_BLOCK_SIZE) {
            cout << RED << "\n[!] ERROR: Input size is smaller than the required target block parameters." << RESET << endl;
            return;
        }

        bool matchFound = false;
        const size_t scanLimit = buffer.size() - TARGET_BLOCK_SIZE;

        for (size_t i = 0; i <= scanLimit; ++i) {
            if (buffer[i] == TARGET_VERSION_HEX) {
                if (validateStructure(i)) {
                    cout << "\n" << GREEN << BOLD << "[SUCCESS] Valid Core Substructure Mapped at Offset: 0x"
                        << hex << uppercase << i << dec << RESET << endl;
                    printHexBlock(i);
                    matchFound = true;
                }
            }
        }

        if (!matchFound) {
            cout << "\n" << YELLOW << "[!] STATUS: Standard " << TARGET_BLOCK_SIZE << "-byte structural boundary signature not detected." << RESET << endl;
            cout << CYAN << "[*] FORENSIC TIP:" << RESET << " Verify if raw padding extraction routines were applied upstream." << endl;
        }
    }
};

int main() {
    // Initialize ANSI terminal escape sequences for Windows console subsystems
#ifdef _WIN32
    system("Color 0");
#endif

    cout << BOLD << CYAN << "==========================================================" << RESET << endl;
    cout << BOLD << CYAN << "          TRINKET CORE STRUCTURAL ANALYZER (v2.5)         " << RESET << endl;
    cout << BOLD << CYAN << "==========================================================" << RESET << endl;

    cout << "\n" << YELLOW << "[INPUT REQUEST]" << RESET << " Drag and drop your target binary file here and press Enter:" << endl;
    cout << "> ";

    string inputPath;
    getline(cin, inputPath);

    if (inputPath.empty()) {
        cout << RED << "[ERROR] Base absolute stream path sequence is empty. Exiting..." << RESET << endl;
        return 1;
    }

    auto analyzer = make_unique<TrinketCoreAnalyzer>(inputPath);

    if (analyzer->loadFile()) {
        analyzer->executeForensicScan();
    }
    else {
        cout << RED << "[ERROR] IO Failure: Unable to map file descriptor. Verify file permissions/path string." << RESET << endl;
    }

    cout << "\n" << CYAN << "[Session Complete]" << RESET << " Press Enter to terminate process..." << endl;
    cin.get();
    return 0;
}