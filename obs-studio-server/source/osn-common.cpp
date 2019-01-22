// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "osn-common.hpp"

typedef struct pthread_mutex_t_* my_pthread_mutex_t;

struct my_obs_context_data
{
	char*             name;
	void*             data;
	obs_data_t*       settings;
	signal_handler_t* signals;
	proc_handler_t*   procs;
	enum obs_obj_type type;

	DARRAY(obs_hotkey_id) hotkeys;
	DARRAY(obs_hotkey_pair_id) hotkey_pairs;
	obs_data_t* hotkey_data;

	DARRAY(char*) rename_cache;
	my_pthread_mutex_t rename_cache_mutex;

	my_pthread_mutex_t*       mutex;
	struct obs_context_data*  next;
	struct obs_context_data** prev_next;

	bool pprivate;
};

struct my_obs_weak_ref
{
	volatile long refs;
	volatile long weak_refs;
};

struct my_obs_weak_source
{
	struct my_obs_weak_ref ref;
	struct obs_source*  source;
};

struct my_obs_source
{
	struct my_obs_context_data context;
	struct obs_source_info  info;
	struct my_obs_weak_source* control;

	/* general exposed flags that can be set for the source */
	uint32_t flags;
	uint32_t default_flags;
};

void ValidateSourceSanity(obs_source_t* source)
{
	volatile my_obs_source* mySource = (my_obs_source*)source;

	if (!mySource)
		goto Crash;

	if (!mySource->control)
		goto Crash;

	if (!mySource->control->ref.refs < 0)
		goto Crash;

	return;

Crash:

	abort();
}