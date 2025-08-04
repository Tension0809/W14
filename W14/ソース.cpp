#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <random>

using namespace std;
namespace fs = std::filesystem;
//���̖���
const vector<string> word = {
    "�wLife is an astounding journey�x",
    "�w�l����������͓̂������邽�߂ł͂Ȃ��A�������邽�߂ł���B�x",
    "�w���m�̐��E��T������l�X�́A�n�}�������Ȃ����s�҂ł���B�x",
    "�w��������Ƃ������Ƃ͐�����Ƃ������Ƃł���D�x",
    "�w�l���͖`����x",
    "�w�^���͉����J������x"
};

struct DiaryEntry {
    string text;
    int score = 0;
};
//�N���X
class DiaryManager {
public:

    DiaryManager() : gen(random_device()()) {
        ifstream flag_file("japan_conquest.ach");
        if (flag_file) {
            japanConquestAchieved = true;
        }
    }

    void run() {
        int choice;
        while (true) {
            displayMenu();
            if (!(cin >> choice)) {
                cout << "��������͂��Ă��������B" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (choice) {
            case 1: addRecord(); break;
            case 2: viewRecord(); break;
            case 3: listVisitedPrefectures(); break;
            case 4: displayKeikenchi(); break;
            case 5: {

                uniform_int_distribution<> dist(0, static_cast<int>(word.size()) - 1);
                int random_index = dist(gen); 
                cout << word[random_index] << endl;
                return;
            }
            default:
                cout << "�����ȑI���ł��B������x���͂��Ă��������B" << endl;
                break;
            }
        }
    }

private:
    bool japanConquestAchieved = false;
    mt19937 gen; 

    void displayMenu() const {
        cout << "\n*�E*�E*�E* �s���{���� ���L�� *�E*�E*�E*" << endl;
        cout << "1. ���L������" << endl;
        cout << "2. ���L������" << endl;
        cout << "3. �K��ς݈ꗗ" << endl;
        cout << "4. �o���l�̕\��" << endl;
        cout << "5. �I��" << endl;
        cout << "*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*�E*" << endl;
        cout << "�ԍ�����͂��Ă�������: ";
    }

    void listVisitedPrefectures() const {
        cout << "\n--- �K�₵���s���{���ꗗ ---" << endl;
        vector<string> prefectures = getVisitedPrefectures();
        if (prefectures.empty()) {
            cout << "�܂����L�͂���܂���B" << endl;
        }
        else {
            sort(prefectures.begin(), prefectures.end());
            for (const auto& pref : prefectures) {
                cout << pref << endl;
            }
        }
        cout << "--------------------------" << endl;
    }

    void addRecord() {
        cout << "\n�s���{��������͂��Ă�������: ";
        string prefecture;
        getline(cin, prefecture);
        string filename = prefecture + ".txt";

        cout << "�{������͂��Ă��������i�I������ɂ�Enter�L�[��2�񉟂��Ă��������j:" << endl;
        string line, content;
        while (getline(cin, line) && !line.empty()) {
            if (!content.empty()) content += "\n";
            content += line;
        }

        if (content.empty()) {
            cout << "\n���L���̂��ߕۑ����܂���ł����B" << endl;
            return;
        }

        int score = 0;
        while (true) {
            cout << "�o���l�i1:�ʉ�, 2:�~�藧����, 3:������, 4:������H�ׂ�, 5:��𖾂������j�����: ";
            if (cin >> score && score >= 1 && score <= 5) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
            cout << "�����ȓ��͂ł��B1����5�̔��p��������͂��Ă��������B" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        ofstream file_out(filename, ios::app);
        if (!file_out) {
            cerr << "�G���[: �t�@�C�����J���܂���ł����B" << endl;
            return;
        }

        if (fs::exists(filename) && fs::file_size(filename) > 0) {
            file_out << "\n\n\n";
        }

        file_out << content << "\nSCORE::" << score;
        file_out.close();
        cout << "\n�ۑ�����" << endl;

        checkJapanConquest();
    }

    void viewRecord() {
        vector<string> prefectures = getVisitedPrefectures();
        if (prefectures.empty()) {
            cout << "\n�܂����L�͂���܂���B" << endl;
            return;
        }
        cout << "\n�ǂ̓��L�����܂����H" << endl;
        sort(prefectures.begin(), prefectures.end());
        for (size_t i = 0; i < prefectures.size(); ++i) {
            cout << i + 1 << ". " << prefectures[i] << endl;
        }

        cout << "\n�ԍ�����͂��Ă�������: ";
        int choice;
        if (!(cin >> choice) || choice < 1 || choice > prefectures.size()) {
            cout << "�����Ȕԍ��ł��B" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string selected_prefecture = prefectures[choice - 1];
        string filename = selected_prefecture + ".txt";
        ifstream file(filename);

        cout << "\n������ �u" << selected_prefecture << "�v�̓��L ������" << endl;
        cout << file.rdbuf();
        cout << "\n����������������������������������������������" << endl;
        file.close();
    }

    void displayKeikenchi() const {
        cout << "\n--- �S���̌o���l---" << endl;
        long long T_total = 0;
        vector<string> prefectures = getVisitedPrefectures();
        sort(prefectures.begin(), prefectures.end());

        for (const auto& prefecture_name : prefectures) {
            vector<DiaryEntry> entries = loadEntriesFromFile(prefecture_name + ".txt");
            if (!entries.empty()) {
                auto max_it = max_element(entries.begin(), entries.end(),
                    [](const DiaryEntry& a, const DiaryEntry& b) {
                        return a.score < b.score;
                    });
                int max_score = max_it->score;
                cout << prefecture_name << ": " << max_score << "�_" << endl;
                T_total += max_score;
            }
        }
        cout << endl;
        cout << "�y�����v: " << T_total << "�_�z" << endl;
    }

    // �������u���{���e�v������
    void checkJapanConquest() {
        if (japanConquestAchieved) {
            return;
        }

        
        if (getVisitedPrefectures().size() >= 47) {
            cout << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!      �������y�S�����e�z������      !!!" << endl;
            cout << "!!! 47�s���{�����ׂĂ�K��܂��� !!!" << endl;
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

            japanConquestAchieved = true;
            ofstream flag_file("japan_conquest.ach");
            flag_file.close();
        }
    }

    vector<string> getVisitedPrefectures() const {
        vector<string> prefectures;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                prefectures.push_back(entry.path().stem().string());
            }
        }
        return prefectures;
    }

    vector<DiaryEntry> loadEntriesFromFile(const string& filename) const {
        vector<DiaryEntry> entries;
        ifstream file_in(filename);
        if (!file_in) return entries;

        stringstream buffer;
        buffer << file_in.rdbuf();
        string file_content = buffer.str();
        file_in.close();

        string separator = "\n\n\n";
        size_t start_pos = 0;

        while (start_pos < file_content.length()) {
            size_t end_pos = file_content.find(separator, start_pos);
            string block;
            if (end_pos == string::npos) {
                block = file_content.substr(start_pos);
                start_pos = file_content.length();
            }
            else {
                block = file_content.substr(start_pos, end_pos - start_pos);
                start_pos = end_pos + separator.length();
            }

            if (!trim(block).empty()) {
                entries.push_back(parseBlock(block));
            }
        }
        return entries;
    }

    DiaryEntry parseBlock(const string& block) const {
        DiaryEntry entry;
        string score_tag = "SCORE::";
        size_t tag_pos = block.rfind(score_tag);

        if (tag_pos != string::npos) {
            entry.text = trim(block.substr(0, tag_pos));
            try {
                entry.score = stoi(block.substr(tag_pos + score_tag.length()));
            }
            catch (...) { 
                entry.score = 0;
            }
        }
        else {
            entry.text = block;
            entry.score = 0;
        }
        return entry;
    }

    string trim(const string& str) const {
        const string whitespace = " \t\n\r\v\f";
        size_t first = str.find_first_not_of(whitespace);
        if (string::npos == first) return "";
        size_t last = str.find_last_not_of(whitespace);
        return str.substr(first, (last - first + 1));
    }
};

int main() {
    DiaryManager manager;
    manager.run();
    return 0;
}