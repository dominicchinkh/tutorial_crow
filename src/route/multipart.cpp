#include <string>

#include "route.h"

using namespace std;

void register_multipart_routes(CrowApp& app) 
{
    //-----------------------------------------------------------
    // Define a POST endpoint for handling file and form uploads

    // Testing with:
    //   echo "Date,Close\n2026-06-01,150.5\n2026-06-02,152.3" > data.csv
    //   curl -k -X POST https://localhost:18080/api/upload -F "asset=AUD_USD" -F "timeframe=1d" -F "csv_file=@data.csv"

    CROW_ROUTE(app, "/api/upload").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        
        // 1. Parse the incoming request into a multipart message
        crow::multipart::message mp_msg(req);

        // 2. Extract plain text form fields using `get_part_by_name()`
        //    Let's assume the form passes an "asset" and a "timeframe" text field
        auto asset_part     = mp_msg.get_part_by_name("asset");
        auto timeframe_part = mp_msg.get_part_by_name("timeframe");

        if (asset_part.body.empty() || timeframe_part.body.empty()) {
            return crow::response(400, "Missing required form fields 'asset' or 'timeframe'.");
        }

        std::string asset     = asset_part.body;
        std::string timeframe = timeframe_part.body;
        
        std::cout << "Received metadata for asset: " << asset << " (" << timeframe << ")\n";

        // 3. Extract the uploaded file part (let's assume the input field name is "csv_file")
        auto file_part = mp_msg.get_part_by_name("csv_file");
        if (file_part.body.empty()) {
            return crow::response(400, "Missing uploaded file 'csv_file'.");
        }

        // 4. Retrieve metadata from the file's part headers (like the actual filename)
        std::string filename = "uploaded_data.csv"; // Fallback name
        
        auto headers_it = file_part.headers.find("Content-Disposition");

        if (headers_it != file_part.headers.end()) {

            // Find the "filename" attribute within the Content-Disposition header parameter map
            auto params_it = headers_it->second.params.find("filename");

            if (params_it != headers_it->second.params.end()) {
                filename = params_it->second; // Captures original name like "historical_prices.csv"
            }
        }

        // 5. Save the file data (the raw binary payload is stored completely in file_part.body)
        std::ofstream out_file(filename, std::ios::binary);

        if (!out_file) {
            return crow::response(500, "Internal server error: Failed to write file to disk.");
        }
        
        out_file << file_part.body;
        out_file.close();

        // 6. Build response payload
        crow::json::wvalue response_json;
        response_json["status"] = "success";
        response_json["saved_as"] = filename;
        response_json["size_bytes"] = file_part.body.size();
        response_json["associated_asset"] = asset;

        return crow::response(200, response_json);
    });
}
