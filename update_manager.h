//
//  updater.h
//  auto updater
//
//  Created by FancyZero on 13-4-18.
//
//

#ifndef __auto_updater__updater__
#define __auto_updater__updater__

#include <iostream>
#include <string>
#include <vector>
struct file_list_item
{
    std::string hash;
    std::string local_path;
    std::string src_url;
};

typedef std::vector<file_list_item> FILE_LIST;
class file_list
{
public:
    const file_list_item* get_file_list_item( const std::string& local_path );
    bool load_from_file( const std::string& filename );
protected:
    FILE_LIST m_list;
};

class update_manager
{
public:
    update_manager();
    ~update_manager();
protected:
    file_list m_file_list;
};
#endif /* defined(__auto_updater__updater__) */
