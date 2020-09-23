// Copyright (C) 2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "testutils.hpp"

#include "dbutils.hpp"
#include "schema.hpp"

#include <glog/logging.h>

#include <sstream>

namespace nf
{

Json::Value
ParseJson (const std::string& val)
{
  std::istringstream in(val);
  Json::Value res;
  in >> res;
  return res;
}

DBTest::DBTest ()
  : db("test", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY)
{
  SetupDatabaseSchema (GetHandle ());
}

void
DBTest::InsertAsset (const Asset& a, const std::string& data)
{
  auto* stmt = db.Prepare (R"(
    INSERT INTO `assets`
      (`minter`, `asset`, `data`)
      VALUES (?1, ?2, ?3)
  )");

  a.BindToParams (stmt, 1, 2);
  if (data == "null")
    BindNullParam (stmt, 3);
  else
    BindParam (stmt, 3, data);

  CHECK (!StepStatement (stmt));
}

void
DBTest::InsertBalance (const Asset& a, const std::string& name,
                       const Amount num)
{
  auto* stmt = db.Prepare (R"(
    INSERT INTO `balances`
      (`name`, `minter`, `asset`, `balance`)
      VALUES (?1, ?2, ?3, ?4)
  )");

  BindParam (stmt, 1, name);
  a.BindToParams (stmt, 2, 3);
  BindParam (stmt, 4, num);

  CHECK (!StepStatement (stmt));
}

} // namespace nf
