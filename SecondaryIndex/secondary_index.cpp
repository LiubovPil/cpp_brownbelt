#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

using namespace std;

// Реализуйте хранилище, поиск в котором можно осуществлять по вторичным индексам
// Put возвращает true, если вставка удалась, и false в противном случае
// GetById возвращает nullptr, если в базе данных нет записи с указанным id
// Erase возвращает true, если удалось удалить элемент с заданным id, и false в противном случае
// RangeByTimestamp, RangeByKarma и AllByUser принимают функтор callback, для записей, удовлетворяющих условий, выполняется callback

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

template<typename Type>
using SecondIndex = multimap<Type, const Record*>;

struct Data {
    Record record;
    SecondIndex<string>::iterator user_It;
    SecondIndex<int>::iterator timestamp_It;
    SecondIndex<int>::iterator karma_It;
};

class Database {
public:
    bool Put(const Record& record) {
        auto [it_data, is_insert] = db_id.insert({record.id, Data{record, {}, {}, {}}});
        if (is_insert) {
            auto& data = it_data->second;
            const Record* ptr_record = &data.record;
            data.timestamp_It = db_timestamp.insert({ record.timestamp, ptr_record });
            data.karma_It = db_karma.insert({ record.karma, ptr_record });
            data.user_It = db_user.insert({ record.user, ptr_record });
            return true;
        }
    }

    const Record* GetById(const string& id) const {
        auto it_data = db_id.find(id);
        return it_data != db_id.end() ? &it_data->second.record : nullptr;
    }

    bool Erase(const string& id) {
        auto it_data = db_id.find(id);
        if (it_data != db_id.end()) {
            db_timestamp.erase(it_data->second.timestamp_It);
            db_karma.erase(it_data->second.karma_It);
            db_user.erase(it_data->second.user_It);
            db_id.erase(id);
            return true;
        }
        else
            return false;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto it_begin = db_timestamp.lower_bound(low);
        auto it_end = db_timestamp.upper_bound(high);
        for (auto& it = it_begin; it != it_end; ++it) {
            if (!callback(*(it->second)))
                break;
        }
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto it_begin = db_karma.lower_bound(low);
        auto it_end = db_karma.upper_bound(high);
        for (auto& it = it_begin; it != it_end; ++it) {
            if (!callback(*(it->second)))
                break;
        }
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto range = db_user.equal_range(user);
        for (auto& it = range.first; it != range.second; ++it) {
            if (!callback(*(it->second)))
                break;
        }
    }

private:
    unordered_map<string, Data> db_id;
    SecondIndex<string> db_user;
    SecondIndex<int> db_karma;
    SecondIndex<int> db_timestamp;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({ "id1", "Hello there", "master", 1536107260, good_karma });
    db.Put({ "id2", "O>>-<", "general2", 1536107260, bad_karma });

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({ "id1", "Don't sell", "master", 1536107260, 1000 });
    db.Put({ "id2", "Rethink life", "master", 1536107260, 2000 });

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({ "id", "Have a hand", "not-master", 1536107260, 10 });
    db.Erase("id");
    db.Put({ "id", final_body, "not-master", 1536107260, -10 });

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
    auto no_record = db.GetById("idd");
    ASSERT(no_record == nullptr);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}