include_directories(${CMAKE_CURRENT_BINARY_DIR}) 
 
set(viper_src
    adblock/AdBlockFilter.cpp
    adblock/AdBlockFilterParser.cpp
    adblock/AdBlockLog.cpp
    adblock/AdBlockLogTableModel.cpp
    adblock/AdBlockManager.cpp
    adblock/AdBlockModel.cpp
    adblock/AdBlockSubscription.cpp
    autofill/AutoFill.cpp
    autofill/AutoFillBridge.cpp
    bookmarks/BookmarkExporter.cpp
    bookmarks/BookmarkFolderModel.cpp
    bookmarks/BookmarkImporter.cpp
    bookmarks/BookmarkManager.cpp
    bookmarks/BookmarkNode.cpp
    bookmarks/BookmarkTableModel.cpp
    cookies/CookieJar.cpp
    cookies/CookieTableModel.cpp
    cookies/DetailedCookieTableModel.cpp
    credentials/CredentialStore.cpp
    database/DatabaseWorker.cpp
    downloads/InternalDownloadItem.cpp
    extensions/ExtStorage.cpp
    highlighters/HTMLHighlighter.cpp
    highlighters/JavaScriptHighlighter.cpp
    history/FavoritePagesManager.cpp
    history/HistoryManager.cpp
    history/HistoryTableModel.cpp
    history/WebPageThumbnailStore.cpp
    icons/FaviconStore.cpp
    icons/FaviconStoreBridge.cpp
    network/BlockedSchemeHandler.cpp
    network/HttpRequest.cpp
    network/NetworkAccessManager.cpp
    network/RequestInterceptor.cpp
    network/SchemeRegistry.cpp
    network/SecurityManager.cpp
    network/ViperNetworkReply.cpp
    network/ViperSchemeHandler.cpp
    search/SearchEngineManager.cpp
    session/SessionManager.cpp
    settings/Settings.cpp
    text_finder/ITextFinder.cpp
    text_finder/TextEditorTextFinder.cpp
    text_finder/WebPageTextFinder.cpp
    url_suggestion/URLSuggestionListModel.cpp
    url_suggestion/URLSuggestionWorker.cpp
    user_agents/UserAgentManager.cpp
    user_scripts/UserScript.cpp
    user_scripts/UserScriptManager.cpp
    user_scripts/UserScriptModel.cpp
    user_scripts/WebEngineScriptAdapter.cpp
    utility/CommonUtil.cpp
    utility/FastHash.cpp
    web/URL.cpp
    web/WebActionProxy.cpp
    web/WebHistory.cpp
    web/WebHitTestResult.cpp
    web/WebPage.cpp
)

if (KF5Wallet_FOUND)
  set(viper_src ${viper_src} credentials/CredentialStoreKWallet.cpp)
endif()

set(viper_core_deps
    ${viper_src}
)

add_library(viper-core STATIC ${viper_src}) 
target_link_libraries(viper-core
    Qt5::Core
    Qt5::Concurrent
    Qt5::Gui
    Qt5::Network
    Qt5::PrintSupport
    Qt5::Sql
    Qt5::Svg
    Qt5::QuickWidgets
    Qt5::Widgets
    Qt5::WebChannel
    Qt5::WebEngineWidgets
)

if (KF5Wallet_FOUND)
    target_link_libraries(viper-core KF5::Wallet)
endif()