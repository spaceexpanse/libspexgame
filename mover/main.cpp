#include "logic.hpp"

#include "xayagame/game.hpp"
#include "xayagame/storage.hpp"

#include <jsonrpccpp/client/connectors/httpclient.h>

#include <google/protobuf/stubs/common.h>

#include <glog/logging.h>

#include <cstdlib>

int
main (int argc, char** argv)
{
  google::InitGoogleLogging (argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CHECK_EQ (argc, 2) << "Usage: moverd JSON-RPC-URL";

  const std::string jsonRpcUrl(argv[1]);
  jsonrpc::HttpClient httpConnector(jsonRpcUrl);

  xaya::Game game("mv");
  game.ConnectRpcClient (httpConnector);
  CHECK (game.DetectZmqEndpoint ());

  xaya::MemoryStorage storage;
  game.SetStorage (&storage);

  mover::MoverLogic rules;
  game.SetGameLogic (&rules);

  game.Run ();

  google::protobuf::ShutdownProtobufLibrary ();
  return EXIT_SUCCESS;
}
