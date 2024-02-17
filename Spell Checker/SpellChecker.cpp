#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Trie Node
struct TrieNode {
  unordered_map<char, TrieNode *> children;
  bool isEndOfWord;
  vector<string> suggestions;
  TrieNode() : isEndOfWord(false) {}
};

// Spell Checker Class
class SpellChecker {
private:
  TrieNode *root;

public:
  SpellChecker() { root = new TrieNode(); }

  // Inserts a word into the trie
  void insert(const string &word) {
    TrieNode *curr = root;
    for (char c : word) {
      if (curr->children.find(c) == curr->children.end()) {
        curr->children[c] = new TrieNode();
      }
      curr = curr->children[c];
    }
    curr->isEndOfWord = true;
  }

  // Load dictionary into the spell checker
  void loadDictionary(const string &filename) {
    ifstream dictionaryFile(filename);
    string word;
    while (getline(dictionaryFile, word)) {
      insert(word);
    }
    dictionaryFile.close();
  }

  // Check if a word exists in the trie
  bool search(const string &word) {
    TrieNode *curr = root;
    for (char c : word) {
      if (curr->children.find(c) == curr->children.end()) {
        return false;
      }
      curr = curr->children[c];
    }
    return curr != nullptr && curr->isEndOfWord;
  }

  // Spell check a word and suggest corrections
  vector<string> spellCheck(const string &word) {
    vector<string> result;
    string lowercaseWord = word;
    string str = "";
    TrieNode *curr = root;
    for (char c : lowercaseWord) {
      if (curr->children.find(c) == curr->children.end() && curr == root)
        return result;
      else if (curr->children.find(c) == curr->children.end() && curr != root) {
        generateSuggestions(curr, str, str, result);
        return result;
      } else
        str += c;
      curr = curr->children[c];
    }
    if (curr != nullptr && curr->isEndOfWord) {
      // Word found, no need for suggestions
      return result;
    }
    return result;
  }

private:
  // Generate suggestions for misspelled word
  void generateSuggestions(TrieNode *node, const string &originalWord,
                           const string &word, vector<string> &suggestions) {
    if (!node)
      return;
    if (node->isEndOfWord) {
      if (word.length() <= originalWord.length() + 3) {
        if (suggestions.size() < 4) {
          suggestions.push_back(word);
        }
      }
    }
    if (suggestions.size() >= 4) {
      return;
    }
    for (auto &child : node->children) {
      generateSuggestions(child.second, originalWord, word + child.first,
                          suggestions);
    }
  }
};

string removeTrailingPunctuation(const std::string &word) {
  std::string result = word;
  while (!result.empty() && ispunct(result.back())) {
    result.pop_back();
  }
  return result;
}

template <typename Func> auto measureResponseTime(Func func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
      .count();
}

int main() {
  // Function to execute with timing
  auto sendRequest = []() {
    SpellChecker spellChecker;
    spellChecker.loadDictionary("./src/dictionary.txt");
    ifstream csvFile("/home/coderpad/app/build/src/example.csv");
    if (!csvFile.is_open()) {
      cerr << "Error: Unable to open example.csv" << endl;
      return;
    }
    string line;
    while (getline(csvFile, line)) {
      stringstream ss(line);
      string word;
      while (ss >> word) {
        word = removeTrailingPunctuation(word);
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (!spellChecker.search(word)) {
          cout << "Misspelled word: " << word << endl;
          vector<string> suggestions = spellChecker.spellCheck(word);
          if (!suggestions.empty()) {
            cout << "Suggestions:";
            for (const auto &suggestion : suggestions) {
              cout << " " << suggestion;
            }
            cout << endl << endl;
          }
        }
      }
    }
    csvFile.close();
  };

  // Measure response time
  auto responseTime = measureResponseTime(sendRequest);
  cout << "Response time: " << responseTime << " microseconds" << endl;

  return 0;
}
