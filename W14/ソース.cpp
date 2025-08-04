#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <random>


/*memo
旅日記
都道府県別に日記を残すプログラム
使い方によっては普通の日記も使えそう
運動の記録とかよさそう．
タスクの進捗の記録もできるかも？


*/



using namespace std;
namespace fs = std::filesystem;

//旅の名言　いいのがあったら追加したい
const vector<string> word = {
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

//クラス！！
class DiaryManager {
public:
    DiaryManager() : gen(random_device()()) {
        ifstream flag_file("japan_conquest.ach");
        if (flag_file) {
            japanConquestAchieved = true;
        }
    }

    //メニュー設定
    void run() {
        int choice;
        while (true) {
            displayMenu();
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
                uniform_int_distribution<> dist(0, static_cast<int>(word.size()) - 1);
                int random_index = dist(gen);
                cout << word[random_index] << endl;
                return;
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

    // メニュー項目
    void displayMenu() const {
        cout << "\n*・*・*・* 都道府県別 日記帳 *・*・*・*" << endl;
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
    //訪問した都道府県
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
    //日記を追加
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
		// 経県値の入力
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
        file_out.close();
        cout << "\n保存完了" << endl;

        checkJapanConquest();
    }

    //日記を見る　想いで
    void viewRecord() {
     
        string filename = selectPrefectureFile();
        if (filename.empty()) {
            return; 
        }

 
        vector<DiaryEntry> entries = loadEntriesFromFile(filename);
        if (entries.empty()) {
            cout << "この都道府県には日記がありません。" << endl;
            return;
        }

        string prefecture_name = fs::path(filename).stem().string();
        
        cout << "\n■□■ 「" << prefecture_name << "」の日記 ■□■" << endl;
        //見やすく
        for (size_t i = 0; i < entries.size(); ++i) {
            cout << "\n--- " << i + 1 << "件目 ---" << endl;
            cout << entries[i].text << endl;
			cout << "（経県値: " << entries[i].score << "）" << endl;//スコアじゃなくて経県値に変更
        }
        cout << "\n■□■□■□■□■□■□■□■□■□■□■□■" << endl;
    }

    //経験値
    void displayKeikenchi() const {
        cout << "\n--- 全国の経県値---" << endl;
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
                cout << prefecture_name << ": " << max_score << "点" << endl;
                T_total += max_score;
            }
        }
        cout << endl;
        cout << "【総合計: " << T_total << "点】" << endl;
    }

    // 実績の確認「日本を知る者」
    void checkJapanConquest() {
        if (japanConquestAchieved) {
            return;
        }

        if (getVisitedPrefectures().size() >= 47) {
            cout << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!     ★★★【全国制覇】★★★     !!!" << endl;
            cout << "!!!        「日本を知る者」         " << endl;
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

            japanConquestAchieved = true;
            ofstream flag_file("japan_conquest.ach");
            flag_file.close();
        }
    }

    //日記の修正
    void editRecord() {
        // 都道府県を選択
        string filename = selectPrefectureFile();
        if (filename.empty()) return;

        // 修正する日記を選択
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

        // 新しい内容を入力
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

        // 4. 日記を更新して保存
        entries[entry_index - 1].text = new_content;
        entries[entry_index - 1].score = new_score;

        if (saveEntriesToFile(filename, entries)) {
            cout << "\n修正完了" << endl;
        }
        else {
            cerr << "エラー: ファイルの保存に失敗しました。" << endl;
        }
    }

    //日記の消去
    void deleteRecord() {
        // 都道府県を選択
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

        //  さすがに確認
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


    // A都道府県ファイルの選択
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

    //B日記の保存
    bool saveEntriesToFile(const string& filename, const vector<DiaryEntry>& entries) {

        ofstream file_out(filename, ios::trunc);
        if (!file_out) {
            return false;
        }

        for (size_t i = 0; i < entries.size(); ++i) {
            file_out << entries[i].text << "\nSCORE::" << entries[i].score;
            if (i < entries.size() - 1) {
                file_out << "\n\n\n";
            }
        }
        file_out.close();
        return true;
    }

	// C訪問した都道府県を取得
    vector<string> getVisitedPrefectures() const {
        vector<string> prefectures;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                prefectures.push_back(entry.path().stem().string());
            }
        }
        return prefectures;
    }

	// Dファイルから日記を読み込む
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

	// E解析，日記エントリを作成
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
	// F前後の空白を削除
    string trim(const string& str) const {
        const string whitespace = " \t\n\r\v\f";
        size_t first = str.find_first_not_of(whitespace);
        if (string::npos == first) return "";
        size_t last = str.find_last_not_of(whitespace);
        return str.substr(first, (last - first + 1));
    }
};

// 動きます
int main() {
    DiaryManager manager;
    manager.run();
    return 0;
}
