// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// 示例程序：演示子选项功能（三级层级）
// 编译: g++ -std=c++11 example.cpp -I../../include -o example
// 运行示例:
//   ./example -add -file path/to/file.txt -recursive -del -force
//   ./example -add -file path/to/file.txt -encoding utf8 -overwrite -del -force

#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    CLI::App app{"SubOption Example Program"};

    bool add_flag = false;
    bool del_flag = false;
    bool force_flag = false;
    std::string file_path;        // Level 2: 需要值的选项
    bool recursive_flag = false;  // Level 2: 标志选项
    std::string encoding;         // Level 3: 需要值的选项
    bool overwrite_flag = false;  // Level 3: 标志选项

    // 允许非标准选项名（单破折号后面跟多个字符）
    app.allow_non_standard_option_names();

    // 平级选项：-add, -del, -force
    app.add_flag("-add", add_flag, "Add option");
    app.add_flag("-del", del_flag, "Delete option");
    app.add_flag("-force", force_flag, "Force option");

    // -add 的子选项：-file (需要值) 和 -recursive (标志)
    // 使用选项组来实现子选项功能
    CLI::Option_group *add_group = app.add_option_group("add_suboptions", "Sub-options for -add");
    add_group->allow_non_standard_option_names();  // 选项组也需要启用非标准选项名
    add_group->add_option("-file", file_path, "File path (requires a value)");
    add_group->add_flag("-recursive", recursive_flag, "Process recursively (flag, no value needed)");

    // 子选项需要 -add 选项存在
    CLI::Option *add_option = app.get_option("-add");
    add_group->needs(add_option);

    // 第三级层级：-file 的子选项
    // 在 -file 选项组下再创建一个选项组
    CLI::Option_group *file_group = add_group->add_option_group("file_suboptions", "Sub-options for -file");
    file_group->allow_non_standard_option_names();
    file_group->add_option("-encoding", encoding, "File encoding (requires a value, e.g., utf8, gbk)");
    file_group->add_flag("-overwrite", overwrite_flag, "Overwrite existing file (flag, no value needed)");
    
    // 子子选项需要 -file 选项存在
    auto *file_option = add_group->get_option("-file");
    file_group->needs(file_option);

    // 解析命令行参数
    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }

    // 输出结果
    std::cout << "解析结果:\n";
    std::cout << "  -add: " << (add_flag ? "true" : "false") << "\n";
    std::cout << "  -del: " << (del_flag ? "true" : "false") << "\n";
    std::cout << "  -force: " << (force_flag ? "true" : "false") << "\n";
    
    if(add_flag) {
        std::cout << "  -file: " << (file_path.empty() ? "(未设置)" : file_path) << "\n";
        std::cout << "  -recursive: " << (recursive_flag ? "true" : "false") << "\n";
        
        if(!file_path.empty()) {
            std::cout << "    -encoding: " << (encoding.empty() ? "(未设置)" : encoding) << "\n";
            std::cout << "    -overwrite: " << (overwrite_flag ? "true" : "false") << "\n";
        }
    }

    return 0;
}

