#include <iostream>
#include <mysql.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;

class DBConnection {
public:
    DBConnection(){}
    virtual ~DBConnection() {}
  virtual MYSQL open() = 0;
  virtual void close(MYSQL mysql) = 0;
  virtual void execQuery(MYSQL& mysql, const char* query) = 0;
};

class ClassThatUsesDB: public DBConnection {
public:
    ClassThatUsesDB() {}
   virtual ~ClassThatUsesDB() {}

   MYSQL open() override
   {
       MYSQL mysql; // Дескриптор соединения c MySql
       mysql_init(&mysql);
       if (&mysql == NULL)
       {
           cout << "Error: can't create MySQL-descriptor" << endl;
                  }

       // Подключаемся к серверу
       if (!mysql_real_connect(&mysql, "localhost", "root", "0000", "testdb", 0, NULL, 0))
       {
           cout << "Error: can't connect to database " << mysql_error(&mysql) << endl;
       }
       else
       {
           cout << "Success!" << endl;
           mysql_set_character_set(&mysql, "utf8");
           return mysql;
       }
   }
   void execQuery(MYSQL &mysql, const char* query) override
   {
       mysql_query(&mysql, query);
   }
   void close(MYSQL mysql) override
   {
       mysql_close(&mysql);
   }
};

void selectSomesing (DBConnection* Connection)
{
    MYSQL mysql = Connection->open();
    Connection->execQuery(mysql, "SELECT* FROM TEST");
    Connection->close(mysql);
}

class MockDB : public DBConnection
{
public:
   
    MOCK_METHOD(MYSQL, open, (), (override));
    MOCK_METHOD(void, execQuery,(MYSQL& mysql, const char* query), (override));
    MOCK_METHOD(void, close, (MYSQL mysql), (override));
   };

TEST(DBConnection, test1)
{
    MockDB mDBreader;
    // указываем «ожидания» по использованию — методы будут вызваны хотя бы N раз
    EXPECT_CALL(mDBreader, execQuery).Times(::testing::AtLeast(1));
    EXPECT_CALL(mDBreader, open).Times(1);
    EXPECT_CALL(mDBreader, close).Times(1);
    selectSomesing(&mDBreader);
}

class SomeTestSuite : public ::testing::Test
{
protected:
    void SetUp()
    {
        DBConnect = new ClassThatUsesDB();
    }

    void TearDown()
    {
        delete DBConnect;
    }

protected:
    DBConnection* DBConnect;
};


int main(int argc, char* argv[])
{
    DBConnection *DBConnect = new ClassThatUsesDB;
    MYSQL discriptor = DBConnect->open();
    DBConnect->execQuery(discriptor, "SELECT NAME from Table_test");
    DBConnect->close(discriptor);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
