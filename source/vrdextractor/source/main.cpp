
#include "vrdlib/common/include/CExtractor.h"

#include "vrdlib/utility/include/LoggerUtility.h"
#include "vrdlib/utility/include/CConflictHandler.h"

#include <boost/program_options.hpp>

#include <filesystem>
#include <iostream>

namespace bfs = std::filesystem;
namespace bpo = boost::program_options;

int main(int argc, char **argv)
{
   auto const executablePath(bfs::path(argv[0]).parent_path());

   bpo::options_description desc("Options");
   desc.add_options()("help,h", "Print this help message")("root,r", bpo::value<bfs::path>()->default_value(bfs::current_path()), "Root directory path")("white-list-regex, w", bpo::value<std::string>()->default_value(".*[.]CR(2|W)$"), "White list regex to filter directory entries")("dry,d", bpo::value<bool>()->default_value(true), "Dry mode, do not update any file")("log-file,l", bpo::value<bfs::path>()->default_value(bfs::current_path() / "vrdextractor.log"), "Path to log-file")("logger-config-file", bpo::value<bfs::path>(), "Path to logger configuration file");

   try
   {
      bpo::variables_map vm;
      bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
      bpo::notify(vm);

      if (vm.count("help"))
      {
         std::cout << desc << '\n';
         return 0;
      }

      bfs::path loggerConfig;
      if (vm.count("logger-config-file"))
      {
         loggerConfig = vm["logger-config-file"].as<bfs::path>();
         if (!bfs::exists(loggerConfig) || bfs::is_directory(loggerConfig))
         {
            std::cerr << "Error: Argument 'logger-config-file' is not pointing to an existing normal file: " << loggerConfig;
            return -1;
         }
      }
      else
      {
         std::vector<std::filesystem::path> const defaultLoggerConfigLocations(
             {executablePath / "etc" / "loggerConfig.xml", bfs::path("/usr") / "share" / "vrdextractor" / "loggerConfig.xml"});

         for (auto const &path : defaultLoggerConfigLocations)
         {
            if (bfs::exists(path) && !bfs::is_directory(path))
            {
               loggerConfig = path;
               break;
            }
         }
      }

      VRD::Utility::initializeLogger(
          loggerConfig, vm["log-file"].as<bfs::path>());
      VRD::CExtractor(
          vm["root"].as<bfs::path>(), vm["white-list-regex"].as<std::string>(), std::make_unique<VRD::Utility::CManualConflictHandlerFactory>(std::cin, std::cout), vm["dry"].as<bool>());
      return 0;
   }
   catch (std::exception const &e)
   {
      std::cerr << "Error: " << e.what() << '\n';
      return -1;
   }
}
