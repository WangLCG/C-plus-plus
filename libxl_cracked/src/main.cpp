#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include "include_cpp/libxl.h"

std::string read_cell_content(libxl::Sheet * sheet, int32_t row, int32_t col)
{
    std::string ret;
    libxl::CellType cell_type = sheet->cellType(row, col);
    switch (cell_type)
    {
    case libxl::CELLTYPE_STRING:
    {
        ret = sheet->readStr(row, col);
    }
    break;
    case libxl::CELLTYPE_NUMBER:
    {
        const int64_t base = 1000000;
        const double num = sheet->readNum(row, col);
        // 如果小数位全为0，当作整数处理，精度(.000001)
        if (int64_t(num * base) / base * base == int64_t(num * base))
        {
            ret = std::to_string(int64_t(num));
        }
        else
        {
            ret = std::to_string(num);
        }
    }
    break;
    case libxl::CELLTYPE_BOOLEAN:
    case libxl::CELLTYPE_ERROR:
    {
        ret = "Unknow Field";
    }
    break;
    case libxl::CELLTYPE_BLANK:
    case libxl::CELLTYPE_EMPTY:
        break;
    }
    return ret;
}

void read_excel_2003()
{
    const std::string excel_2003_file_path = "../excel_files/excel2003.xls";
    libxl::Book* book = xlCreateBook();		// 与2007相比，仅仅这里有点区别而已
    if (nullptr == book)
    {
        std::cerr << "xlCreateBook failed." << std::endl;
        return;
    }
    // linux库需要破解,注册码由注册机生成
#if !defined(WIN32) && !defined(WIN64)
    book->setKey("mylibxl", "linux-2f2129060dcce70c67ba606aa8idk3g8");
#endif
    if (false == book->load(excel_2003_file_path.c_str()))
    {
        std::cerr << "load " << excel_2003_file_path << " failed!" << std::endl;
        return;
    }
    for (int32_t i = 0; i < book->sheetCount(); ++i)
    {
        libxl::Sheet * sheet = book->getSheet(i);
        std::cout << sheet->name() << std::endl;
        for (int32_t row = sheet->firstRow(); row < sheet->lastRow(); ++row)
        {
            std::cout << row << ": ";
            for (int32_t col = sheet->firstCol(); col < sheet->lastCol(); ++col)
            {
                std::cout << col << "-<" << read_cell_content(sheet, row, col) <<"> ";
            }
            std::cout << std::endl;
        }
    }
}

void read_excel_2007()
{
    const std::string excel_2007_file_path = "../excel_files/excel2007.xlsx";
    libxl::Book* book = xlCreateXMLBook();	// 与2003相比，仅仅这里有点区别而已
    if (nullptr == book)
    {
        std::cerr << "xlCreateBook failed." << std::endl;
        return;
    }
    // linux库需要破解,注册码由注册机生成
#if !defined(WIN32) && !defined(WIN64)
    book->setKey("mylibxl", "linux-2f2129060dcce70c67ba606aa8idk3g8");
#endif
    if (false == book->load(excel_2007_file_path.c_str()))
    {
        std::cerr << "load " << excel_2007_file_path << " failed!" << std::endl;
        return;
    }
    for (int32_t i = 0; i < book->sheetCount(); ++i)
    {
        libxl::Sheet * sheet = book->getSheet(i);
        std::cout << sheet->name() << std::endl;
        for (int32_t row = sheet->firstRow(); row < sheet->lastRow(); ++row)
        {
            std::cout << row << ": ";
            for (int32_t col = sheet->firstCol(); col < sheet->lastCol(); ++col)
            {
                std::cout << col << "-<" << read_cell_content(sheet, row, col) << "> ";
            }
            std::cout << std::endl;
        }
    }
}

int32_t main()
{
    read_excel_2003();
    read_excel_2007();
    return 0;
}