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
        std::string src_url;
        std::string dest_file;
    };
    download_job( const job_desc& desc, download_manager* manager )
    :m_desc(desc),m_manager(manager),m_pf(NULL)
    {
        m_stat.state = pending;
        m_stat.result = unknown;
    }
    job_stat    get_job_stat();
    bool        execute();
    bool        download();
    void        cleanup();
    bool        abort();
    CURL*       get_url_handle();
    void        set_succeeded();//set as completed and succeeded;
    void        set_failed();//set as completed and failed;
protected:
    CURL*       m_url_handle;
    download_manager *m_manager;
    pthread_t   m_thread_id;
    job_desc    m_desc;
    job_stat    m_stat;
    FILE*       m_pf;
};


typedef std::vector<download_job*> DOWNLOAD_JOBS;
class download_manager
{
public:
    download_manager();
    ~download_manager();
    void update();
    void execute_job( download_job* job );
    download_job* add_job( const download_job::job_desc& job_desc );
    void abort_job( download_job* job );
    void abort_all();
    CURLM* get_multi_handle();
    download_job* get_job_by_handle( CURL* handle );
protected:
    CURLM*        m_multi_handle;
    int           m_max_simultaneously_jobs;
    DOWNLOAD_JOBS m_download_jobs;
    
};
#endif /* defined(__auto_updater__download_manager__) */
