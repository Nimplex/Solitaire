//
// Created by Pszemek on 29.05.2025.
//

#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

struct ScoreEntry {
    std::string name;
    int moves;

    bool operator<(const ScoreEntry& other) const {
        return moves < other.moves; // fewer moves = better score
    }
};

class ScoreManager {
private:
    std::vector<ScoreEntry> scores;
    std::string filePath;

    void loadFromFile() {
        scores.clear();
        std::ifstream file(filePath);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            size_t lastSpace = line.find_last_of(' ');
            if (lastSpace != std::string::npos) {
                const std::string name = line.substr(0, lastSpace);
                std::string movesStr = line.substr(lastSpace + 1);

                try {
                    int moves = std::stoi(movesStr);
                    scores.push_back({ name, moves });
                } catch (const std::exception&) {
                    // Skip invalid lines
                }
            }
        }

        std::sort(scores.begin(), scores.end());
    }

    void saveToFile() const {
        std::ofstream file(filePath, std::ios::trunc);
        if (file.is_open()) {
            for (const auto&[name, moves] : scores) {
                file << name << " " << moves << "\n";
            }
            file.flush();
        }
    }

public:
    explicit ScoreManager(const std::string& path) : filePath(path) {
        loadFromFile();
    }

    void addScore(const std::string& name, const int moves) {
        scores.push_back({name, moves});
        std::sort(scores.begin(), scores.end());
        saveToFile();
    }

    const std::vector<ScoreEntry>& getAllScores() const {
        return scores;
    }
};

#endif // SCOREMANAGER_H