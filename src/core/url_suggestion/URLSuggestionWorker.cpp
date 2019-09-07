#include "BookmarkManager.h"
#include "BookmarkNode.h"
#include "BookmarkSuggestor.h"
#include "CommonUtil.h"
#include "FastHash.h"
#include "FaviconManager.h"
#include "HistoryManager.h"
#include "HistorySuggestor.h"
#include "URLSuggestion.h"
#include "URLSuggestionWorker.h"

#include <algorithm>
#include <chrono>
#include <iterator>

#include <QtConcurrent>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include <QDebug>

// Used to sort results by relevance in the URLSuggestionWorker
bool compareUrlSuggestions(const URLSuggestion &a, const URLSuggestion &b)
{
    // Account for these factors, in order
    // 1) Number of times the URL was previously typed into the URL bar (ignore this check if a and b have never been typed into the bar)
    // 2) Closeness of the url to the user input (ex: search="viper.com", a="vipers-are-cool.com", b="viper.com/faq", choose b)
    // 2a) Closeness of search term components to url and title components, where applicable
    // 3) Number of visits to the urls
    // 4) Most recent visit
    // [disabled] 5) Type of match to the search term (ex: the page title vs the URL)
    // 5) Alphabetical ordering

    if (!a.URLTypedCount != !b.URLTypedCount)
        return a.URLTypedCount > b.URLTypedCount;

    if (a.IsHostMatch != b.IsHostMatch)
        return a.IsHostMatch;

    // Special case
    if (a.Type == b.Type && a.PercentMatch != b.PercentMatch)
        return a.PercentMatch > b.PercentMatch;

    if (a.VisitCount != b.VisitCount)
        return a.VisitCount > b.VisitCount;

    if (a.LastVisit != b.LastVisit)
        return a.LastVisit < b.LastVisit;

    //if (a.Type != b.Type)
    //    return static_cast<int>(a.Type) < static_cast<int>(b.Type);

    return a.URL < b.URL;
}

URLSuggestionWorker::URLSuggestionWorker(QObject *parent) :
    QObject(parent),
    m_working(false),
    m_searchTerm(),
    m_searchWords(),
    m_suggestionFuture(),
    m_suggestionWatcher(nullptr),
    m_suggestions(),
    m_searchTermWideStr(),
    m_differenceHash(0),
    m_searchTermHash(0),
    m_handlers()
{
    m_suggestionWatcher = new QFutureWatcher<void>(this);
    connect(m_suggestionWatcher, &QFutureWatcher<void>::finished, this, [this](){
        emit finishedSearch(m_suggestions);
    });

    m_handlers.push_back(std::make_unique<BookmarkSuggestor>());
    m_handlers.push_back(std::make_unique<HistorySuggestor>());

    // Allow url suggestion handlers to do things, such as refreshing data, on a regular interval
    QTimer *wordTimer = new QTimer(this);
    wordTimer->setSingleShot(false);
    wordTimer->setInterval(std::chrono::milliseconds(1000 * 60));

    connect(wordTimer, &QTimer::timeout, this, &URLSuggestionWorker::onHandlerTimerTick);
    connect(wordTimer, &QTimer::timeout, wordTimer, static_cast<void(QTimer::*)()>(&QTimer::start));

    wordTimer->start();
}

void URLSuggestionWorker::onHandlerTimerTick()
{
    for (auto &handler : m_handlers)
        handler->timerEvent();
}

void URLSuggestionWorker::findSuggestionsFor(const QString &text)
{
    if (m_working)
    {
        m_working.store(false);
        m_suggestionFuture.cancel();
        m_suggestionFuture.waitForFinished();
    }

    m_searchTerm = text.toUpper().trimmed();

    // Remove any http or https prefix from the term, since we do not want to
    // do a string check on URLs and potentially remove an HTTPS match because
    // the user only entered HTTP 
    QRegularExpression httpExpr(QLatin1String("^HTTP(S?)://"));
    m_searchTerm.replace(httpExpr, QString());

    // Split up search term into different words
    m_searchWords = CommonUtil::tokenizePossibleUrl(m_searchTerm);

    hashSearchTerm();

    m_suggestionFuture = QtConcurrent::run(this, &URLSuggestionWorker::searchForHits);
    m_suggestionWatcher->setFuture(m_suggestionFuture);
}

void URLSuggestionWorker::setServiceLocator(const ViperServiceLocator &serviceLocator)
{
    for (auto &handler : m_handlers)
        handler->setServiceLocator(serviceLocator);

    onHandlerTimerTick();
}

void URLSuggestionWorker::searchForHits()
{
    m_working.store(true);
    m_suggestions.clear();

    FastHashParameters hashParams { m_searchTermWideStr, m_differenceHash, m_searchTermHash };
    for (auto &handler : m_handlers)
    {
        if (!m_working.load())
            return;

        std::vector<URLSuggestion> suggestions
                = handler->getSuggestions(m_working, m_searchTerm, m_searchWords, hashParams);
        m_suggestions.insert(m_suggestions.end(), suggestions.begin(), suggestions.end());
    }

    if (!m_working.load())
        return;

    std::sort(m_suggestions.begin(), m_suggestions.end(), compareUrlSuggestions);
    if (m_suggestions.size() > 35)
        m_suggestions.erase(m_suggestions.begin() + 35, m_suggestions.end());

    m_working.store(false);
}

void URLSuggestionWorker::hashSearchTerm()
{
    m_searchTermWideStr = m_searchTerm.toStdWString();
    m_differenceHash = FastHash::getDifferenceHash(static_cast<quint64>(m_searchTerm.size()));
    m_searchTermHash = FastHash::getNeedleHash(m_searchTermWideStr);
}
