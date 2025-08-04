#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <random>
#include <numeric> 
#include <map>  

/*
 * memo
 * 旅日記プログラム
 * 都道府県別に日記を記録
 * 
 * タスク管理や他の記録にも応用可能？
 * 練習記録とか
 */

using namespace std;
namespace fs = std::filesystem;

// 旅の名言集　いいのがあれば追加したい
const vector<string> TRAVEL_QUOTES = {
    "『Life is an astounding journey』",
    "『人が旅をするのは到着するためではなく、旅をするためである。』",
    "『未知の世界を探求する人々は、地図を持たない旅行者である。』",
    "『旅をするということは生きるということである．』",
    "『人生は冒険や』",
    "『真理は我らを開放する』"
};

struct DiaryEntry {
    string text;
    int score = 0;
};

// 日記管理クラス！！
class DiaryManager {
public:
    DiaryManager() : gen(random_device()()) {
        ifstream flag_file("japan_conquest.ach");
        if (flag_file) {
            japanConquestAchieved = true;
        }
    }

    // メインループ
    void run() {
        while (true) {
            displayMenu();
            int choice;
            if (!(cin >> choice)) {
                cout << "数字を入力してください。" << endl;
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
            case 5: editRecord(); break;
            case 6: deleteRecord(); break;
            case 7: {
                uniform_int_distribution<> dist(0, static_cast<int>(TRAVEL_QUOTES.size()) - 1);
                cout << TRAVEL_QUOTES[dist(gen)] << endl;
                return; // プログラム終了
            }
            default:
                cout << "無効な選択です。もう一度入力してください。" << endl;
                break;
            }
        }
    }

private:
    bool japanConquestAchieved = false;
    mt19937 gen;

    // --- データ処理・計算関数 ---

    // 各都道府県の最大経県値をマップで取得
    map<string, int> getPrefectureMaxScores() const {
        map<string, int> scores;
        for (const auto& prefecture_name : getVisitedPrefectures()) {
            vector<DiaryEntry> entries = loadEntriesFromFile(prefecture_name + ".txt");
            if (!entries.empty()) {
                auto max_it = max_element(entries.begin(), entries.end(),
                    [](const DiaryEntry& a, const DiaryEntry& b) {
                        return a.score < b.score;
                    });
                scores[prefecture_name] = max_it->score;
            }
        }
        return scores;
    }

    // 称号
    string getTitle(int level) const {
        if (japanConquestAchieved) {
            return "★★★【全国制覇】日本の万物を知る者 ★★★";
        }
        if (level >= 20) return "★★★日本の真理を知る者★★★";
        if (level >= 10) return "★★日本の探求者★★";
        if (level >= 5) return "★冒険家★";
        if (level >= 3) return "★旅人";
        return "旅の初心者";
    }

    // --- メニュー表示関連 ---

    // メインメニュー
    void displayMenu() const {
        map<string, int> scores = getPrefectureMaxScores();
        long long total_score = accumulate(scores.begin(), scores.end(), 0LL,
            [](long long sum, const auto& pair) { return sum + pair.second; });

        int level = static_cast<int>(total_score / 10) + 1; 
        string title = getTitle(level);

        cout << "\n【" << title << "】 Lv." << level << " (総経県値: " << total_score << "点)" << endl;
        cout << "*・*・*・* 都道府県別 日記帳 *・*・*・*" << endl;
        cout << "1. 思い出を残す" << endl;
        cout << "2. 思い出を振り返る" << endl;
        cout << "3. 訪れた都道府県" << endl;
        cout << "4. 経県値" << endl;
        cout << "5. 日記の修正" << endl;
        cout << "6. 日記を消去" << endl;
        cout << "7. 終了" << endl;
        cout << "*・*・*・*・*・*・*・*・*・*・*・*・*・*・*・*・*・*" << endl;
        cout << "番号を入力してください: ";
    }

    // 訪問済みの都道府県一覧表示
    void listVisitedPrefectures() const {
        cout << "\n--- 訪問した都道府県 ---" << endl;
        vector<string> prefectures = getVisitedPrefectures();
        if (prefectures.empty()) {
            cout << "まだ日記はありません。" << endl;
        }
        else {
            sort(prefectures.begin(), prefectures.end());
            for (const auto& pref : prefectures) {
                cout << pref << endl;
            }
        }
        cout << "--------------------------" << endl;
    }

    // 経県値
    void displayKeikenchi() const {
        cout << "\n--- 全国の経県値---" << endl;
        map<string, int> scores = getPrefectureMaxScores(); // std::mapはキーでソートされる
        long long total = 0;

        for (const auto& pair : scores) {
            cout << pair.first << ": " << pair.second << "点" << endl;
            total += pair.second;
        }
        cout << endl;
        cout << "【総合計: " << total << "点】" << endl;
    }

    // --- 日記の操作 ---

    // 追加
    void addRecord() {
        cout << "\n都道府県名を入力してください: ";
        string prefecture;
        getline(cin, prefecture);
        string filename = prefecture + ".txt";

        cout << "本文を入力してください（終了するにはEnterキーを2回押してください）:" << endl;
        string line, content;
        while (getline(cin, line) && !line.empty()) {
            if (!content.empty()) content += "\n";
            content += line;
        }

        if (content.empty()) {
            cout << "\n未記入のため保存しませんでした。" << endl;
            return;
        }

        int score = 0;
        while (true) {
            cout << "経県値（1:通過, 2:降り立った, 3:歩いた, 4:名物を食べた, 5:夜を明かした）を入力: ";
            if (cin >> score && score >= 1 && score <= 5) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
            cout << "無効な入力です。1から5の半角数字を入力してください。" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        ofstream file_out(filename, ios::app);
        if (!file_out) {
            cerr << "エラー: ファイルを開けませんでした。" << endl;
            return;
        }

        if (fs::exists(filename) && fs::file_size(filename) > 0) {
            file_out << "\n\n\n";
        }
        file_out << content << "\nSCORE::" << score;
        cout << "\n保存完了" << endl;

        checkJapanConquest(); 
    }

    // 閲覧
    void viewRecord() {
        string filename = selectPrefectureFile();
        if (filename.empty()) return;

        vector<DiaryEntry> entries = loadEntriesFromFile(filename);
        if (entries.empty()) {
            cout << "この都道府県には日記がありません。" << endl;
            return;
        }

        string prefecture_name = fs::path(filename).stem().string();
        cout << "\n■□■ 「" << prefecture_name << "」の日記 ■□■" << endl;
        for (size_t i = 0; i < entries.size(); ++i) {
            cout << "\n--- " << i + 1 << "件目 ---" << endl;
            cout << entries[i].text << endl;
            cout << "（経県値: " << entries[i].score << "）" << endl;
        }
        cout << "\n■□■□■□■□■□■□■□■□■□■□■□■" << endl;
    }

    // 修正
    void editRecord() {
        string filename = selectPrefectureFile();
        if (filename.empty()) return;

        vector<DiaryEntry> entries = loadEntriesFromFile(filename);
        if (entries.empty()) {
            cout << "この都道府県には日記がありません。" << endl;
            return;
        }

        cout << "\nどの項目を修正しますか？ 番号で選んでください。" << endl;
        for (size_t i = 0; i < entries.size(); ++i) {
            cout << i + 1 << ".\n" << "----------\n" << entries[i].text << "\n----------" << endl;
        }

        cout << "番号: ";
        int entry_index;
        if (!(cin >> entry_index) || entry_index < 1 || entry_index > static_cast<int>(entries.size())) {
            cout << "無効な番号です。" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\n新しい本文を入力してください（終了するにはEnterキーを2回押してください）:" << endl;
        string line, new_content;
        while (getline(cin, line) && !line.empty()) {
            if (!new_content.empty()) new_content += "\n";
            new_content += line;
        }

        if (new_content.empty()) {
            cout << "\n未記入のため修正しませんでした。" << endl;
            return;
        }

        int new_score = 0;
        while (true) {
            cout << "新しい経県値（1-5）を入力: ";
            if (cin >> new_score && new_score >= 1 && new_score <= 5) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
            cout << "無効な入力です。1から5の半角数字を入力してください。" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        entries[entry_index - 1].text = new_content;
        entries[entry_index - 1].score = new_score;

        if (saveEntriesToFile(filename, entries)) {
            cout << "\n修正完了" << endl;
        }
        else {
            cerr << "エラー: ファイルの保存に失敗しました。" << endl;
        }
    }

    // 消去
    void deleteRecord() {
        string filename = selectPrefectureFile();
        if (filename.empty()) return;

        vector<DiaryEntry> entries = loadEntriesFromFile(filename);
        if (entries.empty()) {
            cout << "この都道府県には日記がありません。" << endl;
            return;
        }

        cout << "\nどの項目を消去しますか？ 番号で選んでください。" << endl;
        for (size_t i = 0; i < entries.size(); ++i) {
            cout << i + 1 << ".\n" << "----------\n" << entries[i].text << "\n----------" << endl;
        }

        cout << "番号: ";
        int entry_index;
        if (!(cin >> entry_index) || entry_index < 1 || entry_index > static_cast<int>(entries.size())) {
            cout << "無効な番号です。" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\n本当にこの日記を消去しますか？ (y/n): ";
        char confirmation;
        cin >> confirmation;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (confirmation != 'y' && confirmation != 'Y') {
            cout << "消去をキャンセルしました。" << endl;
            return;
        }

        entries.erase(entries.begin() + (entry_index - 1));

        if (entries.empty()) {
            fs::remove(filename);
            cout << "\n消去完了。日記がなくなったためファイルも削除しました。" << endl;
        }
        else {
            if (saveEntriesToFile(filename, entries)) {
                cout << "\n消去完了" << endl;
            }
            else {
                cerr << "エラー: ファイルの保存に失敗しました。" << endl;
            }
        }
    }

    // --- ファイル・ヘルパー関数 ---

    // 全ての日記ファイルを上書き保存
    bool saveEntriesToFile(const string& filename, const vector<DiaryEntry>& entries) {
        ofstream file_out(filename, ios::trunc); // truncでファイルを上書き
        if (!file_out) {
            return false;
        }
        for (size_t i = 0; i < entries.size(); ++i) {
            file_out << entries[i].text << "\nSCORE::" << entries[i].score;
            if (i < entries.size() - 1) {
                file_out << "\n\n\n";
            }
        }
        return true;
    }

    // 操作対象のファイルを選択
    string selectPrefectureFile() {
        vector<string> prefectures = getVisitedPrefectures();
        if (prefectures.empty()) {
            cout << "\nまだ日記はありません。" << endl;
            return "";
        }

        cout << "\nどの都道府県の日記を操作しますか？" << endl;
        sort(prefectures.begin(), prefectures.end());
        for (size_t i = 0; i < prefectures.size(); ++i) {
            cout << i + 1 << ". " << prefectures[i] << endl;
        }

        cout << "\n番号を入力してください: ";
        int choice;
        if (!(cin >> choice) || choice < 1 || choice > static_cast<int>(prefectures.size())) {
            cout << "無効な番号です。" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return "";
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return prefectures[choice - 1] + ".txt";
    }

    // 実績
    void checkJapanConquest() {
        if (japanConquestAchieved) return;

        if (getVisitedPrefectures().size() >= 47) {
            cout << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!     ★★★【全国制覇】★★★     !!!" << endl;
            cout << "!!!        「日本の万物を知る者」        !!!" << endl;
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

            japanConquestAchieved = true;
            ofstream flag_file("japan_conquest.ach"); // 実績達成の証
            flag_file.close();
        }
    }

    // 訪問済みストを取得
    vector<string> getVisitedPrefectures() const {
        vector<string> prefectures;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                prefectures.push_back(entry.path().stem().string());
            }
        }
        return prefectures;
    }

    // ファイルから日記エントリを読み込む
    vector<DiaryEntry> loadEntriesFromFile(const string& filename) const {
        vector<DiaryEntry> entries;
        ifstream file_in(filename);
        if (!file_in) return entries;

        stringstream buffer;
        buffer << file_in.rdbuf();
        string file_content = buffer.str();

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

    // 1つの日記ブロックを解析，DiaryEntryを作成
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

    // 文字列の前後の空白を削除
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
