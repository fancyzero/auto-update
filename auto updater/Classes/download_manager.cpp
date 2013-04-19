//
//  download_manager.cpp
//  auto updater
//
//  Created by FancyZero on 13-4-17.
//
//

#include "download_manager.h"
#include "curl.h"



/*
 download_job
 */
static size_t download_job_write_function(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    static size_t totoal_written = 0;
    FILE *fp = (FILE*)userdata;
    size_t written = fwrite(ptr, size, nmemb, fp);
    totoal_written += written;
    fflush(fp);
    DOWNLOAD_LOG( "write: %d written: %d total: %d", size*nmemb, written, totoal_written );
    return written;
}

static int download_job_progress(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    DOWNLOAD_LOG( "downloading... %f, %f", nowDownloaded, totalToDownload );
    return 0;
}

static void* download_execute_thread_func(void* job)
{
    DOWNLOAD_LOG( "download thread for %x begin", job );
    download_job* djob = (download_job*)job;
    djob->download();
    DOWNLOAD_LOG( "download thread for %x end", job );
    return NULL;
}

download_job::job_stat download_job::get_job_stat()
{
    return m_stat;
}

CURL* download_job::get_url_handle()
{
    return m_url_handle;
}

bool download_job::execute()
{
    m_stat.state = working;
    download();
    //int ret = pthread_create( &m_thread_id, NULL, download_execute_thread_func, this );
    //pthread_join( m_thread_id, NULL );
    return true;
}

void download_job::cleanup()
{
    if ( m_pf != NULL )
        fclose(m_pf);
    if ( m_url_handle != NULL )
        curl_easy_cleanup(m_url_handle);
    m_pf = NULL;
    m_url_handle = NULL;
}

void download_job::set_succeeded()
{
    m_stat.result = succeeded;
    m_stat.state = completed;
    cleanup();
}

void download_job::set_failed()
{
    m_stat.result = failed;
    m_stat.state = completed;
    cleanup();
}

bool download_job::download()
{
    
    if ( m_pf != NULL )
        return true;
    
    std::string outFileName = m_desc.dest_file;
    std::string _packageUrl = m_desc.src_url;
    m_pf = fopen(outFileName.c_str(), "wb");
    if ( m_pf == NULL )
    {
        DOWNLOAD_LOG("can not create file %s", outFileName.c_str());
        set_failed();
        return false;
    }
    DOWNLOAD_LOG("create file %s for write", outFileName.c_str());
    
    // Download pacakge
    //CURLcode res = 0;
    CURL* curl = curl_easy_init();
    
    curl_easy_setopt( curl, CURLOPT_URL, _packageUrl.c_str() );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, download_job_write_function );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, m_pf );
    curl_easy_setopt( curl, CURLOPT_NOPROGRESS, false );
    curl_easy_setopt( curl, CURLOPT_PROGRESSFUNCTION, download_job_progress );
    //curl_easy_setopt( curl, CURLOPT_TIMEOUT, 15 );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 );
    curl_multi_add_handle(m_manager->get_multi_handle(), curl );
    
    /*if (res != 0)
     {
     DOWNLOAD_LOG("error when download package");
     fclose(fp);
     m_stat.result = failed;
     return false;
     }
     */
    m_url_handle = curl;
    DOWNLOAD_LOG("start downloading file %s", _packageUrl.c_str());
    
    //fclose(fp);
    //m_stat.state = completed;
    return true;
}

bool download_job::abort()
{
    m_stat.state = completed;
    m_stat.result = failed;
    return true;
}

/*
 download_manager
 */
download_manager::download_manager()
:m_max_simultaneously_jobs(1)
{
    m_multi_handle = curl_multi_init();
}

download_manager::~download_manager()
{
    curl_multi_cleanup(m_multi_handle);
    curl_global_cleanup();
}

CURLM* download_manager::get_multi_handle()
{
    return m_multi_handle;
}

download_job* download_manager::get_job_by_handle( CURL* handle )
{
    DOWNLOAD_JOBS::const_iterator it;
    for ( it  = m_download_jobs.begin(); it != m_download_jobs.end(); ++it )
    {
        if ( (*it)->get_url_handle() == handle )
            return (*it);
    }
    return *it;
}

void download_manager::execute_job( download_job* job )
{
    job->execute();
}

void download_manager::update()
{
    DOWNLOAD_JOBS::const_iterator it;
    int working_job = 0;
    for ( it  = m_download_jobs.begin(); it != m_download_jobs.end(); ++it )
    {
        if ( (*it) == NULL )
            continue;
        download_job::job_stat js = (*it)->get_job_stat();
        
        if ( js.state == download_job::working )
        {
            working_job ++;
        }
        else if ( js.state == download_job::pending )
        {
            execute_job((*it));
            working_job ++;
        }
        if ( working_job >= m_max_simultaneously_jobs )
            break;
    }
    
    
    int running_handle_count = 0;
    CURLMcode urlret = curl_multi_perform(m_multi_handle, &running_handle_count);
    if ( CURLM_CALL_MULTI_PERFORM != urlret )
    {
        //DOWNLOAD_LOG( "runing handle : %d", running_handle_count );
        if ( running_handle_count > 0)
        {
            timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            
            int max_fd;
            fd_set fd_read;
            fd_set fd_write;
            fd_set fd_except;
            
            FD_ZERO(&fd_read);
            FD_ZERO(&fd_write);
            FD_ZERO(&fd_except);
            
            curl_multi_fdset( m_multi_handle, &fd_read, &fd_write, &fd_except, &max_fd );
            int return_code = select( max_fd + 1, &fd_read, &fd_write, &fd_except, &tv );
            if (-1 == return_code)
            {
                DOWNLOAD_LOG( "select error." );
            }
        }
    }
    
    int msgs_in_queue = 0;
    CURLMsg *msg  = NULL;
    do
    {
        msg = curl_multi_info_read( m_multi_handle,   &msgs_in_queue);
        if ( msg != NULL && msg->easy_handle != NULL )
        {
            if ( msg->msg == CURLMSG_DONE )
            {
                DOWNLOAD_LOG("handle %x done", msg->easy_handle );
                curl_multi_remove_handle( m_multi_handle, msg->easy_handle );
                if ( msg->data.result == CURLE_OK )
                    get_job_by_handle( msg->easy_handle )->set_succeeded();
                else
                    get_job_by_handle( msg->easy_handle )->set_failed();
            }
        }
    }while(msg != NULL);
    
    //sleep(1);
    
}
download_job* download_manager::add_job( const download_job::job_desc& job_desc )
{
    download_job* new_job = new download_job( job_desc, this );
    m_download_jobs.push_back(new_job);
    return new_job;
}


void download_manager::abort_job( download_job* job )
{
    DOWNLOAD_LOG( "abort job, handle : %d", job->get_url_handle() );
    curl_multi_remove_handle( m_multi_handle, job->get_url_handle() );
    curl_easy_cleanup( job->get_url_handle() );
    job->abort();
}

void download_manager::abort_all()
{
    DOWNLOAD_JOBS::const_iterator it;
    
    for ( it  = m_download_jobs.begin(); it != m_download_jobs.end(); ++it )
    {
        if ( (*it) == NULL )
            continue;
        abort_job( *it );
    }
}