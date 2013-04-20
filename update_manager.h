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

class download_manager;

struct file_list_item
{
    std::string hash;
    std::string local_path;
    std::string src_url;
    std::string collection;
};

typedef std::vector<file_list_item> FILE_LIST;
struct file_list
{
public:
    const file_list_item* get_file_list_item( const std::string& local_path );
    bool load_from_file( const std::string& filename );
    void add_item( const file_list_item& item );

    FILE_LIST m_list;
};

class update_manager
{
public:
    update_manager();
    ~update_manager();
    /*
     获取某个 文件集合（collection） 的更新列表
     //如果 collection 为空，则表示包含所有文件
     */
    bool download_files( const file_list& fl );
    file_list get_update_list( const std::string& collection );
    std::string get_full_path( const std::string& local_path );
    bool       load_file_list( const std::string& filename );
    void set_download_manager( download_manager* man )
    {
        m_download_manager = man;
    }
    void set_root_path( const std::string& path )
    {
        m_root_path = path;
    }
    bool update_file_collection( const std::string& collection );
    bool is_update_finished();
protected:
    download_manager* m_download_manager;
    std::string m_root_path;
    file_list   m_file_list;
};
#endif /* defined(__auto_updater__updater__) */
