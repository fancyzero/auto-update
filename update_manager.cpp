//
//  updater.cpp
//  auto updater
//
//  Created by FancyZero on 13-4-18.
//
//

#include "update_manager.h"
#include "pugixml.hpp"
#include <sys/stat.h>
#include "md5.h"
#include "download_manager.h"

std::string get_file_md5( const std::string& filename );

const file_list_item* file_list::get_file_list_item( const std::string& local_path )
{
    for ( FILE_LIST::iterator it = m_list.begin(); it != m_list.end(); ++it )
    {
        if ( (*it).local_path == local_path )
            return &(*it);
    }
    return NULL;
}



bool file_list::load_from_file( const std::string& filename )
{
    m_list.clear();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file( filename.c_str() );
    if ( !result )
    {
        return false;
    }
    
    pugi::xml_node root = doc.child("filelist");
    if ( root == NULL)
        return false;
    pugi::xml_node info_node = root.child("info");
    pugi::xml_node filelist_node = root.child("files");
    if ( filelist_node == NULL || info_node == NULL )
    {
        return false;
    }
    for (pugi::xml_node file = filelist_node.child("file"); file; file = file.next_sibling("file"))
    {
        file_list_item new_item;
        new_item.hash = file.attribute("hash").as_string();
        new_item.local_path = file.attribute("local_path").as_string();
        new_item.src_url = file.attribute("url").as_string();
        new_item.collection = file.attribute("collection").as_string();
        m_list.push_back( new_item );
    }
    return true;
}

void file_list::add_item( const file_list_item& item )
{
    m_list.push_back( item );
}

/*
 update_manager
 */

update_manager::update_manager()
{
    
}

update_manager::~update_manager()
{
    
}

std::string get_file_md5( const std::string& filename )
{
    std::string md5_str = "ffffffffaaaaaaaa";
    unsigned char digest[MD5_DIGEST_LEN];
    FILE* pf = fopen( filename.c_str(), "rb" );
    unsigned char buff[1024];
    md_context ctx;
    md5_begin( &ctx );
    while( !feof(pf) )
    {
        size_t readed = fread( buff, 1, 1024, pf);
        md5_update( &ctx, buff, readed);
        if ( readed < 1024 )
            break;
    }
    fclose(pf);
    char cstr_digest[MD5_DIGEST_LEN*2+1];
    memset(cstr_digest, 0, MD5_DIGEST_LEN*2+1);
    md5_result( &ctx, digest );
    for ( int i = 0; i < MD5_DIGEST_LEN; i++ )
    {
        sprintf(cstr_digest+i*2, "%02x", digest[i]);
    }
    md5_str = cstr_digest;
    return md5_str;
}

bool is_identical( const std::string& filename, const std::string& hash )
{
    // get local file modified time
/*struct stat _stat;
    struct tm* clock;
    stat( file.c_str(), &_stat);
    clock = gmtime(&(_stat.st_mtime));
*/
    if( access( filename.c_str(), F_OK ) != -1 )
    {
        return get_file_md5( filename ) == hash;
    }
    else
        return false;
    
}

bool update_manager::load_file_list( const std::string& filename )
{
    return m_file_list.load_from_file( filename );
}

std::string update_manager::get_full_path( const std::string& local_path )
{
    return m_root_path + local_path;
}

bool update_manager::download_files( const file_list& fl )
{
    for( FILE_LIST::const_iterator it = fl.m_list.begin(); it != fl.m_list.end(); ++it )
    {
        download_job::job_desc desc;
        desc.src_url = (*it).src_url;
        desc.dest_file = get_full_path( (*it).local_path );
        m_download_manager->add_job( desc );
    }
    return true;
}

file_list update_manager::get_update_list( const std::string& collection )
{
    file_list list;
    
    for ( FILE_LIST::const_iterator it = m_file_list.m_list.begin(); it != m_file_list.m_list.end(); ++it )
    {
        if ( collection.empty() || collection == (*it).collection )
        {
            if ( !is_identical( get_full_path((*it).local_path), (*it).hash) )
            {
                DOWNLOAD_LOG( "file: %s has newer version, newer hash: %s", get_full_path((*it).local_path).c_str(), (*it).hash.c_str() );
                list.add_item( *it );
            }
            else
            {
                DOWNLOAD_LOG( "file: %s is newest version, hash: %s", get_full_path((*it).local_path).c_str(), (*it).hash.c_str() );
            }
        }
    }
    return list;
    
}
