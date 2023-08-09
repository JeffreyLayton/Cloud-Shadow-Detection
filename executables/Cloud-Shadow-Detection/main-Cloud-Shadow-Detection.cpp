// ---- GLAD Opengl Wrapper ---- //
#include <glad/glad.h>

// ---- Standard Library ---- //
#include <string>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

// ---- Thirdparty Libraries ---- //
#include <lyra/lyra.hpp>
#include <nlohmann/json.hpp>
#include <toml++/toml.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// ---- Project Files ---- //
#include "CloudMask.h"
#include "CloudShadowMatching.h"
#include "ComputeEnvironment.h"
#include "Functions.h"
#include "GUI.h"
#include "GaussianBlur.h"
#include "ImageOperations.h"
#include "Imageio.h"
#include "PitFillAlgorithm.h"
#include "PotentialShadowMask.h"
#include "ProbabilityRefinement.h"
#include "SceneClassificationLayer.h"
#include "ShadowMaskEvaluation.h"
#include "VectorGridOperations.h"
#include "boilerplate/GLBuffers.h"
#include "boilerplate/GLDebug.h"
#include "boilerplate/Geometry.h"
#include "boilerplate/Log.h"
#include "boilerplate/ScreenQuad.h"
#include "boilerplate/ShaderProgram.h"
#include "types.h"

using namespace lyra;

using namespace Imageio;
using namespace ImageOperations;

using namespace SceneClassificationLayer;
using namespace CloudMask;
using namespace PotentialShadowMask;
using namespace CloudShadowMatching;
using namespace VectorGridOperations;
using namespace ProbabilityRefinement;

using namespace ShadowMaskEvaluation;

int main(int argc, char **argv) {
    Log::debug("Program Started...");
    bool help_ = false;
    Path data_path;
    Path output_path;
    bool use_gui = false;

    // Define the command line parser
    cli cli = help(help_) | opt(data_path, "data_path")["--data_path"]("Input Specs TOML file")
        | opt(output_path, "output_path")["--output_path"]("Output Specs TOML file")
        | opt(use_gui)["-g"]("Run the GUI");

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

    // data_path
    //     = "C:\\Users\\jeffl\\Projects\\DGGS\\Cloud-Shadow-Detection-Result-"
    //       "Generation\\settings\\2020-06-15\\data.toml";
    // use_gui = true;

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

    SupressLibTIFF();

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
            Log::warning("Failed to load an output file, no output will be generated.");
        }
    }

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

    // Load the NIR band of the data set
    Path data_NIR_path = Path(data_table["NIR_path"].value_or<std::string>(""));
    if (data_NIR_path.empty()) {
        Log::error("No NIR path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_NIR;
    try {
        data_NIR = normalize(
            ReadSingleChannelUint16(data_NIR_path), std::numeric_limits<uint16_t>::max()
        );
    } catch (...) {
        Log::error("Error reading NIR band from path: {}", data_NIR_path.string());
        return EXIT_FAILURE;
    }
    size_t data_Size = data_NIR->size();

    // Load the CLP band of the data set
    Path data_CLP_path = Path(data_table["CLP_path"].value_or<std::string>(""));
    if (data_CLP_path.empty()) {
        Log::error("No CLP path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_CLP;
    try {
        data_CLP
            = normalize(ReadSingleChannelUint8(data_CLP_path), std::numeric_limits<uint8_t>::max());
    } catch (...) {
        Log::error("Error reading CLP band from path: {}", data_CLP_path.string());
        return EXIT_FAILURE;
    }

    // Load the CLD band of the data set
    Path data_CLD_path = Path(data_table["CLD_path"].value_or<std::string>(""));
    if (data_CLD_path.empty()) {
        Log::error("No CLD path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_CLD;
    try {
        data_CLD = normalize(ReadSingleChannelUint8(data_CLD_path), 100u);
    } catch (...) {
        Log::error("Error reading CLD band from path: {}", data_CLD_path.string());
        return EXIT_FAILURE;
    }

    // Load the View_zenith band of the data set
    Path data_ViewZenith_path = Path(data_table["ViewZenith_path"].value_or<std::string>(""));
    if (data_ViewZenith_path.empty()) {
        Log::error("No ViewZenith_path path provided");
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

    // Load the Sun_zenith band of the data set
    Path data_SunZenith_path = Path(data_table["SunZenith_path"].value_or<std::string>(""));
    if (data_SunZenith_path.empty()) {
        Log::error("No SunZenith_path path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_SunZenith;
    try {
        data_SunZenith = ReadSingleChannelFloat(data_SunZenith_path);
    } catch (...) {
        Log::error("Error reading SunZenith band from path: {}", data_SunZenith_path.string());
        return EXIT_FAILURE;
    }

    // Load the Sun_azimuth band of the data set
    Path data_SunAzimuth_path = Path(data_table["SunAzimuth_path"].value_or<std::string>(""));
    if (data_SunAzimuth_path.empty()) {
        Log::error("No SunAzimuth path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageFloat> data_SunAzimuth;
    try {
        data_SunAzimuth = ReadSingleChannelFloat(data_SunAzimuth_path);
    } catch (...) {
        Log::error("Error reading SunAzimuth band from path: {}", data_SunAzimuth_path.string());
        return EXIT_FAILURE;
    }

    // Load the SCL band of the data set
    Path data_SCL_path = Path(data_table["SCL_path"].value_or<std::string>(""));
    if (data_SCL_path.empty()) {
        Log::error("No SCL path provided");
        return EXIT_FAILURE;
    }
    std::shared_ptr<ImageUint> data_SCL;
    try {
        data_SCL = ReadSingleChannelUint8(data_SCL_path);
    } catch (...) {
        Log::error("Error reading SCL band from path: {}", data_SCL_path.string());
        return EXIT_FAILURE;
    }

    // Load the RBGA band of the data set
    Path data_RBGA_path = Path(data_table["RBGA_path"].value_or<std::string>(""));
    std::shared_ptr<ImageUint> data_RBGA
        = std::make_shared<ImageUint>(data_NIR->rows(), data_NIR->cols());
    if (data_RBGA_path.empty()) {
        Log::warning("No RBGA path provided");
    } else {
        try {
            data_RBGA = ReadRGBA(data_RBGA_path);
        } catch (...) {
            Log::warning("Error reading RBGA band from path: {}", data_RBGA_path.string());
        }
    }

    // Load the Shadowbaseline band of the data set
    Path data_ShadowBaseline_path
        = Path(data_table["ShadowBaseline_path"].value_or<std::string>(""));
    std::shared_ptr<ImageBool> data_ShadowBaseline
        = std::make_shared<ImageBool>(data_NIR->rows(), data_NIR->cols());
    if (data_ShadowBaseline_path.empty()) {
        Log::warning("No Baseline path provided");
    } else {
        try {
            std::vector<float> data_ShadowBaseline_raw
                = ImageOperations::decomposeRBGA(ReadRGBA(data_ShadowBaseline_path));
            glm::vec3 true_value = {0.f, 1.f, 0.f};
            for (int i = 0; i < data_ShadowBaseline->size(); i++) {
                data_ShadowBaseline->data()[i]
                    = Functions::equal(true_value.x, data_ShadowBaseline_raw[4 * i + 0], 1e-8)
                    && Functions::equal(true_value.y, data_ShadowBaseline_raw[4 * i + 1], 1e-8)
                    && Functions::equal(true_value.z, data_ShadowBaseline_raw[4 * i + 2], 1e-8);
            }
        } catch (...) {
            Log::warning(
                "Error reading RBGA band from path: {}", data_ShadowBaseline_path.string()
            );
            data_ShadowBaseline_path = "";
            data_ShadowBaseline      = nullptr;
        }
    }

    //---------------------------------------------------------------------------------------------------
    Path output_CM_path, output_PSM_path, output_OSM_path, output_FSM_path, output_Alpha_path,
        output_Beta_path, output_PSME_path, output_OSME_path, output_FSME_path,
        output_EvaluationMetric_path;
    if (output_table_ptr) {
        toml::table &output_table = *output_table_ptr;
        Log::debug("Reading Output TOML file for Data...");
        output_CM_path = Path(output_table["CM_path"].value_or<std::string>(""));
        if (output_CM_path.empty()) {
            Log::warning("No CM path provided");
        } else {
            try {
                if (output_CM_path.extension().compare(".tif") != 0) {
                    Log::warning("CM path provided is invalid: {}", output_CM_path.string());
                    output_CM_path = "";
                }
            } catch (...) {
                Log::warning("CM path provided is invalid: {}", output_CM_path.string());
                output_CM_path = "";
            }
        }

        output_PSM_path = Path(output_table["PSM_path"].value_or<std::string>(""));
        if (output_PSM_path.empty()) {
            Log::warning("No PSM path provided");
        } else {
            try {
                if (output_PSM_path.extension().compare(".tif") != 0) {
                    Log::warning("PSM path provided is invalid: {}", output_PSM_path.string());
                    output_PSM_path = "";
                }
            } catch (...) {
                Log::warning("PSM path provided is invalid: {}", output_PSM_path.string());
                output_PSM_path = "";
            }
        }

        output_OSM_path = Path(output_table["OSM_path"].value_or<std::string>(""));
        if (output_OSM_path.empty()) {
            Log::warning("No OSM path provided");
        } else {
            try {
                if (output_OSM_path.extension().compare(".tif") != 0) {
                    Log::warning("OSM path provided is invalid: {}", output_OSM_path.string());
                    output_OSM_path = "";
                }
            } catch (...) {
                Log::warning("OSM path provided is invalid: {}", output_OSM_path.string());
                output_OSM_path = "";
            }
        }

        output_FSM_path = Path(output_table["FSM_path"].value_or<std::string>(""));
        if (output_FSM_path.empty()) {
            Log::warning("No FSM path provided");
        } else {
            try {
                if (output_FSM_path.extension().compare(".tif") != 0) {
                    Log::warning("FSM path provided is invalid: {}", output_FSM_path.string());
                    output_FSM_path = "";
                }
            } catch (...) {
                Log::warning("FSM path provided is invalid: {}", output_FSM_path.string());
                output_FSM_path = "";
            }
        }

        output_Alpha_path = Path(output_table["Alpha_path"].value_or<std::string>(""));
        if (output_Alpha_path.empty()) {
            Log::warning("No Alpha path provided");
        } else {
            try {
                if (output_Alpha_path.extension().compare(".tif") != 0) {
                    Log::warning("Alpha path provided is invalid: {}", output_Alpha_path.string());
                    output_Alpha_path = "";
                }
            } catch (...) {
                Log::warning("Alpha path provided is invalid: {}", output_Alpha_path.string());
                output_Alpha_path = "";
            }
        }

        output_Beta_path = Path(output_table["Beta_path"].value_or<std::string>(""));
        if (output_Beta_path.empty()) {
            Log::warning("No Beta path provided");
        } else {
            try {
                if (output_Beta_path.extension().compare(".tif") != 0) {
                    Log::warning("Beta path provided is invalid: {}", output_Beta_path.string());
                    output_Beta_path = "";
                }
            } catch (...) {
                Log::warning("Beta path provided is invalid: {}", output_Beta_path.string());
                output_Beta_path = "";
            }
        }

        output_PSME_path = Path(output_table["PSME_path"].value_or<std::string>(""));
        if (output_PSME_path.empty()) {
            Log::warning("No PSME path provided");
        } else {
            try {
                if (output_PSME_path.extension().compare(".tif") != 0) {
                    Log::warning("PSME path provided is invalid: {}", output_PSME_path.string());
                    output_PSME_path = "";
                }
            } catch (...) {
                Log::warning("PSME path provided is invalid: {}", output_PSME_path.string());
                output_PSME_path = "";
            }
        }

        output_OSME_path = Path(output_table["OSME_path"].value_or<std::string>(""));
        if (output_OSME_path.empty()) {
            Log::warning("No OSME path provided");
        } else {
            try {
                if (output_OSME_path.extension().compare(".tif") != 0) {
                    Log::warning("OSME path provided is invalid: {}", output_OSME_path.string());
                    output_OSME_path = "";
                }
            } catch (...) {
                Log::warning("OSME path provided is invalid: {}", output_OSME_path.string());
                output_OSME_path = "";
            }
        }

        output_FSME_path = Path(output_table["FSME_path"].value_or<std::string>(""));
        if (output_FSME_path.empty()) {
            Log::warning("No FSME path provided");
        } else {
            try {
                if (output_FSME_path.extension().compare(".tif") != 0) {
                    Log::warning("FSME path provided is invalid: {}", output_FSME_path.string());
                    output_FSME_path = "";
                }
            } catch (...) {
                Log::warning("FSME path provided is invalid: {}", output_FSME_path.string());
                output_FSME_path = "";
            }
        }

        output_EvaluationMetric_path
            = Path(output_table["EvaluationMetric_path"].value_or<std::string>(""));
        if (output_EvaluationMetric_path.empty()) {
            Log::warning("No EvaluationMetric path provided");
        } else {
            try {
                if (output_EvaluationMetric_path.extension().compare(".json") != 0) {
                    Log::warning(
                        "EvaluationMetric path provided is invalid: {}",
                        output_EvaluationMetric_path.string()
                    );
                    output_EvaluationMetric_path = "";
                }
            } catch (...) {
                Log::warning(
                    "EvaluationMetric path provided is invalid: {}",
                    output_EvaluationMetric_path.string()
                );
                output_EvaluationMetric_path = "";
            }
        }
    }

    Log::debug("Initizing Computing Context...");

    ComputeEnvironment::InitMainContext();
    GaussianBlur::init();
    PitFillAlgorithm::init();

    Log::debug("Running Algorithm...");

    // The process calculations ----------------------------------

    const int MinimimumCloudSizeForRayCasting = 3;
    const float DistanceToSun                 = 1.5e9f;
    const float DistanceToView                = 785.f;
    const float ProbabilityFunctionThreshold  = .15f;

    Log::debug(" --- Cloud Detection...");
    // Generate the Cloud mask along with the intermediate result of the blended cloud probability
    GenerateCloudMaskReturn GenerateCloudMask_Return
        = GenerateCloudMask(data_CLP, data_CLD, data_SCL);
    std::shared_ptr<ImageFloat> &BlendedCloudProbability
        = GenerateCloudMask_Return.blendedCloudProbability;
    std::shared_ptr<ImageBool> &output_CM = GenerateCloudMask_Return.cloudMask;

    Log::debug(" --- Cloud Partitioning...");
    // Using the Cloud mask, partition it into individual clouds with collections and a map
    PartitionCloudMaskReturn PartitionCloudMask_Return
        = PartitionCloudMask(output_CM, data_diagonal_distance, MinimimumCloudSizeForRayCasting);
    CloudQuads &Clouds                   = PartitionCloudMask_Return.clouds;
    std::shared_ptr<ImageInt> &CloudsMap = PartitionCloudMask_Return.map;

    Log::debug(" --- Potential Shadow Mask Generation...");
    // Generate the Candidate (or Potential) Shadow Mask
    PotentialShadowMaskGenerationReturn GeneratePotentialShadowMask_Return
        = GeneratePotentialShadowMask(data_NIR, output_CM, data_SCL);
    std::shared_ptr<ImageBool> output_PSM = GeneratePotentialShadowMask_Return.mask;
    std::shared_ptr<ImageFloat> DeltaNIR
        = GeneratePotentialShadowMask_Return.difference_of_pitfill_NIR;

    Log::debug(" --- Solving for Sun and Satillite Position...");
    // Generate a Vector grid for each
    std::shared_ptr<VectorGrid> SunVectorGrid
        = GenerateVectorGrid(toRadians(data_SunZenith), toRadians(data_SunAzimuth));
    std::shared_ptr<VectorGrid> ViewVectorGrid
        = GenerateVectorGrid(toRadians(data_ViewZenith), toRadians(data_ViewAzimuth));
    LMSPointReturn SunLSPointEqualTo_Return
        = LSPointEqualTo(SunVectorGrid, data_diagonal_distance, DistanceToSun);
    glm::vec3 &SunPosition = SunLSPointEqualTo_Return.p;
    LMSPointReturn ViewLSPointEqualTo_Return
        = LSPointEqualTo(ViewVectorGrid, data_diagonal_distance, DistanceToView);
    glm::vec3 &ViewPosition = ViewLSPointEqualTo_Return.p;
    float output_MDPSun     = AverageDotProduct(SunVectorGrid, data_diagonal_distance, SunPosition);
    float output_MDPView = AverageDotProduct(ViewVectorGrid, data_diagonal_distance, ViewPosition);

    Log::debug(" --- Object-based Shadow Mask Generation...");
    // Solve for the optimal shadow matching results per cloud
    MatchCloudsShadowsResults MatchCloudsShadows_Return = MatchCloudsShadows(
        Clouds, CloudsMap, output_CM, output_PSM, data_diagonal_distance, SunPosition, ViewPosition
    );
    std::map<int, OptimalSolution> &OptimalCloudCastingSolutions
        = MatchCloudsShadows_Return.solutions;
    ShadowQuads &CloudCastedShadows        = MatchCloudsShadows_Return.shadows;
    std::shared_ptr<ImageBool> &output_OSM = MatchCloudsShadows_Return.shadowMask;
    float &TrimmedMeanCloudHeight          = MatchCloudsShadows_Return.trimmedMeanHeight;

    Log::debug(" --- Generating Probability Function...");
    // Generate the Alpha and Beta maps to produce the probability surface
    std::shared_ptr<ImageFloat> output_Alpha = ProbabilityRefinement::AlphaMap(DeltaNIR);
    std::shared_ptr<ImageFloat> output_Beta  = ProbabilityRefinement::BetaMap(
        CloudCastedShadows,
        OptimalCloudCastingSolutions,
        output_CM,
        output_OSM,
        BlendedCloudProbability,
        data_diagonal_distance
    );
    UniformProbabilitySurface ProbabilityFunction
        = ProbabilityMap(output_OSM, output_Alpha, output_Beta);

    Log::debug(" --- Final Shadow Mask Generation...");
    std::shared_ptr<ImageBool> output_FSM = ImprovedShadowMask(
        output_OSM,
        output_CM,
        output_Alpha,
        output_Beta,
        ProbabilityFunction,
        ProbabilityFunctionThreshold
    );
    Log::debug("...Finished Algorithm.");
    Log::debug("Evaluating data...");
    ImageBounds output_EvaluationBounds = CastedImageBounds(
        output_PSM, data_diagonal_distance, SunPosition, ViewPosition, TrimmedMeanCloudHeight
    );
    Results PSM_results
        = Evaluate(output_PSM, output_CM, data_ShadowBaseline, output_EvaluationBounds);
    std::shared_ptr<ImageUint> &output_PSME = PSM_results.pixel_classes;
    Results OSM_results
        = Evaluate(output_OSM, output_CM, data_ShadowBaseline, output_EvaluationBounds);
    std::shared_ptr<ImageUint> &output_OSME = OSM_results.pixel_classes;
    Results FSM_results
        = Evaluate(output_FSM, output_CM, data_ShadowBaseline, output_EvaluationBounds);
    std::shared_ptr<ImageUint> &output_FSME = FSM_results.pixel_classes;

    Log::debug("Writing Output According to Output TOML file...");
    if (!output_CM_path.empty()) {
        try {
            WriteSingleChannelUint8(output_CM_path, cast<unsigned int>(output_CM, 1u, 0u));
        } catch (...) { Log::error("Failed to Write CM tif"); }
    }
    if (!output_PSM_path.empty()) {
        try {
            WriteSingleChannelUint8(output_PSM_path, cast<unsigned int>(output_PSM, 1u, 0u));
        } catch (...) { Log::error("Failed to Write PSM tif"); }
    }
    if (!output_OSM_path.empty()) {
        try {
            WriteSingleChannelUint8(output_OSM_path, cast<unsigned int>(output_OSM, 1u, 0u));
        } catch (...) { Log::error("Failed to Write OSM tif"); }
    }
    if (!output_FSM_path.empty()) {
        try {
            WriteSingleChannelUint8(output_FSM_path, cast<unsigned int>(output_FSM, 1u, 0u));
        } catch (...) { Log::error("Failed to Write FSM tif"); }
    }
    if (!output_Alpha_path.empty()) {
        try {
            WriteSingleChannelFloat(output_Alpha_path, output_Alpha);
        } catch (...) { Log::error("Failed to Write Alpha tif"); }
    }
    if (!output_Beta_path.empty()) {
        try {
            WriteSingleChannelFloat(output_Beta_path, output_Beta);
        } catch (...) { Log::error("Failed to Write Beta tif"); }
    }
    if (!output_PSME_path.empty()) {
        try {
            WriteSingleChannelUint8(output_PSME_path, output_PSME);
        } catch (...) { Log::error("Failed to Write PSME tif"); }
    }
    if (!output_OSME_path.empty()) {
        try {
            WriteSingleChannelUint8(output_OSME_path, output_OSME);
        } catch (...) { Log::error("Failed to Write OSME tif"); }
    }
    if (!output_FSME_path.empty()) {
        try {
            WriteSingleChannelUint8(output_FSME_path, output_FSME);
        } catch (...) { Log::error("Failed to Write FSME tif"); }
    }
    if (!output_EvaluationMetric_path.empty()) {
        nlohmann::json evaluation_json;
        evaluation_json["ID"]                          = data_id;
        evaluation_json["Baselined"]                   = !data_ShadowBaseline_path.empty();
        evaluation_json["Sun"]["Average Dot Product"]  = output_MDPSun;
        evaluation_json["View"]["Average Dot Product"] = output_MDPView;
        evaluation_json["Bounds"]["x"]["min"]          = output_EvaluationBounds.p0.x;
        evaluation_json["Bounds"]["x"]["max"]          = output_EvaluationBounds.p1.x;
        evaluation_json["Bounds"]["y"]["min"]          = output_EvaluationBounds.p0.y;
        evaluation_json["Bounds"]["y"]["max"]          = output_EvaluationBounds.p1.y;

        evaluation_json["Potential Shadow Mask"]["Users Accuracy"] = PSM_results.users_accuracy;
        evaluation_json["Potential Shadow Mask"]["Producers Accuracy"]
            = PSM_results.producers_accuracy;
        evaluation_json["Potential Shadow Mask"]["False Positives Relative to Total Pixels"]
            = PSM_results.positive_error_total;
        evaluation_json["Potential Shadow Mask"]["False Negatives Relative to Total Pixels"]
            = PSM_results.negative_error_total;
        evaluation_json["Potential Shadow Mask"]["False Pixels Relative to Total Pixels"]
            = PSM_results.error_total;
        evaluation_json["Potential Shadow Mask"]["False Positives Relative to Shadow Pixels"]
            = PSM_results.positive_error_relative;
        evaluation_json["Potential Shadow Mask"]["False Negatives Relative to Shadow Pixels"]
            = PSM_results.negative_error_relative;
        evaluation_json["Potential Shadow Mask"]["False Pixels Relative to Shadow Pixels"]
            = PSM_results.error_relative;

        evaluation_json["Object-based Shadow Mask"]["Users Accuracy"] = OSM_results.users_accuracy;
        evaluation_json["Object-based Shadow Mask"]["Producers Accuracy"]
            = OSM_results.producers_accuracy;
        evaluation_json["Object-based Shadow Mask"]["False Positives Relative to Total Pixels"]
            = OSM_results.positive_error_total;
        evaluation_json["Object-based Shadow Mask"]["False Negatives Relative to Total Pixels"]
            = OSM_results.negative_error_total;
        evaluation_json["Object-based Shadow Mask"]["False Pixels Relative to Total Pixels"]
            = OSM_results.error_total;
        evaluation_json["Object-based Shadow Mask"]["False Positives Relative to Shadow Pixels"]
            = OSM_results.positive_error_relative;
        evaluation_json["Object-based Shadow Mask"]["False Negatives Relative to Shadow Pixels"]
            = OSM_results.negative_error_relative;
        evaluation_json["Object-based Shadow Mask"]["False Pixels Relative to Shadow Pixels"]
            = OSM_results.error_relative;

        evaluation_json["Final Shadow Mask"]["Users Accuracy"]     = FSM_results.users_accuracy;
        evaluation_json["Final Shadow Mask"]["Producers Accuracy"] = FSM_results.producers_accuracy;
        evaluation_json["Final Shadow Mask"]["False Positives Relative to Total Pixels"]
            = FSM_results.positive_error_total;
        evaluation_json["Final Shadow Mask"]["False Negatives Relative to Total Pixels"]
            = FSM_results.negative_error_total;
        evaluation_json["Final Shadow Mask"]["False Pixels Relative to Total Pixels"]
            = FSM_results.error_total;
        evaluation_json["Final Shadow Mask"]["False Positives Relative to Shadow Pixels"]
            = FSM_results.positive_error_relative;
        evaluation_json["Final Shadow Mask"]["False Negatives Relative to Shadow Pixels"]
            = FSM_results.negative_error_relative;
        evaluation_json["Final Shadow Mask"]["False Pixels Relative to Shadow Pixels"]
            = FSM_results.error_relative;
        try {
            std::ofstream outputFile(output_EvaluationMetric_path);
            outputFile << evaluation_json;
            outputFile.close();
        } catch (...) { Log::error("Failed to Write Evaluation JSON"); }
    }

    // Will perform a render loop in custom viewer
    if (use_gui) {
        Log::debug("Booting up GUI...");
        auto side_lengths = ImageOperations::sides<float>(data_NIR, data_diagonal_distance);
        float major_i     = float(std::max(data_NIR->rows(), data_NIR->cols()));
        float major_v     = std::max(side_lengths.x, side_lengths.y);
        glm::mat4 center
            = {{2.f, 0.f, 0.f, 0.f},
               {0.f, 2.f, 0.f, 0.f},
               {0.f, 0.f, 1.f, 0.f},
               {-1.f, -1.f, 0.f, 1.f}};
        glm::mat4 quad_model  = glm::scale(glm::mat4(1.f), glm::vec3(side_lengths / major_v, 1.f));
        glm::mat4 image_model = quad_model * center * inverse(quad_model)
            * glm::scale(glm::mat4(1.f), glm::vec3(1.f / major_i));
        glm::mat4 vector_model = quad_model * center * inverse(quad_model)
            * glm::scale(glm::mat4(1.f), glm::vec3(1.f / major_v));

        glfwInit();
        std::shared_ptr<Window> window
            = std::make_shared<Window>(1200, 800, "Cloud Shadow Detection");
        std::shared_ptr<GUI> gui = std::make_shared<GUI>(glm::uvec2(1200, 800));
        window->setCallbacks(gui);
        // Image Viewer Programs and geometry --------------------------------
        std::shared_ptr<ShaderProgram> BasicShaderProgram
            = std::make_shared<ShaderProgram>("shaders/Basic.vert", "shaders/Basic.frag");

        std::shared_ptr<ShaderProgram> FloatImageViewerProgram = std::make_shared<ShaderProgram>(
            "shaders/FloatImageViewer.vert", "shaders/FloatImageViewer.frag"
        );
        std::shared_ptr<ShaderProgram> RGBAImageViewerProgram = std::make_shared<ShaderProgram>(
            "shaders/Float4ImageViewer.vert", "shaders/Float4ImageViewer.frag"
        );
        std::shared_ptr<ShaderProgram> SurfaceViewerProgram = std::make_shared<ShaderProgram>(
            "shaders/ProbabilitySurfaceRender.vert",
            "shaders/ProbabilitySurfaceRender.geom",
            "shaders/ProbabilitySurfaceRender.frag"
        );
        std::shared_ptr<ShaderProgram> VectorGridViewerProgram = std::make_shared<ShaderProgram>(
            "shaders/VectorGridViewer.vert",
            "shaders/VectorGridViewer.geom",
            "shaders/VectorGridViewer.frag"
        );
        std::shared_ptr<ShaderProgram> CurrentImageViewerProgram = RGBAImageViewerProgram;

        std::shared_ptr<ScreenQuad> quad = std::make_shared<ScreenQuad>(false, true);

        std::shared_ptr<ShaderStorageBuffer> main_viewer_ssbo
            = std::make_shared<ShaderStorageBuffer>();

        std::string current_image;
        std::shared_ptr<Vert_Geometry> line_geom          = std::make_shared<Vert_Geometry>();
        std::shared_ptr<Vert_Geometry> sun_view_geom      = std::make_shared<Vert_Geometry>();
        std::shared_ptr<Coord_Geometry> vector_geom       = std::make_shared<Coord_Geometry>();
        std::shared_ptr<ShaderStorageBuffer> surface_ssbo = std::make_shared<ShaderStorageBuffer>();
        std::shared_ptr<Coord_Geometry> surface_geom      = std::make_shared<Coord_Geometry>();

        //-----------------------------------------------------

        //-----------------------------------------------------
        std::vector<float> null_data(data_Size, 0.f);

        gui->registerView("Initial RGB Image");
        std::vector<float> RGBA_data = decomposeRBGA(data_RBGA);

        gui->registerView("Initial NIR Band");

        gui->registerView("Initial CLP Band");

        gui->registerView("Initial CLD Band");

        gui->registerView("Initial SCL Image");
        std::vector<float> SCL_data
            = decomposeRBGA(SceneClassificationLayer::GenerateRGBA(data_SCL));

        gui->registerView("Initial View Zenith Angle Band");
        std::shared_ptr<ImageFloat> viewable_ViewZenith = normalize(data_ViewZenith, 360.f);

        gui->registerView("Initial View Azimuth Angle Band");
        std::shared_ptr<ImageFloat> viewable_ViewAzimuth = normalize(data_ViewAzimuth, 360.f);

        gui->registerView("Initial Sun Zenith Angle Band");
        std::shared_ptr<ImageFloat> viewable_SunZenith = normalize(data_SunZenith, 360.f);

        gui->registerView("Initial Sun Azimuth Angle Band");
        std::shared_ptr<ImageFloat> viewable_SunAzimuth = normalize(data_SunAzimuth, 360.f);

        gui->registerView("Generated Cloud Mask");
        std::shared_ptr<ImageFloat> viewable_CloudMask = cast<float>(output_CM);
        std::shared_ptr<ImageUint> CloudMaskOverlay    = obscure(data_RBGA, output_CM, 0xffffffff);

        gui->registerView("Generated View Vector Grid");
        glm::vec3 ave_view_dir = AverageDirection(ViewVectorGrid);

        gui->registerView("Generated Sun Vector Grid");
        glm::vec3 ave_sun_dir = AverageDirection(SunVectorGrid);

        const int deltaVectorGrid = 32;
        sun_view_geom->setVerts({SunPosition, ViewPosition});
        std::vector<glm::ivec3> vector_grid(deltaVectorGrid * deltaVectorGrid);
        for (int i = 0; i < deltaVectorGrid; i++)
            for (int j = 0; j < deltaVectorGrid; j++)
                vector_grid[i + deltaVectorGrid * j] = glm::ivec3(i, j, 0);
        vector_geom->setCoords(vector_grid);

        gui->registerView("Generated Potential Shadow Mask");
        std::shared_ptr<ImageFloat> viewable_PotentialShadowMask = cast<float>(output_PSM);

        gui->registerView("Generated Potential Shadow Mask Overlay");
        std::shared_ptr<ImageUint> PotentialShadowMaskOverlay
            = obscure(CloudMaskOverlay, output_PSM, 0xff0000ff);
        std::vector<float> viewable_PotentialShadowMaskOverlay
            = decomposeRBGA(PotentialShadowMaskOverlay);

        gui->registerView("Generated Object-Based Shadow Mask");
        std::shared_ptr<ImageFloat> viewable_ObjectBasedShadowMask = cast<float>(output_OSM);

        gui->registerView("Generated Object-Based Shadow Mask Overlay");
        std::shared_ptr<ImageUint> ObjectBasedShadowMaskOverlay
            = obscure(CloudMaskOverlay, output_OSM, 0xff0000ff);
        std::vector<float> viewable_ObjectBasedShadowMaskOverlay
            = decomposeRBGA(ObjectBasedShadowMaskOverlay);

        gui->registerView("Generated Alpha(Shadow Value) Map");

        gui->registerView("Generated Beta(Shadow Projected Probability) Map");

        gui->registerView("Generated Probability Surface");
        SurfaceRenderGeom surface_render_geom;

        gui->registerView("Generated Final Shadow Mask");
        std::shared_ptr<ImageUint> FinalShadowMaskOverlay
            = obscure(CloudMaskOverlay, output_FSM, 0xff0000ff);
        std::shared_ptr<ImageFloat> viewable_FinalShadowMask = cast<float>(output_FSM);

        gui->registerView("Generated Final Shadow Mask Overlay");
        std::vector<float> viewable_FinalShadowMaskOverlay = decomposeRBGA(FinalShadowMaskOverlay);

        gui->registerView("Shadow Baseline");
        std::shared_ptr<ImageFloat> viewable_ShadowBaseline = cast<float>(data_ShadowBaseline);

        gui->registerView("Evaluated Potential Shadow Mask");
        std::vector<float> viewable_PotentialShadowMaskEvaluation
            = decomposeRBGA(ShadowMaskEvaluation::GenerateRGBA(output_PSME));

        gui->registerView("Evaluated Object-Based Shadow Mask");
        std::vector<float> viewable_ObjectBasedlShadowMaskEvaluation
            = decomposeRBGA(ShadowMaskEvaluation::GenerateRGBA(output_OSME));

        gui->registerView("Evaluated Final Shadow Mask");
        std::vector<float> viewable_FinalShadowMaskEvaluation
            = decomposeRBGA(ShadowMaskEvaluation::GenerateRGBA(output_FSME));
        //-----------------------------------------------------

        // RENDER LOOP
        while (!window->shouldClose()) {
            glfwPollEvents();
            // Continous Update ------------------------------------------------------------
            glm::vec2 mouse_pos_vector_space = glm::vec2(
                glm::inverse(
                    gui->getProjectionMatrix() * gui->getViewMatrix()
                    * glm::scale(glm::mat4(1.f), glm::vec3(1., -1., 1.)) * vector_model
                )
                * glm::vec4(gui->getMousePos(), 0.f, 1.f)
            );
            glm::ivec2 quad_cull_bounds = gui->getCloudQuadMinMax();
            // View ------------------------------------------------------------------------
            current_image = gui->currentView();
            if (Functions::equal(current_image, "Initial RGB Image")) {
                main_viewer_ssbo->uploadData(
                    RGBA_data.size() * sizeof(float), RGBA_data.data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial SCL Image")) {
                main_viewer_ssbo->uploadData(
                    SCL_data.size() * sizeof(float), SCL_data.data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial NIR Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), data_NIR->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(data_NIR->data(), data_Size);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial CLP Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), data_CLP->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(data_CLP->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial CLD Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), data_CLD->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(data_CLD->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial View Zenith Angle Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_ViewZenith->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_ViewZenith->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial View Azimuth Angle Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_ViewAzimuth->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_ViewAzimuth->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial Sun Zenith Angle Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_SunZenith->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_SunZenith->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Initial Sun Azimuth Angle Band")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_SunAzimuth->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_SunAzimuth->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            }
            // ---------
            else if (Functions::equal(current_image, "Shadow Baseline")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_ShadowBaseline->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_ShadowBaseline->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            }
            // ---------
            else if (Functions::equal(current_image, "Generated View Vector Grid")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), data_NIR->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(true);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Sun Vector Grid")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), data_NIR->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(true);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Cloud Mask")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_CloudMask->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_CloudMask->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Potential Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_PotentialShadowMask->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_PotentialShadowMask->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Potential Shadow Mask Overlay")) {
                main_viewer_ssbo->uploadData(
                    viewable_PotentialShadowMaskOverlay.size() * sizeof(float),
                    viewable_PotentialShadowMaskOverlay.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Object-Based Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float),
                    viewable_ObjectBasedShadowMask->data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_ObjectBasedShadowMask->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(
                           current_image, "Generated Object-Based Shadow Mask Overlay"
                       )) {
                main_viewer_ssbo->uploadData(
                    viewable_ObjectBasedShadowMaskOverlay.size() * sizeof(float),
                    viewable_ObjectBasedShadowMaskOverlay.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Alpha(Shadow Value) Map")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), output_Alpha->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(output_Alpha->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(
                           current_image, "Generated Beta(Shadow Projected Probability) Map"
                       )) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), output_Beta->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(output_Beta->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Final Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    data_Size * sizeof(float), viewable_FinalShadowMask->data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(viewable_FinalShadowMask->data(), data_Size);
                gui->set3DView(false);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            } else if (Functions::equal(current_image, "Generated Final Shadow Mask Overlay")) {
                main_viewer_ssbo->uploadData(
                    viewable_FinalShadowMaskOverlay.size() * sizeof(float),
                    viewable_FinalShadowMaskOverlay.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            }
            // ---------
            else if (Functions::equal(current_image, "Evaluated Potential Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    viewable_PotentialShadowMaskEvaluation.size() * sizeof(float),
                    viewable_PotentialShadowMaskEvaluation.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Evaluated Object-Based Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    viewable_ObjectBasedlShadowMaskEvaluation.size() * sizeof(float),
                    viewable_ObjectBasedlShadowMaskEvaluation.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            } else if (Functions::equal(current_image, "Evaluated Final Shadow Mask")) {
                main_viewer_ssbo->uploadData(
                    viewable_FinalShadowMaskEvaluation.size() * sizeof(float),
                    viewable_FinalShadowMaskEvaluation.data(),
                    GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(false);
                CurrentImageViewerProgram = RGBAImageViewerProgram;
            }
            // ---------
            else {
                main_viewer_ssbo->uploadData(
                    null_data.size() * sizeof(float), null_data.data(), GL_STATIC_DRAW
                );
                gui->setImageHistogram(nullptr, 0);
                gui->set3DView(true);
                CurrentImageViewerProgram = FloatImageViewerProgram;
            }
            //----------------------------------------------------------------------------------
            glEnable(GL_LINE_SMOOTH);
            glPointSize(15.f);
            // glEnable(GL_FRAMEBUFFER_SRGB);
            auto clear_color = gui->getClearColor();
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (!Functions::equal(current_image, "Generated Probability Surface")) {
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                CurrentImageViewerProgram->use();
                CurrentImageViewerProgram->setMat4(quad_model, "M");
                CurrentImageViewerProgram->setMat4(gui->getViewMatrix(), "V");
                CurrentImageViewerProgram->setMat4(gui->getProjectionMatrix(), "P");
                main_viewer_ssbo->bindBase(0);
                CurrentImageViewerProgram->setUint(data_NIR->cols(), "inputImageWidth");
                CurrentImageViewerProgram->setUint(data_NIR->rows(), "inputImageHeight");
                CurrentImageViewerProgram->setUint(data_Size, "inputImageSize");
                quad->draw();

                if (Functions::equal(current_image, "Generated Sun Vector Grid")) {
                    main_viewer_ssbo->uploadData(
                        data_Size * sizeof(glm::vec3), SunVectorGrid->data(), GL_STATIC_DRAW
                    );
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    VectorGridViewerProgram->use();
                    VectorGridViewerProgram->setMat4(image_model, "M");
                    VectorGridViewerProgram->setMat4(gui->getViewMatrix(), "V");
                    VectorGridViewerProgram->setMat4(gui->getProjectionMatrix(), "P");
                    main_viewer_ssbo->bindBase(0);
                    VectorGridViewerProgram->setUint(data_NIR->cols(), "inputImageWidth");
                    VectorGridViewerProgram->setUint(data_NIR->rows(), "inputImageHeight");
                    VectorGridViewerProgram->setUint(data_Size, "inputImageSize");
                    VectorGridViewerProgram->setVec4v(
                        {{1.f, 0.f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f}}, "cols"
                    );
                    VectorGridViewerProgram->setIvec2({deltaVectorGrid, deltaVectorGrid}, "divs");
                    VectorGridViewerProgram->setVec2({100.f, 300.f}, "vec_lengths");
                    VectorGridViewerProgram->setVec3(ave_sun_dir, "ave_dir");
                    VectorGridViewerProgram->setFloat(0.01, "forward_dir_weight");
                    VectorGridViewerProgram->setInt(0, "remove_dc_dir");
                    vector_geom->bind();
                    glDrawArrays(GL_POINTS, 0, vector_grid.size());

                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    BasicShaderProgram->use();
                    BasicShaderProgram->setMat4(vector_model, "M");
                    BasicShaderProgram->setMat4(gui->getViewMatrix(), "V");
                    BasicShaderProgram->setMat4(gui->getProjectionMatrix(), "P");
                    BasicShaderProgram->setVec4({1.f, 0.f, 0.f, 0.f}, "col");
                    sun_view_geom->bind();
                    glDrawArrays(GL_POINTS, 0, 1);
                }

                if (Functions::equal(current_image, "Generated View Vector Grid")) {
                    main_viewer_ssbo->uploadData(
                        data_Size * sizeof(glm::vec3), ViewVectorGrid->data(), GL_STATIC_DRAW
                    );
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    VectorGridViewerProgram->use();
                    VectorGridViewerProgram->setMat4(image_model, "M");
                    VectorGridViewerProgram->setMat4(gui->getViewMatrix(), "V");
                    VectorGridViewerProgram->setMat4(gui->getProjectionMatrix(), "P");
                    main_viewer_ssbo->bindBase(0);
                    VectorGridViewerProgram->setUint(data_NIR->cols(), "inputImageWidth");
                    VectorGridViewerProgram->setUint(data_NIR->rows(), "inputImageHeight");
                    VectorGridViewerProgram->setUint(data_Size, "inputImageSize");
                    VectorGridViewerProgram->setVec4v(
                        {{1.f, 0.f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f}}, "cols"
                    );
                    VectorGridViewerProgram->setIvec2({deltaVectorGrid, deltaVectorGrid}, "divs");
                    VectorGridViewerProgram->setVec2({100.f, 300.f}, "vec_lengths");
                    VectorGridViewerProgram->setVec3(ave_sun_dir, "ave_dir");
                    VectorGridViewerProgram->setFloat(0.01, "forward_dir_weight");
                    VectorGridViewerProgram->setInt(0, "remove_dc_dir");
                    vector_geom->bind();
                    glDrawArrays(GL_POINTS, 0, vector_grid.size());

                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    BasicShaderProgram->use();
                    BasicShaderProgram->setMat4(vector_model, "M");
                    BasicShaderProgram->setMat4(gui->getViewMatrix(), "V");
                    BasicShaderProgram->setMat4(gui->getProjectionMatrix(), "P");
                    BasicShaderProgram->setVec4({1.f, 0.f, 0.f, 0.f}, "col");
                    sun_view_geom->bind();
                    glDrawArrays(GL_POINTS, 1, 1);
                }

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                BasicShaderProgram->use();
                BasicShaderProgram->setMat4(
                    glm::scale(glm::mat4(1.f), glm::vec3(1., -1., 1.)) * image_model, "M"
                );
                BasicShaderProgram->setMat4(gui->getViewMatrix(), "V");
                BasicShaderProgram->setMat4(gui->getProjectionMatrix(), "P");
                line_geom->setVerts(output_EvaluationBounds.lineStrip());
                BasicShaderProgram->setVec4({0.7f, 0.4f, 0.95f, 1.f}, "col");
                line_geom->bind();
                glDrawArrays(GL_LINE_STRIP, 0, 5);

                if (gui->isQuadsVisable()) {
                    BasicShaderProgram->setMat4(
                        glm::scale(glm::mat4(1.f), glm::vec3(1., -1., 1.)) * vector_model, "M"
                    );
                    for (auto &c : Clouds) {
                        if (c.second.pixels.list.size() < quad_cull_bounds.x
                            || c.second.pixels.list.size() > quad_cull_bounds.y)
                            continue;
                        Quad render_quad = c.second.quad;
                        bool mouse_in    = Functions::inXY(render_quad, mouse_pos_vector_space);

                        line_geom->setVerts(render_quad.lineStrip());
                        BasicShaderProgram->setVec4({1.f, mouse_in ? 0.f : .9f, 0.28f, 1.f}, "col");
                        line_geom->bind();
                        glDrawArrays(GL_LINE_STRIP, 0, 5);

                        if (OptimalCloudCastingSolutions[c.first].similarity > 0.f) {
                            // Casted Cloud
                            render_quad.apply(OptimalCloudCastingSolutions[c.first].M);
                            line_geom->setVerts(render_quad.lineStrip());
                            BasicShaderProgram->setVec4(
                                {.3f, mouse_in ? 0.f : 1.f, .7f, 1.f}, "col"
                            );
                            line_geom->bind();
                            glDrawArrays(GL_LINE_STRIP, 0, 5);

                            //// Shadow Quad
                            // render_quad = CastingShadowMask.shadows[c.first].quad;
                            // line_geom.setVerts(render_quad.lineStrip());
                            // BasicProgram->setVec4({ 0.f, mouse_in ? 1.f : 0.f, 1.f, 1.f },
                            // "col"); line_geom.bind(); glDrawArrays(GL_LINE_STRIP, 0, 5);
                        }
                    }
                }
            } else {
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                SurfaceViewerProgram->use();
                SurfaceViewerProgram->setMat4(
                    glm::rotate(glm::mat4(1.f), -float(M_PI_2), {1.f, 0.f, 0.f}), "M"
                );
                SurfaceViewerProgram->setMat4(gui->getViewMatrix(), "V");
                SurfaceViewerProgram->setMat4(gui->getProjectionMatrix(), "P");
                SurfaceViewerProgram->setVec4v(
                    {{1.f, 0.f, 0.f, 1.f}, {1.f, 1.f, 0.f, 1.f}, {1.f, 0.f, 1.f, 1.f}}, "cols"
                );
                SurfaceViewerProgram->setIvec2(gui->getProbabilitySurfaceRoot(), "root");
                SurfaceViewerProgram->setIvec2(gui->getProbabilitySurfaceDim(), "render_dim");
                SurfaceViewerProgram->setIvec2(ProbabilityFunction.resolution(), "total_dim");
                SurfaceViewerProgram->setVec3({0.f, 5.f, 0.f}, "light_pos");
                SurfaceViewerProgram->setVec3(gui->getViewPosition(), "camera_pos");
                surface_render_geom = ProbabilityFunction.MeshData(
                    gui->getProbabilitySurfaceRoot().x,
                    gui->getProbabilitySurfaceRoot().x + gui->getProbabilitySurfaceDim().x - 1,
                    gui->getProbabilitySurfaceRoot().y,
                    gui->getProbabilitySurfaceRoot().y + gui->getProbabilitySurfaceDim().y - 1
                );
                surface_ssbo->uploadData(
                    surface_render_geom.verts.size() * sizeof(glm::vec3),
                    surface_render_geom.verts.data(),
                    GL_STATIC_DRAW
                );
                surface_geom->setCoords(surface_render_geom.tris);
                surface_geom->bind();
                surface_ssbo->bindBase(0);
                glDrawArrays(GL_POINTS, 0, surface_render_geom.tris.size());

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
                BasicShaderProgram->use();
                BasicShaderProgram->setMat4(
                    glm::translate(glm::mat4(1.f), {0.f, .15f, 0.f})
                        * glm::rotate(glm::mat4(1.f), -float(M_PI_2), {1.f, 0.f, 0.f}),
                    "M"
                );
                BasicShaderProgram->setMat4(gui->getViewMatrix(), "V");
                BasicShaderProgram->setMat4(gui->getProjectionMatrix(), "P");
                BasicShaderProgram->setVec4({1.f, 1.f, 1.f, 0.3f}, "col");
                quad->draw();
            }
            //----------------------------------------------------------------------------------
            glDisable(GL_FRAMEBUFFER_SRGB);  // disable sRGB for things like imgui
            gui->draw();
            window->swapBuffers();
        }
        Log::debug("Terminating GUI...");
        // Force destructors
        surface_geom.reset();
        surface_ssbo.reset();
        vector_geom.reset();
        sun_view_geom.reset();
        line_geom.reset();
        main_viewer_ssbo.reset();
        quad.reset();
        CurrentImageViewerProgram.reset();
        VectorGridViewerProgram.reset();
        SurfaceViewerProgram.reset();
        RGBAImageViewerProgram.reset();
        FloatImageViewerProgram.reset();
        BasicShaderProgram.reset();
        gui.reset();
        window.reset();
        glfwTerminate();
    }
    return EXIT_SUCCESS;
}