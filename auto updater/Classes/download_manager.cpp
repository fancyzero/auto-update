//
//  download_manager.cpp
//  auto updater
//
//  Created by FancyZero on 13-4-17.
//
//

#include "download_manager.h"
#include "curl.h"
#include  "md5.h"
#include <sys/stat.h>

/*
 download_job
 */
static size_t download_job_write_function(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    download_job* job = (download_job*)userdata;

    FILE *fp = job->get_file_handle();
    size_t written = fwrite(ptr, size, nmemb, fp);
    job->add_current_unflushed( written );
    //DOWNLOAD_LOG( "write: %d written: %d total: %d", size*nmemb, written );
    return written;
}

static int download_job_progress(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    //DOWNLOAD_LOG( "downloading... %f, %f", nowDownloaded, totalToDownload );
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

void download_job::add_current_unflushed( int size )
{
    m_current_unflushed += size;
    m_total_written += size;
   
    if ( m_current_unflushed > m_flush_step_size )
    {
        
        fflush( m_pf );
        m_current_unflushed -= m_flush_step_size;
         DOWNLOAD_LOG( "flush: %d  total: %d", m_current_unflushed, m_total_written );
    }
}
FILE* download_job::get_file_handle()
{
    return m_pf;
}

void download_job::add_total_written( int size )
{
    
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

bool download_job::is_succeeded()
{
    return is_completed() && m_stat.result == succeeded;
}
bool download_job::is_completed()
{
    return m_stat.state == completed;
}

int download_job::prepare_download()
{
    //look for resume download

    /*
    size_t readed_total = 0;
    int ret = 0;
    if ( m_pf != NULL || m_pf_hash != NULL )
        return -1;
    std::string dest_file_name = m_desc.dest_file;
    std::string hash_file_name = m_desc.dest_file + ".hash";
    unsigned char final_md5[MD5_DIGEST_LEN];
    unsigned char job_md5[MD5_DIGEST_LEN];
    unsigned char file_chunk_md5[MD5_DIGEST_LEN];
    unsigned char hash_chunk_md5[MD5_DIGEST_LEN];
    m_pf = fopen( dest_file_name.c_str(), "rb" );
    m_pf_hash = fopen( hash_file_name.c_str(), "rb" );
    if ( m_pf == NULL || m_pf_hash == NULL )
    {
        ret = 0;
        goto cleanup;
    }
    if ( fread( final_md5, sizeof(final_md5), 1, m_pf_hash ) != 1 )
    {
        ret = 0;
        goto cleanup;
    }
    //check if resume data expired
    md_context md5_ctx;
    md5_begin( &md5_ctx );
    string_to_md5(this->m_desc.hash, job_md5 );
    if ( memcmp( final_md5, job_md5, sizeof(final_md5) ) != 0 )
    {
        ret = 0;
        goto cleanup;
    }

    while( !feof(m_pf) && !feof(m_pf_hash) )
    {
        
        unsigned char data[1024];
        readed_total = 0;
        if ( fread( hash_chunk_md5, sizeof(hash_chunk_md5), 1, m_pf_hash ) != 1 )
            break;
        size_t readed = 0;
        for ( int i = 0; i < 100; i++ )
        {
            readed = fread( data, 1, 1024, m_pf );
            readed_total += readed;
            if ( readed <1024 )
                break;
            md5_update( &md5_ctx, data, 1024 );
        }
        
        if ( readed < 1024*100 )//is last chunk?
        {
            md5_update( &md5_ctx, data, readed );
            md5_result( &md5_ctx, file_chunk_md5 );
            if ( memcmp( hash_chunk_md5, file_chunk_md5, sizeof( hash_chunk_md5 ) ) == 0 )
                ret += readed_total;
            break;
        }
        else
        {
            md5_result( &md5_ctx, file_chunk_md5 );
            if ( memcmp( hash_chunk_md5, file_chunk_md5, sizeof( hash_chunk_md5 ) ) == 0 )
                ret += readed_total;
            else
                break;
            
        }
    }
    
cleanup:
    if ( m_pf )
        fclose( m_pf );
    if ( m_pf_hash )
        fclose( m_pf_hash );
    m_pf = m_pf_hash = NULL;
    
    return 0;
     */
    int ret = 0;
    std::string dest_file_name = m_desc.dest_file;
    std::string hash_file_name = m_desc.dest_file + ".hash";
    struct stat st;
    if ( m_pf != NULL || m_pf_hash != NULL )
        return 0;

    unsigned char final_md5[MD5_DIGEST_LEN];
    unsigned char job_md5[MD5_DIGEST_LEN];
    m_pf_hash = fopen( hash_file_name.c_str(), "rb" );

    if ( m_pf_hash != NULL )
    {
        if ( fread( final_md5, sizeof(final_md5), 1, m_pf_hash ) != 1 )
        {
            ret = 0;
            goto cleanup;
        }
    }
    string_to_md5(this->m_desc.hash, job_md5 );
    if ( memcmp( final_md5, job_md5, sizeof(final_md5) ) != 0 )
    {
        ret = 0;
        goto cleanup;
    }

    stat(dest_file_name.c_str(), &st);
    ret = st.st_size;
cleanup:
    if ( m_pf )
        fclose( m_pf );
    if ( m_pf_hash )
        fclose( m_pf_hash );
    m_pf = m_pf_hash = NULL;
    return ret;
}

bool download_job::download()
{
    m_current_unflushed = 0;
    m_total_written = 0;
    int download_start = prepare_download() - this->m_flush_step_size;
    if ( download_start < 0 )
        download_start = 0;
    DOWNLOAD_LOG("%x resume from: %d", this, download_start );
    if ( m_pf != NULL || m_pf_hash != NULL )
        return true;
    std::string hash_file_name = m_desc.dest_file + ".hash";
    std::string dest_file_name = m_desc.dest_file;
    std::string src_url = m_desc.src_url;
    
    m_pf_hash = fopen( hash_file_name.c_str(), "wb" );
    unsigned char job_md5[MD5_DIGEST_LEN];
    string_to_md5( m_desc.hash, job_md5 );
    if ( m_pf_hash != NULL )
    {
        fwrite( job_md5, 1, sizeof(job_md5), m_pf_hash );
        fclose( m_pf_hash );
        m_pf_hash = NULL;
    }

    if ( download_start > 0 )
    {
        m_pf = fopen(m_desc.dest_file.c_str(), "rb+");
    }
    else
    {
        m_pf = fopen(m_desc.dest_file.c_str(), "wb");
    }
    if ( m_pf == NULL )
    {
        DOWNLOAD_LOG("can not create file %s", dest_file_name.c_str());
        set_failed();
        return false;
    }
    fseek( m_pf, download_start, SEEK_SET );

    DOWNLOAD_LOG("create file %s for write", dest_file_name.c_str());
    
    // Download pacakge
    //CURLcode res = 0;
    CURL* curl = curl_easy_init();
    
    if ( download_start > 0 )
    {
        curl_easy_setopt( curl, CURLOPT_RESUME_FROM, download_start );
    }
    curl_easy_setopt( curl, CURLOPT_URL, src_url.c_str() );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, download_job_write_function );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );
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
    DOWNLOAD_LOG("start downloading file %s", src_url.c_str());
    
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
    return NULL;
}

int download_manager::get_succeeded_job_count()
{
    int count = 0;
    DOWNLOAD_JOBS::const_iterator it;
    for ( it  = m_download_jobs.begin(); it != m_download_jobs.end(); ++it )
    {
        if ( (*it)->is_succeeded() )
            count ++;
    }
    return count;
}

int download_manager::get_job_count()
{
    return m_download_jobs.size();
}

void download_manager::clean_succeeded_job()
{
    DOWNLOAD_JOBS unsucceeded_jobs;
    DOWNLOAD_JOBS::const_iterator it;
    for ( it  = m_download_jobs.begin(); it != m_download_jobs.end(); ++it )
    {
        if ( (*it)->is_succeeded() )
        {
            delete *it;
        }
        else
        {
            unsucceeded_jobs.push_back( (*it) );
        }
    }
    m_download_jobs = unsucceeded_jobs;
    
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
            tv.tv_sec = 0;
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