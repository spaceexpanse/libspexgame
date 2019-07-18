// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GAMECHANNEL_TESTGAME_HPP
#define GAMECHANNEL_TESTGAME_HPP

#include "boardrules.hpp"
#include "channelgame.hpp"
#include "openchannel.hpp"

#include "proto/metadata.pb.h"
#include "proto/stateproof.pb.h"

#include <xayagame/testutils.hpp>
#include <xayautil/uint256.hpp>

#include <xayagame/rpc-stubs/xayarpcclient.h>
#include <xayagame/rpc-stubs/xayawalletrpcclient.h>

#include <json/json.h>
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include <gtest/gtest.h>

#include <sqlite3.h>

#include <memory>
#include <string>

namespace xaya
{

class TestGameFixture;

/**
 * Board rules for a trivial example game used in unit tests.  The game goes
 * like this:
 *
 * The current state is a pair of numbers, encoded simply in a string.  Those
 * numbers are a "current number" and the turn count.  The current
 * turn is for player (number % 2).  When the number is 100 or above, then
 * the game is finished.  A move is simply another, strictly positive number
 * encoded as a string, which gets added to the current "state number".
 * The turn count is simply incremented on each turn made.
 */
class AdditionRules : public BoardRules
{

public:

  std::unique_ptr<ParsedBoardState> ParseState (
      const uint256& channelId, const proto::ChannelMetadata& meta,
      const BoardState& s) const override;

};

/**
 * OpenChannel implementation for our test game.
 */
class AdditionChannel : public OpenChannel
{

private:

  /** If set, then automoves will be processed.  */
  bool automovesEnabled = true;

public:

  Json::Value ResolutionMove (const uint256& channelId,
                              const proto::StateProof& proof) const override;

  Json::Value DisputeMove (const uint256& channelId,
                           const proto::StateProof& proof) const override;

  /**
   * When the last digit of the current number in the addition game is 6-9,
   * we apply an automove of +2.  This way, we can test both a situation
   * where just one automove is applied (8 -> 10) and one where
   * two moves in a row are automatic (6 -> 8 -> 10).
   */
  bool MaybeAutoMove (const ParsedBoardState& state, BoardMove& mv) override;

  /**
   * If the state reached exactly 100, then we send an on-chain move (that
   * is just a string "100").  This can be triggered through auto-moves as well.
   */
  void MaybeOnChainMove (const ParsedBoardState& state,
                         MoveSender& sender) override;

  /**
   * Enables or disables processing of automoves.  When they are disabled,
   * then MaybeAutoMove will always return false, independent of the current
   * state.  This can be used to simulate situations in real games where
   * automoves may become possible for some situation only after user input
   * of some data (but not the move itself).
   */
  void
  SetAutomovesEnabled (const bool val)
  {
    automovesEnabled = val;
  }

};

/**
 * Subclass of ChannelGame that implements a trivial game only as much as
 * necessary for unit tests of the game-channel framework.
 */
class TestGame : public ChannelGame
{

protected:

  void SetupSchema (sqlite3* db) override;
  void GetInitialStateBlock (unsigned& height,
                             std::string& hashHex) const override;
  void InitialiseState (sqlite3* db) override;
  void UpdateState (sqlite3* db, const Json::Value& blockData) override;
  Json::Value GetStateAsJson (sqlite3* db) override;

  const BoardRules& GetBoardRules () const override;

  friend class TestGameFixture;

public:

  AdditionRules rules;
  AdditionChannel channel;

  using ChannelGame::ProcessDispute;
  using ChannelGame::ProcessResolution;

};

/**
 * Test fixture that constructs a TestGame instance with an in-memory database
 * and exposes that to the test itself.  It also runs a mock Xaya Core server
 * for use together with signature verification.
 */
class TestGameFixture : public testing::Test
{

private:

  jsonrpc::HttpServer httpServer;
  jsonrpc::HttpClient httpClient;

  jsonrpc::HttpServer httpServerWallet;
  jsonrpc::HttpClient httpClientWallet;

protected:

  MockXayaRpcServer mockXayaServer;
  XayaRpcClient rpcClient;

  MockXayaWalletRpcServer mockXayaWallet;
  XayaWalletRpcClient rpcWallet;

  TestGame game;

  /**
   * Initialises the test case.  This connects the game instance to an
   * in-memory database and sets up the schema on it.
   */
  TestGameFixture ();

  /**
   * The destructor shuts down the mock Xaya server.
   */
  ~TestGameFixture ();

  /**
   * Returns the raw database handle of the test game.
   */
  sqlite3* GetDb ();

  /**
   * Sets up the mock server to validate *any* message with the given
   * signature as belonging to the given address.  sgn is in raw binary
   * (will be base64-encoded for the RPC).
   */
  void ValidSignature (const std::string& sgn, const std::string& addr);

  /**
   * Expects exactly one call to verifymessage with the given message
   * and signature (both as binary, they will be hashed / base64-encoded).
   * Returns a valid response for the given address.
   */
  void ExpectSignature (const uint256& channelId,
                        const proto::ChannelMetadata& meta,
                        const std::string& topic,
                        const std::string& msg, const std::string& sgn,
                        const std::string& addr);

};

} // namespace xaya

#endif // GAMECHANNEL_TESTGAME_HPP
