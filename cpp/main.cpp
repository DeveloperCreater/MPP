#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

static void printHelp() {
    std::cout << "M++ — The Ultimate Computer Language (native C++)\n";
    std::cout << "Usage:\n";
    std::cout << "  mpp <file.mpp>\n";
    std::cout << "  mpp -r <file.mpp>\n";
    std::cout << "  mpp -e \"print(42)\"\n";
    std::cout << "\n";
    std::cout << "App builder:\n";
    std::cout << "  mpp build <entry.mpp> -o <outDir>\n";
    std::cout << "    Creates a runnable app folder containing mpp.exe + your script.\n";
    std::cout << "  mpp build-app <appDir> -o <outDir>\n";
    std::cout << "    Packages an M++ app folder (app.json + ui/ + app.mpp).\n";
    std::cout << "\n";
    std::cout << "Low-level builtins (native): mem_alloc, mem_read8, mem_write8\n";
}

static int cmdBuild(const std::string& selfPath, int argc, char* argv[]) {
    // mpp build <entry.mpp> -o <outDir>
    if (argc < 4) {
        std::cerr << "Error: build requires <entry.mpp> and -o <outDir>\n";
        return 1;
    }

    std::string entry = argv[2];
    std::string outDir;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        if ((a == "-o" || a == "--out") && i + 1 < argc) {
            outDir = argv[i + 1];
            i++;
        }
    }
    if (outDir.empty()) {
        std::cerr << "Error: build requires -o <outDir>\n";
        return 1;
    }

    namespace fs = std::filesystem;
    try {
        fs::path out = fs::path(outDir);
        fs::create_directories(out);

        fs::path runtimeDst = out / "mpp.exe";
        fs::copy_file(fs::path(selfPath), runtimeDst, fs::copy_options::overwrite_existing);

        fs::path scriptDst = out / "app.mpp";
        fs::copy_file(fs::path(entry), scriptDst, fs::copy_options::overwrite_existing);

        // Create Windows launcher
        fs::path bat = out / "run.bat";
        std::ofstream b(bat.string(), std::ios::binary);
        b << "@echo off\r\n";
        b << "cd /d \"%~dp0\"\r\n";
        b << "mpp.exe -r app.mpp\r\n";
        b.close();

        std::cout << "Built app folder: " << out.string() << "\n";
        std::cout << "Run it with: " << (out / "run.bat").string() << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Build Error: " << e.what() << "\n";
        return 1;
    }
}

static int cmdBuildApp(const std::string& selfPath, int argc, char* argv[]) {
    // mpp build-app <appDir> -o <outDir>
    if (argc < 4) {
        std::cerr << "Error: build-app requires <appDir> and -o <outDir>\n";
        return 1;
    }
    std::string appDir = argv[2];
    std::string outDir;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        if ((a == "-o" || a == "--out") && i + 1 < argc) {
            outDir = argv[i + 1];
            i++;
        }
    }
    if (outDir.empty()) {
        std::cerr << "Error: build-app requires -o <outDir>\n";
        return 1;
    }

    namespace fs = std::filesystem;
    try {
        fs::path out = fs::path(outDir);
        fs::create_directories(out);

        // Runtime
        fs::copy_file(fs::path(selfPath), out / "mpp.exe", fs::copy_options::overwrite_existing);

        // App files
        fs::path src = fs::path(appDir);
        fs::copy_file(src / "app.json", out / "app.json", fs::copy_options::overwrite_existing);
        fs::copy_file(src / "app.mpp", out / "app.mpp", fs::copy_options::overwrite_existing);
        fs::create_directories(out / "ui");
        fs::copy(src / "ui", out / "ui", fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        // Launcher (placeholder): real window hosting is provided by the native host app (WebView2)
        std::ofstream b((out / "run_cli.bat").string(), std::ios::binary);
        b << "@echo off\r\n";
        b << "cd /d \"%~dp0\"\r\n";
        b << "echo This app needs the native WebView2 host to run as a window.\r\n";
        b << "echo (C++ host project: apps\\\\terminal_ui_cpp_host)\r\n";
        b.close();

        std::cout << "Built app package: " << out.string() << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Build Error: " << e.what() << "\n";
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string selfPath = argv[0];
    std::string cmd = argv[1];
    if (cmd == "--help" || cmd == "-h") {
        printHelp();
        return 0;
    }
    if (cmd == "build") {
        return cmdBuild(selfPath, argc, argv);
    }
    if (cmd == "build-app") {
        return cmdBuildApp(selfPath, argc, argv);
    }

    std::string source;
    if (cmd == "-e") {
        if (argc < 3) {
            std::cerr << "Error: -e requires code string\n";
            return 1;
        }
        source = argv[2];
    } else {
        std::string path = (cmd == "-r" && argc >= 3) ? argv[2] : argv[1];
        std::ifstream f(path);
        if (!f) {
            std::cerr << "Error: File not found: " << path << "\n";
            return 1;
        }
        std::stringstream buf;
        buf << f.rdbuf();
        source = buf.str();
    }

    try {
        mpp::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        mpp::Parser parser(std::move(tokens));
        auto program = parser.parse();
        mpp::Interpreter interpreter;
        interpreter.run(program.get());
    } catch (const mpp::ParseError& e) {
        std::cerr << "Parse Error: " << e.what() << "\n";
        return 1;
    } catch (const mpp::RuntimeError& e) {
        std::cerr << "Runtime Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
