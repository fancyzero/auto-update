//
//  download_manager.h
//  auto updater
//
//  Created by FancyZero on 13-4-17.
//
//

#ifndef __auto_updater__download_manager__
#define __auto_updater__download_manager__

#include <iostream>
#include <vector>
#include <string>
#include <cocos2d.h>
#define DOWNLOAD_LOG(format, ...)  cocos2d::CCLog(format, ##__VA_ARGS__)
#include "curl.h"
#include "md5.h"

bool create_path( const char* path );
class download_manager;
class download_job
{
public:
    enum job_state
    {
        pending,
        working,
        completed,
    };
    enum job_result
    {
        unknown,
        aborted,
        succeeded,
        failed,
    };
    struct job_stat
    {
        int total;
        int current;
        job_state state;
        job_result result;
    };
    struct job_desc
    {
        job_desc()
        :compressed(false)
        {
            
        }
        std::string hash;
        std::string src_url;
        std::string dest_file;
        bool        compressed;
    };
    download_job( const job_desc& desc, download_manager* manager )
    :m_desc(desc),m_manager(manager),m_pf(NULL),m_pf_hash(NULL),m_flush_step_size(1024*100),m_total_written(0)
    ,m_current_unflushed(0),m_url_handle(NULL)
    {
        m_stat.state = pending;
        m_stat.result = unknown;
    }
    job_stat    get_stat() const;
    job_desc    get_desc() const;
    bool        execute();
    bool        download();
    void        cleanup();
    bool        abort();
    CURL*       get_url_handle();
    void        set_succeeded();//set as completed and succeeded;
    void        set_failed();//set as completed and failed;
    
    bool        is_succeeded();
    bool        is_completed();
    void        on_download_completed();  
    int         get_flush_step_size();
    int         get_total_written();
    int         get_current_unflushed();
    void        add_current_unflushed( int size );
    void        add_total_written( int size );
    FILE*       get_file_handle();

protected:
    int         prepare_download();
  
    ////////////
    int         m_flush_step_size;
    int         m_total_written;
    int         m_current_unflushed;
    md_context  m_hash_recorder;
    ////////////
    CURL*       m_url_handle;
    download_manager *m_manager;
    pthread_t   m_thread_id;
    job_desc    m_desc;
    job_stat    m_stat;
    FILE*       m_pf;
    FILE*       m_pf_hash;
};


typedef std::vector<download_job*> DOWNLOAD_JOBS;

class download_manager
{
public:
    struct download_status
    {
        download_status()
        :total_files(0),completed_files(0),succeeded_files(0),failed_files(0)
        {
        }
        std::string current_file;
        int total_files;
        int completed_files;
        int succeeded_files;
        int failed_files;
    };
    
    
    download_manager();
    ~download_manager();
    
    void update();
        download_job* add_job( const download_job::job_desc& job_desc );
    void abort_job( download_job* job );
    void abort_all();
    CURLM* get_multi_handle();

    int get_succeeded_job_count();
    int get_job_count();
    void clean_succeeded_job();
    void clean_all_job();
    download_status get_status();
protected:

    void execute_job( download_job* job );
    download_job* get_job_by_handle( CURL* handle );
    
protected:
    CURLM*        m_multi_handle;
    int           m_max_simultaneously_jobs;
    DOWNLOAD_JOBS m_download_jobs;
    
};
#endif /* defined(__auto_updater__download_manager__) */
