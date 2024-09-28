/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "CharacterTest.hpp"

#include "oatpp-postgresql/orm.hpp"
#include "oatpp/json/ObjectMapper.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace postgresql { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class Row : public oatpp::DTO {

  DTO_INIT(Row, DTO);

  DTO_FIELD(String, f_char);
  DTO_FIELD(String, f_bpchar);
  DTO_FIELD(String, f_bpchar4);
  DTO_FIELD(String, f_varchar);
  DTO_FIELD(String, f_text);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    executeQuery("DROP TABLE IF EXISTS oatpp_schema_version_CharacterTest;", {});
    oatpp::orm::SchemaMigration migration(executor, "CharacterTest");
    migration.addFile(1, TEST_DB_MIGRATION "CharacterTest.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion("CharacterTest");
    OATPP_LOGd("DbClient", "Migration - OK. Version={}.", version);

  }

  QUERY(insertValues,
        "INSERT INTO test_characters "
        "(f_char, f_bpchar, f_bpchar4, f_varchar, f_text) "
        "VALUES "
        "(:row.f_char, :row.f_bpchar, :row.f_bpchar4, :row.f_varchar, :row.f_text);",
        PARAM(oatpp::Object<Row>, row), PREPARE(true))

  QUERY(deleteValues,
        "DELETE FROM test_characters;")

  QUERY(selectValues, "SELECT * FROM test_characters;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void CharacterTest::onRun() {

  OATPP_LOGi(TAG, "DB-URL='{}'", TEST_DB_URL);

  auto connectionProvider = std::make_shared<oatpp::postgresql::ConnectionProvider>(TEST_DB_URL);
  auto executor = std::make_shared<oatpp::postgresql::Executor>(connectionProvider);

  auto client = MyClient(executor);

  {
    auto res = client.selectValues();
    if(res->isSuccess()) {
      OATPP_LOGd(TAG, "OK, knownCount={}, hasMore={}", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGd(TAG, "Error, message={}", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<Row>>>();

    oatpp::json::ObjectMapper om;
    om.serializerConfig().json.useBeautifier = true;
    om.serializerConfig().mapper.enabledInterpretations = { "postgresql" };

    auto str = om.writeToString(dataset);

    OATPP_LOGd(TAG, "res={}", str->c_str());

    OATPP_ASSERT(dataset->size() == 3);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_char == nullptr);
      OATPP_ASSERT(row->f_bpchar == nullptr);
      OATPP_ASSERT(row->f_bpchar4 == nullptr);
      OATPP_ASSERT(row->f_varchar == nullptr);
      OATPP_ASSERT(row->f_text == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_char == "#");
      OATPP_ASSERT(row->f_bpchar == "$");
      OATPP_ASSERT(row->f_bpchar4 == "%   ");
      OATPP_ASSERT(row->f_varchar == "^");
      OATPP_ASSERT(row->f_text == "&");
    }

    {
      auto row = dataset[2];
      OATPP_ASSERT(row->f_char == "a");
      OATPP_ASSERT(row->f_bpchar == "b");
      OATPP_ASSERT(row->f_bpchar4 == "cccc");
      OATPP_ASSERT(row->f_varchar == "dddd");
      OATPP_ASSERT(row->f_text == "eeeee");
    }


  }

  {
    auto res = client.deleteValues();
    if (res->isSuccess()) {
      OATPP_LOGd(TAG, "OK, knownCount={}, hasMore={}", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGd(TAG, "Error, message={}", message->c_str());
    }

    OATPP_ASSERT(res->isSuccess());
  }

  {
    auto connection = client.getConnection();
    {
      auto row = Row::createShared();
      row->f_char = nullptr;
      row->f_bpchar = nullptr;
      row->f_bpchar4= nullptr;
      row->f_varchar = nullptr;
      row->f_text = nullptr;
      client.insertValues(row, connection);
    }

    {
      auto row = Row::createShared();
      row->f_char = "a";
      row->f_bpchar = "b";
      row->f_bpchar4= "ccc";
      row->f_varchar = "dddd";
      row->f_text = "eeeee";
      client.insertValues(row, connection);
    }
  }

  {
    auto res = client.selectValues();
    if(res->isSuccess()) {
      OATPP_LOGd(TAG, "OK, knownCount={}, hasMore={}", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGd(TAG, "Error, message={}", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<Row>>>();

    oatpp::json::ObjectMapper om;
    om.serializerConfig().json.useBeautifier = true;
    om.serializerConfig().mapper.enabledInterpretations = { "postgresql" };

    auto str = om.writeToString(dataset);

    OATPP_LOGd(TAG, "res={}", str->c_str());

    OATPP_ASSERT(dataset->size() == 2);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_char == nullptr);
      OATPP_ASSERT(row->f_bpchar == nullptr);
      OATPP_ASSERT(row->f_bpchar4 == nullptr);
      OATPP_ASSERT(row->f_varchar == nullptr);
      OATPP_ASSERT(row->f_text == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_char == "a");
      OATPP_ASSERT(row->f_bpchar == "b");
      OATPP_ASSERT(row->f_bpchar4 == "ccc ");
      OATPP_ASSERT(row->f_varchar == "dddd");
      OATPP_ASSERT(row->f_text == "eeeee");
    }

  }

}

}}}}
