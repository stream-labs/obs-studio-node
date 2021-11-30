/*
 * Profiny - Lightweight Profiler Tool
 * Copyright (C) 2013 Sercan Tutar
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * USAGE:
 *   First PROFINY_CALL_GRAPH_PROFILER or PROFINY_FLAT_PROFILER must be defined
 *   (giving as a compiler option is advised). If you;
 *
 *     - define PROFINY_CALL_GRAPH_PROFILER, it will work as a call-graph profiler
 *     - define PROFINY_FLAT_PROFILER, it will work as a flat profiler
 *     - define neither, Profiny macros will be set to blank (i.e. profiling will be off)
 *     - define both, it will give an error
 *
 *   Later, if you chose PROFINY_CALL_GRAPH_PROFILER, you may want to determine
 *   whether recursive calls will be omitted or not (omitted by default) by calling:
 *
 *     Profiler::setOmitRecursiveCalls(bool)
 *
 *   By default (if the profiling is not off), if your program exits normally, Profinity
 *   will print results in "profinity.out" file. Also, the user can force printing results
 *   at any time by calling:
 *
 *     Profiler::printStats("filename")
 *
 *   See ReadMe.txt for more info.
 *
 *
 * Happy profiling!
 *
 */

#pragma once
#ifndef PROFINY_H_
#define PROFINY_H_


#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


#if defined(PROFINY_CALL_GRAPH_PROFILER) && defined(PROFINY_FLAT_PROFILER)

#	error "PROFINY_CALL_GRAPH_PROFILER and PROFINY_FLAT_PROFILER should not be defined at the same time!"

#elif defined(PROFINY_CALL_GRAPH_PROFILER) || defined(PROFINY_FLAT_PROFILER)

#	define PROFINY_SCOPE \
		std::ostringstream _oss; \
		_oss << /*__FILE__ << ":" << */__FUNCTION__ << ":" << __LINE__; \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_SCOPE_WITH_ID(ID) \
		std::ostringstream _oss; \
		_oss << /*__FILE__ << ":" << */__FUNCTION__ << ":" << __LINE__ << ":" << (ID); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_NAMED_SCOPE(NAME) \
		std::ostringstream _oss; \
		_oss << (NAME); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID) \
		std::ostringstream _oss; \
		_oss << (NAME) << ":" << (ID); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#else

#	define PROFINY_SCOPE

#	define PROFINY_SCOPE_WITH_ID(ID)

#	define PROFINY_NAMED_SCOPE(NAME)

#	define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID)

#endif


#define NANOSEC_TO_SEC(X) ((X) / 1000000000.0)


namespace profiny
{
	class Timer
	{
	public:
		Timer();

		void start();

		void stop();

		double getElapsedTime();

	private:
		double m_startTime;

		double m_stopTime;

		bool m_running;

#ifdef _WIN32
		double m_reciprocalFrequency;
#endif

		double getTime();
	};

	/**********************************************************************/

	class BaseObject
	{
	public:
		BaseObject();

		virtual ~BaseObject();

		void incrRef();

		void decrRef();

		int getRef() const;

	private:
		int m_ref;
	};

	/**********************************************************************/

	class Profile : public BaseObject
	{
		friend class ScopedProfile;
		friend class Profiler;

	private:
		Profile(const std::string& name);

		~Profile();

		bool start();

		bool stop();

		unsigned int getCallCount() const;

		std::string getName() const;

		void getTimes(double& wall) const;

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::map<std::string, Profile*>& getSubProfiles();

		std::map<std::string, Profile*> m_subProfiles;
#else
		bool m_timeStarted;
#endif

		std::string m_name;

		unsigned int m_callCount;

		double m_wallTime;

		Timer m_timer;
	};

	/**********************************************************************/

	class ScopedProfile : public BaseObject
	{
	public:
		ScopedProfile(const std::string& name);

		~ScopedProfile();

	private:
		Profile* m_profile;
	};

	/**********************************************************************/

	class Profiler : public BaseObject
	{
		friend class Profile;
		friend class ScopedProfile;

	public:
		static void printStats(const std::string& filename);

#ifdef PROFINY_CALL_GRAPH_PROFILER
		static void setOmitRecursiveCalls(bool omit);

		static bool getOmitRecursiveCalls();
#endif

	private:
		Profiler();

		~Profiler();

		static Profiler* getInstance();

		Profile* getProfile(const std::string& name);

		static void printStats();

		static void printStats(std::ofstream& fs, std::map<std::string, Profile*>* p, int depth);

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::map<std::string, Profile*>& getCurrentProfilesRoot();

		void pushProfile(Profile* p);

		void popProfile();

		bool isInStack(const std::string& name);
#endif

		std::map<std::string, Profile*> m_profiles;

		static Profiler* m_instance;

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::vector<Profile*> m_profileStack;

		bool m_omitRecursiveCalls;
#endif
	};

	/**********************************************************************/

} // namespace profiny

/**********************************************************************/

inline void intrusive_ptr_add_ref(profiny::BaseObject* p)
{
    if (p != NULL)
    { // pointer is not NULL
        p->incrRef();
    }
}

inline void intrusive_ptr_release(profiny::BaseObject* p)
{
    if (p != NULL)
    { // pointer is not NULL
        p->decrRef();
        if (p->getRef() <= 0)
        { // reference count is zero or less
            delete p;
        }
    }
}

#endif /* PROFINY_H_ */