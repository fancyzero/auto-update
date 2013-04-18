//
//  updater.cpp
//  auto updater
//
//  Created by FancyZero on 13-4-18.
//
//

#include "update_manager.h"
#include "pugixml.hpp"

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
    pugi::xml_parse_result result = doc.load( filename.c_str() );
    if ( !result )
    {
        return false;
    }
    

    pugi::xml_node info_node = doc.child("filelist").child("info");
    pugi::xml_node filelist_node = doc.child("filelist").child("files");
    if ( filelist_node == NULL || info_node == NULL )
    {
        return false;
    }
    for (pugi::xml_node file = filelist_node.child("file"); file; file = file.next_sibling("file"))
    {
        std::cout << file.attribute("url").value();
    }
    return true;
}

update_manager::update_manager()
{
    
}

update_manager::~update_manager()
{
    
}
