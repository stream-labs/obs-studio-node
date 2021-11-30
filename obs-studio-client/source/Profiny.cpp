#include "Profiny.h"

profiny::Timer::Timer()
    : m_startTime(0.0f), m_stopTime(0.0f), m_running(false)
{
#ifdef _WIN32
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    m_reciprocalFrequency = 1.0f / freq.QuadPart;
#endif
}

void profiny::Timer::start()
{
    m_running = true;
    m_startTime = getTime();
}

void profiny::Timer::stop()
{
    m_running = false;
    m_stopTime = getTime() - m_startTime;
}

double profiny::Timer::getElapsedTime()
{
    if (m_running)
        return getTime() - m_startTime;

    return m_stopTime;
}

double profiny::Timer::getTime()
{
#ifdef _WIN32
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    double time = count.QuadPart * m_reciprocalFrequency;
#else
    struct timeval interval;
    clock_gettime(CLOCK_MONOTONIC, &interval);
    double time = interval.tv_sec + interval.tv_usec * 0.000001f;
#endif

    return time;
};

/**********************************************************************/

 profiny::BaseObject::BaseObject() :
        m_ref(0)
{
}

 profiny::BaseObject::~BaseObject()
{
}

 void profiny::BaseObject::incrRef()
{
    ++m_ref;
}

 void profiny::BaseObject::decrRef()
{
    --m_ref;
}

 int profiny::BaseObject::getRef() const
{
    return m_ref;
}

/**********************************************************************/

 profiny::Profile::Profile(const std::string& name) :
#ifndef PROFINY_CALL_GRAPH_PROFILER
        m_timeStarted(false),
#endif
        m_name(name), m_callCount(0), m_wallTime(0.0)
{
}

 profiny::Profile::~Profile()
{
}

 bool profiny::Profile::start()
{
#ifdef PROFINY_CALL_GRAPH_PROFILER
    Profiler::getInstance()->pushProfile(this);
#else
    if (m_timeStarted)
    {
        return false;
    }
    m_timeStarted = true;
#endif
    m_timer.start();
    return true;
}

 bool profiny::Profile::stop()
{
#ifdef PROFINY_CALL_GRAPH_PROFILER
    Profiler::getInstance()->popProfile();
#else
    if (!m_timeStarted)
    {
        return false;
    }
    m_timeStarted = false;
#endif
    m_timer.stop(); // TODO: check if we need this line
    m_wallTime += m_timer.getElapsedTime();
    ++m_callCount;
    return true;
}

 unsigned int profiny::Profile::getCallCount() const
{
    return m_callCount;
}

 std::string profiny::Profile::getName() const
{
    return m_name;
}

 void profiny::Profile::getTimes(double& wall) const
{
    wall = m_wallTime;
}

#ifdef PROFINY_CALL_GRAPH_PROFILER
 std::map<std::string, Profile*>& profiny::Profile::getSubProfiles()
{
    return m_subProfiles;
}
#endif

/**********************************************************************/

 profiny::ScopedProfile::ScopedProfile(const std::string& name) : m_profile(NULL)
{
    std::string n(name);

#ifdef PROFINY_CALL_GRAPH_PROFILER
    if (Profiler::getInstance()->isInStack(n))
    { // profile is already in stack (probably a recursive call)
        if (Profiler::getInstance()->getOmitRecursiveCalls())
        {
            return;
        }
        else
        {
            n = "RECURSIVE@" + n;
        }
    }
#endif

    m_profile = Profiler::getInstance()->getProfile(n);
    if (m_profile != NULL)
    {
        if (!m_profile->start())
        { // cannot start profiler (probably a recursive call for flat profiler)
            delete m_profile;
            m_profile = NULL;
        }
    }
    else
    {
        std::cerr << "Cannot start scoped profiler: " << n << std::endl;
    }
}

 profiny::ScopedProfile::~ScopedProfile()
{
    if (m_profile != NULL)
    {
        m_profile->stop();
    }
}

/**********************************************************************/

profiny::Profiler* profiny::Profiler::m_instance = NULL;

 profiny::Profiler::Profiler()
#ifdef PROFINY_CALL_GRAPH_PROFILER
    : m_omitRecursiveCalls(true)
#endif
{
}

 profiny::Profiler::~Profiler()
{
}

 profiny::Profiler* profiny::Profiler::getInstance()
{
    if (m_instance == NULL)
    {
        m_instance = new Profiler;
        atexit(printStats);
    }
    return m_instance;
}

 profiny::Profile* profiny::Profiler::getProfile(const std::string& name)
{
#ifdef PROFINY_CALL_GRAPH_PROFILER
    std::map<std::string, Profile*>& profiles = getCurrentProfilesRoot();
#else
    std::map<std::string, Profile*>& profiles = m_profiles;
#endif
    std::map<std::string, Profile*>::iterator it = profiles.find(name);
    if (it != profiles.end())
    {
        return it->second;
    }
    else
    {
        Profile* result = new Profile(name);
        profiles[name] = result;
        return result;
    }
}

#ifdef PROFINY_CALL_GRAPH_PROFILER
 std::map<std::string, Profile*>& Profiler::getCurrentProfilesRoot()
{
    return m_profileStack.empty() ? m_profiles : m_profileStack.back()->getSubProfiles();
}

 void Profiler::pushProfile(Profile* p)
{
    m_profileStack.push_back(p);
}

 void Profiler::popProfile()
{
    if (!m_profileStack.empty())
    {
        m_profileStack.pop_back();
    }
}

 bool Profiler::isInStack(const std::string& name)
{
    for (unsigned int i=0; i<m_profileStack.size(); ++i)
    {
        if (m_profileStack[i]->getName() == name)
        {
            return true;
        }
    }
    return false;
}
#endif

 void profiny::Profiler::printStats(std::ofstream& fs, std::map<std::string, profiny::Profile*>* p, int depth)
{
#ifdef PROFINY_CALL_GRAPH_PROFILER
    std::ostringstream oss;
    for (int i=0; i<depth; ++i)
    {
        oss << "\t";
    }
#endif

    for (auto it = p->begin(); it != p->end(); ++it)
    {
        unsigned int cc = it->second->getCallCount();
        double wall;
        it->second->getTimes(wall);
#ifdef PROFINY_CALL_GRAPH_PROFILER
        fs << oss.str() << it->second->getName() << "  T(s):" << wall << "  #:" << cc << "  A(ms):" << wall * 1000 / cc << std::endl;
        printStats(fs, &(it->second->getSubProfiles()), depth+1);
#else
        fs << it->second->getName() << "  T(s):" << wall << "  #:" << cc << "  A(ms):" << wall * 1000 / cc << " total(ms): " << wall * 1000 << std::endl;
#endif
        delete it->second;
    }
}

 void profiny::Profiler::printStats()
{
    printStats("profiny.out");

    delete m_instance;
    m_instance = NULL;
}

 void profiny::Profiler::printStats(const std::string& filename)
{
    std::ofstream fs;
    fs.open(filename.c_str());
    if (!fs.is_open())
    {
        std::cerr << "Cannot open profiler output file: " << filename << std::endl;
        return;
    }
    Profiler::printStats(fs, &(getInstance()->m_profiles), 0);
    fs.close();
}

#ifdef PROFINY_CALL_GRAPH_PROFILER
 void profiny::Profiler::setOmitRecursiveCalls(bool omit)
{
    getInstance()->m_omitRecursiveCalls = omit;
}

 bool profiny::Profiler::getOmitRecursiveCalls()
{
    return getInstance()->m_omitRecursiveCalls;
}
#endif