#include <string>
#include <vector>

#include <lyra/lyra.hpp>
#include <nlohmann/json.hpp>
#include <toml++/toml.h>

#include "Functions.h"
#include "ImageOperations.h"
#include "Imageio.h"
#include "VectorGridOperations.h"
#include "boilerplate/Log.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "types.h"

using namespace lyra;
using namespace std::filesystem;

using namespace Imageio;
using namespace ImageOperations;
using namespace VectorGridOperations;

int main(int argc, char **argv) {
    Log::debug("Program Started...");
    bool help_ = false;
    Path data_path;
    Path output_path;

    // Define the command line parser
    cli cli = help(help_) | opt(data_path, "data_path")["--data_path"]("Input Specs TOML file")
        | opt(output_path, "output_path")["--output_path"]("Output Specs TOML file");

    std::ostringstream helpMessage;
    helpMessage << cli;

    Log::debug("Parsing CLI...");
    parse_result result = cli.parse({argc, argv});

    Log::debug("Interpretting and loading CLI Input...");
    if (!result) {
        Log::error("Error in command line: {}", result.message());
        Log::error("CLI: {}", helpMessage.str());
        return EXIT_FAILURE;
    }
    if (help_) {
        Log::info("CLI: {}", helpMessage.str());
        return EXIT_SUCCESS;
    }

    if (!exists(data_path)) {
        Log::error("Input path does not exist");
        return EXIT_FAILURE;
    }
    if (!is_regular_file(data_path)) {
        Log::error(
            "The provided data path must be a folder or a TOML file: {}", data_path.string()
        );
        Log::error("CLI: {}", helpMessage.str());
        return EXIT_FAILURE;
    }
    if (!data_path.has_extension()) {
        Log::error(
            "The provided data path must be a folder or a TOML file: {}", data_path.string()
        );
        Log::error("CLI: {}", helpMessage.str());
        return EXIT_FAILURE;
    }
    if (data_path.extension().compare(".toml") != 0) {
        Log::error(
            "The provided data path must be a folder or a TOML file: {}", data_path.string()
        );
        Log::error("CLI: {}", helpMessage.str());
        return EXIT_FAILURE;
    }
    toml::table data_file_table = toml::parse_file(data_path.string());
    toml::table *data_table_ptr;
    try {
        data_table_ptr = data_file_table.get_as<toml::table>("Data");
        if (!data_table_ptr) throw std::runtime_error("Invalid input");
        Log::info("Loaded input data: {}", data_path.string());
    } catch (...) {
        Log::error("Input data toml file error occured: {}", data_path.string());
        Log::error("CLI: {}", helpMessage.str());
        return EXIT_FAILURE;
    }

    toml::table output_file_table;
    toml::table *output_table_ptr = nullptr;
    try {
        if (output_path.empty()) throw std::runtime_error("Fail");
        if (!exists(output_path)) throw std::runtime_error("Fail");
        if (!is_regular_file(output_path)) throw std::runtime_error("Fail");
        if (!output_path.has_extension()) throw std::runtime_error("Fail");
        if (output_path.extension().compare(".toml") != 0) throw std::runtime_error("Fail");
        output_file_table = toml::parse_file(output_path.string());
        output_table_ptr  = output_file_table.get_as<toml::table>("Output");
        if (!output_table_ptr) throw std::runtime_error("Fail");
        Log::info("Loaded output data {}: ", output_path.string());
    } catch (...) {
        Log::warning("Failure Occured when loading output, will attempt to use data file.");
        output_table_ptr = output_file_table.get_as<toml::table>("Output");
        if (output_table_ptr) {
            Log::info("Loaded output data {}: ", data_path.string());
        } else {
            Log::error("Output data toml file error occured: {}", data_path.string());
            Log::error("CLI: {}", helpMessage.str());
            return EXIT_FAILURE;
        }
    }

    SupressLibTIFF();

    Log::debug("Reading Input TOML file for Data...");
    toml::table &data_table = *data_table_ptr;
    // Get the data set ID
    std::string data_id = data_table["ID"].value_or<std::string>("");
    if (data_id.empty()) {
        Log::error("No data ID provided");
        return EXIT_FAILURE;
    }

    // Get the 'bbox' array from the nested table.
    auto tomlArray = data_table.get_as<toml::array>("bbox");
    if (!tomlArray) {
        Log::error("Bounding Box not supplied");
        return EXIT_FAILURE;
    }
    if (tomlArray->size() != 4) {
        Log::error("Bounding Box improperly specified");
        return EXIT_FAILURE;
    }
    float data_diagonal_distance;
    try {
        data_diagonal_distance = Functions::distance(
            {tomlArray->get_as<double>(0)->get(), tomlArray->get_as<double>(1)->get()},
            {tomlArray->get_as<double>(2)->get(), tomlArray->get_as<double>(3)->get()}
        );
    } catch (...) {
        Log::error("Bounding Box improperly specified");
        return EXIT_FAILURE;
    }

    // Load the View_zenith band of the data set
    Path data_ViewZenith_path = Path(data_table["ViewZenith_path"].value_or<std::string>(""));
    if (data_ViewZenith_path.empty()) {
        Log::error("No ViewZenith path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_ViewZenith;
    try {
        data_ViewZenith = ReadSingleChannelFloat(data_ViewZenith_path);
    } catch (...) {
        Log::error("Error reading ViewZenith band from path: {}", data_ViewZenith_path.string());
        return EXIT_FAILURE;
    }

    // Load the View_azimuth band of the data set
    Path data_ViewAzimuth_path = Path(data_table["ViewAzimuth_path"].value_or<std::string>(""));
    if (data_ViewAzimuth_path.empty()) {
        Log::error("No ViewAzimuth path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_ViewAzimuth;
    try {
        data_ViewAzimuth = ReadSingleChannelFloat(data_ViewAzimuth_path);
    } catch (...) {
        Log::error("Error reading ViewAzimuth band from path: {}", data_ViewAzimuth_path.string());
        return EXIT_FAILURE;
    }

    toml::table &output_table = *output_table_ptr;
    Path output_HeightVariationMetric_path
        = Path(output_table["HeightVariationMetric_path"].value_or<std::string>(""));
    if (output_HeightVariationMetric_path.empty()) {
        Log::error("No HeightVariationMetric path provided");
        return EXIT_FAILURE;
    } else {
        try {
            if (output_HeightVariationMetric_path.extension().compare(".json") != 0) {
                Log::error(
                    "HeightVariationMetric path provided is incorrect: {}",
                    output_HeightVariationMetric_path.string()
                );
                return EXIT_FAILURE;
            }
        } catch (...) {
            Log::error("HeightVariationMetric path provided is incorrect.");
            return EXIT_FAILURE;
        }
    }

    Log::debug("Initialized... Running Algorithm...");

    // The process calculations ----------------------------------
    const float DistanceToViewStart = 5.f;
    const float DistanceToViewDelta = 5.f;
    const float DistanceToViewEnd   = 2000.f;

    std::shared_ptr<VectorGrid> ViewVectorGrid
        = GenerateVectorGrid(toRadians(data_ViewZenith), toRadians(data_ViewAzimuth));
    struct Element {
        float h;
        float d;
        glm::vec3 pos;
    };
    Element temp;
    std::vector<Element> series;
    LMSPointReturn lmsresult;
    series.reserve((DistanceToViewEnd - DistanceToViewStart) / DistanceToViewDelta);
    for (float h = DistanceToViewStart; h <= DistanceToViewEnd; h += DistanceToViewDelta) {
        temp.h    = h;
        lmsresult = LSPointEqualTo(ViewVectorGrid, data_diagonal_distance, h);
        temp.pos  = lmsresult.p;
        temp.d    = AverageDotProduct(ViewVectorGrid, data_diagonal_distance, lmsresult.p);
        series.push_back(temp);
        Log::debug("Done at height: {}", h);
    }

    Log::debug("Finished Computing...Writting the output...");

    nlohmann::json evaluation_json;
    evaluation_json["ID"] = data_id;

    // Populate the m_ave_dot_series array in JSON
    for (const auto &element : series) {
        nlohmann::json jsonElement
            = {{"Height", element.h},
               {"Average Dot Product", element.d},
               {"Position", {element.pos.x, element.pos.y, element.pos.z}}};
        evaluation_json["m_ave_dot_series"].push_back(jsonElement);
    }

    // Save the JSON object to the file
    try {
        std::ofstream output_stream(output_HeightVariationMetric_path);
        if (output_stream.is_open()) {
            output_stream << evaluation_json;
            output_stream.close();
        } else {
            Log::error("Error writting output");
            return EXIT_FAILURE;
        }
    } catch (...) {
        Log::error("Error writting output");
        return EXIT_FAILURE;
    }

    Log::debug("Finished writting output");

    return EXIT_SUCCESS;
}