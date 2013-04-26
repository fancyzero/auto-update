#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "download_manager.h"
#include "update_manager.h"

using namespace cocos2d;
using namespace CocosDenshion;

update_manager* update_man = NULL;
download_manager* download_man = NULL;
download_job* job = NULL;
download_manager* get_download_manager();

enum update_state
{
    download_file_list,
    download_files,
};

update_state g_updatestate = download_file_list;

CCLabelTTF* log_label;

update_manager* get_update_manager()
{
    if ( update_man == NULL )
    {
        update_man = new update_manager();
        std::string path;
        
        path = CCFileUtils::sharedFileUtils()->fullPathForFilename("filelist.xml");
        
        update_man->set_download_manager( get_download_manager() );
        update_man->set_root_path(CCFileUtils::sharedFileUtils()->getWritablePath());
        update_man->load_file_list( path );
        
    }
    return update_man;
}



download_manager* get_download_manager()
{
    if ( download_man == NULL )
        download_man = new download_manager();
    return download_man;
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback) );
    pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20) );

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition( CCPointZero );
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    log_label = CCLabelTTF::create("Hello World", "Thonburi", 14);

    // ask director the window size
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // position the label on the center of the screen
    log_label->setPosition( ccp(size.width / 2, size.height /2 ) );

    // add the label as a child to this layer
    this->addChild(log_label, 1);

    // add "HelloWorld" splash screen"
    //CCSprite* pSprite = CCSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    //pSprite->setPosition( ccp(size.width/2, size.height/2) );

    // add the sprite as a child to this layer
    //this->addChild(pSprite, 0);
    this->scheduleUpdate();
    
    
    g_updatestate = download_file_list;
    get_update_manager()->update_file_list("http://updateserver/filelist.xml", "filelist.xml", "hahahahaha");
    return true;
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    get_download_manager()->abort_all();
    //CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    //exit(0);
#endif
}



void HelloWorld::update(float delta)
{
    static float curtime = 0;
    setPosition(ccp(sin(curtime)*50, 0));
    curtime += delta;
 
    switch ( g_updatestate )
    {
        case download_file_list:
            if ( get_update_manager()->is_update_finished() )
            {
                g_updatestate = download_files;
                file_list fl = get_update_manager()->get_update_list( "" );
                get_update_manager()->download_files( fl );
            }
            break;
        case download_files:
            //do nothing
            break;
    }
    download_manager::download_status st = get_download_manager()->get_status();
    log_label->setString( st.current_file.c_str() );
    
    get_download_manager()->update();
}